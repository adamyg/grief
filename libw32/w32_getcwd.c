#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_getcwd_c,"$Id: w32_getcwd.c,v 1.19 2025/06/28 11:07:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 getcwd() implementation
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#include "win32_internal.h"
#include "win32_io.h"

#include <ctype.h>
#include <errno.h>

/*
//  NAME
//
//      getcwd, getcwdd - get the pathname of the current working directory
//
//  SYNOPSIS
//
//      #include <unistd.h>
//
//      char *getcwd(char *buf, size_t size);
//      char *getcwdd(char drive, char *buf, size_t size);
//
//  DESCRIPTION
//
//      The getcwd() function shall place an absolute pathname of the current working
//      directory in the array pointed to by buf, and return buf. The pathname copied to
//      the array shall contain no components that are symbolic links. The size argument is
//      the size in bytes of the character array pointed to by the buf argument. If buf is
//      a null pointer, the behavior of getcwd() is unspecified.
//
//      The getcwdd() function retrieves the absolute pathname of the current working for the
//      specificed drive (A thru Z).
//
//  RETURN VALUE
//
//      Upon successful completion, getcwd() shall return the buf argument. Otherwise,
//      getcwd() shall return a null pointer and set errno to indicate the error. The
//      contents of the array pointed to by buf are then undefined.
//
//  ERRORS
//
//      The getcwd() function shall fail if:
//
//      [EINVAL]
//          The size argument is 0.
//
//      [ERANGE]
//          The size argument is greater than 0, but is smaller than the length of the pathname +1.
//
//      The getcwd() function may fail if:
//
//      [EACCES]
//          Read or search permission was denied for a component of the pathname.
//
//      [ENOMEM]
//          Insufficient storage space is available.
*/

LIBW32_API char *
w32_getcwd(char *path, size_t size)
{
    if (NULL == path || size <= 0) {
#if (__GNU_COMPAT)
        return w32_getdirectory();              // dynamic
#else
        errno = EINVAL;
#endif

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        if (x_w32_vfscwd) {                     // vfs chdir()
            const char *in;
            char *out;

            for (in = x_w32_vfscwd, out = path; *in; ++in) {
                if (--size <= 0) {
                    errno = ENOMEM;
                    break;
                }
                *out++ = *in;
            }
            *out = 0;
            return path;
        }

#if defined(UTF8FILENAMES)
        if (w32_utf8filenames_state()) {
            wchar_t *wpath;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:6255)
#endif
            if (NULL != (wpath = alloca(sizeof(wchar_t) * (size + 1))) &&
                    w32_getcwdW(wpath, size)) {
                w32_wc2utf(wpath, path, size);
                return path;
            }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
            return NULL;
        }
#endif  //UTF8FILENAMES

        return w32_getcwdA(path, size);
    }
    return NULL;
}


static unsigned
dir_prefixA(const char *path)
{
    if (0 == memcmp(path, "\\\\?\\", 4)) {
        // consume extended prefix, if present

        if (path[4] && path[5] == ':' && path[6] == '\\') {
            return 4;                           // "\\?\X:\..."  ==> "X:/..."

        } else if (path[4] == 'U' && path[5] == 'N' && path[6] == 'C' && path[7] == '\\') {
            return 4 + 3;                       // "\\?\UNC\..." ==> "//..."
        }
    }
    return 0;
}


static unsigned
dir_prefixW(const wchar_t *path)
{
    if (0 == wmemcmp(path, L"\\\\?\\", 4)) {
        // consume extended prefix, if present

        if (path[4] && path[5] == ':' && path[6] == '\\') {
            return 4;                           // "\\?\X:\..."  ==> "X:/..."

        } else if (path[4] == 'U' && path[5] == 'N' && path[6] == 'C' && path[7] == '\\') {
            return 4 + 3;                       // "\\?\UNC\..." ==> "//..."
        }
    }
    return 0;
}


LIBW32_API char *
w32_getcwdA(char *path, size_t size)
{
    char t_path[WIN32_PATH_MAX];

    if (NULL == path || size <= 0) {
#if (__GNU_COMPAT)
        return w32_getdirectoryA();
#else
        errno = EINVAL;
#endif

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value
        //  is the size, in characters, of the buffer that is required to hold
        //  the path and the terminating null character.
        //
        DWORD ret;

        t_path[0] = 0;
        if ((ret = GetCurrentDirectoryA(_countof(t_path), t_path)) == 0) {
            w32_errno_set();

        } else if (ret >= (DWORD)size || ret >= _countof(t_path)) {
            errno = ENOMEM;

        } else {                                /* standardise to the system separator */
            unsigned prefix;
            const char *in = t_path;
            char *out = path;

            if (0 != (prefix = dir_prefixA(in))) {
                if (prefix == 7)
                    *out++ = '/';               /* leading UNC slash */
                in += prefix;
            }

            for (; *in; ++in) {
                if ('~' == *in) {               /* short-name expand */
                    (void) GetLongPathNameA(t_path, t_path, _countof(t_path));
                    for (in = t_path, out = path; *in; ++in) {
                        *out++ = ('\\' == *in ? '/' : *in);
                    }
                    break;
                }
                *out++ = ('\\' == *in ? '/' : *in);
            }
            *out = 0;
            return path;
        }
    }
    if (path && size > 0) path[0] = 0;
    return NULL;
}


