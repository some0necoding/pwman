#include "../../utils/headers/auth.h"
#include "../../utils/headers/sodiumplusplus.h"
#include "../../utils/headers/cryptography.h"
#include <stdio.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64

#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define CHUNK_SIZE 512

#define ACCT_FILE_PATH "/usr/share/binaries/accounts.list"
#define PASS_FILE_PATH "/usr/share/binaries/passwords.list"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

    int skey_acct = 0;
    int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

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