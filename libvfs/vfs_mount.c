#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_mount_c,"$Id: vfs_mount.c,v 1.16 2020/06/03 15:37:47 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_mount.c,v 1.16 2020/06/03 15:37:47 cvsuser Exp $
 * Virtual file system interface --- mount table management.
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

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"

static TAILQ_HEAD(_vfs_mountq, vfs_mount)       /* vfs mount queue */
                        x_mountq;

static unsigned         x_mountseq;             /* edit sequence for mount queue */

static struct vfs_mount *automount(char *buffer, char **sepp);

static int              canonicalize(const char *fname, char *path, int len);

static char *           seperator_next(char *path);
static char *           seperator_prev(const char *buffer, char *cursor);

/*FIXME*/
extern char *           file_canonicalize(const char *file, char *path, int len);
extern int              file_cmp(const char *f1, const char *f2);
extern int              file_ncmp(const char *f1, const char *f2, int len);


/*  Function:           vfs_mount_start
 *     Mount sub-system runtime initialisation.
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_mount_start(void)
{
    TAILQ_INIT(&x_mountq);
}


/*  Function:           vfs_mount_shutdown
 *      Mount sub-system runtime shutdown.
 *
 *      Unconditional unmount all virtual file-system, ignoring the root.
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_mount_shutdown(void)
{
    struct vfs_mount *vmount;

    VFS_LEVELINC()
    VFS_TRACE(("vfs_mount_shutdown\n"))

    if (NULL != (vmount = vfs_mount_first())) {
        do {
            struct vfs_mount *t_vmount = vfs_mount_next(vmount);

            assert(VMOUNT_MAGIC == vmount->mt_magic);
            assert(vmount->mt_class);

            if (vmount != vfs_root()) {
                /*
                 *  unmount all but the root filesystem
                 */
                vfs_vop_unmount(vmount);
                vfs_mount_delete(vmount);
            }
            vmount = t_vmount;

        } while (vmount);
    }

    VFS_TRACE(("== shutdown complete\n"))
    VFS_LEVELDEC()
}


/*  Function:           vfs_mount_new
 *     Allocate and initialise a mount object.
 *
 *  Parameters;
 *      mountpoint -        Address of the mount point object.
 *      vfs -               Associated virtual file-system class.
 *      flags -             Mount flags.
 *
 *  Returns:
 *      Address of allocate object, otherwise NULL.
 */
struct vfs_mount *
vfs_mount_new(struct vfs_mount *parent,
    const char *mountpoint, struct vfs_class *vfs, unsigned flags, const char *argument)
{
    unsigned mntlen = strlen(mountpoint);
    int appendsep = FALSE;
    struct vfs_mount *vmount;
    char *t_path = NULL;
    const char *name;
    unsigned idx, namlen = 0;

    assert(mountpoint);
    assert(mntlen);

    VFS_LEVELINC()
    VFS_TRACE(("vfs_mount_new(%s, len:%u, flags:0x%x)\n", mountpoint, mntlen, flags))

    /* pre-existing */
    if (vfsio_access(mountpoint, R_OK) != 0) {
        errno = ENOENT;                         /* path doest exist */
        goto bad;
    }

    if (vfs_mount_get(mountpoint, mntlen)) {
        errno = EEXIST;                         /* already mounted */
        goto bad;
    }

    if (! VFS_ISSEP(mountpoint[mntlen - 1])) {
        appendsep = TRUE;                       /* mountpoints should always end in a seperator */
        ++mntlen;
    }

    if (NULL == (t_path = chk_alloc(mntlen + 1))) {
        goto bad;
    }

    for (idx = 0; idx < mntlen; ++idx) {
        t_path[idx] = mountpoint[idx];
        if (VFS_ISSEP(t_path[idx])) {
            t_path[idx] = VFS_PATHSEP;
        }
    }
    if (appendsep) {                            /* append missing seperator */
        t_path[mntlen-1] = VFS_PATHSEP;
    }
    t_path[mntlen] = 0;

    /* split mountpoint into dir and name */
    if ((flags & VMOUNT_FROOT) && 1 == mntlen) {
        name = "";                              /* root directory */

    } else {
        for (name = t_path + (mntlen - 1); name > t_path && !VFS_ISSEP(name[-1]); --name) {
            ++namlen;
        }
        if (0 == namlen || name <= t_path)  {
            errno = EINVAL;
            goto bad;
        }
    }

    VFS_TRACE(("\tpath:%s, name:%.*s (%u)\n", t_path, namlen, name, namlen))

    /* create */
    if (NULL == (vmount =
            chk_alloc(sizeof(struct vfs_mount) + mntlen + 1))) {
        goto bad;
    }
    memset(vmount, 0, sizeof(struct vfs_mount));
    vmount->mt_magic = VMOUNT_MAGIC;
    vmount->mt_mntlen = mntlen;
    vmount->mt_mount = (const char *)(vmount + 1);
    strcpy((char *)vmount->mt_mount, t_path);
    vmount->mt_mnthash = vfs_name_hash(vmount->mt_mount, mntlen);
    vmount->mt_baselen = mntlen - (namlen + 1);
    vmount->mt_basehash = vfs_name_hash(vmount->mt_mount, vmount->mt_baselen);
    vmount->mt_namlen = namlen;
    vmount->mt_name = vmount->mt_mount + (mntlen - (namlen + 1));
    vmount->mt_namhash = vfs_name_hash(vmount->mt_name, namlen);
    vmount->mt_parent = parent;
    vmount->mt_flags = flags;
    vmount->mt_class = vfs;
    TAILQ_INIT(&vmount->mt_fdhandleq);

    TAILQ_INSERT_HEAD(&x_mountq, vmount, mt_node);
    ++x_mountseq;                               /* edit sequence for mount queue */

    ++vfs->v_references;                        /* XXX - vfs_class_link() */

    chk_free(t_path);

    if (0 != vfs_vop_mount(vmount, argument)) {
        vfs_mount_delete(vmount);
        goto bad;
    }

    VFS_TRACE(("== (%s, base:%.*s, name:%.*s, flags:0x%x, hash:0x%x, length:%u) : %p\n",
        vmount->mt_mount, vmount->mt_baselen, vmount->mt_mount, vmount->mt_namlen, vmount->mt_name,
            vmount->mt_flags, vmount->mt_mnthash, mntlen, vmount))
    VFS_LEVELDEC()
    return vmount;

bad:
    VFS_TRACE(("== %s : %s (%d)\n", mountpoint, strerror(errno), errno))
    VFS_LEVELDEC()
    return NULL;
}


