#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_tree_c,"$Id: vfs_tree.c,v 1.19 2019/03/15 23:23:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_tree.c,v 1.19 2019/03/15 23:23:02 cvsuser Exp $
 * Virtual file system interface - tree management.
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
#include "vfs_cache.h"
#include "vfs_lookup.h"
#include "vfs_node.h"
#include "vfs_handle.h"
#include "vfs_tree.h"

typedef struct {
    vfs_dir_t           d_dir;
    vfs_dirent_t        d_entry;                /* Current entry */
    unsigned            d_fence1;
    unsigned            d_fileno;
    struct vfs_node *   d_cwd;
    struct vfs_node *   d_cursor;
    unsigned            d_fence2;
} vfs_tree_dir_t;

static int              vfs_tree_vlookup(struct vfs_lookup *lk);
static int              vfs_tree_vdelete(struct vfs_mount *vmount, struct vfs_node *vnode);

static int              vfs_tree_vaccess(struct vfs_mount *vmount, const char *path, int what);
static int              vfs_tree_vstat(struct vfs_mount *vmount, const char *path, struct stat *sb);
static int              vfs_tree_vlstat(struct vfs_mount *vmount, const char *path, struct stat *sb);

static int              vfs_tree_vopen(struct vfs_mount *vmount, const char *path, int mode, int mask);
static int              vfs_tree_vclose(struct vfs_handle *vhandle);
static int              vfs_tree_vread(struct vfs_handle *vhandle, void *buffer, unsigned length);
static int              vfs_tree_vwrite(struct vfs_handle *vhandle, const void *buffer, unsigned length);
static int              vfs_tree_vseek(struct vfs_handle *vhandle, off_t offset, int whence);
static off_t            vfs_tree_vtell(struct vfs_handle *vhandle);
static int              vfs_tree_vfstat(struct vfs_handle *vhandle, struct stat *sb);
static int              vfs_tree_vioctl(struct vfs_handle *vhandle, int op, void *data);

static int              vfs_tree_vmkdir(struct vfs_mount *vmount, const char *path, mode_t mode);
static int              vfs_tree_vrmdir(struct vfs_mount *vmount, const char *path);
static int              vfs_tree_vchdir(struct vfs_mount *vmount, const char *path);
static vfs_dir_t *      vfs_tree_vopendir(struct vfs_mount *vmount, const char *path);
static vfs_dirent_t *   vfs_tree_vreaddir(struct vfs_handle *vhandle);
static int              vfs_tree_vclosedir(struct vfs_handle *vhandle);

static int              vfs_tree_vchmod(struct vfs_mount *vmount, const char *path, int mode);
static int              vfs_tree_vchown(struct vfs_mount *vmount, const char *path, int owner, int group);
static int              vfs_tree_vlink(struct vfs_mount *vmount, const char *path1, const char *path2);
static int              vfs_tree_vunlink(struct vfs_mount *vmount, const char *path);
static int              vfs_tree_vremove(struct vfs_mount *vmount, const char *path);
static int              vfs_tree_vrename(struct vfs_mount *vmount, const char *oldname, const char *newname);

static int              writecheck(vfs_tree_t *tree, struct vfs_node *node);

static int              vop_create(vfs_tree_t *tree, struct vfs_node *parent, const char *name, int mask, struct vfs_node **node, int *fd);
static int              vop_localget(vfs_tree_t *tree, struct vfs_node *node, int mode, int mask, int *fd);
static int              vop_access(vfs_tree_t *tree, struct vfs_node *node, int what);
static int              vop_open(vfs_tree_t *tree, struct vfs_node *node, int mode, int mask, int *fd);

static TAILQ_HEAD(treeq_t, vfs_tree)            /* trees */
                        x_trees;


void
vfs_tree_start(void)
{
    TAILQ_INIT(&x_trees);
}


struct vfs_tree *
vfs_tree_new(struct vfs_mount *mount, struct vfs_treevops *vops)
{
    struct vfs_tree *tree;
    struct vfs_node *node;

    if (NULL != (tree = chk_alloc(sizeof(struct vfs_tree)))) {
        vfs_tree_init(tree, mount, vops);
        node = vfs_tree_add(tree, NULL, VNODE_DIR, ".", 1);
        node->v_st.st_mode |= 0755;             /* rwxr-xr-x */
        tree->t_root = node;
    }
    return tree;
}


void
vfs_tree_init(struct vfs_tree *tree, struct vfs_mount *vmount, struct vfs_treevops *vops)
{
    assert(tree);
    assert(vmount);
    assert(NULL == vmount->mt_tree);

    memset(tree, 0, sizeof(struct vfs_tree));
    tree->t_magic = VTREE_MAGIC;
    tree->t_mount = vmount;
    if (vops) {
        tree->t_vops = *vops;
    }
    vfs_cache_init(&tree->t_cache, 0);
    TAILQ_INSERT_TAIL(&x_trees, tree, t_nodelist);
    tree->t_nodecount = 0;
    vmount->mt_tree = tree;
}


