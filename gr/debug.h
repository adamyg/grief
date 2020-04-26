#ifndef GR_DEBUG_H_INCLUDED
#define GR_DEBUG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_debug_h,"$Id: debug.h,v 1.12 2020/04/21 00:01:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: debug.h,v 1.12 2020/04/21 00:01:55 cvsuser Exp $
 * Debugging support.
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

#include <stdarg.h>
#include <edsym.h>
#include <eddebug.h>
#include <edtrace.h>

__CBEGIN_DECLS

/*--export--defines--*/
/*
 *  debug_support() object types
 */
#define DBG_STACK_TRACE     0
#define DBG_NEST_LEVEL      1                   /* Level of nesting of execution stack */

#define DBG_INQ_VARS        2                   /* Return list of variable values.  */
#define DBG_INQ_VAR_INFO    3                   /* Return info about an actual variable. */

#define DBG_INQ_BVARS       4                   /* Return list of buffer variable values. */
#define DBG_INQ_BVAR_INFO   5                   /* Return info about an actual variable. */

#define DBG_INQ_MVARS       6                   /* Return list of moduler variable values. */
#define DBG_INQ_MVAR_INFO   7                   /* Return info about an actual variable. */

#define DBG_INQ_OPCODES     8                   /* OPCODE descriptions */
/*--end--*/


enum _dbflags {
/*--export--enum--*/
/*
 *  Debug flags
 */
    DB_TRACE                =0x00000001,        /* General trace */
    DB_REGEXP               =0x00000002,        /* Regular expression debug output */
    DB_UNDO                 =0x00000004,        /* Undo trace */
    DB_FLUSH                =0x00000008,        /* Flush output */
    DB_TIME                 =0x00000010,        /* Time stamp */
    DB_TERMINAL             =0x00000020,        /* Terminal */
    DB_VFS                  =0x00000040,        /* Virtual filesystem */
    DB_NOTRAP               =0x00000080,        /* Disable SIGBUS/SIGSEGV trap handling */
    DB_MEMORY               =0x00001000,        /* Target specific debug services */
    DB_REFS                 =0x00002000,        /* Variable refs */
    DB_PROMPT               =0x00004000,        /* Debug prompting code */
    DB_PURIFY               =0x00008000,        /* Running under Purify(tm) */
    DB_HISTORY              =0x00010000
/*--end--*/
};

#if defined(DO_TRACE_LINE)
#define ED_TRACE_LINES()    trace_lines();
#define ED_TRACE_LINE(_lx)  trace_line(_lp);
#define ED_TRACE_LINE2(_lp) trace_line2(_lp);
#else
#define ED_TRACE_LINES()
#define ED_TRACE_LINE(_lp)
#define ED_TRACE_LINE2(_lp)
#endif

extern int                  trace_flags(void);
extern int                  trace_flagsset(int flags);
extern void                 trace_ilog(const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 trace_term(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 trace_character(int value, int width, const char *buf, int n);
extern void                 trace_listv(const LISTV *lvp);
extern void                 trace_ilist(const LIST *lp);
extern void                 trace_list(const LIST *lp);
extern void                 trace_lines(void);
extern void                 trace_line(const LINE_t *lp);
extern void                 trace_line2(const LINE_t *lp);
extern void                 trace_sym(const SYMBOL *sp);
extern void                 trace_sym_ref(const SYMBOL *sp);
extern void                 trace_trigger(int type, const char *args);
extern void                 trace_refs(void);

__CEND_DECLS

#endif /*GR_DEBUG_H_INCLUDED*/

