#ifndef GR_WIN32_TIME_H_INCLUDED
#define GR_WIN32_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_time_h,"$Id: win32_time.h,v 1.7 2015/02/19 00:17:34 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 time functionality.
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
 */

#include <sys/cdefs.h>

__CBEGIN_DECLS

unsigned int            sleep(unsigned int);

struct timeval;
struct timezone;

int                     gettimeofday(struct timeval *tv, struct timezone *tz);

struct utimbuf;

int                     w32_utime(const char *path, const struct utimbuf *times);

__CEND_DECLS

#endif /*GR_WIN32_TIME_H_INCLUDED*/
