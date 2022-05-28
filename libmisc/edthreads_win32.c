#include <edidentifier.h>
__CIDENT_RCSID(gr_edthreads_win32_c,"$Id: edthreads_win32.c,v 1.20 2022/05/26 16:02:19 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edthreads_win32.c,v 1.20 2022/05/26 16:02:19 cvsuser Exp $
 * C11 threads implementation, for windows
 * based on ISO/IEC 9899:201x Committee Draft, April 12, 2011 N1570
 *
 *
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

#if defined(_WIN32) || defined(WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#undef  WIN32
#define WIN32 0x0601
#endif

#include <edthreads.h>
#include <assert.h>


#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(__STDC_NO_THREADS__)) \
                || defined(HAVE_THREADS_H)

void
edthreads_win32_native(void)
{
}


#elif defined(__MINGW32__)
//
//  pthreads/
//      with the following 'other' POSIX realtime functions missing.
//
#if !defined(HAVE_PTHREAD_H)
#define WIN32_NATIVE_MUTEX
#pragma message "pthreads expected under mingw32; enabling local implementation"
#endif

#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif
#undef   u_char
#include <windows.h>
#include <time.h>
#include <errno.h>


int
usleep(/*usecond_t*/ const unsigned useconds)
{
    HANDLE timer = 0;
    LARGE_INTEGER due = {0};

    due.QuadPart = -(10 * (__int64)useconds);
    SetWaitableTimer(timer, &due, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
    return 0;
}


int
nanosleep(const struct timespec *rqtp, struct timespec *rmtp /*notused*/)
{
    if (!rqtp || rqtp->tv_nsec > 999999999) {
        errno = EINVAL;
        return -1;
    }
    return usleep(rqtp->tv_sec * 1000000 + rqtp->tv_nsec / 1000);
}

#elif defined(_WIN32) || defined(WIN32)
#define WIN32_NATIVE_MUTEX
#endif

//
//  Native Windows threading
//
#if defined(WIN32_NATIVE_MUTEX)

#if defined(__CYGWIN__)
#error incorrect target, win32 only
#endif

#define ED_ASSERT
#include <edassert.h>
#include <tailqueue.h>

#define MTX_MAGIC               MKMAGIC('W','M','t','x')

#define SIGNAL                  0
#define BROADCAST               1
#define signalevt               events[SIGNAL]
#define broadcastevt            events[BROADCAST]

struct threadproc {
    thrd_start_t func;
    void *arg;
};

typedef TAILQ_HEAD(threadtssList, threadtss)
                                threadtssList_t;

struct threadtss {
    TAILQ_ENTRY(threadtss) node;
    tss_dtor_t dtor;
    DWORD key;
};

static CRITICAL_SECTION         x_tssguard;
static threadtssList_t          x_tsslist;
static long long                x_perfscale;

static DWORD WINAPI             ThreadProc(void *p);


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
    if (0 == InterlockedExchange(flag, 1)) {
        (*func)();
    }
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
    int waiters;

    if (!cond) return thrd_error;
#if defined(WIN32_VISTA_ONLY)
    WakeAllConditionVariable(&cond->cond);
#else
    EnterCriticalSection(&cond->guard);
    waiters = cond->waiters;
    LeaveCriticalSection(&cond->guard);
    if (waiters > 0) {
        if (! SetEvent(cond->broadcastevt)) {
            return thrd_error;
        }
    }
#endif
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
    assert(0 == cond->waiters);
    CloseHandle(cond->signalevt);
    CloseHandle(cond->broadcastevt);
    DeleteCriticalSection(&cond->guard);
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
    (void) InitializeCriticalSectionAndSpinCount(&cond->guard, 100);

                                                // auto-reset event
    if (!(cond->signalevt = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        return thrd_error;
    }
                                                // reset event
    if (!(cond->broadcastevt = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        CloseHandle(cond->signalevt);
        return thrd_error;
    }
    cond->waiters = 0;
    return thrd_success;
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
    int waiters;

    if (!cond) return thrd_error;
    EnterCriticalSection(&cond->guard);
    waiters = cond->waiters;
    LeaveCriticalSection(&cond->guard);
    if (waiters > 0) {
        if (! SetEvent(cond->signalevt)) {
            return thrd_error;
        }
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
static __inline DWORD
internal(const struct timespec *ts)
{
    struct timespec now;
    long long diff;

    (void) timespec_get(&now, TIME_UTC);
    diff = ts->tv_sec - now.tv_sec;
    diff += ts->tv_nsec - now.tv_nsec;
    return (diff > 0 ? (DWORD)(diff/1000000) : 1);
}


int
cnd_timedwait(cnd_t * /*__restrict*/ cond, mtx_t * /*__restrict*/ mtx, const struct timespec * /*__restrict*/ ts)
{
    int rc, last;

    assert(mtx);
    assert(MTX_MAGIC == mtx->magic);
    assert(!mtx->recursive);

    EnterCriticalSection(&cond->guard);
    ++cond->waiters;
    LeaveCriticalSection(&cond->guard);

    LeaveCriticalSection(&mtx->guard);          // -- unlock mutex

                                                // wait for either signal or broadcast
    rc = WaitForMultipleObjects(2, cond->events, FALSE, internal(ts));

    EnterCriticalSection(&cond->guard);
    --cond->waiters;
    last = ((WAIT_OBJECT_0 + BROADCAST) == rc && 0 == cond->waiters);
    LeaveCriticalSection (&cond->guard);
    if (last) ResetEvent(cond->broadcastevt);

    EnterCriticalSection(&mtx->guard);          // -- lock mutex

    if (WAIT_TIMEOUT == rc)
        return thrd_timedout;
    return thrd_success;
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
#if defined(WIN32_VISTA_ONLY)
    if (!cond || !mtx) return thrd_error;
    if (! SleepConditionVariableSRW(&cond->cond, &mtx->lock, INFINITE, 0)) {
        return thrd_timedout;
    }
    return thrd_success;

#else
    int rc, last;

    EnterCriticalSection(&cond->guard);
    ++cond->waiters;
    LeaveCriticalSection(&cond->guard);

    assert(!mtx->recursive);
    LeaveCriticalSection(&mtx->guard);          // -- unlock mutex

                                                // wait for either signal or broadcast
    rc = WaitForMultipleObjects(2, cond->events, FALSE, INFINITE);

    EnterCriticalSection(&cond->guard);
    --cond->waiters;
    last = ((WAIT_OBJECT_0 + BROADCAST) == rc && 0 == cond->waiters);
    LeaveCriticalSection (&cond->guard);
    if (last) ResetEvent(cond->broadcastevt);

    EnterCriticalSection(&mtx->guard);          // -- lock mutex

    return thrd_success;
#endif
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
    assert(mtx);
    if (!mtx) return thrd_error;
    (void) InitializeCriticalSectionAndSpinCount(&mtx->guard, 100);
    mtx->recursive = ((type & mtx_recursive) ? 1 : 0);
    mtx->locked = 0;
    mtx->magic = MTX_MAGIC;
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
    assert(MTX_MAGIC == mtx->magic);
    assert(0 == mtx->locked);
    DeleteCriticalSection(&mtx->guard);
    mtx->magic = 0;
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
    assert(MTX_MAGIC == mtx->magic);
    EnterCriticalSection(&mtx->guard);
    if (1 == mtx->locked++) {
        if (! mtx->recursive) {                 // guard against dead-lock
            assert(! mtx->recursive);
            --mtx->locked;
            LeaveCriticalSection(&mtx->guard);
            return thrd_busy;
        }
    }
    assert(mtx->locked);
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
mtx_timedlock(mtx_t * /*__restrict*/ mtx, const struct timespec * /*__restrict*/ ts)
{
    if (!mtx || !ts) return thrd_error;
    assert(0);                                  // TODO
    return thrd_success;
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
    assert(MTX_MAGIC == mtx->magic);
    if (!TryEnterCriticalSection(&mtx->guard)) {
        return thrd_busy;
    }
    if (1 == mtx->locked++) {
        if (! mtx->recursive) {                 // guard against dead-lock
            assert(! mtx->recursive);
            --mtx->locked;
            LeaveCriticalSection(&mtx->guard);
            return thrd_busy;
        }
    }
    assert(mtx->locked);
    return thrd_success;
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
    assert(MTX_MAGIC == mtx->magic);
    assert(mtx->locked);
    if (!mtx->locked) return thrd_error;
    --mtx->locked;
    LeaveCriticalSection(&mtx->guard);
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
    if (0 == (*thr = CreateThread(NULL, 0, ThreadProc, proc, 0, NULL))) {
        free(proc);
        return thrd_error;
    }
    return thrd_success;
}


static DWORD WINAPI
ThreadProc(void *p)
{
    struct threadproc *proc = (struct threadproc *)p;
    thrd_start_t func = proc->func;
    void *arg = proc->arg;
    free(p);
    return (DWORD)(*func)(arg);
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
    return GetCurrentThread();
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
    CloseHandle(thr);
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
    return (thr0 == thr1);
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
    ExitThread(res);
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
    if (WaitForSingleObject(thr, INFINITE) == WAIT_FAILED) {
        return thrd_error;
    }
    if (res) {
        DWORD dwRes = 0;
        GetExitCodeThread(thr, &dwRes);
        *res = dwRes;
    }
    CloseHandle(thr);
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
    if (!duration) return thrd_error;
    Sleep((DWORD)(duration->tv_sec * 1000 + duration->tv_nsec/1000000));
    if (remaining) {
        remaining->tv_sec = 0;
        remaining->tv_nsec = 0;
    }
    return thrd_success;
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
    SwitchToThread();
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
    struct threadtss *tss;

    if (!key) return thrd_error;
    if (NULL == (tss = calloc(1, sizeof(struct threadtss)))) {
        return thrd_nomem;
    }
    if (TLS_OUT_OF_INDEXES == (tss->key = TlsAlloc())) {
        free(tss);
        return thrd_error;
    }
    if (dtor) {
        tss->dtor = dtor;
        EnterCriticalSection(&x_tssguard);      /* -- lock */
        TAILQ_INSERT_TAIL(&x_tsslist, tss, node);
        LeaveCriticalSection(&x_tssguard);      /* -- unlock */
    }
    *key = (void *)tss;
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
    struct threadtss *tss = key;
    if (tss) {
        if (tss->dtor) {
            EnterCriticalSection(&x_tssguard);  /* -- lock */
            TAILQ_REMOVE(&x_tsslist, tss, node);
            LeaveCriticalSection(&x_tssguard);  /* -- unlock */
        }
        TlsFree(tss->key);
        memset(tss, 0, sizeof(struct threadtss));
        free(tss);
    }
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
    struct threadtss *tss = key;
    if (!tss) return NULL;
    return TlsGetValue(tss->key);
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
    struct threadtss *tss = key;
    if (!tss) return thrd_error;
    if (! TlsSetValue(tss->key, val)) {
        return thrd_error;
    }
    return thrd_success;
}


#if !defined(_MSC_VER) || (_MSC_VER < 1900)
void
timespec_get(struct timespec *ts, int __unused_based)
{
    LARGE_INTEGER val;
    unsigned long long now;

    if (! QueryPerformanceCounter(&val)) {
        DWORD ms = GetTickCount();
        ts->tv_sec = ms / 1000;
        ts->tv_nsec = (ms % 1000) * 1000000;
        return;
    }
    now = (unsigned long long)(val.QuadPart/x_perfscale);
    ts->tv_sec = now / 1000000000;
    ts->tv_nsec = (long)(now % 1000000000);
}
#endif	/*FIXME - HAVE_TIMESPEC_GET*/


// typedef VOID (NTAPI *PIMAGE_TLS_CALLBACK) (PVOID DllHandle, DWORD Reason, PVOID Reserved);
static void WINAPI
__tss_callback(void *image, DWORD reason, void *pv)
{
    if (DLL_THREAD_DETACH == reason) {
        unsigned i;

        EnterCriticalSection(&x_tssguard);      /* -- lock */
        for (i = 0; i < TSS_DTOR_ITERATIONS; ++i) {
            struct threadtss *tss = TAILQ_FIRST(&x_tsslist);
            unsigned cnt = 0;

            while (tss) {
                DWORD key = tss->key;
                struct threadtss *next = TAILQ_NEXT(tss, node);
                void * val = TlsGetValue(key);

                if (val) {
                    TlsSetValue(key, NULL);
                    if (tss->dtor) {
                        LeaveCriticalSection(&x_tssguard);
                        (*tss->dtor)(val);
                        EnterCriticalSection(&x_tssguard);
                        ++cnt;
                    }
                }
                tss = next;
            }
            if (!cnt) break;
        }
        LeaveCriticalSection(&x_tssguard);      /* -- unlock */
    }
}


static void WINAPI
__tss_init(void)
{
    LARGE_INTEGER freq;

    TAILQ_INIT(&x_tsslist);
    (void) InitializeCriticalSectionAndSpinCount(&x_tssguard, 100);
    x_perfscale =                               /* 2000+ */
        (QueryPerformanceFrequency(&freq) ? (unsigned long long)(freq.QuadPart/1000000000.0) : 1);
}


/* Need to put the following marker variables into the .CRT section.
 * The .CRT section contains arrays of function pointers.
 * The compiler creates functions and adds pointers to this section
 * for things like C++ global constructors.
 *
 * The XIA, XCA etc are group names with in the section.
 * The compiler sorts the contributions by the group name.
 * For example, .CRT$XCA followed by .CRT$XCB, ... .CRT$XCZ.
 * The marker variables below let us get pointers
 * to the beginning/end of the arrays of function pointers.
 *
 * For example, standard groups are
 *      XCA         sed here, for begin marker
 *      XCC         compiler inits
 *      XCL         library inits
 *      XCU         user inits
 *
 * Runtime hooks:
 *      XCA/XCZ     C++ initializers
 *      XIA/XIZ     C initializers
 *      XPA/XPZ     C pre-terminators
 *      XTA/XTZ     C terminators
 */
#if defined(_MSC_VER)
#pragma section(".CRT$XLC",long,read)
__declspec(allocate(".CRT$XLC")) void *__edthr_xlc = __tss_callback;

#pragma section(".CRT$XIU",long,read)
__declspec(allocate(".CRT$XIU")) void *__edthr_xiu = __tss_init;

#elif defined(__MINGW32__)
#pragma data_seg(".CRT$XLC","DATA")
void *__edthr_xlc = __tss_callback;

__attribute__((constructor))
static void tss_constructor() {
    // objdump -x edthreads_win32.o | grep ctors
    __tss_init();
}

#else
#pragma data_seg(".CRT$XLC","DATA")
void *__edthr_xlc = __tss_callback;

#pragma data_seg(".CRT$XIU","DATA")
void *__edthr_xiu = __tss_init;
#endif /*!_MSC_VER*/

#endif
/*end*/

