#include "../headers/path.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    This function builds a path in the format:
        
        root/{rel_path}
*/
int build_path(char **root, char *rel_path) 
{
    size_t root_size = strlen(*root);
    size_t rel_path_size = strlen(rel_path);

    /* Allocate {root}/{rel_path} */
    *root = realloc(*root, sizeof(char) * (root_size + 1 + rel_path_size + 1));     // {root}/{rel_path}

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