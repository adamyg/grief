#ifndef LIBW32_SYS_MOUNT_H_INCLUDED
#define LIBW32_SYS_MOUNT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_mount_h,"$Id: mount.h,v 1.11 2024/03/31 15:57:29 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mount() implementation
 *
 * Copyright (c) 2012 - 2024, Adam Young.
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

#define	MOUNT_NONE      0
#define	MOUNT_UFS       1
#define	MOUNT_NFS       2
#define	MOUNT_MFS       3
#define	MOUNT_PC        4
#define	MOUNT_LFS       5
#define	MOUNT_LO        6
#define	MOUNT_FDESC     7
#define	MOUNT_PORTAL    8
#define MOUNT_NULL      9
#define MOUNT_UMAP      10
#define MOUNT_KERNFS	11

#define MOUNT_TFS       12
#define MOUNT_TMP       13
#define MOUNT_MSDOS     14
#define MOUNT_ISO9660   15
#define	MOUNT_MAXTYPE	15

#define INITMOUNTNAMES { \
    "none",             /*  0 MOUNT_NONE */ \
    "ufs",              /*  1 MOUNT_UFS */ \
    "nfs",              /*  2 MOUNT_NFS */ \
    "mfs",              /*  3 MOUNT_MFS */ \
    "pc",               /*  4 MOUNT_PC */ \
    "lfs",              /*  5 MOUNT_LFS */ \
    "lo",               /*  6 MOUNT_LO */ \
    "fdesc",            /*  7 MOUNT_FDESC */ \
    "portal",           /*  8 MOUNT_PORTAL */ \
    "null",             /*  9 MOUNT_NULL */ \
    "umap",             /* 10 MOUNT_UMAP */ \
    "kernfs",           /* 11 MOUNT_KERNFS */ \
    "tfs",              /* 12 MOUNT_TFS */ \
    "tmp",              /* 13 MOUNT_TMP */ \
    "msdos",            /* 14 MOUNT_MSDOS */ \
    "iso9660",          /* 15 MOUNT_ISO9660 */ \
    0,                  \
}

#define MNT_WAIT        0
#define MNT_NOWAIT      1

__BEGIN_DECLS

struct statfs;

LIBW32_API int          getfsstat(struct statfs *buf, long bufsize, int mode);
LIBW32_API int          getmntinfo(struct statfs **mntbufp, int flags);

__END_DECLS

#endif /*LIBW32_SYS_MOUNT_H_INCLUDED*/
