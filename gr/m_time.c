#include <edidentifier.h>
__CIDENT_RCSID(gr_m_time_c,"$Id: m_time.c,v 1.19 2024/08/25 06:01:53 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_time.c,v 1.19 2024/08/25 06:01:53 cvsuser Exp $
 * Time primitives.
 *
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

#include <editor.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>                           /* timeval */
#endif

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "eval.h"
#include "m_time.h"
#include "symbol.h"
#include "system.h"


const char *
tm_month_name(int month)
{
    static const char *x_month_name[] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
        };
    assert(month >= 0 && month < 12);
    return x_month_name[ month ];
}


const char *
tm_month_abbrev(int month)
{
    static const char *x_month_abbrev[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec",
        };
    assert(month >= 0 && month < 12);
    return x_month_abbrev[ month ];
}


const char *
tm_day_name(int day)
{
    static const char *x_day_name[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
        };
    assert(day >= 0 && day < 7);
    return x_day_name[ day ];
}


const char *
tm_day_abbrev(int day)
{
    static const char *x_day_abbrev[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
        };
    assert(day >= 0 && day < 7);
    return x_day_abbrev[ day ];
}


static __CINLINE const struct tm *
sys_localtime(time_t timeval, struct tm *result)
{
#if defined(HAVE_LOCALTIME_R)
    return localtime_r(&timeval, result);
#else
    *result = *localtime(&timeval);
    return result;
#endif
}


static __CINLINE const struct tm *
sys_gmtime(time_t timeval, struct tm *result)
{
#if defined(HAVE_GMTIME_R)
    return gmtime_r(&timeval, result);
#else
    *result = *gmtime(&timeval);
    return result;
#endif
}


/*  Function:           do_time
 *      time() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: time - Get the current system time.

        int
        time([int &hour], [int &min], [int &sec], [int &msec])

    Macro Description:
        The 'time()' primitive retrieves the current time in local
        time.

        The following numeric components are returned

            hours   - Hour of the day, in the range [0-23].

            mins    - Minutes of the hour, in the range [0-59].

            secs    - Seconds of the minute, is the range [0-60].

            msecs   - Milliseconds, in the range [0-9999]

    Macro Returns:
        The 'time()' primitive returns the value of time in seconds
        since the Epoch (1970/1/1).

    Example:
        Displays the current date

>       int hour, min, sec, msec;
>
>       time(hour, min, sec, msec);
>       message ("time, %d:%d:%d.%d", hour, min, sec, msec);

    Macro Portability:
        The 'msec' parameter is a Grief extension; the BRIEF version
        returned only hundredths of seconds.

    Macro See Also:
        date, localtime, gmtime
 */
void
do_time(void)                   /* int ([int &hour], [int &min], [int &sec], [int &msec]) */
{
    const struct tm *tp;
    struct tm result = {0};
    time_t tmsec;
    int msec;

    tmsec = sys_time(&msec);
    tp = sys_localtime(tmsec, &result);
    argv_assign_int(1, (accint_t) tp->tm_hour);
    argv_assign_int(2, (accint_t) tp->tm_min);
    argv_assign_int(3, (accint_t) tp->tm_sec);
    argv_assign_int(4, (accint_t) msec);
    acc_assign_int((accint_t) tmsec);
}


/*  Function:           do_date
 *      date() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: date - Get current system date.

        int
        date([int &year], [int &month], [int &day],
                    [string &monname], [string &dayname])

    Macro Description:
        The 'date()' primitive retrieves the current date in local time.

        The following numeric components are returned.

            year -      Year, in the range [1900-2099].

            month -     Month of the year in the range [1-12].

            day -       Day of the month, in the range [1-31].

        in addition if supplied the following string values are
        populated

            monname -   Name of the month (e.g. "January").

            dayname -   Name of the day (e.g. "Monday").

    Macro Returns:
        The 'date' function returns the current value of time in
        seconds since the Epoch which the components represent.

    Example:

        Displays the current date

>       int year, month, days;
>       string dayname;
>
>       date(year, month, day, NULL, dayname);
>       message("%s, %d/%d/%d", dayname, year, month, day);

    Macro See Also:
        time, localtime, gmtime
 */
