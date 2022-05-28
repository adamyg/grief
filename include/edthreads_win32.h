#ifndef GR_EDTHREADS_WIN32_H_INCLUDED
#define GR_EDTHREADS_WIN32_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edthreads_win32_h,"$Id: edthreads_win32.h,v 1.18 2022/05/26 16:02:26 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edthreads_win32.h,v 1.18 2022/05/26 16:02:26 cvsuser Exp $
 * Threads interface
 * ISO/IEC 9899:201x Committee Draft
 * April 12, 2011 N1570
 *
 * Implementation specific, following 7.26.1
 *
 *   o macros
 *
 *      thread_local
 *          which expands to _Thread_local;
 *
 *      ONCE_FLAG_INIT
 *          which expands to a value that can be used to initialize an object of type
 *          once_flag; and TSS_DTOR_ITERATIONS which expands to an integer constant
 *          expression representing the maximum number of times that destructors will
 *          be called when a thread terminates.
 *
 *   o the types are;
 *
 *      cnd_t
 *          which is a complete object type that holds an identifier for a condition
 *          variable;
 *
 *      thrd_t
 *          which is a complete object type that holds an identifier for a thread;
 *
 *      tss_t
 *          which is a complete object type that holds an identifier for a
 *          thread-specific storage pointer;
 *
 *      mtx_t
 *          which is a complete object type that holds an identifier for a mutex;
 *
 *      tss_dtor_t
 *          which is the function pointer type void (*)(void*), used for a destructor
 *          for a thread-specific storage pointer;
 *
 *      thrd_start_t
 *          which is the function pointer type int (*)(void*) that is passed to
 *          thrd_create to create a new thread; and
 *
 *      once_flag
 *          which is a complete object type that holds a flag for use by call_once.
 *
 *   o enumeration constants are;
 *
 *      mtx_plain
 *          which is passed to mtx_init to create a mutex object that supports
 *          neither timeout nor test and return;
 *
 *      mtx_recursive
 *          which is passed to mtx_init to create a mutex object that supports
 *          recursive locking;
 *
 *      mtx_timed
 *          which is passed to mtx_init to create a mutex object that supports
 *          timeout;
 *
 *      thrd_timedout
 *          which is returned by a timed wait function to indicate that the time
 *          specified in the call was reached without acquiring the requested
 *          resource;
 *
 *      thrd_success
 *          which is returned by a function to indicate that the requested
 *          operation succeeded;
 *
 *      thrd_busy
 *          which is returned by a function to indicate that the requested operation
 *          failed because a resource requested by a test and return function is
 *          already in use;
 *
 *      thrd_error
 *          which is returned by a function to indicate that the requested operation
 *          failed; and
 *
 *      thrd_nomem
 *          which is returned by a function to indicate that the requested operation
 *          failed because it was unable to allocate memory.
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#if !defined(GR_EDTHREADS_H_INCLUDED)
#error incorrect usage, only include <edthreads.h>
#endif
#if (!defined(_WIN32) && !defined(WIN32)) || defined(__CYGWIN__)
#error incorrect target, win32 only
#endif

#include <win32_include.h>
#include <time.h>

//Mutex and conditional time specificiation <time.h>
#if !defined(HAVE_TIMESPEC)
#if (defined(_MSC_VER) && (_MSC_VER < 1900)) || \
    (defined(__WATCOMC__) && (__WATCOMC__ < 1300)) || \
        (!defined(__WATCOMC__) && !defined(_MSC_VER) && !defined(__MINGW32__))
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

#elif (defined(__WATCOMC__) && (__WATCOMC__ == 1300))
#ifndef _TIMESPEC_DEFINED   /*open-watcom*/
#define _TIMESPEC_DEFINED
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
#endif
#endif
#endif /*TODO - HAVE_TIMESPEC*/

#if defined(TIME_UTC)
#if (TIME_UTC != 1)
#error  TIME_UTC redefinition error ...
#endif
#else
#define TIME_UTC            1
#endif

#define TSS_DTOR_ITERATIONS 4

#define thread_local

#define ONCE_FLAG_INIT 0

typedef LONG once_flag;

typedef struct {
    unsigned                magic;
    CRITICAL_SECTION        guard;
    unsigned short          recursive;
    unsigned short          locked;
} mtx_t;

typedef struct {
    CRITICAL_SECTION        guard;
    HANDLE                  events[2];
    int                     waiters;
} cnd_t;

typedef HANDLE thrd_t;

typedef void *tss_t;

__CBEGIN_DECLS

#if !defined(_MSC_VER) || (_MSC_VER < 1900)
extern void                 timespec_get(struct timespec *ts, int base);
#endif /*FIXME - HAVE_TIMESPEC_GET*/

__CEND_DECLS

#endif /*GR_EDTHREADS_WIN32_H_INCLUDED*/
