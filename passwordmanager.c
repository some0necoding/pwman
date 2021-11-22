#ifndef NULL
    #include <string.h>
#endif

#ifndef RAND_MAX
    #include <stdlib.h>
#endif

#ifndef TERMIOS_PLUS_PLUS
    #include <termios.h>
#endif

#ifndef STDIO_PLUS_PLUS
    #include <stdio.h>
#endif

#ifndef SOIDUM_PLUS_PLUS
    #include <sodium.h>
#endif

#ifndef AUTH
    #include "headers/auth.h"
#endif
    
#ifndef INPUT_ACQUISITION
    #include "headers/input_acquisition.h"
#endif

/*
    Pretty amazing password manager written in C

    1. Tasks

        Authentication

        - get the password
        - verify the password (login) or validate input (signin)
        - grant or not the access
        
        Execution

        - show passwords (by user)
        - add password (by user)
        - remove password (by user)
        - modify password (by user)
        - retrieve password (by user)
        - encrypting/decrypting passwords
        - saving passwords in the clipboard
        - clearing the clipboard after a defined amount of time
        - keeping all buffers clean

        Closure

        - clear everything (buffers, clipboard)

    2. Commands

        - show(char **args): it shows all accounts with hidden passwords (for security 
            purposes), because its task is to make the user see what are his accounts; 
            if the user wants to retrieve a password he will have to call the get() 
            function. As well as all other commands it accepts multiple args, but 
            those will be ignored because useless (it will run as a void function).

        - add(char **args): it adds a new account to files. It accepts account_name,
            user_or_mail and password as args.
        
        - remove(char **args): it removes an account from files. It accepts
            account_name (which is unique) as single arg.

        - modify(char **args): it modifyies user_or_mail or/and password of an account. 
            Accepted args are not yet decided 'cause this function is a bit messy. 
            Maybe it will consist in some -something -> -p for pass and -u for user.

        - get(char **args): it saves the specified account in the user's clipboard,
            so that the password never gets visible (inputs will have echo disabled).
            It accepts account_name as single arg.

    3. Storage method

        - account_name
        - user_or_mail
        - password 

        all of these may be put in two files, one for keeping accounts' and users'
        names, the other for keeping passwords (/etc/passwd and /etc/shadow style). 
        This because if the user calls show() with all data saved in the same file, 
        it will decrypt all account_name, user_or_name and password, putting the 
        latter at risk uselessly 'cause it will not be shown. 
*/

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64
#define PSM_TOK_DELIM " \t\r\n\a"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

void crypto_alg_init(void);
void psm_start(void);
int psm_exec(void);
void exit_prog(void);

int psm_show(char **args);
int psm_add(char ** args);
int psm_modify(char **args);
int psm_remove(char **args);
int psm_get(char **args);
int psm_exit(char **args);

char **psm_split_line(char *line);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------GLOBAL-VARIABLES-DEFINITION-START-------*/

char *masterkey;
unsigned char *key1;
unsigned char *key2;

/*-------GLOBAL-VARIABLES-DEFINITION-END--------*/

int main(int argc, char const *argv[])
{
    crypto_alg_init();

    psm_start();

    return EXIT_SUCCESS;
}

void crypto_alg_init(void) {
    if (sodium_init() == -1 ) {
        perror("psm: error during cryptographic algorithm's initialization\n");
        perror("psm: exiting...");
        exit(EXIT_FAILURE);
    }
}

void psm_start(void) 
{
    int exit_code = 0;

    if (auth() == 0) {
        exit_code = psm_exec();
    } else {
        exit(EXIT_FAILURE);
    }

    if (exit_code == 1) {
        exit_prog();
    }
}

char *command_names[] = {
    "show",
    "add",
    "modify",
    "remove",
    "get",
    "exit"
};

int (*command_addr[]) (char **) = {
    &psm_show,
    &psm_add,
    &psm_modify,
    &psm_remove,
    &psm_get,
    &psm_exit
};

int psm_num_commands() {
    return sizeof(command_names) / sizeof(char *);
}

int psm_show(char **args)
{
    printf("show\n");

    // time_line:
    // 1. decript the accounts
    //    file in an array
    // 2. list'em all with
    //    hidden passwords
}

int psm_add(char **args)
{
    printf("add\n");

    // time_line:
    // 1. get user input
    // 2. validate user input
    // 3. decrypt the accounts
    //    file
    // 4. add input to the file
    // 5. recrypt the file

    // vulnerabilities:
    // 1. to add a single
    //    account I have to
    //    decrypt the entire
    //    file, putting all
    //    other passes at risk
   
}

int psm_modify(char **args)
{
    printf("modify\n");

    // time_line:
    // 1. get user input
    // 2. validate input
    // 3. decrypt the accounts
    //    file
    // 4. if it exists, remove         
    //    the old account and 
    //    replace it with the 
    //    modified one
    // 5. recrypt the file

    // vulnerabilities:
    // 1. it's needed to decrypt
    //    the entire accounts
    //    file putting all other
    //    passes at risk
}

int psm_remove(char **args) 
{
    printf("remove\n");

    // time_line
    // 1. get user input
    // 2. validate user input
    // 3. decrypt accounts file
    // 4. if it exists, remove 
    //    the account
    // 5. recrypt the file

    // vulnerabilities
    // 1. it's needed to decrypt
    //    the entire accounts
    //    file putting all other
    //    passes at risk
} 

int psm_get(char **args)
{
    printf("get\n");

    // 1. get user input
    // 2. validate used input
    // 3. decrypt accounts file
    // 4. if it exists, copy the
    //    pass in the clipboard
    // 5. recrypt the file
    // 6. after a while (1 min
    //    or less) clear the 
    //    the clipboard

    // vulnerabilities:
    // 1. it's needed to decrypt
    //    the entire accounts
    //    file putting all other
    //    passes at risk
}

int psm_exit(char **args) {
    exit(EXIT_SUCCESS);
}

int psm_launch(char **args) 
{
    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (int i = 0; i < psm_num_commands(); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_addr[i])(args);
        }
    }

    printf("command not found\n");
    return 1;
}

int psm_exec()
{
    char *line;
    char **args;



    while (1) 
    {
        printf("> ");

        line = read_line();
        args = psm_split_line(line);

        psm_launch(args);
    }

    free(line);
    free(args);

    return 1;
}

char **psm_split_line(char *line)
{
    int bufsize = PSM_TOK_BUFSIZE;
    int position = 0;
    char **tokens = (char **) malloc(sizeof(char) * bufsize);
    char *token;

    if (!tokens) {
        perror("psm: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, PSM_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;
        if (position >= bufsize) {
            bufsize += PSM_TOK_BUFSIZE;
            tokens = (char **) realloc(tokens, bufsize);
            if (!tokens) {
                perror("psm: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, PSM_TOK_DELIM);
    }

    tokens[position] = NULL;

    return tokens;
}


void exit_prog(void)
{
    char **empty_args;
    psm_exit(empty_args);
}