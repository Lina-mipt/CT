#pragma once
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

int good_close(int fd);

ssize_t write_all(int fd, const void *buf, size_t count);

int my_copy(int fd_r, int fd_w);

void copy_blk_chr(const char *output_f, int dir_fd_w, mode_t mode, dev_t dev);

void copy_fifo(const char *output_f, int dir_fd_w, mode_t mode);

void copy_lnk(const char *output_f, int dir_fd_w, const char *input_f, const off_t size);

void copy_dir(const char *output_f, int dir_fd_w, mode_t mode);

void copy_file(const char *output_f, const char *input_f, int dir_fd_r, int dir_fd_w);

void copy_all(const struct stat *st, const char *output_f, const char *input_f, int dir_fd_r, int dir_fd_w);

void copy_metodata(int dir_fd_r, int dir_fd_w, const char* input_f, const char* output_f, const struct stat* st);

int copy_with_metadata(const char *input_f, const char *output_f, int dir_fd_r, int dir_fd_w);
