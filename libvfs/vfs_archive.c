#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_archive_c,"$Id: vfs_archive.c,v 1.24 2024/04/17 16:00:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_archive.c,v 1.24 2024/04/17 16:00:29 cvsuser Exp $
 * Virtual file system interface - libarchive driver.
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

#include <editor.h>
#include <errno.h>
#include <sys/stat.h>

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"
#include "vfs_node.h"
#include "vfs_handle.h"
#include "vfs_tree.h"

#if defined(HAVE_ARCHIVE_H) && defined(HAVE_LIBARCHIVE)

#if defined(GNUWIN32_LIBARCHIVE)
#include <gnuwin32_archive.h>
#include <gnuwin32_archive_entry.h>
#else
#include <archive.h>
#include <archive_entry.h>
#endif

static int              vfsarc_mount(struct vfs_mount *vmount, const char *arguments);
static int              vfsarc_unmount(struct vfs_mount *vmount);

static int              vfsarc_tree_vlocalget(struct vfs_tree *tree, struct vfs_node *node, int mode, int mask, int *fd);

/*FIXME*/
extern int              file_cmp(const char *f1, const char *f2);

struct vfs_class *
vfsarc_init(void)
{
    struct vfs_class *vclass;

    vclass = vfs_class_new("library archive", "tar", NULL);
    vfs_tree_vops(vclass);
    vclass->v_impl.i_mount = vfsarc_mount;
    vclass->v_impl.i_unmount = vfsarc_unmount;
    return vclass;
}


static struct archive *
read_new(void)
{
    struct archive *a;

    if (NULL != (a = archive_read_new())) {
        /*
         *  others?
         */
    //  archive_read_support_compression_all(a);
        archive_read_support_format_empty(a);
        archive_read_support_format_ar(a);
    //  archive_read_support_format_cpio(a);
        archive_read_support_format_tar(a);
        archive_read_support_format_gnutar(a);
    //  archive_read_support_format_iso9660(a);
    //  archive_read_support_format_mtree(a);
    //  archive_read_support_format_zip(a);
    }
    return a;
}


static int
vfsarc_mount(struct vfs_mount *vmount, const char *argument)
{
    struct vfs_tree *tree;
    struct archive *a;
    struct archive_entry *entry;
    char path[VFS_MAXPATH+1];
//  const char *err = NULL;
    int rcode = -1;

    VFS_TRACE(("vfsarc_mount(%s, %s)\n", vmount->mt_mount, (argument ? argument : "")))

    if (NULL == (tree = vfs_tree_new(vmount, NULL))) {
        return -1;
    }

    vfs_tree_vop(tree, VTREE_VOP_LOCALGET, (vfs_treevfunc_t)vfsarc_tree_vlocalget);

    strncpy(path, vmount->mt_mount, sizeof(path));
    if (argument && argument[0]) {
        strxcat(path, argument, sizeof(path));
    } else if (vmount->mt_mntlen > 1) {
        path[vmount->mt_mntlen-1] = 0;
    }

    VFS_TRACE(("vfsarc_mount(%s)\n", path))

#ifndef ARCHIVE_DEFAULT_BYTES_PER_BLOCK
#define ARCHIVE_DEFAULT_BYTES_PER_BLOCK 10240
#endif

    if (NULL != (a = read_new())) {
        if (0 == (rcode = archive_read_open_filename(a, path, ARCHIVE_DEFAULT_BYTES_PER_BLOCK))) {
            vmount->mt_data = chk_salloc(path);
            for (;;) {
                unsigned vmode;

                if (ARCHIVE_EOF == (rcode = archive_read_next_header(a, &entry))) {
                    rcode = 0;
                    break;
                }

                if (ARCHIVE_OK != rcode) {
//                  err = archive_error_string(a);
                    rcode = -1;
                    break;
                }

                switch (archive_entry_filetype(entry)) {
                case AE_IFDIR:
                    vmode = VNODE_DIR;
                    break;
                case AE_IFREG:
                    vmode = VNODE_REG;
                    break;
                default:
                    vmode = VNODE_UNKNOWN;
                    break;
                }

                if (VNODE_UNKNOWN != vmode) {
                    const char *pathname = archive_entry_pathname(entry);
                    const struct stat *sb = archive_entry_stat(entry);
                    struct vfs_node *vnode;

                    VFS_TRACE(("push=%s\n", pathname))
                    vnode = vfs_tree_push(tree, vmode, pathname);
                    assert(vnode);
                    if (NULL == vnode) {
                        rcode = -1;
                        break;
                    }
                    memcpy(&vnode->v_st, sb, sizeof(struct stat));
                }
            }
            archive_read_close(a);
            archive_read_free(a);
        }
    }

    if (-1 == rcode) {
        vfs_tree_destroy(vmount->mt_tree);
        vmount->mt_tree = NULL;

    } else {
        /*
         *  configure
         *      RDONLY, implied with current implementation
         *      NAMECACHE. not required yet a good test of the infrustructure.
         */
        vfs_mount_flagset(vmount, VMOUNT_FRDONLY|VMOUNT_FNAMECACHE);
    }

    VFS_TRACE(("\tvfsarc_mount(%s) ; %d\n", path, rcode))
    return (rcode ? -1 : 0);
}


