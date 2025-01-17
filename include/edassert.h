#ifndef GR_EDASSERT_H_INCLUDED
#define GR_EDASSERT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edassert_h,"$Id: edassert.h,v 1.18 2025/01/13 16:20:06 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edassert.h,v 1.18 2025/01/13 16:20:06 cvsuser Exp $
 * Custom assert interface
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

#include <edsym.h>

__CBEGIN_DECLS

#if defined(NDEBUG) || defined(EDNASSERT)

#define EDASSERT(ex)        ((void)0)
#define EDASSERT0(ex,m)       ((void)0)
#define EDASSERT1(ex,m,a)       ((void)0)
#define EDASSERT2(ex,m,a,b)       ((void)0)
#define EDASSERT3(ex,m,a,b,c)       ((void)0)
#define EDASSERT4(ex,m,a,b,c,d)       ((void)0)
#define EDASSERT5(ex,m,a,b,c,d,e)       ((void)0)
#define EDASSERT6(ex,m,a,b,c,d,e,f)       ((void)0)
#define EDASSERT7(ex,m,a,b,c,d,e,f,g)       ((void)0)
#define EDASSERT8(ex,m,a,b,c,d,e,f,g,h)       ((void)0)
#define EDASSERT9(ex,m,a,b,c,d,e,f,g,h,i)       ((void)0)

#else   /*!NDEBUG && !EDNASSERT*/

extern void                 __edassert(const char *file, unsigned line, const char *msg);
extern void                 __edassertf(const char *file, unsigned line, const char *fmt, ...) \
                                    __ATTRIBUTE_FORMAT__((printf, 3, 4));

#define EDASSERT(ex)        ((ex) ? (void)0 : __edassert(__FILE__, __LINE__,  #ex))
#define EDASSERT0(ex,m)       ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m))
#define EDASSERT1(ex,m,a)       ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a))
#define EDASSERT2(ex,m,a,b)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b))
#define EDASSERT3(ex,m,a,b,c)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c))
#define EDASSERT4(ex,m,a,b,c,d)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d))
#define EDASSERT5(ex,m,a,b,c,d,e)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d, e))
#define EDASSERT6(ex,m,a,b,c,d,e,f)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d, e, f))
#define EDASSERT7(ex,m,a,b,c,d,e,f,g)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d, e, f, g))
#define EDASSERT8(ex,m,a,b,c,d,e,f,g,h)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d, e, f, g, h))
#define EDASSERT9(ex,m,a,b,c,d,e,f,g,h,i)      ((ex) ? (void)0 : __edassertf(__FILE__, __LINE__, m, a, b, c, d, e, f, g, h, i))

#if defined(ED_ASSERT)      /* replace system assert interface */
#include <assert.h>
#undef assert
#define assert(e)           ((e) ? (void)0 : __edassert(__FILE__, __LINE__, #e))
#endif

#endif  /*!NDEBUG && !EDNASSERT*/

typedef int (*edAssertTrap_t)(const char *file, unsigned lineno, const char *cond);

enum {
    TRAP_IGNORE = 1,
    TRAP_DIALOG
};

extern int                  edAssertOpt(unsigned flags);
extern int                  edAssertTrap(edAssertTrap_t trap);
extern void                 edAssertAt(edAssertTrap_t trap);
extern const char *         edAssertMsg(void);

__CEND_DECLS

#endif /*GR_EDASSERT_H_INCLUDED*/
