#include <edidentifier.h>
__CIDENT_RCSID(gr_timehttp_c,"$Id: timehttp.c,v 1.3 2015/02/19 00:17:15 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* HTTP time parser.
 *
 *  HTTP applications have historically allowed three different formats
 *  for the representation of date/time stamps:
 *
 *      Sun, 06 Nov 1994 08:49:37 GMT       // RFC 822, updated by RFC 1123
 *      Sunday, 06-Nov-94 08:49:37 GMT      // RFC 850, obsoleted by RFC 1036
 *      Sun Nov  6 08:49:37 1994            // ANSI C's asctime() format
 *
 *      Reference: RFC1945
 *
 *
 *
 * Copyright (c) 2015, Adam Young.
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
#include <edtypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if !defined(HAVE_TIMEGM)
#include <libtime.h>
#define timegm(_tm)             xtimegm(_tm)
#endif


#if defined(XXX_NOTUSED)
static __CINLINE int
tmweekday(const char *wday)
{
    static const char
        xweekdays[7][4] = {
            "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
            };
    int w;

    for (w = 0; w < 7; ++w) {
        if (0 == strncmp(wday, xweekdays[w], 3)) {
            return w;
        }
    }
    return -1;
}
#endif  /*XXX_NOTUSED*/


static __CINLINE int
tmmonth(const char *mon)
{
    static const char
        xmonths[12][4] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
            };
    int m;

    for (m = 0; m < 12; ++m) {
        if (0 == strncmp(mon, xmonths[m], 3)) {
            return m;
        }
    }
    return -1;
}


/*
 *  rfc1123_parser ---
 *      RFC1123 date-time parser, example "Sun, 06 Nov 1994 08:49:37 GMT".
 */
static time_t
rfc1123_parser(const char *date, struct tm *tm)
{
    time_t ret = (time_t)-1;
    char wday[4], mon[4];
    int n, m;

    memset(tm, 0, sizeof(*tm));
    if (7 == (n = sscanf(date, "%3s, %02d %3s %4d %02d:%02d:%02d GMT",
                    wday, &tm->tm_mday, mon, &tm->tm_year, &tm->tm_hour, &tm->tm_min, &tm->tm_sec)) &&
            (m = tmmonth(mon)) >= 0) {

        tm->tm_mon = m;
        tm->tm_year -= 1900;
        tm->tm_isdst = -1;
        ret = timegm(tm);
    }
    return ret;
}


/*
 *  rfc1036_parser ---
 *      RFC1035 date-time parser, example "Sunday, 06-Nov-94 08:49:37 GMT".
 */
static time_t
rfc1036_parser(const char *date, struct tm *tm)
{
    time_t ret = (time_t)-1;
    char wday[11], mon[4];
    int n, m;

    memset(tm, 0, sizeof(*tm));
    if (7 == (n = sscanf(date, "%10s %2d-%3s-%2d %2d:%2d:%2d GMT",
                    wday, &tm->tm_mday, mon, &tm->tm_year, &tm->tm_hour, &tm->tm_min, &tm->tm_sec)) &&
            (m = tmmonth(mon)) >= 0) {

        tm->tm_mon = m;
        if (tm->tm_year < 50) tm->tm_year += 100;
        tm->tm_isdst = -1;
        ret = timegm(tm);
    }
    return ret;
}


/*
 *  brokenrfc850_parser ---
 *      Broken RFC850 date-time parser, example "Sunday, 06-Nov-1994 08:49:37 GMT".
 */
static time_t
broken_rfc850_parser(const char *date, struct tm *tm)
{
    time_t ret = (time_t)-1;
    char wday[11], mon[4];
    int n, m;

    memset(tm, 0, sizeof(*tm));
    if (7 == (n = sscanf(date, "%10s %2d-%3s-%4d %2d:%2d:%2d GMT",
                    wday, &tm->tm_mday, mon, &tm->tm_year, &tm->tm_hour, &tm->tm_min, &tm->tm_sec)) &&
            (m = tmmonth(mon)) >= 0) {

        tm->tm_mon = m;
        tm->tm_year -= 1900;
        tm->tm_isdst = -1;
        ret = timegm(tm);
    }
    return ret;
}


