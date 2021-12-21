#include "./headers/auth.h"
#include "./headers/input_acquisition.h"
#include "./headers/cryptography.h"
#include "./headers/sodiumplusplus.h"

#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64
#define PSM_TOK_DELIM " \t\r\n\a"
#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define ACCT_FILE_PATH "./config_files/accounts.list"
#define PASS_FILE_PATH "./config_files/passwords.list"

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
int psm_launch(char **args) ;
int psm_num_commands(void);

int append_account(unsigned char *acct_name, unsigned char *user_or_mail);
int append_pass(unsigned char *pass);
unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);

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

int psm_num_commands(void) 
{
    return sizeof(command_names) / sizeof(char *);
}

int psm_show(char **args)
{
    char *file_path = ACCT_FILE_PATH;
    
    unsigned char *file_content;
    unsigned char **content_lines;
    unsigned char *content_line;
    unsigned char **tokens;
    unsigned char *acct_name;
    unsigned char *user_mail;

    unsigned char *sprt_line_str = SEPARATE_LINE_STR;
    unsigned char *sprt_tkns_str = SEPARATE_TKNS_STR;

    int pos = 0;

    if (args[1]) {
        printf("\"show\" does not accept arguments\n");
        return -1;
    }

    // the file remains encrypted, while the decrypted content gets stored in a buffer (i.e. file_content).
    if (!(file_content = decrypt_file(file_path, subkeys[skey_one]))) {
        perror("psm: cryptography error");
        return -1;
    }  

    if (!(content_lines = split_by_delim(file_content, sprt_line_str))) {
        return -1;
    }

    while ((content_line = content_lines[pos]) != NULL) {

        if (!(tokens = split_by_delim(content_line, sprt_tkns_str))) {
            return -1;
        }

        acct_name = tokens[0];
        user_mail = tokens[1];

        printf("%1$d.\n\taccount: %2$s\n\tuser: %3$s\n\tpassword: hidden\n", pos+1, acct_name, user_mail);

        pos++;
    }

    return 0;
}

