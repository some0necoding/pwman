#ifndef SODIUM_PLUS_PLUS
#define SODIUM_PLUS_PLUS

#include <string.h>

// reallocs a buffer allocated with sodium
void *sodium_realloc(void *ptr, size_t old_size, size_t new_size);

#endif