#ifndef GR_LIBSTR_H_INCLUDED
#define GR_LIBSTR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_str_h,"$Id: libstr.h,v 1.28 2025/02/07 03:03:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libstr.h,v 1.28 2025/02/07 03:03:22 cvsuser Exp $
 * libstr - String utility library.
 *
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#include <stdarg.h>
#include <edsym.h>

__CBEGIN_DECLS

extern char *               strxcpy(char *dest, const char *src, size_t len);
extern char *               strxcat(char *dest, const char *src, size_t len);
extern size_t               strxlen(const char *str, size_t maxlen);

extern int                  sxprintf(char *buf, size_t size, const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 3, 4));
extern int                  sxprintf0(char *buf, size_t size, const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 3, 4));
extern int                  vsxprintf(char *buf, size_t size, const char *fmt, va_list ap) __ATTRIBUTE_FORMAT__((printf, 3, 0));
extern int                  vsxprintf0(char *buf, size_t size, const char *fmt, va_list ap) __ATTRIBUTE_FORMAT__((printf, 3, 0));

extern long                 str_num(const char *numstr, long minval, long maxval, const char **errp);

extern int                  str_icmp(const char *p1, const char *p2);
extern int                  str_nicmp(const char *p1, const char *p2, size_t len);

extern const char *         str_chrx(const char *p, int c, int *len);
extern void                 str_cpy(char *dst, const char *src);
extern char *               str_rev(char *p);
extern const char *         str_trim(const char *name, size_t *lengthp);

extern char *               str_upper(char *p);
extern char *               str_lower(char *p);

extern int                  str_verscmp(const char *s1, const char *s2);

extern const char *         str_error(const int xerrno);

extern char *               str_tok(char *buf, const char *delims);
extern int                  str_ntok(char *result, size_t retlen, char *buf, const char *delims);

#define NUMPARSE_ERR_UNDERFLOW -4               /* numeric underflow */
#define NUMPARSE_ERR_OVERFLOW  -3               /* numeric overflow */
#define NUMPARSE_ERR_EXPONENT -2                /* invalid exponent */
#define NUMPARSE_ERR_SUFFIX -1                  /* invalid suffix 'x' on numeric constant */
#define NUMPARSE_ERROR 0                        /* syntax error */
#define NUMPARSE_INTEGER 1                      /* integer constant */
#define NUMPARSE_FLOAT 2                        /* float constant */
#define NUMPARSE_DOT 3                          /* . */
#define NUMPARSE_ELLIPSIS 4                     /* .. */

extern const char *         str_numerror(int ret);
extern int                  str_numparse(const char *str, double *dp, long *lp, int *len);
extern int                  str_numparsel(const char *str, double *dp, long long *lp, int *len);
extern int                  str_numparsex(int (*get)(void *), int (*unget)(void *, int ch), void *parm,
                                double *dp, long *lp, int *len);
extern int                  str_numparsexl(int (*get)(void *), int (*unget)(void *, int ch), void *parm,
                                double *dp, long long *lp, int *len);

__CEND_DECLS

#endif /*GR_LIBSTR_H_INCLUDED*/
