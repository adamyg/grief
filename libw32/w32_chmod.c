#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_chmod_c,"$Id: w32_chmod.c,v 1.5 2025/06/28 11:07:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 chmod() system calls.
 *
 * Copyright (c) 2020 - 2025 Adam Young.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif

#include "win32_internal.h"
#include "win32_io.h"

#include <unistd.h>

static int W32ChmodA(const char *path, mode_t mode);
static int W32ChmodW(const wchar_t *path, mode_t mode);

/*
//  NAME
//      chmod
//
//  SYNOPSIS
//      #include <sys/stat.h>
//
//      int chmod(const char *pathname, mode_t mode);
//
//  DESCRIPTION
//      The chmod() and fchmod() system calls change a files mode bits.
//      chmod() changes the mode of the file specified whose pathname is given
//      in pathname, which is dereferenced if it is a symbolic link.
//
//      The file mode consists of the file permission bits plus the set-user-ID,
//      set-group-ID, and sticky bits)  These system calls differ only in how the
//      file is specified.
//
//  ERRORS
//
//      The chmod() function will fail if:
//
//      [EACCES]
//          Search permission is denied on a component of the path prefix.
//      [ELOOP]
//          Too many symbolic links were encountered in resolving path.
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname component is longer than {NAME_MAX}.
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//      [ENOENT]
//          A component of path does not name an existing file or path is an empty string.
//      [EPERM]
//          The effective user ID does not match the owner of the file and the process does not have appropriate privileges.
//      [EROFS]
//          The named file resides on a read-only file system.
//
//      The chmod() function may fail if:
//
//      [EINTR]
//          A signal was caught during execution of the function.
//      [EINVAL]
//          The value of the mode argument is invalid.
//      [ENAMETOOLONG]
//          Pathname resolution of a symbolic link produced an intermediate result whose length exceeds {PATH_MAX}.
*/

LIBW32_API int
w32_chmod(const char *pathname, mode_t mode)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (pathname) {
            wchar_t wpathname[WIN32_PATH_MAX];

            if (w32_utf2wc(pathname, wpathname, _countof(wpathname)) > 0) {
                return w32_chmodW(wpathname, mode);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_chmodA(pathname, mode);
}


LIBW32_API int
w32_chmodA(const char *pathname, mode_t mode)
{
    int ret = -1;                               // success=0, otherwise=-1

    if (NULL == pathname) {
        errno = EFAULT;

    } else if (!*pathname) {
        errno = ENOENT;

    } else {
       char symbuf[WIN32_PATH_MAX];

        if (w32_resolvelinkA(pathname, symbuf, _countof(symbuf), NULL) != NULL) {
            pathname = symbuf;                  // symlink
        }

        if ((ret = W32ChmodA(pathname, mode)) == -1) {
            if (ENOTDIR == errno || ENOENT == errno) {
                if (pathname != symbuf) {       // component error, expand embedded shortcut
                    if (w32_expandlinkA(pathname, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                        ret = W32ChmodA(symbuf, mode);
                    }
                }
            }
        }
    }
    return ret;
}


LIBW32_API int
w32_chmodW(const wchar_t *pathname, mode_t mode)
{
    int ret = -1;                               // success=0, otherwise=-1

    if (NULL == pathname) {
        errno = EFAULT;

    } else if (!*pathname) {
        errno = ENOENT;

    } else {
        wchar_t symbuf[WIN32_PATH_MAX];

        if (w32_resolvelinkW(pathname, symbuf, _countof(symbuf), NULL) != NULL) {
            pathname = symbuf;                  // symlink
        }

        if ((ret = W32ChmodW(pathname, mode)) == -1) {
            if (ENOTDIR == errno || ENOENT == errno) {
                if (pathname != symbuf) {       // component error, expand embedded shortcut
                    if (w32_expandlinkW(pathname, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                        ret = W32ChmodW(symbuf, mode);
                    }
                }
            }
        }
    }
    return ret;
}


static int 
W32ChmodA(const char *pathname, mode_t mode)
{
    const char *expath;
    int ret;

    if (NULL != (expath = w32_extendedpathA(pathname))) {
        pathname = expath;                      // extended abs-path
    }

#undef chmod
    ret = chmod(pathname, mode);

    free((void*)expath);
    return ret;
}


static int
W32ChmodW(const wchar_t *pathname, mode_t mode)
{
    const wchar_t *expath;
    int ret;

    if (NULL != (expath = w32_extendedpathW(pathname))) {
        pathname = expath;                      // extended abs-path
    }

#undef _wchmod
    ret = _wchmod(pathname, mode);

    free((void*)expath);
    return ret;
}

/*end*/
