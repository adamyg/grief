#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_util_c,"$Id: vfs_util.c,v 1.19 2020/04/11 20:33:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_util.c,v 1.19 2020/04/11 20:33:12 cvsuser Exp $
 * Virtual file system - utility functions.
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
#include <edtrace.h>                            /* trace interface */
#include <errno.h>

#include <libstr.h>
#include "vfs_internal.h"
#include "vfs_class.h"
#include "vfs_mount.h"
#include "vfs_handle.h"
#include "vfs_tree.h"

#if defined(__MINGW32__)
#define HAVE_WINSOCK2_H_INCLUDED
#include <windows.h>
#endif

static struct vfs_mount *x_root;                /* base instance */

static char             x_cwd[MAX_PATH+1];      /* current working directory */

unsigned                x_vfs_trace = 0;        /* trace level */


/*  Function:           vfs_init
 *      Initialise the virtual file-system interface, registering all required
 *      default filesystem clasees.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
vfs_init(void)
{
    assert(VFS_VOPMAX == sizeof(struct vfs_implementation)/sizeof(void (*)()));
    
    VFS_TRACE(("vfs_init()\n"))
    vfs_tree_start();
    vfs_class_start();
    vfs_mount_start();
    vfs_handle_start();
    x_root = vfs_mount_new(NULL, "/", vfsbase_init(), VMOUNT_FROOT|VMOUNT_FFULLPATH, NULL);
    assert(x_root);
    vfsarc_init();
    vfscurl_init();
    VFS_TRACE(("== vfs_init complete\n"))
}


/*  Function:           vfs_shutdown
 *      Shut-down the virtual file-system interface, closing all handles and unmounting
 *      all interfaces.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
vfs_shutdown(void)
{ 
    VFS_TRACE(("vfs_shutdown()\n"))
 /* x_root = NULL; */                           /* include root */
    vfs_mount_shutdown();                       /* unmount 'all' virtual file-systems */
 /* vfs_handle_shutdown(); */                   /* close all handles */
    vfs_class_shutdown();
    VFS_TRACE(("== vfs_shutdown complete\n"))
}


/*  Function:           vfs_root
 *      Retrieve the root mount-point.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Address of root mount-point.
 */
struct vfs_mount *
vfs_root(void)
{
    return x_root;
}


/*  Function:           vfs_trace
 *      Virtual-filesystem specific diagnostics.
 *
 *  Parameters:
 *      fmt -               Format specification (sprintf style).
 *      ... -               Additional arguments.
 *
 *  Returns:
 *      Address of root mount-point.
 */
void             
vfs_trace(const char *fmt, ...)
{
    va_list ap;

    if (x_vfs_trace) {                          /* ident */
        trace_log("\t%.*s",
            (x_vfs_trace > 10 ? 10 : x_vfs_trace), "\t\t\t\t\t\t\t\t\t\t");
    } else {
        trace_log("\t");
    }
    va_start(ap, fmt);
    trace_logv(fmt, ap);
    va_end(ap);
}


void
vfscwd_set(const char *path)
{
    vfscwd_set2(NULL, path);
}


void
vfscwd_set2(const char *mount, const char *path)
{
    x_cwd[0] = 0;

    if (NULL != path) {
        unsigned length;

        if (mount) {
            strxcat(x_cwd, mount, sizeof(x_cwd));
        }
        strxcat(x_cwd, path, sizeof(x_cwd));
        vfs_fix_slashes(x_cwd);                 /* XXX - should be mount specific */
        length = strlen(x_cwd);
        if (length > 0) {
            while (VFS_PATHSEP == x_cwd[length-1]) {
                x_cwd[--length] = 0;            /* remove trailing seperator(s) */
            }
        }
        VFS_TRACE(("\tvfscwd_set(%s)\n", x_cwd))
    }
}


const char *
vfscwd_get(void)
{
    if (0 == x_cwd[0]) {
#if defined(WIN32)
        w32_getcwd(x_cwd, sizeof(x_cwd));
#else
        if (NULL == getcwd(x_cwd, sizeof(x_cwd))) {
            x_cwd[0] = 0;                       /* XXX - error handling */
        }
#endif
        vfs_fix_slashes(x_cwd);                 /* XXX - should be mount specific */
        vfs_fix_case(x_cwd);                    /* XXX - should be mount specific */
    }
    return x_cwd;
}


/*  Function:           vfs_dirent_init
 *      Initial the specified directory entry.
 *
 *  Parameters:
 *      fileno -            File number (directory index).
 *      name -              Entry name.
 *      namlen -            Name length, in bytes.
 *      sb -                Optional stat buffer.
 *
 *  Returns:
 *      nothing.
 */
void
vfs_dirent_init(struct vfs_dirent *vdirent, int fd, const char *name, unsigned namlen, struct stat *sb)
{
    vdirent->d_magic = VDIRENT_MAGIC;
    vdirent->d_fileno = fd;
    if (namlen >= sizeof(vdirent->d_name))
        namlen = sizeof(vdirent->d_name) - 1;
    memcpy(vdirent->d_name, (const char *)name, namlen);
    vdirent->d_name[namlen] = 0;
    vdirent->d_namlen = (unsigned short)namlen;
    if (sb) {
        vdirent->d_stat  = VDIRECTST_MAGIC;
        vdirent->d_ctime = sb->st_ctime;
        vdirent->d_mtime = sb->st_mtime;
        vdirent->d_atime = sb->st_atime;
        vdirent->d_mode  = sb->st_mode;
        vdirent->d_size  = sb->st_size;
        vdirent->d_uid   = sb->st_uid;
        vdirent->d_gid   = sb->st_gid;

    } else {
        vdirent->d_stat  = 0;
        vdirent->d_ctime = -1;
        vdirent->d_mtime = -1;
        vdirent->d_atime = -1;
        vdirent->d_mode  = (unsigned)-1;
        vdirent->d_size  = (unsigned)-1;
        vdirent->d_uid   = (unsigned)-1;
        vdirent->d_gid   = (unsigned)-1;
    }
}


