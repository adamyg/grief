#ifndef LIBW32_WIN32_INTERNAL_H_INCLUDED
#define LIBW32_WIN32_INTERNAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_internal_h,"$Id: win32_internal.h,v 1.21 2025/07/24 08:29:46 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * internal definitions
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
 */

#include "w32config.h"

#if defined(__WATCOMC__) && !defined(__STDC_WANT_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#include <win32_include.h>
#include <unistd.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1400) || \
    defined(__WATCOMC__) || \
    defined(__MINGW32__)
#define WIN32_OPEN      _open
#define WIN32_WOPEN     _wopen
#define WIN32_CLOSE     _close
#define WIN32_READ      _read
#define WIN32_WRITE     _write
#define WIN32_CHMOD     _chmod
#define WIN32_WCHMOD    _wchmod
#define WIN32_LSEEK     _lseek
#define WIN32_STRICMP   _stricmp
#define WIN32_STRNICMP  _strnicmp
#define WIN32_STRDUP    _strdup
#define WIN32_STRDUPW   _wcsdup
#else
#define WIN32_OPEN      open
#define WIN32_WOPEN     wopen
#define WIN32_CLOSE     close
#define WIN32_READ      read
#define WIN32_WRITE     write
#define WIN32_CHMOD     chmod
#define WIN32_WCHMOD    wchmod
#define WIN32_LSEEK     lseek
#define WIN32_STRICMP   stricmp
#define WIN32_STRNICMP  strnicmp
#define WIN32_STRDUP    strdup
#define WIN32_STRDUPW   wcsdup
#endif

#if (defined(_MSC_VER) && _MSC_VER >= 1400) || \
    defined(__MINGW32__)
#define WIN32_TZSET     _tzset
#else
#define WIN32_TZSET     tzset
#endif

#if (defined(_MSC_VER) && defined(_WIN64)) || \
    defined(__MINGW64__)
#define OSFHANDLE       intptr_t
#else
#define OSFHANDLE       long
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define WIN32_GETPID    _getpid
#else
#define WIN32_GETPID    getpid
#endif

#define SLASHCHAR       '\\'
#define XSLASHCHAR      '/'
#define SLASH           "\\"
#define LSLASH          L"\\"
#define DELIMITER       ";"
#define LDELIMITER      L";"
#define ISSLASH(c)      (((c) == SLASHCHAR) || ((c) == XSLASHCHAR))

#include <sys/cdefs.h>

__BEGIN_DECLS

#define WIN32_FILDES_DEF    (512)
#define WIN32_FILDES_MAX    (8*1024)            /* was 2048, now 8192/2019 */

extern int              x_w32_cwdn;             /* current/last working drive number, A=1 etc */
extern const char *     x_w32_cwdd[26];         /* last directory, per drive */
extern const char *     x_w32_vfscwd;           /* current UNC path, if any */

int                     w32_iostricmpA (const char *s1, const char *s2);
int                     w32_iostricmpW (const wchar_t* s1, const char* s2);
int                     w32_iostrnicmpA (const char *s1, const char *s2, int slen);
int                     w32_iostrnicmpW (const wchar_t *s1, const char *s2, int slen);

LIBW32_API int          w32_utf8filenames_enable (void);
LIBW32_API int          w32_utf8filenames_disable (void);
LIBW32_API int          w32_utf8filenames_set (int state);
LIBW32_API int          w32_utf8filenames_state (void);

LIBW32_API ino_t        w32_ino_hashA (const char *name);
LIBW32_API ino_t        w32_ino_hashW (const wchar_t *name);
LIBW32_API ino_t        w32_ino_gen (const DWORD fileIndexLow, const DWORD fileIndexHigh);
LIBW32_API ino_t        w32_ino_handle (HANDLE handle);
LIBW32_API ino_t        w32_ino_fildes (int fildes);
LIBW32_API ino_t        w32_ino_fileA (const char *name);
LIBW32_API ino_t        w32_ino_fileW (const wchar_t *name);

LIBW32_API int          w32_utf2wc (const char *src, wchar_t *dest, size_t max);
LIBW32_API int          w32_utf2wcl (const char *src);
LIBW32_API wchar_t *    w32_utf2wca (const char *src, size_t *len);
LIBW32_API int          w32_wc2utf (const wchar_t *src, char *dest, size_t max);
LIBW32_API char *       w32_wc2utfa (const wchar_t *src, size_t *len);

//User account names are limited to 20 characters and group names are limited to 256 characters.
#define WIN32_GROUP_LEN (256)
#define WIN32_LOGIN_LEN (32)

LIBW32_API const struct passwd *w32_passwd_user (void);
LIBW32_API const struct group *w32_group_user (void);

LIBW32_API char *       w32_extendedpathA (const char *path);
LIBW32_API wchar_t *    w32_extendedpathW (const wchar_t *path);

#define FNCMP_FILENAME (0x01)                   // Matching a file-name otherwise a directory,  allowing optional trailing slashes.
#define FNCMP_CASE_SENSITIVE (0x02)             // Enable case sensitively.

LIBW32_API int          w32_filenamecmpA (const char *f1, const char *f2, unsigned flags);
LIBW32_API int          w32_filenamecmpW (const wchar_t *f1, const wchar_t *f2, unsigned flags);

LIBW32_API char *       w32_dos2unixA (char *path);
LIBW32_API wchar_t *    w32_dos2unixW (wchar_t *path);
LIBW32_API char *       w32_unix2dosA (char *path);
LIBW32_API wchar_t *    w32_unix2dosW (wchar_t *path);

LIBW32_API const char * w32_strslashA (const char *path);
LIBW32_API const wchar_t * w32_strslashW (const wchar_t *path);

LIBW32_API int          w32_neterrno_map (int nerrno);
LIBW32_API int          w32_neterrno_set (void);
LIBW32_API int          w32_errno_set (void);
LIBW32_API int          w32_errno_setas (unsigned nerrno);
LIBW32_API int          w32_errno_cnv (unsigned rc);

LIBW32_API SOCKET       w32_sockhandle (int fd);

LIBW32_API void         w32_fdsetinit (void);
LIBW32_API int          w32_fdregister (int limit);
LIBW32_API void         w32_fdsockopen (int fd, SOCKET s);
LIBW32_API SOCKET       w32_fdsockget (int fd);
LIBW32_API void         w32_fdsockclose (int fd, SOCKET s);
LIBW32_API int          w32_issockfd (int fd, SOCKET *s);

LIBW32_API int          w32_reparse_readA (const char *name, char *buf, size_t maxlen);
LIBW32_API int          w32_reparse_readW (const wchar_t *name, wchar_t *buf, size_t maxlen);

extern int              w32_link_resolveA (const char* path, char *buf, size_t maxlen);
extern int              w32_link_resolveW (const wchar_t* path, wchar_t *buf, size_t maxlen);

__END_DECLS

#endif /*LIBW32_WIN32_INTERNAL_H_INCLUDED*/
