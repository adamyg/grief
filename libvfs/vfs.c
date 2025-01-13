#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_c,"$Id: vfs.c,v 1.18 2025/01/13 15:25:26 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs.c,v 1.18 2025/01/13 15:25:26 cvsuser Exp $
 * Virtual file system interface.
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

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"
#include "vfs_handle.h"

#define VFS_FPATH(ident, name, rtype, rerror, prototype, function, args) \
    rtype \
    vfs_##name prototype \
    { \
        char t_path[MAX_PATH], *fname; \
        struct vfs_mount *vmount; \
        const struct vfs_class *vclass; \
        \
        VFS_TRACE(("vfs_%s(%s)\n", #name, path)) \
        assert(ident >= 0 && ident < VFS_VOPMAX); \
        vmount = vfs_mount_lookup(path, t_path, sizeof(t_path), &fname); \
        assert(vmount); \
        ++vmount->mt_stats[ident]; \
        if (NULL != (vclass = vmount->mt_class)) { \
            if (vclass->v_impl.i_##function) { \
                path = t_path; \
                return (*vclass->v_impl.i_##function) args; \
            } \
        } \
        errno = ENOTSUPPORTED; \
        return rerror; \
    }

#define VFS_FHANDLE(ident, name, rtype, rerror, prototype, function, args) \
    rtype \
    vfs_##name prototype \
    { \
        struct vfs_handle *vhandle = vfs_handle_get(handle); \
        \
        VFS_TRACE(("vfs_%s(%d)\n", #name, handle)) \
        assert(ident >= 0 && ident < VFS_VOPMAX); \
        assert(vhandle); \
        if (vhandle) { \
            struct vfs_mount *vmount = vhandle->h_mount; \
            const struct vfs_class *vclass; \
            \
            assert(vmount); \
            ++vmount->mt_stats[ident]; \
            if (NULL != (vclass = vmount->mt_class)) { \
                if (vclass->v_impl.i_##function) { \
                    return (*vclass->v_impl.i_##function) args; \
                } \
            } \
            errno = ENOTSUPPORTED; \
        } else { \
            errno = ENOENT; \
        } \
        return rerror; \
    }

#define VFS_FDIR(ident, name, rtype, rerror, prototype, function, args) \
    static VFS_FHANDLE(ident, impl_##name, rtype, rerror, (int handle), function, args) \
    \
    rtype \
    vfs_##name prototype \
    { \
        VFS_TRACE(("vfs_%s(%p)\n", #name, dir)) \
        assert(ident >= 0 && ident < VFS_VOPMAX); \
        assert(NULL != dir); \
        if (dir) { \
            assert(VDIR_MAGIC == dir->d_magic); \
            return vfs_impl_##function(dir->d_handle); \
        } \
        return rerror; \
    }


int
vfs_mount(const char *mountpoint, unsigned flags, const char *vfsname, const char *argument)
{
    struct vfs_mount *vmount;
    struct vfs_class *vclass;
    unsigned vfslength = 0;

    if (NULL == vfsname || 0 == vfsname[0]) {
        errno = EINVAL;                         /* invalid arguments */
        return -1;
    }

    /*
     *  allow a suffix within the vfsname
     *
     *      [#|//]suffix:
     *
     *  examples
     *      #ftp:
     *      //ftp:
     */
    if (NULL == argument && NULL != strchr(vfsname, ':')) {
        const char *t_name = vfsname;
        unsigned t_length = 0;
        unsigned char ch;

        if ('#' == t_name[0]) {
            ++t_name;                           /* leading '#' remove */
        } else if ('/' == t_name[0] && '/' == t_name[1]) {
            t_name += 2;                        /* leading '//' remove */
        }

        while ((ch = (unsigned char)t_name[t_length]) != 0) {
            if (! isalnum(ch)) {
                break;
            }
            ++t_length;
        }

        if (t_length >= 3 && ':' == ch) {       /* xxx[..]: */
            /*
             *  syntax valid
             *      use suffix as vfsname and pass uncooked as argument since
             *      suffix may imply a transport, for example 'sftp'.
             */
            vfsname = t_name;
            vfslength = t_length;
            argument = vfsname;
        }
    }

    if (NULL == (vclass = vfs_class_get(vfsname, vfslength))) {
        errno = ENODEVICE;                      /* not such device */
        return -1;
    }

    if (NULL == (vmount = vfs_mount_new(vfs_root(), mountpoint, vclass, flags, argument))) {
        return -1;
    }

    return (0);
}


int
vfs_unmount(const char *mountpoint, unsigned flags)
{
    struct vfs_mount *vmount;
    const struct vfs_class *vclass;

    __UNUSED(flags);

    if (NULL == mountpoint || 0 == mountpoint[0]) {
        errno = EINVAL;                         /* invalid arguments */
        return -1;
    }

    if (NULL == (vmount = vfs_mount_get(mountpoint, 0))) {
        errno = ENODEVICE;                      /* unknown mount point */
        return -1;
    }

    if (vmount->mt_references) {
        errno = EBUSY;                          /* files stil open */
        return -1;
    }

    if (0 == vmount->mt_mntlen) {
        errno = EACCES;                         /* root system-root */
        return -1;
    }

    vclass = vmount->mt_class;
    assert(vmount->mt_class);
    if (vclass->v_impl.i_unmount) {
        errno = 0;
        if (0 != (*vclass->v_impl.i_unmount)(vmount)) {
            if (0 == errno) {
                errno = EIO;                    /* EBUSY maybe better */
            }
            return -1;
        }
    }
    vfs_mount_delete(vmount);
    return 0;
}

VFS_FPATH(VFS_VOPOPEN, open, int, -1, (const char *path, int mode, int mask), open, (vmount, fname, mode, mask))

VFS_FHANDLE(VFS_VOPCLOSE, close, int, -1, (int handle), close, (vhandle))

VFS_FHANDLE(VFS_VOPREAD, read, int, -1, (int handle, void *buffer, unsigned length), read, (vhandle, buffer, length))

VFS_FHANDLE(VFS_VOPWRITE, write, int, -1, (int handle, const void *buffer, unsigned length), write, (vhandle, buffer, length))

VFS_FHANDLE(VFS_VOPCLOSE, seek, int, -1, (int handle, off_t offset, int whence), seek, (vhandle, offset, whence))

VFS_FHANDLE(VFS_VOPTELL, tell, off_t, -1, (int handle), tell, (vhandle))

VFS_FHANDLE(VFS_VOPIOCTL, ioctl, int, -1, (int handle, int op, void *data), ioctl, (vhandle, op, data))

VFS_FHANDLE(VFS_VOPFILENO, fileno, int, -1, (int handle), fileno, (vhandle))

VFS_FHANDLE(VFS_VOPFSTAT, fstat, int, -1, (int handle, struct stat *sb), fstat, (vhandle, sb))

VFS_FPATH(VFS_VOPACCESS, access, int, -1, (const char *path, int what), access, (vmount, fname, what))

VFS_FPATH(VFS_VOPSTAT, stat, int, -1, (const char *path, struct stat *sb), stat, (vmount, fname, sb))

VFS_FPATH(VFS_VOPLSTAT, lstat, int, -1, (const char *path, struct stat *sb), lstat, (vmount, fname, sb))

VFS_FPATH(VFS_VOPCHMOD, chmod, int, -1, (const char *path, int mode), chmod, (vmount, fname, mode))

VFS_FPATH(VFS_VOPCHOWN, chown, int, -1, (const char *path, int owner, int group), chown, (vmount, fname, owner, group))

VFS_FPATH(VFS_VOPOPENDIR, opendir, vfs_dir_t *, NULL, (const char *path), opendir, (vmount, fname))

VFS_FDIR(VFS_VOPREADDIR, readdir, vfs_dirent_t *, NULL, (vfs_dir_t *dir), readdir, (vhandle))

int
vfs_statdir(vfs_dirent_t *dent, struct stat *sb)
{
    if (NULL == dent) {
        errno = EINVAL;
        return -1;                              /* invalid parameter */
    }

    assert(VDIRENT_MAGIC == dent->d_magic);
    if (VDIRECTST_MAGIC != dent->d_stat) {
        return -2;                              /* not supported */
    }

    if (NULL == sb) {
        errno = EINVAL;
        return -1;                              /* invalid parameter */
    }

    memset(sb, 0, sizeof(struct stat));
    sb->st_ctime = dent->d_ctime;
    sb->st_mtime = dent->d_mtime;
    sb->st_atime = dent->d_atime;
    sb->st_mode = (mode_t) dent->d_mode;
    sb->st_size = dent->d_size;
    sb->st_uid = dent->d_uid;
    sb->st_gid = dent->d_gid;
    return 0;
}

VFS_FDIR(VFS_VOPCLOSEDIR, closedir, int, -1, (vfs_dir_t *dir), closedir, (vhandle))

VFS_FPATH(VFS_VOPMKDIR, mkdir, int, -1, (const char *path, int mode), mkdir, (vmount, fname, mode))

VFS_FPATH(VFS_VOPRMDIR, rmdir, int, -1, (const char *path), rmdir, (vmount, fname))

VFS_FPATH(VFS_VOPCHDIR, chdir, int, -1, (const char *path), chdir, (vmount, fname))

char *
vfs_cwd(char *buffer, unsigned length)
{
    const char *cwd;

    if (NULL == buffer) {
        vfscwd_set(NULL);                       /* reset */
    }
    cwd = vfscwd_get();                         /* retrieve */
    if (NULL != buffer && length) {
        strxcpy(buffer, cwd, length);
    }
    VFS_TRACE(("vfs_cwd() : %s\n", buffer))
    return buffer;
}

VFS_FPATH(VFS_VOPREADLINK, readlink, int, -1, (const char *path, char *buf, size_t size), readlink, (vmount, fname, buf, size))

VFS_FPATH(VFS_VOPSYMLINK, symlink, int, -1, (const char *path, const char *path2), symlink, (vmount, path, path2))

VFS_FPATH(VFS_VOPLINK, link, int, -1, (const char *path, const char *path2), link, (vmount, path, path2))

VFS_FPATH(VFS_VOPUNLINK, unlink, int, -1, (const char *path), unlink, (vmount, fname))

VFS_FPATH(VFS_VOPRENAME, rename, int, -1, (const char *path, const char *newpath), rename, (vmount, path, newpath))

VFS_FPATH(VFS_VOPREMOVE, remove, int, -1, (const char *path), remove, (vmount, path))

VFS_FPATH(VFS_VOPMKNOD, mknod, int, -1, (const char *path, int mode, int dev), mknod, (vmount, path, mode, dev))

/*end*/
