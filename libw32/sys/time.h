#ifndef GR_TIME_H_INCLUDED
#define GR_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_time_h,"$Id: time.h,v 1.8 2015/02/19 00:17:39 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/time.h implementation.
 *
 * Copyright (c) 1998 - 2015, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <sys/cdefs.h>
#include <sys/utypes.h>                 /* suseconds_t */
#include <sys/select.h>
#include <time.h>

#if defined(NEED_TIMEVAL)
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
//
//  The <sys/time.h> header shall define the timeval structure that includes at
//  least the following members:
//
//      time_t         tv_sec      Seconds.
//      suseconds_t    tv_usec     Microseconds.
//
//  yet current winsock definitions are as follows.
//
struct timeval {
    long                tv_sec;         /* seconds */
    long                tv_usec;        /* and microseconds */
};
#endif
#endif

struct w32_timeval {
    time_t              tv_sec;         /* seconds */
    suseconds_t         tv_usec;        /* and microseconds */
};

struct itimerval {
    struct timeval      it_interval;    /* timer interval */
    struct timeval      it_value;       /* current value */
};

/*
 -  struct timezone {
 -      int tz_minuteswest;             // minutes west of Greenwich
 -      int tz_dsttime;                 // type of dst correction
 -  };
 */

#if !defined(TIMEVAL_TO_TIMESPEC)
#define TIMEVAL_TO_TIMESPEC(tv, ts) {       \
    (ts)->tv_sec = (tv)->tv_sec;            \
    (ts)->tv_nsec = (tv)->tv_usec * 1000;   \
}
#define TIMESPEC_TO_TIMEVAL(tv, ts) {       \
    (tv)->tv_sec = (ts)->tv_sec;            \
    (tv)->tv_usec = (ts)->tv_nsec / 1000;   \
}
#endif

/* Operations on timevals. */
#if !defined(timerisset)
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#endif

#if !defined(timercmp)
#define timercmp(tvp, uvp, cmp)             \
    (((tvp)->tv_sec == (uvp)->tv_sec) ?     \
        ((tvp)->tv_usec cmp (uvp)->tv_usec) : \
        ((tvp)->tv_sec cmp (uvp)->tv_sec))
#endif

#if !defined(timerclear)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#define timeradd(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec + (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec + (uvp)->tv_usec; \
        if ((vvp)->tv_usec >= 1000000) {    \
            (vvp)->tv_sec++;                \
            (vvp)->tv_usec -= 1000000;      \
        }                                   \
    } while (0)

#define timersub(tvp, uvp, vvp)             \
    do {                                    \
        (vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec; \
        (vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec; \
        if ((vvp)->tv_usec < 0) {           \
            (vvp)->tv_sec--;                \
            (vvp)->tv_usec += 1000000;      \
        }                                   \
    } while (0)

#include <sys/cdefs.h>

__BEGIN_DECLS

int                     getitimer(int, struct itimerval *);
int                     setitimer(int, const struct itimerval *, struct timeval *);

#if defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
int                     w32_gettimeofday(struct timeval *, /*struct timezone*/ void *);
int                     w32_select(int, fd_set *, fd_set *, fd_set *, struct timeval *timeout);
#endif

#if defined(NEED_TIMEVAL) || \
        defined(_WINSOCKAPI_) || defined(_WINSOCK2API_)
int                     utimes(const char *, const struct timeval[2]);
#endif

/*
 *  POSIX 1003.1c -- <time.h>
 */

char * __PDECL          ctime_r(const time_t *ctm, char *buf);
char * __PDECL          asctime_r(const struct tm *tm, char *buf);
struct tm * __PDECL     localtime_r(const time_t *ctm, struct tm *res);
struct tm * __PDECL     gmtime_r(const time_t *ctm, struct tm *res);

__END_DECLS

#endif /*GR_TIME_H_INCLUDED*/
