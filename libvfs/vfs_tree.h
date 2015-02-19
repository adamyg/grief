#ifndef GR_VFS_TREE_H_INCLUDED
#define GR_VFS_TREE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_tree_h,"$Id: vfs_tree.h,v 1.7 2015/02/19 00:17:23 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_tree.h,v 1.7 2015/02/19 00:17:23 ayoung Exp $
 * Virtual File System Interface -- node tree definitions.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
#include "vfs_cache.h"
#include <edsym.h>

__CBEGIN_DECLS

/*
 *  forward references
 */
struct vfs_mount;
struct vfs_class;
struct vfs_tree;
struct vfs_lookup;
struct vfs_node;
struct vfs_handle;
struct stat;

/*
 *  Implemenationm of localised tree operations
 */
enum vfs_treevop {
    VTREE_VOP_ACCESS,
    VTREE_VOP_CREATE,
    VTREE_VOP_OPEN,
    VTREE_VOP_READ,
    VTREE_VOP_WRITE,
    VTREE_VOP_TELL,
    VTREE_VOP_CLOSE,
    VTREE_VOP_LOCALGET,
    VTREE_VOP_LOCALPUT
};

struct vfs_treevops {
    int     (* vop_lookup)(struct vfs_lookup *lk);
    int     (* vop_access)(struct vfs_tree *tree, struct vfs_node *node, int what);
    int     (* vop_create)(struct vfs_tree *tree, struct vfs_node *parent, const char *name, int mask, struct vfs_node **node, int *fd);
    int     (* vop_open)(struct vfs_tree *tree, struct vfs_node *node, int mode, int mask, int *fd);
    int     (* vop_read)(struct vfs_tree *tree, struct vfs_handle *handle, char *buffer, size_t length);
    int     (* vop_write)(struct vfs_tree *tree, struct vfs_handle *handle, const char *buffer, size_t length);
    int     (* vop_fstat)(struct vfs_tree *tree, struct vfs_handle *handle, struct stat *sb);
    off_t   (* vop_tell)(struct vfs_tree *tree, struct vfs_handle *handle);
    int     (* vop_seek)(struct vfs_tree *tree, struct vfs_handle *handle, off_t offset, int whence);
    int     (* vop_ioctl)(struct vfs_tree *tree, struct vfs_handle *handle, int op, void *data);
    int     (* vop_close)(struct vfs_tree *tree, struct vfs_handle *handle);
    
    int     (* vop_localget)(struct vfs_tree *tree, struct vfs_node *node, int mode, int mask, int *fd);
    int     (* vop_localput)(struct vfs_tree *tree, struct vfs_node *node, int changed);
};

typedef void (* vfs_treevfunc_t)();             /* generic callback interface */


/*
 *  Node/directory tree .. for virtual filesystem one of
 *  these shall be owned by the mount point.
 */
typedef struct vfs_tree {
    unsigned            t_magic;                /* structure magic */
    struct vfs_mount *  t_mount;                /* owner/mount point */
    struct vfs_treevops t_vops;                 /* tree interface operations */
    TAILQ_ENTRY(vfs_tree) 
                        t_nodelist;             /* list node */
    struct vfs_node *   t_root;                 /* root node */
    struct vfs_cache    t_cache;                /* node cache */
    unsigned            t_nodecount;            /* total assigned nodes */
} vfs_tree_t;

extern void                 vfs_tree_start(void);
extern void                 vfs_tree_stop(void);

extern vfs_tree_t *         vfs_tree_new(struct vfs_mount *mount, struct vfs_treevops *vops);
extern void                 vfs_tree_init(vfs_tree_t *tree, struct vfs_mount *mount, struct vfs_treevops *vops);

extern struct vfs_class *   vfs_tree_vops(struct vfs_class *vclass);
extern int                  vfs_tree_vop(struct vfs_tree *tree, enum vfs_treevop op, vfs_treevfunc_t func);

extern struct vfs_node *    vfs_tree_add(vfs_tree_t *tree, struct vfs_node *parent, unsigned type, const char *name, unsigned namlen);
extern char *               vfs_tree_path(struct vfs_tree *tree, struct vfs_node *node, unsigned module);
extern struct vfs_node *    vfs_tree_push(vfs_tree_t *tree, unsigned type, const char *path);
extern void                 vfs_tree_destroy(vfs_tree_t *node);

extern struct vfs_node *    vfs_tree_lookup(struct vfs_tree *tree, const char *path, unsigned pathlen, unsigned op, unsigned flags);
extern struct vfs_node *    vfs_tree_lookup2(struct vfs_tree *tree, const char *path, unsigned pathlen, unsigned op, unsigned flags, int *error);
extern struct vfs_node *    vfs_tree_lookup3(struct vfs_tree *tree, const char *path, unsigned pathlen, unsigned op, unsigned flags, struct vfs_lookup *lk);

extern struct vfs_node *    vfs_tree_search(vfs_tree_t *tree, struct vfs_node *parent, const char *name, unsigned namlen, unsigned namhash);

__CEND_DECLS

#endif /*GR_VFS_TREE_H_INCLUDED*/
