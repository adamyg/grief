#include <edidentifier.h>
__CIDENT_RCSID(gr_symbol_c,"$Id: symbol.c,v 1.44 2024/05/09 17:22:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: symbol.c,v 1.44 2024/05/09 17:22:07 cvsuser Exp $
 * Symbol management.
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
#include <errno.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "object.h"
#include "symbol.h"
#include "word.h"

int __CCACHEALIGN       x_nest_level = 0;       /* Stack dpeth */
ref_t *                 x_halt_list = NULL;     /* NULL lisp */
accint_t *              x_errno_ptr = NULL;     /* errno reference */

SPTREE *                x_gsym_tbl = NULL;      /* global symbol table */
SPTREE *                x_lsym_tbl[MAX_SYMSTACK+1] = {0}; /* local symbol tables */

static __CINLINE void   sym_clear(SYMBOL *sp);


/*  Function:           sym_init
 *      runtime initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>> [macro]
    Constant: errno - Last system errno number.

        extern int errno;

    Constant Description:
        When a system call detects an error, it returns an integer
        value indicating failure (usually -1) and sets the variable
        'errno' accordingly. This allows interpretation of the
        failure on receiving a -1 and to take action accordingly.

        Successful calls never set errno; once set, it remains until
        another error occurs. It should only be examined after an
        error. Note that a number of system calls overload the
        meanings of these error numbers, and that the meanings must
        be interpreted according to the type and circumstances of the
        call.

        See <Error Codes> for the list of manifest constants exported.

    Constant Portability:
        The values reported by errno are system and function dependent.

    Constant See Also:
        perror, strerror
 */
void
sym_init(void)
{
    const LIST halt[1] = {F_HALT};
    SYMBOL *sp;
    unsigned i;

    /*
     *  prime symbol tables.
     */
    x_gsym_tbl = spinit();                      /* global */
    for (i = 0; i < _countof(x_lsym_tbl); ++i) {
        x_lsym_tbl[i] = spinit();               /* scope symbol stack */
    }
    x_halt_list = rlst_build(halt, 1);          /* special halt list */

    /*
     *  errno is a global int, result of system call errors.
     */
    sp = sym_push(1, "errno", F_INT, 0);
    x_errno_ptr = &sp->s_int;
}


void
sym_shutdown(void)
{
    unsigned i;

    /* global */
    sym_table_delete(x_gsym_tbl);
    spfree(x_gsym_tbl);
    x_gsym_tbl = NULL;

    /* local */
    for (i = 0; i < _countof(x_lsym_tbl); ++i) {
        spfree(x_lsym_tbl[i]);
    }

    /* special */
    r_dec(x_halt_list);
    x_halt_list = NULL;
}


/*  Function:           sym_move
 *      Move symbols between scopes.
 *
 *  Parameters:
 *      sym_tbl - Destination scope.
 *      why - Caller of this interface (eg. "static").
 *
 *  Returns:
 *      nothing
 */
void
sym_move(SPTREE *sym_tbl, const char *why)
{
    const LIST *nextlp, *lp = get_list(1);
    SPBLK *spb, *spb2;
    SYMBOL *sp, *sp2;

    for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
        accint_t type;
        const char *sym;

        /* retrieve "[type] symbol-name" defines. */
        if (atom_xint(lp, &type)) {
            lp = nextlp;
            if ((nextlp = atom_next(lp)) == lp) {
                goto nosym;
            }
        } else {
            type = 0;
        }

        if (NULL == (sym = atom_xstr(lp))) {
nosym:;     ewprintf("%s: missing symbol.", why);
            continue;
        }

        /* locate the remove from source table. */
        if (NULL == (spb = splookup(sym, x_lsym_tbl[x_nest_level]))) {
            ewprintf("%s: '%s' not found at current scope.", why, sym);

            if (NULL == splookup(sym, sym_tbl)) {
                if (F_REFERENCE == type) {      /* create new image */
                    ewprintf("%s: cannot promote reference '%s'.", why, sym);
                    continue;
                }
                spb = sym_alloc(sym, (int)type);
                spenq(spb, sym_tbl);
            }
            continue;
        }

        sp = (SYMBOL *)spb->data;

        if (F_REFERENCE == sp->s_type) {
            ewprintf("%s: cannot promote reference '%s'.", why, sym);
            continue;
        }

        if (SF_SYSTEM & sp->s_flags) {
            ewprintf("%s: system variable '%s'", why, sym);
            continue;
        }

        /*
         *  Remove from local scope
         *  Free old version within destination symbol entry.
         *  Place within new scope
         */
        spdeq(spb, x_lsym_tbl[x_nest_level]);   /* remove */

        if ((spb2 = splookup(sym, sym_tbl)) != NULL) {
            sp2 = (SYMBOL *) spb2->data;
            spdeq(spb2, sym_tbl);
            spfreeblk(spb2);
            sym_free(sp2);
        }

        spenq(spb, sym_tbl);                    /* destination */
    }
}


