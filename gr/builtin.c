#include <edidentifier.h>
__CIDENT_RCSID(gr_builtin_c,"$Id: builtin.c,v 1.73 2025/01/17 12:38:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: builtin.c,v 1.73 2025/01/17 12:38:29 cvsuser Exp $
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
#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#endif

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

#define  WORD_INLINE    /* inline LIST interface */
#include "word.h"                               /* PUT/GET */

#define EERROR          -1
#define EEXECUTE        -2

enum ARGERRORS {
    ERR_NONE = 0,
    ERR_MISSING,
    ERR_INVALID,
    ERR_TOOMANY
};

struct SAVED {                                  /* Save argument references */
    OPCODE      save_type;
    void       *save_ptr;
};

static void             execute_event(int event);
static void             execute_builtin(const BUILTIN *bp, const LIST *lp);

static __CINLINE void   arg_error(const BUILTIN *bp, enum ARGERRORS msg, struct SAVED *saved_str, struct SAVED *ssp, int arg);
static __CINLINE void   arg_free(struct SAVED *saved_str, struct SAVED *ssp);

static int              arg_expand(const BUILTIN *bp, int varargs, int largc,
                            LISTV **largv, LISTV **lap, struct SAVED **lsaved, struct SAVED **ssp);

static int              execute_expr2(const argtype_t arg, const LIST *argp, LISTV *lap);
#if !defined(NDEBUG)
static void             check_hooked(void);
#endif

#define REGEVTNUM       32                      /* Event queue size */

static unsigned         x_evtno = 0;
static unsigned         x_evttail = 0;
static unsigned         x_evthead = 0;
static unsigned         x_evtactive = FALSE;
static unsigned         x_evtqueue[REGEVTNUM];  /* Event queue */

static LISTV            x_margv[MAX_ARGC];      /* Initial stack frame */

unsigned __CCACHEALIGN  mexecflags = 0;         /* Execution flags, 'break', 'return' etc. */
const LISTV *           margv = x_margv;        /* Argument vector */
int                     margc = 0;              /* Argument count */
    //const char *      mname = NULL;           /* Name of current macro primitive */

LINENO *                cur_line = NULL;        /* Current line reference */
LINENO *                cur_col  = NULL;        /* Current column reference */
LINEATTR *              cur_attr = NULL;        /* Attribute reference */

unsigned                mac_sd = 0;             /* Macro stack depth */
struct mac_stack *      mac_sp = NULL;          /* Stack point (== mac_stack[ms_cnt-1]) */
struct mac_stack        mac_stack[MAX_MACSTACK+1] = {0}; /* Macro name stack */

void *                  x_returns = NULL;       /* Assigned to the returns() value */
void *                  x_exception = NULL;     /* Try/catch() exception */


#if !defined(HAVE_ISCSYM) && \
        !defined(_MSC_VER) && !defined(__WATCOMC__) && !defined(__MINGW32__)
static int
iscsym(int c) /*TODO: compat_iscsym()*/
{
    return ('_' == c || isalnum(c));
}
#endif


/*
 *  execute_str ---
 *      Take a string, possibly entered via the command prompt, taking the
 *      form <macro [arguments ... ]>, parse and then execute the specified macro.
 *
 *      Arguments can be either int, float otherwise treated as a string.
 */
int
execute_str(const char *str)
{
    register const unsigned char *cp = (unsigned char *)str;

    unsigned char cmd[MAX_CMDLINE * 2],         /* MAGIC */
        *dp = cmd, *dpend = dp + (sizeof(cmd) - 2 /*nul*/);

    LIST list[LIST_SIZEOF(128)],                /* 128 atoms -- ~1k */
        *lp = list, *lpend = lp + (sizeof(list) - 1 /*HALT*/);

    while (*cp && isspace(*cp)) {
        ++cp;
    }
    if (! *cp) {
        return 0;           /*success, no macro*/
    }

    if ('$' == *cp) {                           /* leading module identifier "$xxxx::" */
        *dp++ = '$';
        while (*++cp && isxdigit(*cp)) {
            *dp++ = *cp;
        }

        if (':' == *cp)  {
            *dp++ = *cp++;
            if (':' == *cp) {
                *dp++ = *cp++;
            }
        }
    }

    while (*cp && iscsym(*cp)) {                /* letter, underscore or digit */
        if (dp >= dpend) {
            ewprintf("Out of space in execute_str(1)");
            return -1;      /*error, no-memory*/
        }
        *dp++ = *cp++;
    }

    if (*cp && !isspace(*cp)) {
        while (*cp && !isspace(*cp)) {          /* add new additional non-space characters */
            if (dp < dpend) {
                *dp++ = *cp;
            }
            ++cp;
        }
        *dp++ = '\0';

        ewprintf("Invalid macro name <%s>", cmd);
        return -1;          /*error, function name*/
    }
    *dp++ = '\0';

    lp = atom_push_sym(lp, (const char *)cmd);  /* macro name */

    while (*cp) {
        while (isspace(*cp)) {
            ++cp;           /*leading whitespace*/
        }
        if (! *cp)  {
            break;          /*end-of-arguments*/
        }

        if (lp >= lpend) {
            ewprintf("Out of space in execute_str(2)");
            return -1;
        }

        if ((*cp >= '0' && *cp <= '9') ||       /* possible numeric */
                (*cp == '-' && (cp[1] >= '0' && cp[1] <= '9')) ||
                (*cp == '+' && (cp[1] >= '0' && cp[1] <= '9')) ||
                (*cp == '.' && (cp[1] >= '0' && cp[1] <= '9')))
        {
            accfloat_t fval;
            accint_t ival;
            int ret, len = 0;

#if (CM_ATOMSIZE == SIZEOF_LONG_LONG && CM_ATOMSIZE != SIZEOF_LONG)
            ret = str_numparsel((const char *)cp, &fval, &ival, &len);
#else
            ret = str_numparse((const char *)cp, &fval, &ival, &len);
#endif
            switch (ret) {
            case NUMPARSE_INTEGER:              /* integer-constant */
            case NUMPARSE_FLOAT:                /* float-constant */
                cp += len;
                if (!*cp || isspace(*cp)) {
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
            register const unsigned char *cp1;
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

            lp = atom_push_const(lp, (const char *)dp); /* copy sub-string */
            for (cp = cp1; *cp && *cp != term;) {
                if (dp >= dpend) {
                    ewprintf("Out of space in execute_str(3)");
                    return -1;
                }
                //XXX: remove quotes?
                *dp++ = *cp++;
            }
            *dp++ = '\0';

            if (*cp) {                          /* consume terminator */
                ++cp;
            }
        }
    }

    atom_push_halt(lp);
    execute_nmacro(list);
    return 0;
}



int
execute_function(const char *function, const char *arg)
{
    LIST list[LIST_SIZEOF(4)],                  /* 4 atoms */
        *lp = list, *lpend = lp + (sizeof(list) - 1 /*HALT*/);

    acc_assign_int(-1);
    lp = atom_push_sym(lp, function);
    if (arg) lp = atom_push_str(lp, arg);
    assert(lp < lpend);
    atom_push_halt(lp);
    execute_nmacro(list);
    return (int)acc_get_ival();
}



void
execute_unassigned(const char *spec, int key, const char *seq)
{
    //
    //  TODO: general execute_key(), pre-compile during assign_to_key()
    //
#define UNASSIGNED_ARGV 16

    LIST list[LIST_SIZEOF(UNASSIGNED_ARGV)],    // 16 atoms
        *lp = list, *lpend = lp + (sizeof(list) - 1 /*HALT*/);

    const int speclen = strlen(spec) + 1 /*nul*/;
    char *cp = memcpy(alloca(speclen), spec, speclen);

    if (NULL == cp)
        return;

    lp = atom_push_sym(lp, cp);                 // macro

    if ((cp = strchr(cp, ' ')) != NULL) {
        //
        //  trailing arguments
        //
        int argv = 0;

        *cp++ = 0;                              // terminate macro

        for (argv = 1; argv < (UNASSIGNED_ARGV - 2); ++argv) {

            // leading white-space
            while (*cp && isspace(*cp))
                ++cp;
            if (! *cp)
                break;                          // EOS

            // sequence placeholder
            if (cp[0] == '{' && cp[1] == '}') {
                // TODO: {k} == key, {s} = seq, {d} = description
                lp = atom_push_const(lp, seq ? seq : "");
                cp += 2;
                continue;
            }

            // numeric value (possible)
            if ((*cp >= '0' && *cp <= '9') ||
                    (*cp == '-' && (cp[1] >= '0' && cp[1] <= '9')) ||
                    (*cp == '+' && (cp[1] >= '0' && cp[1] <= '9')) ||
                    (*cp == '.' && (cp[1] >= '0' && cp[1] <= '9')))
            {
                accfloat_t fval;
                accint_t ival;
                int ret, len = 0;

#if (CM_ATOMSIZE == SIZEOF_LONG_LONG && CM_ATOMSIZE != SIZEOF_LONG)
                ret = str_numparsel((const char *)cp, &fval, &ival, &len);
#else
                ret = str_numparse((const char *)cp, &fval, &ival, &len);
#endif
                switch (ret) {
                case NUMPARSE_INTEGER:          // integer-constant
                case NUMPARSE_FLOAT:            // float-constant
                    cp += len;
                    if (!*cp || isspace(*cp)) {
                        if (ret == NUMPARSE_INTEGER) {
                            lp = atom_push_int(lp, ival);
                        } else {
                            lp = atom_push_float(lp, fval);
                        }
                        continue;
                    }
                    cp -= len;
                    /*FALLTHRU*/
                default:                        // bad number
                    break;
                }
            }

            // string constants
            {
                char term = ' ', *out = cp;

                lp = atom_push_const(lp, (const char*)out);
                if (*cp == '"') {               // quoted string
                    term = '"', ++cp;
                }
                while (*cp) {
                    if (*cp == '\\') {          // escaped, consume; if suitable
                        if (cp[1])
                            ++cp;
                    } else if (*cp == term) {   // terminator
                        break;
                    }
                    *out++ = *cp++;
                }
                *out = 0;                       // terminate string             
            }
        }
    }

    lp = atom_push_int(lp, key);
    assert(lp < lpend);
    atom_push_halt(lp);

    execute_nmacro(list);
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
        break;
    }
}


void
execute_nmacro(const LIST *lp)
{
    sym_local_build();
    execute_macro(lp);
    sym_local_delete(TRUE);
}


void
execute_event_ctrlc(void)
{
    execute_event(REG_CTRLC);
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
        set_signal();
        return;
    }
    --x_evtno;
}


void
execute_macro(const LIST *lp)
{
    for (;;) {
        switch (*lp) {
        case F_LIST:
            execute_macro(lp + CM_ATOM_LIST_SZ);
            if (is_interrupt()) {
                /*
                 *  break, return or signal events
                 */
                if (x_evtno) {                  // pending signals
                    if (0 == x_evtactive++) {   // and not inside signal handler
                        while (x_evtno) {
                            /*
                             *  REG_SIGUSR1, REG_SIGUSR2, REG_SIGCTRLC ...
                             */
                            const unsigned head = (x_evthead++ % REGEVTNUM),
                                evtno = x_evtqueue[head];
                            if (0 == --x_evtno) clear_signal();
                            trigger(evtno);
                        }
                        --x_evtactive;
                    }
                }
                if (is_breakreturn()) {
                    return;
                }
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
    const char *macro;
    register BUILTIN *bp;
    register MACRO *mptr;
    int ret, omsglevel;

    /*
     *  Locate
     */
    switch (*lp) {
    case F_INT:             /* integer-constant */
        acc_assign_int(LGET_INT(lp));
        return;

    case F_FLOAT:           /* float-constant */
        acc_assign_float(LGET_FLOAT(lp));
        return;

    case F_LIT:             /* string literal */
        acc_assign_str(LGET_PTR2(const char, lp), -1);
        return;

    case F_ID:              /* builtin */
#if defined(NDEBUG)
        bp = builtin + LGET_ID(lp);
#else
        {   const int id = LGET_ID(lp);
            assert(id >= 0 || (unsigned)id < builtin_count);
            bp = builtin + id;
        }
#endif
        macro = bp->b_name;
        break;

    case F_SYM:             /* symbol; possible builtin (see: execute_str) */
        macro = LGET_PTR2(const char, lp);
        bp = builtin_lookup(macro);
        break;

    default:                /* unsupported/unexpected */
        panic("execute_xmacro: Unexpected type (0x%x/%u)", *lp, *lp);
        macro = NULL;                           /* quiet uninit warnings */
        bp = NULL;
        break;
    }

    /*
     *  Builtin?
     */
    if (bp) {
        /*
         *  Non-replacement execution
         */
        assert(BUILTIN_MAGIC == bp->b_magic);
        if (0 == (B_REDEFINE & bp->b_flags)) {
            assert(NULL == bp->b_macro && NULL == bp->b_ovargv);
            __CIFDEBUG(++bp->b_reference;)
            execute_builtin(bp, lp_argv);
            return;
        }

        /*
         *  Replacement execution
         */
        if (NULL != (mptr = bp->b_macro)) {     /* replace chain */
            if (bp->b_macro == bp->b_first_macro) {
                bp->b_ovargv = lp_argv;         /* first call, save argument reference */
            }
            __CIFDEBUG(++bp->b_replacement;)
            bp->b_macro = bp->b_macro->m_next;
            macro = bp->b_name;
            goto exec_macro;
        }

exec_replacement:
        if (F_HALT == *lp_argv && bp->b_ovargv) {
            lp_argv = bp->b_ovargv;             /* apply original argument list */
            bp->b_ovargv = NULL;
        }
        __CIFDEBUG(++bp->b_reference;)
        execute_builtin(bp, lp_argv);
        bp->b_macro  = bp->b_first_macro;       /* reset for next call */
        bp->b_ovargv = NULL;
        return;
    }

    /*
     *  Lookup-defined macros
     */
    if (NULL == (mptr = macro_lookup(macro))) {
        if (0 == macro_load(macro)) {
            mptr = macro_lookup(macro);
        }
        if (NULL == mptr) {
undefined_macro:
            triggerx(REG_UNDEFINED_MACRO, "\"%s\"", macro);
            if (FALSE == x_mflag) {
                errorf("%s undefined.", macro);
            }
            return;
        }
    }

    /*
     *  Execute macro
     */
exec_macro:
    if ((ret = macro_autoload(mptr, TRUE)) < 0) {
        if (bp) {                               /* autoload failed */
            goto exec_replacement;
        }
        goto undefined_macro;

    } else if (0 == ret) {
        assert(mptr == macro_lookup(macro));
        mptr = macro_lookup(macro);
        assert(mptr);
        assert(0 == (mptr->m_flags & M_AUTOLOAD));

    } else {
        assert(1 == ret);
    }

    lp = mptr->m_list;
    if (F_HALT == *lp) {
        assert(! bp);                           /* XXX: restore bp->b_macro ? */
        return;                                 /* empty/null macro */
    }

    if (mac_sd >= MAX_MACSTACK) {
        panic("Macro stack overflow (%d).", mac_sd);
    } else {                                    /* -- push */
        register struct mac_stack *stack = mac_stack + mac_sd++;
        mac_registers_t *regs;

        ++mac_sp; assert(stack == mac_sp);

        stack->module = mptr->m_module;
        stack->name   = mptr->m_name;
        stack->caller = NULL;
        stack->argv   = lp_argv;
        stack->argc   = -1;                     /* future use */
        stack->level  = x_nest_level;
        if (NULL != (regs = stack->registers)) {
            assert(REGISTERS_MAGIC == regs->magic);
            memset(&regs->symbols, 0, sizeof(SYMBOL *) * (regs->slots + 1));
        }
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

    if (bp) bp->b_macro = mptr;                 /* pop chain */

    mptr->m_ftime = FALSE;                      /* first time */
    x_msglevel = omsglevel;                     /* restore message level */

    assert(mac_sp == (mac_stack + (mac_sd - 1)));
    --mac_sd; --mac_sp;                         /* --- pop */
}


static void
execute_builtin(const BUILTIN *bp, const LIST *lp)
{
    int varargs = (B_VARARGS & bp->b_flags) ? -1 : 0;
        /* dynamic arguments, -1=enabled otherwise 0=disabled */

    const argtype_t *argtypes = bp->b_arg_types;
    argtype_t argtype = *argtypes;

    const LIST *olp, *ovargv = bp->b_ovargv;    /* original argument list, if replacement */

    struct SAVED localsaved[MAX_ARGC];
    struct SAVED *lsaved = localsaved;
    struct SAVED *ssp = localsaved;

    LISTV localargv[MAX_ARGC];                  /* argument stack */
    LISTV *largv = localargv;
    register LISTV *lap = localargv;
    int largc;

    int exectype, op;

    lap->l_str = bp->b_name;                    /* arg[0] */
    __CIFDEBUG(lap->l_flags = F_LIT;)

    ++lap;                                      /* arg[1] ... */
    largc = 1;

    /*
     *  Pass arguments
     */
    if (F_HALT != (op = *lp) && argtype) {
        do {
            /*
             *  Manage local storage.
             */
            if (++largc > MAX_ARGC) {           /* guard system limits */
                /*
                 *  vararg, syntax is similar to '...', allowing the last argument type to repeat indefinitely.
                 */
                if (largc > varargs) {
                    LISTV *t_lap = lap;
                    if (-1 == (varargs =        /* expand */
                            arg_expand(bp, varargs, largc, &largv, &t_lap, &lsaved, &ssp))) {
                        arg_error(bp, ERR_NONE, lsaved, ssp, 0);
                        return;
                    }
                    lap = t_lap;
                }
            }

            /*
             *  Encode and validate argument against parameter list.
             */
            if (F_NULL == op && 0 == (argtype & ARG_REST)) {
                if (ovargv && NULL != (olp = atom_nth(ovargv, largc - 2))) {
                        //Note: Replacement overhead seeking alt argument,
                        //  could optimise yet at the expense of general non-replacement use case.
                    exectype = execute_expr2(argtype, olp, lap);
                } else {
                    if (0 == (argtype & ARG_OPT)) {
                        arg_error(bp, ERR_MISSING, lsaved, ssp, lap - largv);
                        return;
                    }
                    lap->l_flags = F_NULL;
                    lap->l_int = 0;
                    exectype = F_HALT;
                }
            } else {
                exectype = execute_expr2(argtype, lp, lap);
            }

            switch (exectype) {
            case F_HALT:
            case F_INT:
            case F_FLOAT:
            case F_LIT:
                break;
            case F_STR:
                assert(lap->l_flags == F_STR);
                lap->l_str = ssp->save_ptr = chk_salloc(lap->l_str);
                ssp->save_type = F_STR;
                ++ssp;
                break;
            case F_RLIST:
            case F_RSTR:
#if defined(DO_ARRAY)
            case F_RARRAY:
#endif
                assert(lap->l_flags == exectype);
                ssp->save_ptr = r_inc(lap->l_ref);
                ssp->save_type = exectype;
                ++ssp;
                break;
            case F_LIST:
                assert(lap->l_flags == F_LIST);
                ssp->save_ptr = lst_clone(lap->l_list, NULL);
                ssp->save_type = F_LIST;
                ++ssp;
                break;
#if defined(DO_ARRAY)
            case F_ARRAY:
                assert(lap->l_flags == F_ARRAY);
                ssp->save_ptr = array_clone(lap->l_array, NULL);
                ssp->save_type = F_ARRAY;
                ++ssp;
                break;
#endif
            case F_NULL:
                break;
            case EEXECUTE:                          /* ... */
                goto execute;
            case EERROR:
                /*
                 *  One last chance --
                 *      if we wanted an int-value but we have a float then do a cast for the user.
                 */
                if ((argtype & (ARG_LVAL | ARG_NUM)) == ARG_INT && lap->l_flags == F_FLOAT) {
                    lap->l_int = (accint_t) lap->l_float;
                    lap->l_flags = F_INT;
                    break;
                }
                arg_error(bp, ERR_INVALID, lsaved, ssp, lap - largv);
                return;
            default:
                panic("%s: Unexpected exectype (0x%x/%d)", bp->b_name, exectype, exectype);
                return;
            }
            ++lap;

            /*
             *  Terminate, for example an undefined symbol.
             */
            if (is_return()) {
                arg_free(lsaved, ssp);
                return;
            }

            /*
             *  Move onto the next argument descriptor.
             *  Note: Don't move if an indefinite list and last descriptor; as it repeats.
             */
            if (! varargs || argtypes[1] /*not-last*/) {
                argtype = *++argtypes;          /* next type */
            }

            /*
             *  Move cursor 'lp', defaulting to 'ovargv' if end-of-list.
             *  Note: Unsure whether Brief compatible.
             */
            lp = atom_next_nonnull(lp);
            if (F_HALT == (op = *lp)) {
                /*
                 *  End-of-arguments, utilise replace if available.
                 */
                if (ovargv) {
                    if (NULL != (lp = atom_nth(ovargv, largc - 1))) {
                        ovargv = NULL;
                        op = *lp;
                        continue;               /* additional arguments */
                    }
                }
                break;
            }
        } while (argtype);                      /* terminator? */
    }

    if (argtype) {
        assert(F_HALT == op && op == *lp);

        do {                                    /* check for missing mandatory arguments. */
            if (0 == (argtype & ARG_OPT)) {
                arg_error(bp, ERR_MISSING, lsaved, ssp, 0);
                return;
            }

            /*
             *  NULL pad trailing optional arguments.
             *  Note: No macros currently requires, could reenable via flags (B_NULLPAD).
             */
#if defined(B_NULLPAD)
              if (B_NULLFILL & bp->b_flags) {
                  if (++largc > MAX_ARGC && largc > varargs) {
                      LISTV *t_lap = lap;
                      if (-1 == (varargs =      /* expand */
                              arg_expand(bp, varargs, largc, &largv, &t_lap, &lsaved, &ssp))) {
                          arg_error(bp, ERR_NONE, lsaved, ssp, 0);
                          return;
                      }
                      lap = t_lap;
                  }
              }
              lap->l_flags = F_NULL;
              lap->l_int = 0;
              ++lap;
#endif  //B_NULLPAD

        } while (0 != (argtype = *++argtypes)); /* next argument */

    } else {
        assert(0 == argtype && 0 == *argtypes);
        assert((NULL == lp && op == F_HALT) || op == *lp);

        if (F_HALT != op) {                     /* unexpected argments */
            arg_error(bp, ERR_TOOMANY, lsaved, ssp, 0);
            return;
        }
    }

execute: {
        const LISTV *saved_argv = margv;        /* push stack frame */
        int saved_argc = margc;

        margv = largv;
        margc = largc;

#if defined(DO_PROFILE)
        if (x_profile) {
            timer_start(&timer);
        }
#endif
        (*bp->b_func)(bp->b_parameter);
#if defined(DO_PROFILE)
        if (x_profile) {
            bp->b_profile += timer_end(&timer);
        }
#endif
#if !defined(NDEBUG)
        check_hooked();                         /* trap incorrect curbp/curwp/set_hooked() usage */
#endif

        if (x_dflags) {
            if (0 == (bp->b_flags & B_NOVALUE)) {
                acc_trace();                    /* TODO: replace B_NOVALUE with ARG_VOID */
            }
        }

        arg_free(lsaved, ssp);

        assert((varargs <= 0 && largv == localargv) || (varargs > 0 || largv != localargv));
        if (varargs > 0) {                      /* release dynamic storage */
            assert(lsaved != localsaved);
            chk_free(largv);
            chk_free(lsaved);
        }

        margv = saved_argv;                     /* pop stack frame */
        margc = saved_argc;
    }
}


// Retrieve the name of the current builtin macro.
const char *
execute_name(void)
{
    assert(margv);
    if (margv) {
        assert(F_LIT == margv->l_flags);
        return margv->l_str;                    /* command name, source bp->name */
    }
    return "command";
}


static __CINLINE void
arg_error(const BUILTIN *bp, enum ARGERRORS msg, struct SAVED *saved_str, struct SAVED *ssp, int arg)
{
    /*
     *  Exception and/or errno ...
     */
    switch (msg) {
    case ERR_MISSING:
        if (msg > 1) {
            errorf("%s: parameter %d missing", bp->b_name, arg);
        } else {
            errorf("%s: parameter(s) missing", bp->b_name);
        }
        break;
    case ERR_INVALID:
        errorf("%s: parameter %d invalid", bp->b_name, arg);
        break;
    case ERR_TOOMANY:
        errorf("%s: too many parameters", bp->b_name);
        break;
    case ERR_NONE:
        break;
    }
    arg_free(saved_str, ssp);
}


static __CINLINE void
arg_free(register struct SAVED *saved, register struct SAVED *ssp)
{
    while (ssp > saved) {
        switch ((--ssp)->save_type) {
        case F_STR:
            chk_free((char *) ssp->save_ptr);
            break;
        case F_LIST:
            lst_free((LIST *) ssp->save_ptr);
            break;
#if defined(DO_ARRAY)
        case F_ARRAY:
            array_free((HASH *) ssp->save_ptr);
            break;
#endif
        case F_RLIST:
        case F_RSTR:
#if defined(DO_ARRAY)
        case F_RARRAY:
#endif
            r_dec((ref_t *) ssp->save_ptr);
            break;
        default:
            panic("arg_free: unexpected type (0x%x/%d)", ssp->save_type, ssp->save_type);
            break;
        }
    }
}


static int
arg_expand(const BUILTIN *bp, int varargs, int largc, LISTV **largv, LISTV **lap, struct SAVED **lsaved, struct SAVED **ssp)
{
    const size_t lapi = (size_t)(*lap - *largv);
    const size_t sspi = (size_t)(*ssp - *lsaved);

    struct SAVED *nlsaved = NULL;
    LISTV *nlargv;
    int nvarargs;

    /*
     *  Varargs available ?
     */
    assert(varargs >= -1);
    if (! varargs) {
        ewprintf("%s: parameter limit exceeded", bp->b_name);
        return -1;
    }

    /*
     *  Expand argument vector and associcate save vector,
     *  Note: Save vector is general small, yet resize in tandem as the argument count represents it upper value.
     */
    if (-1 == varargs) {                        /* initial expansion */
        assert(largc == (MAX_ARGC + 1));
        assert(lapi == MAX_ARGC);
        assert(sspi <= lapi);

        nvarargs = MAX_ARGC * 2;
        if (NULL != (nlargv = chk_calloc(nvarargs, sizeof(LISTV)))) {
            if (NULL != (nlsaved = chk_calloc(nvarargs, sizeof(struct SAVED)))) {
                                                /* import current; fixed buffers */
                (void) memcpy(nlsaved, *lsaved, sspi * sizeof(struct SAVED));
                (void) memcpy(nlargv, *largv, MAX_ARGC * sizeof(LISTV));
            } else {
                ewprintf("%s: memory overflow", bp->b_name);
                chk_free(nlargv);
                nlargv = NULL;
                return -1;                      /* allocation error */
            }
        }

    } else {
        assert(largc >= (MAX_ARGC + 1));
        assert(sspi <= lapi);

        nvarargs = varargs * 2;                 /* expand */
        if (NULL != (nlargv = chk_recalloc(*largv, varargs * sizeof(LISTV), nvarargs * sizeof(LISTV)))) {
            nlsaved = chk_recalloc(*lsaved, varargs * sizeof(struct SAVED), nvarargs * sizeof(struct SAVED));
        }

        if (NULL == nlsaved) {
            ewprintf("%s: memory overflow", bp->b_name);
            chk_free(nlargv ? nlargv : *largv);
            arg_free(*lsaved, *ssp);
            chk_free(*lsaved);
            return -1;                          /* reallocation error */
        }
    }

    *largv  = nlargv;                           /* reassociate cursors */
    *lap    = nlargv + lapi;

    *lsaved = nlsaved;
    *ssp    = nlsaved + sspi;

    return nvarargs;
}


/*
 *  Represents possible type-conversions, where
 *
 *      l = ARG_INT     (0x0001)
 *      f = ARG_FLOAT   (0x0002)
 *      s = ARG_STRING  (0x0004)
 *      l = ARG_LIST    (0x0008)
 */
static const int state_tbl[][12] = {
    /*        ERROR,  HALT     F_INT,   F_FLOAT, F_STR,   F_LIT,   F_LIST,  F_ARRAY, F_NULL,  F_RSTR,  F_RLIST, F_FARRAY*/
    /*----*/ {-1,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*---i*/ {-1,     -1,      F_INT,   -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*--f-*/ {-1,     -1,      -1,      F_FLOAT, -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*--fi*/ {-1,     -1,      F_INT,   F_FLOAT, -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*-s--*/ {-1,     -1,      -1,      -1,      F_STR,   F_LIT,   -1,      -1,      -1,      F_RSTR,  -1,      -1      },
    /*-s-i*/ {-1,     -1,      F_INT,   -1,      F_STR,   F_LIT,   -1,      -1,      -1,      F_RSTR,  -1,      -1      },
    /*-sf-*/ {-1,     -1,      -1,      F_FLOAT, F_STR,   F_LIT,   -1,      -1,      -1,      F_RSTR,  -1,      -1      },
    /*-sfi*/ {-1,     -1,      F_INT,   F_FLOAT, F_STR,   F_LIT,   -1,      -1,      -1,      F_RSTR,  -1,      -1      },
    /*l---*/ {-1,     -1,      -1,      -1,      -1,      -1,      F_LIST,  -1,      -1,      -1,      F_RLIST, -1      },
    /*l--i*/ {-1,     -1,      F_INT,   -1,      -1,      -1,      F_LIST,  -1,      -1,      -1,      F_RLIST, -1      },
    /*l-f-*/ {-1,     -1,      -1,      F_FLOAT, -1,      -1,      F_LIST,  -1,      -1,      -1,      F_RLIST, -1      },
    /*l-fi*/ {-1,     -1,      F_INT,   F_FLOAT, -1,      -1,      F_LIST,  -1,      -1,      -1,      F_RLIST, -1      },
    /*ls--*/ {-1,     -1,      -1,      -1,      F_STR,   F_LIT,   F_LIST,  -1,      -1,      F_RSTR,  F_RLIST, -1      },
    /*ls-i*/ {-1,     -1,      F_INT,   -1,      F_STR,   F_LIT,   F_LIST,  -1,      -1,      F_RSTR,  F_RLIST, -1      },
    /*lsf-*/ {-1,     -1,      -1,      F_FLOAT, F_STR,   F_LIT,   F_LIST,  -1,      -1,      F_RSTR,  F_RLIST, -1      },
    /*lsfi*/ {-1,     -1,      F_INT,   F_FLOAT, F_STR,   F_LIT,   F_LIST,  -1,      F_NULL,  F_RSTR,  F_RLIST, -1      },
    };

static const int state2_tbl[][11] = {
    /*
     *  Symbol type conversions.
     *  Notes:
     *    o Symbols are limited to being F_INT, F_FLOAT, F_STR, F_ARRAY, F_LIST types.
     *    o F_HALT(0) conversion ok, otherwise -1 (EERROR).
     */
    /*        F_HALT, F_INT,   F_FLOAT, F_STR,   F_LIT,   F_LIST,  F_ARRAY, F_NULL,  F_RSTR,  F_RLIST, F_FARRAY*/
    /*----*/ {-1,     -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*---i*/ {-1,     F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*--f-*/ {-1,     -1,      F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*--fi*/ {-1,     F_HALT,  F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*-s--*/ {-1,     -1,      -1,      F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*-s-i*/ {-1,     F_HALT,  -1,      F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*-sf-*/ {-1,     -1,      F_HALT,  F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*-sfi*/ {-1,     F_HALT,  F_HALT,  F_HALT,  -1,      -1,      -1,      -1,      -1,      -1,      -1      },
    /*l---*/ {-1,     -1,      -1,      -1,      -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*l--i*/ {-1,     F_HALT,  -1,      -1,      -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*l-f-*/ {-1,     -1,      F_HALT,  -1,      -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*l-fi*/ {-1,     F_HALT,  F_HALT,  -1,      -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*ls--*/ {-1,     -1,      -1,      F_HALT,  -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*ls-i*/ {-1,     F_HALT,  -1,      F_HALT,  -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*lsf-*/ {-1,     -1,      F_HALT,  F_HALT,  -1,      F_HALT,  -1,      -1,      -1,      -1,      -1      },
    /*lsfi*/ {-1,     F_HALT,  F_HALT,  F_HALT,  -1,      F_HALT,  -1,      F_HALT,  -1,      -1,      -1      },
    };

static int
execute_expr2(const argtype_t arg, register const LIST *argp, LISTV *lap)
{
    SYMBOL *sp;
    int type;

    switch (arg & (ARG_REST | ARG_COND | ARG_LVAL)) {
    case ARG_LVAL: {
        /*
         *  pass by reference, lookup symbol
         */
            const OPCODE argtype = *argp;

            if (F_REG == argtype) {             /* REG <symbol> <idx/byte>, 01/04/20 */
                if (NULL == (sp = sym_rlookup(argp[SIZEOF_VOID_P + 1]))) {
                    if (NULL == (sp = sym_elookup(LGET_PTR2(const char, argp)))) {
                        return EERROR;          /* lookup error */
                    }
                }
#if !defined(NDEBUG) && defined(DO_REGISTER_CHECK)
                else {
                    SYMBOL *sp2 = sym_elookup(LGET_PTR2(const char, argp));
                    assert(sp == sp2);
                }
#endif  //_DEBUG
                type = sp->s_type;

            } else if (F_SYM == argtype) {      /* SYM <symbol> */
                if (NULL == (sp = sym_elookup(LGET_PTR2(const char, argp)))) {
                    return EERROR;              /* lookup error */
                }
                type = sp->s_type;

            } else {
                if (F_NULL != argtype) {        /* 05/01/11 */
                    if (ARG_OPT & arg) {
                        type = F_NULL;
                        sp = NULL;              /* optional LVAL, 07/24 */
                    } else {
                        ewprintf("Symbol reference expected");
                        return EERROR;          /* symbol/register expected */
                    }
#if !defined(NDEBUG)
                } else {
                    panic("execute_xmacro: F_WHAT? (0x%x/%u)", argtype, argtype);
                    return EERROR;              /* symbol/register expected */
#endif
                }
            }
            lap->l_sym = sp;
            lap->l_flags = F_SYM;
            assert(type >= 0 && type <= F_OPDATA);
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
         *      see do_if(), which shall execute indirectly when do required.
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
    type = eval(argp, lap) - F_ERROR;
    assert(type >= 0 && type <= (F_OPDATA + 1));
    return state_tbl[arg & ARG_ANY][type];
}


#if !defined(NDEBUG)
static void
check_hooked(void)
{
    LINENO *t_cur_line = cur_line;
    LINENO *t_cur_col  = cur_col;
    LINEATTR *t_cur_attr = cur_attr;
    const cmap_t *t_cur_cmap = cur_cmap;

    set_hooked();

    assert(t_cur_line == cur_line);
    assert(t_cur_col  == cur_col );
    assert(t_cur_attr == cur_attr);
    assert(t_cur_cmap == cur_cmap);
}
#endif  //!NDEBUG


void
set_curbp(BUFFER_t *bp)
{
    curbp = bp;
    set_hooked();
}


void
set_curwp(WINDOW_t *wp)
{
    curwp = wp;
    set_hooked();
}


void
set_curwpbp(WINDOW_t *wp, BUFFER_t *bp)
{
    curwp = wp;
    curbp = bp;
    set_hooked();
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
            *cur_line, *cur_col, (curbp ? curbp->b_bufnum : -1), (curbp ? c_string(curbp->b_fname) : ""));
        currentbp = curbp;
    }

    assert(*cur_line >= 1);
    assert(*cur_col  >= 1);
    assert(cur_cmap  != NULL);
}

/*end*/
