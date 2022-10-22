#include "./utils/headers/input_acquisition.h"
#include "./utils/headers/stdioplusplus.h"

#include "./commands/headers/psm_show.h"
#include "./commands/headers/psm_add.h"
#include "./commands/headers/psm_rm.h"
#include "./commands/headers/psm_get.h"
#include "./commands/headers/psm_help.h"
#include "./commands/headers/psm_exit.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"

#define CONFIGS "/etc/pwman.conf"


// these should be defined and exported (using a getter) in auth.c
#define SKEY_ACCT 0
#define SKEY_PASS 1

const char *HOME;
const char *PATH;

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int setup();

void print_welcome_message();
void start(void);
int loop(void);
int execute(char **args) ;

int split_line(char *line, char **tokens, size_t tokens_size);
int num_commands(void);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

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
    &psm_remove,
    &psm_get,
    &psm_help,
    &psm_exit
};

/*-------------GLOBAL-VARIABLES-END-------------*/

int main(int argc, char const *argv[])
{
    // checking for bad command line args at shell run
    if (argc > 1) {
        printf("this command does not accept arguments\n");
        return 1;
    }

    // setting up the environment
    setup();

    // starting the "shell"
    start();

    return 0;
}

int setup() 
{
    if (access(CONFIGS, F_OK) != 0) {
        PATH = make_path();

        if (add_path(PATH) != 0) {
            perror("psm: I/O error");
            return -1;
        }

    } else if (!(PATH = get_path())) {
        perror("psm: I/O error");
        return -1;
    }
}

// this function builds the path of .pwstore
// which is /home/user/.pwstore
char *make_path() 
{
    char *home;

    // get user's home directory
    if (!(home = getenv("HOME"))) {
        home = getpwuid(getuid())->pw_dir;
    }

    // return pwstore path
    return strcat(home, "/.pwstore");
}

// this function starts the shell
void start(void) 
{
    print_welcome_message();
 
    // starting the shell loop
    if (loop() != 0) {
        exit(EXIT_FAILURE);
    }
}

void print_welcome_message() 
{
    unsigned char *start_txt = "\nWELCOME TO PASSMAN!\n"
                               "\n"
                               "    digit \"help\" for help\n"
                               "    digit \"exit\" to exit passman\n";

    printf("%s\n", start_txt);
}

// this function is basically the shell itself: it prints a prompt on the screen, reads 
// the user input, splits the input in tokens separated by spaces (command name + args) 
// and executes commands using execute() function.
int loop()
{
    size_t line_size = 1024;
    size_t args_size = BUFSIZE;

    char *line = (char *) malloc(line_size);
    char **args = (char **) malloc(args_size);

    int ret_code = -1;

    if (!line | !args) {
        perror("psm: allocation error");
        line ? free(line) : 0;
        args ? free(args) : 0;
        return ret_code;
    }

    while (1) 
    {
        // printing the prompt.
        printf("> ");

        // reading the user input.
        if (read_line(&line, line_size) != 0) {
            goto ret;
        }

        // splitting the input in tokens (command name + arg1 + arg2 + ...).
        if (split_line(line, args, args_size) != 0) {
            goto ret;
        }

        // launching the command passing its name along with its arguments.
        execute(args);
    }

    ret_code = 0;

ret:
    free(line);
    free(args);
    return ret_code;
}

// launching the command passed as an array containing the command name and its arguments.
int execute(char **args) 
{
    // checking for empty commands
    if (args[0] == NULL) {
        return -1;
    }

    // if the first argument of the array equals to the name of a stored command, 
    // the corresponding function is called and the arguments are passed along.
    for (int i = 0; i < num_commands(); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_addr[i])(args);
        }
    }

    printf("command not found\n");
    return -1;
}

int split_line(char *line, char **tokens, size_t tokens_size)
{
    int pos = 0;
    char *token;

    token = strtok(line, TOKEN_DELIM);

    while (token != NULL) {
        tokens[pos] = token;
        pos++;
        if (pos >= tokens_size) {
            tokens_size += BUFSIZE;
            tokens = (char **) realloc(tokens, tokens_size);
            if (!tokens) {
                perror("psm: allocation error\n");
                return -1;
            }
        }

        token = strtok(NULL, TOKEN_DELIM);
    }

    tokens[pos] = NULL;
    return 0;
}

// this function returns the number of commands in command_names array
int num_commands(void) 
{
    return sizeof(command_names) / sizeof(char *);
}

/* // this function initializes sodium library (doc.libsodium.org)
void crypto_alg_init(void) {
    if (sodium_init() == -1 ) {
        perror("psm: error during cryptographic algorithm's initialization\n");
        perror("psm: exiting...");
        exit(EXIT_FAILURE);
    }
} */