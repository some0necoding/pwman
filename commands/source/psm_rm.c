#include "../../utils/headers/config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

char *add_ext(char *fname, const char *ext);
int build_path(char **root, char *rel_path);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

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