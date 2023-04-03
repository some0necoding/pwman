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


#define BUFSIZE 32


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
	const char *PATH = psm_getenv("PATH");
    const char *GPG_ID = psm_getenv("GPG_ID");
    const char *file_name = add_ext(args[1], "gpg");
	const char *file_path = NULL;
    
	char *plaintext = (char *) malloc(BUFSIZE);

    int ret_code = -1;

    pthread_t thread_id;

	if (!PATH || !GPG_ID || !file_name || !plaintext) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: get [FILE]\n");
        goto ret;
    }

    if (!(file_path = build_path(PATH, file_name))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Check for file existance */
    if (access(file_path, F_OK) != 0) {
        printf("No such file or directory\n");
        ret_code = 0;
        goto ret;
    }

    int err = gpg_decrypt_file(file_path, GPG_ID, &plaintext, BUFSIZE);

    if (err == 0) {
        printf("Incorrect passphrase\n");
        ret_code = 0;
        goto ret;
    } else if (err == -1) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Create a thread managing clipboard */
    if (pthread_create(&thread_id, NULL, (void *) save_in_clipboard, plaintext) != 0) {
        fprintf(stderr, "psm:%s:%d: error in thread creation\n", __FILE__, __LINE__);
        goto ret;
    }

    ret_code = 0;

ret:
    if (PATH) free((char *) PATH);
    if (GPG_ID) free((char *) GPG_ID);
    if (file_name) free((char *) file_name);
    if (file_path) free((char *) file_path);
    return ret_code;  
}
