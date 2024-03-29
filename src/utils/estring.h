/* estring.h - efficient string related functions
 *
 * In this and estring.c files are implemented more efficient and safe string
 * related function.
 */

#ifndef ESTRING_H
#define ESTRING_H

#include <stddef.h>

/*
 * efficient strcat(). It copies all bytes of src right after the end of dst,
 * then it returns a pointer to the new end of dst (first null byte). This
 * avoids having to traverse the destination string in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrcat().
 *
 * @param dst    null-terminated destination string
 * @param src    null-terminated source string
 * @return the new end of dst
 */
char *estrcat(char dst[static 1], char const src[static 1]);

/*
 * efficient strncat(). It copies at most n bytes of src right after the end of
 * dst, followed by a null character, then it returns a pointer to the new end
 * of dst (first null byte). This avoids having to traverse the destination
 * string in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrncat().
 *
 * @param dst    null-terminated destination string
 * @param src    null-terminated source string
 * @param n      number of bytes of src to copy after dst
 * @return the new end of dst
 */
char *estrncat(char dst[static 1], char const src[static 1], size_t n);

/*
 * efficient and safe strcat(). It copies all bytes of src right after dstend,
 * followed by a null character, then it returns a pointer to the new dstend
 * (first null byte of dst). This avoids having to traverse the dst in
 * every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 * It also ensures that the dst buffer is large enough to contain the new
 * string, updating dsize accordingly.
 *
 * @param dst    null-terminated destination string
 * @param dstend end of dst
 * @param dsize  allocated size of dst
 * @param src    null-terminated source string
 * @returns      the new dstend if successful, NULL in case of allocation error.
 */
char *esstrcat(char dst[static 1], char dstend[static 1], size_t dsize[static 1],
        char const src[static 1]);

/*
 * efficient and safe strncat(). It copies the first n bytes of src right after
 * dstend, followed by a null character, then it returns a pointer to the new
 * dstend (first null byte of dst). This avoids having to traverse dst in
 * every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 * It also ensures that the dst buffer is large enough to contain the new
 * string, updating dsize accordingly.
 *
 * @param dst    null-terminated destination string
 * @param dstend end of dst
 * @param dsize  allocated size of dst
 * @param src    null-terminated source string
 * @param n      number of bytes of src to copy after dstend
 * @returns      the new dstend if successful, NULL in case of allocation error.
 */
char *esstrncat(char *dst[static 1], char dstend[static 1], size_t dsize[static 1],
        char const src[static 1], size_t n);


/**
 * trim in place all occurencies of the specified character on the right of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 * @param c character to trim
 */
void trimcr(char* s, char c);


/**
 * trim in place all occurencies of the specified character on the left of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 * @param c character to trim
 */
void trimcl(char* s, char c);


/**
 * trim in place all occurencies of the specified character both on the left and
 * on the right of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 * @param c character to trim
 */
void trimc(char* s, char c);


/**
 * trim in place all whitespace characters (i.e. the ones recognised by @ref
 * isspace) on the right of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 */
void trimr(char* s);


/**
 * trim in place all whitespace characters (i.e. the ones recognised by @ref
 * isspace) on the left of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 */
void triml(char* s);


/**
 * trim in place all whitespace characters (i.e. the ones recognised by @ref
 * isspace) both on the left and on the right of s.
 *
 * @param s a null-terminated string. It can be both statically and dynamically
 *          allocated.
 */
void trim(char* s);


#endif
