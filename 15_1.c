#define __USE_GNU
#define _GNU_SOURCE 1
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include<ucontext.h> 
#include<stdio.h> 

void myhandle(int mysignal, siginfo_t *si, void* arg)
{
  printf("Signal is %d\n",mysignal);

  ucontext_t *context = (ucontext_t *)arg;
  context->uc_mcontext.gregs[REG_RIP] = context->uc_mcontext.gregs[REG_RIP] + 0x04 ;
}

int main(void)
{   
    pid_t pid = getpid(); 
    printf("PID of Procces %d\n", pid);
    struct sigaction signals = {};
    signals.sa_handler = myhandle;
    sigemptyset(&signals.sa_mask);
    for (int sig_num = 1; sig_num <= NSIG; sig_num++)
        if (sigaction(sig_num, &signals, NULL)) {
            perror("sigaction");
            printf("Failed to set action for signal %i:\n", sig_num);
            psignal(sig_num, NULL);
        }

    for(;;){ 
        pause();
    }
    return 0;
}
