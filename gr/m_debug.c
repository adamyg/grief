#include <edidentifier.h>
__CIDENT_RCSID(gr_m_debug_c,"$Id: m_debug.c,v 1.42 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_debug.c,v 1.42 2025/02/07 03:03:21 cvsuser Exp $
 * Debug primitives.
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

#include <libstr.h>                             /* str_...()/sxprintf() */
#include "m_debug.h"
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

static void             dbg_nest_level(void);
static void             dbg_inq_vars(int type);
static void             dbg_inq_var_info(int type);
static void             dbg_stack_trace(const char *start_fn);
static void             dbg_inq_opcodes(void);

static SPTREE *         get_sym_level(int type);

static int              x_debug_timestamp = 0;
static int              x_debug_trap = TRUE;

static const struct dbgflg {
    const char *        f_name;
    size_t              f_length;
    int                 f_value;

} x_dbgflgnames[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
    { NFIELD("trace"),      DB_TRACE    },
    { NFIELD("regexp"),     DB_REGEXP   },
    { NFIELD("undo"),       DB_UNDO     },
    { NFIELD("flush"),      DB_FLUSH    },
    { NFIELD("time"),       DB_TIME     },
    { NFIELD("terminal"),   DB_TERMINAL },
    { NFIELD("term"),       DB_TERMINAL },
    { NFIELD("vfs"),        DB_VFS      },
    { NFIELD("notrap"),     DB_NOTRAP   },
    { NFIELD("memory"),     DB_MEMORY   },
    { NFIELD("mem"),        DB_MEMORY   },
    { NFIELD("refs"),       DB_REFS     },
    { NFIELD("prompt"),     DB_PROMPT   },
    { NFIELD("purify"),     DB_PURIFY   },
    { NFIELD("history"),    DB_HISTORY  },
#undef  NFIELD
    };

static int              flag_decode(int mode, const char *spec);
static const struct dbgflg *flag_lookup(const char *name, size_t length);


/*  Function:           do_debug
 *      debug primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: debug - Control runtime debug and tracing

        int 
        debug([int flags|string flags|NULL], [int echo = TRUE])

    Macro Description:
        The 'debug()' primitive enables and disables run-time tracing
        of macros and system acts to the diagnostics log. The new debug
        flags shall be set to 'flags'.

        If 'flags' is omitted, then the current trace mode is toggled
        from 'off' to 'DB_TRACE(1)' and from 'on' to 'off'.

        Otherwise by specifying 'flags' the associated events which
        shall be traced during subsequent macro operations. Flags can
        either be one or more *DB_XXX* constants OR'ed together or
        string containing a comma seperated list of flag names, see the
        table below. If the given flag value is either '0' or an empty
        string, all trace operations are disabled.

        The 'echo' flags controls whether the user is informed of the
        debug state change. When omitted or a non-zero any changes are
        echoed on the command prompt, informing the user of the new
        debug status, otherwise the macro is silent.

    Flags:

        The following debug flags are available, enabling tracing of
        both general and specific macro operations:

(start table,format=nd)
        [Constant       [Name       [Description                         ]

     !  DB_TRACE        trace       General trace.

     !  DB_REGEXP       regexp      Regular expression compilation
                                    and execution.

     !  DB_UNDO         undo        Undo/red trace.

     !  DB_FLUSH        flush       Flush output, shall cause the trace
                                    log to be flushed on each write;
                                    use should be avoided unless
                                    catching crash conditions due to
                                    associated high cost.

     !  DB_TIME         time        Time stamp which trace log.

     !  DB_TERMINAL     terminal    Terminal interface.

     !  DB_VFS          vfs         Virtual file-system.

     !  DB_NOTRAP       notrap      Disable SIGBUS/SIGSEGV trap handling.

     !  DB_MEMORY       memory      Target specific debug memory services: DEBUG only.

     !  DB_REFS         refs        Variable references.

     !  DB_PROMPT       prompt      Command prompting.
     
     !  DB_PURIFY       purify      Running under a memory analysis tool; DEBUG only.
     
     !  DB_HISTORY      history     Command history.
(end table)

    Command lines Options;

        The following command option also effect the generation of
        diagnostics trace.

        -d,--log        - Enable trace (DB_TRACE)

        -f,--flush      - Enable log flushing (DB_FLUSH).

        --nosig         - Disable signal traps (DB_NOSIG).

        -P,--dflags=x   - Debug/profiling flags 'x', being a comma
                          separated list of flag names, as defined
                          above.

        --rtc           - Enable real-time memory checks, if available.

        --native        - Enable native memory allocator, if suitable.

    Diagnostic Log:

        The default name of the diagnostics log is system dependent

        .grief.log      - Unix and Unix systems, including Linux.

        grief.log       - WIN32 and DOS based systems.

    Environment Variables:

        GRIEF_LOG       - If the GRIEF_LOG variable exists within the
                          environment then it overrides the default trace
                          log name.

    Macro Returns:

        The 'debug()' primitive returns the value of the debug flags
        prior to any changes, allowing the caller to restore later.

    Example:

        Enable debug, invoke our macro for testing and then restore the
        previous flags.

>       int odebug = debug(1, FALSE);
>       myfunction();
>       debug(odebug, FALSE):

    Macro Portability:
        The 'echo' option is a Grief extensions.

    Macro See Also:
        inq_debug, dprintf, pause_on_error, error, message
 */
void
do_debug(void)                  /* ([int flags|string flags], [int echo = TRUE]) */
{
    const int echo = get_xinteger(2, TRUE);
    int oflags = trace_flags(),
            nflags = oflags;

    if (! isa_undef(1)) {
        if (isa_string(1)) {                    /* extension, string specification */
            if ((nflags = flag_decode(0, get_str(1))) < 0) {
                acc_assign_int((accint_t)oflags);
                return;
            }
        } else {
            const int value1 = get_xinteger(1, -1);

            if (value1 < 0)  {                  /* -1, return/inq debug flags */
                acc_assign_int((accint_t)oflags);
                return;
            }
            nflags = (value1 & 0xffff);         /* flag set */
        }

        if (DB_NOTRAP & nflags) {
            nflags &= ~DB_NOTRAP;
            x_debug_trap = FALSE;               /* TODO */
        }

        if (DB_TIME & nflags) {
            nflags &= ~DB_TIME;
            x_debug_timestamp = TRUE;           /* TODO */
        }

    } else {
        nflags = (nflags ? 0 : DB_TRACE);       /* toggle between 0 and 1 (DB_TRACE) */
    }

    trace_flagsset(nflags);
 /* x_debug_timestamp = get_xinteger(3, x_debug_timestamp); */

    if (echo) {
        ewprintf("*** DEBUG %s (0x%04x) %s%s%s%s%s",
            (nflags ? "ON" : "OFF"), nflags,
                (nflags & DB_TERMINAL ? " TERMINAL" : ""),
                (nflags & DB_REGEXP   ? " REGEXP"   : ""),
                (nflags & DB_UNDO     ? " UNDO"     : ""),
                (nflags & DB_PROMPT   ? " PROMPT"   : ""),
                (nflags & DB_FLUSH    ? " FLUSH"    : ""));
    }

    trace_log("\n");
    if (! nflags) {
        trace_flush();
    }
    acc_assign_int((accint_t)oflags);
}


/*<<GRIEF>>
    Macro: watch - Watch a symbol.

        void
        watch(string symbol)

    Macro Description:
        The 'watch()' primitive is reserved for future use.

    Macro Returns:
        n/a

    Macro Portability:
        n/a

    Macro See Also:
        debug
*/

static int
flag_decode(int mode, const char *spec)
{
    static const char who[] = "debug";
    const char *comma, *cursor = spec;
    const struct dbgflg *flag;
    int nvalues = 0;

    trace_ilog("flag_decode(mode:%d, spec:%s)", mode, spec);
    while (NULL != (comma = strchr(cursor, ',')) || *cursor) {
        if (NULL != (flag = (NULL == comma ?    /* <value>[,<value>] */
                flag_lookup(cursor, strlen(cursor)) : flag_lookup(cursor, comma - cursor)))) {
            nvalues |= flag->f_value;
        } else {
            if (comma)  {
                errorf("%s: unknown flag '%*s'.", who, (int)(comma - spec), spec);
            } else {
                errorf("%s: unknown flag '%s'.", who, spec);
            }
            return -1;
        }
        if (NULL == comma) break;
        cursor = comma + 1;
    }
    return (1 == mode ? ~nvalues : nvalues);
}


static const struct dbgflg *
flag_lookup(const char *name, size_t length)
{
    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof(x_dbgflgnames)/sizeof(x_dbgflgnames[0])); ++i)
            if (length == x_dbgflgnames[i].f_length &&
                    0 == str_nicmp(x_dbgflgnames[i].f_name, name, length)) {
                return x_dbgflgnames + i;
            }
    }
    return NULL;
}


