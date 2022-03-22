#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetstream_c,"$Id: charsetstream.c,v 1.12 2022/03/21 14:59:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* conversion stream support.
 *
 *
 * Copyright (c) 2010 - 2022, Adam Young.
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
#include <edtypes.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "iconv_stream.h"

static ssize_t                  stream_write(iconv_stream_t *s, const void *buf, size_t insize);
static ssize_t                  stream_read(iconv_stream_t *s, void *buf, size_t outsize);


static int
stream_ierrno(iconv_stream_t *s)
{
    if (s->s_ierrno) {
        return (s->s_ierrno)(s->s_cd);
    }
    return errno;
}


static ssize_t
stream_write(iconv_stream_t *s, const void *buf, size_t insize)
{
    size_t cnvbad = 0, outsize = sizeof(s->s_iobuf);
    const char *inbuf = buf;
    char *outbuf = s->s_iobuf;

    if (NULL == s->s_write) {
        errno = EBADF;
        return -1;
    }

    if (inbuf && insize > 0) {                  /* convert, skipping bad characters */
        /*
         *  Notes:
         *      IRIX iconv() inserts a NUL byte if it cannot convert.
         *      NetBSD inserts a question mark if it cannot convert.
         *      GNUC and Solaris behaves as desired
         */
        while ((size_t)-1 == (s->s_iconv)(s->s_cd, &inbuf, &insize, &outbuf, &outsize) &&
                    EILSEQ == stream_ierrno(s) && insize) {
            ++inbuf;
            --insize;
            ++cnvbad;
        }
    } else {                                    /* flush, generate closing sequence */
        (void) (s->s_iconv)(s->s_cd, NULL, NULL, &outbuf, &outsize);
    }

    if ((outsize = outbuf - s->s_iobuf) > 0) {
        int t_errno = 0;
        ssize_t cnt;                            /* flush converted text */

        outbuf = s->s_iobuf;
        while ((cnt = s->s_write(s->s_handle, outbuf, outsize, &t_errno)) < outsize) {
            if (cnt <= 0) {
                if (EINTR == t_errno) {
                    continue;
                }
                errno = t_errno;
                return -1;
            }
            s->s_outbytes += cnt;
            outbuf += cnt;
            outsize -= cnt;
        }
    }

    insize = inbuf - (const char *)buf;         /* converted arena size */
    s->s_inbytes += insize - cnvbad;
    s->s_cnvbad += cnvbad;
    return insize;
}


static ssize_t
stream_read(iconv_stream_t *s, void *buf, size_t outsize)
{
    size_t cnvbad = 0, insize = s->s_iolen, left;
    char *inbuf = s->s_iobuf;
    char *outbuf = buf;
    int t_errno = 0;
    ssize_t cnt;

    assert(insize <= ICONVSTREAMBUFSZ);
    if (NULL == s->s_read) {
        errno = EBADF;
        return -1;
    }
    for (left = ICONVSTREAMBUFSZ - insize; left > 0;) {
        if ((cnt = s->s_read(s->s_handle, inbuf + insize, left, &t_errno)) <= 0) {
            if (0 == cnt) {
                break;                          /* EOF */
            }
            if (EINTR == t_errno) {
                continue;
            }
            errno = t_errno;
            return -1;
        }
        s->s_inbytes += cnt;
        insize += cnt;
        left -= cnt;
    }

    if (insize > 0) {                           /* convert, skipping bad characters */
        while ((size_t)-1 == (s->s_iconv)(s->s_cd, (const char **)&inbuf, &insize, &outbuf, &outsize) &&
                    EILSEQ == stream_ierrno(s) && insize) {
            ++inbuf;
            --insize;
            ++cnvbad;
        }
    }

    if ((s->s_iolen = insize) > 0) {            /* remainder, EOF return otherwise cache */
        if (0 == cnt && outsize > 0) {
            if (outsize > insize) {
                outsize = insize;
            }
            memmove(outbuf, inbuf, outsize);
            outbuf += outsize;
            if ((s->s_iolen = (insize -= outsize)) > 0) {
                memmove(s->s_iobuf, inbuf, insize);
            }
        } else {
            memmove(s->s_iobuf, inbuf, insize);
        }
    }

    s->s_cnvbad += cnvbad;
    return (outbuf - (char *)buf);
}


