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

        char *outFile = NULL;
        char *inFile = NULL;
        for (int i = 0; i < arg_count; i++) {
            if (strcmp(args[i], ">") == 0) {
                if (i + 1 < arg_count) {
                    outFile = args[i + 1];
                    args[i] = NULL;
                } else {
                    perror("no output file provided");
                }
                break;
            }
            if (strcmp(args[i], "<") == 0) {
                if (i + 1 < arg_count) {
                    inFile = args[i + 1];
                    args[i] = NULL;
                } else {
                    perror("no input file provided");
                }
            }
        }

        int piping = -1;
        for (int i = 0; i < arg_count; i++) {
            if (strcmp(args[i], "|") == 0) {
                piping = i;
                args[i] = NULL;
                break;
            }
        }

        if (piping != -1) {

            char **left_args = args;
            char **right_args = &args[piping + 1];

            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                continue;
            }

            // Left (Producer)
            pid_t pid1 = fork();
            if (pid1 == 0) {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);

                execvp(left_args[0], left_args);
                perror("exec failed (left)");
                exit(1);
            }

            // Right (Consumer)
            pid_t pid2 = fork();
            if (pid2 == 0) {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);

                execvp(right_args[0], right_args);
                perror("exec failed (right)");
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        } else {

            // Standard Fork
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork failed");
            } else if (pid == 0) {
                // Child process
                if (outFile != NULL) {
                    int fd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
                if (inFile != NULL) {
                    int fd = open(inFile, O_RDONLY);
                    if (fd == -1) {
                        perror("open failed");
                        exit(1);
                    }
                    if (dup2(fd, STDIN_FILENO) == -1) {
                        perror("dup2 failed");
                    }
                    close(fd);
                }

                execvp(args[0], args);
                perror("exec failed");
                exit(1);
            } else {
                // Parent process
                waitpid(pid, NULL, 0);
            }
        }
    }
    return 0;
}