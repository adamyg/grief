#ifndef GR_M_TIME_H_INCLUDED
#define GR_M_TIME_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_time_h,"$Id: m_time.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_time.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $
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

#include <edsym.h>

__CBEGIN_DECLS

const char *                tm_month_name(int month);
const char *                tm_month_abbrev(int month);
const char *                tm_day_name(int day);
const char *                tm_day_abbrev(int day);

extern void                 do_cftime(void);
extern void                 do_date(void);
extern void                 do_gmtime(void);
extern void                 do_localtime(void);
extern void                 do_strftime(void);
extern void                 do_time(void);

__CEND_DECLS

#endif /*GR_M_TIME_H_INCLUDED*/
