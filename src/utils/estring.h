#ifndef ESTRING_H
#define ESTRING_H

#include <stddef.h>

/** efficient strcat() */
char *estrcat(char dst[static 1], char const src[static 1]);

/** efficient strncat() */
char *estrncat(char dst[static 1], char const src[static 1], size_t n);

/** efficient and safe strcat() */
char *esstrcat(char dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1]);

/** efficient and safe strncat() */
char *esstrncat(char *dst[static 1], char dstend[static 1], size_t dsize[static 1], 
        char const src[static 1], size_t n);

#endif
