#include "../../utils/headers/input_acquisition.h"
#include "../../utils/headers/config.h"
#include "../../utils/headers/cryptography.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/  

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int build_path(char **root, char *rel_path); 

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*------------GLOBAL-VARIABLES-START------------*/

/*-------------GLOBAL-VARIABLES-END-------------*/

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
    /*
        Syntax:
            add [FILE] 
        
        What the fuck do we have to do?

            1. check that args[1] exists
            2. check args[1] validity
            3. ask for password using secure input
            4. encrypt password with gpg public key which id will be stored in/etc/pwman.con
            5. create file at FILE location
            6. write encrypted password to file
    */

    char *password_plain = calloc(1, sizeof(char));
    char *path = get_env_var("PATH");

    size_t wlen;
    size_t last_index;

    if (!password_plain) {
        perror("psm: allocation error\n");
        return -1;
    }

    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: add [FILE]\n");
        return -1;
    }

    last_index = strlen(args[1]) - 1;

    if (args[1][last_index] == '/') {
        printf("Cannot create empty directories\n");
        return -1;
    }

    printf("Insert password for %s: ", args[1]);

    if (read_line_s(&password_plain, 1) != 0) {
        perror("psm: allocation error\n");
        return -1;
    } 

    if (build_path(&path, args[1]) != 0) {
        perror("psm: allocation error\n");
        return -1;
    } 

    const char *GPG_ID = get_env_var("GPG_ID"); 
    const char *password_enc;

    if (!(password_enc = gpg_encrypt(password_plain, GPG_ID))) {
        perror("psm: cryptography error\n");
        return -1;
    }

    FILE *file = fopen(path, "w");

    if (!file) {
        perror("psm: I/O error");
        return -1;
    }

    if ((wlen = fwrite(password_enc, sizeof(char), strlen(password_enc), file)) < 0) {
        perror("psm: I/O error");
        return -1;
    }

    return 0;
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