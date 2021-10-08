#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char mode_letter(unsigned mode)
{
    switch (mode & S_IFMT)
    {
    case S_IFBLK:
        return 'b';
    case S_IFCHR:
        return 'c';
    case S_IFDIR:
        return 'd';
    case S_IFIFO:
        return 'p';
    case S_IFLNK:
        return 'l';
    case S_IFREG:
        return '-';
    case S_IFSOCK:
        return 's';
    }

    return '?';
}

char dtype_letter(unsigned d_type)
{
    switch (d_type)
    {
    case DT_BLK:
        return 'b';
    case DT_CHR:
        return 'c';
    case DT_DIR:
        return 'd';
    case DT_FIFO:
        return 'p';
    case DT_LNK:
        return 'l';
    case DT_REG:
        return '-';
    case DT_SOCK:
        return 's';
    }

    return '?';
}

int dirw(char *dir_path, int lvl)
{
    DIR *dir_str = opendir(dir_path);
    if (dir_str == NULL)
    {
        perror("Failed to opendir");
        return -1;
    }

    int dir_fd;
    if ((dir_fd = dirfd(dir_str)) == -1)
    {
        perror("Failed to dirfd");
        return -1;
    }
    struct dirent *entry;
    struct stat sb;
    while ((entry = readdir(dir_str)) != NULL)
    {
        for (int i = lvl; i > 0; i--)
            printf("-");
        printf("%s", entry->d_name);

        char type = dtype_letter(entry->d_type);
        if (type == '?')
        {
            if (fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) == 0)
            {
                type = mode_letter(sb.st_mode);
            }
            printf(" %c %s\n", type, entry->d_name);

            if (S_ISDIR(sb.st_mode) && (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
            {
                char *file_path;
                if (asprintf(&file_path, "%s/%s", dir_path, entry->d_name) < 0)
                {
                    printf("Failed to asprintf: Insufficient storage space is available.");
                    return 1;
                }
                dirw(file_path, lvl + 1);
                free(file_path);
            }
        }
        else
        {
            printf(" %c \n", type);
            if (type == 'd' && (strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
            {
                char *file_path;
                if (asprintf(&file_path, "%s/%s", dir_path, entry->d_name) < 0)
                {
                    printf("Failed to asprintf: Insufficient storage space is available.");
                    return 1;
                }
                dirw(file_path, lvl + 1);
                free(file_path);
            }
        }
    }
    closedir(dir_str);
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <pathname> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    dirw(argv[1], 0);
}
