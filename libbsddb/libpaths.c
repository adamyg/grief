/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libpaths.c,v 1.6 2015/03/01 01:53:54 cvsuser Exp $
 *
 * libbsddb <libpath.c>
 *
 * Copyright (c) 2012-2015 Adam Young.
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#if defined(HAVE_PATHS_H)
#include <paths.h>
#endif
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(WIN32)
#define  WINDOWS_MEAN_AND_LEAN
#undef u_char
#include <windows.h>
#include <io.h>
#if defined(W_OK)
#define F_OK            0
#define W_OK            2
#endif
#endif /*WIN32*/


#if !defined(_PATH_TMP)

#if defined(WIN32)
#define access(__p,__m) _access(__p,__m)
#endif

static const char *     tmpdir2(const char *env);


/*  Function:           libbsddb_PATH_TMP
 *      System dependent temporary directory.
 *
 *  Notes:
 *      Result is cached on first call and returned on all others.
 *
 *  Parameters
 *      none
 *
 *  Returns:
 *      Address of path buffer.
 */
const char *
libbsddb_PATH_TMP(void)
{
    static const char *tmpdir = NULL;           /* one-shot */

    if (NULL == tmpdir) {
#if defined(WIN32)
        char t_path[MAX_PATH];
#endif
        const char *p = NULL;

        p = tmpdir2("GRTMP");                   /* GRIEF override */
        if (NULL == p || !*p) {
            p = tmpdir2("BTMP");                /* BRIEF override */
        }

#if defined(WIN32)
        if (NULL == p || !*p) {
            DWORD pathlen;

            if ((pathlen = GetTempPath(sizeof(t_path), t_path)) > 0) {
                if (pathlen > 1 && '\\' == t_path[pathlen-1]) {
                    --pathlen;                  /* remove trailing delimiter */
                }
                t_path[pathlen] = 0;
                if (0 == access(t_path, 0)) {
                    p = t_path;
                }
            }
        }
#endif

        if ((NULL == p || !*p)
                && NULL == (p = tmpdir2("TMP")) /* standard unix tmp */
#if defined(__MSDOS__) || defined(__CYGWIN32__)
                && NULL == (p = tmpdir2("TEMP"))
                && NULL == (p = tmpdir2("TMPDIR"))
#endif
            )
        {
#if defined(_VMS)
            p = "[-]";
#else
            static const char *tmpdirs[] = {
                "/tmp",
#if defined(unix) || defined(__APPLE__)
                "/var/tmp",
#endif
#if defined(__MSDOS__)
                "c:/tmp",
                "c:/temp",
                "c:/windows/temp",
                "/temp",
                "/windows/temp",
#endif
#if defined(__CYGWIN32__)
                "/cygdrive/c/temp",
                "/cygdrive/c/windows/temp",
#endif
                NULL
                };
            int d;

            p = ".";                            /* current directory, default */
            for (d = 0; tmpdirs[d]; ++d)
                if (0 == access(tmpdirs[d], W_OK)) {
                    p = tmpdirs[d];
                    break;
                }
#endif  /*!VMS*/
        }
#if defined(WIN32)
        tmpdir = _strdup(p);
#else
        tmpdir = strdup(p);
#endif
    }
    return tmpdir;
}


static const char *
tmpdir2(const char *env)
{
    const char *p;

    if ((p = getenv(env)) != NULL && p[0] &&
                0 == access(p, W_OK)) {
        return p;
    }
    return NULL;
}

#endif  /*!_PATH_TMP*/