/*  Function:           sym_create
 *      Create a new symbol, initialising it to the defaults.
 *
 *  Parameters:
 *      sp - Symbol.
 *      name - Name (optional).
 *      type - Type.
 *
 *  Returtsns:
 *      nothing
 */
void
sym_create(SYMBOL *sp, const char *name, int type)
{
    if (name) {                                 /* check system limits */
        if (strlen(name) >= sizeof(sp->s_name)) {
            ewprintf("variable name truncated '%.*s'", (int)sizeof(sp->s_name), name);
        }
        strxcpy(sp->s_name, name, sizeof(sp->s_name));
    }

    sp->s_type = type;
    sp->s_flags = 0;
    sp->s_dynamic = 0;

    switch (type) {
    case F_INT:
        sp->s_int = 0;
        break;
    case F_STR:
        sp->s_obj = r_string("");
        break;
    case F_LIST:
        sp->s_obj = r_inc(x_halt_list);
        break;
    case F_FLOAT:
        sp->s_float = 0.0;
        break;
    default:
        assert(type >= 0);
        assert(type < F_MAX);
        sp->s_type = F_INT;
        sp->s_int = 0;
        break;
    }
}


SYMBOL *
sym_push(int global, const char *name, int type, uint16_t flag)
{
    SPBLK *spb;
    SYMBOL *sp;

    assert(type >= 0);
    assert(type < F_MAX);
    spb = sym_alloc(name, type);
    if (global || x_nest_level == 0) {
        spenq(spb, x_gsym_tbl);
    } else {
        spenq(spb, x_lsym_tbl[x_nest_level]);
    }
    sp = (SYMBOL *) spb->data;
    sp->s_flags = (uint8_t)flag;
    return sp;
}


/*  Function:           sym_destroy
 *      Destroy an existing symbol, releasing any resources owned by the symbol.
 *
 *  Parameters:
 *      sp - Symbol.
 *
 *  Returns:
 *      nothing
 */
void
sym_destroy(SYMBOL *sp)
{
    sym_clear(sp);
    sp->s_type = F_NULL;
    sp->s_flags = 0;
    sp->s_dynamic = 0;
}


/*  Function:           sym_free
 *      Destroy an existing symbol and then free symbol, releasing any
 *      resources owned by the symbol.
 *
 *  Parameters:
 *      sp - Symbol.
 *
 *  Returns:
 *      nothing
 */
void
sym_free(SYMBOL *sp)
{
    if (sp) {
        sym_clear(sp);
        chk_free(sp);
    }
}


/*
 *  sym_alloc ---
 *      Function to allocate a new symbol. We return the splay tree block so
 *      the caller can figure out which symbol table to put it in
 */
SPBLK *
sym_alloc(const char *name, int type)
{
    SPBLK *spb;
    SYMBOL *sp;

    spb = (SPBLK *) spblk(sizeof(SYMBOL));
    sp = (SYMBOL *) spb->data;
    sym_create(sp, name, type);
    spb->key = sp->s_name;
    return spb;
}


void
sym_attach(BUFFER_t *bp)
{
    bp->b_syms = spinit();
}


