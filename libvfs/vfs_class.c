#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_class_c,"$Id: vfs_class.c,v 1.17 2025/02/07 03:03:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_class.c,v 1.17 2025/02/07 03:03:23 cvsuser Exp $
 * Virtual file system - utility functions.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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
#include "vfs_class.h"

static TAILQ_HEAD(_vfs_classq, vfs_class)       /* vfs instance queue */
                        x_instanceq;

/*  Function:           vfs_class_start
 *     Class sub-system runtime initialisation.
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_class_start(void)
{
    TAILQ_INIT(&x_instanceq);
}


/*  Function:           vfs_class_shutdown
 *      Class sub-system runtime shutdown.
 *
 *      Unconditional destroy all registered classes, ignoring the root.
 *
 *  Parameters;
 *      none
 *
 *  Returns:
 *      none
 */
void
vfs_class_shutdown(void)
{
    struct vfs_class *vfs;

    if (NULL != (vfs = TAILQ_FIRST(&x_instanceq))) {
        do {
            struct vfs_class *t_vfs = TAILQ_NEXT(vfs, v_node);

            if (vfs->v_prefix[0])  {
                vfs_class_delete(vfs);          /* all but the root class */
            }
            vfs = t_vfs;
        } while (vfs);
    }
}


/*  Function:           vfs_class_new
 *     Allocate and initialise a class object.
 *
 *  Parameters;
 *      desc -              Description.
 *      prefix -            Clas prefix associated virtual file-system
 *                          class, for examples ('ftp' and 'tar').
 *      impl -              Implementation virtual operators.
 *
 *  Returns:
 *      Address of allocate object, otherwise NULL.
 */
struct vfs_class *
vfs_class_new(const char *desc, const char *prefix, struct vfs_implementation *impl)
{
    unsigned desclength = (unsigned)strlen(desc);
    unsigned prefixlength = (unsigned)strlen(prefix);
    struct vfs_class *vfs;                      /* instance */

    assert(desc);
    assert(prefix);

    /*
     *  enforce basic rules,
     *      length >= 3 (stop possible confusion with DOS/WIN32 drive specifications) and must
     *      lead with an alpha and contain only trailing alpha-numeric characters. the only
     *      exception is the root class which is representing using an empty prefix.
     */
    if (*prefix) {
        /*
         *  [a-z][a-z0-9]{2,}
         */
        const char *t_prefix = prefix;

        if (prefixlength < 3 || !isalpha(*t_prefix)) {
            errno = EINVAL;                     /* xxx[xxxx] */
            return NULL;
        }

        while (*++t_prefix) {
            if (! isalnum(*t_prefix)) {
                errno = EINVAL;
                return NULL;
            }
        }
    }

    if (NULL == (vfs = chk_alloc(sizeof(struct vfs_class) + desclength + prefixlength + 2)))  {
        return NULL;                            /* memory allocation error */
    }

    /*
     *  [vfs instance]
     *  [description buffer]
     *  [prefix buffer]
     */
    memset(vfs, 0, sizeof(struct vfs_class));
    vfs->v_magic = VINSTANCE_MAGIC;
    vfs->v_desc = (const char *)(vfs + 1);
    strcpy((char *)vfs->v_desc, desc);
    vfs->v_prefix  = vfs->v_desc + desclength + 1;
    strcpy((char *)vfs->v_prefix, prefix);
    vfs->v_length = prefixlength;
    if (impl) {
        vfs->v_impl = *impl;
    }
    TAILQ_INSERT_TAIL(&x_instanceq, vfs, v_node);

    if (impl && impl->i_initialise) {
        VFS_LEVELINC()
        if (0 != (*impl->i_initialise)(vfs)) {
            /*
             *  initialisation failure\
             *      destroy c;ass
             */
            vfs->v_impl.i_shutdown = NULL;      /* dont call shutdown */
            vfs_class_delete(vfs);
            vfs = NULL;
        }
        VFS_LEVELDEC()
    }

    VFS_TRACE(("\tvfs_class_new('%s', '%s', %p) : %d (%s)\n", \
        desc, prefix, impl, (vfs ? 0 : -1), (vfs ? vfs->v_prefix : "")))
    return vfs;
}


/*  Function:           vfs_class_get
 *     Class sub-system runtime initialisation.
 *
 *  Parameters;
 *      prefix -            Class prefix identifier.
 *      length -            Prefix length. in bytes.
 *
 *  Returns:
 *      Address of class object, otherwise NULL.
 */
struct vfs_class *
vfs_class_get(const char *prefix, unsigned length)
{
    struct vfs_class *vfs = NULL;
    unsigned t_length = 0;

    if (prefix && prefix[0]) {
        if (0 == (t_length = length)) {         /* length missing */
            t_length = (unsigned)strlen(prefix);
        }

        for (vfs = TAILQ_FIRST(&x_instanceq); vfs; vfs = TAILQ_NEXT(vfs, v_node)) {
            assert(VINSTANCE_MAGIC == vfs->v_magic);
            if (vfs->v_length == t_length &&
                    memcmp(vfs->v_prefix, prefix, t_length) == 0) {
                break;                          /* match */
            }
        }
    }

    VFS_TRACE(("\tvfs_class_get('%.*s', %u) : %d (%s)\n", \
        t_length, (prefix ? prefix : ""), length, (vfs ? 0 : -1), (vfs ? vfs->v_prefix : "")))
    return vfs;
}


/*  Function:           vfs_class_delete
 *     Delete the specified vfs-class object.
 *
 *  Parameters;
 *      vfs -               Address of the vfs-class object.
 *
 *  Returns:
 *      nothing
 */
void
vfs_class_delete(struct vfs_class *vfs)
{
    assert(vfs);
    assert(VINSTANCE_MAGIC == vfs->v_magic);

    VFS_TRACE(("\tvfs_class_delete('%s', '%s')\n", vfs->v_desc, vfs->v_prefix))

    assert(0 == vfs->v_references);

    if (vfs->v_impl.i_shutdown) {
        (*vfs->v_impl.i_shutdown)(vfs);
    }

    TAILQ_REMOVE(&x_instanceq, vfs, v_node);
    memset(vfs, 0, sizeof(struct vfs_class));
    chk_free(vfs);
}
/*end*/
