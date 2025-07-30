#ifndef GR_EDTRACE_H_INCLUDED
#define GR_EDTRACE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edtrace_h,"$Id: edtrace.h,v 1.32 2025/01/13 16:20:07 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edtrace.h,v 1.32 2025/01/13 16:20:07 cvsuser Exp $
 * trace log.
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

#include <stdio.h>
#include <stdarg.h>
#include <edtypes.h>

__CBEGIN_DECLS

/// trace active flags
enum {
    EDTRACE_FENABLE = 0x01,
    EDTRACE_FFLUSH  = 0x02,
    EDTRACE_FTIME   = 0x04
};

extern int                  trace_filename(const char *name);
extern void                 trace_active(unsigned flags);
extern int                  trace_isactive(void);       /* returns flags, default=0 */
extern FILE *               trace_stream(void);
extern void                 trace_flush(void);

extern int                  trace_log(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern int                  trace_logv(const char *str, va_list ap) __ATTRIBUTE_FORMAT__((printf, 1, 0));
extern void                 trace_logx(int level, const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 2, 3));
extern void                 trace_logxv(int level, const char *fmt, va_list ap)  __ATTRIBUTE_FORMAT__((printf, 2, 0));
extern void                 trace_hex(const void *data, int n);
extern void                 trace_data(const void *data, int length, const char *term);
extern int                  trace_str(const char *str);

extern const char *         c_string(const char *str);

__CEND_DECLS

#endif /*GR_EDTRACE_H_INCLUDED*/
