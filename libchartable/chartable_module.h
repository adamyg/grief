#ifndef GR_CHARTABLE_MODULE_H_INCLUDED
#define GR_CHARTABLE_MODULE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_chartable_module_h,"$Id: chartable_module.h,v 1.16 2018/10/01 22:10:53 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* Chartable module interface
 *
 *
 * Copyright (c) 2010 - 2018, Adam Young.
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

#include <edtypes.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#ifndef MKMAGIC
#define MKMAGIC(__a, __b, __c, __d) \
              (((uint32_t)(__a))<<24 | ((uint32_t)(__b))<<16 | (__c)<<8 | (__d))
#endif

#define CHARTABLE_PACKAGE_MAGIC MKMAGIC('C', 'x', 'P', 'k')
#define CHARTABLE_MODULE_MAGIC  MKMAGIC('C', 'x', 'M', 'd')

#define CHARTABLE_CCS           MKMAGIC('C', 'c', 's', ' ')
#define CHARTABLE_CES           MKMAGIC('C', 'e', 's', ' ')

#define CHARTABLE_SIGNATURE(__type, __ver) \
                                ((__type << 16) | (__ver))

#if defined(MODULE_STATIC)
#   define MODULE_LINKAGE
#   define MODULE_ENTRY
#elif defined(WIN32)
#   define MODULE_LINKAGE       __declspec(dllexport)
#   define MODULE_ENTRY         __cdecl
#else
#   define MODULE_LINKAGE
#   define MODULE_ENTRY
#endif

struct chartable_ccs1 {
    const char **   names;
    uint32_t        cs;
    uint32_t        mc;
    uint32_t        sz;
    uint32_t      (*unicode)(struct chartable_ccs1 *self, uint32_t ch);
    uint32_t      (*native)(struct chartable_ccs1 *self, uint32_t ch);
    int           (*init)(struct chartable_ccs1 *self);
    int           (*shutdown)(struct chartable_ccs1 *self);
    void           *data;
};

typedef struct chartable_ccs1 chartable_ccs1_t;

struct chartable_module {
    uint32_t        cm_magic;                   /* structure magic */
    uint32_t        cm_signature;
    void *          cm_desc;
    const char **   cm_depends;
};

struct chartable_package {
    uint32_t        cp_magic;                   /* structure magic */
    uint32_t        cp_count; 
    const struct chartable_module * const *cp_modules;
};

typedef struct chartable_module chartable_module_t;

#endif /*GR_CHARTABLE_MODULE_H_INCLUDED*/