/*
 *  asctime_parser ---
 *      asctime() date-time parser, example "Sun Nov  6 08:49:37 1994".
 */
static time_t
asctime_parser(const char *date, struct tm *tm)
{
    time_t ret = (time_t)-1;
    char wday[4], mon[4];
    int n, m;

    memset(tm, 0, sizeof(*tm));
    if (7 == (n = sscanf(date, "%3s %3s %2d %2d:%2d:%2d %4d",
                    wday, mon, &tm->tm_mday,&tm->tm_hour, &tm->tm_min, &tm->tm_sec, &tm->tm_year)) &&
            (m = tmmonth(mon)) >= 0) {

        tm->tm_mon = m;
        tm->tm_year -= 1900;
        tm->tm_isdst = -1;
        ret = timegm(tm);
    }
    return ret;
}


/*
 *  http_datetime_parser ---
 *      HTTP/1 date time parser.
 *
 *      HTTP applications have historically allowed three different formats
 *      for the representation of date/time stamps:
 *
 *          Sun, 06 Nov 1994 08:49:37 GMT       ; RFC 822, updated by RFC 1123
 *          Sunday, 06-Nov-94 08:49:37 GMT      ; RFC 850, obsoleted by RFC 1036
 *          Sun Nov  6 08:49:37 1994            ; ANSI C's asctime() format
 *
 *      The first format is preferred as an Internet standard and represents a
 *      fixed-length subset of that defined by RFC 1123 [6] (an update to RFC 822 [7]).
 *
 *      The second format is in common use, but is based on the obsolete RFC 850 [10]
 *      date format and lacks a four-digit year. HTTP/1.0 clients and servers that
 *      parse the date value should accept all three formats, though they must never
 *      generate the third (asctime) format
 *
 *      All HTTP/1.0 date/time stamps must be represented in Universal Time (UT), also
 *      known as Greenwich Mean Time (GMT), without exception. This is indicated in the
 *      first two formats by the inclusion of "GMT" as the three-letter abbreviation
 *      for time zone, and should be assumed when reading the asctime format.
 *
 *      Reference: http://www.w3.org/Protocols/HTTP/1.0/spec.html#Last-Modified
 */
time_t
timehttp(const char *date, struct tm *tm)
{
    struct tm ttm;
    time_t ret;

    if ((time_t)-1 == (ret = rfc1123_parser(date, &ttm))) {
        if ((time_t)-1 == (ret = rfc1036_parser(date, &ttm))) {
            if ((time_t)-1 == (ret = asctime_parser(date, &ttm))) {
                /*
                 *  non-standard
                 */
                ret = broken_rfc850_parser(date, &ttm);
            }
        }
    }
    if (tm) *tm = ttm;
    return ret;
}


#if defined(LOCAL_MAIN)
/*
 *  test framework
 */
#include <stdio.h>

#if !defined(HAVE_TIMEGM)
#undef LOCAL_MAIN
#include "timegm.c"
#endif

void
main(void)
{
    static const char *tests[] = {
        "Sun, 06 Nov 1994 08:49:37 GMT",        /* RFC 822, updated by RFC 1123 */
        "Sunday, 06-Nov-94 08:49:37 GMT",       /* RFC 850, obsoleted by RFC 1036 */
        "Sun Nov  6 08:49:37 1994",             /* ANSI C's asctime() format */
        "Sunday, 06-Nov-1994 08:49:37 GMT",     /* broken RFC 850 */
        NULL
        };
    const char *test;
    struct tm tm;
    unsigned t;

    for (t = 0; NULL != (test = tests[t]); ++t) {
        const time_t ret = timehttp(test, &tm);

        printf("%-32s = %ld [%04d-%02d-%02d %02d:%02d:%02d]\n", 
            test, (long) ret, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
}
#endif  /*LOCAL_MAIN*/
/*end*/
