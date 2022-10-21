#include "./utils/headers/auth.h"
#include "./utils/headers/input_acquisition.h"
#include "./utils/headers/cryptography.h"
#include "./utils/headers/sodiumplusplus.h"
#include "./utils/headers/array_handling.h"
#include "./utils/headers/clipboard_managment.h"

#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <pthread.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64
#define PSM_TOK_DELIM " \t\r\n\a"
#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define CHUNK_SIZE 512

#define ACCT_FILE_PATH "/usr/share/binaries/accounts.list"
#define PASS_FILE_PATH "/usr/share/binaries/passwords.list"

#define PASS_LENGTH 65

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

void crypto_alg_init(void);
void psm_start(void);
int psm_exec(void);
void exit_prog(void);

int psm_split_line(char *line, char **tokens, size_t tokens_size);
int psm_launch(char **args) ;
int psm_num_commands(void);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

int skey_acct = 0;
int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

int main(int argc, char const *argv[])
{
    if (argc > 1) {
        printf("this command does not accept arguments\n");
        return 1;
    }

    crypto_alg_init();

    psm_start();

    return 0;
}

// this function initializes sodium library (doc.libsodium.org)
void crypto_alg_init(void) {
    if (sodium_init() == -1 ) {
        perror("psm: error during cryptographic algorithm's initialization\n");
        perror("psm: exiting...");
        exit(EXIT_FAILURE);
    }
}

// this function starts the shell after authenticating the user
void psm_start(void) 
{
    int exit_code = 0;

    unsigned char *start_txt;

    start_txt = "\nWELCOME TO PASSMAN!\n"
                "\n"
                "    digit \"help\" for help\n"
                "    digit \"exit\" to exit passman\n";

    if (auth() == 0) {

        printf("%s\n", start_txt);

        exit_code = psm_exec();
    } else {
        exit(EXIT_FAILURE);
    }

    if (exit_code == 1) {
        exit_prog();
    }
}

// array of command names
char *command_names[] = {
    // bring here all commands' names (ex. "show")
};

// array of command functions
int (*command_addr[]) (char **) = {
    // bring here all commands' functions (ex. &psm_show)
};

// this function returns the number of commands in command_names array
int psm_num_commands(void) 
{
    return sizeof(command_names) / sizeof(char *);
}

// this function is the basically the shell itself: it prints a prompt on the screen,
// reads the user input, splits the input in tokens separated by spaces (command name + args) 
// and executes commands using psm_launch().
int psm_exec()
{
    size_t line_size = 1024;
    size_t args_size = PSM_TOK_BUFSIZE;

    char *line = (char *) malloc(line_size);
    char **args = (char **) malloc(args_size);

    int ret_code = -1;

    if (!line | !args) {
        perror("psm: allocation error");
        line ? sodium_free(line) : 0;
        args ? sodium_free(args) : 0;
        return -1;
    }

    while (1) 
    {
        printf("> ");                                   // printing the prompt.

        // reading the user input.
        if (read_line(&line, line_size) != 0) {
            goto ret;
        }

        // splitting the input in tokens (command name + arg1 + arg2 + ...).
        if (psm_split_line(line, args, args_size) != 0) {
            goto ret;
        }

        psm_launch(args);                               // launching the command passing its name along with its arguments.
    }

    ret_code = 0;

ret:
    free(line);
    free(args);
    return ret_code;
}

// launching the command passed as an array containing the command name and its arguments.
int psm_launch(char **args) 
{
    // checking for empty commands
    if (args[0] == NULL) {
        return -1;
    }

    // if the first argument of the array equals to the name of a stored command, 
    // the corresponding function is called and the arguments are passed along.
    for (int i = 0; i < psm_num_commands(); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_addr[i])(args);
        }
    }

    printf("command not found\n");
    return -1;
}

int psm_split_line(char *line, char **tokens, size_t tokens_size)
{
    int position = 0;
    char *token;

    token = strtok(line, PSM_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;
        if (position >= tokens_size) {
            tokens_size += PSM_TOK_BUFSIZE;
            tokens = (char **) realloc(tokens, tokens_size);
            if (!tokens) {
                perror("psm: allocation error\n");
                return -1;
            }
        }

        token = strtok(NULL, PSM_TOK_DELIM);
    }

    tokens[position] = NULL;
    return 0;
}

void exit_prog(void)
{
    char **empty_args;
    psm_exit(empty_args);
}