void
vfs_tree_destroy(struct vfs_tree *tree)
{
    assert(tree);
    assert(VTREE_MAGIC == tree->t_magic);
    assert(tree->t_mount);

    VFS_TRACE(("\tvfs_tree_destroy(%p)\n", tree))
    vfs_node_destroy(tree->t_root);
    vfs_cache_destroy(&tree->t_cache);
    tree->t_root = NULL;

    VFS_TRACE(("\tnodecount:%d\n", tree->t_nodecount))
    assert(0 == tree->t_nodecount);
}


struct vfs_class *
vfs_tree_vops(struct vfs_class *vclass)
{
    vclass->v_impl.i_lookup   = vfs_tree_vlookup;
    vclass->v_impl.i_delete   = vfs_tree_vdelete;

    vclass->v_impl.i_access   = vfs_tree_vaccess;
    vclass->v_impl.i_stat     = vfs_tree_vstat;
    vclass->v_impl.i_lstat    = vfs_tree_vlstat;

    vclass->v_impl.i_open     = vfs_tree_vopen;
    vclass->v_impl.i_close    = vfs_tree_vclose;
    vclass->v_impl.i_read     = vfs_tree_vread;
    vclass->v_impl.i_write    = vfs_tree_vwrite;
    vclass->v_impl.i_seek     = vfs_tree_vseek;
    vclass->v_impl.i_tell     = vfs_tree_vtell;
    vclass->v_impl.i_ioctl    = vfs_tree_vioctl;
    vclass->v_impl.i_fstat    = vfs_tree_vfstat;

    vclass->v_impl.i_opendir  = vfs_tree_vopendir;
    vclass->v_impl.i_readdir  = vfs_tree_vreaddir;
    vclass->v_impl.i_closedir = vfs_tree_vclosedir;

    vclass->v_impl.i_mkdir    = vfs_tree_vmkdir;
    vclass->v_impl.i_rmdir    = vfs_tree_vrmdir;
    vclass->v_impl.i_chdir    = vfs_tree_vchdir;

    vclass->v_impl.i_chmod    = vfs_tree_vchmod;
    vclass->v_impl.i_chown    = vfs_tree_vchown;
    vclass->v_impl.i_link     = vfs_tree_vlink;
    vclass->v_impl.i_unlink   = vfs_tree_vunlink;
    vclass->v_impl.i_remove   = vfs_tree_vremove;
    vclass->v_impl.i_rename   = vfs_tree_vrename;

    return vclass;
}


int
vfs_tree_vop(struct vfs_tree *tree, enum vfs_treevop op, vfs_treevfunc_t func)
{
    struct vfs_treevops *vops = &tree->t_vops;
    vfs_treevfunc_t *t_func = 0;

    switch (op) {
    case VTREE_VOP_ACCESS:    t_func = (vfs_treevfunc_t *)&vops->vop_access; break;
    case VTREE_VOP_CREATE:    t_func = (vfs_treevfunc_t *)&vops->vop_create; break;
    case VTREE_VOP_OPEN:      t_func = (vfs_treevfunc_t *)&vops->vop_open; break;
    case VTREE_VOP_READ:      t_func = (vfs_treevfunc_t *)&vops->vop_read; break;
    case VTREE_VOP_WRITE:     t_func = (vfs_treevfunc_t *)&vops->vop_write; break;
    case VTREE_VOP_TELL:      t_func = (vfs_treevfunc_t *)&vops->vop_tell; break;
    case VTREE_VOP_CLOSE:     t_func = (vfs_treevfunc_t *)&vops->vop_close; break;
    case VTREE_VOP_LOCALGET:  t_func = (vfs_treevfunc_t *)&vops->vop_localget; break;
    case VTREE_VOP_LOCALPUT:  t_func = (vfs_treevfunc_t *)&vops->vop_localput; break;
    }
    if (t_func) {
        *t_func = func;
        return 0;
    }
    return EINVAL;
}


struct vfs_node *
vfs_tree_add(struct vfs_tree *tree, struct vfs_node *parent, unsigned type, const char *name, unsigned namlen)
{
    struct vfs_mount *vmount = tree->t_mount;
    struct vfs_node *node;
    unsigned namhash;

    assert(tree);
    assert(VTREE_MAGIC == tree->t_magic);
    assert(vmount);

    assert(!parent || VNODE_MAGIC == parent->v_magic);
    assert(!parent || VNODE_DIR == parent->v_type);

    assert(name);
    assert(name[0]);

    if (0 == namlen) {
        namlen = strlen(name);                  /* name length */
    }

    VFS_TRACE(("\tvfs_tree_add(%p, %p, %u, %.*s, %u)\n", tree, parent, type, namlen, name, namlen))

    /*
     *  Verify environment
     */
    namhash = vfs_name_hash(name, namlen);
    if (namlen > VFS_MAXNAME) {
        VFS_TRACE(("\t== ENAMETOOLONG (%u)\n", namlen))
        errno = ENAMETOOLONG;                   /* XXX - should we care, verify readdir() implementation */
        return NULL;
    }

    if (NULL != (node = vfs_tree_search(tree, parent, name, namlen, namhash))) {
        VFS_TRACE(("\t== EEXIST\n"))
        errno = EEXIST;                         /* already exists */
        return NULL;
    }

    /*
     *  Create node
     */
    if (NULL == (node = vfs_node_new(type, name, namlen, namhash, parent))) {
        return NULL;
    }
    node->v_owner = tree;
    node->v_mount = vmount;
    ++tree->t_nodecount;

    if (vmount->mt_flags & VMOUNT_FNAMECACHE) {
        if (VNODE_REG == type || VNODE_DIR == type) {
            vfs_cache_link(&tree->t_cache, node, vfs_tree_path(tree, node, FALSE));
        }
    }

    VFS_TRACE(("\t== %p\n", node))
    return node;
}


