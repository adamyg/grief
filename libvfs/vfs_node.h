#ifndef GR_VFS_NODE_H_INCLUDED
#define GR_VFS_NODE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_node_h,"$Id: vfs_node.h,v 1.13 2025/01/13 15:25:26 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_node.h,v 1.13 2025/01/13 15:25:26 cvsuser Exp $
 * Virtual File System Interface -- node definitions.
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

#include <tailqueue.h>
#include <rbtree.h>
#include <edsym.h>

__CBEGIN_DECLS

struct vfs_cache;
struct vfs_mount;

/*
 *  The vnode is the focus of all file activity .  There is a unique vnode
 *  allocated for each file, directory and the root.
 */
typedef struct vfs_node {
    unsigned            v_magic;                /* structure magic */
    unsigned            v_type;                 /* vnode type */
#define VNODE_UNKNOWN           0
#define VNODE_FIFO              1
#define VNODE_DIR               2
#define VNODE_REG               3
#define VNODE_LNK               4
#define VNODE_BAD               5

    struct vfs_mount *  v_mount;                /* mounted file-system */
    void *              v_owner;                /* owners reference */
    struct vfs_node *   v_parent;               /* parent, unless root node */
    TAILQ_HEAD(nodelist_t, vfs_node)
                        v_childlist;            /* children (if any) */
    unsigned            v_childcount;
    unsigned            v_childedit;            /* edit count (inc on each ins/del) */
    unsigned            v_childseq;             /* next child fileno */
    TAILQ_ENTRY(vfs_node)
                        v_listnode;             /* list node */
    struct vfs_cache *  v_cache;                /* cache (if any) */
    unsigned            v_cachelen;             /* length of the name buffer */
    unsigned            v_cachehash;            /* name hash */
    const char *        v_cachename;            /* cache entry name */
    RB_ENTRY(vfs_node)  v_cachetree;            /* name cache node */
    TAILQ_ENTRY(vfs_node)
                        v_cachelru;             /* cache lru node */
    unsigned            v_flags;                /* operational flags */
#define VNODE_FCACHED           0x0001
#define VNODE_FDIRTY            0x0002

    unsigned            v_namlen;               /* length of the name buffer */
    unsigned            v_namhash;              /* hash on name */
    const char *        v_name;                 /* entry name (buffer trails vfs_node buffer) */
    const char *        v_localname;            /* local backing image (if any) */
    unsigned            v_fileno;               /* file number within directory */
    unsigned            v_references;           /* reference count */
    struct stat         v_st;		               /* status block */
} vfs_node_t;

extern struct vfs_node *    vfs_node_new(unsigned type, const char *name, unsigned namlen, unsigned namhash, struct vfs_node *parent);

extern const char *         vfs_node_type2str(unsigned type);
extern unsigned             vfs_node_type2mode(unsigned type);

extern int                  vfs_node_unlink(struct vfs_node *node);
extern void                 vfs_node_destroy(struct vfs_node *node);
extern struct vfs_node *    vfs_node_first(struct vfs_node *node);
extern struct vfs_node *    vfs_node_next(struct vfs_node *node);

extern int                  vfs_node_namecmp(struct vfs_node *node, const char *b, unsigned bnamlen);

__CEND_DECLS

#endif /*GR_VFS_NODE_H_INCLUDED*/
