#include "./utils/headers/input.h"
#include "./utils/headers/fio.h"
#include "./utils/headers/config.h"

#include "./commands/headers/psm_show.h"
#include "./commands/headers/psm_add.h"
#include "./commands/headers/psm_rm.h"
#include "./commands/headers/psm_get.h"
#include "./commands/headers/psm_help.h"
#include "./commands/headers/psm_exit.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

void welcome_message();
void start(void);
int loop(void);
int execute(char **args) ;
int split_line(char *line, char **tokens, size_t tokens_size);
int num_commands(void);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

// array of command names
char *command_names[] = {
    "show",
    "add",
    "rm",
    "get",
    "help",
    "exit"
};

// array of command functions
int (*command_addr[]) (char **) = {
    &psm_show,
    &psm_add,
    &psm_rm,
    &psm_get,
    &psm_help,
    &psm_exit
};

/*-------------GLOBAL-VARIABLES-END-------------*/

int main(int argc, char const *argv[])
{
    char *config_file = get_config_path();

    if (!config_file) {
        perror("psm: allocation error");
        return -1;
    }

    if (access(config_file, F_OK) != 0) {
        printf("You have to run \"pwman-init\" before\n");
        free(config_file);
        return 0;
    }

    /* starting the "shell" */
    start();

    free(config_file);
    return 0;
}

/*
    This function starts the shell
*/
void start(void) 
{
    welcome_message();
 
    /* Starting shell loop */
    if (loop() != 0) {
        exit(EXIT_FAILURE);
    }
}

/*
    This function prints a welcome message
    at start.
*/
void welcome_message() 
{
    char *start_txt = "WELCOME TO PWMAN!\n"
                        "\tdigit \"help\" for help\n"
                        "\tdigit \"exit\" to exit pwman";

    printf("%s\n", start_txt);
}

/*
    This function is basically the shell itself: 
    it prints a prompt on the screen, reads the user
    input, splits the input in tokens separated by
    spaces (command name + args) and executes commands
    using execute() function.
*/
int loop()
{
    size_t line_size = 1024;
    size_t args_size = BUFSIZE;

    char *line = (char *) malloc(line_size);
    char **args = (char **) malloc(args_size);

    int ret_code = -1;

    if (!line | !args) {
        perror("psm: allocation error");
        goto ret;
    }

    while (1) 
    {
        /* Print the prompt */
        printf("> ");

        /* Read the user input */
        if (read_line(&line, line_size) != 0) {
            perror("psm: allocation error");
            goto ret;
        }

        /* Split the input in tokens (command name + arg1 + arg2 + ...) */
        if (split_line(line, args, args_size) != 0) {
            perror("psm: allocation error");
            goto ret;
        }

        /* Launch the command */
        if (execute(args) != 0) {
            goto ret;
        }
    }

    ret_code = 0;

ret:
    line ? free(line) : 0;
    args ? free(args) : 0;
    return ret_code;
}

/*
    This function launchs the command passed as
    an array containing command name and its args.
*/
int execute(char **args) 
{
    /* check for empty commands */
    if (args[0] == NULL) {
        return 0;
    }

    /* Call the function associated with command name */
    for (int i = 0; i < num_commands(); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_addr[i])(args);
        }
    }

    printf("command not found\n");
    return 0;
}

/*
    This funciton splits a line around spaces
    and stores tokens into statically allocated
    tokens buffer.
*/
int split_line(char *line, char **tokens, size_t tokens_size)
{
    int pos = 0;
    char *token;

    token = strtok(line, TOKEN_DELIM);

    while (token) {

        tokens[pos] = token;
        pos++;
        
        if (pos >= tokens_size) {
            
            tokens_size += BUFSIZE;
            tokens = realloc(tokens, sizeof(char *) * tokens_size);
            
            if (!tokens) {
                perror("psm: allocation error\n");
                return -1;
            }
        }

        token = strtok(NULL, TOKEN_DELIM);
    }

    /* NULL terminate array */
    tokens[pos] = NULL;
    return 0;
}

/*
    This function returns the number of commands
    in command_names array.
*/
int num_commands(void) 
{
    return sizeof(command_names) / sizeof(char *);
}   