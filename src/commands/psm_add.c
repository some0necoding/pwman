#include "./psm_add.h"
#include "../utils/input.h"
#include "../utils/config.h"
#include "../utils/crypto.h"
#include "../utils/path.h"

#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


const char *get_dirname(const char *path);


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
	const char *file_path = NULL;
	const char *file_name = NULL;
    const char *PATH = NULL;
    const char *GPG_ID = NULL; 
    const char *cyphertext = NULL;
	const char *dir_name;

	char *plaintext = (char *) malloc(sizeof(char));

    FILE *file = NULL;
    
	size_t wlen;

    int ret_code = -1;
    
	PATH = psm_getenv("PATH");
    GPG_ID = psm_getenv("GPG_ID"); 

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
		ret_code = 0; 
		goto ret;
    }

    printf("Insert password for %s: ", args[1]);

    /* Retrieve plaintext from user */
    if (!(plaintext = read_line_s())) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    printf("\n");

	file_name = add_ext(args[1], "gpg"); 

    if (!file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

	if (!(file_path = build_path(PATH, file_name))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

	if (!(dir_name = get_dirname(file_path))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	/* Encrypt plaintext */
    if (!(cyphertext = gpg_encrypt(plaintext, GPG_ID))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

	if (psm_mkdir(dir_name) != 0) {
		fprintf(stderr, "psm:%s:%d: cannot create directory %s\n", __FILE__, __LINE__, dir_name);
		goto ret;
	}

	// TODO: move following code to a fio.c function (something like file_write())

    file = fopen(file_path, "w");

    if (!file) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		printf("%d\n", errno);
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
    if (PATH) free((char *) PATH);
    if (GPG_ID) free((char *) GPG_ID);
    if (cyphertext) free((char *) cyphertext);
    if (plaintext) free(plaintext);
    if (file) fclose(file);
    return ret_code;
}

const char *get_dirname(const char *path)
{
	char *path_copy = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	char *tmp_dir_name = NULL;
	char *dir_name = NULL;
	char *ret_val = NULL;

	if (!path_copy) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	strcpy(path_copy, path);

	if (!(tmp_dir_name = dirname(path_copy))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	dir_name = (char *) malloc(sizeof(char) * (strlen(tmp_dir_name) + 1));

	if (!dir_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	strcpy(dir_name, tmp_dir_name);

	ret_val = dir_name;

ret:
	if (path_copy) free(path_copy);
	return ret_val;
}
