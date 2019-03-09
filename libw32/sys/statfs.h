#ifndef LIBW32_SYS_STATFS_H_INCLUDED
#define LIBW32_SYS_STATFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_statfs_h,"$Id: statfs.h,v 1.7 2018/09/29 02:25:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 [f]statfs implementation
 *
 * Copyright (c) 1998 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <sys/cdefs.h>
#include <sys/utypes.h>
#include <sys/mount.h>

#define FS_MAGIC    0x11954                     /* Taken from HP-UX */

#define MFSNAMELEN  16                          /* length of fs type name, including null */
#define MNAMELEN    90                          /* length of buffer for returned name */

typedef struct fsid {                           /* file system id type */
    unsigned long   val[2]; 
} fsid_t;

struct statfs {
    long            f_spare2;                   /* placeholder */
    long            f_bsize;                    /* fundamental file system block size */
    long            f_iosize;                   /* optimal transfer block size */
    long            f_blocks;                   /* total data blocks in file system */
    long            f_bfree;                    /* free blocks in fs */
    long            f_bavail;                   /* free blocks avail to non-superuser */
    long            f_files;                    /* total file nodes in file system */
    long            f_ffree;                    /* free file nodes in fs */
    fsid_t          f_fsid;                     /* file system id */
    uid_t           f_owner;                    /* user that mounted the filesystem */
    int             f_type;                     /* type of filesystem (see below) */
    int             f_flags;                    /* copy of mount flags */
    long            f_spare[2];                 /* spare for later */
    char            f_fstypename[MFSNAMELEN];   /* fs type name */
    char            f_mntonname[MNAMELEN];      /* directory on which mounted */
    char            f_mntfromname[MNAMELEN];    /* mounted filesystem */
};

__BEGIN_DECLS

LIBW32_API int      statfs(const char *, struct statfs *);
LIBW32_API int      fstatfs(int, struct statfs *);

__END_DECLS

#endif  /*LIBW32_SYS_STATFS_H_INCLUDED*/
