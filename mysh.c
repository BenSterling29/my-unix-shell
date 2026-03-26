// mysh - a simple Unix shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(void) {
    char line[1024];

    char *args[64];
    int arg_count;

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        char *newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        arg_count = 0;
        args[arg_count] = strtok(line," ");

        while (args[arg_count] != NULL) {
            arg_count++;
            args[arg_count] = strtok(NULL," ");
        }

        printf("You entered: %s\n", line);
    }
    return 0;
}