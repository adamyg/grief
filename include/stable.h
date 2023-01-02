#ifndef GR_STABLE_H_INCLUDED
#define GR_STABLE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_stable_h,"$Id: stable.h,v 1.13 2023/01/01 11:26:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: stable.h,v 1.13 2023/01/01 11:26:59 cvsuser Exp $
 * Self-organising string hash data structure.
 *
 *  This is a library of routines for storing a data structure where the
 *  primary key is an string. The underlying data structure is an extensible
 *  hash using chaining to address clashes. Upon the key count exceeding the
 *  fill factory, the hash shall extend by doubling in size.
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

#include <edtypes.h>


__CBEGIN_DECLS

typedef struct {
/*private:*/
    unsigned long       magic;                  /* magic */
    unsigned long       buckets;                /* buckets */
    unsigned long       himask;                 /* mask hi */
    unsigned long       lomask;                 /* mask lo */
    unsigned long       split;                  /* split point */
    unsigned long       nodes;                  /* node count */
    unsigned long       factor;                 /* fill factor */
    struct stblnode **  table;
} stable_t;


typedef struct stblnode {
/*private:*/
    uint32_t            __magic;                /* magic */
    uint32_t            __hash;
    struct stblnode *   __next;
    union {
        int32_t         i32;
        void *          ptr;
    } __u;
    char                __str[1];

/*public*/
#define stbl_uptr       __u.ptr
#define stbl_ui32       __u.i32
#define stbl_key        __str
} stblnode_t;


typedef struct {
/*private:*/
    unsigned            magic;
    stable_t *          st;
    unsigned            bucket;
    stblnode_t *        node;
} stblcursor_t;


extern int                  stbl_open(stable_t *st, unsigned buckets, unsigned factor);
extern void                 stbl_close(stable_t *st);
extern void                 stbl_clear(stable_t *st);

extern stblnode_t *         stbl_new(stable_t *st, const char *str);
extern stblnode_t *         stbl_nnew(stable_t *st, const char *str, int len);
extern int                  stbl_delete(stable_t *st, const char *str);
extern int                  stbl_ndelete(stable_t *st, const char *str, int len);

extern const char *         stbl_ninsert(stable_t *st, const char *str, int len);
extern const char *         stbl_insert(stable_t *st, const char *str);
extern const char *         stbl_exist(stable_t *st, const char *str);
extern const char *         stbl_nexist(stable_t *st, const char *str, int len);
extern stblnode_t *         stbl_find(stable_t *st, const char *str);
extern stblnode_t *         stbl_nfind(stable_t *st, const char *str, int len);

extern int                  stbl_count(stable_t *st);
extern stblnode_t *         stbl_first(stable_t *st, stblcursor_t *cursor);
extern stblnode_t *         stbl_next(stblcursor_t *cursor);

__CEND_DECLS

#endif /*GR_STABLE_H_INCLUDED*/
