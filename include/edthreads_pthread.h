#ifndef GR_EDTHREADS_PTHREAD_H_INCLUDED
#define GR_EDTHREADS_PTHREAD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edthreads_pthread_h,"$Id: edthreads_pthread.h,v 1.10 2022/03/21 14:55:27 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edthreads_pthread.h,v 1.10 2022/03/21 14:55:27 cvsuser Exp $
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

#include <pthread.h>
#include <time.h>

__CBEGIN_DECLS

#if !defined(__cplusplus) || __cplusplus < 201103L
#define	thread_local _Thread_local
#endif

#define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS

#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT

typedef pthread_once_t once_flag;
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_key_t tss_t;
typedef pthread_t thrd_t;

__CEND_DECLS

#endif /*GR_EDTHREADS_PTHREAD_H_INCLUDED*/
