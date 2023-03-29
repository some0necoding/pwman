#include "./path.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*
    This function builds a path in the format:
        
	{root}/{rel_path}
*/
const char *build_path(const char *root, const char *rel_path) 
{
	char *path = (char *) malloc(sizeof(char) * (strlen(root) + 1 + strlen(rel_path) + 1));

    if (!path) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }

	sprintf(path, "%s/%s", root, rel_path);

    return path;
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


/*
	This function removes the extension ext
	from the string fname.
*/
const char *rm_ext(const char *fname, const char* ext)
{
	char *new_fname = (char *) malloc(sizeof(char) * (strlen(fname) - strlen(ext) + 1));

	if (!new_fname) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

	/* fname does not contain ext */
	if (strcmp(fname+(strlen(fname) - strlen(ext)), ext) != 0) {
		if (new_fname) free(new_fname);
		return fname;
	}

	strncpy(new_fname, fname, strlen(fname) - strlen(ext));
	new_fname[strlen(fname) - strlen(ext)] = '\0';

	return new_fname;
}
