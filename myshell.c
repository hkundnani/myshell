#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

const int SIZE = 80;
const char *COMMANDS[] = {"exit", "cd", "pwd", "set"};

void read_input(char *input) {
    fgets(input, 80, stdin);
}

void parse_input (char *input, char **tokens) {
    char *token;
    int index = 0;
    
    token = strtok(input, " \t\n\r");
    
    while (token != NULL) {
        tokens[index] = token;
        index++;
        token = strtok(NULL, " \t\n\r");
    }
    tokens[index] = NULL;
}

int handle_in_redirection(char *filename, char **tokens) {
    int status;
    FILE *fp;
    pid_t pid = fork();
    if (pid == 0) {
        fp = freopen(filename, "r", stdin);
        if (fp == NULL) {
            printf ("Error in opening file\n");
        }
        if (execvp(tokens[0], tokens) == -1) {
            perror("Error");
        }
        fclose(fp);
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

int handle_out_redirection(char *filename, char **tokens) {
    int status;
    FILE *fp;
    pid_t pid = fork();
    if (pid == 0) {
        fp = freopen(filename, "w+", stdout);
        if (fp == NULL) {
            printf ("Error in opening file\n");
        }
        if (execvp(tokens[0], tokens) == -1) {
            perror("Error");
        }
        fclose(fp);
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

int handle_in_out_redirection(char * filenameIn, char *filenameOut, char **tokens) {
    int status;
    FILE *fpIn;
    FILE *fpOut;
    pid_t pid = fork();
    if (pid == 0) {
        fpIn = freopen(filenameIn, "r", stdin);
        fpOut = freopen(filenameOut, "w+", stdout);
        if (fpIn == NULL || fpOut == NULL) {
            printf ("Error in opening file\n");
        }
        if (execvp(tokens[0], tokens) == -1) {
            perror("Error");
        }
        fclose(fpIn);
        fclose(fpOut);
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

int parse_io_char(char **tokens) {
    char *filenameIn = NULL;
    char *filenameOut = NULL;
    int posIn = 0;
    int posOut = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            filenameIn = tokens[i+1];
            posIn = i;
        }
        if (strcmp(tokens[i], ">") == 0) {
            filenameOut = tokens[i+1];
            posOut = i;
        }
    }

    if (filenameIn != NULL && filenameOut == NULL) {
        if (posIn > 0) {
            tokens[posIn] = NULL;
            return handle_in_redirection(filenameIn, tokens);
        } else {
            return 1;
        }
    } else if (filenameIn == NULL && filenameOut != NULL) {
        if (posOut > 0) {
            tokens[posOut] = NULL;
            return handle_out_redirection(filenameOut, tokens);
        } else {
            return 1;
        }
    } 
    else if (filenameIn != NULL && filenameOut != NULL) {
        if (posIn > 0 && posOut > 0) {
            tokens[posIn] = NULL;
            tokens[posOut] = NULL;
            return handle_in_out_redirection(filenameIn, filenameOut, tokens);
        } else {
            return 1;
        }
    } 
    else {
        return 0;
    }
}

void parse_pipe_char(char **tokens, int *pipe_pos) {
    int index = 0;
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            pipe_pos[index] = i;
            index++;
        }
    }
}

int myshell_cd(char **tokens) {
    if (tokens[1] == NULL) {
        char *token = getenv("HOME");
        if (chdir(token) != 0) {
            printf ("Error executing cd command\n");
        }
    } else {
        if (chdir(tokens[1]) != 0) {
            printf ("Error executing cd command\n");
        }
    }
    return 1; 
}

int myshell_exit(char **tokens) {
    return 0;
}

int myshell_pwd(char **tokens) {
    char *dir = (char*)malloc(sizeof(char) * SIZE);
    if (getcwd(dir, (sizeof(char) * SIZE)) != NULL) {
        printf ("%s\n", dir);
    } else {
        printf ("Error\n");
    }
    return 1;
}

int myshell_set(char **tokens) {
    char *token;
    char param[SIZE][SIZE];
    int index = 0;
    // char param2[SIZE];

    if (tokens[1] == NULL) {
        printf ("Need additional argument");
    } else {
        token = strtok(tokens[1], "=");
        while (token != NULL) {
            for (size_t i=0; i < strlen(token); i++) {
                param[index][i] = token[i];
            }
            index++;
            token = strtok(NULL, "=");
        }
        if (setenv(param[0], param[1], 1) != 0) {
            printf ("Error executing cd command\n");
        }
    }
    // token = getenv(param[0]); 
    // if (token != NULL) {
    //     printf ("token: %s\n", token);
    // }
    return 1;
}

int execute_command(char **tokens) {
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
            perror("Error");
        }
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

// int handle_out_redirection(char *filename, char **tokens) {
//     FILE *fp;
//     fp = freopen(filename, "w+", stdout);
//     if (fp == NULL) {
//         printf ("Error in opening file\n");
//     }
//     execute_command(tokens);
//     fclose(fp);
//     return 1;
// }

int execute_input(char **tokens) {
    int builtinCount = sizeof COMMANDS / sizeof *COMMANDS;
    int retVal = 1;
    int flag = 0;
    // int pipe_pos[SIZE];

    if (tokens[0] == NULL) {
        return 1;
    }
    // check for special character
    int io_char = parse_io_char(tokens);
    // parse_pipe_char(tokens, pipe_pos);
    // if (io_char == 0) {

        // switch(special_char) {
        //     case 2:
        //         retVal = handle_out_redirection(filename, tokens);
        //         return retVal;
        //         break;
        //     case 3:
        //         retVal = handle_in_redirection(filename, tokens);
        //         return retVal;
        //         break;
        //     case 4:
        //         break;
        // }
        // return 1;
    // }
    if (io_char == 0) {
        for  (int i = 0; i < builtinCount; i++) {
            if (strcmp(COMMANDS[i], tokens[0]) == 0) {
                flag = 1;
                switch(i) {
                    case 0:
                        retVal = myshell_exit(tokens);
                        return retVal;
                        break;
                    case 1:
                        retVal = myshell_cd(tokens);
                        return retVal;
                        break;
                    case 2:
                        retVal = myshell_pwd(tokens);
                        return retVal;
                        break;
                    case 3:
                        retVal = myshell_set(tokens);
                        return retVal;
                        break;
                };
            }
        }
    }
    if (flag == 0 && io_char == 0) {
        retVal = execute_command(tokens);
    }
    return retVal;
}

int main () {
    /* Define a variable to read user input. Assuming the input to be not greater than 80 characters */
    char input[SIZE];
    int status;
    do {
        char *tokens[SIZE];

        printf("$ ");

        read_input(input);
        
        parse_input(input, tokens);
        
        status = execute_input(tokens);

        memset(tokens, 0, sizeof tokens);

    } while (status);

    return 0;
}