#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
//#include <linux/fcntl.h>
#define BUFF_SIZE 128

#define BUF_SIZE_CPY 4096

ssize_t write_all(int fd, const void *buf, size_t count)
{
    size_t bytes_written = 0;
    const uint8_t *buf_adder = buf;
    while (bytes_written < (size_t)count)
    {
        ssize_t res = write(fd, buf_adder + bytes_written, count - bytes_written);
        if (res < 0)
            return res;
        bytes_written += res;
    }
    return (ssize_t)bytes_written;
}

int my_copy(int fd_r, int fd_w)
{
    char *str_tmp;

    str_tmp = (char *)malloc(BUF_SIZE_CPY);
    if (str_tmp == NULL)
    {
        perror("Faild to allocate the memory");
        return 6;
    }

    int check;

    while ((check = read(fd_r, str_tmp, BUF_SIZE_CPY)) != 0)
    {
        if (check < 0)
        {
            perror("Failed to read the file");
            return 7;
        }

        if (write_all(fd_w, str_tmp, strlen(str_tmp)) < 0)
        {
            perror("Failed to write");
            return 8;
        }
    }

    return 0;
}

void copy_blk_chr(char *pathname, mode_t mode, dev_t dev)
{
    dev_t dev_res = makedev(major(dev), minor(dev));
    if (mknod(pathname, mode, dev_res) == -1)
    {
        perror("mknod");
        exit(1);
    }
}

void copy_fifo(char *pathname, mode_t mode)
{
    if (mkfifo(pathname, mode) == -1)
    {
        perror("mkfifo");
        exit(1);
    }
}

void copy_lnk(char *pathname, char *pathfrom)
{
    size_t bufsiz = 2048;
    char buf[bufsiz];

    int len = 0;

    if ((len = readlink(pathfrom, buf, bufsiz)) == -1)
    {
        perror("readlink");
        exit(1);
    }

    buf[len] = '\0';

    if (symlink(buf, pathname) == -1)
    {
        perror("symlink");
        exit(1);
    }
}

void solve(const struct stat st, char *pathname, char *pathfrom)
{
    switch (st.st_mode & S_IFMT)
    {
    case S_IFBLK:
        copy_blk_chr(pathname, st.st_mode, st.st_dev);
        break; //"block device";
    case S_IFCHR:
        copy_blk_chr(pathname, st.st_mode, st.st_dev);
        break;
    case S_IFIFO:
        copy_fifo(pathname, st.st_mode);
        break; //"FIFO/pipe";
    case S_IFLNK:
        copy_lnk(pathname, pathfrom);
        break; //"symlink";
    case S_IFREG: {
        int fd_r = open(pathfrom, O_RDONLY, S_IRUSR | S_IRGRP);

        if (fd_r < 0)
        {
            perror("Failed to open file for reading");
            exit(1);
        }

        int fd_w = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd_w < 0)
        {
            perror("Failed to open file for writing");
            exit(1);
        }

        if (my_copy(fd_r, fd_w) > 0)
        {
            perror("Copy");
            exit(1);
        }

    } //"regular file";
    break;
    default: {
        printf("We cann't copy this");
        exit(1);
    } //"unknown?";
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <pathname>  %d\n", argv[0], argc);
        exit(EXIT_FAILURE);
    }

    struct stat st;

    if (lstat(argv[1], &st) == -1)
    {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    solve(st, argv[2], argv[1]);

    exit(EXIT_SUCCESS);
}
