#include "../../utils/headers/auth.h"
#include "../../utils/headers/clipboard_managment.h"
#include "./utils/headers/sodiumplusplus.h"
#include "./utils/headers/cryptography.h"
#include "./utils/headers/array_handling.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64
#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define CHUNK_SIZE 512

#define ACCT_FILE_PATH "/usr/share/binaries/accounts.list"
#define PASS_FILE_PATH "/usr/share/binaries/passwords.list"

#define PASS_LENGTH 65

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);
int find_line_indx(unsigned char **lines, unsigned char *str_to_match, size_t *line_len);
int get_pass(int line_indx, unsigned char *ret_buff, size_t ret_buff_size);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

    int skey_acct = 0;
    int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

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