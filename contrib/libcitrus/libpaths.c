/* $Id: libpaths.c,v 1.10 2021/06/14 14:12:57 cvsuser Exp $
 *
 * libcitrus <paths.h> implementation
 *
 * Copyright (c) 2012-2021 Adam Young.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ==end==
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__WATCOMC__)
#include <shellapi.h>                           /* SHSTDAPI */
#endif
#include <shlobj.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")

#include "../contrib_config.h"                  /* APPLICATIONDIR */
#include "paths.h"

static const char *     x_application_dir = APPLICATIONDIR;

static const char *     getpath(const char *application, const char *dir, char *buffer, const int buflen);
static int              getexedir(char *buf, int maxlen);
static int              getdlldir(char *buf, int maxlen);
static void             dospath(char *path);


/**
 *  Retrieve global system configuration path, equivalent to '/usr/share/i18n/iconv'
 *
 *      <EXEPATH>
 *          <exepath>\i18n\iconv\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Package>\i18n\iconv
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Package>\i18n\iconv
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
__citrus_PATH_ICONV(void)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        getpath(x_application_dir, "i18n/iconv", x_buffer, sizeof(x_buffer));
    }
    return x_buffer;
}


/*
 *  Retrieve global system configuration path, equivalent to '/usr/share/i18n/esdb'
 *
 *      <EXEPATH>
 *          <exepath>\i18n\esdb\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Package>\i18n\esdb
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Package>\i18n\esdb
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
__citrus_PATH_ESDB(const char *subdir)
{
    static char x_buffer[MAX_PATH];
    static int x_len = 0;

    if (0 == x_buffer[0]) {
        getpath(x_application_dir, "i18n/esdb", x_buffer, sizeof(x_buffer));
        x_len = strlen(x_buffer);
    }
    if (subdir)  {
        _snprintf(x_buffer + x_len, sizeof(x_buffer) - x_len, subdir);
        x_buffer[sizeof(x_buffer)-1] = 0;
    } else {
        x_buffer[x_len] = 0;
    }
    return x_buffer;
}


/*
 *  Retrieve global system configuration path, equivalent to '/usr/share/i18n/csmapper'
 *
 *      <EXEPATH>
 *          <exepath>\i18n\csmapper\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Package>\i18n\csmapper
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Package>\i18n\csmapper
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
__citrus_PATH_CSMAPPER(const char *subdir)
{
    static char x_buffer[MAX_PATH];
    static int x_len = 0;

    if (0 == x_buffer[0]) {
        getpath(x_application_dir, "i18n/csmapper", x_buffer, sizeof(x_buffer));
        x_len = strlen(x_buffer);
    }
    if (subdir)  {
        _snprintf(x_buffer + x_len, sizeof(x_buffer) - x_len, subdir);
        x_buffer[sizeof(x_buffer)-1] = 0;
    } else {
        x_buffer[x_len] = 0;
    }
    return x_buffer;
}


/*
 *  Retrieve global system configuration path, equivalent to '/usr/lib/i18n/module'
 *
 *      <EXEPATH>
 *          <exepath>\i18n\module\
 *
 *      <INSTALLPATH>
 *          X:\Program Files\<Package>\i18n\module
 *
 *              SHGetFolderPath(CSIDL_PROGRAM_FILES)
 *              or getenv(ProgramFiles)
 *
 *      <APPDATA>
 *          X:\Documents and Settings\All Users\Application Data\<Package>\i18n\module
 *
 *              SHGetFolderPath(CSIDL_COMMON_APPDATA)
 *              or getenv(ALLUSERSPROFILE)
 */
const char *
__citrus_PATH_I18NMODULE(void)
{
    static char x_buffer[MAX_PATH];

    if (0 == x_buffer[0]) {
        getpath(x_application_dir, "i18n/module", x_buffer, sizeof(x_buffer));
    }
    return x_buffer;
}


static const char *
getpath(const char *application, const char *dir, char *buffer, const int buflen)
{
    int len, done = FALSE;

    // <DLLPATH>, generally same as INSTALLDIR
    if ((len = getdlldir(buffer, buflen)) > 0) {
        _snprintf(buffer + len, buflen - len, "/%s", dir);
        buffer[buflen - 1] = 0;
        if (0 == _access(buffer, 0)) {
            done = TRUE;
        }
    }

    // <EXEPATH>, generally same as INSTALLDIR
    if (! done) {
        if ((len = getexedir(buffer, buflen)) > 0) {
            _snprintf(buffer + len, buflen - len, "/%s", dir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // <INSTALLPATH>
    if (! done) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // <APPDATA>
    if (! done)  {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);
            buffer[buflen - 1] = 0;
            if (0 == _access(buffer, 0)) {
                done = TRUE;
            }
        }
    }

    // default - INSTALLPATH
    if (! done) {
        const char *env;

        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer))) {
            len = strlen(buffer);
            _snprintf(buffer + len, buflen - len, "/%s/%s", application, dir);

        } else if (NULL != (env = getenv("ProgramFiles"))) {
            _snprintf(buffer, buflen, "%s/%s/%s/", env, application, dir);

        } else {
            _snprintf(buffer, buflen, "c:/Program Files/%s/%s", application, dir);
        }
        buffer[buflen - 1] = 0;
        w32_mkdir(buffer, 0666);
    }

    dospath(buffer);
    return buffer;
}


static int
getexedir(char *buf, int maxlen)
{
    if (GetModuleFileNameA(NULL, buf, maxlen)) {
        const int len = strlen(buf);
        char *cp;

        for (cp = buf + len; (cp > buf) && (*cp != '\\'); --cp)
            /*cont*/;
        if ('\\' == *cp) {
            cp[1] = '\0';                       // remove program
            return (cp - buf) + 1;
        }
        return len;
    }
    return -1;
}


static int
getdlldir(char *buf, int maxlen)
{
#if defined(__WATCOMC__)
    HMODULE hm = NULL;

    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &__citrus_PATH_ICONV, &hm) != 0 &&
        GetModuleFileNameA(hm, buf, maxlen)) {
#else
    EXTERN_C IMAGE_DOS_HEADER __ImageBase;

    if (GetModuleFileNameA((HINSTANCE)&__ImageBase, buf, maxlen)) {
#endif
        const int len = strlen(buf);
        char *cp;

        for (cp = buf + len; (cp > buf) && (*cp != '\\'); --cp)
            /*cont*/;
        if ('\\' == *cp) {
            cp[1] = '\0';                       // remove library
            return (cp - buf) + 1;
        }
        return len;
    }
    return -1;
}


static void
dospath(char *path)
{
    const char *in = path;

    while (*in) {
        if ('/' == *in || '\\' == *in) {
            ++in;
            while ('/' == *in || '\\' == *in) {
                ++in;                           // compress
            }
            *path++ = '\\';                     // convert
        } else {
            *path++ = *in++;
        }
    }
    *path = 0;
}

