#ifndef GR_VFS_INTERNAL_H_INCLUDED
#define GR_VFS_INTERNAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_internal_h,"$Id: vfs_internal.h,v 1.21 2021/06/10 06:13:02 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_internal.h,v 1.21 2021/06/10 06:13:02 cvsuser Exp $
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

#include "vfs.h"
#include <edsym.h>
#if defined(_WIN32) || defined(WIN32)
#include <../libw32/win32_io.h>                 /* w32_xxx() */
#include <io.h>                                 /* _chdir() ... */
#endif

__CBEGIN_DECLS

#if defined(ENODEV)
#define ENODEVICE           ENODEV
#elif defined(ENOXIO)
#define ENODEVICE           ENOXIO
#else
#error  Unknown value for ENODEVICE ...
#endif

#if defined(ENOSYS)
#define ENOTSUPPORTED       ENOSYS
#elif defined(EXDEV)
#define ENOTSUPPORTED       EXDEV
#elif defined(EIO)
#define ENOTSUPPORTED       EIO
#else
#error  Unknown value for ENOTSUPPORTED ...
#endif

#if !defined(EOPNOTSUPP)                        /* minggw, limited errno's */
#define EOPNOTSUPP          ENOTSUPPORTED
#endif

#define VFS_PREFIX          '#'
#define VFS_PATHSEP         '/'
#if defined(DOSISH) || defined(WIN32)
#define VFS_PATHSEP2        '\\'
#define VFS_ISSEP(__c)      ((__c) == VFS_PATHSEP || (__c) == VFS_PATHSEP2)
#else
#define VFS_ISSEP(__c)      ((__c) == VFS_PATHSEP)
#endif

#if defined(_WIN32) || defined(WIN32)
/*
 *  win32 posix emulation
 *
 *      #include <win32io.h>
 *
 *      w32_stat/w32_lstat
 *      symlink/readlink
 */
#ifndef HAVE_LSTAT
#define HAVE_LSTAT
#endif
#ifndef HAVE_SYMLINK
#define HAVE_SYMLINK
#endif
#ifndef HAVE_READINK
#define HAVE_READLINK
#endif

#define vfsio_open(_fn, _of, _om)   w32_open(_fn, _of, _om)
#define vfsio_stat(_fn, _sb)        w32_stat(_fn, _sb)
#define vfsio_lstat(_fn, _sb)       w32_lstat(_fn, _sb)
#define vfsio_mkdir(_dn, _m)        w32_mkdir(_dn, _m)
#define vfsio_chdir(_dn)            w32_chdir(_dn)
#define vfsio_rmdir(_dn)            w32_rmdir(_dn)
#define vfsio_symlink(_n1,_n2)      w32_symlink(_n1, _n2)
#define vfsio_readlink(_fn,_bf,_sz) w32_readlink(_fn, _bf, _sz)
#endif

/*
 *  defaults ...
 */
#if (defined(_MSC_VER) && (_MSC_VER >= 1400)) || \
        defined(__WATCOMC__)

#if !defined(vfsio_open)
#define vfsio_open(_fn, _of, _om)   _open(_fn, _of, _om)
#endif
#define vfsio_close(_fd)            _close(_fd)
#define vfsio_read(_fd, _ib, _is)   _read(_fd, _ib, _is)
#define vfsio_write(_fd, _ob, _is)  _write(_fd, _ob, _is)
#define vfsio_lseek(_fd, _o, _w)    _lseek(_fd, _o, _w)

#define vfsio_access(_fn, _m)       w32_access(_fn, _m)
#define vfsio_chmod(_fn, _m)        w32_chmod(_fn, _m)
#define vfsio_unlink(_fn)           w32_unlink(_fn)

#if !defined(vfsio_mkdir)
#define vfsio_mkdir(_dn, _m)        _mkdir(_dn)
#define vfsio_chdir(_dn)            _chdir(_dn)
#define vfsio_rmdir(_dn)            _rmdir(_dn)
#endif

#define vfsio_fdopen(_fd, _om)      _fdopen(_fd, _om)
#define vfsio_fileno(_fs)           _fileno(_fs)
#define vfsio_umask(_mk)            _umask(_mk)

