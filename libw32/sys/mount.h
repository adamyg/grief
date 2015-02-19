#ifndef GR_MOUNT_H_INCLUDED
#define GR_MOUNT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_mount_h,"$Id: mount.h,v 1.6 2015/02/19 00:17:38 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 mount() implementation
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#define MOUNT_UFS   0
#define MOUNT_NFS   1
#define MOUNT_PC    2
#define MOUNT_MFS   3
#define MOUNT_LO    4
#define MOUNT_TFS   5
#define MOUNT_TMP   6

#define MNT_WAIT    0
#define MNT_NOWAIT  1

__BEGIN_DECLS

struct statfs;

extern int                  getmntinfo(struct statfs **mntbufp, int flags);

__END_DECLS

#endif /*GR_MOUNT_H_INCLUDED*/
