#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_base_c,"$Id: vfs_base.c,v 1.24 2020/06/18 13:17:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_base.c,v 1.24 2020/06/18 13:17:29 cvsuser Exp $
 * Virtual file system interface - base implementation.
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

#include <editor.h>
#include <eddir.h>
#include <errno.h>

#include "vfs_class.h"
#include "vfs_handle.h"
#include "vfs_internal.h"
#include "vfs_mount.h"

typedef struct {
    vfs_dir_t           d_dir;                  /* Base (user) object */
    vfs_dirent_t        d_entry;                /* Current entry */
    void  *             d_cached;               /* Cached entry */
    unsigned            d_mntcnt;               /* Number of mounts */
    unsigned            d_mntidx;               /* Current index within mounts */
    unsigned            d_mntseq;               /* Mount list edit sequence */
    struct vfs_mount *  d_mounts[32];           /* Mount point definitions */
    unsigned            d_pathlen;              /* Path length. in bytes */
    char                d_path[VFS_MAXPATH+1];  /* DIrectory path */
} vfsbase_dir_t;

static int              vfsbase_unmount(struct vfs_mount *mount);

static int              vfsbase_open(struct vfs_mount *vmount, const char *path, int mode, int mask);
static int              vfsbase_close(struct vfs_handle *vhandle);
static int              vfsbase_read(struct vfs_handle *vhandle, void *buffer, unsigned length);
static int              vfsbase_write(struct vfs_handle *vhandle, const void *buffer, unsigned length);
static int              vfsbase_seek(struct vfs_handle *vhandle, off_t offset, int whence);
static off_t            vfsbase_tell(struct vfs_handle *vhandle);
static int              vfsbase_ioctl(struct vfs_handle *vhandle, int op, void *);

static int              vfsbase_access(struct vfs_mount *vmount, const char *path, int what);
static int              vfsbase_stat(struct vfs_mount *vmount, const char *path, struct stat *sb);
static int              vfsbase_lstat(struct vfs_mount *vmount, const char *path, struct stat *sb);
static int              vfsbase_fstat(struct vfs_handle *vhandle, struct stat *sb);
static int              vfsbase_chmod(struct vfs_mount *vmount, const char *path, int mode);
static int              vfsbase_chown(struct vfs_mount *vmount, const char *path, int owner, int group);

static vfs_dir_t*       vfsbase_opendir(struct vfs_mount *vmount, const char *path);
static vfs_dirent_t*    vfsbase_readdir(struct vfs_handle *vhandle);
static int              vfsbase_closedir(struct vfs_handle *vhandle);

static int              vfsbase_mkdir(struct vfs_mount *vmount, const char *path, mode_t mode);
static int              vfsbase_rmdir(struct vfs_mount *vmount, const char *path);
static int              vfsbase_chdir(struct vfs_mount *vmount, const char *path);

static int              vfsbase_readlink(struct vfs_mount *vmount, const char *path, char *buf, size_t size);
static int              vfsbase_symlink(struct vfs_mount *vmount, const char *n1, const char *n2);

static int              vfsbase_link(struct vfs_mount *vmount, const char *path1, const char *path2);
static int              vfsbase_unlink(struct vfs_mount *vmount, const char *path);
static int              vfsbase_remove(struct vfs_mount *vmount, const char *path);
static int              vfsbase_rename(struct vfs_mount *vmount, const char *oldname, const char *newname);

static int              vfsbase_fileno(struct vfs_handle *vhandle);


