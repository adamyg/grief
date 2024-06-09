#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetutf16_c,"$Id: charsetutf16.c,v 1.14 2024/04/17 16:00:28 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* Multibyte character - UTF16 utility functionality.
 *
 *
 * Copyright (c) 2010 - 2024, Adam Young.
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

static __CINLINE const void *   utf16_decode(int endian, const void *src, const void *cpend, int32_t *result);
static __CINLINE int            utf16_legal(const int32_t ch);


static __CINLINE const void *
utf16_decode(int endian, const void *src, const void *cpend, int32_t *result)
{
    const unsigned char *cursor = src;
    uint32_t ch = 0;

    if ((cursor + 1) < (unsigned char *)cpend) {
        if (endian) {                           /* bigendian - primary character */
            ch = (((uint32_t)cursor[0] << 8) |
                   (uint32_t)cursor[1]);
        } else {
            ch = (((uint32_t)cursor[1] << 8) |
                   (uint32_t)cursor[0]);
        }
        cursor += 2;

        if (ch >= UNICODE_HI_SURROGATE_START && ch <= UNICODE_HI_SURROGATE_END) {
            register uint32_t ch2;              /* character surrogates */

            if ((cursor + 1) < (unsigned char *)cpend) {
                if (endian) {                   /* big-endian */
                    ch2 = (((uint32_t)cursor[0] << 8) |
                            (uint32_t)cursor[1]);
                } else {
                    ch2 = (((uint32_t)cursor[1] << 8) |
                            (uint32_t)cursor[0]);
                }

                if (ch2 >= UNICODE_LO_SURROGATE_START && ch2 <= UNICODE_LO_SURROGATE_END) {
                    ch = ((ch - UNICODE_HI_SURROGATE_START) * 0x400) +
                        ((ch2 - UNICODE_LO_SURROGATE_START) + 0x10000);
                    cursor += 2;

                } else {
                    ch = -1;                    /* invalid */
                }
            }
        }

    } else if (cursor < (unsigned char *)cpend) {
        ch = -1;                                /* invalid */
        ++cursor;

    } else {
        cursor = NULL;                          /* EOL */
    }

    if (result) {
        *result = (int) ch;
    }
    return cursor;
}


static __CINLINE int
utf16_legal(const int32_t ch)
{
    const uint32_t low16 = (ch & 0xffff);

    if (0x00fffe == low16 || 0x00ffff == low16 || ch > 0x10fffd ||
            (ch >= UNICODE_HI_SURROGATE_START && ch <= UNICODE_LO_SURROGATE_END) ||
            (ch >= 0x00fdd0 && ch <= 0x00fdef)) {
        return 0;
    }
    return 1;
}
 

const void *
charset_utf16_decode(int endian, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf16_decode(endian, src, cpend, &result);

    if (! utf16_legal(result)) {
        *cooked = UNICODE_REPLACE;
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf16be_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf16_decode(1, src, cpend, &result);

    if (! utf16_legal(result)) {
        *cooked = UNICODE_REPLACE;
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf16le_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf16_decode(0, src, cpend, &result);

    if (! utf16_legal(result)) {
        *cooked = UNICODE_REPLACE;
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf16_decode_safe(int endian, const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf16_decode(endian, src, cpend, &result);

    if (! utf16_legal(result)) {
        result = UNICODE_REPLACE;
    }
    *cooked = result;
    return ret;
}


const void *
charset_utf16be_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf16_decode(1, src, cpend, &result);

    if (! utf16_legal(result)) {
        result = UNICODE_REPLACE;
    }
    *cooked = result;
    return ret;
}


const void *
charset_utf16le_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const void *ret = utf16_decode(0, src, cpend, &result);

    if (! utf16_legal(result)) {
        result = UNICODE_REPLACE;
    }
    *cooked = result;
    return ret;
}


int
charset_utf16_encode(int endian, const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    if (endian) {                               /* big-endian */
        cursor[0] = (unsigned char)(ch >> 8);
        cursor[1] = (unsigned char)(ch);
    } else {
        cursor[1] = (unsigned char)(ch >> 8);
        cursor[0] = (unsigned char)(ch);
    }
    return 2;
}


int
charset_utf16be_encode(const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    cursor[0] = (unsigned char)(ch >> 8);
    cursor[1] = (unsigned char)(ch);
    return 2;
}


int
charset_utf16le_encode(const int32_t ch, void *buffer)
{
    unsigned char *cursor = (unsigned char *)buffer;

    cursor[1] = (unsigned char)(ch >> 8);
    cursor[0] = (unsigned char)(ch);
    return 2;
}

/*end*/
