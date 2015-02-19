#ifndef GR_M_TERMINAL_H_INCLUDED
#define GR_M_TERMINAL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_terminal_h,"$Id: m_terminal.h,v 1.5 2014/10/22 02:33:10 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_terminal.h,v 1.5 2014/10/22 02:33:10 ayoung Exp $
 * Terminal screen and keyboard primitives.
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

extern void                 do_get_term_feature(void);
extern void                 do_get_term_features(void);
extern void                 do_set_term_feature(void);
extern void                 do_set_term_features(void);

extern void                 do_set_term_characters(void);
extern void                 do_get_term_characters(void);
extern void                 do_set_term_keyboard(void);
extern void                 do_get_term_keyboard(void);

__CEND_DECLS

#endif /*GR_M_TERMINAL_H_INCLUDED*/
