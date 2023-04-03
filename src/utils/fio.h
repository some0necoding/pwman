#ifndef FIO_UTIL
#define FIO_UTIL

#include <stddef.h>
#include <stdio.h>

/* Returns the size of a stream */
size_t fsize(const char *path);

/* Returns all bytes contained in a stream */
char *fgetall(const char *path);

/* Reads n bytes from file stored at path */
char *fgetn(const char *path, size_t n);

/* Reads a single line from file */
char *freadline(FILE *file);

#endif
