#ifndef GR_BORDER_H_INCLUDED
#define GR_BORDER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_border_h,"$Id: border.h,v 1.12 2014/10/22 02:32:54 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: border.h,v 1.12 2014/10/22 02:32:54 ayoung Exp $
 * Border managment.
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

extern int                  win_tborder(const WINDOW_t *wp);
extern int                  win_bborder(const WINDOW_t *wp);
extern int                  win_rborder(const WINDOW_t *wp);
extern int                  win_lborder(const WINDOW_t *wp);
extern int                  win_lmargin(const WINDOW_t *wp);
extern int                  win_height(const WINDOW_t *wp);
extern int                  win_width(const WINDOW_t *wp);
extern int                  win_uwidth(const WINDOW_t *wp);
extern int                  win_tedge(const WINDOW_t *wp);
extern int                  win_bedge(const WINDOW_t *wp);
extern int                  win_redge(const WINDOW_t *wp);
extern int                  win_rclipped(const WINDOW_t *wp, int redge);
extern int                  win_ledge(const WINDOW_t *wp);
extern int                  win_tline(const WINDOW_t *wp);
extern int                  win_bline(const WINDOW_t *wp);
extern int                  win_fcolumn(const WINDOW_t *wp);
extern int                  win_lcolumn(const WINDOW_t *wp);
extern int                  win_rcolumn(const WINDOW_t *wp);

__CEND_DECLS

#endif /*GR_BORDER_H_INCLUDED*/
