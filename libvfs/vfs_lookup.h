#ifndef GR_VFS_LOOKUP_H_INCLUDED
#define GR_VFS_LOOKUP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_lookup_h,"$Id: vfs_lookup.h,v 1.10 2019/03/15 23:23:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_lookup.h,v 1.10 2019/03/15 23:23:01 cvsuser Exp $
 * Virtual File System Interface -- lookup service.
 *
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

#include <edsym.h>

__CBEGIN_DECLS

struct vfs_mount;
struct vfs_tree;
struct vfs_node;

typedef struct vfs_lookup {
    unsigned            l_op;                   /* operation */
#define LK_OLOOKUP          1                   /* filename lookup */
#define LK_OCREATE          2
#define LK_ORENAME          3
#define LK_ODELETE          4
#define LK_OMKTREE          5

    unsigned            l_flags;                /* search flags */
#define LK_FFOLLOW          0x0001              /* follow links */
#define LK_FNOCROSSMOUNT    0x0002

#define LK_FISDOTDOT        0x0100
#define LK_FISLAST          0x0200
#define LK_FISSYMLINK       0x0400

    struct vfs_mount *  l_mount;                /* mount point being searched */
    struct vfs_node *   l_root;                 /* root of tree being searched */
    struct vfs_node *   l_cwd;                  /* current working directory */
    struct vfs_node *   l_parent;               /* parent/directory node */
    struct vfs_node *   l_node;                 /* current node within search */
    unsigned            l_loopcnt;              /* symlink lookup count (catch loops) */
    unsigned            l_errno;                /* error code */
    const char *        l_cursor;               /* cursor within path */
    unsigned            l_pathlen;
    const char *        l_name;                 /* current name (within path) */
    unsigned            l_namlen;
    unsigned            l_namhash;
    char                l_buffer[ VFS_MAXPATH+1 ];
} vfs_lookup_t;

extern struct vfs_node *    vfs_lookup(struct vfs_mount *tree, const char *path, unsigned pathlen, 
                                    unsigned op, unsigned flags, struct vfs_lookup *lk);

extern struct vfs_node *    vfs_resolve(vfs_lookup_t *lk);

__CEND_DECLS

#endif /*GR_VFS_LOOKUP_H_INCLUDED*/
