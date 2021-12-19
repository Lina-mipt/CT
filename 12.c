#include<stdio.h> 
#include<unistd.h> 
#include<sys/types.h>
#include<limits.h>
#include<grp.h> 
#include<pwd.h> 
#include<string.h> 
#include<stdlib.h> 
#include<sys/time.h>
#include<sys/resource.h>
#include <sys/stat.h>

int main(void) 
{
	pid_t pid = getpid(); 
	printf("PID %d,\t PPID %d\n", pid, getppid()); 
	printf("PGID %d,\t Session ID %d\n", getpgrp(), getsid(pid)); 

	//получаем имя и real ID
	uid_t uid = getuid(); 
	gid_t gid = getgid(); 

    struct passwd * pwuid = getpwuid(uid);
    struct group * grid = getgrgid(gid);
	
	printf("Real user ID %d(%s),\t real group ID %d(%s)\n", uid, (pwuid ? pwuid->pw_name : "?"), gid, (grid ? grid->gr_name : "?"));	

	//effective user/group ID 
	uid_t euid = getuid(); 
	gid_t egid = getgid(); 

    struct passwd * epwuid = getpwuid(euid);
    struct group * egrid = getgrgid(egid);
	
	printf("Effective user ID %d(%s),\t effective group ID %d(%s)\n", euid, (epwuid ? epwuid->pw_name : "?"), egid, (egrid ? egrid->gr_name : "?"));

    int ngroups = getgroups(0, NULL);

    gid_t* groups = (gid_t*)malloc((long unsigned)(ngroups + 1)*sizeof(gid_t));
    int num_g = -1;
    if ((num_g = getgroups(ngroups + 1, groups)) == -1) 
	{
        perror("getgroups");
        exit(1);
    }

    printf("Supplementary groups(%d):\n ", num_g);
    for (int i = 0; i < num_g; i++) 
	{
        struct group * ngrid = getgrgid(groups[i]);
        printf("%d(%s)\n ", groups[i], ngrid ? ngrid->gr_name : "?");
    }
    printf("\n");
    free(groups);

	// get umask
    mode_t u_mask = umask(ALLPERMS);
    printf("umask: %o\n", u_mask);
    u_mask = umask(u_mask); 

    // Sheduling priority
    int pr = getpriority(PRIO_PROCESS, 0);
    if(pr == -1) {
        perror("Failure getpriority");
        exit(EXIT_FAILURE);
    }
    printf("Sheduling priority: %d\n", pr);

	return 0; 
}




