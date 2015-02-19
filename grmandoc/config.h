#ifndef MANDOC_CONFIG_H_INCLUDED
#define MANDOC_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.3 2014/07/17 00:47:14 cvsuser Exp $
 * mandoc config.h
 *
 * Copyright (c) 2014, Adam Young.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * ==end==
 *
 */

/*
 */

#if defined(WIN32) && !defined(__MINGW32__)
#include "../libw32/config.h"
#include <malloc.h>
#define  inline _inline
#define  snprintf _snprintf
#else
#include "../include/config.h"
#endif
#include "mdocversion.h"        /*VERSION*/


/*
 *  compat_fgetln.c
 *  compat_getsubopt.c
 *  compat_strlcpy.c
 *  compat_strlcat.c
 */

#include <stdio.h>
#include <string.h>

#if !defined(HAVE_FGETLN)
extern char *                   fgetln(FILE *fp, size_t *len);
#endif

#if !defined(HAVE_GETSUBOPT)
extern char *suboptarg;
extern int                      getsubopt(char **optionp, char * const *tokens, char **valuep);
#endif

#if !defined(HAVE_STRLCPY)
extern size_t                   strlcpy(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRLCAT)
extern size_t                   strlcat(char *dst, const char *src, size_t siz);
#endif


/*
 *  libsupport
 */

#if !defined(HAVE_ASPRINTF) && !defined(asprintf)
#define NEED_ASPRINTF
#include <stdarg.h>

extern int                      asprintf(char **str, const char *fmt, ...);
extern int                      vasprintf(char **str, const char *fmt, va_list ap);
#endif

#if !defined(HAVE_ISBLANK) && !defined(isblank)
#define NEED_ISBLANK

extern int                      isblank(int ch);
#endif

/*
 *  __BEGIN_DECLS
 *  void my_declarations();
 *  __END_DECLS
 */

#if defined(HAVE_SYS_CDEFS_H)
#include <sys/cdefs.h>
#endif
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#     define __BEGIN_DECLS      extern "C" {
#     define __END_DECLS        }
#  else
#     define __BEGIN_DECLS
#     define __END_DECLS
#  endif
#endif
#ifndef __P
#  if (__STDC__) || defined(__cplusplus) || \
         defined(_MSC_VER) || defined(__PARADIGM__) || defined(__GNUC__) || \
         defined(__BORLANDC__) || defined(__WATCOMC__)
#     define __P(x)             x
#  else
#     define __P(x)             ()
#  endif
#endif

#endif  /*MANDOC_CONFIG_H_INCLUDED*/
/*end*/
