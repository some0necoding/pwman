#include "../../utils/headers/clipboard.h"
#include "../../utils/headers/crypto.h"
#include "../../utils/headers/config.h"
#include "../../utils/headers/fio.h"

#include <gpg-error.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE 32

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

char *add_ext(char *fname, const char *ext);
int build_path(char **root, char *rel_path);
int rm_newline(char **str);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function decrypts a file and stores its content
    into clipboard.

    Arguments:
        [FILE]      this is a relative path which has PATH as root

    Examples:
        get test_file               stored at PATH/test_file
        get test_dir/test_file      stored at PATH/test_dir/test_file
*/
int psm_get(char **args)
{
    char *rel_path = NULL;
    char *cyphertext = calloc(BUFSIZE, sizeof(char));
    char *plaintext = calloc(BUFSIZE, sizeof(char));
    const char *PATH = get_env_var("PATH");
    const char *GPG_ID = get_env_var("GPG_ID");

    size_t rlen;
    int ret_code = -1;

    pthread_t thread_id;

    if (!PATH || !GPG_ID || !cyphertext) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: get [FILE]\n");
        goto ret;
    }

    rel_path = add_ext(args[1], ".gpg");

    /* Check allocation */
    if (!rel_path) {
        perror("psm: allocation error");
        goto ret;
    }

    if (build_path((char **) &PATH, rel_path) != 0) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Check for file existance */
    if (access(PATH, F_OK) != 0) {
        printf("No such file or directory\n");
        ret_code = 0;
        goto ret;
    }

    /* Retrieve cyphertext */
    if ((rlen = fgetall((char *) PATH, &cyphertext, BUFSIZE)) < 0) {
        perror("psm: allocation error");
        goto ret;
    } 

    int err = gpg_decrypt(cyphertext, GPG_ID, &plaintext, BUFSIZE);

    if (err == 0) {
        printf("Incorrect passphrase\n");
        ret_code = 0;
        goto ret;
    } else if (err == -1) {
        perror("psm: allocation error");
        goto ret;
    }

    rm_newline(&plaintext);

    /* Create a thread managing clipboard */
    if (pthread_create(&thread_id, NULL, (void *) save_in_clipboard, plaintext) != 0) {
        perror("psm: error in thread creation");
        goto ret;
    }

    ret_code = 0;

ret:
    rel_path ? free(rel_path) : 0;
    cyphertext ? free(cyphertext) : 0;
    plaintext ? free((char *) plaintext) : 0;
    PATH ? free((char *) PATH) : 0;
    GPG_ID ? free((char *) GPG_ID) : 0;
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

/*
    This function removes a newline char
    after str.
*/
int rm_newline(char **str)
{
    size_t str_size = strlen(*str);
    str[0][str_size - 1] = '\0';

    return 0;
}