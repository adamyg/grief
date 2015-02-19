#ifndef GR_M_DEBUG_H_INCLUDED
#define GR_M_DEBUG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_debug_h,"$Id: m_debug.h,v 1.19 2014/10/22 02:33:02 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_debug.h,v 1.19 2014/10/22 02:33:02 ayoung Exp $
 * Debug primitives.
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

#include <stdarg.h>
#include <edsym.h>

__CBEGIN_DECLS

extern void                 do_debug(void);
extern void                 do_debug_support(void);
extern void                 do_profile(void);
extern void                 inq_debug(void);

__CEND_DECLS

#endif /*GR_M_DEBUG_H_INCLUDED*/