static int
vfsarc_unmount(struct vfs_mount *vmount)
{
    VFS_TRACE(("\tvfsarc_unmount(%s)\n", vmount->mt_mount))
    vfs_tree_destroy(vmount->mt_tree);
    chk_free(vmount->mt_data);
    vmount->mt_tree = NULL;
    return 0;
}


static int
vfsarc_tree_vlocalget(struct vfs_tree *tree, struct vfs_node *node, int mode, int mask, int *fd)
{
    struct vfs_mount *vmount = tree->t_mount;
    const char *path = vfs_tree_path(tree, node, FALSE);
    struct archive *a = NULL;
    int error = ENOENT;

    VFS_LEVELINC()
    VFS_TRACE(("vfsarc_vlocalget(%p, %s%s)\n", tree, vmount->mt_mount, path))

    __CUNUSED(mode)
    __CUNUSED(mask)
    __CUNUSED(fd)

    assert(vmount->mt_data);

    if (node->v_localname) {
        error = 0;                              /* local cache already exists */

    } else if (NULL == (a = read_new())) {
        error = ENOMEM;

    } else {
        if (0 == archive_read_open_filename(a, vmount->mt_data, ARCHIVE_DEFAULT_BYTES_PER_BLOCK)) {
            for (;;) {
                struct archive_entry *entry;
                int rcode;

                if (ARCHIVE_EOF == (rcode = archive_read_next_header(a, &entry))) {
                    VFS_TRACE(("== EOF\n"))
                    break;
                }

                if (ARCHIVE_OK != rcode) {
                    const char *err = archive_error_string(a);

                    VFS_TRACE(("== EIO(%s)\n", err))
                    error = EIO;
                    break;
                }

                if (AE_IFREG == archive_entry_filetype(entry)) {
                    const char *pathname = archive_entry_pathname(entry);

                    VFS_TRACE(("\tpath:%s\n", pathname))

                    if (0 == file_cmp(path, pathname)) {
                        const char *tempnam;
                        int t_fd = -1;

                        if (NULL == (tempnam = vfs_tempnam(&t_fd))) {
                            VFS_TRACE(("== temp file error\n"))
                            error = errno;
                        } else {
                            VFS_TRACE(("\textracting %s=>%s\n", pathname, tempnam))
                            rcode = archive_read_data_into_fd(a, t_fd);
                            close(t_fd);
                            if (ARCHIVE_OK == rcode) {
                                node->v_localname = tempnam;
                                error = 0;
                            } else {
                                unlink(tempnam);
                                chk_free((void *)tempnam);
                                error = EIO;
                            }
                        }
                        break;
                    }
                }
            }
            archive_read_close(a);
            archive_read_free(a);
        }
    }
    chk_free((void *)path);
    VFS_LEVELDEC()
    return error;
}

#else   /*HAVE_ARCHIVE_H && HAVE_LIBARCHIVE*/

struct vfs_class *
vfsarc_init(void)
{
    return NULL;
}

#endif

