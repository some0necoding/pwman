#include "./fio.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MAX_LINE_LENGTH 4096


/*
    This function returns the size of a stream.
*/
size_t fsize(const char *path)
{
    FILE *file = fopen(path, "rb");
    size_t size = 0;
    int ret_code = -1;

    if (!file) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* Move cursor to end of stream */
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

    /* File size equals cursor position */ 
    if((size = (size_t) ftell(file)) < 0) {
        fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        goto ret;
    }

    ret_code = size;

ret:
    if (file) fclose(file);
    return ret_code;
}


/*
	This function gets n bytes from file stored at 
	path. Returns NULL on I/O error or if n is greater
	than file size.
*/
char *fgetn(const char *path, size_t n)
{
	FILE *file = fopen(path, "rb");

	char *content = (char *) malloc(n);
	char *ret_val = NULL;

	char c;

	int pos = 0;

	if (!content || !file) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret; 
	}

	for (int i = 0; i < n; i++) {
		c = fgetc(file);
		content[pos++] = c;
	}

	if (c == EOF && ferror(file)) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		goto ret;
	}

	ret_val = content;
		
ret:
	if (file) fclose(file);
	if (content) free(content);
	return ret_val;
}


/*
    This function returns all bytes of a stream
*/
char *fgetall(const char *path)
{
	char *content;
    char *ret_val;

	size_t filesize;

	if ((filesize = fsize(path)) < 0) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		return NULL;	
	}

	content = fgetn(path, filesize);

	if (!content) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL; 
	}

    return ret_val;
}


/*
    This function reads a line from file and can be 
    used to read a file line by line. It returns the
    line read on success and NULL on failure.
*/
char *freadline(FILE *file) 
{
	char *line = (char *) malloc(sizeof(char) * MAX_LINE_LENGTH);
    
	char c;
    
	int pos = 0;

    if (!file || !line) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }

    while (((c = fgetc(file)) != EOF) && (c != '\n') && (pos < MAX_LINE_LENGTH)) {
		line[pos++] = c;
    }

	line[pos++] = '\0';

    if (c == EOF && ferror(file)) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
        return NULL;
    }

    return line;
}
