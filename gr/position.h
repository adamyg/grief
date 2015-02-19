#ifndef GR_POSITION_H_INCLUDED
#define GR_POSITION_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_position_h,"$Id: position.h,v 1.4 2014/10/22 02:33:14 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: position.h,v 1.4 2014/10/22 02:33:14 ayoung Exp $
 * Buffer position/status.
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

extern void                 position_init(void);
extern void                 position_dump(void);
extern void                 position_shutdown(void);
extern void                 position_save(void);
extern int                  position_restore(int what);

__CEND_DECLS

#endif /*GR_POSITION_H_INCLUDED*/
