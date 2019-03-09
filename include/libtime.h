#ifndef GR_LIBTIME_H_INCLUDED
#define GR_LIBTIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libtime_h,"$Id: libtime.h,v 1.4 2018/10/04 01:28:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libtime.h,v 1.4 2018/10/04 01:28:00 cvsuser Exp $
 * libtime - Miscellaneous time library functions.
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#include <edtypes.h>
#include <time.h>

__CBEGIN_DECLS

struct timespec;

extern void                 timespec_current(struct timespec *ts);

extern struct timespec      timespec_diff(const struct timespec end, const struct timespec start);
extern struct timespec      timespec_sub(struct timespec ts1, struct timespec ts2);
extern int                  timespec_greater(struct timespec ts1, struct timespec ts2);
extern double               timespec_seconds(struct timespec ts1, struct timespec ts2);

extern time_t               timeutcoffsetx(const int year, const int mon, const int mday, const int hour, const int min);
extern time_t               timeutcoffset(const int mon, const int mday);

extern time_t               timehttp(const char *date, struct tm *tm);

extern time_t               xtimegm(struct tm *tm);
extern struct tm *          xgmtime(time_t t, struct tm *tm);

__CEND_DECLS

#endif /*GR_LIBTIME_H_INCLUDED*/
