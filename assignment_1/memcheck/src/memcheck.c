#include <stdio.h>
#include <unistd.h>

void help () {
    printf("Usage: memcheck [options]\nOptions:\n");
    printf("  -a\t\t\tPrint the information of the author of this program and exit.\n");
    printf("  -h\t\t\tPrint this message and exit.\n");
    printf("  -p PROGRAM\t\tDo memory analysis of PROGRAM.\n");
    return;
}

int main (int argc, char* argv[]) {
    char* program_name = NULL;
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
                program_name = optarg;
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
    if (program_name == NULL) {
        printf("-E- No valid options.\n");
        help();
        return -1;
    }

    printf("Program name: %s\n", program_name);

    return 0;
}
