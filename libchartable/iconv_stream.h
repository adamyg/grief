#ifndef GR_ICONV_STREAM_H_INCLUDED
#define GR_ICONV_STREAM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_iconv_stream_h,"$Id: iconv_stream.h,v 1.14 2024/04/17 16:00:29 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* iconv_stream.
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

#include <edtypes.h>
#include <stdio.h>

__CBEGIN_DECLS

typedef size_t            (*iconv_cvtfn_t)(void *cd, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);
typedef int               (*iconv_errnofn_t)(void *cd);
typedef ssize_t           (*iconv_rdfn_t)(void *handle, void *buf, size_t nbytes, int *errcode);
typedef ssize_t           (*iconv_wrfn_t)(void *handle, const void *buf, size_t nbytes, int *errcode);

typedef struct {
    /*
     *  iconv stream descriptor
     */
    MAGIC_t                 s_magic;            /* structure magic */

    size_t                  s_inbytes;          /* runtime usage stats */
    size_t                  s_outbytes;
    size_t                  s_cnvbad;

    iconv_cvtfn_t           s_iconv;            /* conversion interface */
    iconv_errnofn_t         s_ierrno;           /* conversion errno */
    void *                  s_cd;               /* conversion descriptor */

    iconv_rdfn_t            s_read;             /* stream read */
    iconv_wrfn_t            s_write;            /* stream write */
    void *                  s_handle;           /* associated handle */

#define ICONVSTREAMBUFSZ        1024
    char *                  s_buffer;           /* working buffer */
    size_t                  s_buflen;           /* buffer content */
    size_t                  s_iolen;
                                                /* input/output buffer */
    char                    s_iobuf[ICONVSTREAMBUFSZ];
} iconv_stream_t;

extern iconv_stream_t *     iconv_stream_open(iconv_cvtfn_t cnvfn, void *cd, iconv_rdfn_t rdfn, iconv_wrfn_t wrfn, void *handle);
extern iconv_stream_t *     iconv_istream_fopen(void *cd, FILE *fd);
extern iconv_stream_t *     iconv_ostream_fopen(void *cd, FILE *fd);
extern iconv_stream_t *     iconv_istream_open(void *cd, iconv_rdfn_t rdfn, void *handle);
extern iconv_stream_t *     iconv_ostream_open(void *cd, iconv_wrfn_t wrfn, void *handle);
extern size_t               iconv_stream_flush(iconv_stream_t *s);
extern void                 iconv_stream_close(iconv_stream_t *s);
extern size_t               iconv_stream_write(iconv_stream_t *s, const void *buf, size_t insize);
extern ssize_t              iconv_stream_read(iconv_stream_t *s, void *buf, size_t size);

__CEND_DECLS

#endif /*GR_ICONV_STREAM_H_INCLUDED*/
