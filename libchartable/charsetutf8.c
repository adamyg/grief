#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetutf8_c,"$Id: charsetutf8.c,v 1.13 2018/10/01 22:10:53 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* Multibyte character - UTF8 utility functionality.
 *
 *
 * Copyright (c) 2010 - 2018, Adam Young.
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

static __CINLINE int            utf8_illegal(const int32_t ch);
static __CINLINE int            utf8_overlong(const int32_t ch, const size_t length);
static __CINLINE const char *   utf8_decode(const void *src, const void *cpend, int32_t *result);


static __CINLINE int
utf8_illegal(const int32_t ch)
{
    const int32_t low16 = (0xffff & ch);

    if (0xfffe == low16 || 0xffff == low16 ||
            ch <= 0 || ch > UNICODE_MAX ||
            (ch >= UNICODE_HI_SURROGATE_START && ch <= UNICODE_LO_SURROGATE_END)) {
        return TRUE;
    }
    return FALSE;
}


static __CINLINE int
utf8_overlong(const int32_t ch, const size_t length)
{
    if (ch <= 0x80) {
        if (1 != length) return TRUE;
    } else if (ch < 0x800) {
        if (2 != length) return TRUE;
    } else if (ch < 0x10000) {
        if (3 != length) return TRUE;
    } else if (ch < 0x200000) {
        if (4 != length) return TRUE;
    } else if (ch < 0x4000000) {
        if (5 != length) return TRUE;
    } else {
        if (6 != length) return TRUE;
    }
    return FALSE;
}


/*
 *  00000000-01111111  00-7F  0-127     Single-byte encoding (compatible with US-ASCII).
 *  10000000-10111111  80-BF  128-191   Second, third, or fourth byte of a multi-byte sequence.
 *  11000000-11000001  C0-C1  192-193   Overlong encoding: start of 2-byte sequence, but would encode a code point 127.
 *  11000010-11011111  C2-DF  194-223   Start of 2-byte sequence.
 *  11100000-11101111  E0-EF  224-239   Start of 3-byte sequence.
 *  11110000-11110100  F0-F4  240-244   Start of 4-byte sequence.
 *  11110101-11110111  F5-F7  245-247   Restricted by RFC 3629: start of 4-byte sequence for codepoint above 10FFFF.
 *  11111000-11111011  F8-FB  248-251   Restricted by RFC 3629: start of 5-byte sequence.
 *  11111100-11111101  FC-FD  252-253   Restricted by RFC 3629: start of 6-byte sequence.
 *  11111110-11111111  FE-FF  254-255   Invalid: not defined by original UTF-8 specification.
 */
static __CINLINE const char *
utf8_decode(const void *src, const void *cpend, int32_t *result)
{
    register const unsigned char *t_src = (const unsigned char *)src;
    unsigned char ch;
    int32_t ret = 0;
    int remain;

    /*
    //  Bits    Last code point     Byte 1      Byte 2      Byte 3      Byte 4      Byte 5      Byte 6
    //  7       U+007F              0xxxxxxx
    //  11      U+07FF              110xxxxx    10xxxxxx
    //  16      U+FFFF              1110xxxx    10xxxxxx    10xxxxxx
    //  21      U+1FFFFF            11110xxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  26      U+3FFFFFF           111110xx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    //  31      U+7FFFFFFF          1111110x    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx    10xxxxxx
    */
    assert(src < cpend);
    ch = *t_src++;

    if (ch & 0x80) {
                                                /* C0-C1  192-193  Overlong encoding: start of 2-byte sequence. */
        if ((ch & 0xE0) == 0xC0) {              /* C2-DF  194-223  Start of 2-byte sequence. */
            remain = 1;
            ret = ch & 0x1F;

        } else if ((ch & 0xF0) == 0xE0) {       /* E0-EF  224-239  Start of 3-byte sequence. */
            remain = 2;
            ret = ch & 0x0F;

        } else if ((ch & 0xF8) == 0xF0) {       /* F0-F4  240-244  Start of 4-byte sequence. */
            remain = 3;
            ret = ch & 0x07;

        } else if ((ch & 0xFC) == 0xF8) {       /* F8-FB  248-251  Start of 5-byte sequence. */
            remain = 4;
            ret = ch & 0x03;

        } else if ((ch & 0xFE) == 0xFC) {       /* FC-FD  252-253  Start of 6-byte sequence. */
            remain = 5;
            ret = ch & 0x01;

        } else {                                /* invalid continuation (0x80 - 0xbf). */
            ret = -ch;
            goto done;
        }

        while (remain--) {
            if (t_src >= (const unsigned char *)cpend) {
                ret = -ret;
                goto done;
            }
            ch = *t_src++;
            if (0x80 != (0xc0 & ch)) {          /* invalid secondary byte (0x80 - 0xbf). */
                --t_src;
                ret = -ret;
                goto done;
            }
            ret <<= 6;
            ret |= (ch & 0x3f);
        }
    } else {
        ret = ch;
    }

done:;
    *result = ret;
    return (const void *)t_src;
}


