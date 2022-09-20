#include <edidentifier.h>
__CIDENT_RCSID(gr_timeget_c,"$Id: timeget.c,v 1.4 2022/09/20 15:19:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <edthreads.h>
#include <libtime.h>


void
timespec_current(struct timespec *ts)
{
#ifdef HAVE_TIMESPEC_GET
    return timespec_get(&ts, TIME_UTC);

#elif defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_REALTIME)
    return clock_gettime(CLOCK_REALTIME, ts);

#elif defined(HAVE_GETTIMEOFDAY)
    struct timeval tv;
    (void) gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec;
    ts->tv_nsec = tv.tv_usec * 1000;

#else
    ts->tv_sec = time(NULL);
    ts->tv_nsec = 0;
#endif
}
/*end*/


