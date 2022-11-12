#include "../../utils/headers/config.h"

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ftw.h>
#include <errno.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

int build_path(char **root, char *rel_path);
int remove_ext(char **fname, const char *ext); 
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
char *get_last_dir(const char *path);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function lists all directories and files under .pwstore.

    Arguments:
        [DIR]     lists all directories and files under .pwstore/dir if dir exists
        [FILE]    lists file under .pwstore if it exists
*/ 
int psm_show(char **args)
{
    char *PATH = get_env_var("PATH");
    int ret_code = -1;

    if (!PATH) {
        perror("psm: allocation error\n");
        goto ret;
    }    

    if (args[1] && (build_path(&PATH, args[1]) != 0)) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Recursively call print_file on all directories and files */
    if (nftw(PATH, print_file, 20, 0) == -1) {
        if (errno == 2) {
            printf("No such file or directory\n");
        } else {
            perror("psm: I/O error");
        }

        goto ret;
    }

    ret_code = 0;

ret:
    PATH ? free(PATH) : 0;
    return ret_code;
}

/*
    This function prints a file name with the right indentation
*/
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    const char *FMT_TOK = "|--";
    char *last_dir = get_last_dir(path);

    int ret_code = -1;

    if (!last_dir) {
        perror("psm: allocation error");
        goto ret;
    }
    
    remove_ext(&last_dir, ".gpg");

    /* If indentation level equals 0 */ 
    if (ftwbuf->level == 0) {
        printf("%s\n", last_dir);
        ret_code = 0;
        goto ret;
    }

    /* Every indentation level equals 4 spaces */
    size_t indent = ftwbuf->level * 4;
    size_t fmt_name_len = strlen(last_dir) + indent;
    size_t fmt_offset = indent - strlen(FMT_TOK);

    char *fmt_name = calloc(fmt_name_len + 1, sizeof(char));

    if (!fmt_name) {
        perror("psm: allocation error\n");
        goto ret; 
    }

    /* Set all string to whitespaces */
    memset(fmt_name, ' ', fmt_name_len);

    strcpy(fmt_name+fmt_offset, FMT_TOK);
    strcpy(fmt_name+indent, last_dir);

    printf("%s\n", fmt_name);

    ret_code = 0;

ret:
    fmt_name ? free(fmt_name) : 0;
    last_dir ? free((char *) last_dir) : 0;
    return ret_code;
}

/*
    This function returns the last token of a path.

    Example: /foo/boo -> boo
*/
char *get_last_dir(const char *path) 
{
    size_t path_len = strlen(path);
    char *new_path = calloc(path_len + 1, sizeof(char));

    /* Check if path contains at least one '/' */ 
    if (strchr(path, '/')) {
    
        int last_index = 0;

        /* Check path backwards until a '/' is found */ 
        for (int i = (path_len - 1); (i >= 1 && last_index == 0); i--) {
            if (path[i] == '/') {
                last_index = i;
            }
        }

        new_path = realloc(new_path, sizeof(char) * (path_len - last_index));

        if (!new_path) {
            perror("psm: allocation error\n");
            return NULL;
        }

        strcpy(new_path, path+(last_index + 1));
    } else {
        strcpy(new_path, path);
    }

    return new_path;
}

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
    This function returns name after removing ext suffix
    if present.

    Example: remove_ext("file_name.gpg", ".gpg") -> "file_name"
*/
int remove_ext(char **name, const char *ext) 
{
    int ext_len = strlen(ext);
    int name_len = strlen(*name);
    int no_ext_len = name_len - ext_len;

    if (strcmp(*name+no_ext_len, ".gpg") == 0) {
        name[0][no_ext_len] = '\0';
    }

    return 0;
}