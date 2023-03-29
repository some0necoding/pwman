#include "./utils/config.h"
#include "./utils/crypto.h"
#include "./utils/path.h"

#include <gpgme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>

#define DB_NAME ".pwstore"


int setup();
int check_X11();
const char *get_store_path();
const char *get_gpg_fpr();


int main(int argc, char const *argv[]) 
{
    if (setup() != 0) {
        fprintf(stderr, "psm:%s:%d: setup error", __FILE__, __LINE__);
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
    const char *config_file = NULL;
    const char *store_path = NULL;
    const char *gpg_fpr = NULL;
    
	int ret_code = -1;
	
	if (check_X11() != 0) {
		fprintf(stderr, "missing needed X11 display\n");
		goto ret;
	}

    config_file = get_config_path();

    if (!config_file) {
        fprintf(stderr, "psm:%s:%d: allocation error", __FILE__, __LINE__);
        goto ret;
    }

    /* Set up if CONFIG_FILE does no exist */
    if (access(config_file, F_OK) == 0) {
        printf("pwman has already been initialized: %s file exists\n", config_file);
        ret_code = 0;
        goto ret;
    }

    /* $HOME/.pwstore */
    store_path = get_store_path();

    /* gpg key fingerprint */
    gpg_fpr = get_gpg_fpr();

    /* Check allocation */
    if (!store_path || !gpg_fpr) {
      fprintf(stderr, "psm:%s:%d: allocation error", __FILE__, __LINE__);
      goto ret;
    }

	struct stat st = {0};

    /* Create directory in path location with 700 permissions if it does not exist */
    if ((stat(store_path, &st) == -1) && (mkdir(store_path, 0700) != 0)) {
        fprintf(stderr, "psm:%s:%d: I/O error", __FILE__, __LINE__);
        goto ret;
    }

	/* Add PATH environment variable */
    if (psm_putenv("PATH", store_path) != 0) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

	/* Add GPG_ID environment variable */
    if (psm_putenv("GPG_ID", gpg_fpr) != 0) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

	ret_code = 0;

ret:
	if (store_path) free((char *) store_path);
	if (config_file) free((char *) config_file);
    if (gpg_fpr) free((char *) gpg_fpr);
    return ret_code;
}

/*
	Check if X11 is running
*/
int check_X11() 
{
	if (!getenv("DISPLAY")) 
		return -1;

	return 0;
}

/*
    This function returns PATH in the format 
    "PATH=$HOME/.pwstore".
*/
const char *get_store_path() 
{
	const char *home = get_home();

	if (!home) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}
    
	const char *path = build_path(home, DB_NAME);

	if (!path) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

	if (home) free((char *) home);
    return path;
}

/*
    This function makes user choose betweeen
    available gpg keys for encryption and
    returns its fingerprint.
*/
const char *get_gpg_fpr() 
{
    gpgme_key_t key;

    /* Get available keys */
    gpgme_key_t *keys = gpg_get_keys();

    int user_input;
    int pos = 0;

    /* Check allocation */
    if (!keys) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
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

	if (keys) free(keys);

    /* Returning key fingerprint */
    return ret_key;
}
