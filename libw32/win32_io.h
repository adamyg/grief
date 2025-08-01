#ifndef LIBW32_WIN32_IO_H_INCLUDED
#define LIBW32_WIN32_IO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_io_h,"$Id: win32_io.h,v 1.42 2025/07/24 08:29:46 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 io functionality.
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

#undef DATADIR                                  /* namespace issue */

#if defined(_MSC_VER)
#pragma  warning(disable:4115)

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error old WATCOM Version, upgrade to OpenWatcom ...
#endif

#elif defined(__MINGW32__)
#else
#error win32_io.h: unknown/unsupported compiler
#endif

#if !defined(_WIN32_WINCE)                      /* require winsock2.h */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x601                   /* latest features */
#elif (_WIN32_WINNT) < 0x400
#pragma message("unistd: _WIN32_WINNT < 0400")
#endif
#endif  /*_WIN32_WINCE*/

#include <win32_include.h>

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

/*fcntl.h*/
#if !defined(F_GETFL)   /* match Linux definitions */
#define F_GETFL         3       /* get file status flags */
#define F_SETFL         4       /* set file status flags */
#endif

LIBW32_API int          fcntl (int fildes, int ctrl, int);
LIBW32_API int          w32_fcntl (int fildes, int ctrl, int);
LIBW32_API int          w32_fsync (int fildes);

/*io.h*/
LIBW32_API int          w32_open (const char *path, int, ...);
LIBW32_API int          w32_openA (const char *path, int, int);
LIBW32_API int          w32_openW (const wchar_t *path, int, int);

LIBW32_API FILE *       w32_fopen (const char *path, const char *mode);
LIBW32_API FILE *       w32_fopenA (const char *path, const char *mode);
LIBW32_API FILE *       w32_fopenW (const wchar_t *path, const wchar_t *mode);

LIBW32_API int          w32_stat (const char *path, struct stat *sb);
LIBW32_API int          w32_statA (const char *path, struct stat *sb);
LIBW32_API int          w32_statW (const wchar_t *path, struct stat *sb);

LIBW32_API int          w32_lstat (const char *path, struct stat *sb);
LIBW32_API int          w32_lstatA (const char *path, struct stat *sb);
LIBW32_API int          w32_lstatW (const wchar_t *path, struct stat *sb);

LIBW32_API int          w32_read (int fildes, void *buf, size_t nbyte);
LIBW32_API int          w32_write (int fildes, const void *buf, size_t nbyte);

LIBW32_API int          w32_close (int fildes);
LIBW32_API const char * w32_strerror (int errnum);

LIBW32_API int          w32_mkstemp (char *path);
LIBW32_API int          w32_mkstempA (char *path);
LIBW32_API int          w32_mkstempW (wchar_t *path);

LIBW32_API int          w32_mkstemps (char *path, int suffixlen);
LIBW32_API int          w32_mkstempsA (char *path, int suffixlen);
LIBW32_API int          w32_mkstempsW (wchar_t *path, int suffixlen);

LIBW32_API int          w32_mkstempx (char *path);
LIBW32_API int          w32_mkstempxA (char *path);
LIBW32_API int          w32_mkstempxW (wchar_t *path);

LIBW32_API int          w32_link (const char *path1, const char *path2);
LIBW32_API int          w32_linkA (const char *path1, const char *path2);
LIBW32_API int          w32_linkW (const wchar_t *path1, const wchar_t *path2);

LIBW32_API int          w32_unlink (const char *fname);
LIBW32_API int          w32_unlinkA (const char *fname);
LIBW32_API int          w32_unlinkW (const wchar_t *fname);

LIBW32_API int          w32_rename (const char *ofile, const char *nfile);
LIBW32_API int          w32_renameA (const char *ofile, const char *nfile);
LIBW32_API int          w32_renameW (const wchar_t *ofile, const wchar_t *nfile);

LIBW32_API char *       w32_getcwd (char *buffer, size_t size);
LIBW32_API char *       w32_getcwdA (char *buffer, size_t size);
LIBW32_API wchar_t *    w32_getcwdW (wchar_t *buffer, size_t size);

LIBW32_API char *       w32_getdirectory (void);
LIBW32_API char *       w32_getdirectoryA (void);
LIBW32_API wchar_t *    w32_getdirectoryW(void);

LIBW32_API int          w32_getdrive (void);
LIBW32_API int          w32_getsystemdrive (void);
LIBW32_API int          w32_getlastdrive (void);

LIBW32_API int          w32_mkdir (const char *path, int mode);
LIBW32_API int          w32_mkdirA (const char *path, int mode);
LIBW32_API int          w32_mkdirW (const wchar_t *path, int mode);

LIBW32_API int          w32_chdir (const char *path);
LIBW32_API int          w32_chdirA (const char *path);
LIBW32_API int          w32_chdirW (const wchar_t *path);

LIBW32_API int          w32_rmdir (const char *path);
LIBW32_API int          w32_rmdirA (const char *path);
LIBW32_API int          w32_rmdirW (const wchar_t *path);

/*support functions*/

#define SHORTCUT_TRAILING   0x01
#define SHORTCUT_COMPONENT  0x02

LIBW32_API char *       w32_resolvelinkA (const char *path, char *buf, size_t maxlen, int *ret);
LIBW32_API wchar_t *    w32_resolvelinkW (const wchar_t *path, wchar_t *buf, size_t maxlen, int *ret);

LIBW32_API int          w32_expandlink (const char *name, char* buf, size_t buflen, unsigned flags);
LIBW32_API int          w32_expandlinkA (const char *name, char *buf, size_t buflen, unsigned flags);
LIBW32_API int          w32_expandlinkW (const wchar_t *name, wchar_t *buf, size_t buflen, unsigned flags);

LIBW32_API const char * w32_strslashA (const char *path);
LIBW32_API const wchar_t *w32_strslashW (const wchar_t *path);

LIBW32_API int          w32_errno_set (void);
LIBW32_API int          w32_errno_setas (unsigned nerrno);
LIBW32_API int          w32_errno_cnv (unsigned rc);
LIBW32_API int          w32_neterrno_set (void);

extern DWORD            w32_GetFinalPathNameByHandleA (HANDLE handle, char *path, int length);
extern DWORD            w32_GetFinalPathNameByHandleW (HANDLE handle, LPWSTR path, int length);

__END_DECLS

#endif /*LIBW32_WIN32_IO_H_INCLUDED*/
