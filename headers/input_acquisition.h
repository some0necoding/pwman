#ifndef INPUT_ACQUISITION
#define INPUT_ACQUISITION

#include <stddef.h>

// this function securely (i.e. disabling echo) reads all bytes from stdin and 
// stores them into "buffer" (that needs to be allocated using sodium_malloc).
int read_line_s(char *buffer, size_t bufsize);

// reads all bytes from stdin and stores them into "buffer"
int read_line(char *buffer, size_t bufsize);

#endif