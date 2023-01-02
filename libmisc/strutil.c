#include <edidentifier.h>
__CIDENT_RCSID(gr_strutil_c,"$Id: strutil.c,v 1.10 2022/12/03 16:33:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strutil.c,v 1.10 2022/12/03 16:33:05 cvsuser Exp $
 * libstr - Misc string utilities.
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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

#if defined(HAVE_STRVERSCMP) && !defined(_GNU_SOURCE)
#if defined(linux) || defined(__CYGWIN__) //FIXME
#define _GNU_SOURCE
#endif
#endif

#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#include <stdarg.h>
#if defined(HAVE_STRVERSCMP)
#include <string.h>                             /* strverscmp() */
#endif

#include <libstr.h>
#include <unistd.h>


/*  Function:           str_chrx
 *      Searches for the first occurrence of the value 'c' in the string pointed to by 'p'.
 *
 *  Parameters:
 *      p -                 Address of the source string.
 *      c -                 Character to search for.
 *      len -               Field address populated with resulting substring length.
 *                          The value shall either represent the length of the matched
 *                          substring excluding the matched character, otherwise the
 *                          length of entire string if the character wasnt located
 *                          excluding the terminating NUL.
 *
 *  Returns:
 *      The str_chrx routine, upon a successful completion, returns a pointer
 *      to the located character. If the character cannot be located, a NULL
 *      is returned.
 */
const char *
str_chrx(const char *p, int c, int *len)
{
    register int t_len = 0;

    for (;; ++p) {
        if (*p == c) {
            if (len) *len = t_len;
            return p;
        }
        if (*p == '\0') {
            if (len) *len = t_len;
            return NULL;
        }
        ++t_len;
    }
    /*NOTREACHED*/
}

/*end*/
