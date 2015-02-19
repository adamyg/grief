#ifndef GR_M_RULER_H_INCLUDED
#define GR_M_RULER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_ruler_h,"$Id: m_ruler.h,v 1.9 2014/10/22 02:33:08 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_ruler.h,v 1.9 2014/10/22 02:33:08 ayoung Exp $
 * Ruler primitives.
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

extern int                  ruler_rmargin(const BUFFER_t *bp);
extern int                  ruler_lmargin(const BUFFER_t *bp);
extern int                  ruler_colorcolumn(const BUFFER_t *bp);

extern void                 do_use_tab_char(void);
extern void                 do_tabs(void);
extern void                 do_set_tab(void);
extern void                 do_set_indent(void);
extern void                 do_set_margins(void);
extern void                 do_set_ruler(void);
extern void                 do_distance_to_indent(void);
extern void                 do_distance_to_tab(void);

extern void                 inq_tabs(void);
extern void                 inq_tab(void);
extern void                 inq_indent(void);
extern void                 inq_margins(void);
extern void                 inq_ruler(void);

__CEND_DECLS

#endif /*GR_M_RULER_H_INCLUDED*/
