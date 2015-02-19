#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_rwlock_c,"$Id: w32_rwlock.c,v 1.7 2015/02/19 00:17:31 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 rwlock functionality/emulation
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT        0x0501              /* enable xp+ features */
#endif
#include <sys/rwlock.h>
#define WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#include <assert.h>

typedef struct {
    unsigned            magic;
#define RW_MAGIC            0x57333272          /* W32r */
    CRITICAL_SECTION    reader_lock;
    CRITICAL_SECTION    write_lock;
    HANDLE              noreader_cond;
    int                 readers;                /* >= 0 or -99 of writer */
} rw_lock_imp;


//  typedef struct {
//      unsigned            magic;
//  #define RW_MAGIC            0x57333272      /* W32r */
//      SRWLOCK             rwlock;
//      int                 readers;            /* >= 0 or -99 of writer */
//  } rw_lock_imp2;


void
rwlock_init(struct rwlock *rwlock)
{
    rw_lock_imp *rw = (rw_lock_imp *)rwlock;
//  int s1 = sizeof(struct rwlock), s2 = sizeof(rw_lock_imp);

//  InitializeSRWLock(&rw->lock);               /* vista+ */
    assert(sizeof(struct rwlock) >= sizeof(rw_lock_imp));
    InitializeCriticalSection(&rw->reader_lock);
    InitializeCriticalSection(&rw->write_lock);
    rw->noreader_cond = CreateEvent(NULL, TRUE, TRUE, NULL);
    rw->magic = RW_MAGIC;
}


void
rwlock_rdlock(struct rwlock *rwlock)
{
    rw_lock_imp *rw = (rw_lock_imp *)rwlock;

    assert(RW_MAGIC == rw->magic);
//  AcquireSRWLockShared(&rw->lock);
    EnterCriticalSection(&rw->write_lock);
        EnterCriticalSection(&rw->reader_lock);
            if (1 == ++rw->readers) {
                ResetEvent(rw->noreader_cond);
            }
        LeaveCriticalSection(&rw->reader_lock);
    LeaveCriticalSection(&rw->write_lock);
}


void
rwlock_wrlock(struct rwlock *rwlock)
{
    rw_lock_imp *rw = (rw_lock_imp *)rwlock;

    assert(RW_MAGIC == rw->magic);
//  AcquireSRWLockExclusive(&rw->lock);
    EnterCriticalSection(&rw->write_lock);
    if (rw->readers > 0) {
        WaitForSingleObject(rw->noreader_cond, INFINITE);
    }
    assert(0 == rw->readers);
    rw->readers = -99;
}


void
rwlock_rdunlock(struct rwlock *rwlock)
{
    rw_lock_imp *rw = (rw_lock_imp *)rwlock;

    assert(RW_MAGIC == rw->magic);
//  ReleaseSRWLockShared(&rw->lock);
    EnterCriticalSection(&rw->reader_lock);
    assert(rw->readers > 0);
    if (0 == --rw->readers) {
        SetEvent(rw->noreader_cond);
    }
    LeaveCriticalSection(&rw->reader_lock);
}


void
rwlock_wrunlock(struct rwlock *rwlock)
{
    rw_lock_imp *rw = (rw_lock_imp *)rwlock;

    assert(RW_MAGIC == rw->magic);
//  ReleaseSRWLockExclusive(&rw->lock);
    assert(-99 == rw->readers);
    rw->readers = 0;
    LeaveCriticalSection(&rw->write_lock);
}
/*end*/