struct vfs_class *
vfsbase_init(void)
{
    struct vfs_class *vclass;

    vclass = vfs_class_new("base", "", NULL);
    assert(vclass);

    vclass->v_impl.i_unmount  = vfsbase_unmount;

    vclass->v_impl.i_open     = vfsbase_open;
    vclass->v_impl.i_close    = vfsbase_close;
    vclass->v_impl.i_read     = vfsbase_read;
    vclass->v_impl.i_write    = vfsbase_write;
    vclass->v_impl.i_seek     = vfsbase_seek;
    vclass->v_impl.i_tell     = vfsbase_tell;
    vclass->v_impl.i_ioctl    = vfsbase_ioctl;

    vclass->v_impl.i_opendir  = vfsbase_opendir;
    vclass->v_impl.i_readdir  = vfsbase_readdir;
    vclass->v_impl.i_closedir = vfsbase_closedir;

    vclass->v_impl.i_access   = vfsbase_access;
    vclass->v_impl.i_stat     = vfsbase_stat;
    vclass->v_impl.i_lstat    = vfsbase_lstat;
    vclass->v_impl.i_fstat    = vfsbase_fstat;
    vclass->v_impl.i_chmod    = vfsbase_chmod;
    vclass->v_impl.i_chown    = vfsbase_chown;
    vclass->v_impl.i_mkdir    = vfsbase_mkdir;
    vclass->v_impl.i_rmdir    = vfsbase_rmdir;
    vclass->v_impl.i_chdir    = vfsbase_chdir;

    vclass->v_impl.i_readlink = vfsbase_readlink;
    vclass->v_impl.i_symlink  = vfsbase_symlink;
    vclass->v_impl.i_link     = vfsbase_link;
    vclass->v_impl.i_unlink   = vfsbase_unlink;
//  vclacc->v_impl.l_realpath = vfsbase_realpath;

    vclass->v_impl.i_remove   = vfsbase_remove;
    vclass->v_impl.i_rename   = vfsbase_rename;

    vclass->v_impl.i_fileno   = vfsbase_fileno;

    return vclass;
}


static int
vfsbase_unmount(struct vfs_mount *mount)
{
    __UNUSED(mount);
    errno = EOPNOTSUPP;
    return -1;
}


static int
vfsbase_open(struct vfs_mount *vmount, const char *path, int mode, int mask)
{
    int fd = vfsio_open(path, mode, mask);

    if (fd >= 0) {
        struct vfs_handle *vhandle = vfs_handle_new(vmount, 0);

        if (vhandle) {
            vhandle->h_ihandle = fd;
            VFS_TRACE(("vfsbase_open(%p->%d->%d)\n", vhandle, vhandle->h_handle, vhandle->h_ihandle))
            return vhandle->h_handle;
        }
        vfsio_close(fd);
    }
    return -1;
}


static int
vfsbase_close(struct vfs_handle *vhandle)
{
    VFS_TRACE(("vfsbase_close(%p->%d->%d)\n", vhandle, vhandle->h_handle, vhandle->h_ihandle))
    assert(vhandle->h_ihandle >= 0);
    vfsio_close(vhandle->h_ihandle);
    vhandle->h_ihandle = -1;
    vfs_handle_delete(vhandle);
    return 0;
}


static int
vfsbase_read(struct vfs_handle *vhandle, void *buffer, unsigned length)
{
    int fd = vhandle->h_ihandle;
    const unsigned olength = length;
    char *data = (char *)buffer;
    int cnt;

    assert(vhandle->h_ihandle >= 0);
    while (length > 0) {
        if ((cnt = vfsio_read(fd, data, length)) <= 0) {
            if (cnt < 0 && data == (void *)buffer) {
                return -1;
            }
            break;
        }
        length -= cnt; data += cnt;
    }
    return (olength - length);
}


static int
vfsbase_write(struct vfs_handle *vhandle, const void *buffer, unsigned length)
{
    int fd = vhandle->h_ihandle;
    const char *data = (const char *)buffer;
    int total = 0, cnt;

    assert(vhandle->h_ihandle >= 0);
    do {                                        /* EINTR safe */
        if ((cnt = vfsio_write(fd, data, length)) >= 0) {
            data += cnt, total += cnt;
            if ((length -= cnt) == 0) {
                return total;                   /* success */
            }
        }
    } while (cnt >= 0 || (cnt < 0 && errno == EINTR));
    return -1;
}


static int
vfsbase_seek(struct vfs_handle *vhandle, off_t offset, int whence)
{
    assert(vhandle->h_ihandle >= 0);
    return vfsio_lseek(vhandle->h_ihandle, offset, whence);
}


static off_t
vfsbase_tell(struct vfs_handle *vhandle)
{
    assert(vhandle->h_ihandle >= 0);
    return (-1 == vfsio_lseek(vhandle->h_ihandle, 0, 0) ? 0 : -1);
}


