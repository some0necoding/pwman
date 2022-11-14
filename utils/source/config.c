#include "../headers/fio.h"
#include "../headers/config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

char *build_pair(char *key, char *value);
char *get_value(char *pair);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function returns configuration file path (i.e. /etc/pwman.conf)
*/
char *get_config_path() 
{
    char *home_dir;
    char *config_path = ".config/pwman.conf";
    char *path;

    /* Get $HOME */
    if ((home_dir = getenv("HOME"))) {
        home_dir = getpwuid(getuid())->pw_dir;
    } else {
        perror("psm: $HOME not found");
        return NULL;
    }

    path = calloc(strlen(home_dir) + 1 + strlen(config_path) + 1, sizeof(char));

    if (!path) {
        perror("psm: allocation error");
        return NULL;
    }

    /* Build $HOME/.pwstore */
    strcpy(path, home_dir);
    strcat(path, "/");
    strcat(path, config_path);

    return path;
}

/*
    This function add an environment variable in
    the format "key=value" in the file stored at
    CONFIGS path.
*/
int add_env_var(char *key, char *value) 
{
    char *pair = NULL;
    char *config_file = get_config_path();

    size_t wlen;
    size_t pair_len;
    int ret_code = -1;

    FILE *file = NULL;

    if (!config_file) {
        perror("psm: allocation error");
        goto ret;
    } 
    
    file = fopen(config_file, "a");

    if (!file) {
        perror("psm: I/O error");
        goto ret;
    }

    // builds a pair in the format "key=value"
    if (!(pair = build_pair(key, value))) {
        perror("psm: allocation error");
        goto ret;
    }

    pair_len = strlen(pair);

    // writes pair into file
    if ((wlen = fwrite(pair, sizeof(char), pair_len, file)) < 0 ) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    pair ? free(pair) : 0;
    config_file ? free(config_file) : 0;
    file ? fclose(file) : 0;
    return ret_code;
}

/*
    This function returns the value mapped by
    key in the file stored at CONFIGS path.
*/
char *get_env_var(char *key) 
{
    int rlen;
    char *config_file = get_config_path();
    char *pair = calloc(1, sizeof(char));
    char *value = NULL;

    FILE *file = NULL;
    
    if (!config_file || !pair) {
        perror("psm: allocation error");
        goto ret;
    }

    file = fopen(config_file, "r");
    
    if (!file) {
        perror("psm: I/O error\n");
        goto ret;
    }
    
    // reads the file line by line until the key matches
    while (!(rlen = freadline(file, &pair, sizeof(*pair)) < 0) && ((strncmp(pair, key, strlen(key))) != 0));

    if (rlen == EOF || strcmp(pair, "") == 0) {      // match not found
        printf("psm: environment variable %s not found\n", key); 
        goto ret; 
    } else if (rlen < 0) {                                  // an error occured
        perror("psm: I/O error");
        goto ret;
    } else {                                                // match found
        value = get_value(pair);
        
        if (!value) {
            perror("psm: allocation error\n");
            goto ret;
        }
        
        goto ret;
    }

ret:
    config_file ? free(config_file) : 0;
    pair ? free(pair) : 0;
    file ? fclose(file) : 0;
    return value;
}

/*
    This function returns a string containing
    a pair of format "key=value"
*/
char *build_pair(char *key, char *value)
{
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);
    size_t pair_len = key_len + 1 + value_len + 1;      // key + = + value + \n

    char *pair = calloc(pair_len + 1, sizeof(char));

    if (!pair) {
        perror("psm: allocation error\n");
        return NULL;
    }

    strcat(pair, key);
    strcat(pair, "=");
    strcat(pair, value);
    strcat(pair, "\n");

    return pair;
}

/*
    This function returns the string following the
    "=" inside a pair
*/
char *get_value(char *pair) 
{
    char *key;
    const char *delim = "=";

    key = strtok(pair, delim);

    if (key) {
        
        char *val = strtok(NULL, delim);

        if (!val) {
            perror("psm: allocation error\n");
            return NULL;
        } 
        
        size_t val_len = strlen(val);
        char *value = calloc(val_len + 1, sizeof(char));

        if (!value) {
            perror("psm: allocation error\n");
            return NULL;
        }

        strcpy(value, val);
        return value;
    }

    return NULL;
}