iconv_stream_t *
iconv_stream_open(iconv_cvtfn_t cnvfn, void *cd, iconv_rdfn_t rdfn, iconv_wrfn_t wrfn, void *handle)
{
    iconv_stream_t *s;

    if (NULL == (s = (iconv_stream_t *)calloc(sizeof(iconv_stream_t), 1))) {
        return NULL;
    }
    s->s_iconv      = cnvfn;
    s->s_ierrno     = NULL;
    s->s_cd         = cd;
    s->s_inbytes    = 0;
    s->s_outbytes   = 0;
    s->s_buffer     = NULL;
    s->s_buflen     = 0;
    s->s_iolen      = 0;
    s->s_read       = rdfn;
    s->s_write      = wrfn;
    s->s_handle     = handle;
    return s;
}


void
iconv_stream_close(iconv_stream_t *s)
{
    if (s) {
        if (s->s_buffer) {
            free(s->s_buffer);
        }
        free(s);
    }
}


size_t
iconv_stream_write(iconv_stream_t *s, const void *buf, size_t insize)
{
    ssize_t size = insize, cnt = 0;
    const char *outbuf = (const char *)buf;

    /*
     *  flush previous buffer
     */
    if (s->s_buffer && s->s_buflen) {
        char *buffer = s->s_buffer;
        size_t left, buflen;

        if ((buflen = s->s_buflen) >= ICONVSTREAMBUFSZ) {
            errno = E2BIG;
            return -1;
        }

        do {                                    /* concat previous and new data */
            assert(buflen < ICONVSTREAMBUFSZ);
            if ((left = (ICONVSTREAMBUFSZ - buflen)) > size) {
                left = size;
            }
            memcpy(buffer + buflen, outbuf, left);
            outbuf += left;
            size -= left;
                                                /* write buffer */
            if ((cnt = stream_write(s, buffer, buflen += left)) < 0) {
                s->s_buflen = buflen;
                return -1;
            }

            assert(cnt <= buflen);
            if ((buflen -= cnt) > 0) {          /* relocate remainder */
                memmove(buffer, buffer + cnt, buflen);
            }
        } while (size > 0 && left > 0);

        s->s_buflen = buflen;
        assert(0 == buflen);
    }

    /*
     *  current buffer
     */
    if (size <= 0) {
        return insize;
    }
    do {
        if ((cnt = stream_write(s, outbuf, size)) < 0) {
            return -1;
        }
        outbuf += cnt;
        size -= cnt;
    } while (size > 0 && cnt > 0);

    /*
     *  cache remaining content
     */
    if (size > 0) {
        assert(0 == s->s_buflen);
        if (size > ICONVSTREAMBUFSZ) {
            errno = E2BIG;
            return -1;
        }
        if (NULL == s->s_buffer && 
                NULL == (s->s_buffer = malloc(ICONVSTREAMBUFSZ))) {
            s->s_buflen = 0;
            return -1;
        }
        memcpy(s->s_buffer, outbuf, size);
        s->s_buflen = size;
    }
    return insize;
}


ssize_t
iconv_stream_read(iconv_stream_t *s, void *buf, size_t size)
{
    char *inbuf = buf;
    ssize_t cnt;

    do {
        if ((cnt = stream_read(s, inbuf, size)) < 0) {
            return -1;
        }
        inbuf += cnt;
        size -= cnt;
    } while (size > 0 && cnt > 0);

    return (inbuf - (char *)buf);
}


size_t
iconv_stream_flush(iconv_stream_t *s)
{
    size_t res = stream_write(s, NULL, 0);

    (void) (s->s_iconv)(s->s_cd, NULL, NULL, NULL, NULL);
    return res;
}
/*end*/

