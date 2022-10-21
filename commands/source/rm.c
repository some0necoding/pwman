#include "../../utils/headers/auth.h"
#include "../../utils/headers/cryptography.h"
#include "../../utils/headers/sodiumplusplus.h"

#include <stdio.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define PSM_TOK_BUFSIZE 64

#define SEPARATE_LINE_STR "\xD8"   // 0xD8 byte stored as a string to use with string.h functions (marks the end of a line in *.list files)
#define SEPARATE_TKNS_STR "\xF0"   // 0xF0 byte stored as a string to use with string.h functions (separates two tokens on the same line in *.list files)

#define CHUNK_SIZE 512

#define ACCT_FILE_PATH "/usr/share/binaries/accounts.list"
#define PASS_FILE_PATH "/usr/share/binaries/passwords.list"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int remove_account(unsigned char *account_name, int *line_indx);
int remove_password(int line_indx);
unsigned char **split_by_delim(unsigned char *str, unsigned char *delim);
int find_line_indx(unsigned char **lines, unsigned char *str_to_match, size_t *line_len);
unsigned char *rebuild_buff_from_lines(unsigned char **lines, size_t buff_len, int line_to_rm_indx);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

    int skey_acct = 0;
    int skey_pass = 1;

/*-------------GLOBAL-VARIABLES-END-------------*/

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