struct vfs_node *
vfs_tree_push(struct vfs_tree *tree, unsigned type, const char *path)
{
    unsigned pathlen;
    struct vfs_node *node = NULL;

    assert(tree);
    assert(VTREE_MAGIC == tree->t_magic);
    assert(tree->t_mount);
    assert(path && path[0]);

    pathlen = strlen(path);

    VFS_LEVELINC()
    VFS_TRACE(("vfs_tree_push(%u/%s, %s, %u)\n", type, vfs_node_type2str(type), path, pathlen))

    if (VNODE_DIR == type) {
        /*
         *  simple case, just lookup under mktree (auto-creation) enabled
         */
        node = vfs_tree_lookup(tree, path, strlen(path), LK_OMKTREE, 0);

    } else {
        /*
         *  lookup parent directory (auto-create) then push child.
         */
        const char *name = path + pathlen;
        unsigned namlen = 0;

        while (pathlen && !VFS_ISSEP(name[-1])) {
            --name, --pathlen;
            ++namlen;
        }

        assert(namlen);                         /* XXX - check() not assert() */
        if (namlen) {
            node = vfs_tree_lookup(tree, path, pathlen, LK_OLOOKUP, 0);
            assert(node);
            if (node) {
                assert(VNODE_DIR == node->v_type);
                node = vfs_tree_add(tree, node, type, name, namlen);
                assert(node == vfs_tree_lookup(tree, path, strlen(path), LK_OLOOKUP, 0));
            }
        }
    }

    VFS_TRACE(("==> %p\n", node))
    VFS_LEVELDEC()
    return node;
}


/*  Function:           vfs_tree_path
 *      Retrieve the realpath of the specified node and the given tree.
 *
 *  Parameters:
 *      tree -              Associated tree.
 *      node -              Tree node.
 *      mount -             if *true* the mount point is included,
 *                          otherwise excluded.
 *  Returns:
 *      Address of allocated string buffer, caller takes only ownership.
 */
char *
vfs_tree_path(struct vfs_tree *tree, struct vfs_node *node, unsigned mount)
{
    struct vfs_mount *vmount = tree->t_mount;
    unsigned namlen = 0;
    struct vfs_node *t_node;
    unsigned components;
    char *path, *end;

    VFS_LEVELINC()
    VFS_TRACE(("vfs_tree_path(%p,%p,%d)\n", tree, node, mount))

    /*calculate length*/
    for (components = 0, t_node = node; t_node && t_node->v_parent; t_node = t_node->v_parent) {
        if (components++)
            ++namlen;                           /* separator */
        namlen += t_node->v_namlen;             /* component length */
    }

    if (namlen + vmount->mt_mntlen > VFS_MAXPATH) {
        errno = ENAMETOOLONG;                   /* limits */
        return NULL;
    }

    if (mount) {
        namlen += vmount->mt_mntlen;
    }

    /*allocate working storage and build*/
    if (NULL == (path = chk_alloc(namlen + 1))) {
        return NULL;
    }

    end = path + namlen;                        /* end cursor */
    *end = 0;                                   /* terminate buffer */

    for (components = 0, t_node = node; t_node && t_node->v_parent; t_node = t_node->v_parent) {
        if (components++)
            *--end = VFS_PATHSEP;               /* separator */
        end -= t_node->v_namlen;                /* component length */
        memcpy(end, (const char *)t_node->v_name, t_node->v_namlen);
    }

    if (mount) {
        end -= vmount->mt_mntlen;
        memcpy(end, (const char *)vmount->mt_mount, vmount->mt_mntlen);
    }

    VFS_TRACE(("==> len:%d, %p, %s=>%s\n", namlen, path, vmount->mt_mount, path))
    VFS_LEVELDEC()

    assert(end == path);
    return path;
}


struct vfs_node *
vfs_tree_lookup(struct vfs_tree *tree,
    const char *path, unsigned pathlen, unsigned op, unsigned flags)
{
    struct vfs_node *node;
    int error = 0;

    node = vfs_tree_lookup2(tree, path, pathlen, op, flags, &error);
    if (0 == node && error) {
        errno = error;
    }
    return node;
}


