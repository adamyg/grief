#ifndef GR_EDREFOBJ_H_INCLUDED
#define GR_EDREFOBJ_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edrefobj_h,"$Id: edrefobj.h,v 1.21 2019/03/15 23:03:09 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edrefobj.h,v 1.21 2019/03/15 23:03:09 cvsuser Exp $
 * Reference strings
 *
 *
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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
#include <edopcode.h>

__CBEGIN_DECLS

struct ref;

typedef void (*refdelete_t)(struct ref *rp);

typedef struct ref {
    struct refinternals {
        uint32_t        r_magic;                /* structure magic */
#define R_MAGIC             0x52537472          /* RStr */
        uint32_t        r_refs;                 /* Number of times object being referenced */
        int32_t         r_used;                 /* Number of bytes used by object */
        int32_t         r_size;                 /* Bytes allocated for object */
        OPCODE          r_type;                 /* Base type of object */
        void *          r_ptr;                  /* Pointer to the object */
        refdelete_t     r_delete;               /* Destructor */
    } internals;
} ref_t;

extern ref_t *          r_string(const char *str);
extern ref_t *          r_nstring(const char *str, int len);
extern ref_t *          r_list(const LIST *list, int len, refdelete_t func);

extern ref_t *          r_init(OPCODE type, const void *object, int len);
extern ref_t *          r_initx(OPCODE type, const void *object, int len, refdelete_t func);

extern ref_t *          r_alloc(int size);
extern void             r_reassign(ref_t *rp, const char *object, int len);
extern void             r_clear(ref_t *rp);

extern ref_t *          r_push(ref_t *rp, const char *mem, int len, int expansion);
extern ref_t *          r_pop(ref_t *rp, int len) ;
extern ref_t *          r_append(ref_t *rp, const char *mem, int len, int expansion);

extern ref_t *          r_cat(ref_t *rp, const char *str);

extern void             r_incused(ref_t *rp, int inc);
extern ref_t *          r_inc(ref_t *rp);
extern ref_t *          r_dec(ref_t *rp);

#if defined(EDREFOBJ_H_INTERNALS)
#define r_magic         internals.r_magic
#define r_refs          internals.r_refs
#define r_used          internals.r_used
#define r_size          internals.r_size
#define r_ptr           internals.r_ptr
#define r_type          internals.r_type
#define r_delete        internals.r_delete
#else
#define r_ptr(_r)       ((_r)->internals.r_ptr)
#define r_refs(_r)      ((_r)->internals.r_refs)
#define r_type(_r)      ((_r)->internals.r_type)
#define r_used(_r)      ((_r)->internals.r_used)
#endif

__CEND_DECLS

#endif /*GR_EDREFOBJ_H_INCLUDED*/
