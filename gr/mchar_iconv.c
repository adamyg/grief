#include <edidentifier.h>
__CIDENT_RCSID(gr_mchar_iconv_c,"$Id: mchar_iconv.c,v 1.19 2014/11/16 17:28:43 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mchar_iconv.c,v 1.19 2014/11/16 17:28:43 ayoung Exp $
 * Character-set conversion/mapping interface and adapters.
 *
 *
 * Copyright (c) 1998 - 2014, Adam Young.
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <eddebug.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "debug.h"
#include "../libvfs/vfs.h"

#include <assert.h>

#if defined(HAVE_LOCALE_H)
#include <locale.h>
#endif

#if defined(HAVE_LANGINFO_CODESET)
/*TODO*/
#endif

#if defined(HAVE_LIBICU)
#include <unicode/ucnv.h>                       /* libicu */
#endif

#if defined(HAVE_APRICONV) || defined(HAVE_LIBICONV)
#if defined(HAVE_APR_ICONV_H)
#include <apr_iconv.h>                          /* libapriconv */
#endif

#if defined(WIN32)
#if (!defined(HAVE_ICONV_H) || defined(GNUWIN32_LIBICONV)) && \
        !defined(WIN32_DYNAMIC_ICONV)
#define WIN32_DYNAMIC_ICONV                     /* dynamic loader */
#endif
#if defined(WIN32_DYNAMIC_ICONV)
#define  WIN32_ICONV_MAP
#include <win32_iconv.h>
#endif
#endif  /*WIN32*/

#if !defined(WIN32_DYNAMIC_ICONV)
#if defined(HAVE_ICONV_H)
#include <iconv.h>                              /* libiconv/glibc */
#elif defined(HAVE_LIBRECODE)
#include <recodeext.h>
#endif
#endif
#endif

#include "mchar.h"                              /* public header */
#include "../libchartable/libchartable.h"
#include "../libchartable/iconv_stream.h"

static const void *             uchar_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             uchar_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      uchar_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);
static int                      uchar_length(mchar_iconv_t *ic, const int32_t ch);

static const void *             utf8_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             utf8_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      utf8_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);
static int                      utf8_length(mchar_iconv_t *ic, const int32_t ch);

static const void *             utf16be_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             utf16be_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      utf16be_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);

static const void *             utf16le_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             utf16le_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      utf16le_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);

static const void *             utf32be_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             utf32be_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      utf32be_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);

static const void *             utf32le_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             utf32le_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      utf32le_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);

static int                      utf16_length(mchar_iconv_t *ic, const int32_t ch);
static int                      utf32_length(mchar_iconv_t *ic, const int32_t ch);

static mchar_iconv_t *          charset_open(const char *encoding);
static void                     charset_close(mchar_iconv_t *ic);
static const void *             charset_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             charset_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      charset_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);
static int                      charset_length(mchar_iconv_t *ic, const int32_t ch);

#if defined(HAVE_LIBICU)
static mchar_iconv_t *          icu_open(const char *encoding);
static void                     icu_close(mchar_iconv_t *ic);
static const void *             icu_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
static const void *             icu_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked);
static int                      icu_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer);
static int                      icu_length(mchar_iconv_t *ic, const int32_t ch);
static mchar_istream_t *        icu_stream_open(mchar_iconv_t *ic, int fd, const char *filename, int direction);
static void                     icu_stream_close(mchar_iconv_t *ic, mchar_istream_t *is);
#endif

#if defined(HAVE_APRICONV) || defined(HAVE_LIBICONV) || \
            defined(WIN32_DYNAMIC_ICONV)
static mchar_iconv_t *          icnv_open(const char *encoding);
static void                     icnv_close(mchar_iconv_t *ic);
static mchar_istream_t *        icnv_stream_open(struct mchar_iconv *ic, int fd, const char *filename, int direction);
static void                     icnv_stream_close(mchar_iconv_t *ic, mchar_istream_t *is);
#endif

extern void                     charset_iconv_init(void);       /* FIXME/XXX */

