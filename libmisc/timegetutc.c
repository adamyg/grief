#include <edidentifier.h>
__CIDENT_RCSID(gr_timegetutc_c,"$Id: timegetutc.c,v 1.12 2025/01/13 16:06:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: timegetutc.c,v 1.12 2025/01/13 16:06:38 cvsuser Exp $
 * Portable (somewhat) method to determine GMT offset.
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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <edthreads.h>
#include <libtime.h>


time_t
timeutcoffsetx(const int year, const int mon, const int mday, const int hour, const int min)
{
/*  #if defined(HAVE_MKTIME) || \
 *          defined(_MSC_VER) || defined(__WATCOMC__) || defined(unix) || defined(__unix__)
 */

    struct tm local_tm = {0}, utc_tm = {0}, *utc_tmptr;
    time_t local_t, utc_t, utcOffset = 0;

    local_tm.tm_year  = year;
    local_tm.tm_mon   = mon - 1;
    local_tm.tm_mday  = mday;
    local_tm.tm_hour  = hour;
    local_tm.tm_min   = min;
    local_tm.tm_isdst = -1;
    local_t = mktime(&local_tm);

    if (local_t != (time_t) -1) {
#if defined(HAVE_GMTIME_R)
        utc_tmptr = gmtime_r(&local_t, &local_tm);
#else
        utc_tmptr = gmtime(&local_t);
#endif
        utc_tm.tm_year  = utc_tmptr->tm_year;
        utc_tm.tm_mon   = utc_tmptr->tm_mon;
        utc_tm.tm_mday  = utc_tmptr->tm_mday;
        utc_tm.tm_hour  = utc_tmptr->tm_hour;
        utc_tm.tm_min   = utc_tmptr->tm_min;
        utc_tm.tm_isdst = -1;
        utc_t = mktime(&utc_tm);

        if (utc_t != (time_t) -1) {
            utcOffset = (local_t - utc_t);
        }
    }
    return utcOffset;

/*  #else
 *      return -1;
 *  #endif
 */
}


time_t
timeutcoffset(const int mon, const int mday)
{
    return (timeutcoffsetx(2000, mon, mday, 12, 0));
}

/*end*/
