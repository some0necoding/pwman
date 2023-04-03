#ifndef INPUT_UTIL
#define INPUT_UTIL

#include <stdio.h>

// this function securely (i.e. disabling echo) reads all bytes from stdin and 
// stores them into "buffer" (that needs to be allocated using sodium_malloc).
char *read_line_s();

// reads all bytes from stdin and stores them into "buffer"
char *read_line();

#endif
