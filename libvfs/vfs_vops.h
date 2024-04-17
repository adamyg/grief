#ifndef GR_VFS_VOPS_H_INCLUDED
#define GR_VFS_VOPS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_vops_h,"$Id: vfs_vops.h,v 1.14 2024/04/17 15:43:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_vops.h,v 1.14 2024/04/17 15:43:01 cvsuser Exp $
 * Virtual File System Interface -- ops.
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

#include <edsym.h>

__CBEGIN_DECLS

enum {
    VFS_VOPINIT,
    VFS_VOPSHUTDOWN,

    VFS_VOPMOUNT,
    VFS_VOPUNMOUNT,
    VFS_VOPLOOKUP,
    VFS_VOPDELETE,

    VFS_VOPOPEN,
    VFS_VOPCLOSE,
    VFS_VOPREAD,
    VFS_VOPWRITE,
    VFS_VOPSEEK,
    VFS_VOPTELL,
    VFS_VOPIOCTL,

    VFS_VOPOPENDIR,
    VFS_VOPREADDIR,
    VFS_VOPCLOSEDIR,

    VFS_VOPACCESS,
    VFS_VOPSTAT,
    VFS_VOPLSTAT,
    VFS_VOPFSTAT,

    VFS_VOPCHMOD,
    VFS_VOPCHOWN,

    VFS_VOPMKDIR,
    VFS_VOPRMDIR,
    VFS_VOPCHDIR,
    VFS_VOPCWD,

    VFS_VOPREADLINK,
    VFS_VOPSYMLINK,
    VFS_VOPLINK,
    VFS_VOPUNLINK,
    VFS_VOPRENAME,
    VFS_VOPREMOVE,
    VFS_VOPMKNOD,

    VFS_VOPFILENO,
    VFS_VOPERRNO,

    VFS_VOPMAX
};

struct vfs_class;
struct vfs_mount;
struct vfs_lookup;
struct vfs_handle;
struct vfs_node;
struct vfs_dir;
struct vfs_dirent;

struct vfs_implementation {
    int                     (* i_initialise)(struct vfs_class *vfs);
    void                    (* i_shutdown)(struct vfs_class *vfs);

    int                     (* i_mount)(struct vfs_mount *mount, const char *arguments);
    int                     (* i_unmount)(struct vfs_mount *mount);
    int                     (* i_lookup)(struct vfs_lookup *lk);
    int                     (* i_delete)(struct vfs_mount *vmount, struct vfs_node *vnode);

    int                     (* i_open)(struct vfs_mount *vfs, const char *fname, int flags, int mode);
    int                     (* i_close)(struct vfs_handle *handle);
    int                     (* i_read)(struct vfs_handle *handle, void *buffer, unsigned count);
    int                     (* i_write)(struct vfs_handle *handle, const void *buffer, unsigned count);
    int                     (* i_seek)(struct vfs_handle *handle, off_t offset, int whence);
    off_t                   (* i_tell)(struct vfs_handle *handle);
    int                     (* i_ioctl)(struct vfs_handle *handle, int op, void *data);

    struct vfs_dir *        (* i_opendir)(struct vfs_mount *vfs, const char *path);
    struct vfs_dirent *     (* i_readdir)(struct vfs_handle *handle);
    int                     (* i_closedir)(struct vfs_handle *handle);

    int                     (* i_access)(struct vfs_mount *vfs, const char *path, int what);
    int                     (* i_stat)(struct vfs_mount *vfs, const char *path, struct stat * buf);
    int                     (* i_lstat)(struct vfs_mount *vfs, const char *path, struct stat * buf);
    int                     (* i_fstat)(struct vfs_handle *handle, struct stat * buf);

    int                     (* i_chmod)(struct vfs_mount *vfs, const char *path, int mode);
    int                     (* i_chown)(struct vfs_mount *vfs, const char *path, int owner, int group);

    int                     (* i_mkdir)(struct vfs_mount *vfs, const char *path, mode_t mode);
    int                     (* i_rmdir)(struct vfs_mount *vfs, const char *path);
    int                     (* i_chdir)(struct vfs_mount *vfs, const char *path);
    int                     (* i_cwd)(struct vfs_mount *vfs, char *cwd, unsigned length);

    int                     (* i_readlink)(struct vfs_mount *vfs, const char *path, char *buf, size_t size);
    int                     (* i_symlink)(struct vfs_mount *vfs, const char *n1, const char *n2);
    int                     (* i_link)(struct vfs_mount *vfs, const char *p1, const char *p2);
    int                     (* i_unlink)(struct vfs_mount *vfs, const char *path);
    int                     (* i_remove)(struct vfs_mount *vfs, const char *path);
    int                     (* i_rename)(struct vfs_mount *vfs, const char *p1, const char *p2);
    int                     (* i_mknod)(struct vfs_mount *vfs, const char *path, int mode, int dev);

    int                     (* i_fileno)(struct vfs_handle *handle);
    int                     (* i_errno)(struct vfs_mount *vfs);
};

extern int                  vfs_vop_mount(struct vfs_mount *vmount, const char *argument);
extern int                  vfs_vop_unmount(struct vfs_mount *vmount);

extern int                  vfs_vop_lookup(struct vfs_lookup *lk);

extern int                  vfs_vop_readlink(struct vfs_node *node, char *linkpath, int *linklen);
extern int                  vfs_vop_delete(struct vfs_node *node);

__CEND_DECLS

#endif /*GR_VFS_VOPS_H_INCLUDED*/
