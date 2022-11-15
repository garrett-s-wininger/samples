#include <sys/wait.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    int pipe_fds[2];

    if (pipe(pipe_fds) == -1)
    {
        perror("Unable to create pipe and associated descriptors");
        exit(EXIT_FAILURE);
    }

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Unable to fork process");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        close(pipe_fds[1]);
        char received_char;

        printf("CHILD: Received data - ");
        fflush(stdout);

        while (read(pipe_fds[0], &received_char, 1) > 0)
        {
            write(STDOUT_FILENO, &received_char, 1);
        }

        write(STDOUT_FILENO, "\n", 1);
        close(pipe_fds[0]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipe_fds[0]);

        printf("PARENT: Writing to pipe...\n");
        char* msg = "Hello from parent!";
        write(pipe_fds[1], msg, strlen(msg));

        close(pipe_fds[1]);
        
        int status;
        
        if (waitpid(child_pid, &status, 0) == -1)
        {
            perror("Received error waiting on child");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
}