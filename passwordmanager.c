#include "./headers/auth.h"
#include "./headers/input_acquisition.h"
#include "./headers/cryptography.h"
#include "./headers/sodiumplusplus.h"
#include "./headers/array_handling.h"
#include "./headers/clipboard_managment.h"

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

#define ACCT_FILE_PATH "./config_files/accounts.list"
#define PASS_FILE_PATH "./config_files/passwords.list"
#define LOGS_FILE_PATH "./config_files/log.txt"

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
int test_show_password(char **args);
int test_show_account(char **args);

char **psm_split_line(char *line);
int psm_launch(char **args) ;
int psm_num_commands(void);

int append_account(unsigned char *acct_name, unsigned char *user_or_mail);
int append_pass(unsigned char *pass);
unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);
int remove_account(unsigned char *account_name, int *line_indx);
int find_line_indx(unsigned char **lines, unsigned char *str_to_match, size_t *line_len);
int remove_password(int line_indx);
unsigned char *rebuild_buff_from_lines(unsigned char **lines, size_t buff_len, int line_to_rm_indx);
unsigned char *get_pass(int line_indx);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

int skey_acct = 0;
int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

int main(int argc, char const *argv[])
{
    crypto_alg_init();

    psm_start();

    return EXIT_SUCCESS;
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

    if (auth() == 0) {
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
    "show",
    "add",
    "edit",
    "rm",
    "get",
    "exit",
    "pass",
    "acct"
};

// array of command functions
int (*command_addr[]) (char **) = {
    &psm_show,
    &psm_add,
    &psm_modify,
    &psm_remove,
    &psm_get,
    &psm_exit,
    &test_show_password,
    &test_show_account
};

// this function returns the number of commands in command_names array
int psm_num_commands(void) 
{
    return sizeof(command_names) / sizeof(char *);
}

// this function shows for every account th account name, the mail and the password
// marked as hidden. The content processing follows the storage pattern.
// - Arguments: it does not accept arguments.
int psm_show(char **args)
{
    char *file_path = ACCT_FILE_PATH;                           // accounts.list file path.
    
    unsigned char *file_content;                                // decrypted content of accounts.list file as a single string.
    unsigned char **content_lines;                              // array containing the file content splitted in lines. Every line contains an account.
    unsigned char *content_line;                                // a single line of the file content (i.e. a single account).
    unsigned char **tokens;                                     // array containing the account name and the mail of the account, stored in a single line.
    unsigned char *acct_name;                                   // account name.
    unsigned char *user_mail;                                   // username or mail.

    int pos = 0;

    // some kinda input verification.
    if (args[1]) {
        printf("\"show\" does not accept arguments\n");
        return -1;
    }

    // the file remains encrypted, while the decrypted content gets stored in a buffer (i.e. file_content).
    if (!(file_content = decrypt_file(file_path, subkeys[skey_acct]))) {
        perror("psm: cryptography error");
        return -1;
    }  

    // the file content gets splitted in lines and stored in content_lines, that is NULL terminated.
    if (!(content_lines = split_by_delim(file_content, SEPARATE_LINE_STR))) {
        return -1;
    }

    // looping through the lines to print info about every account.
    while ((content_line = content_lines[pos]) != NULL) {

        // the line gets splitted in tokens (account name and user/mail).
        if (!(tokens = split_by_delim(content_line, SEPARATE_TKNS_STR))) {
            return -1;
        }

        acct_name = tokens[0];
        user_mail = tokens[1];

        printf("%1$d.\n\taccount: %2$s\n\tuser: %3$s\n\tpassword: hidden\n", pos+1, acct_name, user_mail);

        pos++;
    }

    return 0;
}

// this function adds a new account to the "database". Account name and user/mail are appended
// to accounts.list file, while the password is appended to passwords.list file. The storage
// process follows the storage pattern.
// - Arguments: account_name, user_or_mail.
int psm_add(char **args)
{
    int ret_code = -1;

    size_t acct_name_len;
    size_t user_mail_len;

    unsigned char *acct_name;
    unsigned char *user_mail;
    unsigned char *pass_word;

    // add accepts only 2 args, so if the 2nd arg doesn't exist or if the 3rd arg do exist, the program throws an error.
    if (!args[2] || args[3]) {
        printf("\"add\" needs to know an account name and the mail used to login (2 arguments needed)\n");
        return -1;
    }

    acct_name_len = strlen(args[1]);
    user_mail_len = strlen(args[2]);

    acct_name = (unsigned char *) sodium_malloc(acct_name_len + 1);
    user_mail = (unsigned char *) sodium_malloc(user_mail_len + 1);

    strcpy(acct_name, args[1]);
    strcpy(user_mail, args[2]);

    printf("choose a password for this account: ");

    // the password is requested after launching the command in order to acquire it safely (read_line_s() is used).
    if (!(pass_word = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }

    printf("\n");

    // appending the account name and the user/mail to accounts.list file
    if (append_account(acct_name, user_mail) != 0) {
        perror("account storage failed");
        return -1;
    }

    // appending the password to passwords.list file
    if (append_pass(pass_word) != 0) {
        perror("password storage failed");
        return -1;
    }

    ret_code = 0;

    printf("account successfully stored!\n");

ret:
    sodium_free(acct_name);
    sodium_free(user_mail);
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
    int line_indx;
    int code;

    if (!args[1] || args[2]) {
        printf("\"rm\" needs to know an account name (1 argument needed)\n");
    }

    if ((code = remove_account(args[1], &line_indx)) < 0) {
        perror("account removal failed");
        return -1;
    } else if (code == 1) {
        printf("account not found\n");
        return 0;
    }

    if (remove_password(line_indx) != 0) {
        perror("password removal failed");
        return -1;
    }

    return 0;
} 

int psm_get(char **args)
{
    size_t line_len;

    pthread_t thread_id;

    int code;
    int line_indx;

    unsigned char *file_content;
    unsigned char **lines;
    unsigned char *pass;

    char *file_path = ACCT_FILE_PATH;

    if (!args[1] || args[2]) {
        printf("\"get\" needs to know an account name (1 argument needed)\n");
    }

    // decrypting accounts.list
    if (!(file_content = decrypt_file(file_path, subkeys[skey_acct]))) {
        return -1;
    }

    // splitting file content in lines
    lines = split_by_delim(file_content, SEPARATE_LINE_STR);

    // finding account index
    if ((line_indx = find_line_indx(lines, args[1], &line_len)) < 0) {
        printf("account not found\n");
        return 0;
    }

    // get password at right index
    if (!(pass = get_pass(line_indx))) {
        perror("psm: I/O error");
        return -1;
    }

    //start a new thread handling password copying in clipboard
    pthread_create(&thread_id, NULL, (void *) save_in_clipboard, pass);

    return 0;
}

/*-------------TEST-------------*/

int test_show_password(char **args) 
{
    unsigned char *file_content;

    char *file_path = PASS_FILE_PATH;  
    char ch;  

    int pos = 0;

    if (!(file_content = decrypt_file(file_path, subkeys[skey_pass]))) {
        perror("test: cryptography error");
        return -1;
    }

    while ((ch = file_content[pos++]) != '\0') {
        if (ch > 126 || ch < 33) {
            printf(" %d ", ch);
        } else {
            printf("%c", ch);
        }
    }

    printf("\n");

    return 0;
}

int test_show_account(char **args) 
{
    unsigned char *file_content;

    char *file_path = ACCT_FILE_PATH;  
    char ch;  

    int pos = 0;

    if (!(file_content = decrypt_file(file_path, subkeys[skey_acct]))) {
        perror("test: cryptography error");
        return -1;
    }

    while ((ch = file_content[pos++]) != '\0') {
        if (ch > 126 || ch < 33) {
            printf(" %d ", ch);
        } else {
            printf("%c", ch);
        }
    }

    printf("\n");

    return 0;
}

/*-------------TEST-------------*/

int psm_exit(char **args) {
    exit(EXIT_SUCCESS);
}

// this function is the basically the shell itself: it prints a prompt on the screen,
// reads the user input, splits the input in tokens separated by spaces (command name + args) 
// and executes commands using psm_launch().
int psm_exec()
{
    char *line;
    char **args;

    while (1) 
    {
        printf("> ");                                   // printing the prompt.

        line = read_line();                             // reading the user input.
        args = psm_split_line(line);                    // splitting the input in tokens (command name + arg1 + arg2 + ...).

        psm_launch(args);                               // launching the command passing its name along with its arguments.
    }

    free(line);
    free(args);

    return 1;
}

// launching the command passed as an array containing the command name and its arguments.
int psm_launch(char **args) 
{
    // checking for empty commands
    if (args[0] == NULL) {
        return 1;
    }

    // if the first argument of the array equals to the name of a stored command, 
    // the corresponding function is called and the arguments are passed along.
    for (int i = 0; i < psm_num_commands(); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_addr[i])(args);
        }
    }

    printf("command not found\n");
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
    if (!(file_content = decrypt_file(file_path, subkeys[skey_acct]))) {
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    file_cont_size = strlen(file_content);

    totl_buff_size = file_cont_size + acct_name_size + 1 + user_mail_size + 1;          // original buffer + account_name + 0xF0 + user_or_mail + 0xD8
    
    totl_content = (unsigned char *) sodium_malloc(totl_buff_size + 1);                 // initialising the new buffer
    totl_content[0] = '\0';

    strcat(totl_content, file_content);                                                 // copying the old buffer into the new bigger buffer
    strcat(totl_content, acct_name);                                                    // appending account_name to the new buffer
    strcat(totl_content, sprt_tkns_char);                                               // appending 0xF0 (separation byte) after account_name
    strcat(totl_content, user_or_mail);                                                 // appending user_or_mail to the new buffer
    strcat(totl_content, sprt_line_char);                                               // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the accounts.list file
    if (encrypt_buffer(totl_content, subkeys[skey_acct], file_path) != 0) {
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
    if (!(file_content = decrypt_file(file_path, subkeys[skey_pass]))) {
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    file_cont_size = strlen(file_content);

    totl_buff_size = file_cont_size + pass_word_size + 1;                               // original buffer + password + 0xD8

    totl_content = (unsigned char *) sodium_malloc(totl_buff_size + 1);                 // initialising the new buffer
    totl_content[0] = '\0';

    strcat(totl_content, file_content);                                                 // copying the old buffer into the new bigger buffer
    strcat(totl_content, pass);                                                         // appending pass to the new buffer
    strcat(totl_content, sprt_line_char);                                               // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the passwords.list file
    if (encrypt_buffer(totl_content, subkeys[skey_pass], file_path) != 0) {
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
    size_t str_len;
    size_t old_size;
    
    unsigned char **tokens = (unsigned char **) sodium_malloc(bufsize);
    unsigned char *token;
    unsigned char *str_copy;
    
    int pos = 0;

    if (!tokens) {
        perror("psm: allocation error\n");
        return NULL;
    }   

    // creating a copy of the original buffer in order to prevent it beeing corrupted by strtok
    str_len = strlen(str);
    str_copy = (unsigned char *) sodium_malloc(str_len + 1);
    strncpy(str_copy, str, str_len);
    str_copy[str_len] = '\0';

    token = strtok(str_copy, delim);

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

// this function removes an account from accounts.list file.
int remove_account(unsigned char *account_name, int *line_indx)
{
    size_t line_len;                                                // deleted line's length
    size_t cont_len;                                                // file content's length
    size_t new_cont_len;                                            // resulting length after deleting the line

    unsigned char *file_content;
    unsigned char *new_file_cont;
    unsigned char **content_lines;                                  // file content splitted into individual lines

    char *file_path = ACCT_FILE_PATH;

    int ret_code = -1;

    // decrypting the file content
    if (!(file_content = decrypt_file(file_path, subkeys[skey_acct]))) {
        perror("psm: cryptography error");
        goto ret;
    }

    // obtaining file content's length
    cont_len = strlen(file_content);

    // splitting file content into individual lines
    content_lines = split_by_delim(file_content, SEPARATE_LINE_STR);

    // finding the index of the line that needs to be removed. the length of this line is stored in line_len.
    if ((*line_indx = find_line_indx(content_lines, account_name, &line_len)) < 0) {
        // account not found
        ret_code = 1;
        goto ret;
    }

    // creating a new buffer that will contain the
    // original content without the deleted line
    new_cont_len = cont_len - (line_len + 1);                       // file_content - (deleted_line_len + end_of_line_byte)
    new_file_cont = rebuild_buff_from_lines(content_lines, 
                                            new_cont_len, 
                                            *line_indx);

    // encrypting the new buffer into accounts.list
    if (encrypt_buffer(new_file_cont, subkeys[skey_acct], file_path) != 0) {
        perror("psm: cryptography error");
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(content_lines);
    return ret_code;
}

// this function returns the index of a line. the line's length will be stored 
// at line_len address.
int find_line_indx(unsigned char **lines, unsigned char *str_to_match, size_t *line_len) 
{
    size_t single_line_len;

    unsigned char *line;
    unsigned char **line_tokens;                                    // account_name and mail_or_user                                

    int pos = 0;

    while ((line = lines[pos]) != NULL) {

        single_line_len = strlen(line);  
        line_tokens = split_by_delim(line, SEPARATE_TKNS_STR);

        // if the account_name matches the one provided by the user, line's index and length get returned.
        if (strcmp(line_tokens[0], str_to_match) == 0) {
            *line_len = single_line_len;
            return pos;
        }

        pos++;
    }

    // if the program runs up to here, it means that no match was found.
    return -1;
}

// this function removes a password from passwords.list file
int remove_password(int line_indx)
{
    size_t line_len;                                                // deleted line's length
    size_t cont_len;                                                // file content's length
    size_t new_cont_len;                                            // resulting length after deleting the line

    unsigned char *file_content;
    unsigned char *new_file_content;
    unsigned char **content_lines;                                  // file content splitted into individual lines

    int ret_code = -1;

    char *file_path = PASS_FILE_PATH;

    // decrypting the file content
    if (!(file_content = decrypt_file(file_path, subkeys[skey_pass]))) {
        perror("psm: cryptography error");
        goto ret;
    }

    // obtaining file content's length
    cont_len = strlen(file_content);

    // splitting file content into individual lines
    content_lines = split_by_delim(file_content, SEPARATE_LINE_STR);
    
    // creating a new buffer that will contain the 
    // original content without the deleted line
    line_len = strlen(content_lines[line_indx]);
    new_cont_len = cont_len - (line_len + 1);                       // file_content - (deleted_line_len + end_of_line_byte)
    new_file_content = rebuild_buff_from_lines(content_lines, 
                                               new_cont_len, 
                                               line_indx);

    // encrypting the new buffer into passwords.list
    if (encrypt_buffer(new_file_content, subkeys[skey_pass], file_path) != 0) {
        perror("psm: cryptography error");
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(content_lines);
    return ret_code;
}

// this function concatenates all lines contained in lines buffer except for the one that needs
// to be deleted. it separates them by using 0xD8 byte.
unsigned char *rebuild_buff_from_lines(unsigned char **lines, size_t buff_len, int line_to_rm_indx) 
{
    size_t lines_amount = arrlen((void **) lines);

    unsigned char *new_buff = (unsigned char *) sodium_malloc(buff_len + 1);
    new_buff[0] = '\0';

    if (buff_len != 0) {    

        for (int i=0; i<lines_amount; i++) {
            // it concatenates all lines that do not match the index to be removed
            if (i != line_to_rm_indx) {
                strcat(new_buff, lines[i]);
                strcat(new_buff, SEPARATE_LINE_STR);
            }
        }

        new_buff[buff_len] = '\0';
    }

    return new_buff;
}

// this function retrieves the password located at line_indx
// index from passwords.file
unsigned char *get_pass(int line_indx)
{
    size_t pass_len;

    unsigned char *pass;
    unsigned char *file_content;
    unsigned char **lines;

    char *file_path = PASS_FILE_PATH;

    // decrypting passwords.list file
    if (!(file_content = decrypt_file(file_path, subkeys[skey_pass]))) {
        return NULL;
    }

    // splitting file_content in lines
    lines = split_by_delim(file_content, SEPARATE_LINE_STR);

    // if line_indx > number of lines return -1
    if (line_indx > arrlen((void **) lines)) {
        perror("psm: get_pass: invalid index");
        return NULL;
    }

    // retrieving line at index line_indx
    pass_len = strlen(lines[line_indx]);
    pass = (unsigned char *) sodium_malloc(pass_len + 1);
    memcpy(pass, lines[line_indx], pass_len);
    pass[pass_len] = '\0';

    return pass;
}