void
sym_detach(BUFFER_t *bp)
{
    sym_table_delete(bp->b_syms);
    spfree(bp->b_syms);
    bp->b_syms = NULL;
}


SYMBOL *
sym_global_lookup(const char *name)
{
    SPBLK *spb;

    if (x_gsym_tbl)
        if ((spb = splookup(name, x_gsym_tbl)) != NULL) {
            return sym_access((SYMBOL *) spb->data);
        }
    return NULL;
}


void
sym_local_build(void)
{
    if (++x_nest_level >= MAX_SYMSTACK) {
        panic("Symbol nesting overflow.");
    }
}


void
sym_local_delete(int outer)
{
    sym_table_delete(x_lsym_tbl[x_nest_level]);
    --x_nest_level;

    if (outer) {
        if (x_returns) {                        /* last returns() value, if any */
            if (! is_return()) {
                acc_assign_object(x_returns);   /* assign unless explicit return */
                obj_trace(x_returns);
            }
            obj_free(x_returns);
            x_returns = NULL;
        }
        clear_return();
    }
}


SYMBOL *
sym_local_lookup(const char *name)
{
    SPBLK *spb;

    if ((spb = splookup(name, x_lsym_tbl[x_nest_level])) != NULL) {
        return sym_access((SYMBOL *) spb->data);
    }
    return NULL;
}


void
sym_macro_delete(SPTREE *sym_tbl)
{
    while (! spempty(sym_tbl)) {
        SPBLK *spb = sphead(sym_tbl);
        const char *name = spb->key;

        spdeq(spb, sym_tbl);                    /* dequeue node */

        if (name && '$' == name[0]) {           /* macro symbol table */
            SPTREE *fp = (SPTREE *)spb->data;

            sym_table_delete(fp);
            chk_free((void *)name);
            spfree(fp);
        } else {                                /* global */
            sym_free((SYMBOL *)spb->data);
        }
        spfreeblk(spb);                         /* release node */
    }
}


void
sym_table_delete(SPTREE *sym_tbl)
{
    while (! spempty(sym_tbl)) {
        SPBLK *spb = sphead(sym_tbl);
        SYMBOL *sp = (SYMBOL *)spb->data;

        spdeq(spb, sym_tbl);
        spfreeblk(spb);
        sym_free(sp);
    }
}


/*  Function:           sym_elookup
 *      Lookup a symbol, applying scoping rules (see sym_lookup) generating an error
 *      under a undefined condition.
 *
 *  Parameters:
 *      name - Symbol name.
 *
 *  Returns:
 *      Symbol object, otherwise NULL;
 */
SYMBOL *
sym_elookup(const char *name)
{
    SYMBOL *sp;

    if (NULL == (sp = sym_lookup(name))) {
        /*
         *  Force macro to return on an undefined symbol error.
         */
        ewprintf("Undefined symbol: %s", name);
       //acc_assign_null();                     /* XXX: 1/4/2020, review return value on error. */
        set_return();
    }
    return sp;
}


/*  Function:           sym_lookup
 *      Lookup a symbol, applying scoping rules.
 *
 *      Search order:
 *
 *      1. Static variable definition in the current function.
 *
 *      2. Buffer local variable.
 *
 *      3. Current local variables of a function.
 *
 *      4. Search all the nested stack frames, back to the outermost function call.
 *
 *      5. module global (static) variables.
 *
 *      6. global variables.
 *
 *  Parameters:
 *      name - Symbol name.
 *
 *  Returns:
 *      Symbol reference, otherwise NULL.
 */
