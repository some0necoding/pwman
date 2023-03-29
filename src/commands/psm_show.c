#include "./psm_show.h"
#include "../utils/config.h"
#include "../utils/path.h"
#include <stddef.h>


// this define needs to remain here (i.e. before including ftw.h)
#define _XOPEN_SOURCE 500


#include <stdio.h>
#include <ftw.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <errno.h>

    
#define FMT_TOK "|--"


int print_file(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf);


/*
    This function lists all directories and files under .pwstore.

    Arguments:
        [DIR]     lists all directories and files under .pwstore/dir if dir exists
        [FILE]    lists file under .pwstore if it exists
*/ 
int psm_show(char **args)
{
    const char *PATH = psm_getenv("PATH");
	const char *file_path;
    int ret_code = -1;

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
    char *file_name = basename((char *) path);

    int ret_code = -1;

    if (!file_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
    }
   
	file_name = (char *) rm_ext(file_name, ".gpg");

    /* If indentation level equals 0 */ 
    if (ftwbuf->level == 0) {
        printf("%s\n", file_name);
        ret_code = 0;
        goto ret;
    }

    /* Every indentation level equals 4 spaces */
	size_t indentation = ftwbuf->level * 4; 

    //char *fmt_name = (char *) malloc(sizeof(char) * (indentation + strlen(FMT_TOK) + strlen(file_name) + 1));
	char *indent = (char *) malloc(sizeof(char) * (indentation + 1));

    if (/*!fmt_name ||*/ !indent) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret; 
    }

    memset(indent, ' ', indentation - 1);
	indent[indentation] = '\0';

	//sprintf(fmt_name, "%s%s%s", indent, FMT_TOK, file_name);
	printf("%s%s%s\n", indent, FMT_TOK, file_name);

    //printf("%s\n", fmt_name);

    ret_code = 0;

ret:
    //fmt_name ? free(fmt_name) : 0;
	if (indent) free(indent);
    if (file_name) free((char *) file_name);
    return ret_code;
}
