#include "./psm_rm.h"
#include "../utils/config.h"
#include "../utils/path.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#define BUFSIZE 16


/*
    This function removes a file from PATH.

    Arguments:
        [FILE]      this is a relative path which has PATH as root

    Examples:
        rm test_file                stored at PATH/test_file
        rm test_dir/test_file       stored at PATH/test_dir/test_file
*/
int psm_rm(char **args) 
{
    const char *PATH = psm_getenv("PATH");
    const char *file_name = add_ext(args[1], "gpg"); 
	const char *file_path = NULL;
	const char *dir_name = NULL;

	char *user_input = (char *) malloc(BUFSIZE);

	int ret_code = -1;

	/* Check allocation */
    if (!PATH || !user_input || !file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Check arguments */
    if (!args[1]) {
        printf("You missed an argument\n"
                        "\tSyntax: rm [FILE]\n");
        goto ret;
    }

    if (!(file_path = build_path(PATH, file_name))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    } 

    /* Ask for confirm */
    printf("Do you really want to permanently delete \"%s\" [y/N]: ", file_path);

	if (!(fgets(user_input, BUFSIZE, stdin))) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		goto ret;
	}

	if (!(dir_name = get_dirname(file_path))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}
    
	/* Delete file */
    if (strcmp(user_input, "y") || strcmp(user_input, "Y")) {

		int ret = unlink(file_path);

		if (ret != 0 && (errno == EISDIR || errno == ENOENT)) {
			fprintf(stdout, "%s is not a valid file\n", file_path);
		} else if (ret != 0 && errno != EISDIR) {
            fprintf(stderr, "psm:%s:%d: I/O error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno);
            goto ret;
        }
    }

	if (psm_rmdir(dir_name) != 0) {
		fprintf(stderr, "psm:%s:%d: cannot delete directory %s\n", __FILE__, __LINE__, dir_name);
		goto ret;
	}

    ret_code = 0;

ret:
    if (PATH) free((char *) PATH);
	if (file_path) free((char *) file_path);
    if (file_name) free((char *) file_name);
	if (dir_name) free((char *) dir_name);
	if (user_input) free(user_input);
	return ret_code;
}
