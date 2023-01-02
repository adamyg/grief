#include <edidentifier.h>
__CIDENT_RCSID(gr_timegm_c,"$Id: timegm.c,v 1.8 2022/12/09 15:41:18 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 2015 - 2023, A.Young
 * Copyright (c) 1997 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <edtypes.h>
#include <libtime.h>
#include <string.h>


#define EPOCH           70
#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define MONSPERYEAR     12


/*
 *  This is a simplifed version of timegm(3) that doesn't accept out of
 *  bound values that timegm(3) normally accepts but those are not
 *  valid in asn1 encodings.
 */

static const int
ndays[2][MONSPERYEAR] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };

static const time_t
utcbases[] = {
    0,                  /* 1970/01/01 */
    31536000,           /* 1971/01/01 */
    63072000,           /* 1972/01/01 */
    94694400,           /* 1973/01/01 */
    126230400,          /* 1974/01/01 */
    157766400,          /* 1975/01/01 */
    189302400,          /* 1976/01/01 */
    220924800,          /* 1977/01/01 */
    252460800,          /* 1978/01/01 */
    283996800,          /* 1979/01/01 */
    315532800,          /* 1980/01/01 */
    347155200,          /* 1981/01/01 */
    378691200,          /* 1982/01/01 */
    410227200,          /* 1983/01/01 */
    441763200,          /* 1984/01/01 */
    473385600,          /* 1985/01/01 */
    504921600,          /* 1986/01/01 */
    536457600,          /* 1987/01/01 */
    567993600,          /* 1988/01/01 */
    599616000,          /* 1989/01/01 */
    631152000,          /* 1990/01/01 */
    662688000,          /* 1991/01/01 */
    694224000,          /* 1992/01/01 */
    725846400,          /* 1993/01/01 */
    757382400,          /* 1994/01/01 */
    788918400,          /* 1995/01/01 */
    820454400,          /* 1996/01/01 */
    852076800,          /* 1997/01/01 */
    883612800,          /* 1998/01/01 */
    915148800,          /* 1999/01/01 */
    946684800,          /* 2000/01/01 */
    978307200,          /* 2001/01/01 */
    1009843200,         /* 2002/01/01 */
    1041379200,         /* 2003/01/01 */
    1072915200,         /* 2004/01/01 */
    1104537600,         /* 2005/01/01 */
    1136073600,         /* 2006/01/01 */
    1167609600,         /* 2007/01/01 */
    1199145600,         /* 2008/01/01 */
    1230768000,         /* 2009/01/01 */
    1262304000,         /* 2010/01/01 */
    1293840000,         /* 2011/01/01 */
    1325376000,         /* 2012/01/01 */
    1356998400,         /* 2013/01/01 */
    1388534400,         /* 2014/01/01 */
    1420070400,         /* 2015/01/01 */
    1451606400,         /* 2016/01/01 */
    1483228800,         /* 2017/01/01 */
    1514764800,         /* 2018/01/01 */
    1546300800,         /* 2019/01/01 */
    1577836800,         /* 2020/01/01 */
    1609459200,         /* 2021/01/01 */
    1640995200,         /* 2022/01/01 */
    1672531200,         /* 2023/01/01 */
    1704067200,         /* 2024/01/01 */
    1735689600,         /* 2025/01/01 */
    1767225600,         /* 2026/01/01 */
    1798761600,         /* 2027/01/01 */
    1830297600,         /* 2028/01/01 */
    1861920000,         /* 2029/01/01 */
    1893456000,         /* 2030/01/01 */
    1924992000,         /* 2031/01/01 */
    1956528000,         /* 2032/01/01 */
    1988150400,         /* 2033/01/01 */
    2019686400,         /* 2034/01/01 */
    2051222400,         /* 2035/01/01 */
    2082758400,         /* 2036/01/01 */
    2114380800,         /* 2037/01/01 */
    2145916800,         /* 2038/01/01 */
    };


static __CINLINE int
isleap(int y)
{
    y += 1900;
    return ((y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0));
}


static __CINLINE void
normalize(int *tensptr, int *unitsptr, int base)
{
    if (*unitsptr >= base) {
        *tensptr += *unitsptr / base;
        *unitsptr %= base;

    } else if (*unitsptr < 0) {
        --*tensptr;
        *unitsptr += base;
        if (*unitsptr < 0) {
            *tensptr -= 1 + (-*unitsptr) / base;
            *unitsptr = base - (-*unitsptr) % base;
        }
    }
}


