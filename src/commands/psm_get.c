#include "./psm_get.h"
#include "../utils/clipboard.h"
#include "../utils/crypto.h"
#include "../utils/config.h"
#include "../utils/fio.h"
#include "../utils/path.h"

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
    const char *PATH = psm_getenv("PATH");
    const char *GPG_ID = psm_getenv("GPG_ID");

    size_t rlen;
    int ret_code = -1;

    pthread_t thread_id;

    if (!PATH || !GPG_ID || !cyphertext || !plaintext) {
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
    PATH ? free((char *) PATH) : 0;
    GPG_ID ? free((char *) GPG_ID) : 0;
    return ret_code;  
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
