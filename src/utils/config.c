#include "./fio.h"
#include "./config.h"
#include "./path.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#define CONFIG_PATH ".config/pwman.conf"


const char *build_pair(const char *key, const char *value);
const char *get_value(char *pair);


/*
    This function returns configuration file path 
	(i.e. /home/$USER/.config/pwman.conf).
*/
const char *get_config_path() 
{
	const char *home = get_home();
	
	if (!home) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}
  
	const char *path = build_path(home, CONFIG_PATH);

	if (home) free((char *) home);
    return path;
}


/*
	Get home directory of current user.
*/
const char *get_home() 
{
	const char *tmp_home = getenv("HOME");	

	if (!tmp_home) {
		fprintf(stderr, "cannot find $HOME environment variable");
		return NULL;
	}

	char *home = (char *) malloc(sizeof(char) * (strlen(tmp_home) + 1));

	if (!home) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(home, tmp_home);

	return home;
}


/*
    This function add an environment variable in
    the format "key=value" in the file stored at
    CONFIG_PATH.
*/
int psm_putenv(const char *key, const char *value) 
{
	int ret_code = -1;

	const char *config_file = get_config_path();

	if (!config_file) {
		fprintf(stderr, "cannot define path of configuration file\n");
		goto ret;
	}

    size_t wlen;
    size_t pair_len;

    FILE *file = fopen(config_file, "a");

    if (!file) {
		fprintf(stderr, "cannot open %s\n", config_file);
        goto ret;
    }

    const char *pair = build_pair(key, value);

	if (!pair) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

    // writes pair into file
    if ((wlen = fwrite(pair, sizeof(char), strlen(pair), file)) < 0 ) {
		fprintf(stderr, "cannot write to %s\n", config_file);
		goto ret;
    }

	ret_code = 0;

ret:
	if (file) fclose(file);
	if (pair) free((char *) pair);
	if (config_file) free((char *) config_file);
    return ret_code;
}


/*
    This function returns the value mapped by
    key in the file stored at CONFIG_PATH.
*/
const char *psm_getenv(const char *key) 
{
    const char *config_file = get_config_path();
    char *pair = (char *) malloc(sizeof(char));

    if (!config_file || !pair) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    FILE *file = fopen(config_file, "r");
    
    if (!file) {
        fprintf(stderr, "cannot open %s\n", config_file);
        goto ret;
    }
    
    // reads the file line by line until the key matches
    while ((pair = freadline(file)) && (strncmp(pair, key, strlen(key))) != 0);

	if (!pair) {							// ERROR
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
	} else if (strcmp(pair, "") == 0) {		// NOT FOUND
        fprintf(stderr, "psm:%s:%d: environment variable %s not found\n", __FILE__, __LINE__, key); 
        goto ret; 
    }

	// else if FOUND
	
	const char *value = get_value(pair);
        
    if (!value) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }
        
ret:
    if (config_file) free((char *) config_file);
    if (pair) free(pair);
    if (file) fclose(file);
    return value;
}


/*
    This function returns a string containing
    a pair of format "key=value".
*/
const char *build_pair(const char *key, const char *value)
{
    char *pair = (char *) malloc(sizeof(char) * (strlen(key) + 1 + strlen(value) + 2)); 

	if (!pair) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

	sprintf(pair, "%s=%s\n", key, value);

    return pair;
}


/*
    This function returns the string following 
	the "=" inside a pair.
*/
const char *get_value(char *pair) 
{
    char *key = strtok(pair, "=");

    if (key) {
        
        const char *val = strtok(NULL, "=");

        if (!val) {
            fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
            return NULL;
        } 
        
		char *value = (char *) malloc(sizeof(char *) * (strlen(val) + 1));

        if (!value) {
            fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
            return NULL;
        }

        strcpy(value, val);
        return value;
    }

    return NULL;
}
