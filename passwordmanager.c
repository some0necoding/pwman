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

int psm_show(char **args);
int psm_add(char ** args);
int psm_modify(char **args);
int psm_remove(char **args);
int psm_get(char **args);
int psm_help(char **args);
int psm_exit(char **args);

int psm_split_line(char *line, char **tokens, size_t tokens_size);
int psm_launch(char **args) ;
int psm_num_commands(void);

int append_account(unsigned char *acct_name, unsigned char *user_or_mail);
int append_pass(unsigned char *pass);
unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);
int remove_account(unsigned char *account_name, int *line_indx);
int find_line_indx(unsigned char **lines, unsigned char *str_to_match, size_t *line_len);
int remove_password(int line_indx);
unsigned char *rebuild_buff_from_lines(unsigned char **lines, size_t buff_len, int line_to_rm_indx);
int get_pass(int line_indx, unsigned char *ret_buff, size_t ret_buff_size);

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
    "show",
    "add",
    "edit",
    "rm",
    "get",
    "help",
    "exit"
};

// array of command functions
int (*command_addr[]) (char **) = {
    &psm_show,
    &psm_add,
    &psm_modify,
    &psm_remove,
    &psm_get,
    &psm_help,
    &psm_exit
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
    size_t content_size = CHUNK_SIZE;

    char *file_path = ACCT_FILE_PATH;                           // accounts.list file path.
    
    unsigned char *content = (unsigned char *) sodium_malloc(content_size);
    unsigned char **content_lines;                              // array containing the file content splitted in lines. Every line contains an account.
    unsigned char *content_line;                                // a single line of the file content (i.e. a single account).
    unsigned char **tokens;                                     // array containing the account name and the mail of the account, stored in a single line.
    unsigned char *acct_name;                                   // account name.
    unsigned char *user_mail;                                   // username or mail.

    int pos = 0;
    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // some kinda input verification.
    if (args[1]) {
        printf("\"show\" does not accept arguments\n");
        sodium_free(content);
        return -1;
    }

    // the file remains encrypted, while the decrypted content gets stored in a buffer (i.e. file_content).
    if (decrypt_file(file_path, subkeys[skey_acct], &content, content_size) != 0) {
        perror("psm: cryptography error");
        goto ret;
    }

    // the file content gets splitted in lines and stored in content_lines, that is NULL terminated.
    if (!(content_lines = split_by_delim(content, SEPARATE_LINE_STR))) {
        goto ret;
    }

    // looping through the lines to print info about every account.
    while ((content_line = content_lines[pos]) != NULL) {

        // the line gets splitted in tokens (account name and user/mail).
        if (!(tokens = split_by_delim(content_line, SEPARATE_TKNS_STR))) {
            goto ret;
        }

        acct_name = tokens[0];
        user_mail = tokens[1];

        printf("%1$d.\n\taccount: %2$s\n\tuser: %3$s\n\tpassword: hidden\n", pos+1, acct_name, user_mail);

        pos++;
    }

    ret_code = 0;

ret:
    sodium_free(content);
    return ret_code;
}

