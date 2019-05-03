#ifndef GR_VFS_H_INCLUDED
#define GR_VFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_h,"$Id: vfs.h,v 1.15 2019/03/15 23:23:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs.h,v 1.15 2019/03/15 23:23:01 cvsuser Exp $
 * Virtial File System Interface.
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

#include <edheaders.h>
#include <edsym.h>

__CBEGIN_DECLS

#define VFS_MAXPATH     MAX_PATH                /* see xxxx.h */
#define VFS_MAXNAME     MAX_NAME                /* ditto */

#if defined(MAXSYMLINKS)
#define VFS_MAXSYMLINKS MAXSYMLINKS
#else
#define VFS_MAXSYMLINKS 30
#endif


/*
 *  Directory entry
 */
typedef struct vfs_dirent {
    unsigned            d_magic;                /* Structure magic */
    unsigned            d_fileno;               /* inode/file number of entry */
    unsigned short      d_type;                 /* Entry type */
    unsigned short      d_namlen;               /* Length of string in d_name */
    unsigned            d_stat;                 /* *true* is status is valid */
    time_t              d_ctime;                /* Creation time */
    time_t              d_mtime;                /* Modification time */
    time_t              d_atime;                /* Access time */
    unsigned            d_uid;                  /* Owner */
    unsigned            d_gid;                  /* Group */
    unsigned            d_size;                 /* File size */
    unsigned            d_mode;                 /* File attributes */
    char                d_name[VFS_MAXNAME+1];  /* File name */
} vfs_dirent_t;


/*
 *  Directory handle
 */
typedef struct vfs_dir {
    unsigned            d_magic;                /* Structure magic */
    int                 d_handle;               /* File handle */
} vfs_dir_t;


/*
 *  File handle
 */
typedef struct vfs_file {
    unsigned            f_magic;                /* Structure magic */
    int                 f_handle;               /* Underlying handle */
    char *              f_buffer;               /* Working buffer/cache */
    const char *        f_path;                 /* File path */
    unsigned            f_bsize;                /* Buffer size, in bytes */
    int                 f_flags;                /* Open flags */
    char *              f_cursor;               /* Buffer cursor */
    unsigned            f_left;                 /* Space left within cache */
} vfs_file_t;

//	typedef int vfsssize_t;

extern void                 vfs_init(void);
extern void                 vfs_shutdown(void);

extern int                  vfs_mount(const char *mountpoint, unsigned flags, const char *prefix, const char *arguments);
extern int                  vfs_unmount(const char *mountpoint, unsigned flags);

extern int                  vfs_open(const char *path, int mode, int mask);
extern int                  vfs_reopen(int fd, const char *path, int mode, int mask);
extern int /*vfsssize_t*/   vfs_read(int handle, void *buffer, unsigned length);
extern int /*vfsssize_t*/   vfs_write(int handle, const void *buffer, unsigned length);
extern int                  vfs_close(int handle);
extern int                  vfs_seek(int handle, off_t offset, int whence);
extern off_t                vfs_tell(int handle);
extern int                  vfs_ioctl(int handle, int op, void *data);
extern int                  vfs_fileno(int handle);

extern int                  vfs_chmod(const char *path, int mode);
extern int                  vfs_chown(const char *path, int owner, int group);

extern vfs_dir_t *          vfs_opendir(const char *path);
extern vfs_dirent_t *       vfs_readdir(vfs_dir_t *dir);
extern int                  vfs_statdir(vfs_dirent_t *dent, struct stat *sb);
extern int                  vfs_closedir(vfs_dir_t *dir);

extern int                  vfs_mkdir(const char *path, int mode);
extern int                  vfs_rmdir(const char *path);
extern int                  vfs_chdir(const char *dir);
extern char *               vfs_cwd(char *buffer, unsigned length);

extern int                  vfs_access(const char *path, int what);
extern int                  vfs_stat(const char *path, struct stat *sb);
extern int                  vfs_lstat(const char *path, struct stat *sb);
extern int                  vfs_fstat(int handle, struct stat *sb);

extern int                  vfs_readlink(const char *path, char *buf, size_t size);
extern int                  vfs_symlink(const char *path, const char *path2);
extern int                  vfs_link(const char *path, const char *path2);
extern int                  vfs_unlink(const char *path);
extern int                  vfs_rename(const char *oldname, const char *newname);
extern int                  vfs_remove(const char *path);
extern int                  vfs_mknod(const char *path, int mode, int dev);

extern vfs_file_t *         vfs_fopen(const char *path, int flags, int mode, unsigned bsize);
extern vfs_file_t *         vfs_fdopen(const char *path, int handle, int flags, int mode, unsigned bsize);
extern int                  vfs_fwrite(vfs_file_t *file, const void *buffer, unsigned size);
extern int                  vfs_fputc(vfs_file_t *file, char ch);
extern int                  vfs_fputs(vfs_file_t *file, const char *str);
extern int                  vfs_fclose(vfs_file_t *file);

__CEND_DECLS

#endif /*GR_VFS_H_INCLUDED*/
