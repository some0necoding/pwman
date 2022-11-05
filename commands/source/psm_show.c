#include "../../utils/headers/config.h"

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ftw.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

int build_path(char **root, char *rel_path);
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
char *get_last_dir(const char *path);
char *remove_ext(const char *fname, const char *ext); 

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function lists all directories and files under .pwstore.

    Arguments:
        [DIR]     lists all directories and files under .pwstore/dir if dir exists
        [FILE]    lists file under .pwstore if it exists
*/ 
int psm_show(char **args)
{
    char *path = get_env_var("PATH");                                   // /home/{user}/.pwstore     

    if (!path) {
        perror("psm: allocation error\n");
    }    

    if (args[1] && (build_path(&path, args[1]) != 0)) {      // /home/{user}/.pwstore/{args[1]}
        perror("psm: ");
    }

    if (nftw(path, print_file, 20, 0) == -1) {
        perror("psm: I/O error");
        return -1;
    }

    free(path);
    return 0;
}

/*
    This function prints a file name with the right indentation
*/
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    const char *last_dir = get_last_dir(path);
    const char *name = remove_ext(last_dir, ".gpg");
    const char *FMT_TOK = "|--";

    if (ftwbuf->level == 0) {
        printf("%s\n", name);
        free((char *) name);
        free((char *) last_dir);
        return 0;
    }

    size_t indent = ftwbuf->level * 4;
    size_t fmt_name_len = strlen(name) + indent;
    size_t fmt_offset = indent - strlen(FMT_TOK);

    char *fmt_name = calloc(fmt_name_len + 1, sizeof(char));

    if (!fmt_name) {
        perror("psm: allocation error\n");
        free((char *) name);
        free((char *) last_dir);
        return -1;
    }

    memset(fmt_name, ' ', fmt_name_len);

    strcpy(fmt_name+fmt_offset, FMT_TOK);
    strcpy(fmt_name+indent, name);

    printf("%s\n", fmt_name);

    free(fmt_name);
    free((char *) name);
    free((char *) last_dir);
    return 0;
}

/*
    This function returns the last token of a path.

    Example: /foo/boo -> boo
*/
char *get_last_dir(const char *path) 
{
    size_t path_len = strlen(path);
    char *new_path = calloc(path_len + 1, sizeof(char));
    
    if (strchr(path, '/')) {
    
        int last_index = 1;
        
        for (int i = (path_len - 1); (i >= 1 && last_index == 1); i--) {
            if (path[i] == '/') {
                last_index = i;
            }
        }

        new_path = realloc(new_path, sizeof(char) * ((path_len - 1) - last_index + 1));

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

    *root = realloc(*root, sizeof(char) * (root_size + 1 + rel_path_size + 1));     // {root}/{rel_path}

    if (!*root) {
        perror("psm: allocation error\n");
        return -1;
    }

    strcat(*root, "/");
    strcat(*root, rel_path);

    return 0;
}

/*
    This function returns name after removing ext suffix
    if present

    Example: remove_ext("file_name.gpg", ".gpg") -> "file_name"
*/
char *remove_ext(const char *name, const char *ext) 
{
    int ext_len = strlen(ext);
    int name_len = strlen(name);
    int no_ext_len = name_len - ext_len;

    char *no_ext_name = NULL;

    if ((no_ext_len >= 0) && (strcmp(name+no_ext_len, ext) == 0)) {
        
        no_ext_name = calloc(no_ext_len + 1, sizeof(char));

        if (!no_ext_name) {
            perror("psm: allocation error\n");
            return NULL;
        }

        memset((char *) name+no_ext_len, '\0', ext_len);
        strcpy(no_ext_name, name);
    
    } else {
        
        no_ext_name = calloc(name_len + 1, sizeof(char));

        if (!no_ext_name) {
            perror("psm: allocation error\n");
            return NULL;
        } 
        
        strcpy(no_ext_name, name);
    }

    return no_ext_name;
}