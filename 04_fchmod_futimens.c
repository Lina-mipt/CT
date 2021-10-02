#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 4096

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
  str_tmp = (char *)malloc(BUF_SIZE);

  if (str_tmp == NULL) 
  {
    perror("Faild to allocate the memory");
    return 7;
  }

  int check;

  while ((check = read(fd_r, str_tmp, BUF_SIZE)) != 0) 
  {
    if (check < 0) 
	{
      perror("Failed to read the file");
      return 8;
    }

    if (write_all(fd_w, str_tmp, strlen(str_tmp)) < 0) 
	{
      perror("Failed to write");
      return 9;
    }
  }

  free(str_tmp); 

  return 0;
}


int main(int argc, char *argv[]) 
{    
	if (argc != 3) 
	{
        printf ("Incorrect usage\n");
        return 1;
    }
 
	struct stat stat_buf;

    if (lstat (argv[1], &stat_buf) == -1) 
	{
		perror("Error with file to copy");
		return 2;
    }

	if (((stat_buf.st_mode) & (S_IFMT)) != S_IFREG) 
	{
		printf("File to copy is not regular file\n");
		return 3;
	}

    int fd_r = open(argv[1], O_RDONLY); //file to read from 
    
	if (fd_r == -1) 
	{
        perror("Cannot open fileto read\n");
        return 4;
    }

	int fd_w = open(argv[2], O_WRONLY| O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR); //file to write in
    
	if (fd_w == -1) 
	{
        perror("Cannot open file to write\n");
        return 5;
    }
	
	int result = my_copy(fd_r, fd_w); 

	int mode = fchmod(fd_w, stat_buf.st_mode);
	
	if (mode != 0) 
	{
		perror("Error with copying meta data");
	}
	
	struct timespec times[2] = {stat_buf.st_atim, stat_buf.st_mtim};
	
	int time = futimens(fd_w, times);

	if (time != 0) 
	{
		perror("Error with copying time");
	}

	if (good_close(fd_r) + good_close(fd_w) > 0)
		return 6;
    
	if (result > 0)
		return result;

    
}