void
vfs_mount_flagset(struct vfs_mount *vmount, unsigned flags)
{
    vmount->mt_flags |= flags;
}


/*  Function:           vfs_mount_delete
 *     Delete the specified mount point object.
 *
 *  Parameters;
 *      vmount -            Address of the mount point object.
 *
 *  Returns:
 *      nothing
 */
void
vfs_mount_delete(struct vfs_mount *vmount)
{
    struct vfs_class *vfs = vmount->mt_class;

    VFS_TRACE(("\tvfs_mount_delete(%s, flags:0x%x, hash:0x%x)\n",
        vmount->mt_mount, vmount->mt_flags, vmount->mt_mnthash))

    assert(VMOUNT_MAGIC == vmount->mt_magic);
    assert(0 == vmount->mt_references);
    TAILQ_REMOVE(&x_mountq, vmount, mt_node);
    ++x_mountseq;                               /* edit sequence for mount queue */

    assert(vfs->v_references >= 1);             /* xxx- - vfs_class_unlink() */
    --vfs->v_references;

    memset(vmount, 0, sizeof(struct vfs_mount));
    chk_free((void *)vmount);
}


/*  Function:           vfs_mount_get
 *      Retrieve the mount point object associated with the given
 *      'mountpoint'.
 *
 *  Parameters;
 *      mountpoint -        Address of the mount point path.
 *      mntlsn  -           Optional length is bytes, if zero when derived from 'mountpoint'.
 *
 *  Returns:
 *      Address of the mount point object, otherwise 0.
 */
struct vfs_mount *
vfs_mount_get(const char *mountpoint, unsigned mntlen)
{
    struct vfs_mount *vmount = NULL;
    unsigned hash;

    assert(mountpoint && mountpoint[0]);

    if (0 == mntlen) {
        mntlen = strlen(mountpoint);
    }
    hash = vfs_name_hash(mountpoint, mntlen);

    if (mntlen && VFS_ISSEP(mountpoint[mntlen-1])) {
        /*
         *  trailing seperator supplied
         */
        for (vmount = vfs_mount_first(); vmount; vmount = vfs_mount_next(vmount)) {
            if (hash == vmount->mt_mnthash)
                if (mntlen == vmount->mt_mntlen &&
                        0 == file_ncmp(vmount->mt_mount, mountpoint, mntlen)) {
                    break;                      /* match */
                }
        }

    } else {
        /*
         *  trailing seperator not supplied
         */
        for (vmount = vfs_mount_first(); vmount; vmount = vfs_mount_next(vmount)) {
            if (hash == vmount->mt_mnthash)
                if (mntlen == vmount->mt_mntlen-1 &&
                        (0 == mntlen || 0 == file_ncmp(vmount->mt_mount, mountpoint, mntlen))) {
                    break;                      /* match */
                }
        }
    }

    VFS_TRACE(("\tvfs_mount_get(%u, %.*s) : %p\n", \
        mntlen, mntlen, mountpoint, vmount))
    return vmount;
}


