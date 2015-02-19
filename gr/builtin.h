#ifndef GR_BUILTIN_H_INCLUDED
#define GR_BUILTIN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_builtin_h,"$Id: builtin.h,v 1.24 2014/10/22 02:32:54 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: builtin.h,v 1.24 2014/10/22 02:32:54 ayoung Exp $
 * Builtin primitive table.
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

extern int                  execute_str(const char *str);
extern void                 execute_expr(const LISTV *lp);
extern void                 execute_macro(const LIST *lp);
extern void                 execute_nmacro(const LIST *lp);
extern void                 execute_xmacro(const LIST *lp, const LIST *lp_argv);
extern void                 execute_event_ctrlc(void);
extern void                 execute_event_usr1(void);
extern void                 execute_event_usr2(void);
extern void                 set_hooked(void);

extern LINENO              *cur_line, *cur_col;
extern LINEATTR            *cur_attr;

extern int                  margc;
extern LISTV               *margv;

extern const char          *x_command_name;
extern int                  x_return;
extern void                *x_returns;

__CEND_DECLS

#endif /*GR_BUILTIN_H_INCLUDED*/
