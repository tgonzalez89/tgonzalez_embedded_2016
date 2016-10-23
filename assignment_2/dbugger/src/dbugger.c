//gcc -o dbugger dbugger.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

// Obtain min and max addresses of binary being debugged
int get_bin_min_max_addr(char* binary, unsigned long* min_addr, unsigned long* max_addr) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) {
        // Redirect stdout to a log file
        int f = open("objdump.log", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        if (f < 0) {
            printf("-E- Can't open file objdump.log");
            return -1;
        }
        dup2(f, 1);
        close(f);
        // Run objdump
        execl("/usr/bin/objdump", "objdump", "-d", binary, NULL);
        printf("-E- Child process didn't run properly.\n");
        return -1;
    } else {
        wait(NULL); // Wait for the child to end
        // Read the log file that the child generated and get the results
        FILE* fp;
        if ((fp = fopen("objdump.log", "r")) == NULL) {
            printf("-E- Can't open file objdump.log for read.\n");
            return -1;
        }
        char line[1024];
        unsigned long addr;
        *min_addr = 0xffffffffffffffff;
        *max_addr = 0;
        while (fgets(line, sizeof(line), fp) != NULL) {
            // Try to obtain min and max
            if (sscanf(line, "  %lx:%*s", &addr) == 1) {
                if (addr < *min_addr) *min_addr = addr;
                if (addr > *max_addr) *max_addr = addr;
            }
        }
        fclose(fp);
    }
}

// Obtain a global variable address and size
int get_var_info(char* binary, char* var_name, long* var_addr, long* var_size) {
    int found = 0;
    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) {
        // Redirect stdout to a log file
        int f = open("nm.log", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
        if (f < 0) {
            printf("-E- Can't open file nm.log");
            return -1;
        }
        dup2(f, 1);
        close(f);
        // Run nm
        execl("/usr/bin/nm", "nm", "-S", binary, NULL);
        printf("-E- Child process didn't run properly.\n");
        return -1;
    } else {
        wait(NULL); // Wait for the child to end
        // Read the log file that the child generated and get the results
        FILE* fp;
        if ((fp = fopen("nm.log", "r")) == NULL) {
            printf("-E- Can't open file nm.log for read.\n");
            return -1;
        }
        char line[1024];
        char var_name_testing[1024];
        while (!found && fgets(line, sizeof(line), fp) != NULL) {
            // Try to obtain var address
            if (sscanf(line, "%lx %lx %*c %s", var_addr, var_size, var_name_testing) == 3)
                if (strcmp(var_name, var_name_testing) == 0)
                    found = 1;
        }
        fclose(fp);
        if (!found) printf("-E- No global variable named '%s'.\n", var_name);
    }
    return found;
}

void help() {
    printf("Usage: dbugger [options]\nOptions:\n");
    printf("  -b BINARY\t\tBinary program to debug.\n");
    printf("  -w VAR_NAME\t\tName of global variable for the debugger to watch.\n");
    printf("  -s\t\t\tStep mode.\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    return;
}

int main(int argc, char* argv[]) {
    // Process arguments
    char* binary = NULL;
    char* var_name = NULL;
    long var_addr = 0;
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
                //var_addr = strtol(optarg, NULL, 16);
                var_name = optarg;
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

    // Obtain min and max addresses of binary being debugged
    unsigned long min_addr = 0;
    unsigned long max_addr = 0xffffffffffffffff;
    get_bin_min_max_addr(binary, &min_addr, &max_addr);
    //printf("-D- min_addr: 0x%lx, max_addr: 0x%lx\n", min_addr, max_addr);
    //getchar();

    // 10% extra: Obtain variable address (and size) from its name
    unsigned long var_size = 8;
    if (var_name != NULL && get_var_info(binary, var_name, &var_addr, &var_size) <= 0)
        return -1;

    pid_t pid = fork();
    if (pid < 0) {
        printf("-E- Fork failed.\n");
        return -1;
    } else if (pid == 0) { // This is the child with the program to analyze
        ptrace(PTRACE_TRACEME, pid, NULL, NULL);
        execl(binary, binary, NULL);
        printf("-E- Binary didn't run properly.\n");
        return -1;
    } else { // This is the parent program
        struct user_regs_struct regs;
        unsigned long ins;
        int status;
        long var_value;
        long prev_var_value;
        int var_value_init = 0;
        do {
            waitpid(pid, &status, 0); // Wait child
            ptrace(PTRACE_GETREGS, pid, NULL, &regs); // Get regs values
            // Only interact with instructions inside min and max
            if ((unsigned long)regs.rip >= min_addr && (unsigned long)regs.rip <= max_addr) {
                ins = ptrace(PTRACE_PEEKTEXT, pid, regs.rip, NULL);
                printf("-I- PC: 0x%016llx Instruction executed: 0x%016lx\n", regs.rip, ins);

                // If variable changed, print old and new value
                if (var_name != NULL) {
                    prev_var_value = var_value;
                    if (!var_value_init) {
                        prev_var_value = ptrace(PTRACE_PEEKTEXT, pid, var_addr, NULL);
                        var_value_init = 1;
                    }
                    var_value = ptrace(PTRACE_PEEKTEXT, pid, var_addr, NULL);
                    // Adjust value depending on size
                    long real_var_value;
                    long real_prev_var_value;
                    switch (var_size) {
                        case 1:
                            real_var_value = (char)var_value;
                            real_prev_var_value = (char)prev_var_value;
                            break;
                        case 2:
                            real_var_value = (short)var_value;
                            real_prev_var_value = (short)prev_var_value;
                            break;
                        case 4:
                            real_var_value = (int)var_value;
                            real_prev_var_value = (int)prev_var_value;
                            break;
                        case 8:
                            break;
                        default:
                            printf("-W- Variable size is %ld bytes. Supported sizes are: 1, 2, 4 and 8. Its value may be wrong.\n", var_size);
                            break;
                    }
                    if (real_prev_var_value != real_var_value) {
                        printf("-I- Variable '%s' at 0x%016lx changed:\n", var_name, var_addr);
                        printf("-I- Old value = 0x%016lx (%ld)\n", prev_var_value, real_prev_var_value);
                        printf("-I- New value = 0x%016lx (%ld)\n", var_value, real_var_value);
                        /*if (!step_mode) {
                            fprintf(stderr, "Press ENTER to continue...");
                            getchar();
                        }*/
                    }
                }

                // In step mode wait for user to press Enter
                if (step_mode) {
                    fprintf(stderr, "...Press ENTER to continue...");
                    getchar();
                }
            }
            ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL); // Continue to next instruction
            //ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        } while (!WIFEXITED(status));
        printf("-I- Program exited!\n");
    }

    return 0;
}
