#include "./psm_show.h"
#include "../utils/config.h"
#include "../utils/path.h"


// this define needs to remain here (i.e. before including ftw.h)
#define _XOPEN_SOURCE 500


#include <ftw.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
    
int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf);


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
	// TODO: write a get_basename function to pass a copy of path to it.
    char *base_name = basename((char *) path);
	char *file_name = NULL;
	char *indent = NULL;

	int ret_code = -1;

	size_t indentation = (ftwbuf->level - 1) * 4; 

    if (!base_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }
   
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
    
	indent = (char *) malloc(sizeof(char) * indentation);

    if (!indent) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret; 
    }

    memset(indent, ' ', indentation);

	printf("%s|---%s\n", indent, file_name);

    ret_code = 0;

ret:
    if (file_name) free(file_name);
	if (indent) free(indent);
    return ret_code;
}
