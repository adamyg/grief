#ifndef GR_M_BUF_H_INCLUDED
#define GR_M_BUF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_buf_h,"$Id: m_buf.h,v 1.21 2014/10/22 02:33:00 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_buf.h,v 1.21 2014/10/22 02:33:00 ayoung Exp $
 * Buffer primitives.
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

extern void                 do_change_window(void);
extern void                 do_create_buffer(int nested);
extern void                 do_create_edge(void);
extern void                 do_delete_buffer(void);
extern void                 do_delete_edge(void);
extern void                 do_insert_mode(void);
extern void                 do_move_edge(void);
extern void                 do_next_buffer(int prev);
extern void                 do_restore_position(void);
extern void                 do_save_position(void);
extern void                 do_set_attribute(void);
extern void                 do_set_buffer(void);
extern void                 do_set_buffer_flags(void);
extern void                 do_set_buffer_flags2(void);
extern void                 do_set_buffer_title(void);
extern void                 do_set_buffer_type(void);
extern void                 do_sort_buffer(void);

extern void                 inq_attribute(void);
extern void                 inq_buffer(void);
extern void                 inq_buffer_flags(void);
extern void                 inq_buffer_title(void);
extern void                 inq_buffer_type(void);
extern void                 inq_line_length(void);
extern void                 inq_lines(void);
extern void                 inq_mode(void);
extern void                 inq_modified(void);
extern void                 inq_system(void);
extern void                 inq_time(void);
extern void                 inq_views(void);
extern void                 inq_names(void);

__CEND_DECLS

#endif /*GR_M_BUF_H_INCLUDED*/