struct vfs_node *
vfs_tree_lookup2(struct vfs_tree *tree,
    const char *path, unsigned pathlen, unsigned op, unsigned flags, int *error)
{
    struct vfs_mount *vmount = tree->t_mount;
    vfs_lookup_t lookup;
    struct vfs_node *node;

    VFS_LEVELINC()
    VFS_TRACE(("vfs_tree_lookup2(%*s)\n", pathlen, path))

    if (NULL == (node = vfs_cache_lookup(&tree->t_cache, path, pathlen))) {
        node = vfs_lookup(vmount, path, pathlen, op, flags, &lookup);
    }

    if (error) {
        *error = lookup.l_errno;                /* lookup error code, if any */
    }

    VFS_TRACE(("==> %p (%d)\n", node, lookup.l_errno))
    VFS_LEVELDEC()

    return node;
}


struct vfs_node *
vfs_tree_lookup3(struct vfs_tree *tree,
    const char *path, unsigned pathlen, unsigned op, unsigned flags, vfs_lookup_t *lk)
{
    struct vfs_mount *vmount = tree->t_mount;

    return vfs_lookup(vmount, path, pathlen, op, flags, lk);
}


struct vfs_node *
vfs_tree_search(struct vfs_tree *tree, struct vfs_node *parent, const char *name, unsigned namlen, unsigned namhash)
{
    assert(tree);
    assert(VTREE_MAGIC == tree->t_magic);
    assert(name && name[0]);

    if (parent && parent->v_childcount) {
        struct vfs_node *node = NULL;
        unsigned count = 0;

        assert(VNODE_DIR == parent->v_type);

        if (0 == namlen) {
            namlen = strlen(name);                  /* name length */
            namhash = vfs_name_hash(name, namlen);
        } else {
            assert(namhash == vfs_name_hash(name, namlen));
        }

        TAILQ_FOREACH(node, &parent->v_childlist, v_listnode) {
            if (node->v_namhash == namhash) {
                if (0 == vfs_node_namecmp(node, name, namlen)) {
                    return node;
                }
            }
            ++count;
        }
        assert(count == parent->v_childcount);
    }
    return NULL;
}


/*  Function:           vfs_tree_vlookup
 *       Virtual lookup operation, called from vfs_lookup() operations.
 *
 *  Parameters:
 *      lk -                Lookup request.
 *
 *  Returns:
 *       0 on success, otherwise non-zero manifest error constant.
 */
int
vfs_tree_vlookup(struct vfs_lookup *lk)
{
    struct vfs_mount *vmount = lk->l_mount;
    struct vfs_tree *tree = vmount->mt_tree;
    struct vfs_node *parent = lk->l_node;
    const char *name = lk->l_name;
    unsigned namlen = lk->l_namlen;
    struct vfs_node *node;

    assert(tree);

    VFS_TRACE(("\ttree_vlookup(%p)\n", lk))

    /*
     *  XXX - test access permissions/or can we shall assume??
     */
    if (NULL != (node = vfs_tree_search(tree, parent, name, namlen, lk->l_namhash))) {
        lk->l_parent = parent;
        lk->l_node = node;
        VFS_TRACE(("\t== found (%p)\n", node))
        return 0;
    }

    /*
     *  XXX - seems like a hack !!
     *
     *      main issue is that create node shall not have the correct attributes,
     *      in other words the permissions are screwed, as such it is asumed the caller shall
     *      populate the v_stat information on completion.
     */
    if (LK_OMKTREE == lk->l_op) {
        lk->l_node = vfs_tree_add(tree, parent, VNODE_DIR, name, namlen);
        VFS_TRACE(("\t== mktree (%p)\n", lk->l_node))
        return 0;
    }
    return EOPNOTSUPP;
}


/*  Function:           vfs_tree_vdelete
 *      Virtual delete operaiton, called during vnode delete operations.
 *
 *  Parameters:
 *      vmount -            Associated mount-point.
 *      node -              Node object.
 *
 *  Returns:
 *       0 on success, otherwise non-zero manifest error constant.
 */
int
vfs_tree_vdelete(struct vfs_mount *vmount, struct vfs_node *node)
{
    struct vfs_tree *tree = vmount->mt_tree;

    assert(tree);
    VFS_TRACE(("\ttree_vdelete(%p)\n", node))
    vfs_cache_unlink(node);
    --tree->t_nodecount;
    return 0;
}


