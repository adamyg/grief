#ifndef GR_VFS_MOUNT_H_INCLUDED
#define GR_VFS_MOUNT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_mount_h,"$Id: vfs_mount.h,v 1.9 2018/10/01 22:16:28 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_mount.h,v 1.9 2018/10/01 22:16:28 cvsuser Exp $
 * Virtual File System Interface -- internal definitions.
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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
#include <tailqueue.h>
#include "vfs_vops.h"

__CBEGIN_DECLS

struct vfs_class;
struct vfs_tree;
struct vfs_node;
struct vfs_handle;

struct vfs_mount {
    unsigned            mt_magic;               /* structure magic */
    struct vfs_mount *  mt_parent;              /* our parent */
    TAILQ_ENTRY(vfs_mount)                      /* global queue node */
                        mt_node;
    unsigned            mt_mntlen;              /* length of mount path, in bytes */
    unsigned            mt_mnthash;             /* name hash */
    unsigned            mt_baselen;             /* length of basename component, in bytes (minus trailing sep) */
    unsigned            mt_basehash;            /* base name hash */
    unsigned            mt_namlen;              /* length of filename component, in bytes (minus trailing sep) */
    unsigned            mt_namhash;             /* filename hash */
    const char *        mt_mount;               /* mount point */
    const char *        mt_name;                /* filename component within mount point */
    unsigned            mt_flags;               /* flags */
#define VMOUNT_FUSERMASK    0x00ff              /* User level flag mask */
#define VMOUNT_FCASEIGNORE  0x0001              /* ignore case */
#define VMOUNT_FCASECONVERT 0x0002              /* convert case */
#define VMOUNT_FRDONLY      0x0010              /* read-only file system */
#define VMOUNT_FVIRTUAL     0x0020              /* mount-point is virtual */

#define VMOUNT_FNAMECACHE   0x0100              /* Name cache enabled */

#define VMOUNT_FROOT        0x1000              /* root file system */
#define VMOUNT_FFULLPATH    0x2000              /* driver requires full path */
#define VMOUNT_FSHUTDOWN    0x4000              /* mount-point shutting down */

    unsigned            mt_references;          /* reference count */
    unsigned            mt_fdcount;             /* number of open files */
    TAILQ_HEAD(fdhandleq_t, vfs_handle)
                        mt_fdhandleq;           /* open handles */
    struct vfs_class *  mt_class;               /* assigned class */
    void *              mt_data;                /* implementation specific (for example ftp login spec) */
    struct vfs_node *   mt_root;                /* root vnode (if any) */
//  struct vfs_node *   mt_cwd;                 /* current working directory (if any) */
    struct vfs_tree *   mt_tree;                /* vnode management */
    int                 mt_errno;               /* last error condition */
    unsigned            mt_stats[VFS_VOPMAX];   /* basic usage stats */
};

extern void                 vfs_mount_start(void);
extern void                 vfs_mount_shutdown(void);

extern struct vfs_mount *   vfs_mount_first(void);
extern struct vfs_mount *   vfs_mount_next(struct vfs_mount *vmount);

extern struct vfs_mount *   vfs_mount_new(struct vfs_mount *parent, const char *mountpoint, struct vfs_class *vfs, unsigned flags, const char *argument);
extern void                 vfs_mount_delete(struct vfs_mount *vmount);
extern void                 vfs_mount_flagset(struct vfs_mount *vmount, unsigned flag);

extern struct vfs_mount *   vfs_mount_get(const char *mountpoint, unsigned mntlen);
extern struct vfs_mount *   vfs_mount_lookup(const char *path, char *buffer, int length, char **fname);
extern unsigned             vfs_mount_list(const char *path, unsigned pathlen, unsigned flags, struct vfs_mount *mounts[], unsigned mntmax);
extern unsigned             vfs_mount_seq(void);

__CEND_DECLS

#endif /*GR_VFS_MOUNT_H_INCLUDED*/