/*  Function:           vfs_tempnam
 *      Return a tempory filename
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Address of dynamic string buffer containing full-path
 *      to the tempory filename,
 */
extern const char *sysinfo_tmpdir();            /* FIXME */

const char *
vfs_tempnam(int *fd)
{
    static unsigned seqno = 1;
    const char *tmpdir = sysinfo_tmpdir();      /* TODO/FIXME - vfs specific */
#if defined(HAVE_LONG_LONG_INT) && !defined(__MINGW32__)
    unsigned long long salt = ((unsigned long long) time(NULL) << 20) + ++seqno;
#else
    unsigned long salt = ((unsigned long)time(NULL) << 16) + ++seqno;
#endif
    char t_name[VFS_MAXPATH+1];
    int t_fd;

    assert(tmpdir);
#if defined(HAVE_LONG_LONG_INT) && !defined(__MINGW32__)
    sxprintf(t_name, sizeof(t_name), "%s%cvfs%lluxXXXXX", tmpdir, VFS_PATHSEP, salt);
#else
    sxprintf(t_name, sizeof(t_name), "%s%cvfs%luxXXXXX", tmpdir, VFS_PATHSEP, salt);
#endif

#if defined(WIN32)
    if ((t_fd = w32_mkstemp(t_name)) >= 0)
#elif defined(HAVE_MKSTEMP) || \
            defined(linux) || defined(sun) || defined(_AIX)
    if ((t_fd = mkstemp(t_name)) >= 0)
#else
    if (NULL != (tmpdir = mktemp(t_name)) && *tmpdir &&
            (t_fd = open(tmpdir, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
#endif
    {
        *fd = t_fd;
        return chk_salloc(t_name);
    }
    return NULL;
}


/*  Function:           vfs_name_hash
 *      Generate and return a simple hash of the mount point
 *
 *  Parameters;
 *      path -              Address of the mount point buffer.
 *      pathlen -           Name of mount point, in bytes.
 *
 *  Returns:
 *      Hash value.
 */
unsigned         
vfs_name_hash(const char *path, unsigned pathlen)
{
    unsigned hash = (unsigned)-1;
    unsigned char ch;
    
    while (pathlen-- && (ch = (unsigned char)*path++) != 0) {
        if (! VFS_ISSEP(ch)) {                  /* ignore seperators */
            hash = hash * 127 + tolower(ch);    /* and case */
        }
    }
    return hash;
}


char *
vfs_fix_slashes(char *str)
{
#if defined(DOSISH) || defined(WIN32)
    if (str && *str) {
        register char *cp = str;
        for (; *cp; ++cp) {
            if (VFS_ISSEP(*cp)) {
                *cp = VFS_PATHSEP;              /* standardise to the system seperator */
            }
        }
    }
#else
    __UNUSED(str)
#endif
    return str;
}


/*  Function:           vfs_fix_case
 *      Convert the filename case.
 *
 *  Parameters:
 *      str -               String buffer.
 *
 *  Returns:
 *      Original buffer.
 */
#if defined(WIN32)
static int                      IsHPFSFileSystem(const char *directory);
#endif

char *
vfs_fix_case(char *str)
{
#if defined(MONOCASE_FILENAMES)
    if (str && *str) {
        register char *cp;

#if defined(WIN32)
        if (IsHPFSFileSystem(str)) {
            return str;
        }
#endif
        for (cp = str; *cp; ++cp) {
            unsigned char ch = (unsigned char)*cp;
            if (isupper(ch)) {
                *cp = (char)tolower(ch);
            }
        }
    }
#else
    __UNUSED(str)
#endif
    return str;
}


#if defined(WIN32)
static int
IsHPFSFileSystem(const char *directory)
{
#define DISABLE_HARD_ERRORS     SetErrorMode (0)
#define ENABLE_HARD_ERRORS      SetErrorMode (SEM_FAILCRITICALERRORS | \
                                        SEM_NOOPENFILEERRORBOX)

    char     szCurDir[MAX_PATH + 1];
    char     bName[4];
    DWORD    flags;
    DWORD    maxname;
    BOOL     rc;
    unsigned nDrive;
                                                // TODO - should cache results
    if (isalpha(directory[0]) && (directory[1] == ':')) {
        nDrive = toupper (directory[0]) - '@';

    } else {
        GetCurrentDirectoryA(MAX_PATH, szCurDir);
        nDrive = szCurDir[0] - 'A' + 1;
    }

    strcpy(bName, "x:\\");
    bName[0] = (char) (nDrive + '@');

    DISABLE_HARD_ERRORS;
    rc = GetVolumeInformationA(bName, (LPTSTR)NULL, 0,
                (LPDWORD)NULL, &maxname, &flags, (LPTSTR)NULL, 0);
    ENABLE_HARD_ERRORS;

    return ((rc) &&                             /* XXX - FS_CASE_IS_PRESERVED */
        (flags & (FS_CASE_SENSITIVE | FS_CASE_IS_PRESERVED))) ? TRUE : FALSE;

#undef DISABLE_HARD_ERRORS
#undef ENABLE_HARD_ERRORS
}
#endif  /*WIN32*/
/*end*/
