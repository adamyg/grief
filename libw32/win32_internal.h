#ifndef GR_WIN32_INTERNAL_H_INCLUDED
#define GR_WIN32_INTERNAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_internal_h,"$Id: win32_internal.h,v 1.7 2015/02/19 00:17:34 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * internal definitions.
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


#include <config.h>
#include <unistd.h>
#include <win32_include.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#define WIN32_OPEN      _open
#define WIN32_CLOSE     _close
#define WIN32_READ      _read
#define WIN32_WRITE     _write
#define WIN32_CHMOD     _chmod
#define WIN32_LSEEK     _lseek
#define WIN32_STRICMP   _stricmp
#define WIN32_STRDUP    _strdup
#define WIN32_GETPID    _getpid
#define WIN32_TZSET     _tzset
#else
#define WIN32_OPEN      open
#define WIN32_CLOSE     close
#define WIN32_READ      read
#define WIN32_WRITE     write
#define WIN32_CHMOD     chmod
#define WIN32_LSEEK     lseek
#define WIN32_STRICMP   stricmp
#define WIN32_STRDUP    strdup
#define WIN32_GETPID    getpid
#define WIN32_TZSET     tzset
#endif

#define SLASHCHAR       '\\'
#define XSLASHCHAR      '/'
#define SLASH           "\\"
#define DELIMITER       ";"
#define ISSLASH(c)      (((c) == SLASHCHAR) || ((c) == XSLASHCHAR))

#include <sys/cdefs.h>

__BEGIN_DECLS

#define WIN32_FILDES_MAX    1024

extern const char *     x_w32_vfscwd;

extern const char *     x_w32_cwdd[26];

ino_t                   w32_ino_hash (const char *name);
ino_t                   w32_ino_gen (const DWORD fileIndexLow, const DWORD fileIndexHigh);
ino_t                   w32_ino_handle (HANDLE handle);
ino_t                   w32_ino_fildes (int fildes);
ino_t                   w32_ino_file (const char *name);

char *                  w32_dos2unix (char *path);
char *                  w32_unix2dos (char *path);

int                     w32_root_unc (const char *path);

const char *            w32_strslash (const char *path);

int                     w32_errno_cnv (unsigned rc);
int                     w32_errno_setas (unsigned rc);
int                     w32_errno_set (void);
int                     w32_errno_net (void);

__END_DECLS

#endif /*GR_WIN32_INTERNAL_H_INCLUDED*/
