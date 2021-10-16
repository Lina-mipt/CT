#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <linux/limits.h>
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

    ssize_t check;

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

    free(str_tmp);
    return 0;
}

void copy_blk_chr(const char *output_f, mode_t mode, dev_t dev)
{
    if (mknod(output_f, mode, dev) == -1)
    {
        perror("mknod");
        exit(1);
    }
}

void copy_fifo(const char *output_f, mode_t mode)
{
    if (mkfifo(output_f, mode) == -1)
    {
        perror("mkfifo");
        exit(1);
    }
}

void copy_lnk(const char *output_f, const char *input_f, const off_t size)
{
    size_t bufsize = size + 1;

    if (bufsize == 0)
        bufsize = PATH_MAX;

    char *buf = malloc(bufsize);

    if (buf == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    ssize_t len = 0;

    if ((len = readlink(input_f, buf, bufsize)) != bufsize)
    {
        perror("readlink");
        free(buf);
        exit(1);
    }

    buf[len] = '\0';

    if (symlink(buf, output_f) == -1)
    {
        perror("symlink");
        free(buf);
        exit(1);
    }

    free(buf);
}

void copy_file(const char *output_f, const char *input_f)
{
    int fd_r = open(input_f, O_RDONLY, S_IRUSR | S_IRGRP);

    if (fd_r < 0)
    {
        perror("Failed to open file for reading");
        exit(1);
    }

    int fd_w = open(output_f, O_WRONLY | O_CREAT | O_TRUNC, 0644);

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

    close(fd_r);
    close(fd_w);
}

void solve(const struct stat *st, const char *output_f, const char *input_f)
{
    switch (st->st_mode & S_IFMT)
    {
    case S_IFBLK:
        copy_blk_chr(output_f, st->st_mode, st->st_dev);
        break; //"block device";
    case S_IFCHR:
        copy_blk_chr(output_f, st->st_mode, st->st_dev);
        break;
    case S_IFIFO:
        copy_fifo(output_f, st->st_mode);
        break; //"FIFO/pipe";
    case S_IFLNK:
        copy_lnk(output_f, input_f, st->st_size);
        break; //"symlink";
    case S_IFREG:
        copy_file(output_f, input_f); //"regular file";
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
        fprintf(stderr, "Usage: %s <output_f>  %d\n", argv[0], argc);
        exit(EXIT_FAILURE);
    }

    struct stat st;

    if (lstat(argv[1], &st) == -1)
    {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    solve(&st, argv[2], argv[1]);

    exit(EXIT_SUCCESS);
}
