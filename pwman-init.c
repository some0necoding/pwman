#include "./utils/headers/config.h"
#include "./utils/headers/crypto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

/*-------------GLOBAL-VARIABLES-END-------------*/

int UID, GID;

/*------------GLOBAL-VARIABLES-START------------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int setup();
char *get_path();
char *get_gpg_id();
char *build_gpg_id(char *id_value);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

int main(int argc, char const *argv[]) 
{
    /* Get current uid and gid */
    UID = getuid();
    GID = getgid();

    /*
        We need to gain root privileges to create
        config files in system directories.

        setuid(0) works only if pwman-init is owned
        by root and set as setuid allowed. This is
        delegated to make utility at build time.
    */
    if (setuid(0) != 0) {
        perror("psm: failed to gain root privileges");
        return -1;
    }

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
    char *id_value = NULL;
    char *gpg_id = NULL;
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
    id_value = get_gpg_id();

    /* Check allocation */
    if (!path || !id_value) {
      perror("psm: allocation error");
      goto ret;
    }

    /* Create file in path location with 700 permissions */
    if (mknod(path, S_IFREG|S_IRWXU, 0) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Set ownership to UID (not root) */
    if (chown(path, UID, GID) != 0) {
        perror("psm: error setting ownership");
        goto ret;
    }

    /* "GPG_ID=id_value" pair */
    gpg_id = build_gpg_id(id_value);

    /* Check allocation */
    if (!gpg_id) {
      perror("psm: allocation error");
      goto ret;
    }

    /* path + \n + gpg_id + \0 */
    buffer = calloc(strlen(path) + 1 + strlen(gpg_id) + 1, sizeof(char));

    /* Check allocation */
    if (!buffer) {
      perror("psm: allocation error");
      goto ret;
    }

    strcpy(buffer, path);
    strcat(buffer, "\n");
    strcat(buffer, gpg_id);

    /* Change permissions to 644 */
    if (chmod(CONFIG_FILE, S_IRWXU|S_IRGRP|S_IROTH) != 0) {
        perror("psm: error setting permissions");
        goto ret;
    }

    file = fopen(CONFIG_FILE, "w");
    size_t wlen;

    /* Check opening */
    if (!file) {
      perror("psm: I/O error");
      goto ret;
    }

    /* Write buffer to CONFIG_FILE */
    if ((wlen = fwrite(buffer, sizeof(char), strlen(buffer), file)) < 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    path ? free(path) : 0;
    id_value ? free(id_value) : 0;
    gpg_id ? free(gpg_id) : 0;
    buffer ? free(buffer) : 0;
    file ? fclose(file) : 0;
    return ret_code;
}

/*
    This function returns the PATH pair in the
    format "PATH=$HOME/.pwstore".
*/
char *get_path() 
{
    char *path_value;
    char *path_key = "PATH=";
    char *path_pair;

    /* Get $HOME */
    if ((path_value = getenv("HOME"))) {
        path_value = getpwuid(getuid())->pw_dir;
    } else {
        perror("psm: $HOME not found");
        return NULL;
    }

    /* Build $HOME/.pwstore */
    strcat(path_value, "/.pwstore");

    path_pair = calloc(strlen(path_key) + strlen(path_value) + 1, sizeof(char));

    if (!path_pair) {
        perror("psm: allocation error");
        return NULL;
    }

    /* Build "PATH=$HOME/.pwstore" pair*/
    strcpy(path_pair, path_key);
    strcat(path_pair, path_value);

    return path_pair;
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

/*
    This function returns the GPG_ID pair
    in the format "GPG_ID=id_value".
*/
char *build_gpg_id(char *id_value) 
{
    char *id_key = "GPG_ID=";
    char *id_pair = calloc(strlen(id_key) + strlen(id_value) + 1, sizeof(char));

    /* Check allocation */
    if (!id_pair) {
        perror("psm: allocation error");
        return NULL;
    }

    strcpy(id_pair, id_key);
    strcat(id_pair, id_value);

    return id_pair;
}