// this function adds a new account to the "database". Account name and user/mail are appended
// to accounts.list file, while the password is appended to passwords.list file. The storage
// process follows the storage pattern.
// - Arguments: account_name, user_or_mail.
int psm_add(char **args)
{
    int ret_code = -1;

    size_t pass_word_len = 64;
    size_t acct_name_len;
    size_t user_mail_len;

    unsigned char *acct_name;
    unsigned char *user_mail;
    unsigned char *pass_word = (unsigned char *) sodium_malloc(pass_word_len);

    if (!pass_word) {
        perror("psm: allocation error");
        return -1;
    }

    // add accepts only 2 args, so if the 2nd arg doesn't exist or if the 3rd arg do exist, the program throws an error.
    if (!args[2] || args[3]) {
        printf("\"add\" needs to know an account name and the mail used to login (2 arguments needed)\n");
        sodium_free(pass_word);
        return -1;
    }

    acct_name_len = strlen(args[1]);
    user_mail_len = strlen(args[2]);

    acct_name = (unsigned char *) sodium_malloc(acct_name_len + 1);
    user_mail = (unsigned char *) sodium_malloc(user_mail_len + 1);

    if (!acct_name | !user_mail) {
        perror("psm: allocation error");
        sodium_free(pass_word);
        acct_name ? sodium_free(acct_name) : 0;
        user_mail ? sodium_free(user_mail) : 0;
        return -1;
    }

    strcpy(acct_name, args[1]);
    strcpy(user_mail, args[2]);

    printf("choose a password for this account: ");

    // the password is requested after launching the command in order to acquire it safely (read_line_s() is used).
    if (read_line_s((char **) &pass_word, pass_word_len) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    printf("\n");

    // appending the account name and the user/mail to accounts.list file
    if (append_account(acct_name, user_mail) != 0) {
        perror("psm: account storage failed");
        goto ret;
    }

    // appending the password to passwords.list file
    if (append_pass(pass_word) != 0) {
        perror("psm: password storage failed");
        goto ret;
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
        return -1;
    }

    if ((code = remove_account(args[1], &line_indx)) < 0) {
        perror("account removal failed");
        return -1;
    } else if (code == 1) {
        printf("account not found\n");
        return -1;
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
    size_t content_size = CHUNK_SIZE;
    size_t pass_size = PASS_LENGTH;

    pthread_t thread_id;

    unsigned char *content = (unsigned char *) sodium_malloc(content_size);
    unsigned char **lines;
    unsigned char *pass = (unsigned char *) sodium_malloc(pass_size);

    char *file_path = ACCT_FILE_PATH;

    int ret_code = -1;
    int line_indx;

    if (!content | !pass) {
        perror("psm: allocation error");
        content ? sodium_free(content) : 0;
        pass ? sodium_free(pass) : 0;
        return -1;
    }

    if (!args[1] || args[2]) {
        printf("\"get\" needs to know an account name (1 argument needed)\n");
        sodium_free(content);
        sodium_free(pass);
        return -1;
    }

    // decrypting accounts.list
    if (decrypt_file(file_path, subkeys[skey_acct], &content, content_size) != 0) {
        goto ret;
    }

    // splitting file content in lines
    lines = split_by_delim(content, SEPARATE_LINE_STR);

    // finding account index
    if ((line_indx = find_line_indx(lines, args[1], &line_len)) < 0) {
        printf("account not found\n");
        goto ret;
    }

    // get password at right index
    if (get_pass(line_indx, pass, pass_size) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    //start a new thread handling password copying in clipboard
    pthread_create(&thread_id, NULL, (void *) save_in_clipboard, pass);

    ret_code = 0;

ret:
    sodium_free(content);
    sodium_free(pass);
    return ret_code;
}

int psm_help(char **args)
{
    unsigned char *help_txt;

    if (args[1]) {
        printf("\"help\" does not accept arguments\n");
        return -1;
    }

    help_txt = "\nWHAT IS PASSMAN:\n"
               "\n"
               "    Passman stores and retrieves accounts along with\n"
	           "    their passwords.\n"
               "\n"
	           "    Every account is made up by three parts:\n"
		       "        - account name\n" 
		       "        - email or username used to log in the account\n"
		       "        - password used to log in the account\n"
               "\n"
               "SYNTAX:\n" 
               "\n"
	           "    show\n" 
	           "    add <account-name> <user-or-mail>\n" 
	           "    rm <account-name>\n"
	           "    get <account-name>\n" 
	           "    help\n"
               "\n"
               "DESCRIPTION:\n"
               "\n"
	           "    show\n" 
		       "        To show all accounts stored.\n"
               "\n"
	           "    add\n" 
		       "        To add a new account.\n"
               "\n"
	           "    rm\n"
		       "        To remove an account.\n" 
               "\n"
	           "    get\n"
		       "        To retrieve a password. Note that it will be saved\n"
               "        into the (X11) clipboard so that it can be directly\n"
               "        pasted.\n"
               "\n"
	           "    help\n" 
		       "        To display help page.\n";

    printf("%s\n", help_txt);

    return 0;
}

int psm_exit(char **args) 
{
    exit(EXIT_SUCCESS);
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

// here's some additional functions needed to run commands

// this function appends a new account (account_name + user_or_mail) to accounts.list file.
// At first it decrypts the file into a buffer, then it appends to this buffer account_name 
// and user_or_mail separating them with 0xF0 byte and ending the line with 0xD8 byte. At
// the end it encrypts the modifyied buffer into accounts.list.
int append_account(unsigned char *acct_name, unsigned char *user_or_mail)
{
    size_t content_size = CHUNK_SIZE;
    size_t acct_name_size = strlen(acct_name);          // size of account_name
    size_t user_mail_size = strlen(user_or_mail);       // size of user_or_mail
    size_t totl_buff_size;                              // size of the new buffer

    char *sprt_tkns_char = SEPARATE_TKNS_STR;           // 0xF0 byte stored as a string so that it can be used with string.h functions
    char *sprt_line_char = SEPARATE_LINE_STR;           // 0xD8 byte stored as a string so that it can be used with string.h functions
    char *file_path = ACCT_FILE_PATH;                   // accounts.list file path

    unsigned char *content = (unsigned char *) sodium_malloc(content_size);
    unsigned char *totl_content;                        // this will get allocated later on

    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // decrypting the accounts.list file into a buffer
    if (decrypt_file(file_path, subkeys[skey_acct], &content, content_size) != 0) {
        sodium_free(content);
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    content_size = strlen(content);
    totl_buff_size = content_size + acct_name_size + 1 + user_mail_size + 1;            // original buffer + account_name + 0xF0 + user_or_mail + 0xD8
    totl_content = (unsigned char *) sodium_malloc(totl_buff_size + 1);                 // initialising the new buffer

    if (!totl_content) {
        perror("psm: allocation error");
        sodium_free(content);
        return -1;
    }

    totl_content[0] = '\0';

    strcat(totl_content, content);                                                 // copying the old buffer into the new bigger buffer
    strcat(totl_content, acct_name);                                                    // appending account_name to the new buffer
    strcat(totl_content, sprt_tkns_char);                                               // appending 0xF0 (separation byte) after account_name
    strcat(totl_content, user_or_mail);                                                 // appending user_or_mail to the new buffer
    strcat(totl_content, sprt_line_char);                                               // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the accounts.list file
    if (encrypt_buffer(totl_content, subkeys[skey_acct], file_path) != 0) {
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(content);
    sodium_free(totl_content);
    return ret_code;
}

// this function appends a new password to passwords.list file. At first it decrypts 
// the file into a buffer, then it appends to this buffer password ending the line 
// with 0xD8 byte. At the end it encrypts the modifyied buffer into passwords.list.
int append_pass(unsigned char *pass)
{
    size_t content_size = CHUNK_SIZE;
    size_t pass_word_size = strlen(pass);
    size_t totl_buff_size;                                                              // size of the new buffer

    char *sprt_line_char = SEPARATE_LINE_STR;                                           // 0xD8 byte as a string so that it can be used with string.h functions
    char *file_path = PASS_FILE_PATH;

    unsigned char *content = (unsigned char *) sodium_malloc(content_size);
    unsigned char *totl_content;                                                        // this will get allocated later on

    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // decrypting the passwords.list file into a buffer
    if (decrypt_file(file_path, subkeys[skey_pass], &content, content_size)) {
        sodium_free(content);
        return -1;
    }

    // based on the file_content size we can determine the total length of the modified buffer
    content_size = strlen(content);
    totl_buff_size = content_size + pass_word_size + 1;                                 // original buffer + password + 0xD8
    totl_content = (unsigned char *) sodium_malloc(totl_buff_size + 1);                 // initialising the new buffer

    if (!totl_content) {
        perror("psm: allocation error");
        sodium_free(content);
        return -1;
    }

    totl_content[0] = '\0';

    strcat(totl_content, content);                                                      // copying the old buffer into the new bigger buffer
    strcat(totl_content, pass);                                                         // appending pass to the new buffer
    strcat(totl_content, sprt_line_char);                                               // appending 0xD8 (separation byte) after user_or_mail

    // encrypting the new buffer into the passwords.list file
    if (encrypt_buffer(totl_content, subkeys[skey_pass], file_path) != 0) {
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(content);
    sodium_free(totl_content);
    return ret_code;
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
                sodium_free(str_copy);
                sodium_free(tokens);
                return NULL;
            }
        }

        token = strtok(NULL, delim);
    }

    tokens[pos] = NULL;

    sodium_free(str_copy);
    return tokens;
}

// this function removes an account from accounts.list file.
int remove_account(unsigned char *account_name, int *line_indx)
{
    size_t content_len = CHUNK_SIZE;
    size_t line_len;                                                // deleted line's length
    size_t new_cont_len;                                            // resulting length after deleting the line

    unsigned char *content = (unsigned char *) sodium_malloc(content_len);
    unsigned char *new_file_cont;
    unsigned char **content_lines;                                  // file content splitted into individual lines

    char *file_path = ACCT_FILE_PATH;

    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // decrypting the file content
    if (decrypt_file(file_path, subkeys[skey_acct], &content, content_len) != 0) {
        perror("psm: cryptography error");
        sodium_free(content);
        return -1;
    }

    // obtaining file content's length
    content_len = strlen(content);

    // splitting file content into individual lines
    content_lines = split_by_delim(content, SEPARATE_LINE_STR);

    if (!content_lines) {
        perror("psm: allocation error");
        sodium_free(content);
        return -1;
    }

    // finding the index of the line that needs to be removed. the length of this line is stored in line_len.
    if ((*line_indx = find_line_indx(content_lines, account_name, &line_len)) < 0) {
        sodium_free(content);
        return 1;   // account not found
    }

    // creating a new buffer that will contain the
    // original content without the deleted line
    new_cont_len = content_len - (line_len + 1);                       // file_content - (deleted_line_len + end_of_line_byte)
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
    sodium_free(content);
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

        if (!line_tokens) {
            perror("psm: allocation error");
            return -1;
        }

        // if the account_name matches the one provided by the user, line's index and length get returned.
        if (strcmp(line_tokens[0], str_to_match) == 0) {
            *line_len = single_line_len;
            sodium_free(line_tokens);
            return pos;
        }

        pos++;
    }

    // if the program runs up to here, it means that no match was found.
    sodium_free(line_tokens);
    return -1;
}

// this function removes a password from passwords.list file
int remove_password(int line_indx)
{
    size_t line_len;                                                // deleted line's length
    size_t content_len = CHUNK_SIZE;                                                // file content's length
    size_t new_cont_len;                                            // resulting length after deleting the line

    unsigned char *content = (unsigned char *) sodium_malloc(content_len);
    unsigned char *new_file_content;
    unsigned char **content_lines;                                  // file content splitted into individual lines

    char *file_path = PASS_FILE_PATH;
    
    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // decrypting the file content
    if (decrypt_file(file_path, subkeys[skey_pass], &content, content_len)) {
        perror("psm: cryptography error");
        sodium_free(content);
        return -1;
    }

    // obtaining file content's length
    content_len = strlen(content);

    // splitting file content into individual lines
    content_lines = split_by_delim(content, SEPARATE_LINE_STR);

    if (!content_lines) {
        perror("psm: allocation error");
        sodium_free(content);
        return -1;
    }
    
    // creating a new buffer that will contain the 
    // original content without the deleted line
    line_len = strlen(content_lines[line_indx]);
    new_cont_len = content_len - (line_len + 1);                       // file_content - (deleted_line_len + end_of_line_byte)
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
    sodium_free(content);
    sodium_free(content_lines);
    return ret_code;
}

// this function concatenates all lines contained in lines buffer except for the one that needs
// to be deleted. it separates them by using 0xD8 byte.
unsigned char *rebuild_buff_from_lines(unsigned char **lines, size_t buff_len, int line_to_rm_indx) 
{
    size_t lines_amount = arrlen((void **) lines);

    unsigned char *new_buff = (unsigned char *) sodium_malloc(buff_len + 1);

    if (!new_buff) {
        perror("psm: allocation error");
        return NULL;
    }

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
int get_pass(int line_indx, unsigned char *ret_buff, size_t ret_buff_size)
{
    size_t pass_len;
    size_t content_size = CHUNK_SIZE;

    unsigned char *content = (unsigned char *) sodium_malloc(content_size);
    unsigned char **lines;

    char *file_path = PASS_FILE_PATH;

    int ret_code = -1;

    if (!content) {
        perror("psm: allocation error");
        return -1;
    }

    // decrypting passwords.list file
    if (decrypt_file(file_path, subkeys[skey_pass], &content, content_size) != 0) {
        perror("psm: cryptography error");
        sodium_free(content);
        return -1;
    }

    // splitting file_content in lines
    lines = split_by_delim(content, SEPARATE_LINE_STR);

    if (!lines) {
        perror("psm: allocation error");
        sodium_free(content);
        return -1;
    }

    // if line_indx > number of lines return -1
    if (line_indx > arrlen((void **) lines)) {
        perror("psm: get_pass: invalid index");
        goto ret;
    }

    // retrieving line at index line_indx
    pass_len = strlen(lines[line_indx]);
    
    if (ret_buff_size < pass_len) {
        ret_buff = (unsigned char *) sodium_realloc(ret_buff, ret_buff_size, (pass_len + 1));

        if (!ret_buff) {
            perror("psm: allocation error");
            goto ret;
        }
    }

    memcpy(ret_buff, lines[line_indx], pass_len);
    ret_buff[pass_len] = '\0';

    ret_code = 0;

ret:
    sodium_free(content);
    sodium_free(lines);
    return ret_code;
}