/*  Function:           inq_debug
 *      inq_debug primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_debug - Retrieve the current debug setting.

        int 
        inq_debug()

    Macro Description:
        The 'inq_debug()' primitive retrieves the current debug flags.

    Macro Returns:
        The 'inq_debug()' primitive returns an integer representing the
        debug flags which are currently in effect.

    Example:

        Enable debug of regular expressions, invoke our macro for
        testing and then restore the previous flags.

>       int odebug = inq_debug();
>       debug(odebug | DB_REGEXP, FALSE);
>       myfunction();
>       debug(odebug, FALSE):

    Macro See Also:
        debug, dprintf, pause_on_error, error, message
 */
void
inq_debug(void)                 /* () */
{
    int ret = trace_flags();

    if (ret) {
        if (x_debug_trap) {
            ret |= DB_NOTRAP;
        }
        if (x_debug_timestamp) {
            ret |= DB_TIME;
        }
    }
    acc_assign_int((accint_t)ret);
}


/*  Function:           do_debug_support
 *      debug_support() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: debug_support - Internal debugger functionality.

        void 
        debug_support(int what, declare object, declare arg)

    Macro Description:
        The 'debug_support()' primitive supports GRIEF debug functions.

        The primitive is a switcher to the actual sub-functions based
        upon 'what', the arguments 'object' and 'arg' are what
        specific.

    Macro Returns:
        nothing

    Macro See Also:
        debug, inq_debug
 */
