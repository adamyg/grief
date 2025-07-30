#include <edidentifier.h>
__CIDENT_RCSID(gr_edthreads_pthread_c,"$Id: edthreads_pthread.c,v 1.18 2025/01/13 16:06:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edthreads_pthread.c,v 1.18 2025/01/13 16:06:38 cvsuser Exp $
 * C11 threads implementation, for/using pthreads
 * based on ISO/IEC 9899:201x Committee Draft, April 12, 2011
 *
 *
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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edthreads.h>

#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_THREADS__)) \
                || defined(HAVE_THREADS_H)

void
edthreads_pthreads_native(void)
{
}

#elif defined(HAVE_PTHREAD_H) || defined(__CYGWIN__)

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#if defined(sun) || defined(HAVE_SCHED_H)
#include <sched.h>
#endif

struct threadproc {
    thrd_start_t func;
    void *arg;
};

static void *ThreadProc(void *p);


// 7.25.2.1
//  Synopsis
//      #include <threads.h>
//      void call_once(once_flag *flag, void (*func)(void));
//  Description
//      The call_once function uses the once_flag pointed to by flag to ensure that
//      func is called exactly once, the first time the call_once function is called with that
//      value of flag. Completion of an effective call to the call_once function synchronizes
//      with all subsequent calls to the call_once function with the same value of flag.
//  Returns
//      The call_once function returns no value.
//
void
call_once(once_flag *flag, void (*func)(void))
{
    pthread_once(flag, func);
}


// 7.25.3.1
//  Synopsis
//      #include <threads.h>
//      int cnd_broadcast(cnd_t *cond);
//  Description
//      The cnd_broadcast function unblocks all of the threads that are blocked on the
//      condition variable pointed to by cond at the time of the call. If no threads are blocked
//      on the condition variable pointed to by cond at the time of the call, the function does
//      nothing.
//  Returns
//      The cnd_broadcast function returns thrd_success on success, or thrd_error
//      if the request could not be honored.
//
int
cnd_broadcast(cnd_t *cond)
{
    if (!cond) return thrd_error;
    if (pthread_cond_broadcast(cond) != 0) {
        return thrd_error;
    }
    return thrd_success;
}


// 7.25.3.2
//  Synopsis
//      #include <threads.h>
//      void cnd_destroy(cnd_t *cond);
//  Description
//      The cnd_destroy function releases all resources used by the condition variable
//      pointed to by cond. The cnd_destroy function requires that no threads be blocked
//      waiting for the condition variable pointed to by cond.
//  Returns
//      The cnd_destroy function returns no value
//
void
cnd_destroy(cnd_t *cond)
{
    assert(cond);
    pthread_cond_destroy(cond);
}


// 7.25.3.3
//  Synopsis
//      #include <threads.h>
//      int cnd_init(cnd_t *cond);
//  Description
//      The cnd_init function creates a condition variable. If it succeeds it sets the variable
//      pointed to by cond to a value that uniquely identifies the newly created condition
//      variable. A thread that calls cnd_wait on a newly created condition variable will
//      block.
//  Returns
//      The cnd_init function returns thrd_success on success, or thrd_nomem if no
//      memory could be allocated for the newly created condition, or thrd_error if the
//      request could not be honored.
//
int
cnd_init(cnd_t *cond)
{
    if (!cond) return thrd_error;
    switch (pthread_cond_init(cond, NULL)) {
    case 0:
        return thrd_success;
    case ENOMEM:
        return thrd_nomem;
    default:
        break;
    }
    return thrd_error;
}


// 7.25.3.4
//  Synopsis
//      #include <threads.h>
//      int cnd_signal(cnd_t *cond);
//  Description
//      The cnd_signal function unblocks one of the threads that are blocked on the
//      condition variable pointed to by cond at the time of the call. If no threads are blocked
//      on the condition variable at the time of the call, the function does nothing and return
//      success.
//  Returns
//      The cnd_signal function returns thrd_success on success or thrd_error if
//      the request could not be honored.
//
int
cnd_signal(cnd_t *cond)
{
    if (!cond) return thrd_error;
    if (pthread_cond_signal(cond) != 0) {
        return thrd_error;
    }
    return thrd_success;
}


// 7.25.3.5
//  Synopsis
//      #include <threads.h>
//      int cnd_timedwait(cnd_t *restrict cond, mtx_t *restrict mtx,
//              const struct timespec *restrict ts);
//  Description
//      The cnd_timedwait function atomically unlocks the mutex pointed to by mtx and
//      endeavors to block until the condition variable pointed to by cond is signaled by a call to
//      cnd_signal or to cnd_broadcast, or until after the TIME_UTC-based calendar
//      time pointed to by ts. When the calling thread becomes unblocked it locks the variable
//      pointed to by mtx before it returns. The cnd_timedwait function requires that the
//      mutex pointed to by mtx be locked by the calling thread.
//  Returns
//      The cnd_timedwait function returns thrd_success upon success, or
//      thrd_timedout if the time specified in the call was reached without acquiring the
//      requested resource, or thrd_error if the request could not be honored.
//
int
cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
{
    if (!cond || !mtx || !ts) return thrd_error;
    switch (pthread_cond_timedwait(cond, mtx, ts)) {
    case 0:
        return thrd_success;
    case ETIMEDOUT:
    case EAGAIN:
        return thrd_timedout;
    default:
        break;
    }
    return thrd_error;
}


// 7.25.3.6
//  Synopsis
//      #include <threads.h>
//      int cnd_wait(cnd_t *cond, mtx_t *mtx);
//  Description
//      The cnd_wait function atomically unlocks the mutex pointed to by mtx and endeavors
//      to block until the condition variable pointed to by cond is signaled by a call to
//      cnd_signal or to cnd_broadcast. When the calling thread becomes unblocked it
//      locks the mutex pointed to by mtx before it returns. The cnd_wait function requires
//      that the mutex pointed to by mtx be locked by the calling thread.
//  Returns
//      The cnd_wait function returns thrd_success on success or thrd_error if the
//      request could not be honored.
//
int
cnd_wait(cnd_t *cond, mtx_t *mtx)
{
    if (!cond || !mtx) return thrd_error;
    if (pthread_cond_wait(cond, mtx) != 0) {
        return thrd_error;
    }
    return thrd_success;
}


// 7.25.4.1
//  Synopsis
//      #include <threads.h>
//      void mtx_destroy(mtx_t *mtx);
//  Description
//      The mtx_destroy function releases any resources used by the mutex pointed to by
//      mtx. No threads can be blocked waiting for the mutex pointed to by mtx.
//  Returns
//      The mtx_destroy function returns no value.
//
void
mtx_destroy(mtx_t *mtx)
{
    assert(mtx);
    pthread_mutex_destroy(mtx);
}


// 7.25.4.2
//  Synopsis
//      #include <threads.h>
//      int mtx_init(mtx_t *mtx, int type);
//  Description
//      The mtx_init function creates a mutex object with properties indicated by type,
//      which must have one of the six values:
//          mtx_plain                   for a simple non-recursive mutex, or
//          mtx_timed                   for a non-recursive mutex that supports timeout, or
//          mtx_plain|mtx_recursive     for a simple recursive mutex, or
//          mtx_timed|mtx_recursive     for a recursive mutex that supports timeout.
//      If the mtx_init function succeeds, it sets the mutex pointed to by mtx to a value that
//      uniquely identifies the newly created mutex.
//  Returns
//      The mtx_init function returns thrd_success on success, or thrd_error if the
//      request could not be honored.
//
int
mtx_init(mtx_t *mtx, int type)
{
    pthread_mutexattr_t attr;
    int mt;

    switch (type) {
    case mtx_plain:
    case mtx_timed:
#if defined(__linux__) || defined(__linux)
        mt = PTHREAD_MUTEX_TIMED_NP;
#else
        mt = PTHREAD_MUTEX_NORMAL;
#endif
        break;
    case mtx_plain | mtx_recursive:
    case mtx_timed | mtx_recursive:
#if defined(__linux__) || defined(__linux)
        mt = PTHREAD_MUTEX_RECURSIVE_NP;
#else
        mt = PTHREAD_MUTEX_RECURSIVE;
#endif
        break;
    default:
        return thrd_error;
    }

    if (0 != pthread_mutexattr_init(&attr)) {
        return thrd_error;
    }

    if (0 != pthread_mutexattr_settype(&attr, mt)) {
        return thrd_error;
    }

    if (0 != pthread_mutex_init(mtx, &attr)) {
        return thrd_error;
    }

    return thrd_success;
}


// 7.25.4.3
//  Synopsis
//      #include <threads.h>
//      int mtx_lock(mtx_t *mtx);
//  Description
//      The mtx_lock function blocks until it locks the mutex pointed to by mtx. If the mutex
//      is non-recursive, it shall not be locked by the calling thread. Prior calls to mtx_unlock
//      on the same mutex shall synchronize with this operation.
//  Returns
//      The mtx_lock function returns thrd_success on success, or thrd_error if the
//      request could not be honored.
//
int
mtx_lock(mtx_t *mtx)
{
    if (!mtx) return thrd_error;
    pthread_mutex_lock(mtx);
    return thrd_success;
}


// 7.25.4.4
//  Synopsis
//      #include <threads.h>
//      int mtx_timedlock(mtx_t *restrict mtx, const struct timespec *restrict ts);
//  Description
//      The mtx_timedlock function endeavors to block until it locks the mutex pointed to by
//      mtx or until after the TIME_UTC-based calendar time pointed to by ts. The specified
//      mutex shall support timeout. If the operation succeeds, prior calls to mtx_unlock on
//      the same mutex shall synchronize with this operation.
//  Returns
//      The mtx_timedlock function returns thrd_success on success, or
//      thrd_timedout if the time specified was reached without acquiring the requested
//      resource, or thrd_error if the request could not be honored.
//
int
mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
{
    if (!mtx || !ts) return thrd_error;

#if defined(__CYGWIN__) || defined(__APPLE__)
    /*
     *  pthread_mutex_timedlock() is optional ...
     */
    int rc = EINVAL;

    if (ts && EBUSY == (rc = pthread_mutex_trylock(mtx))) {

#define ONESECOND_NSEC          (1000000000L)
#define ONEMILLISECOND_NSEC     (ONESECOND_NSEC/1000L)
        struct timespec end, cur, dur;

        end = *ts;
        while (end.tv_nsec < 0) { /*normalise*/
            --end.tv_sec; end.tv_nsec += ONESECOND_NSEC;
        }
        while (end.tv_nsec >= ONESECOND_NSEC) { /*normalise*/
            ++end.tv_sec; end.tv_nsec -= ONESECOND_NSEC;
        }

        while (EBUSY == (rc = pthread_mutex_trylock(mtx))) {

#if defined(CLOCK_REALTIME)
            clock_gettime(CLOCK_REALTIME, &cur);
#elif defined(TIME_UTC)
            timespec_get(&cur, TIME_UTC);
#else
#error unsupported environment ...
#endif

            if ((cur.tv_sec > end.tv_sec) ||
                    ((cur.tv_sec == end.tv_sec) && (cur.tv_nsec >= end.tv_nsec))) {
                break; /*expired*/
            }

            dur.tv_sec = end.tv_sec - cur.tv_sec;
            dur.tv_nsec = end.tv_nsec - cur.tv_nsec;
            if (dur.tv_nsec < 0) {
                --dur.tv_sec; dur.tv_nsec += ONESECOND_NSEC;
            }
            if (dur.tv_sec || (dur.tv_nsec > (ONEMILLISECOND_NSEC*5))) {
                dur.tv_sec = 0; dur.tv_nsec = (ONEMILLISECOND_NSEC*5);
            }

            nanosleep(&dur, NULL);
        }
    }

    switch (rc) {
    case 0:
        return thrd_success;
    case ETIMEDOUT:
    case EBUSY:
        return thrd_timedout;
    default:
        return thrd_error;
    }

#else
    switch (pthread_mutex_timedlock(mtx, ts)) {
    case 0:
        return thrd_success;
    case ETIMEDOUT:
        return thrd_timedout;
    default:
        break;
    }
#endif
    return thrd_error;
}


// 7.25.4.5
//  Synopsis
//      #include <threads.h>
//      int mtx_trylock(mtx_t *mtx);
//  Description
//      The mtx_trylock function endeavors to lock the mutex pointed to by mtx. If the
//      mutex is already locked, the function returns without blocking. If the operation succeeds,
//      prior calls to mtx_unlock on the same mutex shall synchronize with this operation.
//  Returns
//      The mtx_trylock function returns thrd_success on success, or thrd_busy if
//      the resource requested is already in use, or thrd_error if the request could not be
//      honored.
//
int
mtx_trylock(mtx_t *mtx)
{
    if (!mtx) return thrd_error;
    switch (pthread_mutex_lock(mtx)) {
    case 0:
        return thrd_success;
    case EBUSY:
        return thrd_busy;
    default:
        break;
    }
    return thrd_error;
}


// 7.25.4.6
//  Synopsis
//      #include <threads.h>
//      int mtx_unlock(mtx_t *mtx);
//  Description
//      The mtx_unlock function unlocks the mutex pointed to by mtx. The mutex pointed to
//      by mtx shall be locked by the calling thread.
//
//      If the lock was recursive, the call will reduce the recursive lock count and
//      return immediately if the calling task has locked the mutex more than once.
//
//  Returns
//      The mtx_unlock function returns thrd_success on success or thrd_error if
//      the request could not be honored.
//
int
mtx_unlock(mtx_t *mtx)
{
    if (!mtx) return thrd_error;
    pthread_mutex_unlock(mtx);
    return thrd_success;
}


// 7.25.5.1
//  Synopsis
//      #include <threads.h>
//      int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
//  Description
//      The thrd_create function creates a new thread executing func(arg). If the
//      thrd_create function succeeds, it sets the object pointed to by thr to the identifier of
//      the newly created thread. (A thread's identifier may be reused for a different thread once
//      the original thread has exited and either been detached or joined to another thread.)
//      The completion of the thrd_create function synchronizes with the beginning of the
//      execution of the new thread.
//  Returns
//      The thrd_create function returns thrd_success on success, or thrd_nomem if
//      no memory could be allocated for the thread requested, or thrd_error if the request
//      could not be honored.
//
int
thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
    struct threadproc *proc;

    if (!thr || !func) return thrd_error;
    if (NULL == (proc = malloc(sizeof(struct threadproc)))) {
        return thrd_nomem;
    }
    proc->func = func;
    proc->arg = arg;
    if (0 != pthread_create(thr, NULL, ThreadProc, proc)) {
        free(proc);
        return thrd_error;
    }
    return thrd_success;
}