static int
vfsbase_ioctl(struct vfs_handle *vhandle, int op, void *data)
{
    __CUNUSED(op);
    __CUNUSED(data);

    assert(vhandle->h_ihandle >= 0);
#if defined(HAVE_IOCTL) || defined(unix) || defined(__APPLE__)
    return ioctl(vhandle->h_ihandle, op, data);
#else
    errno = EOPNOTSUPP;
    return -1;
#endif
}


static int
vfsbase_fileno(struct vfs_handle *vhandle)
{
    assert(vhandle->h_ihandle >= 0);
    return vhandle->h_ihandle;
}


static int
vfsbase_access(struct vfs_mount *vmount, const char *path, int what)
{
    __CUNUSED(vmount);
    return vfsio_access(path, what);
}


static int
vfsbase_stat(struct vfs_mount *vmount, const char *path, struct stat *sb)
{
    __CUNUSED(vmount);
    return vfsio_stat(path, sb);
}


static int
vfsbase_lstat(struct vfs_mount *vmount, const char *path, struct stat *sb)
{
    __CUNUSED(vmount);
#if defined(HAVE_LSTAT)
    return vfsio_lstat(path, sb);
#else
    return vfsio_stat(path, sb);
#endif
}


static int
vfsbase_fstat(struct vfs_handle *vhandle, struct stat *sb)
{
    assert(vhandle->h_ihandle >= 0);
    return fstat(vhandle->h_ihandle, sb);
}


static int
vfsbase_chmod(struct vfs_mount *vmount, const char *path, int mode)
{
    __CUNUSED(vmount);
    return vfsio_chmod(path, mode);
}


static int
vfsbase_chown(struct vfs_mount *vmount, const char *path, int owner, int group)
{
    __CUNUSED(vmount);
    __CUNUSED(path);
    __CUNUSED(owner);
    __CUNUSED(group);
#if defined(HAVE_CHOWN) || defined(unix) || defined(__APPLE__)
    return chown(path, owner, group);
#else
    errno = EOPNOTSUPP;
    return -1;
#endif
}


static vfs_dir_t *
vfsbase_opendir(struct vfs_mount *vmount, const char *path)
{
    DIR *dir = opendir(path);
    if (dir) {
        unsigned pathlen = strlen(path);
        struct vfs_handle *vhandle = vfs_handle_new(vmount, sizeof(vfsbase_dir_t));

        if (vhandle) {
            vfsbase_dir_t *basedir = (vfsbase_dir_t *)(vhandle + 1);
            vfs_dir_t *vdir = (vfs_dir_t *)(basedir);

            vhandle->h_phandle = (void *)dir;
            vdir->d_magic = VDIR_MAGIC;
            vdir->d_handle = vhandle->h_handle;
            memcpy(basedir->d_path, path, pathlen);
            basedir->d_mntseq = vfs_mount_seq();
            basedir->d_mntcnt = vfs_mount_list(path, pathlen, VMOUNT_FVIRTUAL,
                basedir->d_mounts, sizeof(basedir->d_mounts)/sizeof(struct vfs_mount *));
            return vdir;
        }
        closedir(dir);
    }
    return NULL;
}


static int
isspecialfile(struct dirent *dent)
{
    const char *name = dent->d_name;

    if (name[0] == ',') {
        if (name[1] == 0 || (name[1] == '.' && name[2] == 0)) {
            return 1;
        }
    }
    return 0;
}