/*  Function:           vfs_mount_list
 *      Retrieve the list of mount-point asssociated with the specified
 *      path, upto the upper limit 'mntmax'.
 *
 *  Parameters:
 *      path -              Mount point base path.
 *      flags -             FLags, if any which must be matched.
 *      mounts -            Storage of matched mounts.
 *      mntmax -            Upper limit of mounts within storag.
 *
 *  Returns:
 *      Resulting search count limited to 'mntmax'.
 */
unsigned
vfs_mount_list(const char *path, unsigned pathlen, unsigned flags, struct vfs_mount *mounts[], unsigned mntmax)
{
    struct vfs_mount *vmount = NULL;            /* current mount entry */
    unsigned mntcnt = 0;                        /* total matching mountpoints */

    assert(path);
    assert(mounts);
    assert(mntmax);

    VFS_TRACE(("\tvfs_mount_list(%.*s, %u)\n", pathlen, path, mntmax))

    if (pathlen && VFS_ISSEP(path[pathlen-1])) {
        /*
         *  trailing seperator supplied
         */
        for (vmount = vfs_mount_first(); vmount && mntcnt < mntmax; vmount = vfs_mount_next(vmount)) {
            if (pathlen == vmount->mt_baselen &&
                    file_ncmp(vmount->mt_mount, path, pathlen) == 0) {
                if (0 == flags || (vmount->mt_flags & flags)) {
                    /*
                     *  Matched
                     */
                    VFS_TRACE(("\t%.*s\n", vmount->mt_namlen, vmount->mt_name))
                    mounts[ mntcnt++ ] = vmount;
                }
                break;
            }
        }

    } else {
        /*
         *  trailing seperator not supplied
         */
        for (vmount = vfs_mount_first(); vmount; vmount = vfs_mount_next(vmount)) {
            if (pathlen == vmount->mt_baselen-1 &&
                        (0 == pathlen || file_ncmp(vmount->mt_mount, path, pathlen) == 0)) {
                if (0 == flags || (vmount->mt_flags & flags)) {
                    /*
                     *  Matched
                     */
                    VFS_TRACE(("\t%.*s\n", vmount->mt_namlen, vmount->mt_name))
                    mounts[ mntcnt++ ] = vmount;
                    break;
                }
            }
        }
    }

    VFS_TRACE(("\t== %u\n", mntcnt))
    return mntcnt;
}


/*  Function:           vfs_mount_lookup
 *      xxx
 *
 *  Parameters;
 *      path -              xxx
 *      buffer -            xxx
 *      length -            xxx
 *      fname -             xxx
 *
 *  Returns:
 *      Address of the mount point object, otherwise NULL.
 */
struct vfs_mount *
vfs_mount_lookup(const char *path, char *buffer, int length, char **fname)
{
    struct vfs_mount *vmount = NULL;
    char *cursor;

    assert(NULL != path);
    assert(NULL != buffer);
    assert(length >= 128);

    /*
     *  canonicalize and search against mount table
     */
    canonicalize(path, buffer, length);         /* remove subdir etc */
    length = strlen(buffer);

    VFS_TRACE(("\tvfs_mount_lookup('%s' -> '%s')\n", path, buffer))
    cursor = buffer + length - 1;               /* cursor */
    assert(0 == cursor[1]);

    if (! VFS_ISSEP(*cursor)) {
        if (NULL != (vmount = vfs_mount_get(buffer, length))) {
            /*
             *  absolute match (with missing trailing delimiter)
             *      append missing terminator
             */
            *++cursor = VFS_PATHSEP;
            *++cursor = 0;
        }
    }

    if (NULL == vmount) {
        while (cursor >= buffer) {
            /*
             *  walk backward removing components
             */
            cursor = seperator_prev(buffer, cursor);
            length = (cursor - buffer) + 1;
            if (NULL != (vmount = vfs_mount_get(buffer, length))) {
                if (0 == (vmount->mt_flags & VMOUNT_FFULLPATH)) {
                    *cursor++ = 0;              /* remove seperating terminator */
                }
                break;
            }

            while (cursor >= buffer) {
                const char ch = *cursor;
                if (!VFS_ISSEP(ch)) {
#if defined(DOSISH)
                    if (':' == ch && (cursor - 1) == buffer) {
                        cursor -= 2;            /* X:/ */
                    }
#endif
                    break;
                }
                --cursor;                       /* skip current delimitor(s) */
            }
        }
        assert((vmount && cursor >= buffer) || (!vmount && cursor < buffer));
    }

    /*
     *  Encoded within path using in-line style interface, for example
     *
     *      /home/user/tarfile.tar#arc/tardir/tarfile.txt
     *
     *  auto-mount if required/supported (for example archive files)
     */
    if (NULL == vmount) {
        vmount = automount(buffer, &cursor);
    }

    /*
     *  No match, return base file-system
     */
    if (NULL == vmount) {
        vmount = vfs_root();
        cursor = NULL;
    }

    /*
     *  Return path/filename
     */
    if (NULL == cursor || (vmount->mt_flags & VMOUNT_FFULLPATH)) {
        cursor = buffer;                        /* full path required */
    }
    if (fname) {
        *fname = cursor;
    }

    VFS_TRACE(("\t== vfs_mount_lookup('%s', %d) : prefix:%s, path:%s, fname:%s\n", \
        path, length, vmount->mt_class->v_prefix, \
            buffer, (fname ? (*fname == buffer ? "--" : *fname) : "")))
    return vmount;
}