static void *
ThreadProc(void *p)
{
    struct threadproc *proc = (struct threadproc *)p;
    thrd_start_t func = proc->func;
    void *arg = proc->arg;

    free(p);
    return (void *)(intptr_t)(*func)(arg);
}


// 7.25.5.2
//  Synopsis
//      #include <threads.h>
//      thrd_t thrd_current(void);
//  Description
//      The thrd_current function identifies the thread that called it.
//  Returns
//      The thrd_current function returns the identifier of the thread that called it.
//
thrd_t
thrd_current(void)
{
    return pthread_self();
}


// 7.25.5.3
//  Synopsis
//      #include <threads.h>
//      int thrd_detach(thrd_t thr);
//  Description
//      The thrd_detach function tells the operating system to dispose of any resources
//      allocated to the thread identified by thr when that thread terminates. The thread
//      identified by thr shall not have been previously detached or joined with another
//      thread.
//  Returns
//      The thrd_detach function returns thrd_success on success or thrd_error if
//      the request could not be honored.
//
int
thrd_detach(thrd_t thr)
{
    if (0 != pthread_detach(thr)) {
        return thrd_error;
    }
    return thrd_success;
}


// 7.25.5.4
//  Synopsis
//      #include <threads.h>
//      int thrd_equal(thrd_t thr0, thrd_t thr1);
//  Description
//      The thrd_equal function will determine whether the thread identified by thr0 refers
//      to the thread identified by thr1.
//  Returns
//      The thrd_equal function returns zero if the thread thr0 and the thread thr1 refer to
//      different threads. Otherwise the thrd_equal function returns a nonzero value.
//
int
thrd_equal(thrd_t thr0, thrd_t thr1)
{
    return pthread_equal(thr0, thr1);
}


