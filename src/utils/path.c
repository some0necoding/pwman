#include "./path.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    This function builds a path in the format:
        
        root/{rel_path}
*/
const char *build_path(const char *root, const char *rel_path) 
{
	char *path = (char *) malloc(sizeof(char) * (strlen(root) + 1 + strlen(rel_path) + 1));

    if (!path) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }

	sprintf(path, "%s/%s", root, rel_path);

    return 0;
}

/*
    This function adds the extension ext to
    a string fname.
*/
const char *add_ext(const char *fname, const char *ext)
{
    char *new_fname = (char *) malloc(sizeof(char) * (strlen(fname) + 1 + strlen(ext) + 1));

    if (!new_fname) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }

	sprintf(new_fname, "%s.%s", fname, ext);

    return new_fname;
}
