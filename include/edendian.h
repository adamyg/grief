#ifndef GR_EDENDIAN_H_INCLUDED
#define GR_EDENDIAN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edendian_h,"$Id: edendian.h,v 1.19 2024/10/01 12:55:03 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edendian.h,v 1.19 2024/10/01 12:55:03 cvsuser Exp $
 * Endian interface.
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

/*
 *  BYTE_ORDER, source/
 *      <endian.h>
 *      <sys/endian.h>
 *      <sys/byteorder.h>
 *      <machine/endian.h>
 *      <arpa/nameser_compat.h>
 */
#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
#include <sys/endian.h>
#elif defined(HAVE_SYS_BYTEORDER_H)
#include <sys/byteorder.h>                      /* Solaris */
#elif defined(HAVE_MACHINE_ENDIAN_H)
#include <machine/endian.h>
#elif defined(HAVE_ARPA_NAMESER_COMPAT_H)
#include <arpa/nameser_compat.h>
#endif

/*
 *  autoconf/
 *      IS_BIG_ENDIAN
 *      IS_LITTLE_ENDIAN
 *      IS_UNKNOWN_ENDIAN
 *      IS_UNIVERSAL_BUILD
 */
#if defined(IS_BIG_ENDIAN)                      /* big endian */
#   define HOST_BIG_ENDIAN

#elif defined(IS_LITTLE_ENDIAN)                 /* little endian */
#   define HOST_LITTLE_ENDIAN

#elif defined(IS_UNIVERSAL_BUILD) ||            /* universal */ \
            (defined(AC_APPLE_UNIVERSAL_BUILD) && (AC_APPLE_UNIVERSAL_BUILD))
#   define HOST_UNIVERSAL_ENDIAN

#elif defined(IS_UNKNOWN_ENDIAN)                /* unknown/error */
#   define HOST_UNKNOWN_ENDIAN

#elif defined(BYTE_ORDER)                       /* BYTE_ORDER */
#   if defined(BIG_ENDIAN) && (BYTE_ORDER == BIG_ENDIAN)
#       define HOST_BIG_ENDIAN

#   elif defined(LITTLE_ENDIAN) && (BYTE_ORDER == LITTLE_ENDIAN)
#       define HOST_LITTLE_ENDIAN

#   elif defined(PDP_ENDIAN) && (BYTE_ORDER == PDP_ENDIAN)
#       define HOST_UNKNOWN_ENDIAN
#       error PDP_ENDIAN not supported ...

#   else
#       define HOST_UNKNOWN_ENDIAN
#       error Unknown BYTE_ORDER configuration ...
#   endif

#elif defined(_BYTE_ORDER)                      /* BYTE_ORDER (Solaris) */
#   if defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN)
#       define HOST_BIG_ENDIAN

#   elif defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN)
#       define HOST_LITTLE_ENDIAN

#   else
#       define HOST_UNKNOWN_ENDIAN
#       error Unknown _BYTE_ORDER configuration ...
#   endif

#else                                           /* ??? */
#   define HOST_UNKNOWN_ENDIAN
#   error Unknown target architecture ...
#endif


/*
 *  confirm results/
 *      overkill??
 */
#if defined(BYTE_ORDER)
#if defined(HOST_UNIVERSAL_ENDIAN)
#error BYTE_ORDER host universal endian ...

#elif defined(HOST_UNKNOWN_ENDIAN)
#error BYTE_ORDER host unknown endian ...

#elif defined(BIG_ENDIAN) && (BYTE_ORDER == BIG_ENDIAN)
#if !defined(HOST_BIG__ENDIAN)
#error BYTE_ORDER and HIST_XXX_ENDIAN differ (big/little) ...
#endif

#elif defined(LITTLE_ENDIAN) && (BYTE_ORDER == LITTLE_ENDIAN)
#if !defined(HOST_LITTLE_ENDIAN)
#error BYTE_ORDER and HIST_XXX_ENDIAN differ (little/big) ...
#endif
#endif
#endif /*BYTE_ORDER*/

#endif /*GR_EDENDIAN_H_INCLUDED*/
/*end*/
