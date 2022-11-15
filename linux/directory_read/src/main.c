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
        fprintf(stderr, "ERROR: Unable to open specified directiory\n");
        exit(1);
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
                closedir(dir);
                fprintf(stderr, "ERROR: Could not read from directory stream\n");
                return 1;
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