#include "path.h"
#include "config.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>


#define BUFSIZE 32


int cmplen(const void *a, const void *b);
int is_parentdir(const char *path_1, const char *path_2);
const char **get_dirs(const char *path); 
const char **splitstr(const char *path, const char *delim);
size_t arrlen(void **arr);


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
	char *new_fname = (char *) malloc(sizeof(char) * (strlen(fname) + 1));

	if (!new_fname) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

	/* fname does not contain ext */
	if (strcmp(fname+(strlen(fname) - strlen(ext)), ext) != 0) {
		strcpy(new_fname, fname);
		return new_fname;
	}

	strncpy(new_fname, fname, strlen(fname) - strlen(ext));
	new_fname[strlen(fname) - strlen(ext)] = '\0';

	return new_fname;
}


/*
	This function recursively call mkdir to create all
	sbdirectories of a relative tree under STORE_PATH.
*/
int psm_mkdir(const char *relative_path)
{
	const char *PATH = get_store_path();
	const char **dirs = NULL;
	const char *dir = NULL;
	
	DIR *store_dir = NULL;

	int pos = 0;
	int store_fd;
	int ret_code = -1;

	if (!PATH) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

	store_dir = opendir(PATH);

	if (!store_dir) {
		fprintf(stderr, "psm:%s:%d: I/O error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno);
		goto ret;
	}

	if ((store_fd = dirfd(store_dir)) < 0) {
		fprintf(stderr, "psm:%s:%d: I/O error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno);
		goto ret;
	}

	if (!(dirs = get_dirs(relative_path))) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

	dir = dirs[pos++];
	while (dir) {
		
		if (mkdirat(store_fd, dir, 0700) != 0 && errno != EEXIST) {
			fprintf(stderr, "psm:%s:%d: I/O error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno);
			goto ret;
		}

		dir = dirs[pos++];
	}

	ret_code = 0;

ret:
	if (PATH) free((char *) PATH);
	if (dirs) free(dirs);
	if (dir) free((char *) dir);
	if (store_dir) closedir(store_dir);
	return ret_code;
}

/*
	This function recursively call rmdir to delete all
	empty sbdirectories of a relative tree under STORE_PATH.
*/
int psm_rmdir(const char *relative_path)
{
	const char *PATH = get_store_path();
	const char **dirs = NULL;
	const char *dir = NULL;

	int pos = 0;
	int ret_code = -1;

	if (!PATH) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

	if (!(dirs = get_dirs(relative_path))) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

	qsort(dirs, arrlen((void **) dirs), sizeof(char *), cmplen);

	dir = dirs[pos++];
	while (dir) {

		if (!is_parentdir(dir, PATH) && rmdir(dir) != 0 && errno != EEXIST && errno != ENOTEMPTY) {
			fprintf(stderr, "psm:%s:%d: I/O error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno);
			printf("%s\n", dir);
			goto ret;
		}

		dir = dirs[pos++];
	}

	ret_code = 0;

ret:
	if (PATH) free((char *) PATH);
	if (dirs) free(dirs);
	if (dir) free((char *) dir);
	return ret_code;
}


/*
	This function compares the length of strings.
*/
int cmplen(const void *a, const void *b)
{
	return strlen(*(char * const *)b) - strlen(*(char * const *)a);
}


/*
	This function checks if path_1 is parentdir of path_2.

	Example:
		is_parentdir("/foo/boo", "/foo") -> 0 (i.e. false)
		is_parentdir("/foo", "/foo/boo") -> 1 (i.e. true)
*/
int is_parentdir(const char *path_1, const char *path_2)
{
	if (strncmp(path_1, path_2, strlen(path_1)) <= 0) {
		return 1;
	}

	return 0;
}


const char **get_dirs(const char *path) 
{
	size_t bufsize = BUFSIZE;
	const char **dir_names = splitstr(path, "/");
	const char *dir_name = NULL;
	
	char *current_path = (char *) malloc(sizeof(char) * bufsize);
	
	char **dirs = (char **) malloc(sizeof(char *) * (arrlen((void **) dir_names) + 1));
	char **ret_val = NULL;

	int pos = 0;

	if (!dir_names || !dirs || !current_path) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;	
	}

	current_path[0] = '\0';
	
	dir_name = dir_names[pos];
	while (dir_name) {

		char *rel_path = (char *) malloc(sizeof(char) * (strlen(current_path) + 1 + strlen(dir_name) + 1));

		sprintf(rel_path, "%s/%s", current_path, dir_name);	
	
		if (!rel_path) {
			fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
			goto ret;	
		}

		if (strlen(rel_path) >= strlen(current_path)) {
			
			bufsize += BUFSIZE;
			
			current_path = (char *) realloc(current_path, (sizeof(char) * bufsize));

			if (!current_path) {
				fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
				goto ret;
			}
		
		}
		
		strcpy(current_path, rel_path);

		dirs[pos++] = rel_path;
		dir_name = dir_names[pos];
	
	}

	dirs[pos] = NULL;
	ret_val = dirs;

ret:
	if (dir_names) free(dir_names);
	if (dir_name) free((char *) dir_name);
	if (current_path) free(current_path);
	return (const char **) ret_val;
}

const char **splitstr(const char *path, const char *delim)
{
	size_t tokslen = 2;

	const char *tmp_token = NULL;
	
	char **tokens = (char **) malloc(sizeof(char *) + tokslen);
	char *path_copy = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	char **ret_val = NULL;

	int pos = 0;

	if (!path_copy || !tokens) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;	
	}

	strcpy(path_copy, path);

	tmp_token = strtok(path_copy, delim);
	while (tmp_token) {

		char *token = (char *) malloc(sizeof(char) * (strlen(tmp_token) + 1));

		if (!token) {
			fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
			goto ret;	
		}
		
		strcpy(token, tmp_token);
		tokens[pos++] = token;

		if (pos >= (tokslen - 1)) {

			tokens = (char **) realloc(tokens, (sizeof(char *) * ++tokslen));

			if (!tokens) {
				fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
				goto ret;	
			}
		}

		tmp_token = strtok(NULL, delim);
	}

	tokens[pos] = NULL;
	ret_val = tokens;

ret:
	if (path_copy) free(path_copy);
	return (const char **) ret_val;
}

const char *get_dirname(const char *path)
{
	char *path_copy = (char *) malloc(sizeof(char) * (strlen(path) + 1));
	char *tmp_dir_name = NULL;
	char *dir_name = NULL;
	char *ret_val = NULL;

	if (!path_copy) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	strcpy(path_copy, path);

	if (!(tmp_dir_name = dirname(path_copy))) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	dir_name = (char *) malloc(sizeof(char) * (strlen(tmp_dir_name) + 1));

	if (!dir_name) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret;
	}

	strcpy(dir_name, tmp_dir_name);

	ret_val = dir_name;

ret:
	if (path_copy) free(path_copy);
	return ret_val;
}

size_t arrlen(void **arr)
{
	void *elem;
	int pos = 0;
	size_t size = 0;

	elem = arr[pos++];
	while (elem) {
		size++;
		elem = arr[pos++];
	}

	return size;
}