SYMBOL *
sym_lookup(const char *name)
{
    SPTREE *mp = macro_symbols(mac_sp->module);
    const char *function = mac_sp->name;
    char fnbuf[SYM_FUNCNAME_LEN + 2]; 
    int loop;
    MACRO *mptr;
    SPBLK *spb = NULL;

    fnbuf[0] = '$', strxcpy(fnbuf+1, function, sizeof(fnbuf)-1);

    for (loop = 0; loop++ < 2;) {
        /* function (static) symbol table */
        if ((spb = splookup(fnbuf, mp)) != NULL) {
            /*
             *  function local 'static' symbol table ...
             */
            if ((spb = splookup(name, (SPTREE *)spb->data)) != NULL) {
                goto found;
            }
        }

        /* buffer symbol table */
        if ((spb = splookup(name, curbp->b_syms)) != NULL) {
            goto found;
        }

        /* local symbol tables */
        {   int i;
            for (i = x_nest_level; i > 0; --i) {
                if (NULL != (spb = splookup(name, x_lsym_tbl[i]))) {
                    goto found;
                }
            }
        }

        /* module (static) symbol table */
        if ((spb = splookup(name, mp)) != NULL) {
            goto found;
        }

        /* global symbol */
        if ((spb = splookup(name, x_gsym_tbl)) != NULL) {
            goto found;
        }

        /* macro? */
        mptr = macro_lookup(name);
        if (NULL == mptr || macro_autoload(mptr, FALSE) != 0) {
            break;
        }

        /*
         *  retry, with new macro; loop
         */
    }

    if (spb) {
found:  return sym_access((SYMBOL *) spb->data);
    }
    return NULL;
}


SYMBOL *
sym_access(SYMBOL *sp)
{
    while (F_REFERENCE == sp->s_type) {
        sp = sp->s_sym;                         /* dereference symbol */
    }
    if (SF_DYNAMIC & sp->s_flags) {
        (*sp->s_dynamic)(sp);                   /* update value */
    }
    return sp;
}


void
sym_rassociate(int idx, SYMBOL *sp)
{
    mac_registers_t *regs, *nregs;
    unsigned slots = 0, nslots;

    assert(mac_sd);
    assert(idx >= 0 && idx < 64);               /* system limit */
    assert(sp);

    if (idx >= 0) {
        if (NULL != (regs = mac_sp->registers) &&
                (unsigned)idx < (slots = regs->slots)) {
            /*
             *  available storage
             */
            assert(REGISTERS_MAGIC == regs->magic);
            assert(NULL == regs->symbols[idx]);
                /* register's should not be reassociated */
            regs->symbols[idx] = sp;

        } else {
            /*
             *  expand existing
             */
            assert(!regs || REGISTERS_MAGIC == regs->magic);

            nslots = ALIGN_UP(idx + 1, 16);     /* blocks of 16 */
            assert(nslots > (unsigned)idx);

            if (NULL != (nregs = (mac_registers_t *)
                    chk_recalloc(regs, sizeof(mac_registers_t) + (sizeof(SYMBOL *) * slots),
                        sizeof(mac_registers_t) + (sizeof(SYMBOL *) * nslots)))) {
                nregs->magic = REGISTERS_MAGIC;
                nregs->slots = nslots;
                assert(NULL == nregs->symbols[idx]);
                nregs->symbols[idx] = sp;
                mac_sp->registers = nregs;
            }
        }
    }
}


SYMBOL *
sym_rlookup(int idx)
{
    register mac_registers_t *regs;
    SYMBOL *sp = NULL;

    assert(idx >= 0 && idx < 64);               /* system limit */
    if (NULL != (regs = mac_sp->registers)) {
        assert(REGISTERS_MAGIC == regs->magic);
        if (idx >= 0 && (unsigned)idx < regs->slots) {
            sp = regs->symbols[idx];
        }
    }
    assert(sp); /*should exist*/
    return sp;
}


/*  Function:           sym_dump
 *      Dump the symbol tables
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
sym_dump(void)
{
#if defined(SPLAY_INTERNAL_STATS) && (SPLAY_INTERNAL_STATS)
    int i, j;

    printf("Symbol Table:\n");
    printf("Globals: %s\n", spstats(x_gsym_tbl));
    printf("Locals:\n");
    for (i = j = 0; i < _countof(x_lsym_tbl) && j < 3; ++i) {
        if (x_lsym_tbl[i]->lookups + x_lsym_tbl[i]->enqs + x_lsym_tbl[i]->splays == 0)
            j++;

        printf("  level %02d: %s\n", i, spstats(x_lsym_tbl[i]));
    }
#endif  /*SPLAY_INTERNAL_STATS*/
}


