//gcc -o memcheck memcheck.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait
#include <limits.h> // PATH_MAX
#include <libgen.h> // dirname
#include <fcntl.h> // open, close
#include <sys/stat.h> // stat

void help() {
    printf("Usage: memcheck [options]\nOptions:\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    printf("  -p PROGRAM\t\tDo memory analysis of PROGRAM.\n");
    return;
}

int main(int argc, char* argv[]) {

    // Process arguments
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

    // Get libmemcheck.so real path
    char fullpath[PATH_MAX+1];
    char* ret = realpath(argv[0], fullpath);
    char* dirpath = dirname(fullpath);
    char lib[PATH_MAX+32];
    snprintf(lib, PATH_MAX+32, "%s%s", dirpath, "/../lib/libmemcheck.so");
    if (access(lib, R_OK & F_OK) == -1) {
        printf("-E- Can't read library %s\n", lib);
        return -1;
    }
    // Set the LD_PRELOAD with the libmemcheck.so to be able
    // to analyze the memory with the overloaded functions.
    char preload_lib[PATH_MAX+64];
    snprintf(preload_lib, PATH_MAX+64, "%s%s", "LD_PRELOAD=", lib);
    char* env[] = {preload_lib, NULL};

    // Fork another process to run the program to analyze
    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) { // This is the child with the program to analyze
        printf("-I- %s is running...\n", program);
        // Run the program and redirect the stderr to a log.
        // This log will contain any stderr outputs from the program plus the malloc and free
        // messages from the overloaded functions in libmemcheck.so.
        int f = open("memcheck.log", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        if (f < 0) {
            printf("-E- Can't open file memcheck.log");
            return -1;
        }
        // Redirect stdout to another log file
        int f2 = open("program_output.log", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        if (f2 < 0) {
            printf("-E- Can't open file program_output.log");
            return -1;
        }
        dup2(f, 2); // redirect stderr
        dup2(f2, 1); // redirect stdout
        close(f);
        close(f2);
        // Run the program using LD_PRELOAD
        execle(program, program, NULL, env);
        printf("-E- Child process didn't run properly.\n");
        return -1;
    } else { // This is the parent program
        // Wait for the child to end
        wait(NULL);
        // Read the log file that the child generated and get the results
        FILE* fp;
        if ((fp = fopen("memcheck.log", "r")) == NULL) {
            printf("-E- Can't open file memcheck.log for read.\n");
            return -1;
        }
        char line[1024];
        int size, mallocs, frees, diff;
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (sscanf(line, "%*x = malloc(%d), mallocs: %d frees: %d diff: %d", &size, &mallocs, &frees, &diff) == 4) {
                //printf("m %d %d %d : %d\n", mallocs, frees, diff, size);
            } else if (sscanf(line, "free(%*x), mallocs: %d frees: %d diff: %d", &mallocs, &frees, &diff) == 3) {
                //printf("f %d %d %d\n", mallocs, frees, diff);
            } else {
                printf("-W- Invalid line: %s", line);
            }
        }
        fclose(fp);

        // stdout bug workaround
        struct stat fileStat;
        stat("program_output.log", &fileStat);
        if (fileStat.st_size != 0) {
            mallocs--;
            diff--;
        }

        printf("-I- Memory analysis finished.\n");
        printf("-I- Allocations:   %d\n", mallocs);
        printf("-I- Deallocations: %d\n", frees);
        printf("-I- Leaks found:   %d\n", diff);
    }

    return 0;
}
