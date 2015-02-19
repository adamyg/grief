#ifndef GR_LOCK_H_INCLUDED
#define GR_LOCK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_lock_h,"$Id: lock.h,v 1.7 2014/10/22 02:33:00 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: lock.h,v 1.7 2014/10/22 02:33:00 ayoung Exp $
 * File locking.
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

extern int                  flock_init(void);
extern void                 flock_close(void);
extern int                  flock_set(const char *file, int ask);
extern void                 flock_clear(const char *file);

__CEND_DECLS

#endif /*GR_LOCK_H_INCLUDED*/
