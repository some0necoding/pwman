#ifndef DYNSTR_H
#define DYNSTR_H

#define DEFAULT_CAP 16

#include <stddef.h>

/**
 * representation of a dynstr header
 */
typedef struct {
    size_t __capacity;      // dynstr capacity
    size_t __length;        // dynstr length
    char* __end;            // dynstr end (i.e. first null byte)
} __dynstr_header;

/**
 * dynstr representation
 */
typedef struct {
    __dynstr_header header;
    char *s;
} dynstr;

/** create a new empty dynstr */
dynstr* dynstr_new();

/** create a new dynstr ontaining s */
dynstr* dynstr_news(char const s[static 1]);

/** append str to the end of d */
int dynstr_append(dynstr d[static 1], char const str[static 1]);

/** append up to n bytes of str to the end of d */
int dynstr_appendn(dynstr d[static 1], char const s[static 1], size_t n);

/** return a copy of the string represented by d */
char const* dynstr_tostr(dynstr d[static 1]);

/** free a dynstr object */
void dynstr_free(dynstr d[static 1]);

#endif
