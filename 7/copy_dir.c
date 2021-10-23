#include "copy_dir.h"

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

int get_dir_fd(DIR *dir)
{
    int dir_fd;
    if ((dir_fd = dirfd(dir)) == -1)
    {
        perror("Failed to dirfd: ");
        exit(1);
    }

    return dir_fd;
}

void full_copy_dir(DIR *source, DIR *target)
{
    int dir_fd_r = get_dir_fd(source);
    int dir_fd_w = get_dir_fd(target);

    struct dirent *r_source;
    struct stat sb;

    while ((r_source = readdir(source)) != NULL)
    {
        copy_with_metadata(r_source->d_name, r_source->d_name, dir_fd_r, dir_fd_w);
    }
}
