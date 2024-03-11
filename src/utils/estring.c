/* estring.c - efficient string related functions */

#include <bits/pthreadtypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "estring.h"

char *estrcat(char dst[static 1], char const src[static 1]) {
    while (*dst) dst++;
    while ((*dst++ = *src++));
    return --dst;
}

char *estrncat(char dst[static 1], char const src[static 1], size_t n) {
    while (*dst) dst++;
    while (n-- && (*dst++ = *src++));
    if (++n) dst--;
    *dst = '\0';
    return dst;
}

char *esstrcat(char dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1]) {
    return esstrncat(&dst, dstend, dsize, src, strlen(src));
}

char *esstrncat(char *dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1], size_t n) {
    size_t dstend_offset = dstend - *dst;
    size_t len = dstend_offset + n;
    if (len >= *dsize) {
        while (len >= *dsize)
            *dsize *= 2;
        *dst = (char *) reallocarray(*dst, *dsize, sizeof(char));
        if (!*dst) {
            fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
            return NULL;
        }
        dstend = *dst + dstend_offset;
    }
    return estrncat(dstend, src, n);
}