// 7.25.5.5
//  Synopsis
//      #include <threads.h>
//      _Noreturn void thrd_exit(int res);
//  Description
//      The thrd_exit function terminates execution of the calling thread and sets its result
//      code to res.
//      The program shall terminate normally after the last thread has been terminated. The
//      behavior shall be as if the program called the exit function with the status
//      EXIT_SUCCESS at thread termination time.
//
void
thrd_exit(int res)
{
    pthread_exit((void *)(intptr_t)res);
}


// 7.25.5.6
//  Synopsis
//      #include <threads.h>
//      int thrd_join(thrd_t thr, int *res);
//  Description
//      The thrd_join function joins the thread identified by thr with the current thread by
//      blocking until the other thread has terminated. If the parameter res is not a null pointer,
//      it stores the thread's result code in the integer pointed to by res. The termination of the
//      other thread synchronizes with the completion of the thrd_join function. The thread
//      identified by thr shall not have been previously detached or joined with another thread.
//  Returns
//      The thrd_join function returns thrd_success on success or thrd_error if the
//      request could not be honored.
//
int
thrd_join(thrd_t thr, int *res)
{
    void *code;

    if (pthread_join(thr, &code) != 0) {
        return thrd_error;
    }
    if (res) *res = (int)(intptr_t)code;
    return thrd_success;
}


