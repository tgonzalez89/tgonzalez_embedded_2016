//gcc -o memcheck memcheck.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait
#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <fcntl.h> // open, close

void help () {
    printf("Usage: memcheck [options]\nOptions:\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    printf("  -p PROGRAM\t\tDo memory analysis of PROGRAM.\n");
    return;
}

int main (int argc, char* argv[]) {
    char* program = NULL;
    int arg;
    while ((arg = getopt(argc, argv, "hap:")) != -1) {
        switch (arg) {
            case 'h':
                help();
                return 0;
            case 'a':
                printf("Author: Tomas Gonzalez Aragon\n");
                return 0;
            case 'p':
                program = optarg;
                break;
            case '?':
                help();
                return -1;
            default:
                help();
                return -1;
        }
    }

    int i;
    for (i = optind; i < argc; i++)
        printf ("-W- Ignoring non-option argument: '%s'\n", argv[i]);
    if (program == NULL) {
        printf("-E- No valid options.\n");
        help();
        return -1;
    }

    char fullpath[PATH_MAX+1];
    char* ret = realpath(argv[0], fullpath);
    char* dirpath = dirname(fullpath);
    char preload_lib[PATH_MAX+64];
    snprintf(preload_lib, PATH_MAX+64, "%s%s%s", "LD_PRELOAD=", dirpath, "/../lib/libmemcheck.so");
    char* env[] = {preload_lib, NULL};
    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) {
        int f = open("memcheck.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (f < 0) {
            printf("-E- Can't open file memcheck.log");
            return -1;
        }
        dup2(f, 2);
        close(f);
        execle(program, program, NULL, env);
        printf("-E- Child process didn't run properly.\n");
        return -1;
    } else {
        wait(NULL);
    }

    return 0;
}