LIBW32_API wchar_t *
w32_getcwdW(wchar_t *path, size_t size)
{
    wchar_t t_path[WIN32_PATH_MAX];

    if (NULL == path || size <= 0) {
#if (__GNU_COMPAT)
        return w32_getdirectoryW();             /* dynamic */
#else
        errno = EINVAL;
#endif

    } else if (size < 64) {
        errno = ERANGE;

    } else {
        //  If the function succeeds, the return value is the length, in characters,
        //  of the string copied to lpszLongPath, not including the terminating
        //  null character.
        //
        //  If the lpBuffer buffer is too small to contain the path, the return value
        //  is the size, in characters, of the buffer that is required to hold
        //  the path and the terminating null character.
        //
        DWORD ret;

        t_path[0] = 0;
        if ((ret = GetCurrentDirectoryW(_countof(t_path), t_path)) == 0) {
            w32_errno_set();

        } else if (ret >= (DWORD)size || ret >= _countof(t_path)) {
            errno = ENOMEM;

        } else {                                /* standardise to the system separator */
            unsigned prefix;
            const wchar_t *in = t_path;
            wchar_t *out = path;

            if (0 != (prefix = dir_prefixW(in))) {
                if (prefix == 7)
                    *out++ = '/';               /* leading UNC slash */
                in += prefix;
            }

            for (; *in; ++in) {
                if ('~' == *in) {               /* short-name expand */
                    (void) GetLongPathNameW(t_path, t_path, _countof(t_path));
                    for (in = t_path, out = path; *in; ++in) {
                        *out++ = ('\\' == *in ? '/' : *in);
                    }
                    break;
                }
                *out++ = ('\\' == *in ? '/' : *in);
            }
            *out = 0;
            return path;
        }
    }
    if (path && size > 0) path[0] = 0;
    return NULL;
}


LIBW32_API char *
w32_getdirectory(void)
{
#if defined(UTF8FILENAMES)
    if (w32_utf8filenames_state()) {
        wchar_t *t_wcwd;

        if (NULL != (t_wcwd = w32_getdirectoryW())) {
            char *cwd = w32_wc2utfa(t_wcwd, NULL);
            free((void *) t_wcwd);
            return cwd;
        }
    }
#endif  //UTF8FILENAMES

    return w32_getdirectoryA();
}


LIBW32_API char *
w32_getdirectoryA(void)
{
    const DWORD cdlen = GetCurrentDirectoryA(0, NULL); // includes terminator.
    char *cd;

    if (NULL != (cd = (char *)malloc(cdlen * sizeof(char)))) {
        if (cdlen == (GetCurrentDirectoryA(cdlen, cd) + 1)) { // length excludes terminator.
            unsigned prefix;

            if (0 != (prefix = dir_prefixA(cd))) {
                if (prefix == 7) {              // UNC
                    memmove(cd + 1, cd + prefix, cdlen - prefix);
                    cd[0] = '\\';
                } else {
                    memmove(cd, cd + prefix, cdlen - prefix);
                }
            }
            w32_dos2unixA(cd);
            return cd;
        }
        free((void*)cd);
    }
    return NULL;
}


LIBW32_API wchar_t *
w32_getdirectoryW(void)
{
    const DWORD cdlen = GetCurrentDirectoryW(0, NULL); // includes terminator.
    wchar_t *cd;

    if (NULL != (cd = (wchar_t *)malloc(cdlen * sizeof(wchar_t)))) {
        if (cdlen == (GetCurrentDirectoryW(cdlen, cd) + 1)) { // length excludes terminator.
            unsigned prefix;

            if (0 != (prefix = dir_prefixW(cd))) {
                if (prefix == 7) {              // UNC
                    wmemmove(cd + 1, cd + prefix, cdlen - prefix);
                    cd[0] = '\\';
                } else {
                    wmemmove(cd, cd + prefix, cdlen - prefix);
                }
            }
            w32_dos2unixW(cd);
            return cd;
        }
        free((void*)cd);
    }
    return NULL;
}


LIBW32_API int
w32_getdrive(void)
{
    wchar_t t_path[WIN32_PATH_MAX];
    DWORD ret;

    t_path[0] = 0, t_path[1] = 0;
    if ((ret = GetCurrentDirectoryW(_countof(t_path), t_path)) >= 2) {
        const wchar_t *path = 
            t_path + dir_prefixW(t_path);       // consume extended prefix, if present

        // decode drive
        if (path[1] == ':') {                   // X:\...
            const wchar_t ch = path[0];

            if (ch >= L'A' && ch <= L'Z') {
                return (ch - L'A') + 1;
            }

            if (ch >= L'a' && ch <= L'z') {
                return (ch - L'a') + 1;
            }
        }
    }
    return 0;   // UNC
}


LIBW32_API int
w32_getsystemdrive(void)
{
    wchar_t t_path[WIN32_PATH_MAX];
    DWORD ret;

    t_path[0] = 0, t_path[1] = 0;
    if ((ret = GetSystemDirectoryW(t_path, _countof(t_path))) >= 2) {
        const wchar_t *path = 
            t_path + dir_prefixW(t_path);       // consume extended prefix, if present

        // decode drive
        if (path[1] == ':') {                   // X:\...
            const wchar_t ch = path[0];

            if (ch >= L'A' && ch <= L'Z') {
                return (ch - L'A') + 1;
            }

            if (ch >= L'a' && ch <= L'z') {
                return (ch - L'a') + 1;
            }
        }
    }
    return 0;
}


LIBW32_API int
w32_getlastdrive(void)
{
    return x_w32_cwdn;
}

/*end*/
