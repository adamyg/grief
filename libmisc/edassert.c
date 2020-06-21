#include <edidentifier.h>
__CIDENT_RCSID(gr_edassert_c,"$Id: edassert.c,v 1.21 2020/06/18 13:14:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edassert.c,v 1.21 2020/06/18 13:14:07 cvsuser Exp $
 *
 *  This macro is useful for putting diagnostics into programs. When it is executed, if
 *  expression is false (zero), edAssert() prints:
 *
 *      Assertion failed: expression, file xyz, line nnn
 *
 *  On the current logger output and forces the process to exit. In the error message, xyz is
 *  the name of the source file and nnn the source line number of the edAssert() statement.
 *  The latter are respectively the values of the preprocessor macros __FILE__ and __LINE__.
 *
 *  To enable asserts the source must either be compiled with the preprocessor option
 *  -DASSERT, or with the preprocessor control statement #define ASSERT.
 *
 *
 *
 * Copyright (c) 1998 - 2017, Adam Young.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <assert.h>

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#if !defined(WIN32_LEAN_AND_MEAN)
#define  WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__MINGW32__)
#include <windows.h>
#endif
#endif

#include <edassert.h>

static edAssertTrap_t       x_trap = NULL;
static unsigned             x_flags = 0;
static char                 x_message[1024];


int
edAssertOpt(unsigned flags)
{
    unsigned oflags = x_flags;
    x_flags |= flags;
    return oflags;
}


int
edAssertTrap(edAssertTrap_t trap)
{
    x_trap = trap;
    return 0;
}


void
edAssertAt(edAssertTrap_t trap)
{
    __CUNUSED(trap)
    //TODO - atexit() style interface
}


const char *
edAssertMsg(void)
{
    return (x_message[0] ? x_message : NULL);
}


void
__edassert(
    const char *file, unsigned lineno, const char *cond)
{
    static int doubletrap = 0;

    if (!doubletrap) {

#if defined(_MSC_VER)
        _snprintf(x_message, sizeof(x_message),
            "assertion failed: %s\nFile %s, line %u.-- aborting", cond, file, lineno);
#else
        snprintf(x_message, sizeof(x_message),
            "assertion failed: %s\nFile %s, line %u.-- aborting", cond, file, lineno);
#endif
        x_message[sizeof(x_message)-1] = 0;

        if (x_trap) {
            if (TRAP_IGNORE == (*x_trap)(file, lineno, cond)) {
                return;
            }
        }
        doubletrap = 1;

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
        if (IDABORT == MessageBox((HWND)NULL, x_message, "Assertion Error",
                            MB_ABORTRETRYIGNORE|MB_ICONSTOP)) {
            FatalExit(-1);
        }
#endif  //WIN32

        fputs(x_message, stderr);
        fflush(stderr);
    }

#if defined(_WIN32) || defined(WIN32)
#if defined(__CYGWIN__)
    __assert(file, lineno, cond);
#elif defined(_MSC_VER)
    _wassert((void *)cond, (void *)file, lineno);
#else
    _assert((char *)cond, (char *)file, lineno);
#endif
#endif

#if defined(SIGABRT)
    signal(SIGABRT, SIG_DFL);
#endif
    abort();
    /*NOTREACHED*/
}


void
__edassertf(
    const char *file, unsigned lineno, const char *fmt, ...)
{
    va_list ap;
    char buf[1024];

    va_start(ap, fmt);
#ifdef HAVE_SNPRINTF
    (void) vsnprintf(buf, sizeof(buf), fmt, ap);
#else
    (void) vsprintf(buf, fmt, ap);
#endif
    va_end(ap);
    __edassert(file, lineno, buf);
    /*NOTREACHED*/
}
/*end*/
