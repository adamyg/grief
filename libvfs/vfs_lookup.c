#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_lookup_c,"$Id: vfs_lookup.c,v 1.12 2018/10/01 22:16:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_lookup.c,v 1.12 2018/10/01 22:16:27 cvsuser Exp $
 * Virtual file system interface - node management.
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
#include <errno.h>

#include "vfs_internal.h"
#include "vfs_mount.h"
#include "vfs_tree.h"
#include "vfs_node.h"
#include "vfs_class.h"
#include "vfs_lookup.h"

static int          lookup(vfs_lookup_t *lk);

static int          vop_rdonly(struct vfs_node *node);


/*  Function:       vfs_lookup
 *      xxx
 *
 *  Parameters:
 *      vmount -        xxx
 *      path -          xxx
 *      pathlen -       xxx
 *      op -            xxx
 *      flags -         xxx
 *      lk -            Lookup management object.
 *
 *  Returns:
 *       0 on success, otherwise non-zero manifest error constant.
 */
struct vfs_node *
vfs_lookup(struct vfs_mount *vmount, const char *path, unsigned pathlen,
        unsigned op, unsigned flags, vfs_lookup_t *lk)
{
    struct vfs_tree *tree = vmount->mt_tree;    /* assigned tree (if any) */

    assert(vmount);
    assert(vmount->mt_class);
    assert(path);
    assert(lk);

    memset(lk, 0, sizeof(vfs_lookup_t));

    lk->l_op = op;                              /* operation */
    lk->l_flags = flags;                        /* search flags */
    lk->l_mount = vmount;                       /* vnode tree */

    if (NULL == (lk->l_root = vmount->mt_root)) {
        if (tree) {                             /* .. use assigned tree (if any) */
            lk->l_root = tree->t_root;
        }
    }

//TODO - cwd
//  /* current working directory */
//  if (NULL == (lk->l_cwd = vfs_cwdnode())) {
//      if (NULL == (lk->l_cwd = vmount->mt_cwd) {
//          if (tree) {
//              lk->l_cwd = tree->t_cwd;
//          }
//          lk->l_cwd = vfs_directory();
//      }
//  }

    if (NULL == (lk->l_node = lk->l_cwd)) {
        lk->l_node = lk->l_root;                /* otherwise tree node */
    }
    assert(lk->l_node);
    lk->l_parent = NULL;

    if ((lk->l_pathlen = pathlen) >= sizeof(lk->l_buffer)) {
        lk->l_errno = ENAMETOOLONG;             /* original path too long */
        return NULL;
    }
    memcpy(lk->l_buffer, (const char *)path, lk->l_pathlen);
    lk->l_cursor = lk->l_buffer;

    return vfs_resolve(lk);
}


/*  Function:       vfs_resolve
 *      xxx
 *
 *  Parameters:
 *      lk -            Lookup management object.
 *
 *  Returns:
 *      Address of resolved node, otherwise NULL on failure.
 */
struct vfs_node *
vfs_resolve(vfs_lookup_t *lk)
{
    char linkpath[VFS_MAXPATH+1];
    int linklen;

    VFS_LEVELINC()
    VFS_TRACE(("vfs_resolve(%s, pathlen:%u)\n", lk->l_cursor, lk->l_pathlen))
    for (;;) {
        /*
         *  Check whether root directory should replace current directory
         */
        const char *cursor = lk->l_cursor;
        struct vfs_node *node = lk->l_node;
        int ret;

        assert(cursor);
        if (VFS_ISSEP(*cursor)) {
            VFS_TRACE(("=> root directory\n"))
            while (VFS_ISSEP(*cursor)) {
                ++cursor;                       /* consume leading */
            }
            lk->l_node = node = lk->l_root;
            lk->l_cursor = cursor;
        }

        assert(node);
        assert(VNODE_DIR == node->v_type);

        VFS_LEVELINC()
        if (0 != (ret = lookup(lk)) ||
                (NULL == (node = lk->l_node))) {
            VFS_TRACE(("=> lookup failure\n"))
            VFS_LEVELDEC()
            if (0 == ret)
                ret = EIO;                      /* XXX - required */
            lk->l_errno = ret;
            break;
        }
        VFS_LEVELDEC()

        /*
         *  symlinks
         */
        if ((VNODE_LNK != node->v_type) ||
                (0 == (lk->l_flags & LK_FFOLLOW))) {
            break;                              /* neither a link nor are we following */
        }
        if (++lk->l_loopcnt > VFS_MAXSYMLINKS) {
#if defined(ELOOP)
            lk->l_errno = ELOOP;                /* symlink depth exceeded */
#elif defined(EMLINK)
            lk->l_errno = EMLINK;               /* symlink depth exceeded */
#else
#error Neither ELOOP nor EMLINK are defined
#endif
            break;
        }
        linklen = 0;
        if ((ret = vfs_vop_readlink(lk->l_node, linkpath, &linklen)) != 0) {
            lk->l_errno = ret;                  /* could not resolve symlink */
            break;
        }
        VFS_TRACE(("=> readlink(%u/%*s)\n", linklen, linklen, linkpath))
        if (linklen + lk->l_pathlen >= VFS_MAXPATH) {
            lk->l_errno = ENAMETOOLONG;         /* symlink name too long */
            break;
        }
        if (lk->l_pathlen > 1)                  /* insert result into working buffer */
            memmove(lk->l_buffer + linklen, (const char *)lk->l_cursor, lk->l_pathlen);
        memcpy(lk->l_buffer, (const char *)linkpath, linklen);
        lk->l_cursor = lk->l_buffer;
        lk->l_pathlen += linklen;
    }
    VFS_LEVELDEC()

    if (0 == lk->l_errno) {
        if (lk->l_node)
            return lk->l_node;
        lk->l_errno = ENOENT;                   /* not found */
    }
    return NULL;
}


