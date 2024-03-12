#ifndef DYNSTR_H
#define DYNSTR_H

#define DEFAULT_CAP 16

#include <stddef.h>

/**
 * representation of a dynstr header
 */
typedef struct {
    size_t __capacity;
    size_t __length;
    char* __end;            // dynstr end (i.e. first null byte)
} __dynstr_header;

/**
 * dynstr representation
 */
typedef struct {
    __dynstr_header header;
    char *s;                // actual string
} dynstr;

/**
 * Create a new empty dynstr object. The returned object is dynamically
 * allocated and should be freed by the client using dynstr_free().
 *
 * @return a new empty dynstr object
 */
dynstr* dynstr_new();

/**
 * Create a new dynstr object containing s. The returned object is dynamically
 * allocated and should be freed by the client using dynstr_free().
 *
 * @param s     null-terminated string that will be represented by the returned
 *              dynstr.
 * @return a new dynstr containing s if successful, NULL on allocation error
 */
dynstr* dynstr_news(char const s[static 1]);

/**
 * Append a string to the end of a dynstr.
 *
 * @param d     dynstr object
 * @param s     null-terminated string to append after d
 * @return 0 on success, -1 on allocation error.
 */
int dynstr_append(dynstr d[static 1], char const str[static 1]);

/**
 * Append up to n bytes of a string to the end of a dynstr.
 *
 * @param d     dynstr object
 * @param s     null-terminated string to append after d
 * @param n     bytes of s to copy after d
 * @return 0 on success, -1 on allocation error.
 */
int dynstr_appendn(dynstr d[static 1], char const s[static 1], size_t n);

/**
 * Create a copy of the string represented by a dynstr.
 *
 * @param d     dynstr object
 * @return a null-terminated and heap-allocated string representing d if
 *              successful, NULL otherwise.
 */
char* const dynstr_tostr(dynstr d[static 1]);

/**
 * Free a dynstr object.
 *
 * @param d     dynstr object
 */
void dynstr_free(dynstr d[static 1]);

#endif
