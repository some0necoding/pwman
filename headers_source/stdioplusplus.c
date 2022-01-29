#include "../headers/stdioplusplus.h"

#include "../headers/sodiumplusplus.h"
#include <stdio.h>

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
        return -1;
    }

    // file size is now equal to the cursor position
    if((size = (size_t) ftell(file)) == -1) {
        perror("psm: I/O error");
        return -1;
    }

    return size;
}

// returns all stream bytes comprised between the beginning of the stream
// and the first null byte of the stream. All buffers are allocated using 
// sodium_malloc (doc.libsodium.org).
unsigned char *fgetalls(char *file_path)
{
    size_t rlen;                                // total bytes returned by fread
    size_t file_size;                           
    unsigned char *buff;                        // returned buffer
    FILE *file;                                 

    // getting the file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("error while seeking for file size");
        return NULL;
    }

    // creating a buffer of the same size as the file (+ '\0') to hold its content
    buff = (unsigned char *) sodium_malloc(file_size + 1);
    file = fopen(file_path, "rb");

    // checking for good opening/allocation
    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    // reading as many bytes as the file size
    rlen = fread(buff, 1, file_size, file);

    // error checking
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    // null-terminating the buffer and closing the file
    buff[file_size] = '\0';
    fclose(file);

    return buff;
}

// the same as above but for not sensitive data (all buffers are allocated 
// using standard malloc).
unsigned char *fgetall(char *file_path)
{
    size_t rlen;                                // total bytes returned by fread
    size_t file_size;
    unsigned char *buff;                        // returned buffer
    FILE *file;

    // getting the file size
    if ((file_size = fsize(file_path)) == -1) {
        return NULL;
    }

    // creating a buffer of the same size as the file (+ '\0') to hold its content
    buff = (unsigned char *) malloc(file_size + 1);
    file = fopen(file_path, "rb");

    // checking for good opening/allocation
    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    // reading as many bytes as the file size
    rlen = fread(buff, 1, file_size, file);

    // error checking
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    // null-terminating the buffer and closing the file
    buff[file_size] = '\0';
    fclose(file);

    return buff;
}

// this function gets the string of bytes contained between start_pos (inclusive) 
// and the first null-terminated byte of a stream. It uses only sodium-allocated buffers.
unsigned char *fgetfroms(char *file_path, int start_pos)
{
    size_t file_size;
    size_t content_len;                         // number of bytes between start_pos and EOF
    size_t rlen;                                // total bytes returned by fread
    unsigned char *buff;                        // returned buffer
    FILE *file;

    // getting file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("psm: error while seeking for file size");
        return NULL;
    }

    // input checking
    if (start_pos > file_size | start_pos < 0) {
        perror("psm: invalid stream position");
        return NULL;
    }

    // creating a buffer to hold content from start_pos to EOF
    content_len = file_size - start_pos;
    buff = (unsigned char *) malloc(content_len + 1);
    file = fopen(file_path, "rb");

    // checking for good opening/allocation
    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    // moving the stream position indicator to start_pos
    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        return NULL;
    }

    // reading as many bytes as contained between start_pos and EOF
    rlen = fread(buff, 1, content_len, file);

    // error checking
    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    // null-terminating the buffer and closing the file
    buff[content_len] = '\0';
    fclose(file);

    return buff;
}

// this function gets the file content from start_pos (inclusive)
// to end_pos (exclusive). It uses only sodium-allocated buffers.
// the stream does not either need to be null-terminated or can 
// cointain more null bytes.
unsigned char *fgetfromtos(char *file_path, int start_pos, int end_pos)
{
    size_t content_len;                         // number of bytes between start_pos and end_pos
    size_t rlen;                                // total bytes returned by fread
    unsigned char *buff;                        // returned buffer
    FILE *file;

    // input checking
    if (start_pos < 0 | end_pos < 0 | end_pos < start_pos) {
        perror("psm: invalid stream position");
        return NULL;
    }

    // creating a buffer to hold content from start_pos to end_pos
    content_len = end_pos - start_pos;                                    // selected content + \0
    buff = (unsigned char *) malloc(content_len + 1);
    file = fopen(file_path, "rb");

    // checking for good opening/allocation
    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    // moving the stream position indicator to start_pos
    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        return NULL;
    }

    // reading as many bytes as contained between start_pos and EOF
    rlen = fread(buff, 1, content_len, file);

    // error checking
    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    // null-terminating the buffer and closing the file
    buff[content_len] = '\0';
    fclose(file);

    return buff;
}