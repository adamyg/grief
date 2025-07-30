#ifndef GR_EDATOM_H_INCLUDED
#define GR_EDATOM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(edatom_h,"$Id: edatom.h,v 1.3 2025/01/10 16:48:38 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edatom.h,v 1.3 2025/01/10 16:48:38 cvsuser Exp $
 * Atom representation.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if !defined(HAVE_CONFIG_H)
#error HAVE_CONFIG_H expected to be defined ...
#endif
#include <config.h>

#if (SIZEOF_LONG == SIZEOF_VOID_P)
#define CM_ATOMSIZE         SIZEOF_LONG
typedef long accint_t;
typedef long unsigned accuint_t;

#elif (SIZEOF_LONG_LONG == SIZEOF_VOID_P)
#define CM_ATOMSIZE         SIZEOF_LONG_LONG
typedef long long accint_t;
typedef long long unsigned accuint_t;

#else
#error unable to determine atom size.
#endif

typedef double accfloat_t;

#if (CM_ATOMSIZE == 8)
#define LIST_MAXLEN         0xffff              /* 2^16 */
#elif (CM_ATOMSIZE == 4)
#define LIST_MAXLEN         0x7fff              /* 2^15 */
#else
#error unsupported CM_ATOMSIZE szie
#endif

#endif /*GR_EDATOM_H_INCLUDED*/
