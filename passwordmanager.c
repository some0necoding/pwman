#include "./headers/auth.h"
#include "./headers/input_acquisition.h"
#include "./headers/cryptography.h"

#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <sodium.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64
#define PSM_TOK_DELIM " \t\r\n\a"
#define SEPARATE_LINE_CHAR 0xD8        // this byte separates lines in the file
#define SEPARATE_KEYS_CHAR 0xF0        // this byte separates account_name and mail_or_user in a line

#define PASS_LENGTH 65

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

/*------------GLOBAL-VARIABLES-START------------*/

int skey_one = 0;
int skey_two = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

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
    "rm",
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
    char *file_path = "./config_files/accounts.list";
    unsigned char *file_content;

    if (args[1]) {
        printf("\"show\" does not accept arguments\n");
        return -1;
    }

    // the file remains encrypted, while the decrypted content gets stored in a buffer (i.e. file_content).
    if (!(file_content = decrypt_file(file_path, subkeys[skey_one]))) {
        perror("psm: cryptography error");
        return -1;
    }

    printf("%s\n", file_content);

    // 0xD8 216 11011000 -> to separate lines
    // 0xF0 240 11110000 -> to separate account and mail
    // accounts file pattern -> account_name_1\0xF0mail_or_username_1\0xD8account_name_2\0xF0mail_or_username_2...

    // here I need to split the file content in lines and then to separat'em in two tokens: account and mail. Then I print 'em.
}

int psm_add(char **args)
{
    // add accepts only 2 args, so if the 2nd arg doesn't exist or if the 3rd arg do exist, the program throws an error.
    if (!args[2] || args[3]) {
        printf("\"add\" needs to know an account name and the mail used to login (accepts only two arguments)\n");
    }

    // FIX: declaration here
    size_t acct_size = strlen(args[1]) + 1;
    size_t user_size = strlen(args[2]) + 1;
    size_t pass_size = PASS_LENGTH;

    unsigned char *acct_name = (unsigned char *) sodium_malloc(acct_size);
    unsigned char *user_mail = (unsigned char *) sodium_malloc(user_size);
    unsigned char *pass_word = (unsigned char *) sodium_malloc(pass_size);

    acct_name = args[1];
    user_mail = args[2];

    printf("choose a password for this account: ");

    if (!(pass_word = read_line_s())) {
        perror("psm: I/O error");
        return -1;
    }

    append_account(acct_name, user_mail);
    append_pass(pass_word);

    return 0;
}

int psm_modify(char **args)
{
    printf("modify\n");

    // time_line:
    // 1. validate input
    // 2. decrypt the accounts
    //    file
    // 3. if it exists, remove         
    //    the old account and 
    //    replace it with the 
    //    modified one
    // 4. recrypt the file
}

int psm_remove(char **args) 
{
    printf("remove\n");

    // time_line
    // 1. validate user input
    // 2. decrypt accounts file
    // 3. if it exists, remove 
    //    the account
    // 4. recrypt the file
} 

int psm_get(char **args)
{
    printf("get\n");

    // 1. validate used input
    // 2. decrypt accounts file
    // 3. if it exists, copy the
    //    pass in the clipboard
    // 4. recrypt the file
    // 5. after a while (1 min
    //    or less) clear the 
    //    the clipboard
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