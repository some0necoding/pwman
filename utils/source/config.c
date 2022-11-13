#include "../headers/fio.h"
#include "../headers/config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

/*----------FUNCTIONS-DEFINITION-START----------*/

char *build_pair(char *key, char *value);
char *get_value(char *pair);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function returns configuration file path (i.e. /etc/pwman.conf)
*/
char *get_config_path() 
{
    size_t const_len = strlen(CONFIG_FILE);
    char *path = calloc(const_len + 1, sizeof(char));

    if (!path) {
        perror("psm: allocation error\n");
        return NULL;
    }

    strcpy(path, CONFIG_FILE);
    return path;
}

/*
    This function add an environment variable in
    the format "key=value" in the file stored at
    CONFIGS path.
*/
int add_env_var(char *key, char *value) 
{
    char *pair;

    size_t wlen;
    size_t pair_len;
    int ret_code = -1;

    FILE *file = fopen(CONFIG_FILE, "a");

    if (!file) {
        perror("psm: I/O error");
        return -1;
    }

    // builds a pair in the format "key=value"
    if (!(pair = build_pair(key, value))) {
        perror("psm: allocation error");
        fclose(file);
        return -1;
    }

    pair_len = strlen(pair);

    // writes pair into file
    if ((wlen = fwrite(pair, sizeof(char), pair_len, file)) < 0 ) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    free(pair);
    fclose(file);
    return ret_code;
}

/*
    This function returns the value mapped by
    key in the file stored at CONFIGS path.
*/
char *get_env_var(char *key) 
{
    int rlen;
    char *pair = calloc(1, sizeof(char));
    char *value;

    FILE *file = fopen(CONFIG_FILE, "r");
    
    if (!file) {
        perror("psm: I/O error\n");
        return NULL;
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
        char *value = get_value(pair);
        
        if (!value) {
            perror("psm: allocation error\n");
            goto ret;
        }
        
        fclose(file); 
        return value;
    }

ret:
    fclose(file);
    return NULL;
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
        free(key);
        return value;
    }

    free(key);
    return NULL;
}