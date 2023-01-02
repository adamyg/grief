#ifndef GR_VTYPE_H_INCLUDED
#define GR_VTYPE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vtype_h,"$Id: vtype.h,v 1.12 2023/01/01 11:26:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vtype.h,v 1.12 2023/01/01 11:26:59 cvsuser Exp $
 * Self-organising integer vector data structure.
 *
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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

typedef struct {
    unsigned            size;                   /* element size */
    void *              bins[22];               /* 0 to 2^24 elements */
} vtable_t;

typedef struct {
    unsigned int        i1;
    unsigned int        i2;
} vtable_iterator_t;

int                     vt_init(vtable_t *va, unsigned membsize);
int                     vt_reinit(vtable_t *va);
int                     vt_deinit(vtable_t *va);
vtable_t *              vt_new(unsigned membsize);
int                     vt_del(void *va);
int                     vt_release(vtable_t *va, unsigned int from);
void *                  vt_get(vtable_t *va, unsigned int idx);
int                     vt_index(vtable_t *va, void *elem);
void                    vt_iterate(void *va, vtable_iterator_t *iter);
void *                  vt_next(void *va, vtable_iterator_t *iter);

#endif /*GR_VTYPE_H_INCLUDED*/
