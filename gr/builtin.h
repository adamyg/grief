#ifndef GR_BUILTIN_H_INCLUDED
#define GR_BUILTIN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_builtin_h,"$Id: builtin.h,v 1.26 2021/10/18 13:20:42 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: builtin.h,v 1.26 2021/10/18 13:20:42 cvsuser Exp $
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
extern const char *         execute_name(void);

extern void                 set_hooked(void);

extern LINENO              *cur_line, *cur_col;
extern LINEATTR            *cur_attr;

extern unsigned             mac_sd;             /* Macro stack depth */
extern struct mac_stack    *mac_sp;             /* Macro stack point (== mac_stack[mac_sd-1]) */
extern struct mac_stack     mac_stack[];        /* Macro name stack */

extern int                  margc;
extern const LISTV         *margv;
    //extern const char          *mname;
extern unsigned             mexecflags;

extern void                *x_returns;

#define JF_BREAK                0x001
#define JF_RETURN               0x002
#define JF_CONTINUE             0x004
#define JF_SIGNAL               0x100

#define set_break()         (mexecflags |= JF_BREAK)
#define set_return()        (mexecflags |= JF_RETURN)
#define set_continue()      (mexecflags |= JF_CONTINUE)
#define set_signal()        (mexecflags |= JF_SIGNAL)

#define clear_break()       (mexecflags &= ~JF_BREAK)
#define clear_return()      (mexecflags &= ~JF_RETURN)
#define clear_break_continue() (mexecflags &= ~(JF_BREAK|JF_CONTINUE))
#define clear_signal()      (mexecflags &= ~JF_SIGNAL)

#define is_break()          (mexecflags & JF_BREAK)
#define is_return()         (mexecflags & JF_RETURN)
#define is_continue()       (mexecflags & JF_CONTINUE)
#define is_signal()         (mexecflags & JF_SIGNAL)

#define is_interrupt()      (0 != (mexecflags & (JF_BREAK|JF_RETURN|JF_SIGNAL)))
#define is_breakreturn()    (0 != (mexecflags & (JF_BREAK|JF_RETURN)))

#define not_breakreturn()   (0 == (mexecflags & (JF_BREAK|JF_RETURN)))

__CEND_DECLS

#endif /*GR_BUILTIN_H_INCLUDED*/
