#ifndef GR_PROCSPAWN_H_INCLUDED
#define GR_PROCSPAWN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_procspawn_h,"$Id: procspawn.h,v 1.7 2014/10/22 02:33:16 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: procspawn.h,v 1.7 2014/10/22 02:33:16 ayoung Exp $
 * Process spawn primitive and management.
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

extern void *               proc_add(int pid, const char *macro, void (*cleanup)(void *), const void *udata, int usize);
extern void                 proc_check(void);
extern int                  proc_wait(int pid);
extern int                  proc_enum(int (*callback)(int pid, void *), void *udata);
extern int                  proc_find(int pid, void **udata);

extern const char *         proc_shell_get(void);
extern int                  proc_shell_iscmd(const char *shell);

extern void                 proc_prep_stop(int repaint);
extern void                 proc_prep_start(void);

extern void                 do_shell(void);

extern int                  x_background;

__CEND_DECLS

#endif /*GR_PROCSPAWN_H_INCLUDED*/
