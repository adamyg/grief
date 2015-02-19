#ifndef GR_M_WINDOW_H_INCLUDED
#define GR_M_WINDOW_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_window_h,"$Id: m_window.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_window.h,v 1.7 2014/10/22 02:33:11 ayoung Exp $
 * Window primitives.
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

extern void                 do_attach_buffer(void);
extern void                 do_borders(void);
extern void                 do_change_window_pos(void);
extern void                 do_close_window(void);
extern void                 do_color_index(void);
extern void                 do_create_menu_window(void);
extern void                 do_create_tiled_window(void);
extern void                 do_create_window(void);
extern void                 do_delete_window(void);
extern void                 do_end_of_window(void);
extern void                 do_next_window(void);
extern void                 do_set_ctrl_state(void);
extern void                 do_set_top_left(void);
extern void                 do_set_window(void);
extern void                 do_set_window_flags(void);
extern void                 do_set_window_priority(void);
extern void                 do_top_of_window(void);
extern void                 do_window_color(void);

extern void                 inq_borders(void);
extern void                 inq_ctrl_state(void);
extern void                 inq_top_left(void);
extern void                 inq_window(void);
extern void                 inq_window_buf(void);
extern void                 inq_window_color(void);
extern void                 inq_window_flags(void);
extern void                 inq_window_info(int extended);
extern void                 inq_window_priority(void);
extern void                 inq_window_size(void);

__CEND_DECLS

#endif /*GR_M_WINDOW_H_INCLUDED*/
