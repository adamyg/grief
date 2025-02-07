#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_cache_c,"$Id: vfs_cache.c,v 1.18 2025/02/07 03:03:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_cache.c,v 1.18 2025/02/07 03:03:23 cvsuser Exp $
 * Virtual file system interface - name cache
 *
 *      Names found by directory scans are retained in a cache for future reference.
 *
 *      The cache is indexed using a red-black tree and ags is managed by LRU, so frequently used
 *      names will hang around. Cache is indexed by hash value obtained from (vp, name) where vp
 *      refers to the directory containing name.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#include <editor.h>
#include <errno.h>

#include "vfs_internal.h"
#include "vfs_node.h"
#include "vfs_cache.h"


/*
 *      For simplicity (and economy of storage), names longer than a maximum length of
 *      VFS_MAXCACHENAMLEN, all thr absolue path length exceeding VFS_CACHEPATHLEN
 *      not cached; they occur infrequently in any case, and are almost never of interest.
 */
#ifndef VFS_CACHEPATHLEN
#define VFS_CACHEPATHLEN    128
#endif
#ifndef VFS_CACHENAMELEN
#define VFS_CACHENAMELEN    32
#endif

#define VFS_CACHESIZE       (100 * 1024)        /* 100k */
#define VFS_CACHEELEM       2500                /* 2,500 files */

static int              cachenode_compare(struct vfs_node *a, struct vfs_node *b);

/*FIXME*/
extern int              file_cmp(const char *f1, const char *f2);

RB_PROTOTYPE(cacherb, vfs_node, v_cachetree, cachenode_compare);
RB_GENERATE(cacherb, vfs_node, v_cachetree, cachenode_compare);


void
vfs_cache_init(struct vfs_cache *cache, unsigned flags)
{
    assert(cache);
    memset(cache, 0, sizeof(struct vfs_cache));
    cache->c_magic = VCACHE_MAGIC;
    cache->c_flags = flags;
    RB_INIT(&cache->c_tree);
    TAILQ_INIT(&cache->c_lru);
}


void
vfs_cache_destroy(struct vfs_cache *cache)
{
    assert(cache);
    assert(VCACHE_MAGIC == cache->c_magic);
    assert(0 == cache->c_elem);
    assert(0 == cache->c_size);
    cache->c_magic = ~VCACHE_MAGIC;
}


struct vfs_node *
vfs_cache_lookup(struct vfs_cache *cache, const char *abspath, unsigned abslen)
{
    struct vfs_node *node = NULL;

    assert(cache);
    assert(VCACHE_MAGIC == cache->c_magic);
    assert(abspath);

    if (abslen) {
        struct vfs_node find = {0};
        cacherb_t *rb = &cache->c_tree;

        find.v_magic = VNODE_MAGIC;
        find.v_cache = cache;
        find.v_cachehash = vfs_name_hash(abspath, abslen);
        find.v_cachename = abspath;
        find.v_cachelen = abslen;
        rb = &cache->c_tree;
        node = RB_FIND(cacherb, rb, &find);
    }
    VFS_TRACE(("\tvfs_cache_lookup(%.*s,%d) : %p\n", abslen, abspath, abslen, node))
    return node;
}


/*  Function:           vfs_cache_link
 *      Link the specified node to the given cache.
 *
 *  Parameters:
 *      cache -             Cache to associated node.
 *      node -              Address of node object.
 *      abspath -           Absolute path node.
 *
 *  Returns:
 *      1 if successful, otherwise 0 is already associated or caching is
 *      disabled/not suitable.
 */
int
vfs_cache_link(struct vfs_cache *cache, struct vfs_node *node, char *abspath)
{
    assert(cache);
    assert(VCACHE_MAGIC == cache->c_magic);
    assert(node);
    assert(VNODE_MAGIC == node->v_magic);
    assert(abspath);
    assert(abspath[0]);

    if ((node->v_flags & VNODE_FCACHED) == 0) {
        assert(NULL == node->v_cache);

        if (VNODE_DIR == node->v_type || VNODE_REG == node->v_type) {
            unsigned abslen = (unsigned)strlen(abspath);

            if (abslen && abslen < VFS_CACHEPATHLEN) {
                cacherb_t *rb = &cache->c_tree;
                struct cachelru_t *lru = &cache->c_lru;

                /* expire stale node based on limits, LRU */
                if ((cache->c_size > VFS_CACHESIZE) ||
                        (cache->c_elem > VFS_CACHEELEM)) {
                    vfs_cache_unlink(TAILQ_FIRST(lru));
                }

                /* add new node */
                node->v_cache = cache;
                node->v_cachehash = vfs_name_hash(abspath, abslen);
                node->v_cachename = abspath;
                node->v_cachelen = abslen;
                assert(NULL == RB_FIND(cacherb, rb, node));
                RB_INSERT(cacherb, rb, node);
                TAILQ_INSERT_TAIL(lru, node, v_cachelru);
                node->v_flags |= VNODE_FCACHED;
                cache->c_size += abslen;
                ++cache->c_elem;
                return 1;
            }
        }
    }
    chk_free(abspath);
    return 0;
}


/*  Function:           vfs_cache_unlink
 *      Unlink the specified node from it associated cache.
 *
 *  Parameters:
 *      node -              Address of node object.
 *
 *  Returns:
 *      1 if successful, otherwise 0 if the node was not associated.
 */
int
vfs_cache_unlink(struct vfs_node *node)
{
    struct vfs_cache *cache;
    int ret = 0;

    assert(node);
    assert(VNODE_MAGIC == node->v_magic);

    if (NULL == (cache = node->v_cache)) {
        assert(0 == (node->v_flags & VNODE_FCACHED));

    } else {
        cacherb_t *rb = &cache->c_tree;
        struct cachelru_t *lru = &cache->c_lru;

        assert(VCACHE_MAGIC == cache->c_magic);
        assert(cache->c_size);
        assert(cache->c_elem);
        assert(node->v_flags & VNODE_FCACHED);
        assert(node->v_cachename);

        RB_REMOVE(cacherb, rb, node);
        TAILQ_REMOVE(lru, node, v_cachelru);
        --cache->c_elem;
        cache->c_size -= (unsigned)strlen(node->v_cachename);
        chk_free((char *)node->v_cachename);
        node->v_cachename = NULL;
        node->v_cachehash = 0;
        node->v_flags &= ~VNODE_FCACHED;
        node->v_cache = NULL;
        ret = 1;
    }
    return ret;
}


/*  Function:           cachenode_compare
 *      Compare two nodes within a cache cache, returning their comparsion
 *      value in cache order.
 *
 *   Parameters:
 *       a -                First node image.
 *       b -                Second node image.
 *
 *  Returns:
 *      Zero(0) if the nodes are equal, one(1) upon (a > b) otherwise
 *      negative-one(-1) when (a < b).
 */
static int
cachenode_compare(struct vfs_node *a, struct vfs_node *b)
{
    assert(VNODE_MAGIC == a->v_magic);
    assert(VNODE_MAGIC == b->v_magic);

    if (a->v_cachehash == b->v_cachehash) {
        if (a->v_cachelen == b->v_cachelen) {
            int rc;

            if (0 == (rc = file_cmp(a->v_cachename, b->v_cachename))) {
                return 0;

            } else if (rc > 0) {
                return 1;
            }

        } else if (a->v_cachelen > b->v_cachelen) {
            return (1);
        }

    } else if (a->v_cachehash > b->v_cachehash) {
        return (1);
    }
    return (-1);
}
/*end*/
