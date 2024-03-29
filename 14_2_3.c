#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>  

int main(void) {

    int pipe_fds[2];
    if (pipe(pipe_fds) < 0) {
        perror("pipe");
        return 1; 
    } 
    pid_t child_id = fork(); 
    if(child_id < 0) {
        perror("fork"); 
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return 1; 
    } 

    // this code executed in child
    if(child_id == 0) {
        //closed unused reading endpoint of the pipe
        close(pipe_fds[0]);  
        if(dup2(pipe_fds[1], fileno(stdout)) < 0) {
            perror("dup2"); 
            close(pipe_fds[1]);
            return 1; 
        } 
        close(pipe_fds[1]); 
        execlp(
            "last", 
            "last", NULL
        ); 
        perror("execlp"); 
        return 1;
    }  
    close(pipe_fds[1]);  
    if(dup2(pipe_fds[0], fileno(stdin)) < 0) {
        perror("dup2"); 
        close(pipe_fds[0]);
        return 1; 
    }  

    close(pipe_fds[0]); 
    execlp(
        "wc", 
        "wc", "-l", NULL
    );
    perror("failed to exec 'wc -l'"); 
    return 0;
}
