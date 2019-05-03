#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_vops_c,"$Id: vfs_vops.c,v 1.11 2019/03/15 23:23:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_vops.c,v 1.11 2019/03/15 23:23:04 cvsuser Exp $
 * Virtual file system - virtual operators.
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
#include <errno.h>

#include "vfs_internal.h"
#include "vfs_mount.h"
#include "vfs_class.h"
#include "vfs_lookup.h"
#include "vfs_node.h"


/*  Function:           vfs_vop_mount
 *      Execute the file-system specific mount() method.
 *
 *  Paramaters:
 *      vmount -            Mount object.
 *      argument -          Argument buffer.
 *
 *  Returns:
 *      0 on success, otherwise the non-zero system error code.
 */
int
vfs_vop_mount(struct vfs_mount *vmount, const char *argument)
{
    struct vfs_class *vclass;
    int ret = 0;

    VFS_LEVELINC()
    VFS_TRACE(("vop_mount(%p,%s)\n", vmount, (argument?argument:"")))

    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(vmount->mt_class);

    vclass = vmount->mt_class;
    if (vclass->v_impl.i_mount) {
        ret = (*vclass->v_impl.i_mount)(vmount, argument);
    }
    VFS_TRACE(("== %d\n", ret))
    VFS_LEVELDEC()
    return ret;
}


/*  Function:           vfs_vop_unmount
 *      Execute the file-system specific unmount() method.
 *
 *  Paramaters:
 *      vmount -            Mount object.
 *
 *  Returns:
 *      0 on success, otherwise the non-zero system error code.
 */
int
vfs_vop_unmount(struct vfs_mount *vmount)
{
    struct vfs_class *vclass;
    int ret = 0;

    VFS_LEVELINC()
    VFS_TRACE(("vop_unmount(%p)\n", vmount))

    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(vmount->mt_class);

    vclass = vmount->mt_class;
    if (vclass->v_impl.i_unmount) {
        vmount->mt_flags |= VMOUNT_FSHUTDOWN;
        ret = (*vclass->v_impl.i_unmount)(vmount);
    }

    VFS_TRACE(("== %d\n", ret))
    VFS_LEVELDEC()
    return ret;
}


/*  Function:           vfs_vop_lookup
 *      Execute the file-system specific lookup() method.
 *
 *  Paramaters:
 *      lk -                Lookup control block.
 *
 *  Returns:
 *      0 on success, otherwise the non-zero system error code.
 */
int
vfs_vop_lookup(struct vfs_lookup *lk)
{
    struct vfs_mount *vmount = lk->l_mount;
    struct vfs_class *vclass;
    int ret = ENOENT;

    VFS_LEVELINC()
    VFS_TRACE(("vop_lookUp(%p, %p, %.*s)\n", vmount, lk->l_node, lk->l_namlen, lk->l_name))

    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(vmount->mt_class);
    assert(lk);

    vclass = vmount->mt_class;
    if (vclass->v_impl.i_lookup) {
        ret = (*vclass->v_impl.i_lookup)(lk);
    }
    
    VFS_TRACE(("== %d\n", ret))
    VFS_LEVELDEC()
    return ret;
}


/*  Function:           vfs_vop_delete
 *      Execute the file-system specific delete() method.
 *
 *  Paramaters:
 *      node -              Node reference to be deleted.
 *
 *  Returns:
 *      nothing
 */
int
vfs_vop_delete(struct vfs_node *node)
{
    struct vfs_mount *vmount = node->v_mount;
    struct vfs_class *vclass;
    int ret = ENOENT;

    VFS_LEVELINC()
    VFS_TRACE(("vop_destroy(%p)\n", node))

    assert(node);
    assert(VNODE_MAGIC == node->v_magic);
    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(vmount->mt_class);

    vclass = vmount->mt_class;
    if (vclass->v_impl.i_delete) {
        ret = (*vclass->v_impl.i_delete)(vmount, node);
    }

    VFS_TRACE(("== %d\n", ret))
    VFS_LEVELDEC()
    return ret;
}


/*  Function:           vfs_vop_readlink
 *      Execute the file-system specific readlink() method.
 *
 *  Paramaters:
 *      vmount -            Lookup control block.
 *      linkpath -          Return buffer.
 *      linklen -           Length of the return buffer.
 *
 *  Returns:
 *      0 on success, otherwise the non-zero system error code.
 */
int
vfs_vop_readlink(struct vfs_node *node, char *linkpath, int *linklen)
{
    struct vfs_mount *vmount = node->v_mount;
 /* struct vfs_class *vclass; */
    int ret = ENOENT;

    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(vmount->mt_class);
    assert(linkpath);
    assert(linklen);

    VFS_LEVELINC()
    VFS_TRACE(("vop_readlink(%p)\n", node))
    *linklen = 0;

/*
//  if (vclass->v_impl.i_readlink) {
//      ret = (*impl->i_lookup)(vmount, vfs_tree_path(l->node), linkpath, &linklen);
//  }
*/

    VFS_TRACE(("== %d\n", ret))
    VFS_LEVELDEC()
    return ret;                                 /* XXX - symlinks currently ignored */
}
/*end*/