int psm_add(char **args)
{
    int ret_code = -1;

    unsigned char *acct_name;
    unsigned char *user_mail;
    unsigned char *pass_word;

    // add accepts only 2 args, so if the 2nd arg doesn't exist or if the 3rd arg do exist, the program throws an error.
    if (!args[2] || args[3]) {
        printf("\"add\" needs to know an account name and the mail used to login (accepts only two arguments)\n");
        return -1;
    }

    acct_name = args[1];
    user_mail = args[2];

    printf("choose a password for this account: ");

    if (!(pass_word = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }

    printf("\n");

    append_account(acct_name, user_mail);
    append_pass(pass_word);

    ret_code = 0;

    printf("account successfully stored!\n");

ret:
    //sodium_free(acct_name); need to change the allocation of args to sodium_malloc
    //sodium_free(user_mail); need to change the allocation of args to sodium_malloc
    sodium_free(pass_word);
    return ret_code;
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

// here's some additional functions needed to run commands

// this function appends a new account (account_name + user_or_mail) to accounts.list file.
// At first it decrypts the file into a buffer, then it appends to this buffer account_name 
// and user_or_mail separating them with 0xF0 byte and ending the line with 0xD8 byte. At
// the end it encrypts the modifyied buffer into accounts.list.
int append_account(unsigned char *acct_name, unsigned char *user_or_mail)
{
    size_t file_cont_size;                              // size of the old buffer
    size_t acct_name_size = strlen(acct_name);          // size of account_name
    size_t user_mail_size = strlen(user_or_mail);       // size of user_or_mail
    size_t totl_buff_size;                              // size of the new buffer

    char *sprt_tkns_char = SEPARATE_TKNS_STR;           // 0xF0 byte stored as a string so that it can be used with string.h functions
    char *sprt_line_char = SEPARATE_LINE_STR;           // 0xD8 byte stored as a string so that it can be used with string.h functions
    char *file_path = ACCT_FILE_PATH;                   // accounts.list file path

    unsigned char *file_content;
    unsigned char *totl_content;

    // decrypting the accounts.list file into a buffer
    if (!(file_content = decrypt_file(file_path, subkeys[skey_one]))) {
        perror("psm: cryptography error");
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    file_cont_size = strlen(file_content);
    totl_buff_size = file_cont_size + acct_name_size + 1 + user_mail_size + 1 + 1;      // original buffer + account_name + 0xF0 + user_or_mail + 0xD8 + \0

    totl_content = (unsigned char *) sodium_malloc(totl_buff_size);                     // initialising the new buffer

    strncpy(totl_content, file_content, file_cont_size + 1);                            // copying the old buffer into the new bigger buffer
    strncat(totl_content, acct_name, acct_name_size);                                   // appending account_name to the new buffer
    strncat(totl_content, sprt_tkns_char, 1);                                           // appending 0xF0 (separation byte) after account_name
    strncat(totl_content, user_or_mail, user_mail_size);                                // appending user_or_mail to the new buffer
    strncat(totl_content, sprt_line_char, 1);                                           // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the accounts.list file
    if (encrypt_buffer(totl_content, subkeys[skey_one], file_path) != 0) {
        perror("psm: cryptography error");
        sodium_free(totl_content);
        return -1;
    }

    sodium_free(totl_content);

    return 0;
}

// this function appends a new password to passwords.list file. At first it decrypts 
// the file into a buffer, then it appends to this buffer password ending the line 
// with 0xD8 byte. At the end it encrypts the modifyied buffer into passwords.list.
int append_pass(unsigned char *pass)
{
    size_t file_cont_size;                              // size of the old buffer
    size_t pass_word_size = strlen(pass);               // size of pass
    size_t totl_buff_size;                              // size of the new buffer

    char *sprt_line_char = SEPARATE_LINE_STR;           // 0xD8 byte stored as a string so that it can be used with string.h functions
    char *file_path = PASS_FILE_PATH;                   // passwords.list file path

    unsigned char *file_content;
    unsigned char *totl_content;

    // decrypting the passwords.list file into a buffer
    if (!(file_content = decrypt_file(file_path, subkeys[skey_two]))) {
        perror("psm: cryptography error");
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    file_cont_size = strlen(file_content);
    totl_buff_size = file_cont_size + pass_word_size + 1 + 1;                           // original buffer + password + 0xD8 + \0

    totl_content = (unsigned char *) sodium_malloc(totl_buff_size);                     // initialising the new buffer

    strncpy(totl_content, file_content, file_cont_size + 1);                            // copying the old buffer into the new bigger buffer
    strncat(totl_content, pass, pass_word_size);                                        // appending pass to the new buffer
    strncat(totl_content, sprt_line_char, 1);                                           // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the passwords.list file
    if (encrypt_buffer(totl_content, subkeys[skey_two], file_path) != 0) {
        perror("psm: cryptography error");
        sodium_free(totl_content);        
        return -1;
    }

    sodium_free(totl_content);

    return 0;
}

// this function splits a generic string in tokens using a delimiter string as reference.
// all tokens are returned in an array.
unsigned char **split_by_delim(unsigned char *str, unsigned char *delim)
{
    size_t bufsize = PSM_TOK_BUFSIZE;
    size_t old_size;
    
    unsigned char **tokens = (unsigned char **) sodium_malloc(sizeof(unsigned char) * bufsize);
    unsigned char *token;
    
    int pos = 0;

    if (!tokens) {
        perror("psm: allocation error\n");
        return NULL;
    }   

    token = strtok(str, delim);

    while (token != NULL) {
        tokens[pos] = token;
        pos++;
        if (pos >= bufsize) {
            old_size = bufsize;
            bufsize += PSM_TOK_BUFSIZE;
            tokens = (unsigned char **) sodium_realloc(tokens, old_size, bufsize);
            if (!tokens) {
                perror("psm: allocation error\n");
                return NULL;
            }
        }

        token = strtok(NULL, delim);
    }

    tokens[pos] = NULL;

    return tokens;
}