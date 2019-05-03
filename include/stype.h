#ifndef GR_STYPE_H_INCLUDED
#define GR_STYPE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_stype_h,"$Id: stype.h,v 1.11 2019/03/15 23:03:11 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: stype.h,v 1.11 2019/03/15 23:03:11 cvsuser Exp $
 * Self-organising integer data structure
 *
 *  library of routines for storing a data structure where the primary key is an integer. 
 *
 *  Goal keep overheads to an absolute minimum yet have a pretty good lookup time. 
 *
 *  The implementation maintains an array of integer/pointer pairs, which is sorted 
 *  into order every when necessary; allowing access in numerical order.
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

typedef uint32_t stypekey_t;                    /* Key object */

typedef struct {
    stypekey_t          se_key;                 /* Key value */
    void *              se_ptr;                 /* Pointer to value */
} sentry_t;

struct sbitmap;

typedef struct {
#if defined(STYPE_INTERNAL)
/*private:*/
    uint32_t            st_magic;               /* Structure magic */
    uint32_t            st_dirty;               /* Table status */
    uint32_t            st_bsize;               /* Bitmap size. */
    uint32_t            st_bused;               /* Number of used bitmap entries */
    struct sbitmap *    st_bitmaps;             /* Bitmap structure. */
    uint32_t            st_size;                /* Number of entries available, total size. */
    uint32_t            st_used;                /* Number entries allocated. */
    uint32_t            st_seqno;               /* Edit sequence number */
    sentry_t *          st_block;               /* Array allocated to entry. */
#else
    uint32_t            __st_magic;
    uint32_t            __st_dirty;
    uint32_t            __st_bsize;
    uint32_t            __st_bused;
    void *              __st_bitmaps;
    uint32_t            __st_count;
    uint32_t            __st_used;
    uint32_t            __st_seqno;
    void *              __st_block;
#endif
} stype_t;

typedef struct {
/*private;*/
    uint32_t            sc_magic;               /* structure magic */
    uint32_t            sc_index;
    uint32_t            sc_seqno;
    stype_t *           sc_stype;
    sentry_t *          sc_last;
} stypecursor_t;

stype_t *               stype_alloc(void);
void                    stype_free(stype_t *sp);

int                     stype_used(stype_t *sp);
sentry_t *              stype_block(stype_t *sp);

int                     stype_insert(stype_t *sp, stypekey_t key, void *ptr);
sentry_t *              stype_lookup(stype_t *sp, stypekey_t key);
void                    stype_replace(stype_t *sp, sentry_t * sep, void *ptr);
void                    stype_remove(stype_t *sp, sentry_t *sep);
int                     stype_delete(stype_t *sp, stypekey_t key);

sentry_t *              stype_first(stype_t *sp, stypecursor_t *cursor);
sentry_t *              stype_next(stypecursor_t *cursor);

#endif /*GR_STYPE_H_INCLUDED*/