void
do_date(void)                   /* int ([int year], [int mon], [int day], [string monname], [string dayname]) */
{
    const time_t tmsec = time(NULL);
    const struct tm *tp;
    struct tm result = {0};

    tp = sys_localtime(tmsec, &result);
    argv_assign_int(1, (accint_t) (tp->tm_year + 1900));
    argv_assign_int(2, (accint_t) (tp->tm_mon + 1));
    argv_assign_int(3, (accint_t) tp->tm_mday);
    argv_assign_str(4, tm_month_name(tp->tm_mon));
    argv_assign_str(5, tm_day_name(tp->tm_wday));
    acc_assign_int((accint_t) tmsec);           /* extension */
}



/*  Function:           do_localtime
 *      localtime() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: localtime - Convert a time value to local time components.

        int
        localtime([int time = NULL], [int &year], [int &mon], [int &mday],
                [string &monname], [string &dayname], [int &hour],
                    [int &min], [int &sec])

    Macro Description:
        The 'localtime()' primitive converts the 'time' in seconds since
        the Epoch (1970/1/1) into its components, expressed as local
        time also known as wall-clock. If 'time' is omitted the current
        time is broken-down into components.

    Macro Returns:
        The 'localtime' function returns the value of time in seconds
        since the Epoch which the components represent.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        time, gmtime
 */
void
do_localtime(void)              /* int ([int &time], [int &year], [int &mon], [int &mday], [string &monname],
                                            [string &dayname], [int &hour], [int &min], [int &sec]) */
{                                               /*14/07/00*/
    const time_t tmsec = (time_t) get_xaccint(1, (accint_t) time(NULL));
    const struct tm *tp;
    struct tm result = {0};

    tp = sys_localtime(tmsec, &result);
    argv_assign_int(2, (accint_t) (tp->tm_year + 1900));
    argv_assign_int(3, (accint_t) (tp->tm_mon + 1));
    argv_assign_int(4, (accint_t) tp->tm_mday);
    argv_assign_str(5, tm_month_name(tp->tm_mon));
    argv_assign_str(6, tm_day_name(tp->tm_wday));
    argv_assign_int(7, (accint_t) tp->tm_hour);
    argv_assign_int(8, (accint_t) tp->tm_min);
    argv_assign_int(9, (accint_t) tp->tm_sec);
    acc_assign_int((accint_t) tmsec);
}


/*  Function:           do_gmtime
 *      gmtime() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: gmtime - Convert a time value to UTC time components.

        int
        gmtime([int time = NULL], [int &year], [int &mon], [int &mday],
                [string &monname], [string &dayname], [int &hour],
                    [int &min], [int &sec])

    Macro Description:
        The 'gmtime()' primitive converts the 'time' in seconds since the
        Epoch (1970/1/1) into its components, expressed as Coordinated
        Universal Time (UTC). If 'time' is omitted the current time is
        broken-down into components.

    Macro Returns:
        The 'gmtime' function returns the value of time in seconds
        since the Epoch which the components represent.

    Macro Portability:
        A Grief extension

    Macro See Also:
        time, localtime
 */
void
do_gmtime(void)                 /* int ([int &time], [int &year], [int &mon], [int &mday], [string &monname],
                                            [string &dayname], [int &hour], [int &min], [int &sec]) */
{                                               /*14/07/00*/
    const time_t tmsec = (time_t) get_xaccint(1, (accint_t) time(NULL));
    const struct tm *tp;
    struct tm result = {0};

    tp = sys_gmtime(tmsec, &result);
    argv_assign_int(2, (accint_t) (tp->tm_year + 1900));
    argv_assign_int(3, (accint_t) (tp->tm_mon + 1));
    argv_assign_int(4, (accint_t) tp->tm_mday);
    argv_assign_str(5, tm_month_name(tp->tm_mon));
    argv_assign_str(6, tm_day_name(tp->tm_wday));
    argv_assign_int(7, (accint_t) tp->tm_hour);
    argv_assign_int(8, (accint_t) tp->tm_min);
    argv_assign_int(9, (accint_t) tp->tm_sec);
    acc_assign_int((accint_t) tmsec);
}


