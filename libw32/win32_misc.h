#ifndef LIBW32_WIN32_MISC_H_INCLUDED
#define LIBW32_WIN32_MISC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_misc_h,"$Id: win32_misc.h,v 1.20 2025/06/28 11:07:21 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 public interface
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#include "win32_include.h"
#include <sys/cdefs.h>

__BEGIN_DECLS

enum w32ostype {            /* generalised machine types, ignoring server */
    OSTYPE_WIN_11,
    OSTYPE_WIN_10,
    OSTYPE_WIN_8,
    OSTYPE_WIN_7,
    OSTYPE_WIN_VISTA,
    OSTYPE_WIN_NT,
    OSTYPE_WIN_CE,
    OSTYPE_WIN_95
};

#define SYSDIR_TEMP         0x000001
#define SYSDIR_WINDOWS      0x000002
#define SYSDIR_SYSTEM       0x000003 
#define SYSDIR_PROGRAM_FILES  0x000004

#define WIN32_PATH_MAX      1024                /* 255, unless UNC names are used */
#define WIN32_LINK_DEPTH    8

int                         w32_htof(HANDLE handle);
HANDLE                      w32_ftoh(int pid);

HANDLE                      w32_osfhandle(int fildes);
int                         w32_osfdup(HANDLE osfhandle, int flags);

LIBW32_API enum w32ostype   w32_ostype(void);

LIBW32_API int              w32_getprogdir(char *buf, int maxlen);
LIBW32_API int              w32_getprogdirA(char *buf, int maxlen);
LIBW32_API int              w32_getprogdirW(wchar_t *buf, int maxlen);

LIBW32_API int              w32_getsysdir(int id, char *buf, int maxlen);
LIBW32_API int              w32_getsysdirA(int id, char *buf, int maxlen);
LIBW32_API int              w32_getsysdirW(int id, wchar_t *buf, int maxlen);

LIBW32_API int              w32_regstrget(const char *subkey, const char *valuename, char *buf, int len);
LIBW32_API int              w32_regstrgetx(HKEY hkey, const char *subkey, const char *valuename, char *buf, int len);
LIBW32_API const char *     w32_getlanguage(char *buffer, int len);

LIBW32_API const char *     w32_selectfolder(const char *message, char *path, int buflen);
LIBW32_API const char *     w32_selectfolderA(const char *message, char *path, int buflen);
LIBW32_API const wchar_t *  w32_selectfolderW(const wchar_t *message, wchar_t *path, int buflen);

LIBW32_API int              w32_IsElevated(void);
LIBW32_API int              w32_IsAdministrator(void);

LIBW32_API const char *     w32_syserrorA(DWORD dwError, char *buf, size_t bufien);
LIBW32_API const char *     w32_vsyserrorA(DWORD dwError, char *buf, size_t bufien, ...);
LIBW32_API const wchar_t *  w32_syserrorW(DWORD dwError, wchar_t *buf, size_t buflen);
LIBW32_API const wchar_t *  w32_vsyserrorW(DWORD dwError, wchar_t *buf, size_t buflen, ...);

__END_DECLS

#endif /*LIBW32_WIN32_MISC_H_INCLUDED*/
