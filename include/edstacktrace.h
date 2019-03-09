#ifndef GR_EDSTACKTRACE_H_INCLUDED
#define GR_EDSTACKTRACE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edstacktrace_h,"$Id: edstacktrace.h,v 1.6 2018/10/04 01:28:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edstacktrace.h,v 1.6 2018/10/04 01:28:00 cvsuser Exp $
 * Diagnostics support.
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

#include <stdio.h>
#include <edsym.h>

__CBEGIN_DECLS

extern void                 edbt_init(const char *progname, int options, FILE *out);
extern void                 edbt_auto(void);
extern void                 edbt_stackdump(FILE *out, int level);
extern int                  edbt_symbol(void *address, char *buffer, int buflen);
extern const char *         edbt_demangle(const char *mname, char *dname, int dlen);

__CEND_DECLS

#endif /*GR_EDSTACKTRACE_H_INCLUDED*/
