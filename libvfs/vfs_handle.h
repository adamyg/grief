#ifndef GR_VFS_HANDLE_H_INCLUDED
#define GR_VFS_HANDLE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_handle_h,"$Id: vfs_handle.h,v 1.11 2019/03/15 23:23:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_handle.h,v 1.11 2019/03/15 23:23:01 cvsuser Exp $
 * Virtual File System Interface -- internal definitions.
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

#include <rbtree.h>
#include <tailqueue.h>
#include <edsym.h>

__CBEGIN_DECLS

struct vfs_mount;
struct vfs_node;

struct vfs_handle {
    unsigned                h_magic;            /* structure magic */
    RB_ENTRY(vfs_handle)    h_rbnode;           /* rbtree node */
    struct vfs_mount *      h_mount;            /* mount point */
    TAILQ_ENTRY(vfs_handle) h_mtnode;           /* mount table file-descriptor table */
    int                     h_handle;           /* handle identifier */
    int                     h_ihandle;          /* working integer handle, implementation specific */
    void *                  h_phandle;          /* implementation specific */
    struct vfs_node *       h_vnode;            /* node reference (if any) */
    int                     h_errcode;          /* last errnno status (if any) */
};

extern void                 vfs_handle_start(void);
extern void                 vfs_handle_shutdown(void);
extern struct vfs_handle *  vfs_handle_new(struct vfs_mount *vfs, unsigned additional);
extern void                 vfs_handle_delete(struct vfs_handle *vhandle);
extern struct vfs_handle *  vfs_handle_get(int handle);

__CEND_DECLS

#endif /*GR_VFS_HANDLE_H_INCLUDED*/
