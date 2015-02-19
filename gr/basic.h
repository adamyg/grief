#ifndef GR_BASIC_H_INCLUDED
#define GR_BASIC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_basic_h,"$Id: basic.h,v 1.12 2014/10/22 02:32:52 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: basic.h,v 1.12 2014/10/22 02:32:52 ayoung Exp $
 * Basic cursor movement.
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

extern void                 move_abs(const LINENO nline, const LINENO ncol);
extern void                 move_rel(LINENO line, LINENO col);
extern void                 move_next_char(int n, int tabs);
extern void                 move_prev_char(int n);
extern void                 mov_forwchar(int n);
extern int                  mov_backchar(int n, int wrap);
extern void                 mov_forwline(int n);
extern void                 mov_backline(int n);
extern void                 mov_gotoline(int n);

extern void                 do_beginning_of_line(void);
extern void                 do_down(void);
extern void                 do_end_of_buffer(void);
extern void                 do_end_of_line(void);
extern void                 do_left(void);
extern void                 do_move_abs(void);
extern void                 do_move_rel(void);
extern void                 do_next_char(void);
extern void                 do_page_down(void);
extern void                 do_page_up(void);
extern void                 do_prev_char(void);
extern void                 do_right(void);
extern void                 do_top_of_buffer(void);
extern void                 do_up(void);
extern void                 inq_position(void);

__CEND_DECLS

#endif /*GR_BASIC_H_INCLUDED*/