/*
 *  sym_isconstant ---
 *      Determine if the given symbol is constant.
 */
int
sym_isconstant(SYMBOL *sp, const char *msg)
{
    if (0 == (sp->s_flags & SF_CONSTANT)) {
        return 0;
    }
    ewprintf("%s: symbol '%s' is constant", msg, sp->s_name);
    return 1;
}


/*
 *  sym_clear ---
 *      Clear memory in object before assigning new value.
 */
static __CINLINE void
sym_clear(SYMBOL *sp)
{
    switch (sp->s_type) {
    case F_INT:
    case F_FLOAT:
    case F_NULL:
     // sp->s_int = 0;
        break;
    case F_REFERENCE:
        sp->s_sym = NULL;
        break;
    case F_STR:
    case F_LIST:
        r_dec(sp->s_obj);
        sp->s_obj = NULL;
        break;
    case F_LIT:
    case F_RLIST:
    case F_RSTR:
        panic("sym_clear: Unexpected type ? (0x%x/%d)", sp->s_type, sp->s_type);
        break;
    default:
        panic("sym_clear: Unknown type ? (0x%x/%d)", sp->s_type, sp->s_type);
        break;
    }
}


/*
 *  sym_assign_list ---
 *      Assign a freshly created list to a symbol.
 */
void
sym_assign_list(SYMBOL *sp, const LIST *lp)
{
    assert(F_LIST == sp->s_type);
    sym_clear(sp);
    sp->s_type = F_LIST;
    sp->s_obj = rlst_clone(lp);
    trace_sym(sp);
}


/*
 *  sym_donate_list ---
 *      Assign the list to a symbol.
 */
void
sym_donate_list(SYMBOL *sp, LIST *lp, int llen)
{
    assert(F_LIST == sp->s_type);
    sym_clear(sp);
    sp->s_type = F_LIST;
    sp->s_obj = rlst_create(lp, llen);
    trace_sym(sp);
}


/*
 *  sym_assign_ref ---
 *      Assign a reference to a symbol.
 */
void
sym_assign_ref(SYMBOL *sp, ref_t *rp)
{
    assert(F_STR == sp->s_type || F_LIST == sp->s_type || (sp->s_flags & SF_POLY));
    if (sp->s_obj != rp) {
        sym_clear(sp);
        sp->s_type = r_type(rp);
        sp->s_obj = r_inc(rp);
    }
    trace_sym(sp);
}


/*
 *  sym_donate_ref ---
 *      Assign a reference to a symbol.
 */
void
sym_donate_ref(SYMBOL *sp, ref_t *rp)
{
    assert(F_STR == sp->s_type || F_LIST == sp->s_type || (sp->s_flags & SF_POLY));
    assert(rp != sp->s_obj);
    sym_clear(sp);
    sp->s_type = r_type(rp);
    sp->s_obj = rp;
    trace_sym(sp);
}


/*
 *  sym_assign_str ---
 *      Assign a string to the symbol.
 */
void
sym_assign_str(SYMBOL *sp, const char *str)
{
    assert(F_STR == sp->s_type || (sp->s_flags & SF_POLY));
    sym_clear(sp);
    sp->s_type = F_STR;
    sp->s_obj = r_string(str);
    trace_ilog("  %s := %s\n", sp->s_name, (const char *)r_ptr(sp->s_obj));
}


void
sym_assign_nstr(SYMBOL *sp, const char *str, int len)
{
    assert(F_STR == sp->s_type || (sp->s_flags & SF_POLY));
    sym_clear(sp);
    sp->s_type = F_STR;
    sp->s_obj = r_nstring(str, len);
    trace_ilog("  %s := %s\n", sp->s_name, (const char *)r_ptr(sp->s_obj));
}


/*
 *  sym_assign_int ---
 *      Assign an integer to the symbol.
 */