static mchar_iconv_t            x_iconv_internal[] = {
            { 0, "binary", 1,
                    uchar_decode,
                    uchar_decode_safe,
                    uchar_encode,
                    uchar_length,
            },
            { 0, MCHAR_UTF8, 1,
                    utf8_decode,
                    utf8_decode_safe,
                    utf8_encode,
                    utf8_length,
            },
            { 0, MCHAR_UTF16BE, 2,
                    utf16be_decode,
                    utf16be_decode_safe,
                    utf16be_encode,
                    utf16_length,
            },
            { 0, MCHAR_UTF16LE, 2,
                    utf16le_decode,
                    utf16le_decode_safe,
                    utf16le_encode,
                    utf16_length,
            },
            { 0, MCHAR_UTF32BE, 4,
                    utf32be_decode,
                    utf32be_decode_safe,
                    utf32be_encode,
                    utf32_length,
            },
            { 0, MCHAR_UTF32LE, 4,
                    utf32le_decode,
                    utf32le_decode_safe,
                    utf32le_encode,
                    utf32_length,
            }
        };


/*  Function:           mchar_iconv_open
 *      Open a conversion session, allowing conversion between an external encoding and Unicode.
 *
 *  Parameters:
 *      encoding -          Encoding.
 *
 *  Returns:
 *      Handle.
 */
mchar_iconv_t *
mchar_iconv_open(const char *encoding)
{
    static unsigned once = 0;
    mchar_iconv_t *ic;
    unsigned i;

    if (0 == once) {				/* runtime initialisation */
        ++once;
	for (i = 0, ic = x_iconv_internal; i < (sizeof(x_iconv_internal)/sizeof(x_iconv_internal[0])); ++i, ++ic) {
            ic->ic_magic = MCHAR_ICONV_MAGIC;
        }
        charset_iconv_init();
    }

    if (encoding && *encoding) {
        int encodinglen;

        encoding = mchar_vim_strip(encoding);
        encodinglen = (int)strlen(encoding);
        if ('+' == *encoding) {                 /* special, force use of iconv interface */
            ++encoding;
        } else {
            for (i = 0, ic = x_iconv_internal; i < (sizeof(x_iconv_internal)/sizeof(x_iconv_internal[0])); ++i, ++ic) {
                if (0 == charset_compare(ic->ic_encoding, encoding, encodinglen)) {
                    trace_log("iconv_open(%s) = <%s>\n", encoding, ic->ic_encoding);
                    return ic;
                }
            }

            if (NULL != (ic = charset_open(encoding))) {
                trace_log("iconv_open(%s) = charset\n", encoding);
                return ic;
            }
        }

#if defined(FIXME) && defined(HAVE_LIBICU)      /* ICU, external to utf8 */
        if (NULL != (ic = icu_open(encoding))) {
            trace_log("iconv_open(%s) = ICU\n", encoding);
            return ic;
        }
#endif

#if defined(HAVE_APRICONV) || defined(HAVE_LIBICONV) || \
            defined(WIN32_DYNAMIC_ICONV)        /* iconv, external to utf8 */
        if (NULL != (ic = icnv_open(encoding))) {
            trace_log("iconv_open(%s) = CNV\n", encoding);
            return ic;
        }
#endif

        /*
         *  command line tools, via pipe??
         *      iconv
         *      recode
         *      enconv/enca
         *      cstocs
         */
    }

    trace_log("iconv_open(%s) = <%s>\n", (encoding ? encoding : "NULL"),
        x_iconv_internal[0].ic_encoding);
    return x_iconv_internal;
}


/*  Function:           mchar_iconv_close
 *      Close a conversion session.
 *
 *  Parameters:
 *      iconv -             Conversion handle.
 *
 *  Returns:
 *      nothing
 */
void
mchar_iconv_close(mchar_iconv_t *ic)
{
    if (ic) {
        assert(MCHAR_ICONV_MAGIC == ic->ic_magic);
        if (ic->ic_close) {
            ic->ic_close(ic);
            chk_free((void *)ic);
        }
    }
}


mchar_istream_t *
mchar_stream_open(mchar_iconv_t *ic, int fd, const char *filename, const char *mode)
{
    if (ic && ic->ic_stream_open) {
        assert(MCHAR_ICONV_MAGIC == ic->ic_magic);
        if (mode) {
            if ('r' == mode[0]) {               /* read */
                return (ic->ic_stream_open)(ic, fd, filename, 0);

            } else if ('w' == mode[0]) {        /* write */
                return (ic->ic_stream_open)(ic, fd, filename, 1);
            }
        }
    }
    return NULL;
}


