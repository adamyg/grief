#ifndef GR_M_SYSTEM_H_INCLUDED
#define GR_M_SYSTEM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_system_h,"$Id: m_system.h,v 1.7 2014/10/22 02:33:10 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_system.h,v 1.7 2014/10/22 02:33:10 ayoung Exp $
 * System primitives.
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

extern void                 do_getenv(void);
extern void                 do_putenv(void);
extern void                 do_getpid(void);
extern void                 do_getuid(void);
extern void                 do_geteuid(void);

extern void                 inq_clock(void);

__CEND_DECLS

#endif /*GR_M_SYSTEM_H_INCLUDED*/
