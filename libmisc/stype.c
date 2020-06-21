#include <edidentifier.h>
__CIDENT_RCSID(gr_stype_c,"$Id: stype.c,v 1.20 2020/06/20 19:29:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: stype.c,v 1.20 2020/06/20 19:29:20 cvsuser Exp $
 * Simple integer data table.
 *
 *
 *
 * Copyright (c) 1998 - 2020, Adam Young.
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

#define ED_LEVEL 1
#define ED_ASSERT

#include <edtypes.h>
#include <edassert.h>

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#if !defined(DO_LOCALMAIN)
#include <chkalloc.h>
#endif
#define  STYPE_INTERNAL
#include <stype.h>
#include <libmisc.h>

#define SB_MASK         0xff
#define SB_KEYBITS      (sizeof(stypekey_t) * 8)
#define SB_RANGE(x)     ((x) & ~SB_MASK)
#define SB_BITMAP       ((SB_MASK + 1)/SB_KEYBITS)

#define SB_SET(s,c)     ((s)->sb_bits[(c)/SB_KEYBITS] |=  (1 << (c)%SB_KEYBITS))
#define SB_TST(s,c)     ((s)->sb_bits[(c)/SB_KEYBITS] &   (1 << (c)%SB_KEYBITS))
#define SB_CLR(s,c)     ((s)->sb_bits[(c)/SB_KEYBITS] &= ~(1 << (c)%SB_KEYBITS))

#define SB_INCREMENT    32
#define SB_BMINCREMENT  2

#ifndef MKMAGIC
#define MKMAGIC(a, b, c,d) \
                (((uint32_t)(a))<<24|((uint32_t)(b))<<16|(c)<<8|(d))
#endif

#define STMAGIC         MKMAGIC('s','t','y','l')
#define SNMAGIC         MKMAGIC('s','t','y','n')
#define SCMAGIC         MKMAGIC('s','t','y','c')
#define SMMAGIC1        MKMAGIC('s','t','m','1')
#define SMMAGIC2        MKMAGIC('s','t','m','2')

typedef struct sbitmap {
    uint32_t            sb_magic1;
    stypekey_t          sb_start;
    uint32_t            sb_count;
    uint32_t            sb_bits[SB_BITMAP];
    uint32_t            sb_magic2;
} sbitmap_t;

static void             stype_sort(stype_t *sp);
static int              stype_sort_fn(const void *a1, const void *a2);
static int              stype_lookup_fn(const void *pkey, const void *val);


/*  Function:           stype_alloc
 *      Allocate a new stype object.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing.
 */
stype_t *
stype_alloc(/*unsigned flags*/void)
{
    stype_t *sp;

    if (NULL != (sp = chk_calloc(sizeof(stype_t), 1))) {
        sp->st_magic = STMAGIC;
        sp->st_dirty = FALSE;

        sp->st_bsize = 1;
        sp->st_bused = 0;
        sp->st_bitmaps = chk_calloc(sp->st_bsize, sizeof(sbitmap_t));

        sp->st_size  = SB_INCREMENT;
        sp->st_used  = 0;
        sp->st_block = chk_calloc(sp->st_size, sizeof(sentry_t));
    }
    return sp;
}


/*  Function:           stype_free
 *      Free an allocated block.
 *
 *  Parameters:
 *      sp -                Table object.
 *
 *  Returns:
 *      nothing.
 */
void
stype_free(stype_t *sp)
{
    if (NULL != sp) {
        assert(STMAGIC == sp->st_magic);
        chk_free(sp->st_bitmaps);
        chk_free(sp->st_block);
        chk_free(sp);
    }
}


/*  Function:           stype_bitmap
 *      Add key to the bitmap in the stype structure, if it does not exist
 *      then a new element shall be allocated.
 *
 *  Parameters:
 *      sp -                Table object.
 *      key -               Key.
 *
 *  Returns:
 *      *true* on success, otherwise *fa;se*.
 */
static int
stype_bitmap(stype_t *sp, stypekey_t key)
{
    sbitmap_t *bp;
    stypekey_t start;
    uint32_t idx;

    /* lookup */
    start = SB_RANGE(key);
    for (idx = 0, bp = sp->st_bitmaps; idx < sp->st_bused; ++idx, ++bp) {

        assert(SMMAGIC1 == bp->sb_magic1);
        assert(bp->sb_count);
        assert(SMMAGIC2 == bp->sb_magic2);

        if (bp->sb_start == start) {
            break;
        }
    }

    /* create (if required) */
    if (idx >= sp->st_bused) {
        /*
         *  resize bitmap
         *      entries are managed in a single linear block
         */
        if (idx >= sp->st_bsize) {
            void * nb;

            sp->st_bsize += SB_BMINCREMENT;
            if (NULL == (nb =
                    chk_realloc(sp->st_bitmaps, sp->st_bsize * sizeof(sbitmap_t)))) {
                sp->st_bsize -= SB_BMINCREMENT;
                return 0;                       /* allocation memory */
            }
            sp->st_bitmaps = nb;
        }

        assert(sp->st_bused < sp->st_bsize);
        bp = &sp->st_bitmaps[sp->st_bused++];
        memset(bp, 0, sizeof(sbitmap_t));
        bp->sb_start  = start;
        bp->sb_magic1 = SMMAGIC1;
        bp->sb_magic2 = SMMAGIC2;
    }

    key -= start;
    SB_SET(bp, key);
    ++bp->sb_count;
    return 1;
}


/*  Funcion:            stype_insert
 *      Insert a new netry into the table.
 *
 *      Add a new entry to the object. No validation done for duplicates.
 *
 *  Parameters:
 *      sp -                Table object.
 *      key -               Key.
 *      ptr -               Associated key value.
 *
 *  Returns:
 *      *true* on success, otherwise *fa;se*.
 */
int
stype_insert(stype_t *sp, stypekey_t key, void *ptr)
{
    sentry_t *sep;

    assert(STMAGIC == sp->st_magic);
    assert(NULL == stype_lookup(sp, key));

    if (! stype_bitmap(sp, key)) {
        return 0;
    }

    if (sp->st_used >= sp->st_size) {
        /*
         *  resize buffer
         *      entries are managed in a single linear block
         */
        void *nb;

        sp->st_size += SB_INCREMENT;
        if (NULL == (nb =
                chk_realloc(sp->st_block, sp->st_size * sizeof(sentry_t)))) {
            sp->st_size -= SB_INCREMENT;
            return -1;
        }
        sp->st_block = nb;
    }

    assert(sp->st_used < sp->st_size);
    sep = &sp->st_block[sp->st_used++];
    sep->se_key = key;
    sep->se_ptr = ptr;

    if (sp->st_used > 1 && key < sep[-1].se_key) {
        sp->st_dirty = TRUE;                    /* new key out of order */
    }

    ++sp->st_seqno;                             /* edit sequnence number */
    return 1;
}


/*  Function:           stype_lookup
 *      Lookup by key
 *
 *  Parameters:
 *      sp -                Table object.
 *      key -               Key.
 *
 *  Returns:
 *      Address of entry, otherwise NULL.
 */
sentry_t *
stype_lookup(stype_t *sp, stypekey_t key)
{
    stypekey_t start;
    uint32_t idx;
    sbitmap_t *bp;

    assert(sp);
    assert(STMAGIC == sp->st_magic);

    start = SB_RANGE(key);                      /* scan bitmap */
    for (bp = sp->st_bitmaps, idx = 0; idx < sp->st_bused; ++idx, ++bp) {

        assert(SMMAGIC1 == bp->sb_magic1);
        assert(bp->sb_count);
        assert(SMMAGIC2 == bp->sb_magic2);

        if (bp->sb_start == start) {
            uint32_t k = (uint32_t)(key - start);

            assert(k <= SB_MASK);
            if (! SB_TST(bp, k)) {
                return NULL;                    /* not found */
            }
            break;
        }
    }

    if (idx >= sp->st_bused) {
        return NULL;
    }

    if (sp->st_dirty) {
        stype_sort(sp);
    }

    return bsearch(&key, sp->st_block, sp->st_used, sizeof(sentry_t), stype_lookup_fn);
}


static void
stype_sort(stype_t * sp)
{
    qsort(sp->st_block, sp->st_used, sizeof(sentry_t), stype_sort_fn);
    sp->st_dirty = FALSE;
}


static int
stype_sort_fn(const void *a1, const void *a2)
{
    const sentry_t *p1 = a1;
    const sentry_t *p2 = a2;

    return (int) (p1->se_key - p2->se_key);
}


static int
stype_lookup_fn(const void *pkey, const void *val)
{
    const stypekey_t *key = pkey;
    const sentry_t *p = val;

    return (*key > p->se_key) ? 1 : (*key < p->se_key) ? -1 : 0;
}


/*  Function to replace an entry in the table.
 *
 *      This should only be called if you've already done an stype_lookup() and know that
 *      you are not going to end up with an out of order table.
 */
void
stype_replace(stype_t *sp, sentry_t *sep, void *val)
{
    assert(STMAGIC == sp->st_magic);
    sp = sp;
    sep->se_ptr = val;
}


/*  Function:           stype_used
 *      Retrieve the allocated element count.
 *
 *  Parameters:
 *      sp -                Table object.
 *
 *  Returns:
 *      Allocate element count.
 */
int
stype_used(stype_t * sp)
{
    assert(sp);
    assert(STMAGIC == sp->st_magic);
    return sp->st_used;
}


/*  Function:           stype_block
 *      Retrieve the underlying table storage.
 *
 *  Parameters:
 *      sp -                Table object.
 *
 *  Returns:
 *      Underlying block.
 */
sentry_t *
stype_block(stype_t *sp)
{
    assert(sp);
    assert(STMAGIC == sp->st_magic);
    return sp->st_block;
}


/*  Function:           stype_remove
 *      Remove the specified element.
 *
 *  Parameters:
 *      sp -                Table object.
 *      sep -               Entry record.
 *
 *  Returns:
 *      nothing
 */
void
stype_remove(stype_t * sp, sentry_t *sep)
{
    uint32_t idx, remaining;
    stypekey_t key, start;
    sbitmap_t *bp;

    assert(sp && sep);
    assert(STMAGIC == sp->st_magic);
    assert(sep >= sp->st_block);
    assert(sep < sp->st_block + sp->st_bused);

    idx = sep - sp->st_block;
    assert(idx < sp->st_used);

    key = sep->se_key;
    if ((remaining = --sp->st_used - idx) > 0) {
        memmove(sep, (const void *)(sep + 1), remaining * sizeof(sentry_t));
    }

    start = SB_RANGE(key);
    for (bp = sp->st_bitmaps, idx = 0; idx < sp->st_bused; ++idx, ++bp) {

        assert(SMMAGIC1 == bp->sb_magic1);
        assert(bp->sb_count);
        assert(SMMAGIC2 == bp->sb_magic2);

        if (bp->sb_start == start) {
            uint32_t k = (uint32_t)(key - start);

            assert(k <= SB_MASK);
            SB_CLR(bp, k);

            if (0 == --bp->sb_count) {
                if ((remaining = --sp->st_bused - idx) > 0) {
                    memmove(bp, (const void *)(bp + 1), remaining * sizeof(sbitmap_t));
                }
            }
            break;
        }
    }

    ++sp->st_seqno;                             /* edit sequnence number */
}


/*  Function:           stype_delete
 *      Remove the specified element by key.
 *
 *  Parameters:
 *      sp -                Table object.
 *      key -               Entry key.
 *
 *  Returns:
 *      *true* on success, otherwise *false*.
 */
int
stype_delete(stype_t * sp, stypekey_t key)
{
    sentry_t *sep;

    if (NULL == (sep = stype_lookup(sp, key))) {
        return 0;                               /* key not found */
    }
    stype_remove(sp, sep);
    return 1;
}


sentry_t *
stype_first(stype_t * sp, stypecursor_t *cursor)
{
    assert(sp && cursor);
    assert(STMAGIC == sp->st_magic);
    memset(cursor, 0, sizeof(stypecursor_t));
    cursor->sc_magic = SCMAGIC;
    cursor->sc_stype = sp;
    cursor->sc_seqno = sp->st_seqno;
    return stype_next(cursor);
}


sentry_t *
stype_next(stypecursor_t *cursor)
{
    stype_t * sp;
    sentry_t * sep = NULL;
    uint32_t index;

    assert(cursor);
    assert(SCMAGIC == cursor->sc_magic);
    sp = cursor->sc_stype;
    assert(STMAGIC == sp->st_magic);
    assert(cursor->sc_seqno == sp->st_seqno);

    index = cursor->sc_index;
    while (NULL == sep && index < sp->st_used) {
        sep = sp->st_block + index++;
    }
    cursor->sc_index = index;
    return sep;
}


#if defined(DO_LOCALMAIN)
/*
 *  test framework ...
 */
void *                  chk_calloc(size_t elm, size_t size) { return calloc(elm, size); }
void *                  chk_realloc(void *p, size_t size) { return realloc(p, size); }
void                    chk_free(void *p) { free(p); }

#if defined(ELEMENT_HUGE)
#define ELEMENTS        100000
#elif defined(ELEMENT_LARGE)
#define ELEMENTS        10000
#else
#define ELEMENTS        1000
#endif

static int              elements[ELEMENTS];
static void *           data[ELEMENTS];

static int
Test1(stype_t *tbl, uint32_t loop)
{
    sentry_t *entry;
    stypecursor_t cursor;
    uint32_t i;

    printf("test1 (loop:%U):\n", loop);

    printf(" o insert\n");
    for (i = 0; i < ELEMENTS; ++i) {
        sentry_t *entry;

        elements[i] = (int) rand();
        data[i] = (void *) rand();
        assert(1 == st_insert(tbl, elements[i], data[i]));
    }

    assert(ELEMENTS == stype_used(tbl));

    printf(" o lookup\n");
    for (i = 0; i < ELEMENTS; ++i) {
        assert(NULL != (entry = stype_lookup(tbl, elements[i])));
        assert(entry->se_ptr == data[i]);
    }

    printf(" o cursor\n");
    i = 0;
    if (NULL != (entry =  st_first(tbl, &cursor)))
        do {
            assert(i < ELEMENTS);
            ++i;
        } while (NULL != (entry = stype_next(&cursor)));
    assert(i == stype_used(tbl));

    printf(" o delete\n");
    for (i = 0; i < ELEMENTS; ++i) {
        assert(1 == st_delete(tbl, elements[i]));
        assert(NULL == stype_lookup(tbl, elements[i]));
    }
    assert(0 == stype_used(tbl));
}


int
main(void)
{
    stype_t *tbl;

    printf("stype start\n");

    printf(" o create\n");
    tbl = st_alloc();

    Test1(tbl, 1);
    Test1(tbl, 2);

    printf(" o close\n");
    st_free(tbl);

    printf("stype complete\n");
    return 0;
}
#endif  //DO_LOCALMAIN
