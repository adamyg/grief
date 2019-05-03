#ifndef GR_VFS_GLOB_H_INCLUDED
#define GR_VFS_GLOB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_glob_h,"$Id: vfs_glob.h,v 1.8 2019/03/15 23:23:01 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_glob.h,v 1.8 2019/03/15 23:23:01 cvsuser Exp $
 * Virtual file system interface - glob implementation.
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

#include <edtypes.h>
#include <bsd_glob.h>

typedef struct {
        int             gl_pathc;               /* Count of total paths so far. */
        int             gl_matchc;              /* Count of paths matching pattern. */
        int             gl_offs;                /* Reserved at beginning of gl_pathv. */
        char          **gl_pathv;               /* List of paths matching pattern. */
        struct stat   **gl_statv;               /* Stat entries corresponding to gl_pathv */

        glob_t          _gl_glob;               /* Private -- underlying implementation */
} vfs_glob_t;

__CBEGIN_DECLS

extern int              vfs_glob(const char *pattern, int flags, 
                                    int (*errfunc)(const char *epath, int eerrno), vfs_glob_t *pglob);
extern void             vfs_globfree(vfs_glob_t *pglob);

__CEND_DECLS

#endif /*GR_VFS_GLOB_H_INCLUDED*/
