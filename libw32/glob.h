#ifndef GR_GLOB_H_INCLUDED
#define GR_GLOB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_glob_h,"$Id: glob.h,v 1.4 2022/03/21 14:29:39 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <glob.h>
 *
 * This file is part of the GRIEF Editor.
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
 * Path name pattern matching
 *
 * Copyright (C) 2002 Michael Ringgaard. All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $NetBSD: glob.h,v 1.26 2010/09/06 14:38:56 christos Exp $   
 */

#include <win32_include.h>
#include <sys/cdefs.h>

#ifndef __gl_size_t
#define __gl_size_t size_t
#endif
#ifndef __gl_stat_t
#define __gl_stat_t struct stat
#endif

typedef struct {
        __gl_size_t gl_pathc;       /* count of total paths so far */
        __gl_size_t gl_matchc;      /* count of paths matching pattern */
        __gl_size_t gl_offs;        /* reserved at beginning of gl_pathv */
        int gl_flags;               /* copy of flags parameter to glob() */
        int (*gl_errfunc)();        /* copy of errfunc parameter to glob() */
        char **gl_pathv;            /* list of paths matching pattern */

        /*
         * Alternate filesystem access methods for glob; replacement versions of
         *  closedir(3), readdir(3), opendir(3), stat(2) and lstat(2).
         */
        void (*gl_closedir) __P((void *));
        struct dirent *(*gl_readdir) __P((void *));
        void *(*gl_opendir) __P((const char *));
        int (*gl_lstat) __P((const char *, __gl_stat_t *));
        int (*gl_stat) __P((const char *, __gl_stat_t *));
} glob_t;

/*
 * flags for glob(3)
 */
#define GLOB_APPEND     0x0001      /* Append to output from previous call */
#define GLOB_DOOFFS     0x0002      /* Specify how many null pointers to add to the beginning of gl_pathv */
#define GLOB_ERR        0x0004      /* Return on error */
#define GLOB_MARK       0x0008      /* Append / to matching directories */
#define GLOB_NOCHECK    0x0010      /* Return pattern itself if nothing matches */
#define GLOB_NOSORT     0x0020      /* Don't sort */
#define GLOB_NOESCAPE   0x1000      /* Disable backslash escaping */

    /* Non POSIX options */
#define GLOB_ALTDIRFUNC 0x0040      /* Use alternately specified directory funcs. */
#define GLOB_BRACE      0x0080      /* Expand braces ala csh */
#define GLOB_MAGCHAR    0x0100      /* Pattern had globbing characters */
#define GLOB_NOMAGIC    0x0200      /* GLOB_NOCHECK without magic chars (csh) */
#define GLOB_LIMIT      0x0400      /* Limit number of returned paths */
#define GLOB_TILDE      0x0800      /* Expand tilde names from the passwd file */
#define GLOB_PERIOD     0x2000      /* Allow metachars to match leading periods. */
#define GLOB_NO_DOTDIRS 0x4000      /* Make . and .. vanish from wildcards. */
#define GLOB_STAR       0x8000      /* Use glob ** to recurse directories */
#define GLOB_QUOTE      0           /* source compatibility - quote special chars with \ */

    /* Source compatibility, these are the old names */
#define GLOB_MAXPATH    GLOB_LIMIT
#define GLOB_ABEND      GLOB_ABORTED

/*
 * Error values returned by glob(3)
 */
#define GLOB_NOSPACE    (-1)        /* malloc call failed */
#define GLOB_ABORTED    (-2)        /* unignored error */
#define GLOB_NOMATCH    (-3)        /* no match and GLOB_NOCHECK was not set */

__BEGIN_DECLS

LIBW32_API int glob (const char *pattern, int flags, int (*errfunc)(const char *, int), glob_t *pglob);
LIBW32_API void globfree (glob_t *pglob);

__END_DECLS

#endif /*GR_GLOB_H_INCLUDED*/



