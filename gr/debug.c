#include <edidentifier.h>
__CIDENT_RCSID(gr_debug_c,"$Id: debug.c,v 1.38 2025/06/28 11:08:25 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: debug.c,v 1.38 2025/06/28 11:08:25 cvsuser Exp $
 * internal debug/diagnostics.
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

#include <editor.h>

#include <stdarg.h>
#include <assert.h>

#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#define  __EDDEBUG_INTERNALS
#include "debug.h"

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "symbol.h"
#include "word.h"

int                     x_dflags = 0;           /* global flags */

static void             trace_sym1(const SYMBOL *sp);
static void             trace_list0(const LIST *lp, int level);
static void             trace_string(const char *str);


/*  Function:           trace_flagsset
 *      Retrieve the current diagnostics flags.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Flag value.
 */
int
trace_flags(void)
{
    return x_dflags;
}


/*  Function:           trace_flagsset
 *      Configure the trace diagnostics.
 *
 *  Parameters:
 *      flag -              Configuration flags.
 *
 *  Returns:
 *      Previous flag value.
 */
int
trace_flagsset(int flags)
{
    int odflags = x_dflags;
    x_dflags = flags;
    if (x_dflags) {
        trace_active(DB_FLUSH & x_dflags ? 2 : 1);
    } else {
        trace_active(0);
    }
    return odflags;
}


/*  Function:           trace_ilog
 *      Macro nesting indented diagnostics output.
 *
 *  Parameters:
 *      msg -               Message.
 *      ... -               Parameters.
 *
 *  Returns:
 *      nothing
 */
void
#if defined(_MSC_VER) && defined(_Printf_format_string_)
trace_ilog(_Printf_format_string_ const char *fmt, ...)
#else
trace_ilog(const char *fmt, ...)
#endif
{
    if (x_dflags) {
        va_list ap;
        va_start(ap, fmt);
        trace_logxv(x_nest_level, fmt, ap);
        va_end(ap);
    }
}


/*  Function:           trace_term
 *      Terminal low-level diagnostics output.
 *
 *  Parameters:
 *      msg -               Message.
 *      ... -               Parameters.
 *
 *  Returns:
 *      nothing
 */
void
#if defined(_MSC_VER) && defined(_Printf_format_string_)
trace_term(_Printf_format_string_ const char* fmt, ...)
#else
trace_term(const char *fmt, ...)
#endif
{
    if (DB_TERMINAL & x_dflags) {
        va_list ap;
        va_start(ap, fmt);
        trace_logv(fmt, ap);
        va_end(ap);
    }
}


#if defined(XXX_NOTUSED)
void
trace_listv(const LISTV *lvp)
{
    trace_ilog("lvp=%p ", lvp);
    switch (lvp->l_flags) {
    case F_INT:
        trace_log("F_INT   = %" ACCINT_FMT, lvp->l_int);
        break;
    case F_FLOAT:
        trace_log("F_FLOAT = %" ACCFLOAT_FMT, lvp->l_float);
        break;
    case F_LIT:
        trace_log("F_LIT   = %s", lvp->l_str);
        break;
    case F_STR:
        trace_log("F_STR   = %s", lvp->l_str);
        break;
    case F_LIST:
        trace_log("F_LIST  = %p", lvp->l_list);
        break;
    case F_RLIST:
        trace_log("F_FLIST = %d/%p", r_refs(lvp->l_ref), r_ptr(lvp->l_ref));
        break;
    case F_NULL:
        trace_log("NULL");
        break;
    case F_ID:
        trace_log("F_ID    = %d", (int)lvp->l_int);
        break;
    default:
        trace_log("<dont-know type=%d>", lvp->l_flags);
        break;
    }
    trace_log("\n");
}
#endif


void
trace_ilist(const LIST *lp)
{
    if (x_dflags) {
        trace_list0(lp, -1);
    }
}


void
trace_list(const LIST *lp)
{
    if (x_dflags) {
        trace_list0(lp, 0);
    }
}


static void
trace_ref(const ref_t *rp)
{
    if (DB_REFS & x_dflags) {
        trace_log("*%p/%u->", rp, r_refs(rp));
    }
}


void
trace_character(int value, int width, const char *str, int n)
{
    const char *end = str + n;
    char buf[(32 * 3) + 1];                     /* 32 characters + NUL */
    size_t len = 0;

    buf[0] = '\0';
    if (str) {
        while (str < end) {
            if (len >= (sizeof(buf) - 8)) {
                len += sprintf(buf + len, "...");
                break;
            }
            len += sprintf(buf + len, "%02x ", (unsigned char) *str++);
        }
    }
    trace_log("character(value:%4d/0x%04x: width:%d) == [%d] %s\n",
        value, value, width, n, buf);
}


void
trace_lines(void)
{
    const BUFFER_t *bp;
    LINENO lineno = 0;
    char buf[200];

    trace_log("Dump all Lines\n");
    for (bp = buf_first(); bp; bp = buf_next((BUFFER_t *)bp)) {
        register const LINE_t *lp;

        trace_log("Buffer %s:\n", c_string(bp->b_fname));
        TAILQ_FOREACH(lp, &bp->b_lineq, l_node) {
            sxprintf(buf, sizeof(buf), "\tLine %3u: old=%u, used=%2d, size=%2d, fl=0x%x, chunk=%p\n",
                (unsigned)++lineno, (unsigned)lp->l_oldlineno, (int)lp->l_used, (int)lp->l_size,
                (unsigned)lflags(lp), lp->l_chunk);
            trace_str(buf);
        }
        trace_str("\n");
    }
}


void
trace_line(const LINE_t *lp)
{
    const lineflags_t iflags = liflags(lp);
    char buf[128],  *p = buf;

    if (0 == x_dflags) {
        return;
    }

    p += sxprintf(p, sizeof(buf), "%p: old=%u used=%d size=%d flags=0x%x chunk=%p", lp,
            (unsigned)lp->l_oldlineno, (int)llength(lp), (int)lp->l_size, (unsigned)iflags, lp->l_chunk);

    if (linfile(lp)) {
        p += sprintf(p, " INFILE");
    } else if (lincore(lp)) {
        p += sprintf(p, " text=%p INCORE", lp->l_text);
    }
    if (LI_LOCKED & iflags) {
        strcpy(p, " LOCKED");  p += 7;
    }

    strcpy(p, "\n");
    trace_str(buf);
}


void
trace_line2(const LINE_t *lp)
{
    static char *bline = NULL;
    static int blength = 0;

    const LINECHAR *cp, *start, *end;
    int length;

#if defined(DB_LINE2)
    if (0 == (DB_LINE2 & x_dflags)) {
#else
    if (0 == x_dflags) {
#endif
        return;
    }

    length = llength(lp);
    start = ltext(lp);
    end = start + length;

    trace_line(lp);

    if (start && length) {
        const int alength = (((length * 3) + 32) | 0xff);
        char *bcursor;
        int idx;

        if (blength < alength) {
            char *aline;

            if (NULL == (aline = chk_realloc(bline, alength))) {
                return;
            }
            blength = alength;
            bline = aline;
        }

        bcursor = bline;
        for (idx = 0, cp = start; cp < end; ++cp) {
            bcursor += sprintf(bcursor, "%02x ", (idx++) & 0xff);
        }
        trace_log("line {%s}\n", bline);

        bcursor = bline;
        for (cp = start; cp < end; ++cp) {
            bcursor += sprintf(bcursor, "%02x ", (unsigned)*cp);
        }
        trace_log("line [%s]\n", bline);

        bcursor = bline;
        for (cp = start; cp < end; ++cp) {
            const unsigned ch = *cp;

            switch (ch) {
            case '\a': bcursor += sprintf(bcursor, "\\a "); break;
            case '\b': bcursor += sprintf(bcursor, "\\b "); break;
            case '\n': bcursor += sprintf(bcursor, "\\n "); break;
            case '\r': bcursor += sprintf(bcursor, "\\r "); break;
            case '\t': bcursor += sprintf(bcursor, "\\t "); break;
            case '\v': bcursor += sprintf(bcursor, "\\v "); break;
            case '\f': bcursor += sprintf(bcursor, "\\f "); break;
            case 0x1b: bcursor += sprintf(bcursor, "\\e "); break;
            case 0x0:  bcursor += sprintf(bcursor, "\\0 "); break;
            default:
                if (ch <= 0x7f && isprint(ch)) {
                    bcursor += sprintf(bcursor, "%c  ", ch);
                } else {
                    bcursor += sprintf(bcursor, ".. ");
                }
                break;
            }
        }
        trace_log("line <%s>\n", bline);
    }
}


/*
 *  trace_sym ---
 *      Function to trace assignments to symbols.
 */
void
trace_sym(const SYMBOL *sp)
{
    if (0 == x_dflags) {
        return;
    }
    trace_ilog("  %s := ", sp->s_name);
    trace_sym1(sp);
}


/*
 *  trace_sym_ref ---
 *      Function to trace references to a symbol.
 */
void
trace_sym_ref(const SYMBOL *sp)
{
    if (0 == x_dflags) {
        return;
    }
    trace_ilog("  lookup %s = ", sp->s_name);
    trace_sym1(sp);
}


/*
 *  trace_sym1 ---
 *      Print value of a symbol.
 */
static void
trace_sym1(const SYMBOL *sp)
{
    switch (sp->s_type) {
    case F_INT:
        trace_log("%" ACCINT_FMT "\n", sp->s_int);
        break;
    case F_FLOAT:
        trace_log("%" ACCFLOAT_FMT "\n", sp->s_float);
        break;
    case F_STR:
        trace_log("'%s'\n", c_string(r_ptr(sp->s_obj)));
        break;
    case F_LIST:
        if (sp->s_obj) {
            trace_list((LIST *) r_ptr(sp->s_obj));
        } else {
            trace_log("NIL\n");
        }
        break;
    case F_NULL:
        trace_log("NULL\n");
        break;
    default:
        trace_log("unknown type '%d'", (int)sp->s_type);
        assert(0);
    }
}


void
trace_trigger(int type, const char *args)
{
    static const char *triggers[] = {
        "REG_TYPED",
        "REG_EDIT",
        "REG_ALT_H",
        "REG_UNASSIGNED",
        "REG_IDLE",
        "REG_EXIT",
        "REG_NEW",
        "REG_CTRLC",
        "REG_INVALID",
        "REG_INTERNAL",
        "REG_MOUSE",
        "REG_PROC_INPUT",
        "REG_KEYBOARD",
        "REG_STARTUP",
        "REG_BOOKMARK",
        "REG_INSERT_MODE",
        "REG_BUFFER_MOD",
        "REG_BUFFER_WRITE",
        "REG_BUFFER_RENAME",
        "REG_BUFFER_DELETE",
        "REG_FILE_SAVE",
        "REG_FILE_WRITTEN",
        "REG_FILE_CHANGE",
        "REG_SIGUSR",
        "REG_UNDEFINED_MACRO",
        "REG_REGRESS"
        /*
         *  XXX - auto-generate
         */
        };

    trace_log("*** TRIGGER=%s(%s) ***\n",
        (type >= 0 && type < (int)(sizeof(triggers)/sizeof(triggers[0]))) ?
                triggers[type] : "UNKNOWN", (args ? args : ""));
}


void
trace_refs(void)
{
    if (x_dflags) {
        const BUILTIN *bp, *endbp = builtin + builtin_count;

        for (bp = builtin; bp < endbp; ++bp) {
            trace_log("%5u  %5u  %s\n", bp->b_reference, bp->b_replacement, c_string(bp->b_name));
#if defined(DO_PROFILE)
#endif
        }
    }
}


static void
trace_list0(const LIST *lp, int level)
{
    int tokcnt = 0, depth = 0;
    ref_t *rp;

    if (NULL == lp) {
        if (level <= 0) {
            if (-1 == level) {
                trace_ilog("(nil)\n");
            } else {
                trace_log(" (nil)\n");
            }
            return;
        }
        trace_ilog("nil");
        return;
    }

    if (level <= 0) {
        if (-1 == level) {
            trace_ilog("(");
        } else {
            trace_log(" (");
        }
        level = 0;
    }

    for (;;) {
        if (tokcnt++) {
            trace_log(" ");
        }
        switch (*lp) {
        case F_INT:
            trace_log("%" ACCINT_FMT, (accint_t)LGET_INT(lp));
            break;
        case F_FLOAT:
            trace_log("%" ACCFLOAT_FMT, LGET_FLOAT(lp));
            break;
        case F_LIT:
            trace_string(LGET_PTR2(const char, lp));
            break;
        case F_STR:
            trace_string(LGET_PTR2(const char, lp));
            break;
        case F_RSTR:
            rp = LGET_PTR2(ref_t, lp);
            trace_ref(rp);
            trace_string((const char *) r_ptr(rp));
            break;
        case F_LIST:
            ++depth;
            if (x_dflags & DB_REFS) {
                trace_log("%p->(%u, ", lp, (unsigned)LGET_LEN(lp));
            } else {
                trace_log("(");
            }
            break;
        case F_RLIST:
            rp = LGET_PTR2(ref_t, lp);
            trace_ref(rp);
            if (x_dflags & DB_REFS) {
                trace_log("(%u, ", (unsigned)r_used(rp));
            } else {
                trace_log("(");
            }
            trace_list0((LIST *)(r_ptr(rp)), level+1);
            trace_log(")");
            break;
        case F_HALT:
            if (0 == level)
                trace_str(")");
            if (0 == depth) {
                if (0 == level) {
                    trace_str("\n");
                }
                return;
            }
            --depth;
            break;
        case F_NULL:
            trace_str("NULL");
            break;

        case F_ID: {
                const int id = LGET_ID(lp);
                const char *name = builtin[id].b_name;

                assert(id >= 0 && (unsigned)id < builtin_count);
                trace_str(name);
            }
            break;
        case F_SYM:
            trace_string(LGET_PTR2(const char, lp));
            break;
        case F_REG:
            trace_string(LGET_PTR2(const char, lp));
            break;

        default:
            trace_log("<dont-know type=%u - aborting dump> ", *lp);
            return;
        }
        lp += sizeof_atoms[*lp];
    }
    /*NOTREACHED*/
}


static void
trace_string(const char *str)
{
    trace_log("\"");
    if (str) {
        unsigned char c;

        while ((c = ((unsigned char)*str++)) != 0) {
            switch (c) {
            case '\"': trace_log("\"");  break;
            case '\a': trace_log("\\a"); break;
            case '\b': trace_log("\\b"); break;
            case '\n': trace_log("\\n"); break;
            case '\r': trace_log("\\r"); break;
            case '\t': trace_log("\\t"); break;
            case '\v': trace_log("\\v"); break;
            case '\f': trace_log("\\f"); break;
            case 0x1b: trace_log("\\e"); break;
            default:
                if (isprint(c)) {
                    trace_log("%c", c);
                } else {
                    trace_log("\\%03o", c);
                }
            }
        }
    }
    trace_log("\"");
}

/*end*/
