#include <edidentifier.h>
__CIDENT_RCSID(gr_strcopy_c,"$Id: strcopy.c,v 1.11 2025/01/13 16:06:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strcopy.c,v 1.11 2025/01/13 16:06:38 cvsuser Exp $
 * libstr - String copy utility functions.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <config.h>
#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#include <libstr.h>
#include <unistd.h>

#if defined(HAVE__MEMCCPY)
#define DO_MEMCCPY _memccpy
#elif defined(HAVE_MEMCCPY)
#define DO_MEMCCPY memccpy
#endif


/*  Function:           strxcpy
 *      Length delimited copy like strncpy but always NUL terminates the destination buffer.
 *
 *  Parameters:
 *      dst - Destination buffer.
 *      src - Source buffer.
 *      len - Length of destination buffer, in bytes.
 *
 *  Note:
 *      strxcpy() behaves like strlcpy().
 *
 *      Unlike strncpy, which if count is greater than the length of strSource, the destination
 *      string is padded with null characters up to length count, strings are only terminated
 *      not zero padded.
 *
 *  Returns:
 *      Destination buffer.
 */
char *
strxcpy(char *dst, const char *src, int len)
{
#if defined(DO_MEMCCPY)
    if (len--) {    /* reserve nul */
        if (NULL == DO_MEMCCPY(dst, src, 0, len)) {
            dst[len] = 0;                       // NUL not encountered.
        }
    }

#elif (0)
    if (len--) {    /* reserve nul */
        const int srclen = strlen(src);
        if (srclen < len) {
            (void) memcpy(dst, src, srclen + 1 /* + nul*/);

        } else {
            (void) memcpy(dst, src, len);
            dst[len] = '\0'; /*nul terminate*/
        }
    }

#else
    if (len) {      /* space available */
        register unsigned count = (unsigned)(len - 1);
        register const char *in = src;
        register char *out = dst;

        while (count && (*out++ = *in++) != 0) {
            --count;                            // copy, length limited.
        }
        *out = 0;                               // terminate.
    }
#endif

    return dst;
}


/*  Function:           strxcat
 *      Length delimited concat similar to strncat() excepts limits the destination length.
 *
 *  Parameters:
 *      dst - Destination buffer.
 *      src - Source buffer.
 *      len - Maximum number of characters within the destination, including nul.
 *
 *  Note:
 *      strlcat() behaves like strlcat().
 *
 *  Returns:
 *      Destination buffer address.
 */
char *
strxcat(char *dst, const char *src, int len)
{
    if (--len) {
#if defined(DO_MEMCCPY)
        const size_t length = strlen(dst);

        if (NULL == DO_MEMCCPY(dst + length, src, 0, len - length)) {
            dst[len] = 0;                       // NUL not encountered.
        }

#else
        char *cursor = dst;
        const size_t length = strlen(cursor);

        if (length < len) {
            cursor += length;
            for (len -= length; len--;) {
                if (0 == (*cursor++ = *src++)) {
                    return dst;
                }
            }
            *cursor = '\0';                     // terminate.
        }
#endif
    }
    return dst;
}


/*  Function:           str_cpy
 *      Overlapping region safe strcpy().
 *
 *  Parameters:
 *      dst - Destination
 *      src - Source.
 *
 *  Returns:
 *      nothing
 */
void
str_cpy(char *dst, const char *src)
{
    if (dst != src) {
    /*
     *  if (src < dst) {                        // copy from end
     *      size_t len = strlen(src) + 1;
     *
     *      for (src += len, dst += len; len; --len) {
     *          *--dst = *--src;
     *      }
     *  } else if (src != dst) {                // copy from front
     *      char ch;
     *      do {
     *          ch = *src++;
     *          *dst++ = ch;
     *      } while (ch);
     *  }
     */
        memmove(dst, src, strlen(src) + 1);
    }
}

/*end*/
