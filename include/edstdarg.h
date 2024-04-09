#ifndef GR_EDSTDARG_H_INCLUDED
#define GR_EDSTDARG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edstdarg_h,"$Id: edstdarg.h,v 1.4 2024/04/08 15:07:03 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edstdarg.h,v 1.4 2024/04/08 15:07:03 cvsuser Exp $
 * stdarg() interface/implemenation.
 *
 *
 * Copyright (c) Adam Young.
 * All rights reserved.
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#ifndef ED_VA_COPY
#if defined(va_copy) || defined(HAVE_VA_COPY)
    /*
     *  ISO C99 and later
     */
#define ED_VA_COPY(__dst, __src)    va_copy (__dst, __src)
#ifndef HAVE_VA_COPY
#define HAVE_VA_COPY 1
#endif

#elif defined(__va_copy) || defined(HAVE___VA_COPY)
    /*
     */
#define ED_VA_COPY(__dst, __src)    __va_copy (__dst, __src)
#if defined(NEED_VA_COPY)
#define va_copy(__dst, __src)       ED_VA_COPY (__dst, __src)
#endif

#elif defined(__WATCOMC__)
    /*
     *  older Watcom implementations
     */
#define ED_VA_COPY(__dst, __src)    memcpy ((void *)&(__dst), (const void *)&(__src), sizeof(va_list))

#else
    /*
     *  default ...
     */
#define ED_VA_COPY(__dst, __src)    do { (__dst) = (__src) } while (0)
#endif
#endif  /*ED_VA_COPY*/

#ifndef HAVE_VA_COPY
#if defined(NEED_VA_COPY) && !defined(va_copy)
#define va_copy(__dst, __src)       ED_VA_COPY (__dst, __src)
#endif
#endif

#endif /*GR_EDSTDARG_H_INCLUDED*/

/*end*/



