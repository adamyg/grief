#ifndef GR_M_CALLER_H_INCLUDED
#define GR_M_CALLER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_caller_h,"$Id: m_caller.h,v 1.4 2014/10/22 02:33:00 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_caller.h,v 1.4 2014/10/22 02:33:00 ayoung Exp $
 * Caller primitives.
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

extern void                 do_set_calling_name(void);
extern void                 inq_called(void);

__CEND_DECLS

#endif /*GR_M_CALLER_H_INCLUDED*/
