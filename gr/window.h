#ifndef GR_WINDOW_H_INCLUDED
#define GR_WINDOW_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_window_h,"$Id: window.h,v 1.24 2014/10/22 02:33:25 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: window.h,v 1.24 2014/10/22 02:33:25 ayoung Exp $
 * Window management.
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

#define W_MINWIDTH          16
#define W_MINHEIGHT         4

#define WCTRL0_INTERNAL                         /* internal controls */ \
                            ((1 << WCTRLO_USER_VSCROLL) | (1 << WCTRLO_USER_HSCROLL) | \
                             (1 << WCTRLO_USER_VTHUMB)  | (1 << WCTRLO_USER_HTHUMB))

extern WINDOW_t *           window_new(const WINDOW_t *clone);
extern void                 window_append(WINDOW_t *wp);

extern WINDOW_t *           window_first(void);
extern WINDOW_t *           window_next(WINDOW_t *wp);

extern int                  window_create(int flag, const char *title, int x, int y, int w, int h);
extern void                 window_delete(WINDOW_t *wp);
extern void                 window_title(WINDOW_t *wp, const char *top, const char *bottom);
extern int                  window_top_line(WINDOW_t *wp, int top_line);
extern int                  window_center_line(WINDOW_t *wp, int line);
extern void                 attach_buffer(WINDOW_t *wp, BUFFER_t *b);
extern void                 detach_buffer(WINDOW_t *wp);
extern WINDOW_t *           window_argument(int n);
extern WINDOW_t *           window_xargument(int n);
extern WINDOW_t *           window_lookup(int num);
extern WINDOW_t *           window_vsplit(void);
extern WINDOW_t *           window_hsplit(void);
extern void                 win_modify(int flag);
extern void                 window_modify(WINDOW_t *wp, int flag);
extern void                 window_attr(WINDOW_t *wp);

extern void                 window_harden(void);
extern void                 window_sort(void);
extern void                 window_corners(void);
extern int                  window_ctrl_set(WINDOW_t *wp, uint32_t ctrl);
extern void                 window_ctrl_clr(WINDOW_t *wp, uint32_t ctrl);
extern int                  window_ctrl_test(const WINDOW_t *wp, uint32_t ctrl);

extern WINDOW_t             x_window_null;

extern int                  x_window_nextattr;
extern int                  x_window_popups;

extern int                  x_display_top;
extern uint32_t             x_display_ctrl;

extern int                  x_display_scrollcols;
extern int                  x_display_scrollrows;
extern int                  x_display_mincols;
extern int                  x_display_minrows;
extern int                  x_display_numbercols;

extern uint32_t             w_ctrl_mask;
extern uint32_t             w_ctrl_state;
extern uint32_t             w_ctrl_stack[];

extern int                  xf_borders;

__CEND_DECLS

#endif /*GR_WINDOW_H_INCLUDED*/
