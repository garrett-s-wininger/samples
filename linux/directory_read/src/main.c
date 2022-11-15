#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    DIR* dir = opendir(".");

    if (!dir)
    {
        perror("Unable to open specified directiory");
        exit(EXIT_FAILURE);
    }

    printf("Entries in current directory:\n\n");

    for (;;)
    {
        errno = 0;
        struct dirent* result = readdir(dir);

        if (!result)
        {
            if (errno != 0)
            {
                perror("Could not read from directory stream\n");
                closedir(dir);
                exit(EXIT_FAILURE);
            }
            else
            {
                break;
            }
        }
        else
        {
            printf("(%d) %s\n", result->d_ino, result->d_name);
        }
    }

    closedir(dir);
    return 0;
}