#ifndef GR_WIN32_ICONV_H_INCLUDED
#define GR_WIN32_ICONV_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_iconv_h,"$Id: win32_iconv.h,v 1.6 2015/02/19 00:17:34 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 iconv dynamic loader.
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <sys/cdefs.h>
#include <unistd.h>

__CBEGIN_DECLS

extern int              w32_iconv_connect(int verbose);
extern void             w32_iconv_shutdown(void);

extern void *           w32_iconv_open(const char *to, const char *from);
extern int              w32_iconv(void *fd, const char **from, size_t *fromlen, char **to, size_t *tolen);
extern void             w32_iconv_close(void *fd);

#if defined(WIN32_ICONV_MAP)
typedef void *iconv_t;

#define iconv_open(__to, __from) \
                        w32_iconv_open(__to, __from)
#define iconv(__fd, __from, __fromlen, __to, __tolen) \
                        w32_iconv(__fd, __from, __fromlen, __to, __tolen)
#define iconv_close(__fd) \
                        w32_iconv_close(__fd)
#endif /*WIN32_ICONV_MAP*/

__CEND_DECLS

#endif /*GR_WIN32_ICONV_H_INCLUDED*/
