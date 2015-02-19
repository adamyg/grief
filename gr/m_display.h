#ifndef GR_M_DISPLAY_H_INCLUDED
#define GR_M_DISPLAY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_display_h,"$Id: m_display.h,v 1.8 2014/10/22 02:33:02 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_display.h,v 1.8 2014/10/22 02:33:02 ayoung Exp $
 * Display primitives.
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

extern void                 do_beep(void);
extern void                 do_cursor(void);
extern void                 do_display_mode(void);
extern void                 do_set_font(void);
extern void                 do_inq_font(void);
extern void                 do_set_wm_name(void);
extern void                 do_display_windows(void);
extern void                 do_screen_dump(void);

extern void                 inq_display_mode(void);
extern void                 inq_screen_size(void);

__CEND_DECLS

#endif /*GR_M_DISPLAY_H_INCLUDED*/
