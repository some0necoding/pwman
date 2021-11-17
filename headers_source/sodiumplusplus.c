#ifndef EOF
    #include <stdio.h>
#endif

#include <sodium.h>
#include <string.h>

#include "sodiumplusplus.h"

// reallocs a buffer allocated with sodium
void *sodium_realloc(void *ptr, size_t old_size, size_t new_size)
{
    void *new_ptr;

    // if the old pointer is 0 there's nothing to realloc so it simply calls sodium_malloc
    if (ptr == 0) {
        return sodium_malloc(new_size);
    }

    // if the old size is greather than the new one return the old pointer
    if (new_size <= old_size) {
        return ptr;
    }

    // creates a larger pointer
    new_ptr = sodium_malloc(new_size);
    // copies the old pointer's content in the new pointer
    memcpy(new_ptr, ptr, old_size);
    // frees the old pointer in order to not leave a buffer hanging around mlocked
    sodium_free(ptr);
    return new_ptr;
}