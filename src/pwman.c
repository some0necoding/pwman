#include "./utils/input.h"
#include "./utils/fio.h"
#include "./utils/config.h"
#include "./utils/init.h"

#include "./commands/psm_show.h"
#include "./commands/psm_add.h"
#include "./commands/psm_rm.h"
#include "./commands/psm_get.h"
#include "./commands/psm_help.h"
#include "./commands/psm_exit.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#define LINE_SIZE 1024
#define BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"
#define WELCOME_MESSAGE "WELCOME TO PWMAN!\n" \
                        "\tdigit \"help\" for help\n" \
                        "\tdigit \"exit\" to exit pwman"

void welcome_message();
void start(void);
int loop(void);
int execute(char **args) ;
int split_line(char *line, char **tokens, size_t tokens_size);
int num_commands(void);


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


int main(int argc, char const *argv[])
{
    const char *config_file = get_config_path();

	int ret_code = -1;

    if (!config_file) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret; 
	}

    if (access(config_file, F_OK) != 0 && init() != 0) {
		fprintf(stderr, "psm:%s:%d: error while pwman setup\n", __FILE__, __LINE__);
		goto ret;	
    }

    start();		// starting the "shell"

	ret_code = 0;

ret:
    if (config_file) free((char *) config_file);
    return ret_code;
}

/*
    This function starts the shell
*/
void start(void) 
{
	printf("%s\n", WELCOME_MESSAGE);

    /* Starting shell loop */
    if (loop() != 0) {
        exit(EXIT_FAILURE);
    }
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
    char *line = NULL;
    char **args = NULL;

    int ret_code = -1;
    
	line = (char *) malloc(LINE_SIZE);
    args = (char **) malloc(BUFSIZE);

    if (!line | !args) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    while (1) 
    {
        /* Print the prompt */
        printf("> ");

        /* Read the user input */
        if (!(line = read_line())) {
            fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
            goto ret;
        }

        /* Split the input in tokens (command name + arg1 + arg2 + ...) */
        if (split_line(line, args, BUFSIZE) != 0) {
            fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
            goto ret;
        }

        /* Launch the command */
        if (execute(args) != 0) {
            goto ret;
        }
    }

    ret_code = 0;

ret:
    if (line) free(line);
    if (args) free(args);
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
                fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
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
