#include "../../utils/headers/input_acquisition.h"
#include "../../utils/headers/config.h"
#include "../../utils/headers/cryptography.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

int build_path(char **root, char *rel_path); 
char *add_ext(char *fname, const char *ext);

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
    This function builds a path in the format:
        
        {root}/{rel_path}
*/
int build_path(char **root, char *rel_path) 
{
    size_t root_size = strlen(*root);
    size_t rel_path_size = strlen(rel_path);

    /* Allocate {root}/{rel_path} */
    *root = realloc(*root, sizeof(char) * (root_size + 1 + rel_path_size + 1));

    /* Check allocation */
    if (!*root) {
        perror("psm: allocation error\n");
        return -1;
    }

    /* Build {root}/{rel_path} */
    strcat(*root, "/");
    strcat(*root, rel_path);

    return 0;
}

/*
    This function adds the extension ext to
    a string fname.
*/
char *add_ext(char *fname, const char *ext)
{
    size_t ext_size = strlen(ext);
    size_t fname_size = strlen(fname);

    /* Allocate fname + ext */
    char *new_fname = calloc(fname_size + ext_size + 1, sizeof(char));

    /* Check allocation */
    if (!new_fname) {
        perror("psm: allocation error");
        return NULL;
    }

    /* Build fname + ext */
    strcpy(new_fname, fname);
    strcat(new_fname, ext);

    return new_fname;
}