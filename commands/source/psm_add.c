#include "../headers/psm_add.h"
#include "../../utils/headers/input.h"
#include "../../utils/headers/config.h"
#include "../../utils/headers/crypto.h"
#include "../../utils/headers/path.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

int add_newline(char **str);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function adds a new file to PATH encrypting it using
    public gpg key of id GPG_ID.

    Arguments:
        [FILE]      this is a relative path which has PATH as root

    Examples: 
        add file_name               stored at PATH/file_name.gpg
        add dir_name/file_name      stored at PATH/dir_name/file_name.gpg
*/
int psm_add(char **args)
{
    char *plaintext = calloc(1, sizeof(char));
    char *rel_path = NULL;
    
    const char *PATH = get_env_var("PATH");
    const char *GPG_ID = get_env_var("GPG_ID"); 
    const char *cyphertext = NULL;

    FILE *file = NULL;

    size_t wlen;
    size_t last_index;

    int ret_code = -1;

    /* Check allocation */
    if (!plaintext || !PATH || !GPG_ID) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: add [FILE]\n");
        goto ret;
    }

    last_index = strlen(args[1]) - 1;

    /* Check for trailing slash (i.e. directory) */
    if (args[1][last_index] == '/') {
        printf("Cannot create empty directories\n");
        goto ret;
    }
    
    rel_path = add_ext(args[1], ".gpg"); 

    /* Check allocation */
    if (!rel_path) {
        perror("psm: allocation error");
        goto ret;
    }

    printf("Insert password for %s: ", rel_path);

    /* Retrieve plaintext from user */
    if (read_line_s(&plaintext, 1) != 0) {
        perror("psm: allocation error");
        goto ret;
    }

    printf("\n");

    /* Add newline char after plaintext */
    if (add_newline(&plaintext) != 0) {
        perror("psm: allocation error");
        goto ret;
    }

    if (build_path((char **) &PATH, rel_path) != 0) {
        perror("psm: allocation error");
        goto ret;
    } 

    /* Encrypt plaintext */
    if (!(cyphertext = gpg_encrypt(plaintext, GPG_ID))) {
        perror("psm: allocation error");
        goto ret;
    }

    file = fopen(PATH, "w");

    if (!file) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Write cyphertext to file */
    if ((wlen = fwrite(cyphertext, sizeof(char), strlen(cyphertext), file)) < 0) {
        perror("psm: I/O error");
        goto ret; 
    }

    ret_code = 0;

ret:
    rel_path ? free(rel_path) : 0;
    GPG_ID ? free((char *) GPG_ID) : 0;
    file ? fclose(file) : 0;
    PATH ? free((char *) PATH) : 0;
    plaintext ? free(plaintext) : 0;
    return ret_code;
}

/*
    This function add a newline char
    after str.
*/
int add_newline(char **str)
{
    size_t str_size = strlen(*str);

    /* Stretch the array to fit one more char */
    *str = realloc(*str, sizeof(char) * (str_size + 2));

    if (!*str) {
        perror("psm: allocation error");
        return -1;
    }

    str[0][str_size] = '\n';
    str[0][str_size + 1] = '\0';
    
    return 0;
}