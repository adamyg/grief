#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetistream_c,"$Id: charsetistream.c,v 1.7 2015/02/19 00:17:03 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* iconv stream support.
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

#include <config.h>
#include <edtypes.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#if defined(WIN32) && \
        (!defined(HAVE_ICONV_H) || defined(GNUWIN32_LIBICONV)) && \
            !defined(WIN32_DYNAMIC_ICONV)
#define WIN32_DYNAMIC_ICONV                     /* dynamic loader */
#endif

#if defined(WIN32_DYNAMIC_ICONV)
#define  WIN32_ICONV_MAP
#include <win32_iconv.h>

#elif defined(HAVE_ICONV_H)
#include <iconv.h>
#endif


#include "iconv_stream.h"

iconv_stream_t *
iconv_istream_open(void *cd, iconv_rdfn_t rdfn, void *handle)
{
#if defined(WIN32_DYNAMIC_ICONV)
    return iconv_stream_open((iconv_cvtfn_t)w32_iconv, cd, rdfn, NULL, (void *)handle);

#elif defined(HAVE_ICONV_H)
    return iconv_stream_open((iconv_cvtfn_t)iconv, cd, rdfn, NULL, (void *)handle);

#else
    return NULL;
#endif
}


iconv_stream_t *
iconv_ostream_open(void *cd, iconv_wrfn_t wrfn, void *handle)
{
#if defined(WIN32_DYNAMIC_ICONV)
    return iconv_stream_open((iconv_cvtfn_t)w32_iconv, cd, NULL, wrfn, (void *)handle);

#elif defined(HAVE_ICONV_H)
    return iconv_stream_open((iconv_cvtfn_t)iconv, cd, NULL, wrfn, (void *)handle);

#else
    return NULL;
#endif
}
