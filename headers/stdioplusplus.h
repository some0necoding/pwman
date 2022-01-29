#ifndef STDIO_PLUS_PLUS
#define STDIO_PLUS_PLUS

#include <stdio.h>

// returns the size of a null-terminated file.
size_t fsize(char *file_path);

// returns all bytes comprised between the beginning of the file and EOF in a secure way.
unsigned char *fgetalls(char *file_path);

// the same as above but for not sensitive data.
unsigned char *fgetall(char *file_path);

// returns all bytes comprised between start_pos (inclusive) and EOF in a secure way.
unsigned char *fgetfroms(char *file_path, int start_pos);

// returns all bytes comprised between start_pos (inclusive) and end_pos (exclusive) in a secure way.
unsigned char *fgetfromtos(char *file_path, int start_pos, int end_pos);

#endif