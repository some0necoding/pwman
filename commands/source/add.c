#include "../../utils/headers/sodiumplusplus.h"
#include "../../utils/headers/input_acquisition.h"
#include "../../utils/headers/auth.h"
#include "../../utils/headers/cryptography.h"

#include <stdio.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/  

#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define ACCT_FILE_PATH "/usr/share/binaries/accounts.list"
#define PASS_FILE_PATH "/usr/share/binaries/passwords.list" 

#define CHUNK_SIZE 512

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int append_account(unsigned char *acct_name, unsigned char *user_or_mail);
int append_pass(unsigned char *pass);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

int skey_acct = 0;
int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

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