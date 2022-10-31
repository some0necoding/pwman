#include "../headers/stdioplusplus.h"
#include "../headers/array_handling.h"

#include <stdlib.h>
#include <string.h>
#include <pwd.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define CONFIGS "/home/marco/coding/c/projects/pwman/test/config.test"  // meant for testing: default = "/etc/pwman.conf"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

char *build_pair(char *key, char *value);
int startswith(char *str, char *substr);
char *get_value(char *pair);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

// this function returns configuration file path
// (i.e. /etc/pwman.conf)
char *get_config_path() 
{
    int const_len = strlen(CONFIGS);

    // path length is malloc'd as const_len + 1
    char *path = malloc(sizeof(char) * (const_len + 1));

    // allocation check
    if (check_allocation(path) != 0) return NULL;

    // initializing the array to null in order
    // to prevent undefined behaviour
    memset(path, '\0', const_len + 1);

    // copying CONFIGS to path
    strcpy(path, CONFIGS);
    return path;
}

// this function adds an environment variable
// in the format key=value inside config file
int add_env_var(char *key, char *value) 
{
    char *pair;

    int wlen;                       // bytes written 
    int pair_len;
    int ret_code = -1;

    FILE *file = fopen(CONFIGS, "a");

    if (!file) {
        perror("psm: I/O error");
        return -1;
    }

    // building a pair out of key and value in the
    // format "key=value"
    if (!(pair = build_pair(key, value))) {
        perror("psm: allocation error");
        fclose(file);
        return -1;
    }

    pair_len = strlen(pair);

    // writing the pair to file
    if ((wlen = fwrite(pair, sizeof(char), pair_len, file)) < 0 ) {
        perror("psm: I/O error");
        goto ret;
    }

ret:
    free(pair);
    fclose(file);
    return 0;
}

// returns the value mapped by key in the file
// CONFIGS
char *get_env_var(char *key) 
{
    int rlen;                                   // bytes read
    char *pair = malloc(sizeof(char));
    char *value;

    FILE *file = fopen(CONFIGS, "r");
    
    if (!file) {
        perror("psm: I/O error\n");
        return NULL;
    }
    
    // reading the file line by line until the key matches
    while (!(rlen = freadline(file, &pair, sizeof(*pair)) < 0) && ((startswith(pair, key)) != 0));

    if (rlen == EOF || strcmp(pair, "") == 0) {     // match not found
        printf("psm: environment variable %s not found\n", key); 
        return NULL;
    } else if (rlen < 0) {                          // an error occured
        perror("psm: I/O error");
        return NULL;
    } else {                                        // match found
        char *value = get_value(pair);
        if (check_allocation(value) != 0) return NULL;
        return value;
    }

    return NULL;
}

// building a pair out of key and value
// in the format "key=value"
char *build_pair(char *key, char *value)
{
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);
    size_t pair_len = key_len + 1 + value_len; 

    char *pair = malloc(sizeof(char) * (pair_len + 2));

    // allocation check
    if (check_allocation(pair) != 0) return NULL;

    // initializing the array to null in order
    // to prevent undefined behaviour
    memset(pair, '\0', pair_len + 2);

    strcat(pair, key);
    strcat(pair, "=");
    strcat(pair, value);
    strcat(pair, "\n");

    return pair;
}

// checks if a string starts with prefix substr
int startswith(char *str, char *substr) 
{
    int str_len = strlen(str);
    int substr_len = strlen(substr);

    // overflow checking
    if (str_len < substr_len) {
        perror("psm: overflow");
        return -1;
    }

    for (int i=0; i < substr_len; i++) {
        if (str[i] != substr[i]) {
            return -1;
        }
    }

    return 0;
}

// returns the value contained in a pair
char *get_value(char *pair) 
{
    char *key;
    const char *delim = "=";

    key = strtok(pair, delim);

    if (key) {
        
        char *val = strtok(NULL, delim);
        
        if (check_allocation(val) != 0) return NULL;
        
        int val_len = strlen(val);
        char *value = malloc(sizeof(*value) * (val_len + 1));

        if (check_allocation(value) != 0) return NULL;

        strcpy(value, val);

        free(key);
        return value;
    }

    free(key);
    return NULL;
}