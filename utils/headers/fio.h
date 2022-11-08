#ifndef STDIO_PLUS_PLUS
#define STDIO_PLUS_PLUS

#include <stddef.h>
#include <stdio.h>

/* Returns the size of a stream */
size_t fsize(char *path);

/* Returns all bytes contained in a stream */
int fgetall(char *path, char **buf, size_t bufsize);

/* Reads a single line from file */
int freadline(FILE *file, char **ret_buff, size_t bufsize);

#endif