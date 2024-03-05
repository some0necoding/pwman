/* estring.h - efficient string related functions
 *
 * In this and estring.c files are implemented more efficient and safe string
 * related function.
 */

#ifndef ESTRING_H
#define ESTRING_H

#include <stddef.h>

/*
 * efficient strcat(). It copies all bytes of src right after dstend, then it
 * returns a pointer to the new dstend (i.e. the first null byte).
 * This avoids having to traverse the destination string in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrcat().
 *
 * @param dstend the end (i.e. address of the first null byte) of the
 *               destination string
 * @param src    null-terminated string
 * @return the new dstend
 */
char *estrcat(char dst[static 1], char const src[static 1]);

/*
 * efficient strncat(). It copies at most n bytes of src right after dstend,
 * followed by a null character, then it returns a pointer to the new dstend
 * (i.e. the first null byte).
 * This avoids having to traverse the destination string in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrncat().
 *
 * @param dstend the end (i.e. address of the first null byte) of the
 *               destination string
 * @param src    null-terminated string
 * @param n      number of bytes of src to copy after dstend
 * @return the new dstend
 */
char *estrncat(char dst[static 1], char const src[static 1], size_t n);

/*
 * efficient and safe strcat(). It copies all bytes of src right after dst,
 * followed by a null character, then it returns a pointer to the new dstend
 * (i.e. the first null byte).
 * This avoids having to traverse the dst in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 * It also ensures that the dst buffer is large enough to contain the new 
 * string, updating dsize accordingly.
 *
 * @param dst    null-terminated string
 * @param dstend end (i.e. address of first null byte) of dst
 * @param dsize  allocated size of dst
 * @param src    null-terminated string
 * @returns      the new dstend if successful, NULL in case of allocation error.
 */
char *esstrcat(char dst[static 1], char dstend[static 1], size_t dsize[static 1],
        char const src[static 1]);

/*
 * efficient and safe strncat(). It copies the first n bytes of src right after
 * dst, followed by a null character, then it returns a pointer to the new
 * dstend (i.e. the first null byte).
 * This avoids having to traverse dst in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 * It also ensures that the dst buffer is large enough to contain the new
 * string, updating dsize accordingly
 *
 * @param dst    null-terminated string
 * @param dstend end (i.e. address of first null byte) of dst
 * @param dsize  allocated size of dst
 * @param src    null-terminated string
 * @param n      number of bytes of src to copy after dstend
 * @returns      the new dstend if successful, NULL in case of allocation error.
 */
char *esstrncat(char *dst[static 1], char dstend[static 1], size_t dsize[static 1],
        char const src[static 1], size_t n);

#endif
