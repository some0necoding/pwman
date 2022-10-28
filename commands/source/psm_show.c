#include "../../utils/headers/config.h"
#include "../../utils/headers/array_handling.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

/*----------CONSTANTS-DEFINITION-START----------*/

char *PATH;         // local variable PATH

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

// every file has a path, a name and 
// an indentation level.
typedef struct {
    char *path;
    char *name;
    int indentation;
} file_name;

/*-------------GLOBAL-VARIABLES-END-------------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int list(char *path);
file_name *get_subdirs(char *path, int *subdirs_qty);
int list_rec(char *path);
int list_file(char *path);
int is_dir(char *path);
int is_file(char *path);
char *build_path(char *root, char *rel_path);
char *get_last_dir(char *path);
int print_file(char *name, int indentation);
int get_indentation_level(char *sub_path);
int get_tok_num(char *path); 

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function lists all directories and files under .pwstore.

    Arguments:
        [DIR]     lists all directories and files under .pwstore/dir if dir exists
        [FILE]    lists file under .pwstore if it exists
*/ 
int psm_show(char **args)
{
    char *path = get_env_var("PATH");          // /home/{user}/.pwstore     

    if (!path) return -1;

    if (args[1]) {
        path = build_path(path, args[1]);      // /home/{user}/.pwstore/{args[1]}
    }

    // environment path is stored inside PATH variable
    // with additional subdirectories if provided by user 
    size_t path_len = strlen(path);
    PATH = malloc(sizeof(char) * (path_len + 1));
    strcpy(PATH, path); 

    // listing subdirectories and files under PATH 
    list(PATH);

    free(path);
    free(PATH);
    return 0;
}

/*
    This function checks if the user is trying to show a file (in this 
    case it is just printed) or a directory (in this case it is scanned
    for subdirs and other files).
*/
int list(char *path) 
{
    // the last dir in the PATH is the root of the tree
    char *root = get_last_dir(path);
    printf("%s\n", root);

    if (is_dir(path) == 0) {            // dirs are scanned for subdirs and files
        list_rec(path);           
    } else if (is_file(path) == 0) {    // files are just printed
        print_file(path, 1);
    } else {                            // otherwise path does not exist
        printf("%s is not in the password store\n", path);
    }

    free(root);
    return 0;
}

/*
    This function recursively scans directories for subdirectories and
    files
*/
int list_rec(char *path) 
{
    int subdirs_qty;
    file_name *subdirs = get_subdirs(path, &subdirs_qty);

    for (int i=0; i < subdirs_qty; i++) {

        file_name subdir = subdirs[i];

        if (is_file(subdir.path) == 0) {
            print_file(subdir.name, subdir.indentation);
        } else if (is_dir(subdir.path) == 0) {
            print_file(subdir.name, subdir.indentation);
            list_rec(subdir.path);
        }
    }

    for (int i = 0; i < subdirs_qty; i++) {
        file_name subdir = subdirs[i];
        free(subdir.path);
    }

    free(subdirs);

    return 0;
}

/*
    This function returns an array of file_name structs 
    representing all path subdirectories and file 
*/
file_name *get_subdirs(char *path, int *subdirs_qty) 
{
    DIR *dir = opendir(path);
    struct dirent *direntp;

    // creating subdirs array
    size_t subdirs_len = 1;
    file_name *subdirs = malloc(sizeof(file_name) * subdirs_len);
    int pos = 0;

    // checking for good opening
    if (!dir) {
        perror("psm: I/O error");
        return NULL;
    }

    // for every subdir create a struct and add it
    // to subdirs array
    while ((direntp = readdir(dir)) != NULL) {
        if ((strcmp(direntp->d_name, ".") != 0) && (strcmp(direntp->d_name, "..") != 0)) {

            // if the array is too small it gets stretched
            if (pos >= subdirs_len) {
                subdirs_len += 1;
                subdirs = realloc(subdirs, sizeof(*subdirs) * subdirs_len);
            }

            file_name subdir;
            subdir.name = direntp->d_name;                      // file or subdir name
            subdir.path = build_path(path, subdir.name);        // file or subdir path

            int indentation_level = get_indentation_level(subdir.path);

            subdir.indentation = indentation_level;             // indentation level for formatting

            subdirs[pos] = subdir;
            pos++;
        }
    }

    // returning also array size
    *subdirs_qty = pos;
    return subdirs;
}

/*
    This function returns the indentation level of the subdir
    based on the distance between tree's root and subdir

    path 1: /foo/boo/loo  -> 3
    path 2: /foo/boo      -> 2

    distance = 3 - 2 = 1

    print_line() output:

    boo
     |--loo
    ____ <- distance = 1
*/
int get_indentation_level(char *sub_path) 
{
    int path_dir_num = get_tok_num(PATH);
    int sub_path_dir_num = get_tok_num(sub_path);

    return sub_path_dir_num - path_dir_num;
}

/*
    This function returns the number of tokens contained 
    in path (i.e. the number of / excluding the trailing 
    one if it exists)

    Example 1: /foo/boo  -> 2
    Example 2: /foo/boo/ -> 2
                       ^
                this is ignored
*/
int get_tok_num(char *path) 
{
    int path_len = strlen(path);
    int tok_num = 0;

    for (int i = 0; i < path_len; i++) {
        if (path[i] == '/' && path[i + 1] != '\0') {
            tok_num++;
        }
    }

    return tok_num;
}

/*
    This function prints a file name with the right indentation
*/
int print_file(char *name, int indentation) 
{
    int name_len = strlen(name);
    int fmt_name_len = name_len + (indentation * 4);
    char *fmt_name = malloc(fmt_name_len + 1);
    char *fmt_tok = "|--";

    for (int i = 0; i < fmt_name_len; i++) {
        fmt_name[i] = ' ';
    }

    int fmt_index = (fmt_name_len - name_len) - 3;
    strcpy(fmt_name+fmt_index, fmt_tok);
    strcpy(fmt_name+(indentation * 4), name);

    printf("%s\n", fmt_name);
    
    free(fmt_name);
    return 0;
}

/*
    This function returns the last token of a path

    Example: /foo/boo -> boo
*/
char *get_last_dir(char *path) 
{
    if (path[0] == '/') {

        int path_len = strlen(path);
        int first_index = 1;

        for (int i = path_len; (i >= 1 && first_index == 1); i--) {
            if (path[i] == '/') {
                first_index = (i + 1);
            }
        }

        char *new_path = malloc(sizeof(char) * (path_len - first_index + 1));

        strcpy(new_path, path+first_index);
        return new_path;
    }

    return path;
}

/*
    This function checks if the path is a
    directory
*/
int is_dir(char *path) 
{
    struct stat fstat;

    if (stat(path, &fstat) < 0) {
        return -1;
    }

    if (S_ISDIR(fstat.st_mode)) {
        return 0;
    }

    return -1;
}

/*
    This function checks if the path is a
    file
*/
int is_file(char *path) 
{
    struct stat fstat;

    if (stat(path, &fstat) < 0) {
        return -1;
    }

    if (S_ISREG(fstat.st_mode)) {
        return 0;
    }
    
    return -1;
}

/*
    This function builds a path in the format:
        
        root/{rel_path}
*/
char *build_path(char *root, char *rel_path) 
{
    size_t root_size = strlen(root);
    size_t rel_path_size = strlen(rel_path);

    char *full_path = malloc(sizeof(char) * root_size + 1 + rel_path_size + 1);     // {root}/{rel_path}

    strcpy(full_path, root);
    strcat(full_path, "/");
    strcat(full_path, rel_path);

    return full_path;
}