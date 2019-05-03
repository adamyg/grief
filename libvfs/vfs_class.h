#ifndef GR_VFS_CLASS_H_INCLUDED
#define GR_VFS_CLASS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_class_h,"$Id: vfs_class.h,v 1.10 2019/03/15 23:23:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_class.h,v 1.10 2019/03/15 23:23:01 cvsuser Exp $
 * Virtual File System Interface -- filessytem class definitions.
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

#include <tailqueue.h>
#include "vfs_vops.h"
#include <edsym.h>

__CBEGIN_DECLS

struct vfs_class {
    unsigned            v_magic;                /* structure magic */
    TAILQ_ENTRY(vfs_class)
                        v_node;
    const char *        v_desc;                 /* description */
    const char *        v_prefix;
    unsigned            v_length;               /* prefix length */
    unsigned            v_references;           /* reference count */
    unsigned            v_flags;
#define VCLASS_FAUTOMOUNT   0x0001              /* allow automounts */
#define VCLASS_FFULLPATH    0x0002              /* driver requires fullpath */

    struct vfs_implementation v_impl;           /* implementation */
};

extern void                 vfs_class_start(void);
extern void                 vfs_class_shutdown(void);
extern struct vfs_class *   vfs_class_new(const char *desc, const char *prefix, struct vfs_implementation *impl);
extern void                 vfs_class_delete(struct vfs_class *vfs);
extern struct vfs_class *   vfs_class_get(const char *prefix, unsigned length);

__CEND_DECLS

#endif /*GR_VFS_CLASS_H_INCLUDED*/
