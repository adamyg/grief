#ifndef GR_RULER_H_INCLUDED
#define GR_RULER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_ruler_h,"$Id: ruler.h,v 1.7 2014/10/22 02:33:17 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ruler.h,v 1.7 2014/10/22 02:33:17 ayoung Exp $
 * Ruler management.
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

#include <edtypes.h>

__CBEGIN_DECLS

extern void                 tabchar_set(int tabs);
extern int                  tabchar_get(void);

extern int                  ruler_next_stop(const LINENO *ruler, int column);
extern int                  ruler_next_tab(const BUFFER_t *bp, int column);
extern int                  ruler_next_indent(const BUFFER_t *bp, int column);

__CEND_DECLS

#endif /*GR_RULER_H_INCLUDED*/
