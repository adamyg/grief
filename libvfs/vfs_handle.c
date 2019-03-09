#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_handle_c,"$Id: vfs_handle.c,v 1.11 2018/10/01 22:16:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_handle.c,v 1.11 2018/10/01 22:16:27 cvsuser Exp $
 * Virtual file system - file handle management.
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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
#include <edhandles.h>
#include <errno.h>

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_mount.h"
#include "vfs_handle.h"

typedef RB_HEAD(handletree, vfs_handle) handletree_t;

static int              handle_compare(struct vfs_handle *a, struct vfs_handle *b);

RB_PROTOTYPE(handletree, vfs_handle, h_rbnode, handle_compare);
RB_GENERATE(handletree, vfs_handle, h_rbnode, handle_compare);

static handletree_t     x_handletree;
static struct vfs_handle *x_handlecache;

static int              x_handle = GRBASE_FILE; /* vfs handle sequence */


/*  Function:       vfs_handle_start
 *      Initialise vfs handle subsystem.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_handle_start(void)
{
    RB_INIT(&x_handletree);
    x_handlecache = NULL;
}


/*  Function:       vfs_handle_shutdown
 *      Shutdown the vfs handle subsystem.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_handle_shutdown(void)
{
}


/*  Function:       vfs_handle_new
 *      Allocate and initialise a new handle object.
 *
 *  Parameters:
 *      vmount -        Mount/owner.
 *      additional -    Additional storage to allocated along with the base
 *                      object requirements.
 *
 *  Returns:
 *      Address of allocated handle object, otherwise NULL.
 */
struct vfs_handle *
vfs_handle_new(struct vfs_mount *vmount, unsigned additional)
{
    struct vfs_handle *vhandle;

    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);

    additional += sizeof(struct vfs_handle);
    if (NULL == (vhandle = chk_alloc(additional))) {
        return NULL;
    }

    memset(vhandle, 0, additional);
    vhandle->h_magic = VHANDLE_MAGIC;
    vhandle->h_mount = vmount;
    vhandle->h_handle = x_handle++;             /* unique identifier */
    vhandle->h_ihandle = -1;                    /* unassigned */
    assert(NULL == RB_FIND(handletree, &x_handletree, vhandle));
    RB_INSERT(handletree, &x_handletree, vhandle);
    TAILQ_INSERT_TAIL(&vmount->mt_fdhandleq, vhandle, h_mtnode);
    ++vmount->mt_fdcount;

    VFS_TRACE(("vfs_handle_new(%p) : %p(%d)\n", vmount, vhandle, vhandle->h_handle))
    return vhandle;
}


/*  Function:       vfs_handle_get
 *      Retrieve the handle object associated with the specified handle identifier.
 *
 *  Parameters:
 *      handle -        Unique handle identifier.
 *
 *  Returns:
 *      Address of associatd handle object, otherwise NULL.
 */
struct vfs_handle *
vfs_handle_get(int handle)
{
    struct vfs_handle *vhandle;

    if (x_handlecache && handle == x_handlecache->h_handle) {
        /*
         *  Simple last handle cache
         *      should address most sequental read/write operations.
         */
        vhandle = x_handlecache;
    
    } else {
        /*
         *  Lookup
         */
        struct vfs_handle find;

        find.h_magic = VHANDLE_MAGIC;
        find.h_handle = handle;
        if (NULL != (vhandle = RB_FIND(handletree, &x_handletree, &find))) {
            VFS_TRACE(("vfs_handle_get(%d) : %p\n", handle, vhandle))
            return vhandle;
        }
        x_handlecache = vhandle;
    }    
    VFS_TRACE(("vfs_handle_get(%d) : NULL\n", handle))
    return NULL;
}


/*  Function:       vfs_handle_delete
 *      Delete the specified handle object.
 *
 *  Parameters:
 *      vhandle -       Address of handle object.
 *
 *  Returns:
 *      nothing
 */
void
vfs_handle_delete(struct vfs_handle *vhandle)
{
    struct vfs_mount *vmount = vhandle->h_mount;

    assert(vhandle);
    assert(VHANDLE_MAGIC == vhandle->h_magic);
    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);

    VFS_TRACE(("vfs_handle_delete(%p->%d)\n", vhandle, vhandle->h_handle))

    RB_REMOVE(handletree, &x_handletree, vhandle);
    TAILQ_REMOVE(&vmount->mt_fdhandleq, vhandle, h_mtnode);
    --vmount->mt_fdcount;
    vhandle->h_magic = 0;
    x_handlecache = NULL;
    chk_free(vhandle);
}


/*  Function:       handle_compare
 *      Compare two nodes within a handle tree, returning their comparsion
 *      value in handle order.
 *
 *  Parameters:
 *      a -             First node image.
 *      b -             Second node image
 *
 *  Returns:
 *      Zero(0) if the nodes are equal, one(1) upon (a > b) otherwise
 *      negative-one(-1) when (a < b).
 */
static int
handle_compare(struct vfs_handle *a, struct vfs_handle *b)
{
    assert(VHANDLE_MAGIC == a->h_magic);
    assert(VHANDLE_MAGIC == b->h_magic);

    if (a->h_handle == b->h_handle) {
        return 0;

    } else if (a->h_handle > b->h_handle) {
        return 1;
    }
    return -1;
}
/*end*/
