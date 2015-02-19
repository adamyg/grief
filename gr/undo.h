#ifndef GR_UNDO_H_INCLUDED
#define GR_UNDO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_undo_h,"$Id: undo.h,v 1.15 2014/10/22 02:33:24 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: undo.h,v 1.15 2014/10/22 02:33:24 ayoung Exp $
 * undo and redo facilities.
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

extern void                 undo_init(void);
extern void                 undo_close(void);

extern void                 u_replace(const char *str, FSIZE_t del, FSIZE_t ins);
extern FSIZE_t              u_insert(FSIZE_t cnt, int dot);
extern void                 u_delete(FSIZE_t cnt);
extern void                 u_chain(void);
extern void                 u_terminate(void);
extern void                 u_soft_start(void);
extern void                 u_dot(void);
extern void                 u_raise(void);
extern void                 u_drop(void);
extern void                 u_anchor(void);
extern void                 u_scrap(void);

extern void                 do_undo(int count);
extern void                 do_redo(void);

__CEND_DECLS

#endif /*GR_UNDO_H_INCLUDED*/
