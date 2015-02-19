/* $Id: libsupport.c,v 1.2 2014/08/14 01:57:58 ayoung Exp $
 *
 * libmandoc support functions
 *
 * Copyright (c) 2014, Adam Young.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ==end==
 */

#include "config.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/*
 *  asprintf()
 *  vasprintf()
 *
 */
static int libsupport;

#if defined(NEED_ASPRINTF)

#ifndef VA_COPY
# if defined(HAVE_VA_COPY) || defined(va_copy)
        /* ISO C99 and later */
#define VA_COPY(__dst, __src)   va_copy(__dst, __src)
# elif defined(HAVE___VA_COPY) || defined(__va_copy)
        /* gnu */
#define VA_COPY(__dst, __src)   __va_copy(__dst, __src)
# elif defined(__WATCOMC__)
        /* Older Watcom implementations */
#define VA_COPY(__dst, __src)   memcpy((__dst), (__src), sizeof (va_list))
# else
#define VA_COPY(__dst, __src)   (__dst) = (__src)
# endif
#endif  /*VA_COPY*/


int
asprintf(char **str, const char *fmt, ...)
{
    va_list ap;
    int size;

    va_start(ap, fmt);
    size = vasprintf(str, fmt, ap);
    va_end(ap);
    return size;
}


int
vasprintf(char **str, const char *fmt, va_list ap)
{
    va_list tap;
    char *buf = NULL;
    int osize, size;

    VA_COPY(tap, ap);
    osize = vsnprintf(NULL, 0, fmt, tap);
    if (osize < 0 ||
            (NULL == (buf = (char *)malloc(osize + 16)))) {
        size = -1;
    } else {
        size = vsprintf(buf, fmt, ap);
        assert(size == osize);
    }
    *str = buf;
    va_end(tap);
    va_end(ap);
    return size;
}
#endif  /*NEED_ASPRINTF*/


/*
 *  isblank()
 *
 */
#if defined(NEED_ISBLANK)
int
isblank(int ch)
{
    return (' ' == ch || '\t' == ch);
}
#endif  /*NEED_ISBLANK*/
