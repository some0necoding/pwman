#include "./fio.h"
#include "./config.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#define CONFIG_PATH ".config/pwman.conf"

/*----------FUNCTIONS-DEFINITION-START----------*/

char *build_pair(const char *key, const char *value);
char *get_value(char *pair);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function returns configuration file path (i.e. /etc/pwman.conf)
*/
char *get_config_path() 
{
	char *tmp_home = getenv("HOME");

	if (!tmp_home) {
		fprintf(stderr, "cannot find $HOME environment variable");
		return NULL;
	}

	char *home = (char *) malloc(sizeof(char) * (strlen(tmp_home) + 1));
	strcpy(home, tmp_home);

	if (!home) {
		fprintf(stderr, "psm: allocation error\n");
		return NULL;
	}

    char *path = malloc(sizeof(char) * (strlen(home) + 1 + strlen(CONFIG_PATH) + 1));

	if (!path) {
		fprintf(stderr, "psm: allocation error\n");
		free(home);
		return NULL;
	}

    /* Build $HOME/.config/pwman.conf */
	sprintf(path, "%s/%s", home, CONFIG_PATH);

	free(home);
    return path;
}

/*
    This function add an environment variable in
    the format "key=value" in the file stored at
    CONFIGS path.
*/
int add_env_var(const char *key, const char *value) 
{
	const char *config_file = get_config_path();

	if (!config_file) {
		fprintf(stderr, "cannot get config path\n");
	}

    size_t wlen;
    size_t pair_len;

    FILE *file = fopen(config_file, "a");

    if (!file) {
		fprintf(stderr, "cannot open %s\n", config_file);
        return -1;
    }

    const char *pair = build_pair(key, value);

    // writes pair into file
    if ((wlen = fwrite(pair, sizeof(char), strlen(pair), file)) < 0 ) {
		fprintf(stderr, "cannot write to %s\n", config_file);
        fclose(file);
		free((char *) pair);
		free((char *) config_file);
		return -1;
    }

	fclose(file);
	free((char *) pair);
	free((char *) config_file);
    return 0;
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
char *build_pair(const char *key, const char *value)
{
    char *pair = (char *) malloc(sizeof(char) * (strlen(key) + 1 + strlen(value) + 2)); 

	sprintf(pair, "%s=%s\n", key, value);

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
