#include <sys/stat.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <dirent.h> 
#include <stdio.h> 
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>

char mode_letter(unsigned mode) 
{
	switch(mode & S_IFMT) 
	{
		case S_IFBLK: return 'b'; 
		case S_IFCHR: return 'c'; 
		case S_IFDIR: return 'd'; 
		case S_IFIFO: return 'p';
		case S_IFLNK: return 'l'; 
		case S_IFREG: return '-'; 
		case S_IFSOCK: return 's'; 
	}

	return '?'; 
}

char dtype_letter(unsigned d_type) 
{
	switch(d_type) 
	{
		case DT_BLK: return 'b'; 
		case DT_CHR: return 'c'; 
		case DT_DIR: return 'd'; 
		case DT_FIFO: return 'p'; 
		case DT_LNK: return 'l';
		case DT_REG: return '-'; 
		case DT_SOCK: return 's'; 
	}

	return '?'; 
}

int main(int argc, char * argv[])  
{
	if(argc != 2) 
	{
		fprintf(stderr, "Usage: %s <pathname> \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	DIR *dir_str = opendir(argv[1]); 
	if(dir_str == NULL) 
	{
		perror("Failed to opendir"); 
		return 1; 
	}

	int dir_fd;  

	if((dir_fd = dirfd(dir_str)) == -1)
	{
		perror("Failed to dirfd: "); 
		return 2; 
	}

	struct dirent *entry; 
	struct stat sb; 

	while ((entry = readdir(dir_str)) != NULL)
	{
		char type = dtype_letter(entry->d_type); 
        if (type == '?')
		{
		    if (fstatat(dir_fd, entry->d_name, &sb, AT_SYMLINK_NOFOLLOW) == -1)
		    {
			    type = mode_letter(sb.st_mode); 
		    }
        }

		printf("%c %s\n", type, entry->d_name);
	}
	
	if(closedir(dir_str) == -1) 
	{
		perror("Failed to close dir"); 
		return 4; 
	}

	return 0; 
}
