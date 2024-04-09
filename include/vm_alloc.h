#ifndef GR_VM_ALLOC_H_INCLUDED
#define GR_VM_ALLOC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vm_alloc_h,"$Id: vm_alloc.h,v 1.15 2024/04/08 15:07:13 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vm_alloc.h,v 1.15 2024/04/08 15:07:13 cvsuser Exp $
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

#include <tailqueue.h>
#include <chkalloc.h>

#define VMALIGNMENT     sizeof(vmalignment_t)
#define VMALIGNED(x)    ((unsigned long)x & (VMALIGNMENT-1))


#ifndef ALIGN_DOWN
#define ALIGN_DOWN(n, align) \
                        ((unsigned long)(n) & ~((align) - 1))
#define ALIGN_UP(n, align) \
                        ALIGN_DOWN((unsigned long)(n)+(align)-1, (align))
#endif

typedef union {                             /* system alignment */
    long                x_long;
    double              x_double;
    float               x_float;
} vmalignment_t;


typedef TAILQ_HEAD(vmbankq_t, _vmbank)
                        vmbankq_t;          /* Bank queue */


typedef TAILQ_HEAD(vmblockq_t, _vmblock)
                        vmblockq_t;         /* Block (free) queue */


typedef struct {
    unsigned long       vp_magic;           /* Structure magic */
    unsigned long       vp_size;            /* Size, in bytes */
    unsigned long       vp_ublksiz;         /* User block size */
    unsigned long       vp_blksiz;          /* Buffer size, in bytes */
    unsigned long       vp_elems;           /* Total buffers */
    unsigned long       vp_flags;           /* Flags */
    vmbankq_t           vp_bankq;           /* Block list */
    vmblockq_t          vp_freeq;           /* Free list */
    unsigned long       vp_freecnt;         /* Current free number */
} vmpool_t;

extern int              vm_dflag;           /* debug malloc flag */

extern int              vm_init(vmpool_t *pool, unsigned blksiz, unsigned size);
extern void *           vm_alloc(vmpool_t *pool, unsigned blksiz);
extern void             vm_zap(vmpool_t *pool);
extern void             vm_destroy(vmpool_t *pool);
extern void *           vm_new(vmpool_t *pool);
extern void             vm_free(vmpool_t *pool, void *);
extern int              vm_reference(vmpool_t *pool, void *);
extern int              vm_unreference(vmpool_t *pool, void *buf);

#endif /*GR_VM_ALLOC_H_INCLUDED*/
