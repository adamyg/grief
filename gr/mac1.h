#ifndef GR_MAC1_H_INCLUDED
#define GR_MAC1_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_mac1_h,"$Id: mac1.h,v 1.26 2020/04/21 00:01:57 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mac1.h,v 1.26 2020/04/21 00:01:57 cvsuser Exp $
 * Basic primitives.
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

extern void                 do_backspace(void);
extern void                 do_break(void);
extern void                 do_breaksw(void);
extern void                 do_continue(void);
extern void                 do_delete_char(void);
extern void                 do_delete_line(void);
extern void                 do_delete_to_eol(void);
extern void                 do_do(void);
extern void                 do_execute_macro(void);
extern void                 do_lexicalblock(void);
extern void                 do_first_time(void);
extern void                 do_for(void);
extern void                 do_foreach(void);
extern void                 do_goto_line(void);
extern void                 do_goto_old_line(void);
extern void                 do_grief_version(void);
extern void                 do_if(void);
extern void                 do_input_mode(void);
extern void                 do_insert(int proc);
extern void                 do_insert_buffer(void);
extern void                 do_insertf(void);
extern void                 do_nothing(void);
extern void                 do_rand(void);
extern void                 do_read(void);
extern void                 do_redraw(void);
extern void                 do_refresh(void);
extern void                 do_returns(int returns);
extern void                 do_self_insert(void);
extern void                 do_set_msg_level(void);
extern void                 do_sleep(void);
extern void                 do_srand(void);
extern void                 do_switch(void);
extern void                 do_throw(void);
extern void                 do_try(void);
extern void                 do_unimp(void);
extern void                 do_version(void);
extern void                 do_while(void);
extern void                 inq_msg_level(void);

extern int                  x_selfinsert;

__CEND_DECLS

#endif /*GR_MAC1_H_INCLUDED*/
