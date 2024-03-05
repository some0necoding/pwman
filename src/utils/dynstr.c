/* dynstr.c - dynamic strings
 *
 * Implementation of dynamic strings. This means not having anymore to check 
 * for size and calling realloc.
 *
 * All dynstr objects must be created using one function between dynstr_new() 
 * and dynstr_news() and must be freed with dynstr_free() after their use.
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

dynstr* dynstr_new() {
    return __dynstr_new(DEFAULT_CAP);
}

dynstr* dynstr_news(char const s[static 1]) {
    size_t slen = strlen(s);
    dynstr *d = __dynstr_new(slen * 2);
    d->header.__end = estrncat(d->header.__end, s, slen);
    d->header.__length += slen;
    return d;
}

int dynstr_append(dynstr d[static 1], char const s[static 1]) {
    return dynstr_appendn(d, s, strlen(s));
}

int dynstr_appendn(dynstr d[static 1], char const s[static 1], size_t n) {
    d->header.__end = esstrncat(&d->s, d->header.__end, &d->header.__capacity, s, n);
    if (!d->header.__end) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__);
        return -1;
    }
    d->header.__length = d->header.__end - d->s;
    return 0;
}

char const* dynstr_tostr(dynstr d[static 1]) {
    return strdup(d->s);
}

void dynstr_free(dynstr d[static 1]) {
    free(d->s);
    free(d);
}