void
mchar_stream_push(mchar_istream_t* is, const char *buffer, size_t buflen)
{
    if (is) {
        assert(MCHAR_ISTREAM_MAGIC == is->is_magic);
        if (0 == is->is_direction) {
            if (buffer && buflen) {
                is->is_buffer = buffer;
                is->is_buflen = buflen;
            }
        }
    }
}


size_t
mchar_stream_read(mchar_istream_t *is, char *buffer, size_t buflen, size_t *inbytes)
{
    size_t cnt = 0;

    if (is) {
        assert(MCHAR_ISTREAM_MAGIC == is->is_magic);
        if (0 == is->is_direction) {
            is->is_iobytes = 0;
            cnt = iconv_stream_read((iconv_stream_t *)is->is_istream, buffer, buflen);
            if (inbytes) *inbytes = is->is_iobytes;
        }
    }
    return cnt;
}


size_t
mchar_stream_write(mchar_istream_t *is, const char *buffer, size_t buflen, size_t *outbytes)
{
    size_t cnt = 0;

    if (is) {
        assert(MCHAR_ISTREAM_MAGIC == is->is_magic);
        if (1 == is->is_direction) {
            is->is_iobytes = 0;
            cnt = iconv_stream_write((iconv_stream_t *)is->is_istream, buffer, buflen);
            if (outbytes) *outbytes = is->is_iobytes;
        }
    }
    return cnt;
}


void
mchar_stream_close(mchar_istream_t *is)
{
    if (is) {
        mchar_iconv_t *ic;

        assert(MCHAR_ISTREAM_MAGIC == is->is_magic);
        if (NULL != (ic = is->is_iconv)) {
            if (ic->ic_stream_close) {
                (ic->ic_stream_close)(ic, is);
            }
        }
        chk_free((void *)is);
    }
}


static ssize_t
stream_rdfn(void *handle, void *buf, size_t nbytes, int *errcode)
{
    mchar_istream_t *is = (mchar_istream_t *)handle;
    ssize_t cnt;

    assert(MCHAR_ISTREAM_MAGIC == is->is_magic);

    if (NULL != is->is_buffer) {                /* internal push-back buffer */
        size_t buflen;

        if (nbytes < (buflen = is->is_buflen)) {
            (void) memcpy(buf, is->is_buffer, nbytes);
            is->is_buflen  -= nbytes;
            is->is_buffer  += nbytes;
            is->is_iobytes += nbytes;
            return nbytes;
        }

        (void) memcpy(buf, is->is_buffer, buflen);
        is->is_buffer = NULL;

        if ((cnt = vfs_read(is->is_filehandle, (char *)buf + buflen, nbytes - buflen)) >= 0) {
            is->is_iobytes += (cnt += buflen);

        } else if (buflen > 0) {
            is->is_iobytes += (cnt = buflen);

        } else if (0 == is->is_iobytes) {
            is->is_iobytes = cnt;
        }

    } else {
        if ((cnt = vfs_read(is->is_filehandle, buf, nbytes)) >= 0) {
            is->is_iobytes += cnt;

        } else if (0 == is->is_iobytes) {
            is->is_iobytes = cnt;
        }
    }
    *errcode = errno;
    return cnt;
}


static ssize_t
stream_wrfn(void *handle, const void *buf, size_t nbytes, int *errcode)
{
    mchar_istream_t *is = (mchar_istream_t *)handle;
    ssize_t cnt;

    assert(MCHAR_ISTREAM_MAGIC == is->is_magic);

    cnt = vfs_write(is->is_filehandle, buf, nbytes);
    is->is_iobytes = cnt;
    *errcode = errno;
    return cnt;
}


/*
 *  Single byte implementation.
 */
static const void *
uchar_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    if (src < cpend) {
        *cooked = *raw = *((unsigned char *)src);
        return ((unsigned char *)src) + 1;
    }
    return NULL;
}


static const void *
uchar_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    if (src < cpend) {
        *cooked = *((unsigned char *)src);
        return ((unsigned char *)src) + 1;
    }
    return NULL;
}


static int
uchar_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    *((unsigned char *)buffer) = (unsigned char)ch;
    return 1;
}