/*  Function:           do_strftime
 *      strftime() primitive, which simply wraps the library system function strftime().
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strftime - Format time and date.

        string
        strftime([string format = NULL], [int time = NULL])

    Macro Description:
        The 'strftime()' primitive is an interface to the system library
        function 'strftime'. Unless 'time' is specified, the current
        time shall be formatted otherwise the stated time shall be
        formatted.

        The 'format' specification is a string and may contain special
        character sequences called conversion specifications, each of
        which is introduced by a '%' character and terminated by some
        other character known as a conversion specifier character. All
        other character sequences are ordinary character sequences.

    Conversion specifications:

        Most implementations support the following conversion
        specifications.

        %a - is replaced by the locale's abbreviated weekday name.

        %A - is replaced by the locale's full weekday name.

        %b - is replaced by the locale's abbreviated month name.

        %B - is replaced by the locale's full month name.

        %c - is replaced by the locale's appropriate date and time
             representation.

        %C - is replaced by the century number (the year divided by 100
             and truncated to an integer) as a decimal number [00-99].

        %d - is replaced by the day of the month as a decimal
             number [01,31].

        %D - same as %m/%d/%y.

        %e - is replaced by the day of the month as a decimal number
             [1,31]; a single digit is preceded by a space.

        %h - same as %b.

        %H - is replaced by the hour (24-hour clock) as a decimal
             number [00,23].

        %I - is replaced by the hour (12-hour clock) as a decimal
             number [01,12].

        %j - is replaced by the day of the year as a decimal
             number [001,366].

        %m - is replaced by the month as a decimal number [01,12].

        %M - is replaced by the minute as a decimal number [00,59].

        %n - is replaced by a newline character.

        %p - is replaced by the locale's equivalent of either a.m. or p.m.

        %r - is replaced by the time in a.m. and p.m. notation; in the
             POSIX locale this is equivalent to %I:%M:%S %p.

        %R - is replaced by the time in 24 hour notation (%H:%M).

        %S - is replaced by the second as a decimal number [00,61].

        %t - is replaced by a tab character.

        %T - is replaced by the time (%H:%M:%S).

        %u - is replaced by the weekday as a decimal number [1, 7],
             with 1 representing Monday.

        %U - is replaced by the week number of the year (Sunday as the
             first day of the week) as a decimal number [00,53].

        %V - is replaced by the week number of the year (Monday as the
             first day of the week) as a decimal number [01, 53]. If
             the week containing 1 January has four or more days in the
             new year, then it is considered week 1. Otherwise, it is
             the last week of the previous year, and the next week is
             week 1.

        %w - is replaced by the weekday as a decimal number [0, 6],
             with 0 representing Sunday.

        %W - is replaced by the week number of the year (Monday as the
             first day of the week) as a decimal number [00, 53]. All
             days in a new year preceding the first Monday are
             considered to be in week 0.

        %x - is replaced by the locale's appropriate date representation.

        %X - is replaced by the locale's appropriate time representation.

        %y - is replaced by the year without century as a decimal
             number [00,99].

        %Y - is replaced by the year with century as a decimal number.

        %Z - is replaced by the timezone name or abbreviation, or by no
             bytes if no timezone information exists.

        %% - is replaced by %.

    Macro Returns:
        The 'strftime' function returns a string containing the
        formatted time and date.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        time, cftime, date, stat, localtime and gmtime
 */
void
do_strftime(void)               /* string ([string format = NULL], [int time = NULL], [string locale = TODO]) */
{
    const char *fmt = get_xstr(1);
    const time_t tmsec = (time_t) get_xaccint(2, (accint_t) time(NULL));
    const struct tm *tp;
    struct tm result = {0};
    char buffer[1024];
    int ret;

    tp = sys_localtime(tmsec, &result);
/*XXX, bsd or ICU strftime()*/
    ret = strftime(buffer, (int)sizeof(buffer), (fmt ? fmt : "%c"), tp);
    acc_assign_str(buffer, ret);
}


/*  Function:           do_cftime
 *      cftime() primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: cftime - Format time and date.

        string
        cftime(string format, int time)

    Macro Description:
        The 'cftime()' primitive is an alternative interface to <strftime>.

    Macro Returns:
        The 'cftime' function returns a string containing the formatted
        time and date.

    Macro Portability:
        Provided for CRiSPEdit compatility, see <strftime>.

    Macro See Also:
        strftime
 */
void
do_cftime(void)                 /* string (string format, int time) */
{
    do_strftime();
}
/*end*/
