#define _XOPEN_SOURCE 500

#include <fcntl.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdint.h> 
#include <unistd.h>
#include <stdlib.h>
#include <limits.h> 

// I don't know how to realize fd closing in the best way. It seems to me, that my version looks more like a crutch than a beautiful and correct solution. So, I need your advice. 

int good_close(int fd) 
{
	if (close(fd) < 0) 
	{
		perror("Failure during close"); 
		return 7; 
	}

	return 0; 
}

ssize_t pwrite_all(int fd, const void *buf, size_t count, off_t pos) 
{
	size_t bytes_written = 0; 
	const uint8_t *buf_adder = buf; 
	while (bytes_written < (size_t) count) 
	{
		ssize_t res = pwrite(fd, buf_adder + bytes_written, count - bytes_written, pos); 
		if(res < 0) 
			return res; 
		pos += (off_t)res; 
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

	if(strcmp(argv[1], argv[2]) == 0) 
	{
		perror("You can't copy the file to itself"); 
		return 8; 
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

	str_tmp = (char*)malloc(SHRT_MAX); 
	if (str_tmp == NULL) 
	{
		perror("Faild to allocate the memory");
		if(good_close(fd_r) + good_close(fd_w) > 0) 
			return 7; 
		return 4;
	}

	int check; 
	off_t pos = 0; 
	while((check = pread(fd_r, str_tmp, SHRT_MAX, pos)) != 0) 
	{
		if(check < 0) 
		{
			perror("Failed to read the file"); 
			if(good_close(fd_w) + good_close(fd_r) > 0) 
				return 7;
			return 5;  
		}
		
		if (pwrite_all(fd_w, str_tmp,  strlen(str_tmp), pos) < 0)
		{
			perror("Failed to write"); 
			if(good_close(fd_w) + good_close(fd_r) > 0) 
				return 7; 
			return 6;
		}

		pos+=(off_t) check; 
	}

	if(good_close(fd_r) + good_close(fd_w) > 0) 
		return 7; 

	return 0; 
}