static int
uchar_length(mchar_iconv_t *ic, const int32_t ch)
{
    __CUNUSED(ic)
    __CUNUSED(ch)
    return 1;
}


/*
 *  UTF8 implementation.
 */
static const void *
utf8_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    return charset_utf8_decode(src, cpend, cooked, raw);
}


static const void *
utf8_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    return charset_utf8_decode_safe(src, cpend, cooked);
}


static int
utf8_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    return charset_utf8_encode(ch, buffer);
}


static int
utf8_length(mchar_iconv_t *ic, const int32_t ch)
{
    __CUNUSED(ic)
    return charset_utf8_length(ch);
}


/*
 *  UTF16BE implementation.
 */
static const void *
utf16be_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    return charset_utf16be_decode(src, cpend, cooked, raw);
}


static const void *
utf16be_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    return charset_utf16be_decode_safe(src, cpend, cooked);
}


static int
utf16be_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    return charset_utf16be_encode(ch, buffer);
}


/*
 *  UTF16LE implementation.
 */
static const void *
utf16le_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    return charset_utf16le_decode(src, cpend, cooked, raw);
}


static const void *
utf16le_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    return charset_utf16le_decode_safe(src, cpend, cooked);
}


static int
utf16le_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    return charset_utf16le_encode(ch, buffer);
}


/*
 *  UTF32LE implementation.
 */
static const void *
utf32be_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    return charset_utf32be_decode(src, cpend, cooked, raw);
}


static const void *
utf32be_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    return charset_utf32be_decode_safe(src, cpend, cooked);
}


static int
utf32be_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    return charset_utf32be_encode(ch, buffer);
}


/*
 *  UTF32LE implementation.
 */
static const void *
utf32le_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    __CUNUSED(ic)
    return charset_utf32le_decode(src, cpend, cooked, raw);
}


static const void *
utf32le_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    __CUNUSED(ic)
    return charset_utf32le_decode_safe(src, cpend, cooked);
}


static int
utf32le_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    __CUNUSED(ic)
    return charset_utf32le_encode(ch, buffer);
}


/*
 *  UTF16/32 support.
 */
static int
utf16_length(mchar_iconv_t *ic, const int32_t ch)
{
    __CUNUSED(ic)
    __CUNUSED(ch)
    return 2;
}


static int
utf32_length(mchar_iconv_t *ic, const int32_t ch)
{
    __CUNUSED(ic)
    __CUNUSED(ch)
    return 4;
}


/*
 *  libcharsettable interface.
 */
static mchar_iconv_t *
charset_open(const char *encoding)
{
    charset_iconv_t *charset = charset_iconv_open(encoding, 0);
    const size_t encodinglen = (charset ? strlen(encoding) + 1 : 0);
    mchar_iconv_t *ic;

    if (NULL == charset ||
            NULL == (ic = chk_alloc(sizeof(mchar_iconv_t) + encodinglen))) {
        if (charset) charset_iconv_close(charset);
        return NULL;
    }
    (void) memset(ic, 0, sizeof(mchar_iconv_t));
    (void) memcpy((void *)(ic + 1), encoding, encodinglen);
    ic->ic_magic        = MCHAR_ICONV_MAGIC;
    ic->ic_encoding     = (const char *)(ic + 1);
    ic->ic_unit         = 1;                    /* FIXME */
    ic->ic_decode       = charset_decode;
    ic->ic_decode_safe  = charset_decode_safe;
    ic->ic_encode       = charset_encode;
    ic->ic_length       = charset_length;
    ic->ic_close        = charset_close;
    ic->ic_xhandle      = (void *)charset;
    return ic;
}


static void
charset_close(mchar_iconv_t *ic)
{
    charset_iconv_close(ic->ic_xhandle);
}


static const void *
charset_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    return charset_iconv_decode(ic->ic_xhandle, src, cpend, cooked, raw);
}


static const void *
charset_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    int32_t raw;
    return charset_iconv_decode(ic->ic_xhandle, src, cpend, cooked, &raw);
}


static int
charset_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
    return charset_iconv_encode(ic->ic_xhandle, ch, buffer);
}


static int
charset_length(mchar_iconv_t *ic, const int32_t ch)
{
    return charset_iconv_length(ic->ic_xhandle, ch);
}


/*
 *  ICU interface.
 */
