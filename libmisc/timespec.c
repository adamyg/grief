#include <edidentifier.h>
__CIDENT_RCSID(gr_timespec_c,"$Id: timespec.c,v 1.10 2022/12/03 16:33:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: timespec.c,v 1.10 2022/12/03 16:33:06 cvsuser Exp $
 * timespec util functions.
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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
#include <libtime.h>


struct timespec
timespec_diff(const struct timespec end, const struct timespec start)
{
    struct timespec diff;

    if ((end.tv_nsec - start.tv_nsec) < 0) {
        diff.tv_sec  = end.tv_sec - start.tv_sec - 1;
        diff.tv_nsec = 1000000000 +end.tv_nsec - start.tv_nsec;
    } else {
        diff.tv_sec  = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec -start.tv_nsec;
    }
    return diff;
}


struct timespec
timespec_sub(struct timespec ts1, struct timespec ts2)
{
    struct timespec rtn_val;
    int xsec;
    int sign = 1;

    if (ts2.tv_nsec > ts1.tv_nsec ) {
        xsec = (int)((ts2.tv_nsec - ts1.tv_nsec) / (1E9 + 1));
        ts2.tv_nsec -= (long int)(1E9 * xsec);
        ts2.tv_sec += xsec;
    }

    if ((ts1.tv_nsec - ts2.tv_nsec) > 1E9 ) {
        xsec = (int)((ts1.tv_nsec - ts2.tv_nsec) / 1E9);
        ts2.tv_nsec += (long int)(1E9 * xsec);
        ts2.tv_sec -= xsec;
    }

    rtn_val.tv_sec = ts1.tv_sec - ts2.tv_sec;
    rtn_val.tv_nsec = ts1.tv_nsec - ts2.tv_nsec;

    if (ts1.tv_sec < ts2.tv_sec) {
        sign = -1;
    }

    rtn_val.tv_sec = rtn_val.tv_sec * sign;

    return rtn_val;
}


int
timespec_greater(struct timespec ts1, struct timespec ts2)
{
    if (ts1.tv_sec < ts2.tv_sec) {
        return 0;
    } else if (ts1.tv_sec == ts2.tv_sec) {
        if (ts1.tv_nsec < ts2.tv_nsec) {
            return 0;
        }
    }
    return 1;
}


double
timespec_seconds(struct timespec ts1, struct timespec ts2)
{
    struct timespec diff;
    double ret;

    diff = timespec_sub(ts1, ts2);
    ret = (double)diff.tv_sec;
    ret += (double)diff.tv_nsec/(double)1E9;
    return ret;
}

/*end*/
