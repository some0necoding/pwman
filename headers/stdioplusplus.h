#ifndef STDIO_PLUS_PLUS
#define STDIO_PLUS_PLUS

#include <stdio.h>
#include <sodiumplusplus.h>

// returns the size of a file
extern int fsize(char *file_path);

// returns all bytes of a file in a secure way (all buffers are allocated using sodium)
extern unsigned char *fgetalls(char *file_path);

// the same as above but for not sensitive data (all buffers are allocated using standard malloc)
extern unsigned char *fgetall(char *file_path);

// this function gets the file content from start_pos (inclusive)
// to the end of the stream. It uses only sodium-allocated buffers.
extern unsigned char *fgetfroms(char *file_path, int start_pos);

// this function gets the file content from start_pos (inclusive)
// to end_pos (exclusive). It uses only sodium-allocated buffers.
extern unsigned char *fgetfromtos(char *file_path, int start_pos, int end_pos);

#endif