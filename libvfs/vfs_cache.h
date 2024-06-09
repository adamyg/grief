#ifndef GR_VFS_CACHE_H_INCLUDED
#define GR_VFS_CACHE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_cache_h,"$Id: vfs_cache.h,v 1.12 2024/04/17 15:43:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_cache.h,v 1.12 2024/04/17 15:43:00 cvsuser Exp $
 * Virtual File System Interface -- name cache.
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

#include <rbtree.h>
#include <edsym.h>

__CBEGIN_DECLS

struct vfs_node;

typedef RB_HEAD(cacherb, vfs_node) cacherb_t;

struct vfs_cache {
    unsigned            c_magic;
    unsigned            c_flags;
    cacherb_t           c_tree;             /* name cache tree */
    TAILQ_HEAD(cachelru_t, vfs_node)
                        c_lru;              /* name cache list, in lru order */
    unsigned            c_elem;             /* name cache element count */
    unsigned            c_size;             /* name cache size, in bytes */
};

extern void                 vfs_cache_init(struct vfs_cache *cache, unsigned flags);
extern void                 vfs_cache_destroy(struct vfs_cache *cache);

extern struct vfs_node *    vfs_cache_lookup(struct vfs_cache *cache, const char *path, unsigned patlen);
extern int                  vfs_cache_link(struct vfs_cache *cache, struct vfs_node *node, char *abspath);
extern int                  vfs_cache_unlink(struct vfs_node *node);

__CEND_DECLS

#endif /*GR_VFS_CACHE_H_INCLUDED*/
