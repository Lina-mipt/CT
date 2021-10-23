#pragma once
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "my_copy.h"

// char dtype_letter(unsigned d_type);

int get_dir_fd(DIR *dir);

void full_copy_dir(DIR *source, DIR *target);
