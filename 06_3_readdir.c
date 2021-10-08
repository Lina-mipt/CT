#define HAVE_D_TYPE
#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#define handle_error(msg)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        perror(msg);                                                                                                   \
        exit(EXIT_FAILURE);                                                                                            \
    } while (0)

struct linux_dirent
{
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};

#define BUF_SIZE 64

char *f_type(char d_type)
{
    switch (d_type)
    {
    case DT_REG:
        return "regular";
    case DT_DIR:
        return "directory";
    case DT_FIFO:
        return "FIFO";
    case DT_SOCK:
        return "socket";
    case DT_LNK:
        return "symlink";
    case DT_BLK:
        return "block dev";
    case DT_CHR:
        return "char dev";
    default:
        return "???";
    }
}

int main(int argc, char *argv[])
{
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos;
    char d_type;

    fd = open(argc > 1 ? argv[1] : ".", O_RDONLY | O_DIRECTORY);
    if (fd == -1)
        handle_error("open");

    for (;;)
    {
        nread = syscall(SYS_getdents, fd, buf, BUF_SIZE);
        if (nread == -1)
            handle_error("getdents");

        if (nread == 0)
            break;

        printf("file type  d_name\n");
        for (bpos = 0; bpos < nread;)
        {
            d = (struct linux_dirent *)(buf + bpos);
            d_type = *(buf + bpos + d->d_reclen - 1);
            printf("%-10s %s\n", f_type(d_type), (char *)d->d_name);
            bpos += d->d_reclen;
        }
    }

    exit(EXIT_SUCCESS);
}
