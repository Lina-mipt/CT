#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t cpid, w;
    int wstatus;

    cpid = fork();

    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0)
    { /* Code executed by child */
        printf("Child PID is %ld\n", (long)getpid());
        if (argc == 1)
            pause(); /* Wait for signals */
        _exit(atoi(argv[1]));
    }
    else
    { /* Code executed by parent */
        do
        {
            w = waitpid(cpid, &wstatus, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(wstatus))
            {
                printf("exited, status=%d\n", WEXITSTATUS(wstatus));
            }
            else if (WIFSIGNALED(wstatus))
            {
                printf("killed by signal %d\n", WTERMSIG(wstatus));
            }
            else if (WIFSTOPPED(wstatus))
            {
                printf("stopped by signal %d\n", WSTOPSIG(wstatus));
            }
            else if (WIFCONTINUED(wstatus))
            {
                printf("continued\n");
            }
        } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
        exit(EXIT_SUCCESS);
    }
    /*
        WIFEXITED(wstatus)   -- не равно нулю, если дочерний процесс успешно завершился
        WIFSIGNALED(wstatus) -- возвращает истинное значение, если дочерний процесс завершился из-за необработанного
       сигнала WIFSTOPPED(wstatus)  -- возвращает истинное значение, если дочерний процесс, из-за которого функция
       вернула управление, в настоящий момент остановлен; это возможно, только если использовался флаг WUNTRACED
    */
}
