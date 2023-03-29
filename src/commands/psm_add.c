#include "./psm_add.h"
#include "../utils/input.h"
#include "../utils/config.h"
#include "../utils/crypto.h"
#include "../utils/path.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
    char *plaintext = (char *) malloc(sizeof(char));

    const char *PATH = psm_getenv("PATH");
    const char *GPG_ID = psm_getenv("GPG_ID"); 
    const char *cyphertext;
	const char *file_path;

    size_t wlen;

    int ret_code = -1;

    if (!plaintext || !PATH || !GPG_ID) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: add [FILE]\n");
        goto ret;
    }

    /* Check for trailing slash (i.e. directory) */
    if (args[1][strlen(args[1]) - 1] == '/') {
        printf("Cannot create empty directories\n");
        goto ret;
    }

    printf("Insert password for %s: ", args[1]);

    /* Retrieve plaintext from user */
    if (!(plaintext = read_line_s())) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    printf("\n");

	const char *file_name = add_ext(args[1], "gpg"); 

    if (!file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

	if (!(file_path = build_path(PATH, file_name))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    } 

    /* Encrypt plaintext */
    if (!(cyphertext = gpg_encrypt(plaintext, GPG_ID))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    FILE *file = fopen(file_path, "w");

    if (!file) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Write cyphertext to file */
    if ((wlen = fwrite(cyphertext, sizeof(char), strlen(cyphertext), file)) < 0) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret; 
    }

    ret_code = 0;

ret:
	if (file_path) free((char *) file_path);
    if (file_name) free((char *) file_name);
    if (GPG_ID) free((char *) GPG_ID);
    if (file) fclose(file);
    if (PATH) free((char *) PATH);
    if (plaintext) free(plaintext);
    if (cyphertext) free((char *) cyphertext);
    return ret_code;
}
