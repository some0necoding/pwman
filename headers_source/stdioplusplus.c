#ifndef EOF
    #include <stdio.h>
#endif

#ifndef SODIUM_PLUS_PLUS
    #include "headers/sodiumplusplus.h"
#endif

#include "headers/stdioplusplus.h"

// returns the size of a file
int fsize(char *file_path)
{
    FILE *file = fopen(file_path, "r");
    int size = 0;
    
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
    if((size = (int) ftell(file)) == -1) {
        perror("psm: I/O error");
        return -1;
    }

    return size;
}

// returns all bytes of a file in a secure way (all buffers are allocated using sodium)
unsigned char *fgetalls(char *file_path)
{
    size_t rlen;
    size_t buff_len;
    int file_size;
    unsigned char *buff;
    FILE *file;

    // get the file size
    if ((file_size = fsize(file_path)) == -1) {
        perror("error while seeking for file size");
        return NULL;
    }

    // create a buffer of the same size as the file (+ 1) to hold its content
    buff_len = file_size + 1; // content + '\0'
    buff = (unsigned char *) sodium_malloc(buff_len);
    file = fopen(file_path, "rb");

    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    // read as many bytes as the file size
    rlen = fread(buff, 1, file_size, file);

    // error checking
    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    // add a final NULL char to make the buffer easier to handle
    buff[buff_len - 1] = '\0';
    fclose(file);

    return buff;
}

// the same as above but for not sensitive data (all buffers are allocated using standard malloc)
unsigned char *fgetall(char *file_path)
{
    size_t rlen;
    size_t buff_len;
    int file_size;
    unsigned char *buff;
    FILE *file;

    if ((file_size = fsize(file_path)) == -1) {
        return NULL;
    }

    buff_len = file_size + 1; // content + '\0'
    buff = (unsigned char *) malloc(buff_len);
    file = fopen(file_path, "rb");

    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    rlen = fread(buff, 1, file_size, file);

    if ((rlen != file_size) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    buff[buff_len - 1] = '\0';
    fclose(file);

    return buff;
}

// this function gets the file content from start_pos (inclusive)
// to the end of the stream. It uses only sodium-allocated buffers.
unsigned char *fgetfroms(char *file_path, int start_pos)
{
    int file_size;
    size_t content_len;
    size_t buff_len;
    size_t rlen;
    unsigned char *buff;
    FILE *file;

    if ((file_size = fsize(file_path)) == -1) {
        perror("psm: error while seeking for file size");
        return NULL;
    }

    if (start_pos > file_size | start_pos < 0) {
        perror("psm: invalid stream position");
        return NULL;
    }

    content_len = file_size - start_pos;
    buff_len = content_len + 1; // selected content + '\0'
    buff = (unsigned char *) malloc(buff_len);
    file = fopen(file_path, "rb");

    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        return NULL;
    }

    rlen = fread(buff, 1, content_len, file);

    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    buff[buff_len - 1] = '\0';
    fclose(file);

    return buff;
}

// this function gets the file content from start_pos (inclusive)
// to end_pos (exclusive). It uses only sodium-allocated buffers.
unsigned char *fgetfromtos(char *file_path, int start_pos, int end_pos)
{
    int file_size;
    size_t content_len;
    size_t buff_len;
    size_t rlen;
    unsigned char *buff;
    FILE *file;

    if ((file_size = fsize(file_path)) == -1) {
        return NULL;
    }

    if (start_pos > file_size | end_pos > file_size | start_pos < 0 | end_pos < 0) {
        perror("psm: invalid stream position");
        return NULL;
    }

    content_len = end_pos - start_pos;
    buff_len = content_len + 1; // selected content + 1
    buff = (unsigned char *) malloc(buff_len);
    file = fopen(file_path, "rb");

    if (!(file && buff)) {
        perror("psm: allocation error");
        return NULL;
    }

    if (fseek(file, start_pos, SEEK_SET) != 0) {
        perror("psm: I/O error");
        return NULL;
    }

    rlen = fread(buff, 1, content_len, file);

    if ((rlen != content_len) && (ferror(file) != 0)) {
        perror("psm: I/O error");
        fclose(file);
        return NULL;
    }

    buff[buff_len - 1] = '\0';
    fclose(file);

    return buff;
}