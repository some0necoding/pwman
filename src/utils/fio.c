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
    This function returns all bytes of a stream
*/
char *fgetall(const char *path)
{
	size_t bufsize = MAX_LINE_LENGTH;

    FILE *file = fopen(path, "rb");
	
	char *content = (char *) malloc(sizeof(char) * bufsize);
    char *ret_val = NULL;

	char c;

    if (!file || !content) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        goto ret; 
    }

	int pos = 0;
	while ((c = fgetc(file)) != EOF) {
	
		content[pos++] = c;

		if (pos >= bufsize) {
	
			bufsize += MAX_LINE_LENGTH;
			content = realloc(content, bufsize);

			if (!content) {
				fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
				goto ret; 
			}
		}
	}

	if (c == EOF && ferror(file)) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		goto ret;
	}

	ret_val = content;

ret:    
    if (file) fclose(file);
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
