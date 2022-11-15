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
        exit(EXIT_SUCCESS);
    }
    else if (forked_pid > 0)
    {
        printf("PARENT: Waiting on process (%d)...\n", forked_pid);
        
        int status;

        if (waitpid(forked_pid, &status, 0) == -1)
        {
            perror("Received error while waiting on child process");
            exit(EXIT_FAILURE);
        }

        printf("PARENT: Child exited successfully\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        perror("Unable to fork child process");
        exit(EXIT_FAILURE);
    }
}