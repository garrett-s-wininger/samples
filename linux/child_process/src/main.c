#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    pid_t forked_pid = fork();

    if (forked_pid == 0)
    {
        printf("CHILD: Entered new process\n");
        exit(0);
    }
    else if (forked_pid > 0)
    {
        printf("PARENT: Waiting on process (%d)...\n", forked_pid);
        
        int status;
        pid_t changed_pid = waitpid(forked_pid, &status, 0);

        if (changed_pid != forked_pid)
        {
            fprintf(stderr, "ERROR: Received error while waiting on child process\n");
            exit(-1);
        }

        printf("PARENT: Child exited successfully\n");
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to fork child process\n");
        exit(-1);
    }
}