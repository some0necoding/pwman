#include "./utils/config.h"
#include "./utils/crypto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

#define CONFIG_PATH "$HOME/.config/pwman.conf"

/* ---------- FUNCTIONS DEFINITION START ---------- */

int setup();
char *get_path();
char *get_gpg_id();

/* ----------- FUNCTIONS DEFINITION END ----------- */


/*
	To check if X11 is running:

	if (!getenv("DISPLAY")) {
		fprintf(stderr, "missing X11 display, have you forgotten something?\n");
		exit(1);
	}

*/


int main(int argc, char const *argv[]) 
{
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
    char *gpg_id = NULL;
    const char *config_file = get_config_path();

    int ret_code = -1;

    if (!config_file) {
        perror("psm: allocation error");
        goto ret;
    }

    /* Set up if CONFIG_FILE does no exist */
    if (access(config_file, F_OK) == 0) {
        printf("pwman has already been initialized: %s file exists\n", config_file);
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

    /* Create directory in path location with 700 permissions if it does not exist */
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
	if (path) free(path);
	if (config_file) free((char *) config_file);
    if (gpg_id) free(gpg_id);
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

    int user_input;
    int pos = 0;

    /* Check allocation */
    if (!keys) {
        fprintf(stderr, "psm: allocation error\n");
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
    scanf("%d", &user_input);

	char *ret_key = (char *) malloc(sizeof(char *) * (strlen(keys[user_input - 1]->fpr) + 1));
	strcpy(ret_key, keys[user_input - 1]->fpr);

	free(keys);

    /* Returning key fingerprint */
    return ret_key;
}