#if defined(HAVE_LIBICU)
static mchar_iconv_t *
icu_open(const char *encoding)
{
    UErrorCode err = U_ZERO_ERROR;
    UConverter *icu = ucnv_open(encoding, &err);
    const size_t encodinglen = (icu ? strlen(encoding) + 1 : 0);
    mchar_iconv_t *ic;

    if (U_FAILURE(err) ||
            NULL == (ic = chk_alloc(sizeof(mchar_iconv_t) + encodinglen))) {
        if (icu) ucnv_close(icu);
        return NULL;
    }
    (void) memset(ic, 0, sizeof(mchar_iconv_t));
    (void) memcpy((void *)(ic + 1), encoding, encodinglen);
    ic->ic_magic        = MCHAR_ICONV_MAGIC;
    ic->ic_encoding     = (const char *)(ic + 1);
    ic->ic_unit         = 1;                    /* FIXME */
    ic->ic_decode       = icu_decode;
    ic->ic_decode_safe  = icu_decode_safe;
    ic->ic_encode       = icu_encode;
    ic->ic_stream_open  = icu_stream_open;
    ic->ic_stream_close = icu_stream_close;
    ic->ic_length       = icu_length;
    ic->ic_close        = icu_close;
    ic->ic_xhandle      = (void *)icu;
    return ic;
}


static void
icu_close(mchar_iconv_t *ic)
{
    ucnv_close((UConverter *) ic->ic_xhandle);
}


static const void *
icu_decode(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw)
{
    if (src < cpend) {
        UConverter *icu = ic->ic_xhandle;
        UErrorCode err = U_ZERO_ERROR;
        UChar32 ch = ucnv_getNextUChar(icu, (const char **) &src, (const char *) cpend, &err);

        if (U_SUCCESS(err)) {
            *cooked = *raw = ch;
            return src;
        }
        ucnv_reset(icu);
    }
    return NULL;
}


static const void *
icu_decode_safe(mchar_iconv_t *ic, const void *src, const void *cpend, int32_t *cooked)
{
    if (src < cpend) {
        UConverter *icu = ic->ic_xhandle;
        UErrorCode err = U_ZERO_ERROR;
        UChar32 ch = ucnv_getNextUChar(icu, (const char **) &src, (const char *) cpend, &err);

        if (U_SUCCESS(err)) {
            *cooked = ch;
            return src;
        }
        ucnv_reset(icu);
    }
    return NULL;
}


static int
icu_encode(mchar_iconv_t *ic, const int32_t ch, void *buffer)
{
//  TODO
//  UConverter *icu = ic->ic_xhandle;
//  UErrorCode err = U_ZERO_ERROR;

    __CUNUSED(ic)
    __CUNUSED(ch)
    __CUNUSED(buffer)
    assert(0);
    return -1;
}


static int
icu_length(mchar_iconv_t *ic, const int32_t ch)
{
//  TODO
//  UConverter *icu = ic->ic_xhandle;
//  UErrorCode err = U_ZERO_ERROR;

    __CUNUSED(ic)
    __CUNUSED(ch)
    assert(0);
    return -1;
}


static size_t
icu_stream2u(void *cd, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    mchar_istream_t *is = (mchar_istream_t *)cd;
    mchar_iconv_t *ic = is->is_iconv;

    if (inbuf && inbytes) {
        const char *icursor = *inbuf;
        char *ocursor = *outbuf;

        UConverter *icuutf8 = (UConverter *)is->is_istream;
        UConverter *icu = ic->ic_xhandle;
        UErrorCode err = U_ZERO_ERROR;
                                                /* external -> UTF16 -> UTF8 */
        ucnv_convertEx(icuutf8, icu, outbuf, ocursor + *outbytes, inbuf, icursor + *inbytes,
                NULL, NULL, NULL, NULL, TRUE, TRUE, &err);

        *inbytes -= *inbuf - icursor;
        *outbytes -= *outbuf - ocursor;

        if (! U_SUCCESS(err)) {
            if (U_INVALID_CHAR_FOUND == err || U_ILLEGAL_CHAR_FOUND == err) {
                errno = EILSEQ;
                return (size_t)-1;
            }
            if (U_BUFFER_OVERFLOW_ERROR == err) {
                errno = E2BIG;
                return (size_t)-1;
            }
            errno = EINVAL;
            return (size_t)-1;
        }
    }
    return 0;
}


