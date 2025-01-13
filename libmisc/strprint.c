#include <edidentifier.h>
__CIDENT_RCSID(gr_strprint_c,"$Id: strprint.c,v 1.12 2025/01/13 16:06:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strprint.c,v 1.12 2025/01/13 16:06:38 cvsuser Exp $
 * libstr - String print utilities.
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

#if defined(linux) || defined(__CYGWIN__) //FIXME
#define _GNU_SOURCE
#endif

#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#if defined(HAVE_STRVERSCMP)
#include <string.h>                             /* strverscmp() */
#endif
#include <stdarg.h>

#include <libstr.h>
#include <unistd.h>

static void                 fatal(const char *msg);

/*  Function:           sxprintf
 *      Length limited sprintf.
 *
 *  Parameters:
 *      buf - Destination buffer.
 *      size - Buffer length, in bytes.
 *      fmt - Message format specification.
 *      ... - Arguments.
 *
 *  Returns:
 *      Number of bytes printed. In the event size was exceeded, the return value
 *      represents the number of bytes which would have been written.
 */
int
sxprintf(char *buf, int size, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsxprintf(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}


int
sxprintf0(char *buf, int size, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsxprintf0(buf, size, fmt, ap);
    va_end(ap);
    return ret;
}


/*  Function:           vsxprintf
 *      Length limited vsprintf.
 *
 *  Parameters:
 *      buf - Destination buffer.
 *      size - Buffer length, in bytes.
 *      fmt - Message format specification.
 *      ap - Arguments.
 *
 *  Returns:
 *      Number of bytes printed; not including the nul terminator. In the event size was
 *      exceeded, the return value represents the number of bytes which would have been
 *      written if the buffer was correctly sized; ie, if the return is greater then size
 *      the resulting buffer was truncated.
 */
int
vsxprintf(char *buf, int size, const char *fmt, va_list ap)
{
    int ret;

#if defined(__GNUC__) || defined(HAVE_VSNPRINTF)
    ret = vsnprintf(buf, size, fmt, ap);
#elif defined(_WIN32) || defined(HAVE__VSNPRINTF)
    ret = _vsnprintf(buf, size, fmt, ap);
#else
    ret = vsprintf(buf, fmt, ap);
#endif
    if (ret < 0) {          /* warning, a few implementations return -1 on a format/overflow conditions */
        fatal("[v]sxprintf underflow");
    } else if (ret > size) {
        fatal("[v]sxprintf overflow");
    }
    return ret;
}


/*  Function:           vsxprintf0
 *      Length limited vsprintf with implied null-termination of the buffer.
 *
 *  Parameters:
 *      buf - Destination buffer.
 *      size - Buffer length, in bytes.
 *      fmt - Message format specification.
 *      ap - Arguments.
 *
 *  Returns:
 *      Number of bytes printed; not including the nul terminator. In the event size was
 *      exceeded, the return value represents the number of bytes which would have been
 *      written if the buffer was correctly sized; ie, if the return is greater then size
 *      the resulting buffer was truncated.
 */
int
vsxprintf0(char *buf, int size, const char *fmt, va_list ap)
{
    int ret;

#if defined(__GNUC__) || defined(HAVE_VSNPRINTF)
    ret = vsnprintf(buf, size, fmt, ap);
#elif defined(_WIN32) || defined(HAVE__VSNPRINTF)
    ret = _vsnprintf(buf, size, fmt, ap);
#else
    ret = vsprintf(buf, fmt, ap);
#endif
    if (ret < 0) {          /* warning, a few implementations return -1 on a format/overflow conditions */
        fatal("[v]sxprintf0 underflow");
    } else if (ret > size) {
        fatal("[v]sxprintf0 overflow");
    }
    if (ret && size > 0) {
        buf[size - 1] = 0;
    }
    return ret;
}


#if (TODO)
int
axprintf(char **strp, const char *fmt, ...)
{
}


int
vaxprintf(char **strp, const char *fmt, va_list ap)
{
}
#endif


static void
fatal(const char *msg)
{
    fprintf(stderr, "%s", msg);
    assert(0);
    abort();
}

/*end*/
