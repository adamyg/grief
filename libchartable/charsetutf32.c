#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetutf32_c,"$Id: charsetutf32.c,v 1.9 2015/02/19 00:17:04 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* Multibyte character - UTF32 utility functionality.
 *
 *
 * Copyright (c) 2010 - 2015, Adam Young.
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

#include <editor.h>
#include "libchartable.h"

static __CINLINE const void *   utf32_decode(int endian, const void *src, const void *cpend, int32_t *result);
static __CINLINE int            utf32_legal(const int32_t ch);


static __CINLINE const void *
utf32_decode(int endian, const void *src, const void *cpend, int32_t *result)
{
    const unsigned char *cursor = src;
    uint32_t ch = 0;

    if (cursor + 3 < (unsigned char *)cpend) {
        if (endian) {                           /* big-endian */
            ch = (((uint32_t)cursor[0] << 24) |
                  ((uint32_t)cursor[1] << 16) |
                  ((uint32_t)cursor[2] << 8 ) |
                   (uint32_t)cursor[3]);
        } else {
            ch = (((uint32_t)cursor[3] << 24) |
                  ((uint32_t)cursor[2] << 16) |
                  ((uint32_t)cursor[1] << 8 ) |
                   (uint32_t)cursor[0]);
        }
        cursor += 4;

    } else if (cursor < (unsigned char *)cpend) {
        if (cursor + 1 == (unsigned char *)cpend) {
            ch = *cursor++;
        } else {
            ch = -1;                            /* invalid */
            ++cursor;
        }

    } else {
        cursor = NULL;                          /* EOL */
    }

    if (result) {
        *result = (int) ch;
    }
    return cursor;
}


static __CINLINE int
utf32_legal(const int32_t ch)
{
    const uint32_t low16 = (ch & 0xffff);

    if (ch < 0 || 0x00fffe == low16 || 0x00ffff == low16 || ch > 0x10fffd ||
            (ch >= UNICODE_HI_SURROGATE_START && ch <= UNICODE_LO_SURROGATE_END) ||
            (ch >= 0x00fdd0 && ch <= 0x00fdef)) {
        return 0;
    }
    return 1;
}


const void *
charset_utf32_decode(int endian, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf32_decode(endian, src, cpend, &result);

    if (!utf32_legal(result)) {
        *cooked = UNICODE_REPLACE;              /* replacement character */
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf32be_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf32_decode(1, src, cpend, &result);

    if (!utf32_legal(result)) {
        *cooked = UNICODE_REPLACE;              /* replacement character */
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf32le_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf32_decode(0, src, cpend, &result);

    if (!utf32_legal(result)) {
        *cooked = UNICODE_REPLACE;              /* replacement character */
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf32_decode_safe(int endian, const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf32_decode(endian, src, cpend, &result);

    if (!utf32_legal(result)) {
        result = UNICODE_REPLACE;               /* replacement character */
    }
    *cooked = result;
    return ret;
}


const void *
charset_utf32be_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf32_decode(1, src, cpend, &result);

    if (!utf32_legal(result)) {
        result = UNICODE_REPLACE;               /* replacement character */
    }
    *cooked = result;
    return ret;
}


const void *
charset_utf32le_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf32_decode(0, src, cpend, &result);

    if (!utf32_legal(result)) {
        result = UNICODE_REPLACE;               /* replacement character */
    }
    *cooked = result;
    return ret;
}


int
charset_utf32_encode(int endian, const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    if (endian) {                               /* big endian */
        cursor[0] = (unsigned char)(ch >> 24);
        cursor[1] = (unsigned char)(ch >> 16);
        cursor[2] = (unsigned char)(ch >> 8);
        cursor[3] = (unsigned char)(ch);
    } else {
        cursor[3] = (unsigned char)(ch >> 24);
        cursor[2] = (unsigned char)(ch >> 16);
        cursor[1] = (unsigned char)(ch >> 8);
        cursor[0] = (unsigned char)(ch);
    }
    return 0;
}


int
charset_utf32be_encode(const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    cursor[0] = (unsigned char)(ch >> 24);
    cursor[1] = (unsigned char)(ch >> 16);
    cursor[2] = (unsigned char)(ch >> 8);
    cursor[3] = (unsigned char)(ch);
    return 4;
}


int
charset_utf32le_encode(const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    cursor[3] = (unsigned char)(ch >> 24);
    cursor[2] = (unsigned char)(ch >> 16);
    cursor[1] = (unsigned char)(ch >> 8);
    cursor[0] = (unsigned char)(ch);
    return 4;
}
/*end*/

