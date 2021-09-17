#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdint.h> 
#include <unistd.h> 

ssize_t dprintf_all(int fd, const char  *buf, size_t str_len)  
{
	size_t bytes_written = 0;  
	while (bytes_written < (size_t) str_len) 
	{
		ssize_t res = dprintf(fd, "%s", buf + bytes_written); 
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
	
	int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644); 

	if (fd < 0) 
	{
		perror("Failed to open file for writing"); 
		return 2;
	}

	if (dprintf_all(fd, argv[2],  strlen(argv[2])) < 0)
	{
		perror("Failed to write"); 
		close(fd); 
		return 3;
	}

	if (close(fd) < 0) 
	{
		perror("Failure during close"); 
		return 4; 
	}

	return 0; 
}
