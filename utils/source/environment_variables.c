#include <stdio.h>
#include <string.h>
#include <pwd.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE 64  
#define CONFIGS "/etc/pwman.conf"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

char *build_pair(char *key, char *value);
int startswith(char *str, char *substr);
char *get_value(char *pair);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

int add_env_var(char *key, char *value) 
{
    int wlen;
    char *pair = build_pair(key, value);
    int pair_len = strlen(pair);

    FILE *file = fopen(CONFIGS, "w");

    if ((wlen = fwrite(pair, sizeof(char), pair_len, file)) < 0 ) {
        perror("psm: I/O error");
        return -1;
    }

    fclose(file);
    return 0;
}

char *build_pair(char *key, char *value)
{
    int key_len = strlen(key);
    int value_len = strlen(value);

    char *pair = (char *) malloc(key_len + 1 + value_len);

    strcat(pair, key);
    strcat(pair, "=");
    strcat(pair, value);

    return pair;
}

char *get_env_var(char *key) 
{
    int rlen;
    char *pair = malloc(BUFSIZE);
    char *value;

    FILE *file = fopen(CONFIGS, "r");

    while (!(rlen = freadline(file, pair, BUFSIZE) < 0) && ((startswith(pair, key)) != 0));

    if (rlen < 0) {
        perror("psm: I/O error");
        return NULL;
    } else if (!(value = get_value(pair))) {
        perror("psm: ");
        return NULL;
    } else {
        return value;
    }

    return NULL;
}

int startswith(char *str, char *substr) 
{
    int str_len = strlen(str);
    int substr_len = strlen(substr);

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

char *get_value(char *pair) 
{
    char *key;
    char *value;
    const char *delim = "=";

    key = strtok(pair, delim);

    if (key != NULL) {
        value = strtok(NULL, delim);
    }

    return value;
}