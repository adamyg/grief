#include <edidentifier.h>
__CIDENT_RCSID(gr_builtin_c,"$Id: builtin.c,v 1.53 2014/11/16 17:28:37 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: builtin.c,v 1.53 2014/11/16 17:28:37 ayoung Exp $
 * Builtin expresssion evaluation.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_... */
#include "builtin.h"
#include "cmap.h"                               /* cur_cmap */
#include "debug.h"
#include "echo.h"
#include "eval.h"                               /* get_str() ... */
#include "keywd.h"
#include "lisp.h"
#include "mac1.h"                               /* x_break */
#include "macros.h"
#include "main.h"
#include "register.h"
#include "symbol.h"                             /* sym_... */
#include "tty.h"
#include "word.h"                               /* PUT/GET */

#define EERROR          -1
#define EEXECUTE        -2

struct saved {                                  /* Save argument references */
    OPCODE      save_type;
    void       *save_ptr;
};


static void             execute_event(int event);
static void             execute_builtin(BUILTIN *bp, const LIST *lp);
static void             arg_error(BUILTIN *bp, int msg, struct saved *saved_str, struct saved *ssp, int arg);
static void             arg_free(struct saved *saved_str, struct saved *ssp);
static int              execute_expr2(argtype_t arg, const LIST *argp, LISTV *lap);

#define REGEVTNUM       32                      /* Event queue size */


static unsigned         x_evtno = 0;
static unsigned         x_evttail = 0;
static unsigned         x_evthead = 0;
static unsigned         x_evtactive = FALSE;
static unsigned         x_evtctrlc = 0;         /* SIGINT count */
static unsigned         x_evtqueue[REGEVTNUM];  /* Event queue */

static LISTV            x_margv[MAX_ARGC];      /* Initial stack frame */

int                     x_return = FALSE;       /* Set to TRUE when a 'return' is executed. */
void *                  x_returns = NULL;       /* Assigned to the returns() value */
void *                  x_exception = NULL;     /* Try/catch() exception */
const char *            x_command_name = NULL;  /* Name of current macro primitive */

LINENO *                cur_line = NULL;        /* Current line reference */
LINENO *                cur_col  = NULL;        /* Current column reference */
LINEATTR *              cur_attr = NULL;        /* Attribute reference */

LISTV *                 margv = x_margv;        /* Argument vector */
int                     margc = 0;


/*
 *  execute_str ---
 *      Function to take a string like what the user can type at the
 *      command prompt,  perform  a simple parse on it and execute the
 *      specified macro
 */
int
execute_str(const char *str)
{
    register const char *cp = str;
    char buf[MAX_CMDLINE * 2];                  /* MAGIC */
    int dp = 0;

    LIST tmpl[LIST_SIZEOF(128)], *lp = tmpl;    /* 128 atoms -- ~1k */

    while (isspace(*cp)) {
        ++cp;
    }
    if (*cp == '\0') {
        return 0;
    }

    lp = atom_push_sym(tmpl, buf + dp);         /* macro name */

    while (*cp && !isspace(*cp)) {
        if (dp >= (int)sizeof(buf)-2) {
            ewprintf("Out of space in execute_str(1)");
            return -1;
        }
        buf[dp++] = *cp++;
    }
    buf[dp++] = '\0';

    while (*cp) {
        while (isspace(*cp)) {
            ++cp;
        }

        if ('\0' == *cp)  {
            break;
        }

        if (lp >= (tmpl + sizeof(tmpl))) {
            ewprintf("Out of space in execute_str(2)");
            return -1;
        }

        if ((*cp >= '0' && *cp <= '9') ||
                (*cp == '-' && (cp[1] >= '0' && cp[1] <= '9')) ||
                (*cp == '.' && (cp[1] >= '0' && cp[1] <= '9')))
        {
            accfloat_t fval;
            accint_t ival;
            int ret, len = 0;

            ret = str_numparse(cp, &fval, &ival, &len);
            switch (ret) {                      
            case NUMPARSE_INTEGER:              /* integer-constant */
            case NUMPARSE_FLOAT:                /* float-constant */
                cp += len;
                if (0 == *cp || isspace(*cp)) {
                    if (ret == NUMPARSE_INTEGER) {
                        lp = atom_push_int(lp, ival);
                    } else {
                        lp = atom_push_float(lp, fval);
                    }
                    break;
                }
                cp -= len;
                /*FALLTHRU*/
            default:                            /* bad number */
                goto string;
            }

        } else {
            register const char *cp1;
            char term;

string:;    term = ' ';
            if (*cp == '"') {                   /* quoted string? */
                term = '"', ++cp;
            }
            cp1 = cp;

            while (*cp) {
                if (*cp == '\\') {              /* escaped? */
                    ++cp;
                } else if (*cp == term) {       /* terminator? */
                    break;
                }
                ++cp;
            }

            lp = atom_push_const(lp, buf + dp); /* copy sub-string */
            for (cp = cp1; *cp && *cp != term;) {
                if (dp >= (int)(sizeof(buf) - 2)) {
                    ewprintf("Out of space in execute_str(3)");
                    return -1;
                }
                buf[dp++] = *cp++;
            }
            buf[dp++] = '\0';

            if (*cp) {                          /* consume terminator */
                ++cp;
            }
        }
    }

    atom_push_halt(lp);
    execute_nmacro(tmpl);
    return 0;
}


void
execute_expr(const LISTV *lp)
{
    switch (lp->l_flags) {
    case F_INT:
        acc_assign_int(lp->l_int);
        break;

    case F_LIST:
        execute_macro((const LIST *) lp->l_list);
        break;

    case F_RLIST:
        execute_macro((const LIST *) r_ptr(lp->l_ref));
        break;

    default:
        panic("execute_expr: Internal evaluation error %d?", lp->l_flags);
        ewprintf("execute_expr: Internal evaluation error");
        break;
    }
}


void
execute_nmacro(const LIST *lp)
{
    if (++x_nest_level >= MAX_NESTING) {
        panic("Macro nesting overflow.");
    }
    execute_macro(lp);
    sym_local_delete();
}


void
execute_event_ctrlc(void)
{
    execute_event(REG_CTRLC);
//  ++x_evtctrlc;
}


void  
execute_event_usr1(void)
{
    execute_event(REG_SIGUSR1);
}


void                 
execute_event_usr2(void)
{
    execute_event(REG_SIGUSR2);
}


static void
execute_event(int event)
{
    if (++x_evtno <= REGEVTNUM) {
        const unsigned tail = (x_evttail++ % REGEVTNUM);
        x_evtqueue[ tail ] = event;
        return;
    }
    --x_evtno;
}


void
execute_macro(const LIST *lp)
{
    static int handling_ctrlc = FALSE;

    while (1) {
        switch (*lp) {
        case F_LIST:
            if (x_evtno) {
                if (0 == x_evtactive++) {
                    while (x_evtno) {
                        /*
                        *  REG_SIGUSR1, REG_SIGUSR2, REG_SIGCTRLC ...
                        */
                        const unsigned head = (x_evthead++ % REGEVTNUM),
                            event = x_evtqueue[head];
                        --x_evtno;

                        switch (event) {
                        case REG_CTRLC:
                            if (! handling_ctrlc) {
                                handling_ctrlc = TRUE;
                                trigger(event);
                                handling_ctrlc = FALSE;
                            }
                            break;
                        default:
                            trigger(event);
                            break;
                        }
                    }
                    --x_evtactive;
                }
            }

//OLD
//          if (x_evtctrlc && !handling_ctrlc) {
//              handling_ctrlc = TRUE;
//              trigger(REG_CTRLC);
//              x_evtctrlc = FALSE;
//              handling_ctrlc = FALSE;
//          }

            execute_macro(lp + sizeof_atoms[*lp]);
            if (x_break || x_return) {
                return;
            }
            lp += LGET_LEN(lp);
            continue;

        case F_HALT:
            return;

        default:
            trace_ilist(lp);
            execute_xmacro(lp, lp + sizeof_atoms[*lp]);
            return;
        }
    }
}


void
execute_xmacro(register const LIST *lp, const LIST *lp_argv)
{
    const char *macro = NULL;
    register BUILTIN *bp = NULL;
    register MACRO *mptr = NULL;
    MACRO *saved_macro = NULL;
    int ret, omsglevel = 0;

    /*
     *  Locate
     */
    switch (*lp) {
    case F_ID: {            /* builtin */
            const int id = LGET_ID(lp);
            assert(id >= 0 || (unsigned)id < builtin_count);
            bp = builtin + id;
            macro = bp->b_name;
        }
        break;

    case F_INT:             /* integer-constant */
        acc_assign_int(LGET_INT(lp));
        return;

    case F_FLOAT:           /* float-constant */
        acc_assign_float(LGET_FLOAT(lp));
        return;

    case F_LIT:             /* string literal */
        acc_assign_str(LGET_PTR2(const char, lp), -1);
        return;

    case F_STR:             /* symbol */
        macro = LGET_PTR2(const char, lp);
        bp = builtin_lookup(macro);
        break;

    case F_RSTR:            /* symbol */
        macro = r_ptr(LGET_PTR2(ref_t, lp));
        bp = builtin_lookup(macro);
        panic("execute_xmacro: RSTR??");
        break;

    default:                /* unsupported/unexpected */
        panic("execute_xmacro: type? (%d)", *lp);
        break;
    }

    /*
     *  Builtin?
     */
    if (bp) {
        assert(BUILTIN_MAGIC == bp->b_magic);

        if (B_REDEFINE & bp->b_flags) {
            if (NULL == bp->b_macro) {          /* end of call chain, reset on next call */
                bp->b_macro = bp->b_first_macro;
                goto exec_builtin;
            }

            if (bp->b_macro == bp->b_first_macro) {
                bp->b_ovargv = lp_argv;         /* first call, save argument reference */
            }

            mptr = saved_macro = bp->b_macro;   /* setup 4 next recursive calls */
            bp->b_macro = bp->b_macro->m_next;
            macro = bp->b_name;
            goto exec_macro;
        }
exec_builtin:
        execute_builtin(bp, lp_argv);
        bp->b_ovargv = NULL;
        return;
    }

    /*
     *  Lookup-defined macros
     */
    assert(macro);
    if (NULL == (mptr = macro_lookup(macro))) {
        if (0 == macro_load(macro)) {
            mptr = macro_lookup(macro);
        }
    }

    if (NULL == mptr) {
undefined_macro:
#if (XXX_LISTEXEC)
        {                                       /* XXX - implicited list execution */
            SYMBOL *sp;
            if (NULL != (sp = sym_lookup(str))) {
                if (sp->s_type == F_FLIST) {
                    /*
                     *  create an unnamed function and execute??
                     *      issues under which module, scoping problems.
                     */
                    LIST *lp = ((ref_t *)sp->s_obj)->r_ref;

                    if (F_ID == *lp && MACRO == lp[1]) {
                        mptr = macro_unnamed(lp);
                        goto exec_macro;
                    }
                }
        }
#endif

        triggerx(REG_UNDEFINED_MACRO, "\"%s\"", macro);
        if (FALSE == x_mflag) {
            errorf("%s undefined.", macro);
        }
        return;
    }

    /*
     *  Execute macro
     */
exec_macro:
    if ((ret = macro_autoload(mptr, TRUE)) < 0) {
        if (bp) {                               /* autoload failed */
            goto exec_builtin;
        }
        goto undefined_macro;

    } else if (0 == ret) {
        assert(mptr == macro_lookup(macro));
        mptr = macro_lookup(macro);
        assert(mptr != NULL);
        assert(0 == (mptr->m_flags & M_AUTOLOAD));

    } else {
        assert(1 == ret);
    }

    lp = mptr->m_list;
    if (F_HALT == *lp) {
        return;                                 /* empty/null macro */
    }

    assert(ms_cnt >= 0);
    if (ms_cnt >= MAX_NESTING) {
        panic("Macro stack overflow (%d).", ms_cnt);
    } else {
        struct mac_stack *stack = mac_stack + ms_cnt++;

        stack->module = mptr->m_module;
        stack->name   = mptr->m_name;
        stack->caller = NULL;
        stack->argv   = lp_argv;
        stack->argc   = -1;	                /* future use */
        stack->level  = x_nest_level;
    }
    omsglevel = x_msglevel;                     /* message level */
	
    if ((M_STATIC & mptr->m_flags) || '$' != mptr->m_module[0]) {
        trace_log("Execute macro: %s::%s\n", mptr->m_module, mptr->m_name);
    } else {
        trace_log("Execute macro: ::%s\n", mptr->m_name);
    }

#if (XXX_PROFILE)
    timestart(&timer)
#endif
    execute_nmacro(lp);
#if (XXX_PROFILE)
    mptr->m_usage += timerdiff(&timer);
    ++mptr->m_hits;
#endif

    if (bp) {
        assert(mptr == saved_macro);
        bp->b_macro = saved_macro;
    }

    mptr->m_ftime = FALSE;                      /* first time */
    x_msglevel = omsglevel;                     /* restore message level */
    --ms_cnt;
    assert(ms_cnt >= 0);
}


static void
execute_builtin(register BUILTIN *bp, register const LIST *lp)
{
    register const argtype_t *bp_argp = bp->b_arg_types;
    int bp_argc = bp->b_argc;
    register int largc = 1;
    int indefinite_args = 0;
    const LIST *ovargv = bp->b_ovargv;
    struct saved t_saved_str[MAX_ARGC];
    struct saved *saved_str = t_saved_str;
    struct saved *ssp;
    LISTV t_largv[MAX_ARGC];
    LISTV *largv_dynamic = NULL;
    LISTV *largv = t_largv;
    register LISTV *lap;
    int type;

    ssp = saved_str;
    lap = largv+1;

    /*
     *  no arguments list, use original argument list (if any).
     */
    ++bp->b_reference;
    if (ovargv) {
        if (lp == ovargv) {
            ovargv = NULL;                      /* primary execution */
        } else {                                /* replacement call */
            ++bp->b_replacement;
            if (F_HALT == *lp && ovargv) {
                lp = ovargv;
                ovargv = NULL;
            }
        }
    }

    /*
     *  If the number of valid arguments for a command is < 0, then take the
     *  absolute value, and remember how many arguments are specified.
     *
     *  This syntax is used to mean that the same as '...' in ANSI C,
     *  i.e. allow the last argument type to repeat indefinitely.
     */
    if (bp_argc < 0) {
        bp_argc = -bp_argc;
        indefinite_args = -1;
    }

    if (bp_argc > MAX_ARGC) {                   /* guard system limits */
        ewprintf("%s: parameter definition too large", bp->b_name);
        arg_error(bp, FALSE, saved_str, ssp, 0);
        return;
    }

    /*
     *  Keep executing following loop until we run out of arguments passed by
     *  the macro, or we've processed all the arguments needed by the primitive
     */
    while (bp_argc > 0 && F_HALT != *lp) {
        LIST atom = F_HALT;                     /* temporary atom register */

        /*
         *  decode original argument
         */
        const LIST *oparg = NULL;

        if (ovargv) {
            atom = *ovargv;                     /* current atom */
            if (F_HALT != atom) {
                oparg = ovargv;
                if (F_LIST == atom) {
                    ovargv += LGET_LEN(ovargv);
                } else {
                    assert(atom < F_MAX);
                    ovargv += sizeof_atoms[atom];
                }
            } else {
                ovargv = NULL;
            }
        }

        /*
         *  encode and validate argument against parameter list
         */
        ++largc;

        if (largc > MAX_ARGC) {                 /* guard system limits */
            if (indefinite_args) {
                /*
                 *   Dynamic argument list sizing
                 */
                const int sspi = (int)(ssp - saved_str);
                const int lapi = (int)(lap - largv);

                if (-1 == indefinite_args) {
                    indefinite_args = MAX_ARGC * 2;
                    largv_dynamic = chk_calloc(indefinite_args, indefinite_args * sizeof(LISTV));
                    saved_str = chk_calloc(indefinite_args, sizeof(struct saved));
                    largv = largv_dynamic;

                    memcpy(largv, t_largv, sizeof(t_largv));
                    memcpy(saved_str, t_saved_str, sizeof(t_saved_str));

                } else if (largc > indefinite_args) {
                    indefinite_args *= 2;
                    largv_dynamic = chk_realloc(largv_dynamic, indefinite_args * sizeof(LISTV));
                    saved_str = chk_realloc(saved_str, indefinite_args * sizeof(struct saved));
                    largv = largv_dynamic;
                }
                ssp = saved_str + sspi;
                lap = largv + lapi;

            } else {
                ewprintf("%s: parameter number too large", bp->b_name);
                arg_error(bp, FALSE, saved_str, ssp, 0);
                return;
            }
        }

        if (0 == (*bp_argp & ARG_REST) && F_NULL == *lp) {
            if (oparg) {
                type = execute_expr2(*bp_argp, oparg, lap);

            } else {
                if (0 == (*bp_argp & ARG_OPT)) {
                    arg_error(bp, TRUE, saved_str, ssp, lap - largv);
                    return;
                }
                lap->l_int = 0;
                lap->l_flags = F_NULL;
                type = F_HALT;
            }
        } else {
            type = execute_expr2(*bp_argp, lp, lap);
        }

        switch (type) {
        case EEXECUTE:
            goto execute;

        case F_INT:
        case F_FLOAT:
        case F_NULL:
        case F_HALT:
        case F_LIT:
            break;

        case F_STR:
            lap->l_str = ssp->save_ptr = chk_salloc(lap->l_str);
            ssp->save_type = F_STR;
            ++ssp;
            break;

        case F_RLIST:
        case F_RSTR:
            ssp->save_ptr = r_inc(lap->l_ref);
            ssp->save_type = type;
            lap->l_flags = type;
            ++ssp;
            break;

        case F_LIST:
            ssp->save_ptr = lst_clone(lap->l_list, NULL);
            ssp->save_type = F_LIST;
            lap->l_flags = F_LIST;
            ++ssp;
            break;

        case EERROR:
            /*
             *  One last chance --
             *      if we wanted an int-value but we have a float then do a cast for the user.
             */
            if ((*bp_argp & (ARG_LVAL | ARG_NUM)) == ARG_INT && lap->l_flags == F_FLOAT) {
                lap->l_flags = F_INT;
                lap->l_int = (accint_t) lap->l_float;
                break;
            }
            arg_error(bp, TRUE, saved_str, ssp, lap - largv);
            return;

        default:
            ewprintf("%s: default case (type=%d)", bp->b_name, type);
            panic("default case (%d/%x)", type, type);
        }

        /*
         *  Skip to next argument descriptor. Don't skip to next one if we have
         *  an indefinite argument list and this is the last descriptor.
         */
        if (! indefinite_args || 1 != bp_argc) {
            --bp_argc;
            ++bp_argp;
        }
        ++lap;

        /*
         *  move cursor 'lp', defaulting to 'ovargv' if end-of-list
         *  Unsure whether Crisp compatible.
         */
        lp = atom_next(lp);
        if (F_HALT == *lp) {
            if (ovargv) {
                lp = ovargv;
                ovargv = NULL;
            }
        }

        /*
         *  Skip rest of arguments if we are executing a return.
         *  This can happen for example on a symbol being undefined.
         */
        if (x_return) {
            arg_free(saved_str, ssp);
            return;
        }
    }

    if (F_HALT != *lp) {
        ewprintf("%s: too many arguments", bp->b_name);
        arg_error(bp, FALSE, saved_str, ssp, 0);
        return;
    }

    /*
     *  If user hasn't specified enough arguments then complain if any of
     *  the arguments are mandatory.
     */
    while (bp_argc-- > 0) {
        if (0 == (*bp_argp++ & ARG_OPT)) {
            ewprintf("%s: parameter(s) missing", bp->b_name);
            arg_error(bp, FALSE, saved_str, ssp, lap - largv);
            return;
        }
        lap->l_flags = F_NULL;
        lap->l_int = 0;
        ++lap;
    }

execute: {
        LISTV *saved_argv = margv;              /* push stack frame */
        int saved_argc = margc;

        margv = largv;
        margc = largc;
        set_hooked();                           /* why??? - create set_curwp/curbp functions */

#if defined(DO_PROFILE)
        if (x_profile) {
            timer_start(&timer);
        }
#endif
        x_command_name = bp->b_name;
        (*bp->b_func)(bp->b_parameter);
#if defined(DO_PROFILE)
        if (x_profile) {
            bp->b_profile += timer_end(&timer);
        }
#endif
        if (x_dflags && 0 == (bp->b_flags & B_NOVALUE)) {
            acc_trace();
        }

        arg_free(saved_str, ssp);
        if (largv_dynamic) {                    /* release dynamic storage */
            assert(largv == largv_dynamic);
            assert(saved_str != t_saved_str);
            chk_free(largv_dynamic);
            chk_free(saved_str);
        }
        margv = saved_argv;                     /* pop stack frame */
        margc = saved_argc;
    }
}


static void
arg_error(BUILTIN *bp, int msg, struct saved *saved_str, struct saved *ssp, int arg)
{
    if (msg) {
        errorf("%s: parameter %d invalid", bp->b_name, arg);
    }
    arg_free(saved_str, ssp);
}


static void
arg_free(register struct saved *saved_str, register struct saved *ssp)
{
    while (ssp > saved_str) {
        switch ((--ssp)->save_type) {
        case F_STR:
            chk_free((char *) ssp->save_ptr);
            break;

        case F_LIST:
            lst_free((LIST *) ssp->save_ptr);
            break;

#if defined(F_ARRAY)
        case F_ARRAY:
            assoc_free((HASH *) ssp->save_ptr);
            break;
#endif

        case F_RLIST:
        case F_RSTR:
#if defined(F_RARRAY)
        case F_RARRAY:
#endif
            r_dec((ref_t *) ssp->save_ptr);
            break;

        default:
            break;
        }
    }
}


/*
 *  Represents possible type-conversions, where
 *
 *      l = ARG_INT     (0x0001)
 *      f = ARG_FLOAT   (0x0002)
 *      s = ARG_STRING  (0x0004)
 *      l = ARG_LIST    (0x0008)
 */
static const int state_tbl[][13] = {
    /*        EERROR HALT F_INT, F_STR, F_LIST, F_NULL, F_ID, F_END, POLY, F_LIT, F_RSTR, F_FLOAT, F_RLIST*/
    /*----*/ {-1,   -1,  -1,    -1,    -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1,      -1     },
    /*---i*/ {-1,   -1,  F_INT, -1,    -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1,      -1     },
    /*--f-*/ {-1,   -1,  -1,    -1,    -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_FLOAT, -1     },
    /*--fi*/ {-1,   -1,  F_INT, -1,    -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_FLOAT, -1     },
    /*-s--*/ {-1,   -1,  -1,    F_STR, -1,     -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, -1,      -1     },
    /*-s-i*/ {-1,   -1,  F_INT, F_STR, -1,     -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, -1,      -1     },
    /*-sf-*/ {-1,   -1,  -1,    F_STR, -1,     -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, F_FLOAT, -1     },
    /*-sfi*/ {-1,   -1,  F_INT, F_STR, -1,     -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, F_FLOAT, -1     },
    /*l---*/ {-1,   -1,  -1,    -1,    F_LIST, -1,     -1,   -1,    -1,   -1,    -1,     -1,      F_RLIST},
    /*l--i*/ {-1,   -1,  F_INT, -1,    F_LIST, -1,     -1,   -1,    -1,   -1,    -1,     -1,      F_RLIST},
    /*l-f-*/ {-1,   -1,  -1,    -1,    F_LIST, -1,     -1,   -1,    -1,   -1,    -1,     F_FLOAT, F_RLIST},
    /*l-fi*/ {-1,   -1,  F_INT, -1,    F_LIST, -1,     -1,   -1,    -1,   -1,    -1,     F_FLOAT, F_RLIST},
    /*ls--*/ {-1,   -1,  -1,    F_STR, F_LIST, -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, -1,      F_RLIST},
    /*ls-i*/ {-1,   -1,  F_INT, F_STR, F_LIST, -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, -1,      F_RLIST},
    /*lsf-*/ {-1,   -1,  -1,    F_STR, F_LIST, -1,     F_ID, -1,    -1,   F_LIT, F_RSTR, F_FLOAT, F_RLIST},
    /*lsfi*/ {-1,   -1,  F_INT, F_STR, F_LIST, F_NULL, F_ID, -1,    -1,   F_LIT, F_RSTR, F_FLOAT, F_RLIST},
    };


static const int state2_tbl[][11] = {
    /*        HALT  F_INT,  F_STR,  F_LIST, F_NULL, F_ID, F_END, POLY, F_LIT, F_RSTR, F_FLOAT*/
    /*----*/ {-1,   -1,     -1,     -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*---i*/ {-1,   F_HALT, -1,     -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*--f-*/ {-1,   -1,     -1,     -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*--fi*/ {-1,   F_HALT, -1,     -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*-s--*/ {-1,   -1,     F_HALT, -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*-s-i*/ {-1,   F_HALT, F_HALT, -1,     -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*-sf-*/ {-1,   -1,     F_HALT, -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*-sfi*/ {-1,   F_HALT, F_HALT, -1,     -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*l---*/ {-1,   -1,     -1,     F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*l--i*/ {-1,   F_HALT, -1,     F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*l-f-*/ {-1,   -1,     -1,     F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*l-fi*/ {-1,   F_HALT, -1,     F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*ls--*/ {-1,   -1,     F_HALT, F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*ls-i*/ {-1,   F_HALT, F_HALT, F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     -1    },
    /*lsf-*/ {-1,   -1,     F_HALT, F_HALT, -1,     -1,   -1,    -1,   -1,    -1,     F_HALT},
    /*lsfi*/ {-1,   F_HALT, F_HALT, F_HALT, F_HALT, -1,   -1,    -1,   -1,    -1,     F_HALT},
    };


static int
execute_expr2(argtype_t arg, const LIST *argp, register LISTV *lap)
{
    SYMBOL *sp;

    switch (arg & (ARG_REST | ARG_COND | ARG_LVAL)) {
    case ARG_LVAL: {
            /*
             *  pass by reference
             */
            int type;

            if (F_STR != *argp) {
                if (F_NULL != *argp) {          /* 05/01/11 */
                    ewprintf("Symbol reference expected");
                }
                return EERROR;                  /* symbol name expected */
            }

            if ((sp = sym_elookup(LGET_PTR2(const char, argp))) == NULL) {
                return EERROR;                  /* lookup error */
            }

            type = sp->s_type;
            lap->l_sym = sp;
            lap->l_flags = type;
            assert(type >= 0 && type < 11);
            return state2_tbl[arg & ARG_ANY][type];
        }

    case ARG_REST:
        /*
         *  ...
         */
        lap->l_list = argp;
        lap->l_flags = F_LIST;
        return EEXECUTE;

    case ARG_COND:
        /*
         *  conditional, return unprocessed
         */
        lap->l_list = argp;
        lap->l_flags = F_LIST;
        return F_HALT;
    }

    /**
     *  Note:   eval() returns either F_ERROR (-1) for a data-type, hence:
     *
     *              eval() - F_ERROR (ie eval() + 1).
     */
    return state_tbl[arg & ARG_ANY][eval(argp, lap) - F_ERROR];
}


void
set_hooked(void)
{
    static BUFFER_t *currentbp = 0;

    if (curwp && curwp->w_bufp == curbp) {      /* current window */
        cur_line = &curwp->w_line;
        cur_col  = &curwp->w_col;
        cur_attr = &curbp->b_attrcurrent;
        cur_cmap = curbp->b_cmap ? curbp->b_cmap :
                        (curwp->w_cmap ? curwp->w_cmap : x_default_cmap);

    } else if (curbp) {                         /* current buffer */
        cur_line = &curbp->b_line;
        cur_col  = &curbp->b_col;
        cur_attr = &curbp->b_attrcurrent;
        cur_cmap = x_default_cmap;

    } else {                                    /* unknown, setup defaults */
        static LINENO t_line = 1, t_col = 1;
        static LINEATTR t_attr = 0;             /* WHITE/BLACK */

        cur_line = &t_line;
        cur_col  = &t_col;
        cur_attr = &t_attr;
        cur_cmap = x_default_cmap;
        t_line   = 1;
        t_col    = 1;
        t_attr   = 0;
    }

    if (curbp != currentbp) {
        trace_ilog("set_hooked(line:%d,col:%d,num:%d,fname:\"%s\")\n", \
            *cur_line, *cur_col, (curbp ? curbp->b_bufnum : -1), (curbp ? curbp->b_fname : ""));
        currentbp = curbp;
    }

    assert(*cur_line >= 1);
    assert(*cur_col  >= 1);
    assert(cur_cmap  != NULL);
}
/*end*/