int
vfs_tree_vaccess(struct vfs_mount *vmount, const char *path, int what)
{
    VFS_TRACE(("\ttree_vaccess(%s,%s)\n", vmount->mt_mount, path))
    __UNUSED(what);
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vstat(struct vfs_mount *vmount, const char *path, struct stat *sb)
{
    struct vfs_tree *tree = vmount->mt_tree;
    struct vfs_node *node;

    VFS_TRACE(("\ttree_vstat(%s,%s)\n", vmount->mt_mount, path))
    node = vfs_tree_lookup(tree, path, strlen(path), LK_OLOOKUP, LK_FFOLLOW);
    if (node) {
        *sb = node->v_st;
        return 0;
    }
    errno = ENOENT;
    return -1;
}


int
vfs_tree_vlstat(struct vfs_mount *vmount, const char *path, struct stat *sb)
{
    struct vfs_tree *tree = vmount->mt_tree;
    struct vfs_node *node;

    VFS_TRACE(("\ttree_vlstat(%s,%s)\n", vmount->mt_mount, path))
    node = vfs_tree_lookup(tree, path, strlen(path), LK_OLOOKUP, 0);
    if (node) {
        *sb = node->v_st;
        return 0;
    }
    errno = ENOENT;
    return -1;
}


int
vfs_tree_vchmod(struct vfs_mount *vmount, const char *path, int mode)
{
    VFS_TRACE(("\ttree_vchmod(%s,%s)\n", vmount->mt_mount, path))
    __UNUSED(mode);
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vchown(struct vfs_mount *vmount, const char *path, int owner, int group)
{
    VFS_TRACE(("\ttree_vchown(%s,%s)\n", vmount->mt_mount, path))
    __UNUSED(owner);
    __UNUSED(group);
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vmkdir(struct vfs_mount *vmount, const char *path, mode_t mode)
{
    VFS_TRACE(("\ttree_vmkdir(%s,%s,0%04o)\n", vmount->mt_mount, path, mode))
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vrmdir(struct vfs_mount *vmount, const char *path)
{
    VFS_TRACE(("\ttree_vrmdir(%s,%s)\n", vmount->mt_mount, path))
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vchdir(struct vfs_mount *vmount, const char *path)
{
    struct vfs_tree *tree = vmount->mt_tree;
    struct vfs_node *node;

    VFS_TRACE(("\ttree_vchdir(%s,%s)\n", vmount->mt_mount, path))
    node = vfs_tree_lookup(tree, path, strlen(path), LK_OLOOKUP, LK_FFOLLOW);
    if (node) {
        if (VNODE_DIR == node->v_type) {
            /*
             *  XXX - need sys_realpath() function
             */
            vfscwd_set2(vmount->mt_mount, path /*, TODO - node*/);
            return 0;
        }
        errno = ENOTDIR;
    }
    errno = EOPNOTSUPP;
    return -1;
}


vfs_dir_t *
vfs_tree_vopendir(struct vfs_mount *vmount, const char *path)
{
    struct vfs_tree *tree = vmount->mt_tree;
    vfs_node_t *node;

    VFS_TRACE(("\ttree_vopendir(%s,%s)\n", vmount->mt_mount, path))
    node = vfs_tree_lookup(tree, path, strlen(path), LK_OLOOKUP, 0);
    if (node) {
        if (VNODE_DIR == node->v_type) {
            struct vfs_handle *vhandle =
                        vfs_handle_new(vmount, sizeof(vfs_tree_dir_t));

            if (vhandle) {
                vfs_tree_dir_t *dir = (vfs_tree_dir_t *)(vhandle + 1);
                vfs_dir_t *vdir = &dir->d_dir;

                vdir->d_magic = VDIR_MAGIC;
                vdir->d_handle = vhandle->h_handle;
                dir->d_fileno = 0;
                dir->d_cwd = node;
                dir->d_cursor = NULL;
                return vdir;
            }
        } else {
            errno = ENOTDIR;
        }
    }
    return NULL;
}


vfs_dirent_t *
vfs_tree_vreaddir(struct vfs_handle *vhandle)
{
    vfs_tree_dir_t *dir = (vfs_tree_dir_t *)(vhandle + 1);
    unsigned fd = dir->d_fileno++;
    const char *name = NULL;
    vfs_node_t *node;

    if (0 == fd) {
        name = ".";
        node = dir->d_cwd;

    } else if (1 == fd) {
        name = "..";
        node = dir->d_cwd->v_parent;             /* maybe NULL */

    } else {
        if (2 == fd) {
            node = vfs_node_first(dir->d_cwd);
        } else if (NULL != (node = dir->d_cursor)) {
            node = vfs_node_next(node);
        }
        if (NULL != (dir->d_cursor = node)) {
            name = node->v_name;
        }
    }

    if (name) {
        vfs_dirent_t *vdirent = &dir->d_entry;

        vfs_dirent_init(vdirent, fd, name, strlen(name), node ? &node->v_st : NULL);
        VFS_TRACE(("\ttree_vreaddir(%u, %s) : %p\n", fd, name, vdirent))
        return vdirent;
    }

    VFS_TRACE(("\ttree_vreaddir() : NULL\n"))
    return NULL;
}


int
vfs_tree_vclosedir(struct vfs_handle *vhandle)
{
    VFS_TRACE(("\ttree_vclosedir(%p)\n", vhandle))
    vfs_handle_delete(vhandle);
    return (0);
}


int
vfs_tree_vlink(struct vfs_mount *vmount, const char *path1, const char *path2)
{
    VFS_TRACE(("\ttree_vlink(%s,%s,%s)\n", vmount->mt_mount, path1, path2))
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vunlink(struct vfs_mount *vmount, const char *path)
{
    VFS_TRACE(("\ttree_vunlink(%s,%s)\n", vmount->mt_mount, path))
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vremove(struct vfs_mount *vmount, const char *path)
{
    VFS_TRACE(("\ttree_vremove(%s,%s)\n", vmount->mt_mount, path))
    errno = EOPNOTSUPP;
    return -1;
}


int
vfs_tree_vrename(struct vfs_mount *vmount, const char *oldname, const char *newname)
{
    VFS_TRACE(("\ttree_vrename(%s,%s=>%s)\n", vmount->mt_mount, oldname, newname))
    errno = EOPNOTSUPP;
    return -1;
}


/*  Function:           vfs_tree_vopen
 *      Virtual open operation, called from open() system calls.
 *
 *  Parameters:
 *      vmount -            Mount point.
 *      path -              Path within mount point.
 *      mode -              Open mode.
 *      mask -              File creation permissions mask.
 *
 *  Returns:
 *      0 on success, otherwise non-zero manifest error constant.
 */
int
vfs_tree_vopen(struct vfs_mount *vmount, const char *path, int mode, int mask)
{
    struct vfs_tree *tree = vmount->mt_tree;
    struct vfs_node *node = NULL;
    unsigned pathlen = strlen(path);
    int rwmode = 0;
    int error = 0;
    int fd = -1;

    assert(tree);

    VFS_LEVELINC()
    VFS_TRACE(("tree_vopen(%s,%s,0x%x,0x%x)\n", vmount->mt_mount, path, mode, mask))

    if (NULL == tree->t_vops.vop_open && NULL == tree->t_vops.vop_localget) {
        error = EOPNOTSUPP;                     /* open not enabled */
        goto bad;
    }

    rwmode = (mode & (O_RDWR|O_WRONLY|O_RDONLY));
    if (O_RDWR != rwmode && O_WRONLY != rwmode && O_RDONLY != rwmode) {
        error = EINVAL;                         /* only one mode allowed */
        goto bad;
    }

    if ((mode & O_TRUNC) && O_RDONLY == rwmode) {
        error = EINVAL;
        goto bad;
    }

    if (mode & O_CREAT) {
        unsigned flags = LK_FNOCROSSMOUNT /*| LK_FLOCKPARENT*/;
        vfs_lookup_t lookup;

#if defined(O_NOFOLLOW) && (O_NOFOLLOW)         /* nonstandard */
        if (0 == (mode & O_EXCL) && 0 == (mode & O_NOFOLLOW)) {
#else
        if (0 == (mode & O_EXCL)) {
#endif
            flags |= LK_FFOLLOW;                /* follow symlinks */
        }
        node = vfs_tree_lookup3(tree, path, pathlen, LK_OCREATE, flags, &lookup);
        if (NULL == node) {
            if ((error = lookup.l_errno) != 0 ||
                    (error = vop_create(tree, lookup.l_parent, lookup.l_cursor, mask, &node, &fd)) != 0) {
                goto bad;
            }
            mode &= ~O_TRUNC;
        } else {
            if (mode & O_EXCL) {
                error = EEXIST;
                goto bad;
            }
            mode &= ~O_CREAT;
        }
    } else {
#if defined(O_NOFOLLOW) && (O_NOFOLLOW)         /* nonstandard */ 
        unsigned flags = ((mode & O_NOFOLLOW) ? 0 : LK_FFOLLOW) |
                            LK_FNOCROSSMOUNT /*| LK_FLOCKLEAF*/;
#else
        unsigned flags = LK_FFOLLOW | LK_FNOCROSSMOUNT /*| LK_FLOCKLEAF*/;
#endif

        node = vfs_tree_lookup2(tree, path, pathlen, LK_OLOOKUP, flags, &error);
        if (NULL == node) {
            goto bad;
        }
    }

    if (VNODE_LNK == node->v_type) {
        error = EMLINK;
        goto bad;
    }

    if (VNODE_DIR == node->v_type) {
        error = EISDIR;                         /* neither read nor write (not posix) */
        goto bad;
    }

    if (VNODE_REG != node->v_type) {
        error = EOPNOTSUPP;                     /* SOCK etc */
        goto bad;
    }

    if ((mode & O_CREAT) == 0) {
        if (rwmode != O_WRONLY) {               /* test read access */
            if ((error = vop_access(tree, node, R_OK)) != 0) {
                goto bad;
            }
        }

        if (rwmode != O_RDONLY) {               /* test write access */
            if ((error = writecheck(tree, node)) != 0 ||
                    (error = vop_access(tree, node, W_OK)) != 0) {
                goto bad;
            }
        }

        if ((error = vop_localget(tree, node, mode, mask, &fd)) != 0) {
            goto bad;
        }
    }

    if (fd < 0) {
        if ((error = vop_open(tree, node, mode, mask, &fd)) != 0) {
            goto bad;
        }
        assert(fd >= 0);
    }
    VFS_TRACE(("== %d\n", fd))
    VFS_LEVELDEC()
    return fd;

bad:;
    assert(error);
    VFS_TRACE(("== -1 (%d/%s)\n", error, strerror(error)))
    VFS_LEVELDEC()

    vmount->mt_errno = error;
    errno = error;                              /* XXX - needs work */
    return -1;
}


int
vfs_tree_vclose(struct vfs_handle *vhandle)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    int ret = 0;                                /* default return */

    if (tree->t_vops.vop_close) {
        ret = (tree->t_vops.vop_close)(tree, vhandle);

    } else {
        if (vhandle->h_ihandle >= 0) {
	    vfsio_close(vhandle->h_ihandle);		
            vhandle->h_ihandle = -1;
        }
    }
    VFS_TRACE(("\ttree_vclose(%p=>%d) = %d\n", vhandle, vhandle->h_ihandle, ret))
    vfs_handle_delete(vhandle);
    return 0;
}


int
vfs_tree_vread(struct vfs_handle *vhandle, void *buffer, unsigned length)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    int ret = 0;                                /* default return */

    if (tree->t_vops.vop_read) {
        ret = (tree->t_vops.vop_read)(tree, vhandle, buffer, length);

    } else {
        if (vhandle->h_ihandle >= 0) {
            ret = vfsio_read(vhandle->h_ihandle, buffer, length);
        }
    }
    VFS_TRACE(("\ttree_vread(%p=>%d,%p,%u) = %d\n", vhandle, vhandle->h_ihandle, buffer, length, ret))
    return ret;
}


int
vfs_tree_vwrite(struct vfs_handle *vhandle, const void *buffer, unsigned length)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    int ret = 0;                                /* default return */

    if (tree->t_vops.vop_write) {
        ret = (tree->t_vops.vop_write)(tree, vhandle, buffer, length);

    } else {
        if (vhandle->h_ihandle >= 0) {
            ret = vfsio_write(vhandle->h_ihandle, buffer, length);
        }
    }
    VFS_TRACE(("\ttree_vwrite(%p=>%d,%p,%u) = %d\n", vhandle, vhandle->h_ihandle, buffer, length, ret))
    return ret;
}


int
vfs_tree_vfstat(struct vfs_handle *vhandle, struct stat *sb)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    int ret;

    if (tree->t_vops.vop_fstat) {
        ret = (tree->t_vops.vop_fstat)(tree, vhandle, sb);

    } else {
        if (vhandle->h_ihandle >= 0) {
            ret = fstat(vhandle->h_ihandle, sb);

        } else {
            errno = EOPNOTSUPP;                 /* XXX - work maybe return -EOPNOTSUPP */
            ret = -1;
        }
    }
    VFS_TRACE(("\ttree_vfstat(%p=>%d,%p) = %d\n", vhandle, vhandle->h_ihandle, sb, ret))
    return ret;
}


int
vfs_tree_vseek(struct vfs_handle *vhandle, off_t offset, int whence)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    int ret;

    if (tree->t_vops.vop_seek) {
        ret = (tree->t_vops.vop_seek)(tree, vhandle, offset, whence);

    } else {
        if (vhandle->h_ihandle >= 0) {
	    ret = vfsio_lseek(vhandle->h_ihandle, offset, whence);

        } else {
            errno = EOPNOTSUPP;
            ret = -1;
        }
    }
    VFS_TRACE(("\ttree_vseek(%p,%d,%d) = %d\n", vhandle, (int)offset, whence, ret))
    return ret;
}


off_t
vfs_tree_vtell(struct vfs_handle *vhandle)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    off_t ret;

    if (tree->t_vops.vop_tell) {
        ret = (tree->t_vops.vop_tell)(tree, vhandle);

    } else if (tree->t_vops.vop_seek) {
        ret = (-1 == (tree->t_vops.vop_seek)(tree, vhandle, 0, 0) ? -1 : 0);

    } else {
        if (vhandle->h_ihandle >= 0) {
	    ret = (-1 == vfsio_lseek(vhandle->h_ihandle, 0, 0) ? -1 : 0);

        } else {
            errno = EOPNOTSUPP;
            ret = -1;
        }
    }
    VFS_TRACE(("\ttree_vtell(%p)\n", vhandle))
    return ret;
}


int
vfs_tree_vioctl(struct vfs_handle *vhandle, int op, void *data)
{
    struct vfs_tree *tree = (struct vfs_tree *)vhandle->h_phandle;
    off_t ret;

    if (tree->t_vops.vop_ioctl) {
        ret = (tree->t_vops.vop_ioctl)(tree, vhandle, op, data);

    } else {
        if (vhandle->h_ihandle >= 0) {
#if defined(WIN32) || !defined(HAVE_IOCTL)
	    errno = EOPNOTSUPP; 
#else
            ret = ioctl(vhandle->h_ihandle, op, data);
#endif
        } else {
            errno = EOPNOTSUPP;
            ret = -1;
        }
    }
    VFS_TRACE(("\ttree_vioctl(%p,%d,%p) : %d\n", vhandle, op, data, (int)ret))
    return ret;
}


/*  Function:           writecheck
 *      Check for write permissions on the specified vnode.
 *
 *  Parameters:
 *      tree -              Tree object.
 *      node -              Node reference.
 *
 *  Returns:
 *      0 if writeable, otherwise associated error code.
 */
static int
writecheck(struct vfs_tree *tree, struct vfs_node *node)
{
    unsigned rdonly = (tree->t_mount->mt_flags & VMOUNT_FRDONLY);

    switch (node->v_type) {
    case VNODE_REG:
    case VNODE_LNK:
        if (rdonly) {
            return EROFS;
        }
        break;
    default:
        return EOPNOTSUPP;
    }
    return (0);
}


/*  Function:           vop_access
 *      Execute the access operator.
 *
 *  Parameters:
 *      tree -              Tree object.
 *      node -              Node reference.
 *      what -              Access to be tested (R_OK, W_OK or X_OK).
 *
 *  Notes:
 *      Default return is success, its assumed later vop's shall validate
 *      permissions if needed.
 */
static int
vop_access(struct vfs_tree *tree, struct vfs_node *node, int what)
{
    int ret = 0;                                /* default return */

    if (tree->t_vops.vop_access)  {
        ret = (tree->t_vops.vop_access)(tree, node, what);

    } else {
        switch (what) {
        case R_OK:
#if !defined(S_IRGRP)				/* FIXME/MINGW */
#define S_IRGRP		0
#define S_IROTH		0
#endif
            if ((node->v_st.st_mode & (S_IRUSR|S_IRGRP|S_IROTH)) == 0) {
                errno = EPERM;
                ret = -1;
            }
            break;
        case W_OK:
#if !defined(S_IWGRP)				/* FIXME/MINGW */
#define S_IWGRP		0
#define S_IWOTH		0
#endif
            if ((node->v_st.st_mode & (S_IWUSR|S_IWGRP|S_IWOTH)) == 0) {
                errno = EPERM;
                ret = -1;
            }
            break;
#if (X_OK != R_OK)
        case X_OK:
#if !defined(S_IXGRP)				/* FIXME/MINGW */
#define S_IXGRP		0
#define S_IXOTH		0
#endif
            if ((node->v_st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) == 0) {
                errno = EPERM;
                ret = -1;
            }
            break;
#endif
        default:
            errno = EINVAL;
            ret = -1;
            break;
        }
    }

    VFS_TRACE(("\tvop_access(%d-%s) = %d\n", what, \
        (what == R_OK ? "r--" : W_OK == what ? "-w-" : X_OK == what ? "--x" : "???"), ret))
    return ret;
}


/*
 *
 *
 *  Notes:
 *      Default return is success, its assumed final open vop's shall
 *      validate permissions etc.
 */
static int
vop_localget(vfs_tree_t *tree, struct vfs_node *node, int mode, int mask, int *fd)
{
    int ret = 0;                                /* default return */

    if (tree->t_vops.vop_localget) {
        ret = (tree->t_vops.vop_localget)(tree, node, mode, mask, fd);
    }
    VFS_TRACE(("\tvop_localget() = %d\n", ret))
    return ret;
}


static int
vop_create(struct vfs_tree *tree, struct vfs_node *parent, const char *name, int mask, struct vfs_node **node, int *fd)
{
    int ret = EOPNOTSUPP;                       /* default return */

    if (tree->t_vops.vop_create) {
        ret = (tree->t_vops.vop_create)(tree, parent, name, mask, node, fd);
    }
    VFS_TRACE(("\tvop_create() = %d\n", ret))
    return ret;
}


static int
vop_open(struct vfs_tree *tree, struct vfs_node *node, int mode, int mask, int *fd)
{
    int ret = EOPNOTSUPP;                       /* default return */

    if (tree->t_vops.vop_open) {
        ret = (tree->t_vops.vop_open)(tree, node, mode, mask, fd);

    } else {
        if (node->v_localname) {
            /*
             *  O_RDONLY|O_WRONLY|O_RDWR
             *  O_TRUNC|O_APPEND|O_SYNC|O_NONBLOCK
             *  O_BINARY|O_TEXT
             *  O_NOINHERIT
             */
            struct vfs_handle *vhandle = NULL;
            int t_fd;

            if ((t_fd = vfsio_open(node->v_localname, mode, mask)) >= 0) {
                if (NULL != (vhandle = vfs_handle_new(tree->t_mount, 0 /*node*/))) {
                    vhandle->h_ihandle = t_fd;
                    vhandle->h_phandle = (void *)tree;
                    vhandle->h_vnode = node;
                    *fd = vhandle->h_handle;
                    ret = 0;

                } else {
		    vfsio_close(t_fd);
                    ret = ENOMEM;
                }
            } else {
                ret = errno;
            }
        }
    }
    VFS_TRACE(("\tvop_open() = %d\n", ret))
    return ret;
}
/*end*/
