#ifndef GR_REGISTER_H_INCLUDED
#define GR_REGISTER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_register_h,"$Id: register.h,v 1.15 2014/10/22 02:33:17 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: register.h,v 1.15 2014/10/22 02:33:17 ayoung Exp $
 * Event management.
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

extern void                 register_init(void);
extern void                 register_shutdown(void);
extern void                 register_attach(BUFFER_t *bp);
extern void                 register_detach(BUFFER_t *bp);

extern int                  trigger(int type);
extern int                  triggerx(int type, const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 2, 3));
extern void                 trigger_idle(void);

extern void                 do_call_registered_macro(void);
extern void                 do_register_macro(int unique);
extern void                 do_unregister_macro(void);
extern void                 do_set_idle_default(void);
extern void                 inq_idle_default(void);
extern void                 inq_idle_time(void);

extern accint_t             xf_interval;
extern time_t               x_time_last_key;
extern int                  x_trigger_level;

__CEND_DECLS

#endif /*GR_REGISTER_H_INCLUDED*/
