#include <edidentifier.h>
__CIDENT_RCSID(gr_strconvert_c,"$Id: strconvert.c,v 1.11 2022/08/10 14:25:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strconvert.c,v 1.11 2022/08/10 14:25:21 cvsuser Exp $
 * libstr - String convert utility functions.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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


/*  Function:           str_rev
 *      Reverse the order of the characters in the string, pointed to by 's'.
 *
 *  Parameters:
 *      p - Address of the source string.
 *
 *  Returns:
 *      The str_rev routine returns the original string 'p'.
 */
char *
str_rev(char *p)
{
    register char *l = p, *r = p;
    char ch;

    r += (strlen(p) - 1);
    while (r > l) {
        ch = *r;
        *r = *l;
        *l = ch;
        --r; ++l;
    }
    return p;
}


/*  Function:           str_upper
 *      Converts characters contained within the string to uppercase.
 *
 *  Parameters:
 *      p - Address of the source string.
 *
 *   Returns:
 *      The str_rev routine returns the original string 'p'.
 */
char *
str_upper(char *p)
{
    register char *s = p;

    while (*s) {
        *s = (char)toupper(*s);
        ++s;
    }
    return p;
}


/*  Function:           str_lower
 *      Converts to characters contained within the string to lowercase.
 *
 *  Parameters:
 *      p - Address of the source string.
 *
 *   Returns:
 *      The str_rev routine returns the original string 'p'.
 */
char *
str_lower(char *p)
{
    register char *s = p;

    while (*s) {
        *s = (char)tolower(*s);
        ++s;
    }
    return p;
}

/*end*/