static size_t
icu_stream2x(void *cd, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes)
{
    mchar_istream_t *is = (mchar_istream_t *)cd;
    mchar_iconv_t *ic = is->is_iconv;

    if (inbuf && inbytes) {
        const char *icursor = *inbuf;
        char *ocursor = *outbuf;

        UConverter *icuutf8 = (UConverter *)is->is_istream;
        UConverter *icu = ic->ic_xhandle;
        UErrorCode err = U_ZERO_ERROR;
                                                /* UTF8 -> UTF16 -> external */
        ucnv_convertEx(icu, icuutf8, outbuf, ocursor + *outbytes, inbuf, icursor + *inbytes,
                NULL, NULL, NULL, NULL, TRUE, TRUE, &err);

        *inbytes -= *inbuf - icursor;
        *outbytes -= *outbuf - ocursor;

        if (! U_SUCCESS(err)) {
            if (U_INVALID_CHAR_FOUND == err || U_ILLEGAL_CHAR_FOUND == err) {
                errno = EILSEQ;
                return (size_t)-1;
            }
            if (U_BUFFER_OVERFLOW_ERROR == err) {
                errno = E2BIG;
                return (size_t)-1;
            }
            errno = EINVAL;
            return (size_t)-1;
        }
    }
    return 0;
}


static mchar_istream_t *
icu_stream_open(struct mchar_iconv *ic, int fd, const char *filename, int direction)
{
    UConverter *icuutf8;
    UErrorCode err;

    if (NULL != (icuutf8 = ucnv_open("utf-8", &err))) {
        mchar_istream_t *is;

        if (NULL != (is = chk_alloc(sizeof(mchar_istream_t)))) {
            iconv_stream_t *ics;

            (void) memset(is, 0, sizeof(mchar_istream_t));
            if (NULL != (ics = (direction ?
                    iconv_stream_open(icu_stream2x, (void *)is, NULL, stream_wrfn, (void *)is) :
                    iconv_stream_open(icu_stream2u, (void *)is, stream_rdfn, NULL, (void *)is)))) {

                is->is_magic        = MCHAR_ISTREAM_MAGIC;
                is->is_filename     = chk_salloc(filename);
                is->is_filehandle   = fd;
                is->is_direction    = direction;
                is->is_buffer       = NULL;
                is->is_buflen       = 0;
                is->is_iconv        = ic;
                is->is_istream      = ics;
                is->is_handle       = (void *)icuutf8;

                ic->ic_internal     = MCHAR_UTF8;
                ic->ic_decode       = utf8_decode;
                ic->ic_decode_safe  = utf8_decode_safe;
                ic->ic_encode       = utf8_encode;
                ic->ic_length       = utf8_length;
                return is;
            }
            chk_free((void *)is);
        }
    }
    return NULL;
}


static void
icu_stream_close(struct mchar_iconv *ic, mchar_istream_t *is)
{
    __CUNUSED(ic)
    __CUNUSED(is)
}
#endif  /*HAVE_LIBICU*/


/*
 *  iconv interface.
 */
#if defined(HAVE_APRICONV) || defined(HAVE_LIBICONV) || \
        defined(WIN32_DYNAMIC_ICONV)

#if defined(WIN32_DYNAMIC_ICONV)
typedef void *iconv_t;
#endif
#define ICONV_NULL      ((iconv_t) -1)

static void *           my_iconv_open(const char *to, const char *from);

