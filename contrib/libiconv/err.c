/* $Id: err.c,v 1.5 2024/06/04 13:15:59 cvsuser Exp $
 * win32 <err.h> implementation
 *
 * Copyright (c) 2012 - 2024 Adam Young.
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

#include "namespace.h"
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <err.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__WATCOMC__)
#include <shellapi.h>                           /* SHSTDAPI */
#endif
#include <shlobj.h>

#if (0)
static const char *x_progname = NULL;

void
setprogname(const char *progname)
{
    free((void *)x_progname);
    x_progname = (progname ? strdup(progname) : NULL);
}


const char *
getprogname(void)
{
    if (NULL == x_progname) {
        char *path;

        if (NULL == (path = malloc(MAX_PATH + 1)) ||
                !GetModuleFileName(NULL, path, MAX_PATH)) {
            x_progname = strdup("progname");
            free(path);
        } else {
            path[MAX_PATH] = 0;
            x_progname = path;
        }
    }
    return x_progname;
}
#endif /*libw32*/


void
err(int ret, const char *fmt, ...)
{
    const int x_errno = errno;
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s : ", getprogname());
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, " : %d (%s)\n", x_errno, w32_strerror(x_errno));
    va_end(ap);
    exit(ret);
}


void
errx(int ret, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s : ", getprogname());
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
    exit(ret);
}


void
warn(const char *fmt, ...)
{
    const int x_errno = errno;
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s : ", getprogname());
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, " : %d (%s)\n", x_errno, w32_strerror(x_errno));
    va_end(ap);
}


void
warnx(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s : ", getprogname());
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    va_end(ap);
}

