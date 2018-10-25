#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

const int SIZE = 80;
const char *COMMANDS[] = {"exit", "cd", "pwd", "set"};
const char *PIPE = "|";
const char *SPACE = " \t\n\r";

int bg = 0;
int myls = 0;


// Function to read the input from the stdin
void read_input(char *input) {
    fgets(input, 80, stdin);
}

// Function to set the path for myls
void set_path() {
    char *dir = (char*)malloc(sizeof(char) * SIZE);
    char *full_path = NULL;
    
    /* Get the current directory path, concat myls executable to it and set it as MYPATH */
    if (getcwd(dir, (sizeof(char) * SIZE)) != NULL) {
        full_path = strcat(dir, "/myls");
        if (setenv("MYPATH", full_path, 1) != 0) {
            perror("Error");
        }
    } else {
        perror("Error");
    }
}

// Function to tokenize the input received from stdin
void parse_input (char *input, char **tokens, const char *delim) {
    char *token;
    int index = 0;
    
    token = strtok(input, delim);
    
    while (token != NULL) {
        // Set the background process flag if & is present
        if (strcmp(token, "&") == 0) {
            bg = 1;
            token = NULL;
        // Set the myls flag if myls is present to set the path    
        } else if (strcmp(token, "myls") == 0) {
            myls = 1;
            set_path();            
        }
        tokens[index] = token;
        index++;
        token = strtok(NULL, delim);
    }
    tokens[index] = NULL;
}

