#ifndef SODIUM_PLUS_PLUS
#define SODIUM_PLUS_PLUS

#include <string.h>
#include <sodium.h>

// reallocs a buffer allocated with sodium
void *sodium_realloc(void *ptr, size_t old_size, size_t new_size);

// frees a 2d array allocated using sodium
void sodium_free_2d_arr(int size, unsigned char **arr);

#endif