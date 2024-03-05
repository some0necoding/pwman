/* estring.c - efficient string related functions */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "estring.h"

char *estrcat(char dstend[static 1], char const src[static 1]) {
    while ((*dstend++ = *src++));
    return --dstend;
}

char *estrncat(char dstend[static 1], char const src[static 1], size_t n) {
    while (n-- && (*dstend++ = *src++));
    *dstend = 0;
    return dstend;
}

char *esstrcat(char dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1]) {
    return esstrncat(&dst, dstend, dsize, src, strlen(src));
}

char *esstrncat(char *dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1], size_t n) {
    size_t len = (dstend - *dst) + n;
    if (len >= *dsize) {
        while (len >= *dsize)
            *dsize *= 2;
        *dst = (char *) reallocarray(*dst, *dsize, sizeof(char));
        if (!*dst) {
            fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
            return NULL;
        }
    }
    return estrncat(dstend, src, n);
}