static mchar_iconv_t *
icnv_open(const char *encoding)
{
    iconv_t ihandle = ICONV_NULL, ohandle = ICONV_NULL;
    const size_t encodinglen = (encoding ? strlen(encoding) + 1 : 0);
    char toencoding[128] = {0};
    mchar_iconv_t *ic;

    if (encoding) strxcpy(toencoding, encoding, sizeof(toencoding));
    strxcat(toencoding, " //TRANSLIT", sizeof(toencoding));

    if (NULL == (ic = chk_alloc(sizeof(mchar_iconv_t) + encodinglen)) ||
            ICONV_NULL == (ihandle = my_iconv_open("utf-8 //IGNORE", encoding)) ||
            ICONV_NULL == (ohandle = my_iconv_open(toencoding, "utf-8"))) {
        if (ic) {
            if (ICONV_NULL != ihandle) {
                iconv_close(ihandle);
            }
            chk_free((void *)ic);
        }
        return NULL;
    }
    (void) memset(ic, 0, sizeof(mchar_iconv_t));
    (void) memcpy((void *)(ic + 1), encoding, encodinglen);
    ic->ic_magic        = MCHAR_ICONV_MAGIC;
    ic->ic_encoding     = (const char *)(ic + 1);
    ic->ic_unit         = 1;
    ic->ic_internal     = MCHAR_UTF8;
    ic->ic_close        = icnv_close;
    ic->ic_decode       = utf8_decode;          /* internal always UTF-8 */
    ic->ic_decode_safe  = utf8_decode_safe;
    ic->ic_encode       = utf8_encode;
    ic->ic_length       = utf8_length;
    ic->ic_stream_open  = icnv_stream_open;     /* native streaming */
    ic->ic_stream_close = icnv_stream_close;
    ic->ic_ihandle      = ihandle;
    ic->ic_ohandle      = ohandle;
    return ic;
}


static mchar_istream_t *
icnv_stream_open(struct mchar_iconv *ic, int fd, const char *filename, int direction)
{
    mchar_istream_t *is;

    trace_ilog("icnv_stream_open(%d)\n", direction);
    if (NULL != (is = chk_alloc(sizeof(mchar_istream_t)))) {
        iconv_stream_t *ics;

        (void) memset(is, 0, sizeof(mchar_istream_t));
        if (NULL != (ics =
                (0 == direction ?
                    iconv_istream_open(ic->ic_ihandle, stream_rdfn, (void *)is) :
                    iconv_ostream_open(ic->ic_ohandle, stream_wrfn, (void *)is)))) {
            is->is_magic        = MCHAR_ISTREAM_MAGIC;
            is->is_filename     = chk_salloc(filename);
            is->is_filehandle   = fd;
            is->is_direction    = direction;
            is->is_buffer       = NULL;
            is->is_buflen       = 0;
            is->is_iconv        = ic;
            is->is_istream      = ics;
            is->is_handle       = NULL;
            return is;
        }
        chk_free((void *)is);
    }
    return NULL;
}


static void
icnv_stream_close(struct mchar_iconv *ic, mchar_istream_t *is)
{
    __CUNUSED(ic)
    __CUNUSED(is)
    trace_ilog("icnv_stream_close()\n");
}


static void
icnv_close(mchar_iconv_t *ic)
{
    if (ic->ic_ihandle) {
        iconv_close((iconv_t) ic->ic_ihandle);
        ic->ic_ihandle = 0;
    }
    if (ic->ic_ohandle) {
        iconv_close((iconv_t) ic->ic_ohandle);
        ic->ic_ohandle = 0;
    }
}


/*
 *  Call iconv_open() with a check if iconv() works properly (there are broken versions).
 *
 *  Returns (void *)-1 if failed:
 *      (should return iconv_t, but that causes problems with prototypes).
 */
static void *
my_iconv_open(const char *to, const char *from)
{
    static int iconv_ok = -1;                   /* one-shot test */
    iconv_t fd = ICONV_NULL;

    if (0 == iconv_ok) {
        return (void *) -1;                     /* detected a broken iconv() previously */
    }

    if (ICONV_NULL != (fd = iconv_open(to, from))) {
        if (-1 == iconv_ok) {
           /*
            *   Test for broken iconv() versions, thru a dummy iconv() call.
            *   The symptoms are that after outputting the initial shift state the
            *   "to" pointer is NULL and conversion stops for no apparent reason
            *   after about 8160 characters.
            */
#if !defined(ICONV_TESTLEN)
#define ICONV_TESTLEN           (1024)
#endif
            char tobuf[ICONV_TESTLEN];
            size_t tolen;
            char *p;

            p = (char *)tobuf;
            tolen = sizeof(tobuf);
            (void) iconv(fd, NULL, NULL, &p, &tolen);
            if (NULL == p) {
                iconv_ok = FALSE;
                iconv_close(fd);
                fd = ICONV_NULL;
            } else {
                iconv_ok = TRUE;
            }
        }
    }
    return (void *)fd;
}

#endif  /*HAVE_APRICONV || HAVE_LIBICONV*/
/*end*/
