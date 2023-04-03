#include "config.h"
#include "crypto.h"
#include "path.h"

#include <gpgme.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <pwd.h>
#include <sys/stat.h>
//#include <errno.h>
//#include <sys/types.h>


#define BUFSIZE 16

int check_X11();
const char *get_gpg_fpr();


/*
    This function sets up a config file containing path 
    to password store directory and gpg key fingerprint 
    that will be used.
*/
int init()
{
    const char *config_file = get_config_path();	// $HOME/.config/pwman.conf
    const char *store_path = get_store_path();		// $HOME/.pwstore
    const char *gpg_fpr = get_gpg_fpr();			// gpg key fingerprint
    
	int ret_code = -1;

	/* Check allocation */
    if (!config_file || !store_path || !gpg_fpr) {
        fprintf(stderr, "psm:%s:%d: allocation error", __FILE__, __LINE__);
        goto ret;
    }

	/* Check if X11 is running */
	if (check_X11() != 0) {
		fprintf(stderr, "missing needed X11 display\n");
		goto ret;
	}

	// TODO: maybe this should be removed now
    /* Set up if config_file does not exist */
    if (access(config_file, F_OK) == 0) {
        ret_code = 0;
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
    This function makes user choose betweeen
    available gpg keys for encryption and
    returns its fingerprint.
*/
const char *get_gpg_fpr()
{
    gpgme_key_t key;
    gpgme_key_t *keys = gpg_get_keys();

	char *user_input = (char *) malloc(BUFSIZE);
	char *tmp_fpr, *fpr;
	char *ret_val = NULL;

	int key_pos;
    int pos = 0;

    /* Check allocation */
    if (!keys || !user_input) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret; 
	}

    /* List secret keys */
    while ((key = keys[pos++])) {
        printf("[%d]: %s\n"
               "\t%s\n", pos, key->fpr, key->uids->uid);
    }

    printf("Choose the gpg key you want to use to encrypt your passwords: ");

	if (!(fgets(user_input, BUFSIZE, stdin))) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		goto ret;	
	}

	if ((key_pos = atoi(user_input)) == 0) {
		printf("%s is not a valid input\n", user_input);
		goto ret;	
	}
	
	tmp_fpr = keys[key_pos - 1]->fpr;

	fpr = (char *) malloc(strlen(tmp_fpr) + 1);

	if (!fpr) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;	
	}

	strcpy(fpr, tmp_fpr);

	ret_val = fpr;

ret:
	if (user_input) free(user_input);
	if (keys) free(keys);
    return ret_val;
}
