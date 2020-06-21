#ifndef MANDOC_CONFIG_H_INCLUDED
#define MANDOC_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: config.h,v 1.19 2020/06/18 12:50:24 cvsuser Exp $
 * mandoc config.h
 *
 * Copyright (c) 2014 - 2020, Adam Young.
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

#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)

#include "../libw32/config.h"

#include <stddef.h>
#include <malloc.h>
#include <unistd.h>

#ifndef  snprintf
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define  snprintf _snprintf    /*2015+*/
#endif
#endif

#ifndef  mktemp
#define  mktemp _mktemp
#endif
#ifndef  chdir
#define  chdir w32_chdir
#endif
#ifndef  mkdir
#define  mkdir w32_mkdir
#endif
#ifndef  rmdir
#define  rmdir w32_rmdir
#endif
#ifndef  getcwd
#define  getcwd w32_getcwd
#endif
#ifndef  realpath
#define  realpath w32_realpath
#endif
#ifndef  lstat
#define  lstat w32_lstat
#endif
#define  inline _inline
#include "../libw32/win32_child.h"

#else /*!WIN32*/
#include "../include/config.h"
#endif

/*FIXME: edbuildinfo.h*/
#define  OSNAME "GRIEF Edit 3.2"

#include "mdocversion.h"        /*VERSION and binary names*/

/*
 *  compat_err.c (1.13.4)
 *  compat_fgetln.c
 *  compat_getsubopt.c
 *  compat_reallocarray.c (1.13.4)
 *  compat_strlcat.c
 *  compat_strcasestr.c
 *  compat_strlcpy.c
 *  compat_strtonum (1.13.4)
 */

#if !defined(_GNU_SOURCE)
#if defined(linux) || defined(__CYGWIN__) //FIXME
#define _GNU_SOURCE             /*see: string.h*/
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#if !defined(HAVE_PROG)
extern void                     setprogname(const char *);
extern const char *             getprogname(void);
#endif

#if !defined(HAVE_MKDTEMP)
extern char *                   mkdtemp(char *path);
#endif

#if !defined(HAVE_ERR)
extern void                     err(int eval, const char *fmt, ...);
extern void                     errx(int eval, const char *fmt, ...);
extern void                     warn(const char *fmt, ...);
extern void                     warnx(const char *fmt, ...);
#endif

#if !defined(HAVE_FGETLN)
extern char *                   fgetln(FILE *fp, size_t *len);
#endif

#if !defined(GAVE_GETLINE)
extern ssize_t                  getline(char **buf, size_t *bufsz, FILE *fp);
#endif

#if !defined(HAVE_GETSUBOPT)
extern char *suboptarg;
extern int                      getsubopt(char **optionp, char * const *tokens, char **valuep);
#endif

#if !defined(HAVE_REALLOCARRAY)
void *                          reallocarray(void *optr, size_t nmemb, size_t size);
#endif

#if !defined(HAVE_RECALLOCARRAY)
void *                          recallocarray(void *ptr, size_t oldnmemb, size_t newnmemb, size_t size);
#endif

#if !defined(HAVE_STRCASESTR) || defined(__CYGWIN__) /*missing?*/
extern char *                   strcasestr(const char *s, const char *find);
#endif

#if !defined(HAVE_STRLCPY)
extern size_t                   strlcpy(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRLCAT)
extern size_t                   strlcat(char *dst, const char *src, size_t siz);
#endif

#if !defined(HAVE_STRNDUP)
extern char *                   strndup(const char *str, size_t maxlen);
#endif

#if !defined(HAVE_STRTONUM)
extern long long                strtonum(const char *numstr, long long minval, long long maxval, const char **errstrp);
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

#if !defined(__GNUC__) && !defined(__clang__)
#ifndef __attribute__           //FIXME: HAVE_ATTRIBUTE
#define __attribute__(__x)
#endif
#endif

#include "portable_endian.h"

#endif  /*MANDOC_CONFIG_H_INCLUDED*/
/*end*/

