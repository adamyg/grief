#ifndef GR_M_VFS_H_INCLUDED
#define GR_M_VFS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_vfs_h,"$Id: m_vfs.h,v 1.6 2014/10/22 02:33:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_vfs.h,v 1.6 2014/10/22 02:33:11 ayoung Exp $
 * Virtual file system -- macro interface.
 *
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

#include <edsym.h>

__CBEGIN_DECLS

extern void                 do_vfs_mount(void);
extern void                 do_vfs_unmount(void);
extern void                 inq_vfs_mounts(void);

__CEND_DECLS

#endif /*GR_M_VFS_H_INCLUDED*/
