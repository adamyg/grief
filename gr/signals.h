#ifndef GR_SIGNALS_H_INCLUDED
#define GR_SIGNALS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_signals_h,"$Id: signals.h,v 1.3 2014/10/22 02:33:19 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: signals.h,v 1.3 2014/10/22 02:33:19 ayoung Exp $
 * Signal handling
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

#include <time.h>
#include <edsym.h>

__CBEGIN_DECLS

extern void                 signals_init(int mode);
extern void                 signals_shutdown(void);

extern void                 sighandler_abrt(int sig);

__CEND_DECLS

#endif /*GR_SIGNALS_H_INCLUDED*/
