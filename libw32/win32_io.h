#ifndef GR_WIN32_IO_H_INCLUDED
#define GR_WIN32_IO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_io_h,"$Id: win32_io.h,v 1.22 2015/02/19 00:17:34 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 io functionality.
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#undef DATADIR                                  /* namespace issue */

#if defined(_MSC_VER)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#error untested MSVC C/C++ Version ...
#endif
#endif
#endif
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
#define _WIN32_WINNT        0x601               /* latest features */
#elif (_WIN32_WINNT) < 0x400
#pragma message("unistd: _WIN32_WINNT < 0400")
#endif
#endif  /*_WIN32_WINCE*/

#include <win32_include.h>

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */

__BEGIN_DECLS

/*fcntl.h*/
#if !defined(F_GETTL)
#define F_GETFL         1
#define F_SETFL         2
#endif

int                     fcntl (int fildes, int ctrl, int);
int                     w32_fsync (int fildes);

/*io.h*/
struct stat;

int                     w32_open (const char *name, int, ...);
int                     w32_stat (const char *name, struct stat *sb);
int                     w32_read (int fildes, void *buf, unsigned int nbyte);
int                     w32_write (int fildes, const void *buf, unsigned int nbyte);

ssize_t                 pread (int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t                 pwrite (int fildes, const void *buf, size_t nbyte, off_t offset);

int                     w32_close (int fildes);
const char *            w32_strerror (int errnum);
int                     w32_unlink (const char *fname);

int                     w32_mkstemp (char *path);
int                     w32_mkstempx (char *path);

int                     w32_link (const char *, const char *);
int                     w32_lstat (const char *, struct stat *);

char *                  w32_getcwd (char *buffer, int size);
char *                  w32_getcwdd (char drive, char *buffer, int size);

int                     w32_mkdir (const char *fname, int mode);
int                     w32_chdir (const char *fname);
int                     w32_rmdir (const char *fname);

/*support functions*/
int                     w32_root_unc (const char *path);

const char *            w32_strslash (const char *path);

int                     w32_errno_cnv (unsigned rc);
int                     w32_errno_setas (unsigned rc);
int                     w32_errno_set (void);
int                     w32_errno_net (void);

__END_DECLS

#endif /*GR_WIN32_IO_H_INCLUDED*/
