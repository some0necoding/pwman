#include "./fio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 64

/*
    This function returns the size of a 
    null-terminated stream.
*/
size_t fsize(char *path)
{
    FILE *file = fopen(path, "r");
    size_t size = 0;
    int ret_code = -1;

    if (!file) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Move cursor to end of stream */
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    /* File size equals cursor position */ 
    if((size = (size_t) ftell(file)) < 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = size;

ret:
    file ? fclose(file) : 0;
    return ret_code;
}

/*
    This function returns all bytes of a stream
*/
int fgetall(char *path, char **buf, size_t bufsize)
{
    FILE *file = fopen(path, "rb");
    
    size_t rlen;
    size_t file_size;

    int ret_code = -1;

    if (!file) {
        perror("psm: allocation error");
        goto ret; 
    }

    if ((file_size = fsize(path)) == -1) {
        perror("psm: I/O error");
        goto ret;
    }

    /* Create buffer */
    if (bufsize < (file_size + 1)) {
        *buf = realloc(*buf, sizeof(char *) * (file_size + 1));

        if (!*buf) {
            perror("psm: allocation error");
            goto ret;
        }
    }

    rlen = fread(*buf, 1, file_size, file);

    /* Error checking */
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    memcpy(*buf+file_size, "\0", 1);

    ret_code = 0;

ret:    
    file ? fclose(file) : 0;
    return ret_code;
}

/*
    This function reads a line from file and can be 
    used to read a file line by line. It returns the
    number of characters read on success, 0 on EOF 
    reached and -1 on failure.
*/
int freadline(FILE *file, char **buf, size_t bufsize) 
{
    char ch;
    int pos = 0;

    if (!file) {
        perror("psm: I/O error");
        return -1;
    }

    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        
        memcpy(*buf+pos, &ch, sizeof(char));
        pos++;

        /* Stretch buffer if needed */
        if (pos >= bufsize) {

            bufsize += BUFSIZE;
            *buf = realloc(*buf, sizeof(char *) * bufsize);

            if (!*buf) {
                perror("psm: allocation error");
                return -1;
            }
        }
    }

    /* Stretch buffer to fit NULL char if needed */
    if (pos >= bufsize) {
        
        bufsize += 1;
        *buf = realloc(*buf, sizeof(**buf) * bufsize);

        if (!*buf) {
            perror("psm: allocation error");
            return -1;
        }
    }
        
    memcpy(*buf+pos, "\0", sizeof(char));

    /* If EOF is reached return 0 */
    if (ch == EOF) {
        return 0;
    }

    return pos;
}
