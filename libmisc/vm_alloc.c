#include <edidentifier.h>
__CIDENT_RCSID(gr_vm_alloc_c,"$Id: vm_alloc.c,v 1.21 2022/09/20 15:19:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vm_alloc.c,v 1.21 2022/09/20 15:19:12 cvsuser Exp $
 * Simple buffer pool.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <string.h>
#include <assert.h>
#include <errno.h>

#if !defined(DO_LOCALMAIN)
#include <chkalloc.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include "vm_alloc.h"

    /*
     *  DEFSIZE             Default pool arean
     *
     *  MAGIC               Structure magic, protects data within a unallocated
     *                      cell image against a user addressing a previously
     *                      freed buffer.
     *
     *  BLKSIZE             Determine physical size of each cell within the arena.
     *
     *  BLKADDR             Address the specified block within the arena.
     *
     *  BUF2BLK             Convert a buffer (user) address to a block address.
     *
     *  BLK2BUF             Convert a block address to a buffer address.
     */
#ifndef MKMAGIC
#define MKMAGIC(a,b,c,d)    (((unsigned long)(a))<<24|((unsigned long)(b))<<16|(c)<<8|(d))
#endif

#define DEFSIZE             ((2 * 8192) - (128 + MINBLKSIZ))

#define POOLMAGIC           MKMAGIC('V','M','b','p')
#define BANKMAGIC           MKMAGIC('V','M','b','k')

#define FENCE1              0x05050505
#define FENCE2_FREE         0x54545454
#define FENCE2_USED         0x45454545
#define FENCE3_FREE         0x50505050

#define BLKSIZE(p)          (sizeof(vmheader_t) + (p)->vp_blksiz)
#define MINBLKSIZ           (sizeof(vmblock_t) - sizeof(vmheader_t))

#define BLKADDR(p, b, i)    (vmblock_t *) \
                                ((char *)((b) + 1) + (BLKSIZE(p) * (i)))

#define BUF2BLK(b)          ((vmblock_t *) (((vmheader_t *)(b)) - 1))
#define BLK2BUF(c)          ((void *) (((vmheader_t *)(c)) + 1))

    /*
     *  Pool bank
     */
typedef struct _vmbank {
    union {
        vmalignment_t       alignment;
        struct {
            unsigned long   magic;
            TAILQ_ENTRY(_vmbank) node;
            void *          owner;
            unsigned long   fence;
        } d;
    } u;

#define vb_magic            u.d.magic
#define vb_node             u.d.node
#define vb_owner            u.d.owner
#define vb_fence            u.d.fence

} vmbank_t;

    /*
     *  Block representation, where vmheader should be used for any pointer
     *  arithmetic, where as vmblock is used for all structure access.
     *
     *      vmheader_t      Header of each buffer, visible whilst the buffer
     *                      is both allocated and freed.  Represent the system
     *                      overhead for each buffer.
     *
     *      vmblock_t       Buffer block image, visible only whilst the buffer
     *                      is contained within free-list.  Warning, this image
     *                      overlays the top of buffer space.  Additional
     *                      fields to vmblock_t represent the smallest allowed
     *                      buffer size.
     */
typedef struct _vmheader {
    union {
        vmalignment_t       alignment;
        struct {
            unsigned long   fence1;
            void *          owner;
            unsigned long   count;
            unsigned long   fence2;
        } d;
    } u;
} vmheader_t;


typedef struct _vmblock {
    vmheader_t              vm_header;
#define vm_fence1           vm_header.u.d.fence1
#define vm_owner            vm_header.u.d.owner
#define vm_count            vm_header.u.d.count
#define vm_fence2           vm_header.u.d.fence2
    TAILQ_ENTRY(_vmblock)   vm_node;
    unsigned long           vm_fence3;
} vmblock_t;


int                         vm_dflag = 0;

#if defined(DO_LOCALMAIN)
void *                      chk_alloc(size_t size) { return malloc(size); }
void *                      chk_realloc(void *p, size_t size) { return realloc(p, size); }
void                        chk_free(void *p) { free(p); }
#endif
static int                  PoolNew(vmpool_t *pool);
static int                  PoolCheck(vmpool_t *pool, const int all);


int
vm_init(vmpool_t *pool, unsigned ublksiz, unsigned size)
{
    unsigned long blksiz, elems;

    assert(pool != NULL);
    if (pool == NULL) {
        return (EINVAL);
    }

    /* blocksize, min and round */
    blksiz = ublksiz;
    if (blksiz < MINBLKSIZ) {
        blksiz = MINBLKSIZ;
    }
    blksiz = ALIGN_UP(blksiz, VMALIGNMENT);

    /* size arena, def, verify and round */
    if (size == 0) {
        size = DEFSIZE;
    }
    if (size < sizeof(vmheader_t) + blksiz) {
        return (EINVAL);
    }
    size = ALIGN_UP(size, VMALIGNMENT);

    /* build image */
    elems = size / (sizeof(vmheader_t) + blksiz);
    pool->vp_magic   = POOLMAGIC;
    pool->vp_blksiz  = blksiz;
    pool->vp_ublksiz = ublksiz;                 /* original 'user' size */
    pool->vp_elems   = elems;
    pool->vp_flags   = 0;
    pool->vp_size    = sizeof(vmbank_t) + (elems * BLKSIZE(pool));
    TAILQ_INIT(&pool->vp_bankq);
    TAILQ_INIT(&pool->vp_freeq);
    pool->vp_freecnt = 0;

#if !defined(DO_LOCALMAIN)
    if (vm_dflag) {
        pool->vp_flags = 0x1001;                /* memory check special */
        return 0;
    }
#endif
    return PoolNew(pool);
}


void *
vm_alloc(vmpool_t *pool, unsigned ublksiz)
{
    if (pool->vp_magic == 0) {
        vm_init( pool, ublksiz, 0);             /* auto construct */
    } else {
        assert( pool->vp_ublksiz == ublksiz );  /* otherwise verify ulbksiz */
    }
    return vm_new(pool);
}


void
vm_zap(vmpool_t *pool)
{
    vmbank_t *bank;

    assert(pool->vp_magic == POOLMAGIC);
    while ((bank = TAILQ_FIRST(&pool->vp_bankq)) != NULL) {
        assert(bank->vb_magic == BANKMAGIC);
        assert(bank->vb_owner == (void *)pool);
        assert(bank->vb_fence == BANKMAGIC);

        TAILQ_REMOVE(&pool->vp_bankq, bank, vb_node);
        chk_free(bank);
    }
    TAILQ_INIT(&pool->vp_freeq);
    pool->vp_freecnt = 0;
}


void
vm_destroy(vmpool_t *pool)
{
    if (pool) {
        vm_zap(pool);
        pool->vp_magic = 0;
    }
}


void *
vm_new(vmpool_t *pool)
{
    void *buf = NULL;
    vmblock_t *block;

    assert(pool->vp_magic == POOLMAGIC);
#if !defined(DO_LOCALMAIN)
    if ((pool->vp_flags & 0x1001) == 0x1001) {
        return chk_alloc(pool->vp_ublksiz);
    }
#endif

    if ((block = TAILQ_FIRST(&pool->vp_freeq)) == NULL) {
        assert(pool->vp_freecnt == 0);
        if (PoolNew(pool) == 0) {
            block = TAILQ_FIRST(&pool->vp_freeq);
        }
    }

    assert(block);

    if (block) {
        assert(pool->vp_freecnt > 0);
        assert(block->vm_fence1 == FENCE1);
        assert(block->vm_count  == 0);
        assert(block->vm_fence2 == FENCE2_FREE);
        assert(block->vm_fence3 == FENCE3_FREE);

        TAILQ_REMOVE(&pool->vp_freeq, block, vm_node);
        block->vm_count = 1;
        block->vm_fence2 = FENCE2_USED;
        --pool->vp_freecnt;

        buf = BLK2BUF(block);
#if defined(FILL_MEMORY)
        memset(buf, 'A', pool->vp_blksiz);
#elif defined(ZERO_MEMORY) || defined(DO_LOCALMAIN)
        memset(buf, 0, pool->vp_blksiz);
#endif
        assert(block->vm_fence2 == FENCE2_USED);
    }
    return (buf);
}


int
vm_reference(vmpool_t *pool, void *buf)
{
    vmblock_t *block;

    assert(pool->vp_magic == POOLMAGIC);
#if !defined(DO_LOCALMAIN)
    if ((pool->vp_flags & 0x1001) == 0x1001) {
        return -1;                              /* not supported */
    }
#endif
    block = BUF2BLK(buf);
    assert(block->vm_fence1 == FENCE1);
    assert(block->vm_fence2 == FENCE2_USED);
    assert(block->vm_owner  == pool);
    assert(block->vm_count  >= 1);
    return ++block->vm_count;
}


int
vm_unreference(vmpool_t *pool, void *buf)
{
    vmblock_t *block;

    assert(pool->vp_magic == POOLMAGIC);
#if !defined(DO_LOCALMAIN)
    if ((pool->vp_flags & 0x1001) == 0x1001) {
        return -1;                              /* not supported */
    }
#endif
    block = BUF2BLK(buf);
    assert(block->vm_fence1 == FENCE1);
    assert(block->vm_fence2 == FENCE2_USED);
    assert(block->vm_owner  == pool);
    assert(block->vm_count  >= 1);
    
    if (0 == --block->vm_count) {
        block->vm_fence2 = FENCE2_FREE;
        TAILQ_INSERT_HEAD(&pool->vp_freeq, block, vm_node);
        block->vm_fence3 = FENCE3_FREE;
        ++pool->vp_freecnt;
    }
    return block->vm_count;
}


void
vm_free(vmpool_t *pool, void *buf)
{
    vmblock_t *block;

    assert(pool->vp_magic == POOLMAGIC);
#if !defined(DO_LOCALMAIN)
    if ((pool->vp_flags & 0x1001) == 0x1001) {
        if (buf) {
            chk_free(buf);
        }
        return;
    }
#endif

    block = BUF2BLK(buf);
    assert(block->vm_fence1 == FENCE1);
    assert(block->vm_fence2 == FENCE2_USED);
    assert(block->vm_owner == pool);
    assert(block->vm_count == 1);

    block->vm_count = 0;
    block->vm_fence2 = FENCE2_FREE;
    TAILQ_INSERT_HEAD(&pool->vp_freeq, block, vm_node);
    block->vm_fence3 = FENCE3_FREE;
    ++pool->vp_freecnt;
}


static int
PoolNew(vmpool_t *pool)
{
    vmbank_t *bank;
    vmblock_t *block;
    unsigned cnt;

    if ((bank = chk_alloc(pool->vp_size)) == NULL) {
        return ENOMEM;
    }

    bank->vb_magic = BANKMAGIC;
    TAILQ_INSERT_HEAD(&pool->vp_bankq, bank, vb_node);
    bank->vb_owner = (void *)pool;
    bank->vb_fence = BANKMAGIC;

    for (cnt = 0; cnt < pool->vp_elems; ++cnt) {
        block = BLKADDR(pool, bank, cnt);
        block->vm_fence1 = FENCE1;
        block->vm_owner  = pool;
        block->vm_count  = 0;
        block->vm_fence2 = FENCE2_FREE;
        TAILQ_INSERT_HEAD(&pool->vp_freeq, block, vm_node);
        block->vm_fence3 = FENCE3_FREE;
        ++pool->vp_freecnt;
    }
    return (0);
}


#if (NOTUSED)
static int
PoolCheck(vmpool_t *pool, const int all)
{
    vmbank_t *bank;

    assert(pool->vp_elems == pool->vp_size / (sizeof(vmheader_t) + pool->vp_blksiz));

    assert(pool->vp_freecnt == 0 || TAILQ_FIRST(&pool->vp_freeq) != NULL);

    if ((bank = TAILQ_FIRST(&pool->vp_bankq)) != NULL)
        do {
            assert(bank->vb_magic == BANKMAGIC);
            assert(bank->vb_owner == (void *)pool);
            assert(bank->vb_fence == BANKMAGIC);

        } while ((bank = TAILQ_NEXT(bank, vb_node)) != NULL);

    if (all) {
        if ((bank = TAILQ_FIRST(&pool->vp_bankq)) != NULL)
            do {
                vmblock_t *block, *t_block;
                unsigned long cnt;

                for (cnt = 0; cnt < pool->vp_elems; cnt++) {
                    block = BLKADDR(pool, bank, cnt);

                    assert(block->vm_fence1 == FENCE1);
                    assert(block->vm_owner == pool);

                    if (block->vm_fence2 != FENCE2_FREE) {
                        assert(block->vm_count >= 1);
                        assert(block->vm_fence2 == FENCE2_USED);

                    } else {
                        assert(block->vm_count == 0);
                        assert(block->vm_fence3 == FENCE3_FREE);

                        t_block = TAILQ_FIRST(&pool->vp_freeq);
                        while (t_block != NULL && t_block != block) {
                            t_block = TAILQ_NEXT(t_block, vm_node);
                        }
                        assert(t_block != NULL);
                    }
                }
            } while ((bank = TAILQ_NEXT(bank, vb_node)) != NULL);
    }
    return (0);
}
#endif


#if defined(DO_LOCALMAIN)

#define PTRS            100
#define BLOCK           69

static void *           ptrs[PTRS];

typedef union {
    char                x_char;
    int                 x_int;
    long                x_long;
    double              x_double;
    float               x_float;
} x_t;


int
main(void)
{
    vmpool_t pool;
    x_t *ptr;
    int i, i2;

    printf("vm_alloc\n");
    vm_init(&pool, BLOCK, 0);
    for (i2 = 0; i2 < 3; i2++) {
        for (i = 0; i < PTRS; i++) {
            if ((ptr = vm_new(&pool)) != NULL) {
                ptrs[i] = ptr;
                (void) memset(ptr, 'A', BLOCK);
                ptr->x_char = 0;
                ptr->x_int = 0;
                ptr->x_long = 0;
                ptr->x_double = 0;
                ptr->x_float = 0;
            }
        }
        for (i = 0; i < PTRS; i++) {
            vm_free(&pool, ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    vm_zap(&pool);
    vm_destroy(&pool);
    printf("...done\n");
    return (0);
}

#endif  /*DO_LOCALMAIN*/
