#ifndef GR_M_MSG_H_INCLUDED
#define GR_M_MSG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_msg_h,"$Id: m_msg.h,v 1.16 2014/10/22 02:33:05 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_msg.h,v 1.16 2014/10/22 02:33:05 ayoung Exp $
 * Message and formatting primitives.
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

extern void                 do_message(void);
extern void                 do_error(void);
extern void                 do_print(void);
extern void                 do_printf(void);
extern void                 do_dprintf(void);
extern void                 do_sprintf(void);
extern void                 do_format(void);
extern void                 do_pause_on_error(void);
extern void                 do_pause_on_message(void);

__CEND_DECLS

#endif /*GR_M_MSG_H_INCLUDED*/
