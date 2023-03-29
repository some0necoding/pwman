#include "./psm_rm.h"
#include "../utils/config.h"
#include "../utils/path.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
    This function removes a file from PATH.

    Arguments:
        [FILE]      this is a relative path which has PATH as root

    Examples:
        rm test_file                stored at PATH/test_file
        rm test_dir/test_file       stored at PATH/test_dir/test_file
*/
int psm_rm(char **args) 
{
    const char *file_name = NULL; 
	const char *file_path = NULL;
    const char *PATH = NULL;
    
	int ret_code = -1;

    PATH = psm_getenv("PATH");
    
	/* Check allocation */
    if (!PATH) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: rm [FILE]\n");
        goto ret;
    }

    file_name = add_ext(args[1], "gpg"); 

    /* Check allocation */
    if (!file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    if (!(file_path = build_path(PATH, file_name))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    } 

    /* Ask for confirm */
    printf("Do you really want to permanently delete \"%s\" [y/N]:", file_path);
    int user_input = getchar();

    /* Delete file */
    if (user_input == 'y' || user_input == 'Y') {
        if (remove(file_path) != 0) {
            fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
            goto ret;
        }
    }

    ret_code = 0;

ret:
    if (file_name) free((char *) file_name);
	if (file_path) free((char *) file_path);
    if (PATH) free((char *) PATH);
    return ret_code;
}
