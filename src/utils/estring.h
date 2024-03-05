/* estring.h - efficient string related functions
 *
 * In this and estring.c files are implemented more efficient and safe string
 * related function.
 */

#ifndef ESTRING_H
#define ESTRING_H

#include <stddef.h>

/*
 * More efficient implementation of strcat(). It copies all bytes of src right
 * after dstend, then it returns a pointer to the new end of dst (i.e. the first
 * null byte) to avoid having to traverse it in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrcat().
 *
 * @param dstend the end (i.e. address of the first null byte) of the
 *               destination string
 * @param src    null-terminated string
 */
char *estrcat(char dst[static 1], char const src[static 1]);

/*
 * More efficient implementation of strncat(). It copies at most n bytes of src
 * right after dstend, then it returns a pointer to the new end of dst (i.e.
 * the first null byte) to avoid having to traverse it in every call.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
 *
 * NOTE: this funciton can suffer from buffer overflow. Consider using
 * esstrncat().
 *
 * @param dstend the end (i.e. address of the first null byte) of the
 *               destination string
 * @param src    null-terminated string
 * @param n      number of bytes of src to copy after dstend
 */
char *estrncat(char dst[static 1], char const src[static 1], size_t n);

/*
 * More efficient and safe implementation of strcat(). It copies all bytes of
 * src right after dst, then it returns a pointer to the new end of dst (i.e.
 * the first null byte) to avoid having to traverse it in every call. Also it
 * ensures that the dst buffer is large enough to contain the new string,
 * updating dsize accordingly.
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
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
 * More efficient and safe implementation of strncat(). It copies the first n
 * bytes of src right after dst, then it returns a pointer to the new end of
 * dst (i.e. the first null byte) to avoid having to traverse it in every call.
 * Also it ensures that the dst buffer is large enough to contain the new
 * string, updating dsize accordingly
 * More info at https://www.joelonsoftware.com/2001/12/11/back-to-basics/
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