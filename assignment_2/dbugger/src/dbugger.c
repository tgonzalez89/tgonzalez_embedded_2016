//gcc -o dbugger dbugger.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

void help() {
    printf("Usage: dbugger [options]\nOptions:\n");
    printf("  -b BINARY\t\tBinary program to debug.\n");
    printf("  -w VAR_ADDR\t\tAddress of variable for the debugger to watch.\n");
    printf("  -s\t\t\tStep mode.\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    return;
}

int main(int argc, char* argv[]) {

    // Process arguments
    char* binary = NULL;
    long var_addr = 0;
    int var_addr_opt = 0;
    int step_mode = 0;
    int arg;
    while ((arg = getopt(argc, argv, "hab:w:s")) != -1) {
        switch (arg) {
            case 'h':
                help();
                return 0;
            case 'a':
                printf("Author: Tomas Gonzalez Aragon\n");
                return 0;
            case 'b':
                binary = optarg;
                break;
            case 'w':
                var_addr = strtol(optarg, NULL, 0);
                var_addr_opt = 1;
                break;
            case 's':
                step_mode = 1;
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
    if (binary == NULL) {
        printf("-E- No valid options.\n");
        help();
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) { // This is the child with the program to analyze
        ptrace(PTRACE_TRACEME, pid, NULL, NULL);
        execl(binary, binary, NULL);
    } else { // This is the parent program
        struct user_regs_struct regs;
        long ins;
        int status;
        int var_value = ptrace(PTRACE_PEEKTEXT, pid, var_addr, NULL);
        int prev_var_value;
        do {
            waitpid(pid, &status, 0);
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            if (step_mode) {
                fprintf(stderr, "Press ENTER to continue...");
                getchar();
            }
            if (var_addr_opt) {
                prev_var_value = var_value;
                var_value = ptrace(PTRACE_PEEKTEXT, pid, var_addr, NULL);
                if (prev_var_value != var_value) {
                    printf("Variable at 0x%lx changed:\nOld value = %d\nNew value = %d\n", var_addr, prev_var_value, var_value);
                    if (!step_mode) {
                        fprintf(stderr, "Press ENTER to continue...");
                        getchar();
                    }
                }
            }
            ins = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);
            printf("PC: 0x%llx Instruction executed: 0x%lx\n", regs.rip, ins);
            ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL);
            //ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        } while (!WIFEXITED(status));
        printf("Program exited!\n");
    }

    return 0;
}
