#ifndef GR_M_LINE_H_INCLUDED
#define GR_M_LINE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_line_h,"$Id: m_line.h,v 1.6 2014/10/22 02:33:05 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_line.h,v 1.6 2014/10/22 02:33:05 ayoung Exp $
 * Line primitives.
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

extern void                 inq_line_flags(void);
extern void                 do_find_line_flags(void);
extern void                 do_find_marker(void);
extern void                 do_mark_line(void);
extern void                 do_set_line_flags(void);

__CEND_DECLS

#endif /*GR_M_LINE_H_INCLUDED*/
