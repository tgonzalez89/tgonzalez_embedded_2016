//gcc -o memcheck memcheck.c (for use with LD_PRELOAD)
//gcc -L../lib -o memcheck memcheck.c -lmemcheck (works in this file, not with exec)
//export LD_LIBRARY_PATH=`realpath ../lib`

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>

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
    while ((arg = getopt(argc, argv, "ahp:")) != -1) {
        switch (arg) {
            case 'a':
                printf("Author: Tomas Gonzalez Aragon\n");
                return 0;
            case 'h':
                help();
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

    char* env[] = {"LD_PRELOAD=/home/tomas/Documents/tgonzalez_embedded_2016/assignment_1/memcheck/lib/.libs/libmemcheck.so", NULL};
    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
    } else if (pid == 0) {
        execle(program, program, NULL, env);
        printf("-E- Child process didn't run properly.\n");
    } else {
        wait(NULL);
    }

    //DEBUG
    free(malloc(10));

    return 0;
}
