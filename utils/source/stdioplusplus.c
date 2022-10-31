#include "../headers/stdioplusplus.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/array_handling.h"

#include <stdio.h>
#include <string.h>

#define BUFSIZE 64

// returns the size of a null-terminated stream (thus this will not work 
// with bytes streams that contains multiple null bytes or does not contain 
// it at all).
size_t fsize(char *file_path)
{
    FILE *file = fopen(file_path, "r");
    size_t size = 0;
    
    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    // the cursor is positioned at the end of the stream
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("psm: I/O error");
        fclose(file);
        return -1;
    }

    // file size is now equal to the cursor position
    if((size = (size_t) ftell(file)) == -1) {
        perror("psm: I/O error");
        fclose(file);
        return -1;
    }

    fclose(file);
    return size;
}

// stores all stream bytes comprised between the beginning of the stream and the first null 
// byte of the stream into "ret_buff" (that needs to be allocated using sodium_malloc). All 
// buffers are allocated using sodium_malloc (doc.libsodium.org).
int fgetalls(char *file_path, char **ret_buff, size_t ret_buff_size)
{
    size_t tmp_size;
    size_t rlen;                                // total bytes returned by fread
    size_t file_size;

    FILE *file = fopen(file_path, "rb");

    int ret_code = -1;                     

    // checking for good opening
    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    // getting the file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("error while seeking for file size");
        goto ret;
    }

    tmp_size = file_size + 1;

    // creating a buffer of the same size as the file (+ '\0') to hold its content
    if (ret_buff_size < tmp_size) {
        *ret_buff = (char *) sodium_realloc(*ret_buff, ret_buff_size, tmp_size);

        if (!*ret_buff) {
            perror("psm: allocation error");
            goto ret;
        }
    }

    // reading as many bytes as the file size
    rlen = fread(*ret_buff, 1, file_size, file);

    // error checking
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    // null-terminating the buffer and closing the file
    memcpy(*ret_buff+file_size, "\0", 1);
    
    ret_code = 0;

ret:
    fclose(file);
    return ret_code;
}

// stores all stream bytes comprised between the beginning of the stream
// and the first null byte of the stream into "ret_buff".
int fgetall(char *file_path, char **ret_buff, size_t ret_buff_size)
{
    size_t tmp_size;
    size_t rlen;                                // total bytes returned by fread
    size_t file_size;

    FILE *file = fopen(file_path, "rb");

    int ret_code = -1;

    // checking for good opening
    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    // getting the file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("psm: error while seeking for file size");
        goto ret;
    }

    tmp_size = file_size + 1;

    // creating a buffer of the same size as the file (+ '\0') to hold its content
    if (ret_buff_size < tmp_size) {
        *ret_buff = (char *) realloc(*ret_buff, tmp_size);

        if (!*ret_buff) {
            perror("psm: allocation error");
            goto ret;
        }
    }

    // reading as many bytes as the file size
    rlen = fread(*ret_buff, 1, file_size, file);

    // error checking
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    // null-terminating the buffer and closing the file
    memcpy(*ret_buff+file_size, "\0", 1);

    ret_code = 0;

ret:    
    fclose(file);
    return ret_code;
}