static vfs_dirent_t *
vfsbase_readdir(struct vfs_handle *vhandle)
{
    vfsbase_dir_t *dir = (vfsbase_dir_t *)(vhandle + 1);
    vfs_dirent_t *vdirent = NULL;
    struct dirent *dent = NULL;
    unsigned ismountpoint = 0;                  /* mountpoint, 0=no, 1=real, 2=virtual*/
    const char *name = NULL;
    unsigned namlen = 0;

    /* retrieve next directory entry */
    if (NULL != (dent = dir->d_cached)) {
        dir->d_cached = NULL;                   /* clear cached element */
    } else {
        if (NULL == (dent = readdir((DIR *)vhandle->h_phandle))) {
            vhandle->h_errcode = errno;
        }
    }
    if (dent)  {
        name = (const char *)dent->d_name;
        namlen = strlen(name);
    }

    /* reload mount list, if stale */
    if (dir->d_mntcnt) {
        unsigned mntseq;                        /* current mount sequence */

        if ((mntseq = vfs_mount_seq()) != dir->d_mntseq) {
            dir->d_mntseq = mntseq;
            dir->d_mntcnt = vfs_mount_list(dir->d_path, dir->d_pathlen, VMOUNT_FVIRTUAL,
                              dir->d_mounts, sizeof(dir->d_mounts)/sizeof(struct vfs_mount *));
        }
    }

    /* insert any local mount points into the directory listing */
    if (NULL == dent || 0 == isspecialfile(dent)) {
        if (dir->d_mntidx < dir->d_mntcnt) {
            /*
             *  XXX - needs work
             */
            struct vfs_mount *vmount = dir->d_mounts[ dir->d_mntidx++ ];

            assert(vmount);
            dir->d_cached = dent;               /* cache entry for next readdir() */
            name = vmount->mt_name;             /* mount point name */
            namlen = vmount->mt_namlen;
            ismountpoint = 1;
        }
    }
    if (name) {
        vdirent = &dir->d_entry;
#if defined(_MSC_VER) || defined(__WATCOMC__)
        vfs_dirent_init(vdirent, vdirent->d_ino, name, namlen, NULL);
#else
        vfs_dirent_init(vdirent, vdirent->d_fileno, name, namlen, NULL);
#endif
    }
    return vdirent;
}


static int
vfsbase_closedir(struct vfs_handle *vhandle)
{
    DIR *dir = (DIR *)vhandle->h_phandle;
    int ret = closedir(dir);

    vfs_handle_delete(vhandle);
    return (ret);
}


static int
vfsbase_mkdir(struct vfs_mount *vmount, const char *path, mode_t mode)
{
    __CUNUSED(vmount);
    __CUNUSED(mode);
    return vfsio_mkdir(path, mode);
}


static int
vfsbase_rmdir(struct vfs_mount *vmount, const char *path)
{
    __CUNUSED(vmount);
    return vfsio_rmdir(path);
}


extern const char *sys_cwd(char *path, int size);   /*FIXME*/

static int
vfsbase_chdir(struct vfs_mount *vmount, const char *path)
{
    const int ret = vfsio_chdir(path);

    __CUNUSED(vmount);
    if (0 == ret) {
        /*
         *  XXX - must convert to an abs path
         *      maybe keep two cwd, one for last physical and the other being the last
         *      virtual current-working directory.
         */
        char t_cwd[VFS_MAXPATH+1];
        VFS_TRACE(("chdir(%s) ==>%s\n", path, sys_cwd(t_cwd, sizeof(t_cwd))))
        vfscwd_set(NULL);
    }
    return ret;
}


static int
vfsbase_readlink(struct vfs_mount *vmount, const char *path, char *buf, size_t size)
{
    __CUNUSED(vmount);
#if defined(HAVE_READLINK) || defined(unix) || \
        defined(_WIN32) || defined(WIN32) || defined(__APPLE__)
    return vfsio_readlink(path, buf, size);
#else
    errno = EOPNOTSUPP;
    return -1;
#endif
}


static int
vfsbase_symlink(struct vfs_mount *vmount, const char *n1, const char *n2)
{
    __CUNUSED(vmount);
#if defined(HAVE_SYMLINK) || defined(unix) || \
        defined(_WIN32) || defined(WIN32) || defined(__APPLE__)
    return vfsio_symlink(n1, n2);
#else
    errno = EOPNOTSUPP;
    return -1;
#endif
}


static int
vfsbase_link(struct vfs_mount *vmount, const char *path1, const char *path2)
{
    __CUNUSED(vmount);
    __CUNUSED(path1);
    __CUNUSED(path2);
#if defined(HAVE_LINK) || defined(unix) || defined(__APPLE__)
    return link(path1, path2);
#else
    errno = EOPNOTSUPP;                         /*FIXME/WIN32*/
    return -1;
#endif
}


static int
vfsbase_unlink(struct vfs_mount *vmount, const char *path)
{
    __CUNUSED(vmount);
    return vfsio_unlink(path);
}


static int
vfsbase_remove(struct vfs_mount *vmount, const char *path)
{
    __UNUSED(vmount);
    return remove(path);
}


static int
vfsbase_rename(struct vfs_mount *vmount, const char *oldname, const char *newname)
{
    __UNUSED(vmount);
    return rename(oldname, newname);
}
/*end*/