/*  Function:           vfs_mount_seq
 *      Retrieve the current mount list edit sequence number which is incremented on each mount
 *      list addition or deletion.
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      Edit sequence number.
 */
unsigned
vfs_mount_seq(void)
{
    return x_mountseq;
}


/*  Function:           automount
 *     AUtomaticly mount a supported/enabled virtual file-system
 *
 *  *Syntax*
 *      <mount-point>#<vfs-prefix>/<path>
 *
 *  Parameters;
 *      buffer -            Mount-point buffer.
 *      sepp -              Address of file-name component within the mount-point.
 *
 *  Returns:
 *      Address of mont object, otherwise NULL.
 */
static struct vfs_mount *
automount(char *buffer, char **sepp)
{
    struct vfs_mount *vmount = NULL;
    char *prefix, *subpath;

    if (NULL != (prefix = strchr(buffer, VFS_PREFIX)) && prefix > buffer) {
        if (NULL != (subpath = seperator_next(prefix + 1)) && subpath > prefix + 1) {
            struct vfs_class *vclass =
                        vfs_class_get(prefix + 1, (subpath - prefix) -1);

            assert(NULL == vfs_mount_get(buffer, prefix - buffer));
            VFS_TRACE(("\td. %.*s:%s\n", (int)((subpath - prefix) - 1), prefix + 1, subpath))

            if (vclass) {
                if (vclass->v_flags & VCLASS_FAUTOMOUNT) {
                    /*
                     *  auto-mount -- UNTESTED
                     */
                    char restore = *prefix;

                    assert(*subpath == VFS_PATHSEP);
                    *prefix = 0;                /* terminate mount point */

                    if (NULL != (vmount = vfs_mount_new(vfs_root(), buffer, vclass, 0, NULL))) {
                        /*
                         *  remove vfs prefix, for example "#tar/"
                         */
                        assert(*subpath == VFS_PATHSEP);
                        if (sepp) {
                            *sepp = subpath;
                        }
                    } else {
                        *prefix = restore;
                    }
                }
            }
        }
    }
    return vmount;
}


static int
canonicalize(const char *path, char *buffer, int length)
{
    file_canonicalize(path, buffer, length);
    return 0;
}


/*  Function:           vfs_mount_first
 *      xxx
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      Address of the first mount point object, otherwise NULL.
 */
struct vfs_mount *
vfs_mount_first(void)
{
    struct vfs_mount *vmount = TAILQ_FIRST(&x_mountq);
    if (vmount) {
        assert(VMOUNT_MAGIC == vmount->mt_magic);
        assert(vmount->mt_mntlen);
    }
    return vmount;
}


/*  Function:           vfs_mount_first
 *      xxx
 *
 *  Parameters;
 *      vmount -            Mount object.
 *
 *  Returns:
 *      Address of the next mount point object, otherwise NULL.
 */
struct vfs_mount *
vfs_mount_next(struct vfs_mount *vmount)
{
    assert(vmount);
    assert(VMOUNT_MAGIC == vmount->mt_magic);
    vmount = TAILQ_NEXT(vmount, mt_node);
    if (vmount) {
        assert(VMOUNT_MAGIC == vmount->mt_magic);
        assert(vmount->mt_mntlen);
    }
    return vmount;
}


static char *
seperator_next(char *path)
{
    char *slash = strchr(path, VFS_PATHSEP);
#if defined(DOSISH)         /* / and \ */
    char *slash2 = strchr(path, VFS_PATHSEP2);

    if (slash2 && slash2 < slash)
        return slash2;
#endif
    return slash;
}


static char *
seperator_prev(const char *buffer, char *cursor)
{
    while (cursor > buffer) {
        const char ch = *cursor;
        if (VFS_ISSEP(ch)) {
            break;
        }
        --cursor;                       /* skip until delimitor */
    }
    while (cursor > buffer) {
        const char ch = cursor[-1];
        if (!VFS_ISSEP(ch)) {
            break;
        }
        --cursor;                       /* consume trailing duplicate delimitor(s) */
    }
    return cursor;
}
/*end*/
