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

        if (arg_count == 0) continue;

        if (strcmp(args[0],"cd") == 0) {
            char *target;

            if (args[1] == NULL) {
                target = getenv("HOME");
            } else {
                target = args[1];
            }
            if (chdir(target) == -1) {
                perror("cd failed");
            }
            continue;
        }

        if (strcmp(args[0],"exit") == 0) break;

        char *fileName = NULL;
        int direction = 0;
        for (int i = 0; i < arg_count; i++) {
            if (strcmp(args[i], ">") == 0) {
                direction = 1;
                if (i + 1 < arg_count) {
                    fileName = args[i + 1];
                    args[i] = NULL;
                } else {
                    perror("no output file provided");
                }
                break;
            }
            if (strcmp(args[i], "<") == 0) {
                direction = 2;
                if (i + 1 < arg_count) {
                    fileName = args[i + 1];
                    args[i] = NULL;
                } else {
                    perror("no input file provided");
                }
            }
        }



        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
        } else if (pid == 0) {
            // Child process

            if (fileName != NULL) {
                if (direction == 1) {
                    int fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open failed");
                        exit(1);
                    }
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2 failed");
                        close(fd);
                        exit(1);
                    }
                    close(fd);
                }
                if (direction == 2) {
                    int fd = open(fileName, O_RDONLY);
                    if (fd == -1) {
                        perror("open failed");
                        exit(1);
                    }
                    if (dup2(fd, STDIN_FILENO) == -1) {
                        perror("dup2 failed");
                    }
                    close(fd);
                }
            }


            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else {
            // Parent process
            waitpid(pid, NULL, 0);
        }



    }
    return 0;
}