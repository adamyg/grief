#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_glob_c,"$Id: vfs_glob.c,v 1.12 2022/03/21 14:27:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_glob.c,v 1.12 2022/03/21 14:27:22 cvsuser Exp $
 * Virtual file system interface - glob implementation.
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

#include <config.h>
#include <eddir.h>

#include <libstr.h>
#include "vfs_glob.h"
#include "vfs_internal.h"

static void  *          gl_opendir(const char *);
static struct glob_dirent *gl_readdir(void *, struct glob_dirent *);
static void             gl_closedir(void *);
static int              gl_lstat(const char *, struct stat *);
static int              gl_stat(const char *, struct stat *);


int
vfs_glob(const char *pattern, int flags, int (*errfunc)(const char *, int), vfs_glob_t *pglob)
{
    glob_t *glob = &pglob->_gl_glob;
    int ret;

    memset(glob, 0, sizeof(glob_t));
    glob->gl_opendir     = gl_opendir;
    glob->gl_readdir     = gl_readdir;
    glob->gl_closedir    = gl_closedir;
    glob->gl_lstat       = gl_lstat;
    glob->gl_stat        = gl_stat;
    if ((ret = bsd_glob(pattern, flags|GLOB_ALTDIRFUNC, errfunc, glob)) >= 0) {
        pglob->gl_pathc  = glob->gl_pathc;
        pglob->gl_matchc = glob->gl_matchc;
        pglob->gl_offs   = glob->gl_offs;
        pglob->gl_pathv  = glob->gl_pathv;
        pglob->gl_statv  = glob->gl_statv;
    }
    return ret;
}


void
vfs_globfree(vfs_glob_t *pglob)
{
    bsd_globfree(&pglob->_gl_glob);
}


static void  *
gl_opendir(const char *path)
{
    return vfs_opendir(path);
}


static struct glob_dirent *
gl_readdir(void *dirp, struct glob_dirent *entry)
{
    vfs_dirent_t *dent;

    if (NULL == (dent = vfs_readdir((vfs_dir_t *)dirp))) {
            return NULL;
    }
    entry->d_name = dent->d_name;
    entry->d_user = dent;
    return entry;
}


static void
gl_closedir(void *dirp)
{
    vfs_closedir(dirp);
}


static int
gl_lstat(const char *path, struct stat *sb)
{
    return vfs_lstat(path, sb);
}


static int
gl_stat(const char *path, struct stat *sb)
{
    return vfs_stat(path, sb);
}
/*end*/