time_t
xtimegm(struct tm *tm)
{
    struct tm t_tm = *tm;
    int year, mon, mday, i;
    time_t res = 0;

    if (t_tm.tm_sec >= SECSPERMIN + 2 || t_tm.tm_sec < 0) {
        normalize(&t_tm.tm_min, &t_tm.tm_sec, SECSPERMIN);
    }
    normalize(&t_tm.tm_hour, &t_tm.tm_min,  MINSPERHOUR);
    normalize(&t_tm.tm_mday, &t_tm.tm_hour, HOURSPERDAY);
    normalize(&t_tm.tm_year, &t_tm.tm_mon,  MONSPERYEAR);

    if ((year = t_tm.tm_year) < 2 || year > 138)
        return -1;                              /* 1902 - 2038 */
    if ((mon  = t_tm.tm_mon)  < 0 || mon  > 11)
        return -1;
    if ((mday = t_tm.tm_mday) < 1 || mday > ndays[isleap(year)][mon])
        return -1;
    if (t_tm.tm_hour < 0 || t_tm.tm_hour > 23)
        return -1;
    if (t_tm.tm_min  < 0 || t_tm.tm_min  > 59)
        return -1;
    if (t_tm.tm_sec  < 0 || t_tm.tm_sec  > 60)
        return -1;                              /* 0.. 60 (allow leap seconds) */

//  for (i = 139; i < year; ++i) {              /* 2039 > */
//      res += (isleap(i) ? 366 : 365);
//  }

    for (i = 0; i < mon; ++i) {
        res += ndays[isleap(year)][i];
    }

    res += mday - 1;
    res *= HOURSPERDAY;
    res += t_tm.tm_hour;
    res *= MINSPERHOUR;
    res += t_tm.tm_min;
    res *= SECSPERMIN;
    res += t_tm.tm_sec;

    if (year >= EPOCH) {                        /* 1970 ... */
//      res += utcbases[ (year > 138 ? 138 : year) - 70 ];
        res += utcbases[ year - EPOCH ];
    } else {
        res -= utcbases[ EPOCH - year ];        /* 1902 ... */
        if (isleap(year)) {
            res -= (HOURSPERDAY * MINSPERHOUR * SECSPERMIN);
        }
    }

    return res;
}


struct tm *
xgmtime(time_t t, struct tm *tm)
{
    time_t secday = t % (3600 * 24);
    time_t days = t / (3600 * 24);

    memset(tm, 0, sizeof(*tm));

    tm->tm_sec  = (int) (secday % 60);
    tm->tm_min  = (int)((secday % 3600) / 60);
    tm->tm_hour = (int) (secday / 3600);

    tm->tm_year = 70;
    while (1) {
        const int dayinyear = (isleap(tm->tm_year) ? 366 : 365);

        if (days < dayinyear) {
            break;
        }
        tm->tm_year += 1;
        days -= dayinyear;
    }
    tm->tm_mon = 0;

    while (1) {
        const int daysinmonth = ndays[isleap(tm->tm_year)][tm->tm_mon];

        if (days < daysinmonth) {
            break;
        }
        days -= daysinmonth;
        tm->tm_mon++;
    }
    tm->tm_mday = (int)(days + 1);

    return tm;
}



#if defined(LOCAL_MAIN)
#include <stdio.h>
#include <stdlib.h>

#if !defined(HAVE_TIMEGM)
static time_t
timegm(struct tm *tm) { return -1; }
#endif

void
main(void)
{
    int mon, year;

    putenv("TZ=UTC");
#if defined(_MSC_VER)
    _tzset();
#else
    tzset();
#endif

    for (year = 1901; year < 2040; ++year) {
        for (mon = 1; mon <= 12; ++mon) {
            struct tm tmi = {0};

            tmi.tm_year = year - 1900;
            tmi.tm_mon  = mon - 1;
            tmi.tm_mday = 1;
            tmi.tm_hour = 0;
            tmi.tm_min  = 0;
            tmi.tm_sec  = 0;

            {   const time_t t1 = xtimegm(&tmi),
                    t2 = mktime(&tmi),
                    t3 = timegm(&tmi);

                printf("%12ld = %12ld %c = %12ld %c     // %04d/%02d/%02d\n",
                    (long) t1, (long) t2, (t1 == t2 ? '*' : ' '),
                        (long) t3, (t1 == t3 ? '*' : ' '), year, mon, 1);
            }
        }
    }
}
#endif  /*LOCAL_MAIN*/

/*eof*/
