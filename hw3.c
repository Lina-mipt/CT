#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdint.h> 
#include <unistd.h>
#include <stdlib.h>

#define STR_SIZE 256 

void good_close(int fd) 
{
	if (close(fd) < 0) 
	{
		perror("Failure during close"); 
		exit(7); 
	}
}

ssize_t write_all(int fd, const void *buf, size_t count) 
{
	size_t bytes_written = 0; 
	const uint8_t *buf_adder = buf; 
	while (bytes_written < (size_t) count) 
	{
		ssize_t res = write(fd, buf_adder + bytes_written, count - bytes_written); 
		if(res < 0) 
			return res; 
		bytes_written += res; 
			
	}
	return (ssize_t)bytes_written; 
}

int main(int argc, char * argv[]) 
{
	if (argc != 3) 
	{
		fprintf(stderr, "Usage: %s path text", argv[0]); 
		return 1; 
	}
	
	int fd_r = open(argv[1], O_RDONLY, S_IRUSR | S_IRGRP);  

	if (fd_r < 0) 
	{
		perror("Failed to open file for reading"); 
		return 2;
	} 

	int fd_w = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644); 

	if (fd_w < 0) 
	{
		perror("Failed to open file for writing"); 
		return 3;
	} 

	char * str_tmp; 

	str_tmp = (char*)malloc(STR_SIZE); 
	if (str_tmp == NULL) 
	{
		perror("Faild to allocate the memory");
		good_close(fd_r); 
		good_close(fd_w); 
		return 4;
	}

	int check; 

	while((check = read(fd_r, str_tmp, STR_SIZE)) != 0) 
	{
		if(check < 0) 
		{
			perror("Failed to read the file"); 
			good_close(fd_w); 
			good_close(fd_r); 
			return 5;  
		}
		
		if (write_all(fd_w, str_tmp,  strlen(str_tmp)) < 0)
		{
			perror("Failed to write"); 
			good_close(fd_w); 
			good_close(fd_r);
			return 6;
		}
	}

	good_close(fd_r); 
	good_close(fd_w);  

	return 0; 
}
