#include <edidentifier.h>
__CIDENT_RCSID(gr_str2num_c,"$Id: strnum.c,v 1.5 2017/01/29 04:33:31 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strnum.c,v 1.5 2017/01/29 04:33:31 cvsuser Exp $
 * libstr - String to numeric.
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

#include <libstr.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>


/*  str_num -- reliably convert string value to an integer.

    SYNOPSIS:
        long
        str_num(const char *numstr,
                  long minval, long maxval, const char **errp);

    DESCRIPTION:
        The str_num() function converts the string in nptr to a long
        long value. The str_num() function was designed to
        facilitate safe, robust programming and overcome the
        shortcomings of the atoi(3) and strtol(3) family of interfaces.

        The string may begin with an arbitrary amount of whitespace
        (as determined by isspace(3)) followed by a single optional
        '+' or '-' sign.

        The remainder of the string is converted to a long long value
        according to base 10.

        The value obtained is then checked against the provided
        minval and maxval bounds. If errstr is non-null, str_num()
        stores an error string in *errstr indicating the failure.>

    EXAMPLES:
        Using 'str_num()' correctly is meant to be simpler than the
        alternative functions.

            const char *errstr;
            int value;

            value = str_num(optarg, 1, 64, &errstr);
            if (errstr)
                printf("input error %s: %s", errstr, optarg);

        The above example will guarantee that the value is between 1
        and 64 (inclusive).

    ERRORS:
        If an error occurs, errno is one of the following manifest
        constants.

            o ERANGE    - The given string was out of range.
            o EINVAL    - The given string did not consist solely of digit characters.
            o EINVAL    - The supplied minval was larger than maxval.

        and errstr will be set to one of the following strings:

            o too large - The result was larger than the provided maximum value.
            o too small - The result was smaller than the provided minimum value.
            o invalid   - The string did not consist solely of digit characters.)
*/
long
str_num(const char *numstr, long minval, long maxval, const char **errp)
{
    struct errval {
        const char *errstr;
#define INVALID         1
#define TOOSMALL        2
#define TOOLARGE        3
        int err;

    } ev[4] = {
        { NULL,         0 },
        { "invalid",    EINVAL },
        { "too small",  ERANGE },
        { "too large",  ERANGE },
        };
    long val = 0;
    int error = 0;
    char *ep;

    ev[0].err = errno;
    errno = 0;
    if (minval > maxval) {
        error = INVALID;
    } else {
        val = strtol(numstr, &ep, 10);
        if (numstr == ep || *ep != '\0') {
            error = INVALID;
        } else if ((val == LONG_MIN && errno == ERANGE) || val < minval) {
            error = TOOSMALL;
        } else if ((val == LONG_MAX && errno == ERANGE) || val > maxval) {
            error = TOOLARGE;
        }
    }
    if (errp != NULL) *errp = ev[error].errstr;
    errno = ev[error].err;
    return (error ? 0 : val);
}
/*end*/
