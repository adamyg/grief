#include <edidentifier.h>
__CIDENT_RCSID(gr_strtrim_c,"$Id: strtrim.c,v 1.7 2022/12/03 16:33:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strtrim.c,v 1.7 2022/12/03 16:33:05 cvsuser Exp $
 * libstr - String trim utilities.
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
#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#include <libstr.h>
#include <unistd.h>

static __CINLINE int    is_white(const int ch);
static __CINLINE int    is_quote(const int ch);

const char *
str_trim(const char *name, int *length)
{
    int len;

    if (name && length && (len = *length) > 0) {
        int quote = 0;

        while (len > 0 && !quote &&
                (is_white(name[0]) || (quote = is_quote(name[0])))) {
            ++name, --len;                      /* trim leading, stop at first quote */
        }

        quote = 0;
        while (len > 0 && !quote &&
                (is_white(name[len - 1]) || (quote = is_quote(name[len - 1])))) {
            --len;                              /* trim trailing, stop at first quote */
        }
        *length = len;
    }
    return name;
}


/*  Function:           is_white
 *      Determine whether a white-space character (ie. space or tab).
 *
 *  Parameters:
 *      c -                 Character value.
 *
 *  Returns:
 *      *true* if a white-space character, otherwise *false*.
 */
static __CINLINE int
is_white(const int ch)
{
    return (' ' == ch || '\t' == ch);
}


static __CINLINE int
is_quote(const int ch)
{
    return ('\'' == ch || '"' == ch);
}

/*end*/
