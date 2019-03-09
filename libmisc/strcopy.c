#include <edidentifier.h>
__CIDENT_RCSID(gr_strcopy_c,"$Id: strcopy.c,v 1.5 2017/01/29 04:33:31 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strcopy.c,v 1.5 2017/01/29 04:33:31 cvsuser Exp $
 * libstr - String copy utility functions.
 *
 *
 * Copyright (c) 1998 - 2017, Adam Young.
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


/*  Function:           strxcpy
 *      Length delimited copy like strncpy but always NUL terminates the
 *      destination buffer.
 *
 *  Parameters:
 *      dest - Destination buffer.
 *      src - Source buffer.
 *      len - Length of destination buffer, in bytes.
 *
 *  Returns:
 *      Destination buffer.
 */
char *
strxcpy(char *dest, const char *src, int len)
{
    (void) strncpy(dest, src, len);
    if (len > 0) {
        dest[len - 1] = '\0';
    }
    return dest;
}


/*  Function:           strxcat
 *      Length delimited concat like strncat but always nul terminates
 *      the destination buffer.
 *
 *  Parameters:
 *      dest - Destination buffer.
 *      src - Source buffer.
 *      len - Length of destination buffer, in bytes.
 *
 *  Returns:
 *      Destination buffer.
 */
char *
strxcat(char *dest, const char *src, int len)
{
    (void) strncat(dest, src, len);
    if (len > 0) {
        dest[len - 1] = '\0';
    }
    return dest;
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
