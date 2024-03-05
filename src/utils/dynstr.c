/* dynstr.c - dynamic strings
 *
 * Implementation of dynamic strings. This means not having anymore to check 
 * for size and calling realloc.
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynstr.h"
#include "estring.h"

/**
 * Returns a new empty dynstr object of capacity cap. This function is meant 
 * for internal use only.
 */
static dynstr* __dynstr_new(size_t cap) {
    dynstr* d = calloc(1, sizeof(dynstr));
    if (!d) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }
    d->header.__capacity = cap;
    d->s = (char *) calloc(d->header.__capacity, sizeof(char));
    if (!d->s) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
        return NULL;
    }
    d->header.__length = 0;
    d->header.__end = d->s;
    return d;
}

/**
 * Returns a new empty dynstr object. If cap is < 0 default capacity is used.
 * The returned object is dynamically allocated and should be freed by the 
 * client using dynarr_free().
 */
dynstr* dynstr_new() {
    return __dynstr_new(DEFAULT_CAP);
}

/**
 * Returns a new dynstr object containing s. The returned object is dynamically 
 * allocated and should be freed by the client using dynarr_free().
 *
 * @param s null-terminated string that will be copied inside the new dynarr
 * @return the new dynarr if successful, NULL on allocation error
 */
dynstr* dynstr_news(char const s[static 1]) {
    size_t slen = strlen(s);
    dynstr *d = __dynstr_new(slen * 2);
    d->header.__end = estrncat(d->header.__end, s, slen);
    d->header.__length += slen;
    return d;
}

/**
 * This function appends the null-terminated string str to the end of d.
 * Obviously d should have been already allocated with dynarr_new().
 *
 * @param d     dynarr object
 * @param s     null-terminated string to append after d
 * @return 0 on success, -1 on allocation error.
 */
int dynstr_append(dynstr d[static 1], char const s[static 1]) {
    return dynstr_appendn(d, s, strlen(s));
}

/**
 * This function appends up to n bytes of the null-terminated string str to the 
 * end of d. Obviously d should have been already allocated with dynarr_new().
 *
 * @param d     dynarr object
 * @param s     null-terminated string to append after d
 * @param n     bytes of s to copy after d
 * @return 0 on success, -1 on allocation error.
 */
int dynstr_appendn(dynstr d[static 1], char const s[static 1], size_t n) {
    d->header.__end = esstrncat(&d->s, d->header.__end, &d->header.__capacity, s, n);
    if (!d->header.__end) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
        return -1;
    }
    d->header.__length = d->header.__end - d->s;
    return 0;
}

/**
 * This function returns a copy of the string represented by d. The returned
 * string is heap allocated and null-terminated.
 */
char const* dynstr_tostr(dynstr d[static 1]) {
    return strdup(d->s);
}

/**
 * Free a dynstr object. This function should be used in place of free().
 */
void dynstr_free(dynstr d[static 1]) {
    free(d->s);
    free(d);
}

