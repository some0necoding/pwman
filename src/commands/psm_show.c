#include "psm_show.h"
#include "../utils/config.h"
#include "../utils/path.h"


// this define needs to remain here (i.e. before including ftw.h)
#define _XOPEN_SOURCE 500


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include <ftw.h>
    
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
const char *get_basename(const char *path);


/*
    This function lists all directories and files under .pwstore.

    Arguments:
        [DIR]     lists all directories and files under .pwstore/dir if dir exists
        [FILE]    lists file under .pwstore if it exists
*/ 
int psm_show(char **args)
{
	const char *file_path = NULL;
    const char *PATH = NULL;

    int ret_code = -1;

    PATH = psm_getenv("PATH");
    
	if (!PATH) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }    

    if (args[1] && (!(file_path = build_path(PATH, args[1])))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }

	/* Recursively call print_file on all directories and files */
    if (nftw(PATH, print_file, 20, 0) == -1) {
        if (errno == 2) {
            printf("No such file or directory\n");
        } else {
            fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        }

        goto ret;
    }

    ret_code = 0;

ret:
	if (file_path) free((char *) file_path);
    if (PATH) free((char *) PATH);
    return ret_code;
}


/*
    This function prints a file name with the right indentation
*/
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	size_t indentation = 0;

	if (ftwbuf->level > 0) {
		indentation = (ftwbuf->level - 1) * 4;
	}
    
	const char *base_name = get_basename(path);

	char *indent = (char *) malloc(indentation + 1);
	char *file_name = NULL;

	int ret_code = -1;

    if (!base_name || !indent) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }
   
    memset(indent, ' ', indentation);
	indent[indentation] = '\0';

	file_name = (char *) rm_ext(base_name, "gpg");

	if (!file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	/* If indentation level equals 0 */ 
    if (ftwbuf->level == 0) {
        printf("Password Store\n");
        ret_code = 0;
        goto ret;
    }
    
	printf("%s|---%s\n", indent, file_name);

    ret_code = 0;

ret:
	if (base_name) free((char *) base_name);
    if (file_name) free(file_name);
	if (indent) free(indent);
    return ret_code;
}

const char *get_basename(const char *path)
{
	char *path_copy = (char *) malloc(strlen(path) + 1);

	if (!path_copy) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

	strcpy(path_copy, path);

	const char *tmp_base_name = basename((char *) path_copy);
	char *base_name = (char *) malloc(strlen(tmp_base_name) + 1);

	if (!base_name) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		if (path_copy) free((char *) path_copy);
		return NULL;
	}

	strcpy(base_name, tmp_base_name);

	if (path_copy) free(path_copy);
	return base_name;
}