void
do_debug_support(void)          /* void (int what, declare object, declare arg) */
{
    const int what = get_xinteger(1, 0);

    switch (what) {
    case DBG_STACK_TRACE:
        dbg_stack_trace(get_xstr(3));
        break;

    case DBG_NEST_LEVEL:
        dbg_nest_level();
        break;

    case DBG_INQ_VARS:
        dbg_inq_vars(DBG_INQ_VARS);
        break;

    case DBG_INQ_VAR_INFO:
        dbg_inq_var_info(DBG_INQ_VARS);
        break;

    case DBG_INQ_BVARS:
        dbg_inq_vars(DBG_INQ_BVARS);
        break;

    case DBG_INQ_BVAR_INFO:
        dbg_inq_var_info(DBG_INQ_BVARS);
        break;

    case DBG_INQ_MVARS:
        dbg_inq_vars(DBG_INQ_MVARS);
        break;

    case DBG_INQ_MVAR_INFO:
        dbg_inq_var_info(DBG_INQ_MVARS);
        break;

    case DBG_INQ_OPCODES:
        dbg_inq_opcodes();
        break;

    default:
        acc_assign_int(-1);
        break;
    }
}


/*
 *  dbg_stack_trace ---
 *      Function to return a list containing a stack trace
 */
static void
dbg_stack_trace(const char *start_fn)
{
    LIST *newlp, *lp;
    int llen, i;

    llen = (mac_sd * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
    if (mac_sd <= 0 || NULL == (newlp = lst_alloc(llen, mac_sd))) {
        acc_assign_null();
        return;
    }

    if (NULL == start_fn || *start_fn == '\0') {
        start_fn = NULL;
    }

    for (lp = newlp, i = (mac_sd - 1); i >= 0; --i) {
        const char *macname = mac_stack[i].name;

        if (start_fn) {
            if (strcmp(start_fn, macname) != 0) {
                continue;
            }
            start_fn = NULL;
            continue;
        }
        lp = atom_push_str(lp, macname);
    }
    atom_push_halt(lp);
    acc_donate_list(newlp, llen);
}


/*
 *  dbg_nest_level ---
 *      Function is used to return the current execution nesting level.
 */
static void
dbg_nest_level(void)
{
    acc_assign_int((accint_t) mac_sd);
}


/*
 *  dbg_inq_vars ---
 *      Function returns a list of variables at the specified execution level.
 */
static void
dbg_inq_vars(int vartype)
{
    SPTREE *treep;
    LIST *newlp, *lp;
    SPBLK **array;
    int atoms, llen;
    int i;

    if (NULL == (treep = get_sym_level(vartype))) {
        acc_assign_null();
        return;
    }

    /* determine storage */
    array = spflatten(treep);
    atoms = spsize(treep);
    llen  = (atoms * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
    if (0 == atoms || NULL == (newlp = lst_alloc(llen, atoms))) {
        acc_assign_null();
        return;
    }

    for (lp = newlp, i = 0; array[i]; ++i) {
        const SYMBOL *sp = array[i]->data;

        lp = atom_push_str(lp, sp->s_name);
    }
    atom_push_halt(lp);

    acc_donate_list(newlp, llen);
    chk_free(array);
}


/*
 *  dbg_inq_var_info ---
 *      Function to return type and value of a variable at a
 *      specified nesting level.
 */
static void
dbg_inq_var_info(int vartype)
{
    const char *name = get_str(3);
    SPTREE *treep;
    LIST *newlp, *lp;
    SPBLK *spb;
    SYMBOL *sp;
    unsigned char type;
    int llen;

    if (NULL == name || NULL == (treep = get_sym_level(vartype)) ||
            NULL == (spb = splookup(name, treep))) {
        acc_assign_null();
        return;
    }
    sp = sym_access((SYMBOL *) spb->data);

    assert(sp->s_type < F_MAX);
    switch (sp->s_type) {                       /* type conversion */
    case F_STR:
        type = F_RSTR;
        break;
    case F_LIST:
        type = F_RLIST;
        break;
    default:
        type = sp->s_type;
        break;
    }

    llen = sizeof_atoms[F_INT] + sizeof_atoms[type] + sizeof_atoms[F_HALT];
    if (NULL == (lp = newlp = lst_alloc(llen, 2))) {
        acc_assign_null();
        return;
    }
                                                /* type and value */
    lp = atom_push_int(lp, (accint_t)sp->s_type);
    switch (sp->s_type) {
    case F_INT:
        lp = atom_push_int(lp, sp->s_int);
        break;
    case F_FLOAT:
        lp = atom_push_float(lp, sp->s_float);
        break;
    case F_STR:
    case F_LIST:
        lp = atom_push_ref(lp, sp->s_obj);
        break;
    default:
        panic("sym_export: unexpected type ? (0x%x/%d)", sp->s_type, sp->s_type);
        break;
    }
    atom_push_halt(lp);                         /* terminator */

    acc_donate_list(newlp, llen);
}


/*
 *  dbg_inq_opcodes ---
 *      Retrieve the current OPCODE description.
 */
static void
dbg_inq_opcodes(void)
{
    const char *desc = "";
    LIST *newlp, *lp;
    int opcode, llen;

    llen = (F_MAX * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];
    if (NULL == (lp = newlp = lst_alloc(llen, F_MAX))) {
        acc_assign_null();
        return;
    }

    assert(0 == F_HALT && 14 == F_MAX);         /* confirm namespace below */
    for (opcode = F_HALT; opcode < F_MAX; ++opcode) {
        switch (opcode) {
        case F_HALT:   desc = "?0?     "; break;
        case F_INT:    desc = "int     "; break;
        case F_FLOAT:  desc = "float   "; break;
        case F_STR:    desc = "string  "; break;
        case F_LIT:    desc = "lit     "; break;
        case F_LIST:   desc = "list    "; break;
        case F_ARRAY:  desc = "array   "; break;
        case F_NULL:   desc = "null    "; break;
        case F_RSTR:   desc = "rstr    "; break;
        case F_RLIST:  desc = "rlist   "; break;
        case F_RARRAY: desc = "rarray  "; break;
        case F_ID:     desc = "id      "; break;
        case F_SYM:    desc = "symbol  "; break;
        case F_REG:    desc = "register"; break;
        default:
            panic("dbq_inq_opcodes: unexpected type ? (0x%x/%d)", opcode, opcode);
            break;
        }
        lp = atom_push_str(lp, desc);           /* const string */
    }
    atom_push_halt(lp);                         /* terminator */

    acc_donate_list(newlp, llen);
}


/*
 *  get_sym_level ---
 *      Function which returns pointer to a symbol table needed by
 *      debug support functions.
 */
static SPTREE *
get_sym_level(int vartype)
{
    /* Buffer variables */
    if (DBG_INQ_BVARS == vartype) {
        if (isa_integer(2)) {
            const int value2 = get_xinteger(2, -1);
            BUFFER_t *bp = (value2 > 0 ? buf_lookup(value2) : curbp);

            if (bp) {
                return bp->b_syms;
            }
        }

    /* Module */
    } else if (DBG_INQ_MVARS == vartype) {
        const char *module = get_xstr(2);

        if (module) {
            SPTREE *sym_module;

            if ((sym_module = module_symbols(module)) != NULL) {
                return sym_module;
            }
        }

    /* Global or specified scope */
    } else if (DBG_INQ_VARS == vartype) {
        if (isa_integer(2)) {
            const int value2 = get_xinteger(2, -1);

            if (value2 < 0) {
                return x_gsym_tbl;

//???       } else if (value2 <= ms_cnt) {
//              return x_lsym_tbl[ mac_stack[ value2 ].level ];

            } else if (value2 <= x_nest_level) {
                return x_lsym_tbl[ value2 ];
            }
        }
    }

    acc_assign_null();
    return NULL;
}


/*  Function:           do_profile
 *      profile primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: profile - Profiling support.

        void 
        profile([int flags])

    Macro Description:
        The 'profile()' primitive controls the profiler flags.

        If 'flags' is omitted, then the current profiler state is
        toggled, otherwise sets the profiler state to the specified
        'flags'.

    Macro Returns:
        nothing

    Macro See Also:
        debug
 */
void
do_profile(void)
{
    if (!isa_undef(1)) {
        xf_profile = (get_xinteger(1, 0) & 0xffff);
    } else {
        xf_profile = !xf_profile;
    }
    ewprintf("[Profiling %s]", (xf_profile ? "ON" : "OFF"));
}

/*end*/
