#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void proc_info(const char *procname)
{
    printf("%s: PID %d, PPID %d, PGID %d, SID %d\n", procname, getpid(), getppid(), getpgid(0), getsid(0));
}

int main(void)
{
    pid_t child_id = fork();
    if (child_id < 0)
    {
        perror("fork");
        return 1;
    }
    // this code is executed in child process only
    if (child_id == 0)
    {
        //перенаправляем поток ошибок
        if (dup2(fileno(stderr), fileno(stdout)) < 0)
        {
            perror("dup2");
        }
        proc_info("child");
        return 0;
    }
    // this code is executed in parent process only
    proc_info("parent");
    wait(NULL);

    return 0;
}
