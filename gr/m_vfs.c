#include <edidentifier.h>
__CIDENT_RCSID(gr_m_vfs_c,"$Id: m_vfs.c,v 1.16 2014/10/22 02:33:11 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_vfs.c,v 1.16 2014/10/22 02:33:11 ayoung Exp $
 * Virtual file system interface.
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

#include <editor.h>
#include <errno.h>
#include "../libvfs/vfs.h"
#include "../libvfs/vfs_mount.h"                /* mount flags */
#include "../libvfs/vfs_class.h"

#include "m_vfs.h"                              /* public interface */

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* diagnostic/trace functionality */
#include "eval.h"                               /* get_str */
#include "lisp.h"                               /* lst_...() */
#include "symbol.h"                             /* system_call() */
#include "word.h"                               /* LPUT_...() */


/*  Function;           do_vfs_mount
 *      vfs_mount() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: vfs_mount - Mount a virtual file-system.

        int
        vfs_mount(
            string mountpoint, int flags = 0, string vfsname, 
                [string arguments])
                
    Macro Description:
        The 'vfs_mount()' primitive mounts a virtual file-system.

    Macro Parameters:
        mountpoint - String containing the mount point, being the
            logical path representing the root of the mounted resource.
        
        flags - Integer mount flags.
        
        vfsname - String containing the virtual file-system driver to be
            applied.

        arguments - Optional string arguments to be passed upon the
            underlying vfs implementation; the format and values
            required are specific to the virtual file-system driver
            referenced within 'vfsname'.

    Macro Returns:
        The 'vfs_mount()' primitive returns 0 on success, otherwise -1
        on error and 'errno' contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        vfs_unmount, inq_vfs_mounts
 */
void
do_vfs_mount(void)              /* int (string mountpoint, int flags = 0, string vfsname, [string arguments] */
{
    const char *mountpoint = get_str(1);
    const int flags = (get_xinteger(2, 0) & VMOUNT_FUSERMASK);
    const char *vfsname = get_str(3);
    const char *arguments = get_xstr(4);
    int ret;

    ret = vfs_mount(mountpoint, flags, vfsname, arguments);
    acc_assign_int((accint_t) ret);
    system_call(ret);
}


/*  Function;           do_vfs_unmount
 *      vfs_unmount() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: vfs_unmount - Unmount a virtual file-system.

        int
        vfs_unmount(string mountpoint, int flags = 0)
                
    Macro Description:
        The 'vfs_unmount()' primitive unmounts the specified virtual
        file-system referenced by 'mountpoint'.

    Macro Parameters:
        mountpoint - String containing the mount point, being the
            logical path representing the root of the mounted resource.

        flags - Optional integer unmount flags.

    Macro Returns:
        The 'vfs_unmount()' primitive returns 0 on success, otherwise -1
        on error and 'errno' contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        vfs_mount, inq_vfs_mounts
 */
void
do_vfs_unmount(void)            /* (string mountpoint, int flags = 0) */
{
    const char *mountpoint = get_str(1);
    const int flags = get_xinteger(2, 0);
    int ret;

    ret = vfs_unmount(mountpoint, flags);
    acc_assign_int((accint_t) ret);
    system_call(ret);
}


/*  Function;           inq_vfs_mounts
 *       ing_vfs_mounts() primitive.
 *
 *   Parameters:
 *       none
 *
 *   Returns:
 *       nothing
 *
 *<<GRIEF>>
    Macro: inq_vfs_mounts - Retrieve list of mounts.

        list
        inq_vfs_mounts()
                
    Macro Description:
        The 'inq_vfs_mounts()' primitive retrieves a list of three
        elements describing each of the current mounted virtual
        file-systems.

    Macro Parameters:
        none

    Macro Returns:
        Returns a list of mount points, each mount description contains
        the following elements.

            mountpoint - String containing the mount point, being the
                logical path representing the root of the mounted resource.

            prefix - Prefix string, unique name detailing the underlying
                file-syste type, examples include 'ftp' and 'gzip'.

            flags  - Integer flags.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        vfs_mount, vfs_unmount
 */
void
inq_vfs_mounts(void)            /* list () */
{
    struct vfs_mount *vmount;
    LIST *mountlist, *lp;
    unsigned count, llength, elength;

    /* accumulate details */
    count = 0;
    if (NULL != (vmount = vfs_mount_first())) {
        do {
            ++count;
        } while (NULL != (vmount = vfs_mount_next(vmount)));
    }

    /* allocate storage */
    elength = sizeof_atoms[F_LIST] + (2 * sizeof_atoms[F_RSTR]) + (1 * sizeof_atoms[F_INT]) + sizeof_atoms[F_HALT];
    llength = (count * elength) + sizeof_atoms[F_HALT];
    if (0 == count || NULL == (mountlist = lst_alloc(llength, count))) {
        acc_assign_null();
        return;
    }

    /* build and publish list */
    lp = mountlist;
    vmount = vfs_mount_first();
    do {
        /*
         *  list of elements:
         *      [0] = mount
         *      [1] = prefix
         *      [2] = flags
         *      others?
         */
        *lp = F_LIST;
        LPUT_LEN(lp, (uint16_t)elength);        /* element list */
        lp += sizeof_atoms[F_LIST];
        lp = atom_push_str(lp, vmount->mt_mount);
        lp = atom_push_str(lp, vmount->mt_class->v_prefix);
        lp = atom_push_int(lp, vmount->mt_flags);
        lp = atom_push_halt(lp);
        --count;

    } while (NULL != (vmount = vfs_mount_next(vmount)));
    assert(0 == count);
    atom_push_halt(lp);

    acc_donate_list(mountlist, llength);
}
/*end*/