void
sym_assign_int(SYMBOL *sp, accint_t value)
{
    assert(F_INT == sp->s_type || (sp->s_flags & SF_POLY));
    sym_clear(sp);
    sp->s_type = F_INT;
    sp->s_int = value;
    trace_ilog("  %s := %" ACCINT_FMT "\n", sp->s_name, value);
}


/*
 *  sym_assign_float ---
 *      Assign a float to the symbol, releasing any existing symol.
 */
void
sym_assign_float(SYMBOL *sp, accfloat_t value)
{
    assert(F_FLOAT == sp->s_type || (sp->s_flags & SF_POLY));
    sym_clear(sp);
    sp->s_type = F_FLOAT;
    sp->s_float = value;
    trace_ilog("  %s := %" ACCFLOAT_FMT "\n", sp->s_name, value);
}


/*  Function:           argv_assign_list
 *      Assign a copy of the list value to a symbol if the argv element is pointing to a symbol name.
 *
 *  Parameters:
 *      argi - Argument index.
 *      list - Value.
 *
 *  Returns:
 *      nothing
 */
void
argv_assign_list(int argi, const LIST *list)
{
    assert(argi > 0);
    assert(argi <= MAX_ARGC);
    if (!isa_undef(argi)) {
        sym_assign_list(get_symbol(argi), list);
    }
}


/*  Function:           argv_donate_list
 *      Donate the list to a symbol if the argv element is pointing to a
 *      symbol name, otherwise release the list resource.
 *
 *  Parameters:
 *      argi - Argument index.
 *      list - Value.
 *
 *  Returns:
 *      nothing
 */
void
argv_donate_list(int argi, LIST *lp, int llen)
{
    assert(argi > 0);
    assert(argi <= MAX_ARGC);
    assert(lp);
    if (!isa_undef(argi)) {
        sym_donate_list(get_symbol(argi), lp, llen);
    } else {
        lst_free(lp);
    }
}


/*  Function:           argv_assign_int
 *      Assign am integer value to a symbol if the argv element is pointing to a symbol name.
 *
 *  Parameters:
 *      argi - Argument index.
 *      val - Value.
 *
 *  Returns:
 *      nothing
 */
void
argv_assign_int(int argi, accint_t val)
{
    assert(argi > 0);
    assert(argi <= MAX_ARGC);
    if (!isa_undef(argi)) {
        sym_assign_int(get_symbol(argi), val);
    }
}


/*  Function:           argv_assign_str
 *      Assign a string value to a symbol if the argv element is pointing to a symbol name.
 *
 *  Parameters:
 *      argi - Argument index.
 *      val - Value.
 *
 *  Returns:
 *      Length of the assigned string, in bytes; not including NUL terminator.
 */
int
argv_assign_str(int argi, const char *val)
{
    int ret = 0;

    assert(argi > 0);
    assert(argi <= MAX_ARGC);
    if (!isa_undef(argi)) {
        if (val && *val) {
            ret = strlen(val);
            sym_assign_nstr(get_symbol(argi), val, ret);
        } else {
            sym_assign_nstr(get_symbol(argi), "", 0);
            ret = 0;
        }
    }
    return ret;
}


void
argv_assign_nstr(int argi, const char *val, int slen)
{
    assert(argi > 0);
    assert(argi <= MAX_ARGC);
    if (!isa_undef(argi)) {
        if (val && slen) {
            sym_assign_nstr(get_symbol(argi), val, slen);
        } else {
            sym_assign_nstr(get_symbol(argi), "", 0);
        }
    }
}


/*
 *  system_call ---
 *      Function called with the result of a system call. If the system call failed,
 *      then update the global variable errno with the C version of errno.
 */
int
system_call(int ret)
{
    if (ret < 0) {
        *x_errno_ptr = (accint_t) errno;
    }
    return ret;
}


/*
 *  system_errno -
 *      Assign the global 'errno' to the specific value 'ret'.
 */
void
system_errno(int ret)
{
    *x_errno_ptr = (accint_t) ret;
}

/*end*/