// 7.25.5.7
//  Synopsis
//      #include <threads.h>
//      int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
//  Description
//      The thrd_sleep function suspends execution of the calling thread until either the
//      interval specified by duration has elapsed or a signal which is not being ignored is
//      received. If interrupted by a signal and the remaining argument is not null, the
//      amount of time remaining (the requested interval minus the time actually slept) is stored
//      in the interval it points to. The duration and remaining arguments may point to the
//      same object.
//      The suspension time may be longer than requested because the interval is rounded up to
//      an integer multiple of the sleep resolution or because of the scheduling of other activity
//      by the system. But, except for the case of being interrupted by a signal, the suspension
//      time shall not be less than that specified, as measured by the system clock TIME_UTC.
//  Returns
//      The thrd_sleep function returns zero if the requested time has elapsed, -1 if it has
//      been interrupted by a signal, or a negative value if it fail
//
int
thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
    //  POSIX.1-2001, sun(-lrt)
    return nanosleep(duration, remaining);
}


// 7.25.5.8
//  Synopsis
//      #include <threads.h>
//      void thrd_yield(void);
//  Description
//      The thrd_yield function endeavors to permit other threads to run, even if the current
//      thread would ordinarily continue to run.
//  Returns
//      The thrd_yield function returns no value.
//
void
thrd_yield(void)
{
#if defined(HAVE_PTHREAD_YIELD)
    pthread_yield();                            /* AIX */

#elif defined(sun) || defined(HAVE_SCHED_YIELD)
    sched_yield();                              /* sun(-lrt), linux */

#else
    struct timespec duration = {0, 1};
    nanosleep(&duration, NULL);                 /* POSIX.1-2001*/
#endif
}


