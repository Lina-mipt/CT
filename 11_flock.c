#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 256

int main(void) {
    int fd = open("counter.txt", O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);//-rw-r--r--r--
    if (fd == -1) {
        perror("Failed to open");
        return 0;
    }
    if(flock(fd, LOCK_EX) == -1) {
        perror("Failed to flock");
        close(fd);
        return 0;
    }
    
    char buf[BUFSIZE];
    if(read(fd, buf, sizeof(buf)) == -1) {
        perror("Failed to read");
        close(fd);
        return 0;
    }
    char *bufin;
    if (asprintf(&bufin, "%d", (atoi(buf) + 1)) < 0)
    {
        printf("Failed to asprintf");
        close(fd);
        free(bufin);
    }
    if(pwrite(fd, bufin, strlen(bufin), 0) == -1) {
        perror("Failed to pwrite");
        free(bufin);
        close(fd);
        return 0;
    }
    free(bufin);
    if(flock(fd, LOCK_UN) == -1) {
        perror("Failed to flock");
        close(fd);
        return 0;
    }
    close(fd);
    return 1;
}