/*  Function:       lookup
 *      Lookup engine
 *
 *  Parameters:
 *      lk -            Lookup management object.
 *
 *  Returns:
 *      0 on success, otherwise non-zero manifest error constant.
 */
static int
lookup(vfs_lookup_t *lk)
{
    struct vfs_node *node;
    const char *name, *cursor;
    unsigned namhash = 0;
    int namlen, error = 0;

dirloop:;
    /*
     *  search for the next directory
     */
    node = lk->l_node;
    name = lk->l_cursor;
    namlen = 0;
    for (cursor = name; *cursor && !VFS_ISSEP(*cursor); ++cursor) {
        ++namlen;
    }
    if ((namlen = cursor - lk->l_cursor) > VFS_MAXNAME) {
        error = ENAMETOOLONG;
        goto bad;
    }
    namhash = vfs_name_hash(name, namlen);
    lk->l_name = name;
    lk->l_namlen = namlen;
    lk->l_namhash = namhash;
    lk->l_pathlen -= namlen;
    lk->l_cursor = cursor;

    VFS_TRACE(("lookup(name:%.*s, namlen:%u, namhash:%u, pathlen:%u, cursor:%s, node:%p)\n", \
        namlen, name, namlen, namhash, lk->l_pathlen, cursor, lk->l_node))

    lk->l_flags &= ~(LK_FISDOTDOT|LK_FISLAST|LK_FISSYMLINK);
    if (2 == namlen &&  '.' == name[1] && '.' == name[0]) {
        lk->l_flags |= LK_FISDOTDOT;
    }
    if (!*cursor) {
        lk->l_flags |= LK_FISLAST;
    }

    /*
     *  allow ("/" or "") of addressing a directory same as ("/." and ".")
     */
    if (0 == namlen) {
        if (VNODE_DIR != node->v_type) {
            error = ENOTDIR;
            goto bad;
        }
        if (LK_OLOOKUP != lk->l_op && LK_OMKTREE != lk->l_op) {
            error = EISDIR;
            goto bad;
        }
        return (0);
    }

    /*
     *  handle ".."
     */
    if (lk->l_flags & LK_FISDOTDOT) {
        /*
         *  XXX - lookup backwards, verify support going out of the
         *      current mount-point into the parent.
         */
        if (node->v_parent) {
            lk->l_node = node->v_parent;        /* same tree */
            goto dirloop;
        } else {
            if (lk->l_flags & LK_FNOCROSSMOUNT) {
                error = EXDEV;                  /* cross-device link */
                goto bad;
            }
            lk->l_node = NULL;                  /* root non-virtual tree */
            if (lk->l_mount)                    /* parent */
                lk->l_mount = lk->l_mount->mt_parent;
            goto dirloop;
        }
    }

    /*
     *  lookup/resolve
     */
    if (0 != (error = vfs_vop_lookup(lk))) {
        goto bad;
    }
    cursor = lk->l_cursor;                      /* reload */
    if (VNODE_LNK == node->v_type &&
            ((lk->l_flags & LK_FFOLLOW) || VFS_ISSEP(*cursor))) {
        lk->l_flags = LK_FISSYMLINK;
        return 0;
    }

    /*
     *  not a symlink .. deal with next compoent if any.
     */
    if (VFS_ISSEP(*cursor)) {
        while (VFS_ISSEP(*cursor)) {
            ++cursor;
            --lk->l_pathlen;
        }
        lk->l_cursor = cursor;
        goto dirloop;                           /* next component */
    }

    if (LK_ODELETE == lk->l_op || LK_ORENAME == lk->l_op) {
        if (vop_rdonly(node)) {
            error = EROFS;                      /* read only file system */
            goto bad;
        }
    }
    return (0);

bad:;
    VFS_TRACE(("\t= %s (%d)\n", strerror(error), error))
    lk->l_node = NULL;
    return error;
}


/*  Function:       vop_rdonly
 *      Determine whether mounted on read-only file system.
 *
 *  Paramaters:
 *      node -          Lookup control block.
 *
 *  Returns:
 *      1 on read-only, otherwise 0.
 */
static int
vop_rdonly(struct vfs_node *node)
{
    struct vfs_mount *vmount = node->v_mount;
    int rdonly = (vmount && vmount->mt_flags & VMOUNT_FRDONLY) ? 1 : 0;

    return rdonly;
}
/*end*/
