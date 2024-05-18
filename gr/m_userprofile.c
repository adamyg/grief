#include <edidentifier.h>
__CIDENT_RCSID(gr_m_userprofile_c,"$Id: m_userprofile.c,v 1.13 2024/05/17 14:19:59 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_userprofile.c,v 1.13 2024/05/17 14:19:59 cvsuser Exp $
 * User profile primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <edconfig.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_userprofile.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"

#if defined(WIN32)
#if !defined(WINDOWS_MEAN_AND_LEAN)
#define  WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#include <userenv.h>                            /* GetUserProfileDirectory() */

#include <shlobj.h>                             /* SHGetFolderPath */

#pragma comment(lib, "userenv.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shfolder.lib")

static const char *     getprofilepath(const char *application, const char *dir, char *buffer, const int buflen);
static int              profileaccess(char *buffer, int buflen, int leading, const char *application, const char *subdir);
static int              exepath(char *buf, int maxlen);
static char *           unixpath(char *path);
#endif

/*  Function:           inq_profile
 *      inq_profile primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_profile - Retrieve profile directory.

        string
        inq_profile()

    Macro Description:
        The 'inq_profile()' primitive retrieves the directory within
        which user specific profile information can be stored,
        examples include the restore state and personal dictionary.

        This directory shall be system specific.

        o On Unix style systems,
            the profile is normally a sub-directory within the home
            directory of the current user is specified by the $HOME
            environment variable; this is also represented by the '~'
            directory.

>               ~/.grief

        o On Windows systems,
            the profile is normally a sub-directory within the
            directory that serves as a common repository for
            application-specific data. A typical path is

>               C:\Documents and Settings\<user>\Application Data\.grief.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_profile()' primitive returns a string containing
        system dependent profile directory.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_environment
 */
void
inq_profile(void)               /* string () */
{
    acc_assign_str(userprofile(), -1);
}


const char *
userprofile(void)
{
    static const char *grprofile = NULL;

    if (NULL == grprofile)
    {
    #if defined(WIN32)
        char buffer[MAX_PATH];
        const char *profile
            = getprofilepath("Grief", GRPROFILE, buffer, sizeof(buffer));

        grprofile = chk_salloc(profile ? profile : "~/" GRPROFILE);
    #else
        grprofile = "~/" GRPROFILE;
    #endif
    }
    return grprofile;
}


#if defined(WIN32)
/*
 *  Order:
        o CSIDL_APPDATA
            The file system directory that serves as a common
            repository for application-specific data. A typical path
            is C:\Documents and Settings\username\Application Data.

        o CSIDL_PROFILE
            The user's profile folder. A typical path is
            C:\Users\username. Applications should not create files
            or folders at this level; they should put their data
            under the locations referred to by CSIDL_APPDATA or
            CSIDL_LOCAL_APPDATA.

        o CSIDL_COMMON_APPDATA
            The file system directory that contains application data
            for all users. A typical path is C:\Documents and
            Settings\All Users\Application Data. This folder is used
            for application data that is not user specific

            This information will not roam and is available to anyone
            using the computer.

        o CSIDL_PROGRAM_FILES

        o EXEPATH
 */
static const char *
getprofilepath(const char *application, const char *dir, char *buffer, const int buflen)
{
    int len;

    // CSIDL_APPDATA
    buffer[0] = 0;
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, buffer)) &&
                (len = strlen(buffer)) > 0) {
        if (! profileaccess(buffer, buflen, len, "", dir)) {
            buffer[0] = 0;
        }
    }

    // CSIDL_PROFILE
    if (!*buffer) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, buffer)) &&
                (len = strlen(buffer)) > 0) {
            if (! profileaccess(buffer, buflen, len, "", dir)) {
                buffer[0] = 0;
            }
        }
    }

    // CSIDL_COMMON_APPDATA
    if (!*buffer) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buffer)) &&
                (len = strlen(buffer)) > 0) {
            if (! profileaccess(buffer, buflen, len, "", dir)) {
                buffer[0] = 0;
            }
        }
    }

    // CSIDL_PROGRAM_FILES
    if (!*buffer) {
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, buffer)) &&
                (len = strlen(buffer)) > 0) {
            if (! profileaccess(buffer, buflen, len, application, dir)) {
                buffer[0] = 0;
            }
        }
    }

    // <EXEPATH>, generally same as <INSTALLDIR>
    if (!*buffer) {
        if ((len = exepath(buffer, buflen)) > 0) {
            if (! profileaccess(buffer, buflen, len, "", dir)) {
                buffer[0] = 0;
            }
        }
    }

    // defaults
    if (!*buffer) {
        const char *env;

        if (NULL == (env = getenv("ProgramFiles"))) {
            env = "c:/Program Files/";
        }
        len = strlen(env);
        strxcpy(buffer, env, buflen);
        (void) profileaccess(buffer, buflen, len, application, dir);
    }

    return unixpath(buffer);
}


static int
profileaccess(char *buffer, int buflen, int leading, const char *application, const char *subdir)
{
    (void) _snprintf(buffer + leading, buflen - leading, "/%s/%s",
                (application ? application : ""), (subdir ? subdir : ""));
    buffer[buflen - 1] = 0;

    if (0 == _access(buffer, 0) &&
            (GetFileAttributesA(buffer) & FILE_ATTRIBUTE_DIRECTORY)) {
        return TRUE;
    }

    CreateDirectoryA(buffer, NULL);

    if (0 == _access(buffer, 0) &&
            (GetFileAttributesA(buffer) & FILE_ATTRIBUTE_DIRECTORY)) {
        return TRUE;
    }

    return FALSE;
}


static int
exepath(char *buf, int maxlen)
{
    if (GetModuleFileNameA(NULL, buf, maxlen)) {
        const int len = strlen(buf);
        char *cp;

        for (cp = buf + len; (cp > buf) && (*cp != '\\') && (*cp != '/'); --cp) {
            /*cont*/;                           // trailing delimiter
        }
        if ('\\' == *cp || '/' == *cp) {
            cp[1] = '\0';                       // remove program
            return (cp - buf) + 1;
        }
        return len;
    }
    return -1;
}


static char *
unixpath(char *path)
{
    const char *in = path;
    char c, *out = path;

    while (0 != (c = *in++)) {
        if ('\\' == c || '/' == c) {
            while ('\\' == *in || '/' == *in) {
                ++in;                           // compress
            }
            *out++ = '/';                       // convert
        } else {
            *out++ = c;
        }
    }
    *out = 0;
    return path;
}
#endif  //WIN32
