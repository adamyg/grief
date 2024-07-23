#ifndef GR_TTYUTIL_H_INCLUDED
#define GR_TTYUTIL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyutil_h,"$Id: ttyutil.h,v 1.2 2024/07/23 12:00:35 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyutil.h,v 1.2 2024/07/23 12:00:35 cvsuser Exp $
 * TTY utility functions
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

extern int tty_defaultscheme(void);
extern int tty_identification(const char *rv, int timeoutms);
extern int tty_luminance(int timeoutms);
extern int tty_write(const char *buffer, int length);
extern int tty_read(char *buffer, int length, int timeoutms);

__CEND_DECLS

#endif /*GR_TTYUTIL_H_INCLUDED*/
