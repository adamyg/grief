#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_node_c,"$Id: vfs_node.c,v 1.13 2022/03/21 14:27:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_node.c,v 1.13 2022/03/21 14:27:23 cvsuser Exp $
 * Virtual file system interface - node management.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"
#include "vfs_node.h"


struct vfs_node *
vfs_node_new(unsigned type, const char *name, unsigned namlen, unsigned namhash, struct vfs_node *parent)
{
    unsigned size = sizeof(struct vfs_node) + namlen + 1;
    time_t now = time(NULL);
    struct vfs_node *node;

    if (NULL == (node = chk_alloc(size))) {
        return NULL;                            /* memory allocation error */
    }
    memset(node, 0, size);
    node->v_magic = VNODE_MAGIC;
    node->v_type = type;
    TAILQ_INIT(&node->v_childlist);
    node->v_namhash = namhash;
    node->v_namlen = namlen;
    node->v_name = (const char *)(node + 1);
    memcpy((char *)node->v_name, name, namlen);
    node->v_references = 1;
    node->v_st.st_ctime = now;
    node->v_st.st_mtime = now;
    node->v_st.st_atime = now;
    node->v_st.st_mode = vfs_node_type2mode(type);
#if defined(HAVE_GETEUID) || defined(unix)
    node->v_st.st_uid = geteuid();
    node->v_st.st_gid = getegid();
#elif defined(HAVE_GETUID)
    node->v_st.st_uid = getuid();
    node->v_st.st_gid = getgid();
#else
    node->v_st.st_uid = 42;
    node->v_st.st_gid = 42;
#endif
    if (parent) {
        node->v_fileno = ++parent->v_childseq;
        TAILQ_INSERT_TAIL(&parent->v_childlist, node, v_listnode);
        ++parent->v_childcount;
        ++parent->v_childedit;
    }
    node->v_parent = parent;
    return node;
}


/*  Function:           vfs_node_destroy
 *      Unconditionally destroy the given node and any children.
 *
 *  Parameters:
 *      node -              Root of node tree to be destroyed.
 *
 *  Returns:
 *      nothing.
 */
void
vfs_node_destroy(struct vfs_node *node)
{
    struct vfs_node *parent = NULL;

    VFS_LEVELINC()
    VFS_TRACE(("node_destroy(%p)\n", node))

    assert(node);
    assert(VNODE_MAGIC == node->v_magic);

    /* destroy children */
    if (node->v_childcount) {
        unsigned count = 0, childcount = node->v_childcount;
        struct vfs_node *child;

        while (NULL != (child = TAILQ_FIRST(&node->v_childlist))) {
            assert(VNODE_MAGIC == child->v_magic);
            assert(node == child->v_parent);
            vfs_node_destroy(child);
            ++count;
        }
        assert(0 == node->v_childcount);
        assert(count == childcount);
    }
    assert(NULL == vfs_node_first(node));

    vfs_vop_delete(node);

    /* unlink from parent */
    if (NULL != (parent = node->v_parent)) {
        assert(VNODE_MAGIC == parent->v_magic);
        assert(parent->v_childcount);

        TAILQ_REMOVE(&parent->v_childlist, node, v_listnode);
        --parent->v_childcount;
        ++parent->v_childedit;
        node->v_parent = NULL;
    }

    /* destroy any local backing image */
    if (node->v_localname) {
        VFS_TRACE(("\tlocalname(%s)\n", node->v_localname))
        vfsio_unlink(node->v_localname);
        chk_free((char *)node->v_localname);
        node->v_localname = NULL;
    }
    node->v_magic = 0x050a050a;
    chk_free(node);
    VFS_LEVELDEC()
}



const char *
vfs_node_type2str(unsigned type)
{
    switch(type) {
    case VNODE_FIFO: return "fifo";
    case VNODE_DIR:  return "dir";
    case VNODE_REG:  return "reg";
    case VNODE_LNK:  return "lnk";
    case VNODE_BAD:  return "bad";
    case VNODE_UNKNOWN:
        return "Unknown";
    }
    return "<unknown-type>";
}


unsigned
vfs_node_type2mode(unsigned type)
{
    switch(type) {
    case VNODE_FIFO: return S_IFIFO;
    case VNODE_DIR:  return S_IFDIR;
    case VNODE_REG:  return S_IFREG;
#if defined(S_IFLNK)				/*FIXME/MINGW*/
    case VNODE_LNK:  return S_IFLNK;
#endif
    }
    return 0;
}


int
vfs_node_unlink(struct vfs_node *node)
{
    assert(node);
    assert(VNODE_MAGIC == node->v_magic);

    if (VNODE_DIR == node->v_type) {
        errno = EISDIR;                         /* can not unlink a directory */
        return -1;
    }
    assert(0 == node->v_childcount);
    assert(node->v_references);
    if (node->v_references <= 1) {              /* allow references==0 */
        node->v_references = 0;
        vfs_node_destroy(node);
    }
    --node->v_references;
    return 0;
}


/*  Function:           vfs_node_first
 *      Return the first node within the current directory
 *
 *  Parameters:
 *      node -              Directory parent.
 *
 *  Returns:
 *      Address of the first chld node, otherwise NULL.
 */
struct vfs_node *
vfs_node_first(struct vfs_node *node)
{
    assert(node);
    assert(VNODE_MAGIC == node->v_magic);
    return TAILQ_FIRST(&node->v_childlist);
}


/*  Function:           vfs_node_next
 *      Return the next node within the current directory
 *
 *  Parameters:
 *      node -              Current directory child.
 *
 *  Returns:
 *      Address of the next chld node, otherwise NULL.
 */
struct vfs_node *
vfs_node_next(struct vfs_node *node)
{
    assert(node);
    assert(VNODE_MAGIC == node->v_magic);
    return TAILQ_NEXT(node, v_listnode);
}


/*  Function:           vfs_node_namecmp
 *      Compare the specified 'node' against the filename 'name'.   The comparsion
 *      shall be specific the mount case options.
 *
 *  Parameters:
 *      node -              Source node.
 *      name -              FIle name to compare against.
 *      namlen -            Length of name. in bytes.
 *
 *  Returns:
 *      Result of the comparsion, zero if a match otherwise non-zero.
 */
int
vfs_node_namecmp(struct vfs_node *node, const char *name, unsigned namlen)
{
    assert(node);
    assert(node->v_mount);

    if (node->v_namlen == namlen) {
        if (node->v_mount)
            if (node->v_mount->mt_flags & VMOUNT_FCASEIGNORE) {
                return str_nicmp(node->v_name, name, namlen);
            }
        return strncmp(node->v_name, name, namlen);
    }
    return (node->v_namlen > namlen ? 1 : -1);
}
/*end*/