// this function gets the string of bytes contained between start_pos (inclusive) 
// and the first null-terminated byte of a stream, storing it in "ret_buff" (that
// needs to be allocated using sodium_malloc). It uses only sodium-allocated buffers.
int fgetfroms(char *file_path, int start_pos, char **ret_buff, size_t ret_buff_size)
{
    size_t tmp_size;
    size_t file_size;
    size_t content_len;                         // number of bytes between start_pos and EOF
    size_t rlen;                                // total bytes returned by fread

    FILE *file = fopen(file_path, "rb");

    int ret_code = -1;

    // checking for good opening
    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    // getting file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("psm: error while seeking for file size");
        goto ret;
    }

    // input checking
    if (start_pos > file_size | start_pos < 0) {
        perror("psm: invalid stream position");
        goto ret;
    }

    // creating a buffer to hold content from start_pos to EOF
    content_len = file_size - start_pos;
    tmp_size = content_len + 1;

    if (ret_buff_size < tmp_size) {
        *ret_buff = (char *) sodium_realloc(*ret_buff, ret_buff_size, tmp_size);

        if (!*ret_buff) {
            perror("psm: allocation error");
            goto ret;
        }
    }


    // moving the stream position indicator to start_pos
    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    // reading as many bytes as contained between start_pos and EOF
    rlen = fread(*ret_buff, 1, content_len, file);

    // error checking
    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    // null-terminating the buffer and closing the file
    memcpy(*ret_buff+content_len, "\0", 1);

    ret_code = 0;

ret:
    fclose(file);
    return ret_code;
}

// this function gets the file content from start_pos (inclusive) to end_pos (exclusive) and
// stores it in "ret_buff" (that should be allocated using sodium_malloc). 
// It uses only sodium-allocated buffers. The stream does not either need to be null-terminated
// or to cointain only one null byte.
int fgetfromtos(char *file_path, int start_pos, int end_pos, char **ret_buff, size_t ret_buff_size)
{
    size_t tmp_size;
    size_t file_size;
    size_t content_len;                         // number of bytes between start_pos and end_pos
    size_t rlen;                                // total bytes returned by fread

    FILE *file = fopen(file_path, "rb");

    int ret_code = -1;

    // checking for good opening
    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((file_size = fsize(file_path)) == -1) {
        perror("psm: error while seeking for file size");
        goto ret;
    }

    // input checking
    if (start_pos < 0 | end_pos < 0 | end_pos < start_pos | end_pos > file_size | start_pos > file_size) {
        perror("psm: invalid stream position");
        goto ret;
    }

    // creating a buffer to hold content from start_pos to end_pos
    content_len = end_pos - start_pos;                                    // selected content + \0 
    tmp_size = content_len + 1;

    if (ret_buff_size < tmp_size) {
        *ret_buff = (unsigned char *) sodium_realloc(*ret_buff, ret_buff_size, tmp_size);

        if (!*ret_buff) {
            perror("psm: allocation error");
            goto ret;
        }
    }

    // moving the stream position indicator to start_pos
    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    // reading as many bytes as contained between start_pos and EOF
    rlen = fread(*ret_buff, 1, content_len, file);

    // error checking
    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        goto ret;
    }

    // null-terminating the buffer and closing the file
    memcpy(*ret_buff+content_len, "\0", 1);

    ret_code = 0;

ret:
    fclose(file);
    return ret_code;
}

// it reads a line from file and can be used to read a file line
// by line. When it reaches EOF it returns EOF (-1), -2 is used
// for errors.
int freadline(FILE *file, char **ret_buff, size_t bufsize) 
{
    char c;
    int pos = 0;

    if (!file) {
        perror("psm: I/O error");
        return -2;
    }

    while ((c = fgetc(file)) != EOF && c != '\n') {
        memcpy(*ret_buff+pos, &c, sizeof(char));
        pos++;

        if (pos >= bufsize) {

            size_t old_size = bufsize;
            bufsize += BUFSIZE;
            *ret_buff = realloc(*ret_buff, sizeof(**ret_buff) * bufsize);

            if (!*ret_buff) {
                perror("psm: allocation error");
                return -2;
            }

            // set to null newly allocated memory in order
            // to prevent undefined behaviour
            memset(*ret_buff+old_size, '\0', (BUFSIZE - 1));
        }
    }

    if (pos >= bufsize) {
        bufsize += 1;
        *ret_buff = realloc(*ret_buff, sizeof(**ret_buff) * bufsize);
        memcpy(*ret_buff+pos, '\0', sizeof(char));
    }

    if (c == EOF) {
        return EOF;
    }

    return pos;
}