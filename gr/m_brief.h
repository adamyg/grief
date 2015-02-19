#ifndef GR_M_BRIEF_H_INCLUDED
#define GR_M_BRIEF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_brief_h,"$Id: m_brief.h,v 1.3 2014/10/22 02:33:00 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_brief.h,v 1.3 2014/10/22 02:33:00 ayoung Exp $
 * Brief compatibility interface; most where introducted within 3.1
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

extern void                 do_del(void);
extern void                 do_dos(void);
extern void                 do_save_keystroke_macro(void);
extern void                 do_set_btn2_action(void);
extern void                 do_set_mouse_action(void);
extern void                 do_set_mouse_type(void);

extern void                 inq_brief_level(void);
extern void                 inq_btn2_action(void);
extern void                 inq_environment(void);
extern void                 inq_mouse_action(void);
extern void                 inq_mouse_type(void);

__CEND_DECLS

#endif /*GR_M_BRIEF_H_INCLUDED*/
