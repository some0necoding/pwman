#include "./utils/headers/config.h"
#include "./utils/headers/crypto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

/*-------------GLOBAL-VARIABLES-END-------------*/

int UID, GID;

/*------------GLOBAL-VARIABLES-START------------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int setup();
char *get_path();
char *get_gpg_id();

/*-----------FUNCTIONS-DEFINITION-END-----------*/

int main(int argc, char const *argv[]) 
{
    UID = getuid();
    GID = getgid();

    if (setup() != 0) {
        perror("psm: setup error");
        return -1;
    }

    return 0;
}

/*
    This function sets up a config file containing path 
    to password store directory and gpg key fingerprint 
    that will be used.
*/
int setup()
{
    char *path = NULL;
    char *path_pair = NULL;
    char *gpg_id = NULL;
    char *id_pair = NULL;
    char *buffer = NULL;

    FILE *file = NULL;

    int ret_code = -1;

    /* Set up if CONFIG_FILE does no exist */
    if (access(CONFIG_FILE, F_OK) == 0) {
        printf("pwman has already been initialized: %s file exists\n", CONFIG_FILE);
        ret_code = 0;
        goto ret;
    }

    /* $HOME/.pwstore */
    path = get_path();

    /* gpg key fingerprint */
    gpg_id = get_gpg_id();

    /* Check allocation */
    if (!path || !gpg_id) {
      perror("psm: allocation error");
      goto ret;
    }

    struct stat st = {0};

    /* Create directory in path location with 700 permissions */
    if ((stat(path, &st) == -1) && (mkdir(path, 0700) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Add PATH environment variable */
    if (add_env_var("PATH", path) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Add GPG_ID environment variable */
    if (add_env_var("GPG_ID", gpg_id) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    path ? free(path) : 0;
    path_pair ? free(path_pair) : 0;
    gpg_id ? free(gpg_id) : 0;
    id_pair ? free(id_pair) : 0;
    buffer ? free(buffer) : 0;
    file ? fclose(file) : 0;
    return ret_code;
}

/*
    This function returns PATH in the format 
    "PATH=$HOME/.pwstore".
*/
char *get_path() 
{
    char *home_dir;
    char *db_name = ".pwstore";
    char *path;

    /* Get $HOME */
    if ((home_dir = getenv("HOME"))) {
        home_dir = getpwuid(getuid())->pw_dir;
    } else {
        perror("psm: $HOME not found");
        return NULL;
    }

    path = calloc(strlen(home_dir) + 1 + strlen(db_name) + 1, sizeof(char));

    if (!path) {
        perror("psm: allocation error");
        return NULL;
    }

    /* Build $HOME/.pwstore */
    strcpy(path, home_dir);
    strcat(path, "/");
    strcat(path, db_name);

    return path;
}

/*
    This function makes user choose betweeen
    available gpg keys for encryption and
    returns its fingerprint.
*/
char *get_gpg_id() 
{
    gpgme_key_t key;

    /* Get available keys */
    gpgme_key_t *keys = gpg_get_keys();

    int userInput;
    int pos = 0;

    /* TODO: keys=NULL could also mean "no keys found" */
    if (!keys) {
        perror("psm: allocation error");
        return NULL;
    }

    /* List secret keys */
    while ((key = keys[pos])) {

        char *fpr = key->fpr;
        char *uid = key->uids->uid;

        printf("%d. ID: %s\n"
                        "\t%s\n", (pos + 1), fpr, uid);         

        pos++;
    }

    printf("Choose the gpg key you want to use to encrypt your passwords: ");
    scanf("%d", &userInput);

    /* Returning key fingerprint */
    return keys[userInput - 1]->fpr;
}