#else

#if !defined(vfsio_open)
#define vfsio_open(_fn, _of, _om)   open(_fn, _of, _om)
#endif
#define vfsio_close(_fd)            close(_fd)
#define vfsio_read(_fd, _ib, _is)   read(_fd, _ib, _is)
#define vfsio_write(_fd, _ob, _is)  write(_fd, _ob, _is)
#define vfsio_lseek(_fd, _o, _w)    lseek(_fd, _o, _w)

#ifndef vfsio_stat
#define vfsio_stat(_fn, _sb)        stat(_fn, _sb)
#if defined(HAVE_LSTAT)
#define vfsio_lstat(_fn, _sb)       lstat(_fn, _sb)
#endif
#endif

#ifndef vfsio_symlink
#if defined(HAVE_SYMLINK)
#define vfsio_symlink(_n1,_n2)      symlink(_n1, _n2)
#endif
#if defined(HAVE_READLINK) || defined(HAVE_SYMLINK)
#define vfsio_readlink(_fn,_bf,_sz) readlink(_fn, _bf, _sz)
#endif
#endif

#if !defined(vfsio_access)
#define vfsio_access(_fn, _m)       access(_fn, _m)
#endif
#define vfsio_chmod(_fn, _m)        chmod(_fn, _m)
#define vfsio_unlink(_fn)           unlink(_fn)

#if !defined(vfsio_mkdir)
#define vfsio_mkdir(_dn, _m)        mkdir(_dn, _m)
#define vfsio_chdir(_dn)            chdir(_dn)
#define vfsio_rmdir(_dn)            rmdir(_dn)
#endif

#define vfsio_fdopen(_fd, _om)      fdopen(_fd, _om)
#define vfsio_fileno(_fs)           fileno(_fs)
#define vfsio_umask(_mk)            umask(_mk)
#endif


/*
 *  trace support
 */
#if (1)
extern unsigned             x_vfs_trace;

extern void                 vfs_trace(const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));

#define VFS_TRACE(x)        vfs_trace x;
#define VFS_TRACE2(x)       vfs_trace x;
#define VFS_LEVELINC()      ++x_vfs_trace;
#define VFS_LEVELDEC()      if (x_vfs_trace) --x_vfs_trace;

#else
#define VFS_TRACE(x)
#define VFS_TRACE2(x)
#define VFS_LEVELINC()
#define VFS_LEVELDEC()
#endif


/*
 *  general definitions
 */

#define VINSTANCE_MAGIC     MKMAGIC('V','f','s','V')
#define VMOUNT_MAGIC        MKMAGIC('V','f','s','M')
#define VTREE_MAGIC         MKMAGIC('V','f','s','T')
#define VHANDLE_MAGIC       MKMAGIC('V','f','s','H')
#define VDIR_MAGIC          MKMAGIC('V','f','s','D')
#define VDIRENT_MAGIC       MKMAGIC('V','f','s','E')
#define VDIRECTST_MAGIC     0x4252
#define VFILE_MAGIC         MKMAGIC('V','f','s','F')
#define VCACHE_MAGIC        MKMAGIC('V','f','s','C')
#define VNODE_MAGIC         MKMAGIC('V','f','s','N')

extern struct vfs_class *   vfsbase_init(void);
extern struct vfs_class *   vfsarc_init(void);
extern struct vfs_class *   vfscurl_init(void);

extern struct vfs_mount *   vfs_root(void);

extern void                 vfscwd_set(const char *path);
extern void                 vfscwd_set2(const char *mount, const char *path);
extern const char *         vfscwd_get(void);

extern void                 vfs_dirent_init(struct vfs_dirent *vdirent, int fileno,
                                    const char *name, unsigned namlen, struct stat *sb);

extern unsigned             vfs_name_hash(const char *name, unsigned length);

extern char *               vfs_fix_slashes(char *str);
extern char *               vfs_fix_case(char *str);

extern const char *         vfs_tempnam(int *fd);

__CEND_DECLS

#endif /*GR_VFS_INTERNAL_H_INCLUDED*/
