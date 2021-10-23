#include "my_copy.h"

int good_close(int fd)
{
    if (close(fd) < 0)
    {
        perror("Failure during close");
        return 10;
    }

    return 0;
}

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

        if (write_all(fd_w, str_tmp, check) < 0)
        {
            perror("Failed to write");
            return 8;
        }
    }

    free(str_tmp);
    return 0;
}

void copy_blk_chr(const char *output_f, int dir_fd_w, mode_t mode, dev_t dev)
{
    if (mknodat(dir_fd_w, output_f, mode, dev) == -1)
    {
        perror("mknod");
        exit(1);
    }
}

void copy_fifo(const char *output_f, int dir_fd_w, mode_t mode)
{
    if (mkfifoat(dir_fd_w, output_f, mode) == -1)
    {
        perror("mkfifo");
        exit(1);
    }
}

void copy_dir(const char *output_f, int dir_fd_w, mode_t mode)
{
    if (strcmp(output_f, ".") == 0 || strcmp(output_f, "..") == 0)
     return; 
    if (mkdirat(dir_fd_w, output_f, mode) == -1)
    {
        printf("%s\n", output_f);
        perror("mkdir");
        exit(1);
    }
}

void copy_lnk(const char *output_f, int dir_fd_w, const char *input_f, const off_t size)
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

    if (symlinkat(buf, dir_fd_w, output_f) == -1)
    {
        perror("symlink");
        free(buf);
        exit(1);
    }

    free(buf);
}

void copy_file(const char *output_f, const char *input_f, int dir_fd_r, int dir_fd_w)
{
    int fd_r = openat(dir_fd_r, input_f, O_RDONLY, S_IRUSR | S_IRGRP);

    if (fd_r < 0)
    {
        perror("Failed to open file for reading");
        exit(1);
    }

    int fd_w = openat(dir_fd_w, output_f, O_WRONLY | O_CREAT | O_TRUNC, 0644);

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


void copy_metodata(int dir_fd_r, int dir_fd_w, const char* input_f, const char* output_f, const struct stat* st)
{
    int fd_r = openat(dir_fd_r, input_f, O_RDONLY); // file to read from

    if (fd_r == -1)
    {
        perror("Cannot open fileto read\n");
        exit(1);
    }

    int fd_w = openat(dir_fd_w, output_f, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // file to write in

    if (fd_w == -1)
    {
        perror("Cannot open file to write\n");
        exit(1);
    }

    int mode = fchmod(fd_w, st->st_mode);

    if (mode != 0)
    {
        perror("Error with copying meta data");
        exit(1);
    }

    struct timespec times[2] = {st->st_atim, st->st_mtim};

    int time = futimens(fd_w, times);

    if (time != 0)
    {
        perror("Error with copying time");
        exit(1);
    }

    int owner = fchown(fd_w, st->st_uid, st->st_gid);

    if (owner != 0)
    {
        perror("fchown");
        exit(1);
    }

    if (good_close(fd_r) + good_close(fd_w) > 0)
        exit(1);
}

void copy_all(const struct stat *st, const char *output_f, const char *input_f, int dir_fd_r, int dir_fd_w)
{
    switch (st->st_mode & S_IFMT)
    {
    case S_IFBLK:
        copy_blk_chr(output_f, dir_fd_w, st->st_mode, st->st_dev);
		copy_metodata(dir_fd_r, dir_fd_w, input_f, output_f, st);
        break; //"block device";
    case S_IFCHR:
        copy_blk_chr(output_f, dir_fd_w, st->st_mode, st->st_dev);
        copy_metodata(dir_fd_r, dir_fd_w, input_f, output_f, st);
		break;
    case S_IFIFO:
        copy_fifo(output_f, dir_fd_w, st->st_mode);
        copy_metodata(dir_fd_r, dir_fd_w, input_f, output_f, st);
		break; //"FIFO/pipe";
    case S_IFLNK:
        copy_lnk(output_f, dir_fd_w, input_f, st->st_size);
		copy_metodata(dir_fd_r, dir_fd_w, input_f, output_f, st);
        break; //"symlink";
    case S_IFREG:
        copy_file(output_f, input_f, dir_fd_r, dir_fd_w); //"regular file"
		copy_metodata(dir_fd_r, dir_fd_w, input_f, output_f, st);		
        break;
    case S_IFDIR:
        copy_dir(output_f, dir_fd_w, st->st_mode);
    default: {
        break;
    } //"unknown?";
    }
}

int copy_with_metadata(const char *input_f, const char *output_f, int dir_fd_r, int dir_fd_w)
{
    struct stat st;

    if (fstatat(dir_fd_r, input_f, &st, AT_SYMLINK_NOFOLLOW) == -1)
    {
        perror("fstatat");
        return 1;
    }

    copy_all(&st, output_f, input_f, dir_fd_r, dir_fd_w); 

	
    return 0;
}
