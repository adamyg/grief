#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_access_c,"$Id: w32_access.c,v 1.6 2025/06/28 11:07:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win2 access() system calls
 *
 * Copyright (c) 2021 - 2025 Adam Young.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#include "win32_internal.h"
#include "win32_io.h"

#include <unistd.h>

static int W32AccessA(const char *path, int amode);
static int W32AccessW(const wchar_t *path, int amode);


/*
//  NAME
//      access - determine accessibility of a file.
//
//  SYNOPSIS
//      #include <unistd.h>
//
//      int access(const char *path, int amode);
//
//  DESCRIPTION
//      The access() function shall check the file named by the pathname pointed
//      to by the path argument for accessibility according to the bit pattern
//      contained in amode, using the real user ID in place of the effective user
//      ID and the real group ID in place of the effective group ID.
//
//      The value of amode is either the bitwise-inclusive OR of the access
//      permissions to be checked (R_OK, W_OK, X_OK) or the existence test (F_OK).
//
//      If any access permissions are checked, each shall be checked individually,
//      as described in the Base Definitions volume of IEEE Std 1003.1-2001,
//      Chapter 3, Definitions. If the process has appropriate privileges,
//      an implementation may indicate success for X_OK even if none of the
//      execute file permission bits are set.
//
//  RETURN VALUE
//      If the requested access is permitted, access() succeeds and shall
//      return 0; otherwise, -1 shall be returned and errno shall be set to
//      indicate the error.
//
//  ERRORS
//      The access() function shall fail if:
//
//      [EACCES]
//          Permission bits of the file mode do not permit the requested access,
//          or search permission is denied on a component of the path prefix.
//
//      [ELOOP]
//          A loop exists in symbolic links encountered during resolution of
//          the path argument.
//
//      [ENAMETOOLONG]
//          The length of the path argument exceeds {PATH_MAX} or a pathname
//          component is longer than {NAME_MAX}.
//
//      [ENOENT]
//          A component of path does not name an existing file or path is an
//          empty string.
//
//      [ENOTDIR]
//          A component of the path prefix is not a directory.
//
//      [EROFS]
//          Write access is requested for a file on a read-only file system.
//
//      The access() function may fail if:
//
//      [EINVAL]
//          The value of the amode argument is invalid.
//
//      [ELOOP]
//          More than {SYMLOOP_MAX} symbolic links were encountered during
//          resolution of the path argument.
//
//      [ENAMETOOLONG]
//          As a result of encountering a symbolic link in resolution of the
//          path argument, the length of the substituted pathname string
//          exceeded {PATH_MAX}.
//
//      [ETXTBSY]
//          Write access is requested for a pure procedure (shared text)
//          file that is being executed.
*/
int
w32_access(const char *path, int amode)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        if (path) {
            wchar_t wpath[WIN32_PATH_MAX];

            if (w32_utf2wc(path, wpath, _countof(wpath)) > 0) {
                return w32_accessW(wpath, amode);
            }
            return -1;
        }
    }
#endif  //UTF8FILENAMES

    return w32_accessA(path, amode);
}


int
w32_accessA(const char *path, int amode)
{
    int ret = -1;                               // success=0, otherwise=-1

    if (NULL == path) {
        errno = EFAULT;

    } else if (!*path) {
        errno = ENOENT;

    } else {
        char symbuf[WIN32_PATH_MAX];

        if (w32_resolvelinkA(path, symbuf, _countof(symbuf), NULL) != NULL) {
            path = symbuf;                      // symlink
        }

        if ((ret = W32AccessA(path, amode)) == -1) {
            if (ENOTDIR == errno || ENOENT == errno) {
                if (path != symbuf) {           // component error, expand embedded shortcut
                    if (w32_expandlinkA(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                        ret = W32AccessA(symbuf, amode);
                   }
                }
            }
        }
    }
    return ret;
}


int
w32_accessW(const wchar_t *path, int amode)
{
    int ret = -1;                               // success=0, otherwise=-1

    if (NULL == path) {
        errno = EFAULT;

    } else if (!*path) {
        errno = ENOENT;

    } else {
        wchar_t symbuf[WIN32_PATH_MAX];

        if (w32_resolvelinkW(path, symbuf, _countof(symbuf), NULL) != NULL) {
            path = symbuf;                      // symlink
        }

        if ((ret = W32AccessW(path, amode)) == -1) {
            if (ENOTDIR == errno || ENOENT == errno) {
                if (path != symbuf) {           // component error, expand embedded shortcut
                    if (w32_expandlinkW(path, symbuf, _countof(symbuf), SHORTCUT_COMPONENT)) {
                        ret = W32AccessW(symbuf, amode);
                   }
                }
            }
        }
    }
    return ret;
}


static int
W32AccessA(const char *path, int amode)
{
    const char *expath;
    int ret;

    if (NULL != (expath = w32_extendedpathA(path))) {
        path = expath;                          // extended abs-path
    }

#undef _access
    ret = _access(path, amode);

    free((void*)expath);
    return ret;
}


static int 
W32AccessW(const wchar_t *path, int amode)
{
    const wchar_t *expath;
    int ret;

    if (NULL != (expath = w32_extendedpathW(path))) {
        path = expath;                          // extended abs-path
    }

#undef _waccess
    ret = _waccess(path, amode);

    free((void*)expath);
    return ret;
}

/*end*/