// 7.25.6.1
//  Synopsis
//      #include <threads.h>
//      int tss_create(tss_t *key, tss_dtor_t dtor);
//  Description
//      The tss_create function creates a thread-specific storage pointer with destructor
//      dtor, which may be null.
//
//      An optional destructor function may be associated with each key value. At
//      thread exit, if a key value has a non-NULL destructor pointer, and the thread
//      has a non-NULL value associated with that key, the value of the key is set to
//      NULL, and then the function pointed to is called with the previously associated
//      value as its sole argument. The order of destructor calls is unspecified if
//      more than one destructor exists for a thread when it exits.
//
//      If, after all the destructors have been called for all non-NULL values with
//      associated destructors, there are still some non-NULL values with associated
//      destructors, then the process is repeated. If, after at least
//      {TSS_DTOR_ITERATIONS} iterations of destructor calls for outstanding
//      non-NULL values, there are still some non-NULL values with associated
//      destructors, implementations may stop calling destructors, or they may continue
//      calling destructors until no non-NULL values with associated destructors exist,
//      even though this might result in an infinite loop.
//  Returns
//      If the tss_create function is successful, it sets the thread-specific storage
//      pointed to by key to a value that uniquely identifies the newly created pointer
//      and returns thrd_success; otherwise, thrd_error is returned and the
//      thread-specific storage pointed to by key is set to an undefined value.
//
int
tss_create(tss_t *key, tss_dtor_t dtor)
{
    if (!key) return thrd_error;
    if (pthread_key_create(key, dtor) != 0) {
        return thrd_error;
    }
    return thrd_success;
}


// 7.25.6.2
//  Synopsis
//      #include <threads.h>
//      void tss_delete(tss_t key);
//  Description
//      The tss_delete function releases any resources used by the thread-specific
//      storage identified by key.
//  Returns
//      The tss_delete function returns no value.
//
void
tss_delete(tss_t key)
{
    (void)pthread_key_delete(key);
}


// 7.25.6.3
//  Synopsis
//      #include <threads.h>
//      void *tss_get(tss_t key);
//  Description
//      The tss_get function returns the value for the current thread held in the
//      thread-specific storage identified by key.
//  Returns
//      The tss_get function returns the value for the current thread if successful, or
//      zero if unsuccessful.
//
void *
tss_get(tss_t key)
{
    return pthread_getspecific(key);
}


// 7.25.6.4
//  Synopsis
//      #include <threads.h>
//      int tss_set(tss_t key, void *val);
//  Description
//      The tss_set function sets the value for the current thread held in the thread-specific
//      storage identified by key to val.
//  Returns
//      The tss_set function returns thrd_success on success or thrd_error if the
//      request could not be honored.
//
int
tss_set(tss_t key, void *val)
{
    if (pthread_setspecific(key, val) != 0) {
        return thrd_error;
    }
    return thrd_success;
}

#else

extern void edthreads_pthreads_notimplemented(void);
void
edthreads_pthreads_notimplemented(void)
{
}

#endif
/*end*/

