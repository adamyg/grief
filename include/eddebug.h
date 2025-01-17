#ifndef GR_EDDEBUG_H_INCLUDED
#define GR_EDDEBUG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_eddebug_h,"$Id: eddebug.h,v 1.31 2025/01/13 16:20:06 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: eddebug.h,v 1.31 2025/01/13 16:20:06 cvsuser Exp $
 * Debug functions.
 *
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

#include <stdarg.h>
#include <edtrace.h>

__CBEGIN_DECLS

#if !defined(__EDDEBUG_INTERNALS)
extern const int            x_dflags;
#endif

/*
 *  trace support, usage
 *
 *      ED_TRACE(("my trace message\n"))
 */

extern int                  trace_log(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern int                  trace_logv(const char *fmt, va_list ap);
extern void                 trace_term(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 trace_ilog(const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 trace_data(const void *data, int length, const char *term);

#if defined(ED_LEVEL)
#if (ED_LEVEL >= 1)
#define ED_TRACE(__x)       trace_log __x;
#define ED_ITRACE(__x)      trace_ilog __x;
#define ED_DATA(__x)        trace_data __x;
#define ED_TERM(__x)        trace_term __x;
#define ED_TRACEX(__f,__x)  if ((__f) & x_dflags) trace_log __x;
#define ED_DATAX(__f,__x)   if ((__f) & x_dflags) trace_data __x;

#if (ED_LEVEL >= 2)
#define ED_TRACE2(__x)      trace_log __x;
#define ED_ITRACE2(__x)     trace_ilog __x;
#define ED_DATA2(__x)       trace_data __x;
#define ED_TERM2(__x)       trace_term __x;
#define ED_TRACE2X(__f,__x) if ((__f) & x_dflags) trace_log __x;
#define ED_DATA2X(__f,__x)  if ((__f) & x_dflags) trace_data __x;

#if (ED_LEVEL >= 3)
#if !defined(DO_NOTRACE_CHARACTER)
#if !defined(DO_TRACE_CHARACTER)
#define DO_TRACE_CHARACTER
#endif
#endif
#if !defined(DO_NOTRACE_LINE)
#if !defined(DO_TRACE_LINE)
#define DO_TRACE_LINE
#endif
#endif

#define ED_TRACE3(__x)      trace_log __x;
#define ED_ITRACE3(__x)     trace_ilog __x;
#define ED_DATA3(__x)       trace_data __x;
#define ED_TERM3(__x)       trace_term __x;
#define ED_TRACE3X(__f,__x) if ((__f) & x_dflags) trace_log __x;
#define ED_DATA3X(__f,__x)  if ((__f) & x_dflags) trace_data __x;

#endif  /*ED_LEVEL3*/
#endif  /*ED_LEVEL2*/
#endif  /*ED_LEVEL1*/
#endif  /*ED_LEVEL*/


/*
 *  void defaults.
 */
#if !defined(ED_TRACE3)
#define ED_TRACE3(__x)
#define ED_ITRACE3(__x)
#define ED_DATA3(__x)
#define ED_TERM3(__x)
#define ED_TRACE3X(__x,__f)
#define ED_DATA3X(__x,__f)

#if !defined(ED_TRACE2)
#define ED_TRACE2(__x)
#define ED_ITRACE2(__x)
#define ED_DATA2(__x)
#define ED_TERM2(__x)
#define ED_TRACE2X(__x,__f)
#define ED_DATA2X(__x,__f)

#if !defined(ED_TRACE)
#define ED_TRACE(__x)
#define ED_ITRACE(__x)
#define ED_DATA(__x)
#define ED_TERM(__x)
#define ED_TRACEX(__x,__f)
#define ED_DATAX(__x,__f)

#endif  /*ED_TRACE3*/
#endif  /*ED_TRACE2*/
#endif  /*ED_TRACE*/

__CEND_DECLS

#endif /*GR_EDDEBUG_H_INCLUDED*/
