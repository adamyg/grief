#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetfstream_c,"$Id: charsetfstream.c,v 1.9 2022/03/21 14:59:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* iconv FILE stream support.
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
#if defined(HAVE_ICONV_H)
#include <stdio.h>
#include <iconv.h>
#endif
#include <unistd.h>

#include "iconv_stream.h"

#if defined(HAVE_ICONV_H)
static ssize_t                  stream_fwrite(void *handle, const void *buf, size_t size, int *errcode);
static ssize_t                  stream_fread(void *handle, void *buf, size_t size, int *errcode);
#endif

iconv_stream_t *
iconv_istream_fopen(void *cd, FILE *fd)
{
#if defined(HAVE_ICONV_H)
    return iconv_stream_open((iconv_cvtfn_t)iconv, cd, stream_fread, NULL, (void *)fd);
#else
    return NULL;
#endif
}


iconv_stream_t *
iconv_ostream_fopen(void *cd, FILE *fd)
{
#if defined(HAVE_ICONV_H)
    return iconv_stream_open((iconv_cvtfn_t)iconv, cd, NULL, stream_fwrite, (void *)fd);
#else
    return NULL;
#endif
}


#if defined(HAVE_ICONV_H)
static ssize_t
stream_fwrite(void *handle, const void *buf, size_t size, int *errcode)
{
    size_t cnt = fwrite(buf, 1, size, (FILE *)handle);
    *errcode = errno;
    return (cnt && !ferror((FILE *)handle)) ? cnt : -1;
}


static ssize_t
stream_fread(void *handle, void *buf, size_t size, int *errcode)
{
    size_t cnt = fread(buf, 1, size, (FILE *)handle);
    *errcode = errno;
    return (!ferror((FILE *)handle)) ? cnt : -1;
}
#endif  /*HAVE_ICONV_H*/