// Function to handle input redirection
int handle_in_redirection(char *filename, char **tokens) {
    int status;
    FILE *fp;
    pid_t pid = fork();

    if (pid == 0) {
        fp = freopen(filename, "r", stdin);
        if (fp == NULL) {
            printf ("Error");
        } else {
            if (execvp(tokens[0], tokens) == -1) {
                perror("Error");
            }
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

// Function to handle output redirection
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

// Function to handle both input and output redirection
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

/* Function to check if input/output redirection is present and call the respective functions to handle input/output redirection */
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

// Function to change the directory
int myshell_cd(char **tokens) {
    
    /* If no directory name is mentioned then change the directory to home directory */ 
    if (tokens[1] == NULL) {
        char *token = getenv("HOME");
        if (chdir(token) != 0) {
            perror ("Error");
        }
    } else {
        if (chdir(tokens[1]) != 0) {
            perror ("Error");
        }
    }
    return 1;
}

// Function to exit myshell
int myshell_exit(char **tokens) {
    return 0;
}

// Function to show the path of current working directory
int myshell_pwd(char **tokens) {
    char *dir = (char*)malloc(sizeof(char) * SIZE);
    
    if (getcwd(dir, (sizeof(char) * SIZE)) != NULL) {
        printf ("%s\n", dir);
    } else {
        perror ("Error");
    }
    free (dir);
    return 1;
}

// Function to set the environment variable
int myshell_set(char **tokens) {
    char *token;
    char param[SIZE][SIZE];
    int index = 0;

    // Check if variable is passed along with set
    if (tokens[1] == NULL) {
        printf ("Error: Need additional arguments\n");
    } else {
        /* Tokenize the environment variable into variable and value and use setenv to set the value */
        token = strtok(tokens[1], "=");
        while (token != NULL) {
            for (size_t i=0; i < strlen(token); i++) {
                param[index][i] = token[i];
            }
            index++;
            token = strtok(NULL, "=");
        }
        if (setenv(param[0], param[1], 1) != 0) {
            perror ("Error");
        }
    }
    return 1;
}

// Function to execute the external commands
int execute_external(char **tokens) {
    int status;
    pid_t pid = fork();
    
    if (pid == 0) {
        if (strcmp(tokens[0], "myls") == 0) {
            if (execvp(getenv("MYPATH"), tokens) == -1) {
                perror("Error");
            }    
        } else if (execvp(tokens[0], tokens) == -1) {
            perror("Error");
        }
        exit(EXIT_FAILURE);
    } else if (pid == -1) {
        perror ("Error");
    } 
    else {
        if (bg == 0) {
            waitpid(pid, &status, 0);
        } else {
            bg = 0;
        }
    }
    return 1;
}

/* Function to check if the command is a built-in command and call the respective functions */
int execute_builtIn(char **tokens) {
    int builtinCount = sizeof COMMANDS / sizeof *COMMANDS;

    for  (int i = 0; i < builtinCount; i++) {
        if (strcmp(COMMANDS[i], tokens[0]) == 0) {
            switch(i) {
                case 0:
                    return myshell_exit(tokens);
                    break;
                case 1:
                    return myshell_cd(tokens);
                    break;
                case 2:
                    return myshell_pwd(tokens);
                    break;
                case 3:
                    return myshell_set(tokens);
                    break;
            };
        }
    }
    return 0;
}

int parse_pipe_char(char *input, char **tokens) {
    char *token[SIZE];
    char *cmds[SIZE];
    int flag = 0;
    int count = 0;
    pid_t pid;
    
    for (int i = 0; tokens[i] != NULL; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            flag = 1;
            count++;
        }
    }
    if (flag == 1) {
        int pipefd[2];
        int fd = 0;
        int status;
        parse_input(input, cmds, PIPE);

        for (int i = 0; cmds[i] != NULL; i++) {
            parse_input(cmds[i], token, SPACE);

            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            
            pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if(pid == 0) {
                dup2(fd, 0);
                if (cmds[i+1] != NULL) {
                    dup2(pipefd[1], 1);
                }
                close(pipefd[0]);
                if (execvp(token[0], token) == -1) {
                    perror("Error");
                }
                exit(EXIT_FAILURE);
            } else {
                waitpid(pid, &status, 0);
                close(pipefd[1]);
                fd = pipefd[0];
            }
        }
    }
    
    // int flag = 0;
    // char *cmds[SIZE]; 
    // char *token[SIZE];

    // for (int i = 0; tokens[i] != NULL; i++) {
    //     if (strcmp(tokens[i], "|") == 0) {
    //         flag = 1;
    //         break;
    //     }
    // }
    // if (flag == 1) {
    //     parse_input(input, cmds, PIPE);
    //     for (int i = 0; cmds[i] != NULL; i++) {
    //         parse_input(cmds[i], token, SPACE);
    //         if (execute_builtIn(token) == 0 && (strcmp(token[0], "exit") != 0)) {
    //             execute_external(token);
    //         } else if ((strcmp(token[0], "exit") == 0)) {
    //             flag = 0;
    //             break;
    //         }
    //         memset(token, 0, sizeof token);
    //     }
    //     memset(cmds, 0, sizeof cmds);
    // }
    return flag;
}

int execute_input(char *input, char **tokens) {
    
    int retVal = 1;

    // check for special character
    int io_char = parse_io_char(tokens);
    int pipe_char = parse_pipe_char(input, tokens);

    if (io_char == 0 && pipe_char == 0) {
        retVal = execute_builtIn(tokens);
        if (retVal == 0 && (strcmp(tokens[0], "exit") != 0)) {
            retVal = execute_external(tokens);    
        } 
    }
    return retVal;
}

int main (int argc, char **argv) {
    /* Define a variable to read user input. Assuming the input to be not greater than 80 characters */
    char input[SIZE];
    char inputClone[SIZE];
    int status;

    do {
        char *tokens[SIZE];

        printf("$ ");

        read_input(input);

        strcpy(inputClone, input);
        
        parse_input(input, tokens, SPACE);
        
        if (tokens[0] == NULL) {
            status = 1;
        } else {
            status = execute_input(inputClone, tokens);
        }

        memset(tokens, 0, sizeof tokens);

    } while (status && !feof(stdin));

    return 0;
}