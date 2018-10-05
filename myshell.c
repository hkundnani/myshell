#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

int myshell_cd(char **tokens) {
    if (tokens[1] == NULL) {
        printf ("Expects additional argument after cd\n");
    } else {
        if (chdir(tokens[1]) != 0) {
            printf ("Error executing cd command\n");
        }
    }
    return 1; 
}

int execute_command(char **tokens) {
    int status;
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(tokens[0], tokens) == -1) {
            printf ("Error in executing command\n");
        }
    } else if (pid == -1) {
        printf ("Error at forking\n");
    } 
    else {
        waitpid(pid, &status, 0);
    }
    return 1;
}

int execute_input(char **tokens) {
    int builtinCount = sizeof COMMANDS / sizeof *COMMANDS;
    int retVal = 1;
    int flag = 1;

    if (tokens[0] == NULL) {
        return 1;
    }
    for  (int i = 0; i < builtinCount; i++) {
        if (strcmp(COMMANDS[i], tokens[0]) == 0) {
            flag = 0;
            switch(i) {
                case 0:
                    retVal = 0;
                    break;
                case 1:
                    retVal = myshell_cd(tokens);
                    execute_command(tokens);
                    break;
                case 2:
                    retVal = execute_command(tokens);
                    break;
                case 3:
                    break;
            };
        }
    }

    if (flag) {
        printf ("The command is not supported\n");
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