const void *
charset_utf8_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    int32_t result = 0;
    const char *ret = utf8_decode(src, cpend, &result);

    if (result <= 0 || utf8_illegal(result) ||
            (ret && utf8_overlong(result, ret - (const char *)src))) {
        *raw = (result < 0 ? -result : result);
        *cooked = UNICODE_REPLACE;              /* replacement character */
        return ret;
    }
    *cooked = *raw = result;
    return ret;
}


const void *
charset_utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked)
{
    int32_t result = 0;
    const char *ret = utf8_decode(src, cpend, &result);

    if (result <= 0 || utf8_illegal(result) ||
            (ret && utf8_overlong(result, ret - (const char *)src))) {
        result = UNICODE_REPLACE;               /* replacement character */
    }
    *cooked = result;
    return ret;
}


int
charset_utf8_length(const int32_t ch)
{
    if ((ch & 0x80) == 0x00) {
        return 1;
    } else if ((ch & 0xe0) == 0xc0) {
        return 2;
    } else if ((ch & 0xf0) == 0xe0) {
        return 3;
    } else if ((ch & 0xf8) == 0xf0) {
        return 4;
    } else if ((ch & 0xfc) == 0xF8) {
        return 5;
    } else if ((ch & 0xfe) == 0xfc) {
        return 6;
    }
    return 1;  /* illegal UTF-8 code */
}


int
charset_utf8_encode(const int32_t ch, void *buffer)
{
    register unsigned char *t_buffer = (unsigned char *)buffer;
    register unsigned t_ch = (unsigned) ch;
    int count = 0;

    if (ch < 0) {
        count = 0;

    } else if (t_ch < 0x80) {
        t_buffer[0] = (unsigned char)t_ch;
        count = 1;

    } else if (t_ch < 0x800) {
        t_buffer[0] = (unsigned char)(0xC0 | ((t_ch >> 6)  & 0x1F));
        t_buffer[1] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 2;

    } else if (t_ch < 0x10000) {
        t_buffer[0] = (unsigned char)(0xE0 | ((t_ch >> 12) & 0xF));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 3;

    } else if (t_ch < 0x200000) {
        t_buffer[0] = (unsigned char)(0xF0 | ((t_ch >> 18) & 0x7));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 4;

    } else if (t_ch < 0x4000000) {
        t_buffer[0] = (unsigned char)(0xF8 | ((t_ch >> 24) & 0x3));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 18) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[4] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 5;

    } else {
        t_buffer[0] = (unsigned char)(0xFC | ((t_ch >> 30) & 0x1));
        t_buffer[1] = (unsigned char)(0x80 | ((t_ch >> 24) & 0x3F));
        t_buffer[2] = (unsigned char)(0x80 | ((t_ch >> 18) & 0x3F));
        t_buffer[3] = (unsigned char)(0x80 | ((t_ch >> 12) & 0x3F));
        t_buffer[4] = (unsigned char)(0x80 | ((t_ch >> 6)  & 0x3F));
        t_buffer[5] = (unsigned char)(0x80 |  (t_ch        & 0x3F));
        count = 6;
    }
    return count;
}
/*end*/
