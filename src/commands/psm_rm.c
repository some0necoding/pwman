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
    char *rel_path = NULL;
    const char *PATH = get_env_var("PATH");

    int ret_code = -1;

    /* Check allocation */
    if (!PATH) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: rm [FILE]\n");
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

    /* Ask for confirm */
    printf("Do you really want to permanently delete \"%s\" [y/N]:", PATH);
    int userInput = getchar();

    /* Delete file */
    if (userInput == 'y' || userInput == 'Y') {
        if (remove(PATH) != 0) {
            perror("psm: I/O error");
            goto ret;
        }
    }

    ret_code = 0;

ret:
    rel_path ? free(rel_path) : 0;
    PATH ? free((char *) PATH) : 0;
    return ret_code;
}
