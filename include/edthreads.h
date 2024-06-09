#ifndef GR_EDTHREADS_H_INCLUDED
#define GR_EDTHREADS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edthreads_h,"$Id: edthreads.h,v 1.21 2024/04/08 15:07:03 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edthreads.h,v 1.21 2024/04/08 15:07:03 cvsuser Exp $
 * Threads interface
 * ISO/IEC 9899:201x Committee Draft
 * April 12, 2011 N1570
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#if defined(__MINGW32__) || (defined(__CYGWIN__) && defined(HAVE_PTHREAD_H))
#define __STDC_NO_THREADS__ /*PTHREADS or WIN32*/
#endif

#include <edsym.h>

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_THREADS__)) \
                || defined(HAVE_THREADS_H)
#include <threads.h>

#else  /*NON-NATIVE*/

enum {
    mtx_plain       =0x0,
    mtx_recursive   =0x1,
    mtx_timed       =0x2
};

enum {
    thrd_error      =0,
    thrd_success    =1,
    thrd_timedout   =2,
    thrd_busy       =3,
    thrd_nomem      =4
};

__CBEGIN_DECLS

typedef void (*tss_dtor_t)(void *val);
typedef int (*thrd_start_t)(void *arg);

__CEND_DECLS

#if defined(HAVE_THREAD_H)
#include <edthreads_thread.h>
#elif defined(HAVE_PTHREAD_H) || defined(__CYGWIN__)
#include <edthreads_pthread.h>
#elif defined(_WIN32) || defined(WIN32)
#include <edthreads_win32.h>
#else
#error Unsupported target ...
#endif

__CBEGIN_DECLS

extern void                 call_once(once_flag *flag, void (*func)(void));

extern int                  cnd_broadcast(cnd_t *cond);
extern void                 cnd_destroy(cnd_t *cond);
extern int                  cnd_init(cnd_t *cond);
extern int                  cnd_signal(cnd_t *cond);
extern int                  cnd_timedwait(cnd_t * cond, mtx_t *mtx, const struct timespec *ts);
extern int                  cnd_wait(cnd_t *cond, mtx_t *mtx);

extern void                 mtx_destroy(mtx_t *mtx);
extern int                  mtx_init(mtx_t *mtx, int type);
extern int                  mtx_lock(mtx_t *mtx);
extern int                  mtx_timedlock(mtx_t *mtx, const struct timespec *ts);
extern int                  mtx_trylock(mtx_t *mtx);
extern int                  mtx_unlock(mtx_t *mtx);

extern int                  thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
extern thrd_t               thrd_current(void);
extern int                  thrd_detach(thrd_t thr);
extern int                  thrd_equal(thrd_t thr0, thrd_t thr1);
extern /*_Noreturn*/ void   thrd_exit(int res);
extern int                  thrd_join(thrd_t thr, int *res);
extern int                  thrd_sleep(const struct timespec *duration, struct timespec *remaining);
extern void                 thrd_yield(void);

extern int                  tss_create(tss_t *key, tss_dtor_t dtor);
extern void                 tss_delete(tss_t key);
extern void *               tss_get(tss_t key);
extern int                  tss_set(tss_t key, void *val);

__CEND_DECLS

#endif /*NON-NATIVE*/

__CBEGIN_DECLS

extern struct timespec      timespec_diff(const struct timespec end, const struct timespec start);
extern struct timespec      timespec_sub(struct timespec ts1, struct timespec ts2);

__CEND_DECLS

#endif /*GR_EDTHREADS_H_INCLUDED*/

