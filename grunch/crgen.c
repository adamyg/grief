#include <edidentifier.h>
__CIDENT_RCSID(gr_crgen_c,"$Id: crgen.c,v 1.32 2014/10/22 02:33:28 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crgen.c,v 1.32 2014/10/22 02:33:28 ayoung Exp $
 * generic code generator routines.
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

#include "grunch.h"                             /* local definitions */

static int              globals_compile(int);
static void             globals_cleanup(void);

static void             compile_node2(const node_t *np, int level);

static void             open_bracket(const node_t *np);
static void             gen_bracket(const node_t *np, int level);
static void             gen_keywd(const char *keywd);

static void             funct_init(void);
static void             funct_push0(void);
static void             funct_push(const char *name, int lineno);
static void             funct_type(const char *name, argtype_t type);
static void             funct_symtypeof(const char *name, symtype_t stype);
static void             funct_typeof(const char *name);
static void             funct_check(void);
static void             funct_print(void);
static void             funct_pop(void);

int                     x_generate_ascii = TRUE;

static int              x_initialisor_level = 0;
static int              x_globals = 0;

static const gen_t      gen_ascii =
    {
        gena_macro,
        gena_list,
        gena_id,
        gena_lit,
        gena_end_list,
        gena_int,
        gena_string,
        gena_float,
        gena_token,
        gena_finish,
        gena_null
    };

static const gen_t      gen_binary =
    {
        genb_macro,
        genb_list,
        genb_id,
        genb_lit,
        genb_end_list,
        genb_int,
        genb_string,
        genb_float,
        genb_token,
        genb_finish,
        genb_null
    };


#define gen_macro()     x_generate_ascii ? \
                            (*gen_ascii.g_macro)() :    (*gen_binary.g_macro)()
#define gen_list()      x_generate_ascii ? \
                            (*gen_ascii.g_list)() :     (*gen_binary.g_list)()
#define gen_id(id)      x_generate_ascii ? \
                            (*gen_ascii.g_id)(id) :     (*gen_binary.g_id)(id)
#define gen_lit(l)      x_generate_ascii ? \
                            (*gen_ascii.g_lit)(l) :     (*gen_binary.g_lit)(l)
#define gen_end_list()  x_generate_ascii ? \
                            (*gen_ascii.g_end_list)() : (*gen_binary.g_end_list)()
#define gen_int(n)      x_generate_ascii ? \
                            (*gen_ascii.g_int)(n) :     (*gen_binary.g_int)(n)
#define gen_string(s)   x_generate_ascii ? \
                            (*gen_ascii.g_string)(s) :  (*gen_binary.g_string)(s)
#define gen_float(f)    x_generate_ascii ? \
                            (*gen_ascii.g_float)(f) :   (*gen_binary.g_float)(f)
#define gen_token(t)    x_generate_ascii ? \
                            (*gen_ascii.g_token)(t) :   (*gen_binary.g_token)(t)
#define gen_finish()    x_generate_ascii ? \
                            (*gen_ascii.g_finish)() :   (*gen_binary.g_finish)()
#define gen_null()      x_generate_ascii ? \
                            (*gen_ascii.g_null)() :     (*gen_binary.g_null)()


/*
 *  init_codegen ---
 *      initialise the code generator.
 */
void
init_codegen(void)
{
    x_globals = 0;
    x_initialisor_level = 0;
    init_ascii();
    init_binary();
    funct_init();
}


void
compile_main(node_t *maintree)
{
    if (maintree) {
        compile_func(SC_STATIC<<SC_SHIFT, "_init", NULL, maintree);

    } else if (hd_syms && ll_first(hd_syms)) {
        gen_macro();
        gen_keywd("macro");
        gen_int(0x01);                          /* static */
        gen_id("_init");
        if (globals_compile(FALSE)) {
            gen_end_list();
        }
        gen_end_list();
    }
    globals_cleanup();
    gen_finish();
    funct_init();
}


static int
reserved_builtin(const char *function)
{
    static const char *
        x_reserved[] = {
            "_init",
            "main",
            "execute_macro"
            };
    unsigned i;

    for (i = 0; i < (sizeof(x_reserved)/sizeof(x_reserved[0])); ++i) {
        if (0 == strcmp(function, x_reserved[i])) {
            return 1;
        }
    }
    return 0;
}


void
compile_func(symtype_t type, const char *function, Head_p arglist, node_t *stmts)
{
    int is_replacement = ((type & SC_MASK) == (SC_REPLACEMENT<<SC_SHIFT)) ? 1 : 0;
    int is_static = ((type & SC_MASK) == (SC_STATIC << SC_SHIFT)) ? 1 : 0;
    int do_end = FALSE;
    int mode = 0;

    /*
     *  Replacement warning
     */
    if (builtin_lookup(function)) {
        if (reserved_builtin(function)) {
            crerrorx(RC_REPLACING_BUILTIN, "replacement of reserved function '%s'", function);

        } else {
            if (! is_replacement) {
                crwarnx(RC_REPLACING_BUILTIN, "implicit replacement of the builtin function '%s'", function);
                is_replacement = 1;             /* implicit replacement */
            }
        }
    } else {
        if (is_replacement) {
            crwarnx(RC_REPLACING_MACRO, "replacing non-builtin macro '%s'", function);
        }
    }

    gen_macro();

    /*
     *  macro header and macro name part
     */
    gen_keywd("macro");
    if (is_static)
        mode |= 0x01;                           /* CM_MACRO_FSTATIC */
    if (is_replacement)
        mode |= 0x02;                           /* CM_MACRO_FREPLACEMENT */
    if (mode)
        gen_int(mode);
    gen_lit(function);

    /*
     *  no code in the function, just terminate the macro definition
     */
    if (NULL == stmts) {
        gen_end_list();
        return;
    }

    if (0 == strcmp(function, "_init")) {
        do_end = globals_compile(stmts != NULL);
    } else {
        do_end = compile_arglist(arglist, stmts != NULL);
    }

    if (stmts) {
        /*
         * If we got a multi-line macro, dont enclose the whole lot in yet another
         * layer of list brackets
         */
        if (stmts->type == node_keywd && stmts->atom.ival == K_BLOCK) {
            compile_list((Head_p) stmts->right, 1, do_end, FALSE);

        } else {
            if (!do_end && stmts->left && stmts->right) {
                gen_list();
            }
            compile_node(stmts, 1);
            if (!do_end && stmts->left && stmts->right) {
                gen_end_list();
            }
        }

        if (do_end) {
            gen_end_list();
        }
    }

    gen_end_list();
    if (do_end) {
        gen_end_list();
    }
}


/*
 *  The following function is used to generate code for all the global symbols we've seen
 */
static int
globals_compile(int flag)
{
    register List_p lp;
    register symbol_t *sp;
    symtype_t ty, tm, sc, qu;
    int got_decl = 0;
    symprim_t j;

    /* Dont generate anything if no symbols defined */
    if (NULL == hd_syms || NULL == ll_first(hd_syms)) {
        return FALSE;
    }

    /* We must have at least one non-extern declaration before we start outputting
     * any code. So we need to have a quick peek at the declarations list.
     */
    for (lp = ll_first(hd_syms); got_decl != 0x03 && lp; lp = ll_next(lp)) {
        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
        sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
        tm = (sp->s_type & TM_MASK) >> TM_SHIFT;
        ty = (sp->s_type & TY_MASK);

        if (tm != TM_FUNCTION && sc != SC_EXTERN) {
            if (qu & TQ_CONST) {
                got_decl |= 0x04;
            }

            switch (ty) {
            case TY_INT:
            case TY_STRING:
            case TY_ENUMI:
            case TY_LIST:
            case TY_ARRAY:
            case TY_DECLARE:
            case TY_FLOAT:
            case TY_DOUBLE:
                got_decl |= (sc == SC_STATIC ? 0x02 : 0x01);
                ++x_globals;
                break;
            }
        }
    }

    if (! got_decl) {
        return FALSE;
    }

    if (flag) {
        gen_list();
    }

    gen_list();

    /* Output declarations, but merge all declarations of each type */
    for (j = 0; j < SYMPRIM_MAX; ++j) {
        const char *keywd = "";
        int done_keywd = FALSE;

        for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
            sp = (symbol_t *) ll_elem(lp);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
            sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
            tm = (sp->s_type & TM_MASK) >> TM_SHIFT;
            ty = (sp->s_type & TY_MASK);

            if (tm != TM_FUNCTION && sc != SC_EXTERN) {
                switch (j) {
                case SYMPRIM_INT:
                    if (TY_INT != ty) {
                        if (TY_ENUMI != ty || TY_INT != enum_type(sp)) {
                            continue;
                        }
                    }
                    keywd = "int";
                    break;
                case SYMPRIM_STRING:
                    if (TY_STRING != ty) {
                        if (TY_ENUMI != ty || TY_STRING != enum_type(sp)) {
                            continue;
                        }
                    }
                    keywd = "string";
                    break;
                case SYMPRIM_LIST:
                    if (TY_LIST != ty)
                        continue;
                    keywd = "list";
                    break;
                case SYMPRIM_ARRAY:
                    if (TY_ARRAY != ty)
                        continue;
                    keywd = "array";
                    break;
                case SYMPRIM_DECLARE:
                    if (TY_DECLARE != ty)
                        continue;
                    keywd = "declare";
                    break;
                case SYMPRIM_FLOAT:
                    if (TY_FLOAT != ty && TY_DOUBLE != ty)
                        continue;
                    keywd = "float";
                    break;
                default:
                    assert(0);
                    break;
                }

                if (! done_keywd) {
                    gen_keywd(keywd);
                    done_keywd = TRUE;
                }
                gen_id(sp->name);
            }
        }

        if (done_keywd) {
            gen_end_list();
        }
    }

    /* Output any initialisation code for each global */
    if (hd_globals)
        for (lp = ll_first(hd_globals); lp; lp = ll_next(lp)) {
            compile_node((node_t *) ll_elem(lp), 1);
        }

    /* const */
    if ((got_decl & 0x04) && (got_decl & (0x01 | 0x02))) {
        gen_keywd("const");
        for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
            sp = (symbol_t *) ll_elem(lp);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
            sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
            tm = (sp->s_type & TM_MASK) >> TM_SHIFT;

            if (tm != TM_FUNCTION && sc != SC_EXTERN) {
                if (qu & TQ_CONST)
                    switch (ty) {
                    case TY_INT:
                    case TY_STRING:
                    case TY_ENUMI:
                    case TY_LIST:
                    case TY_ARRAY:
                    case TY_DECLARE:
                    case TY_FLOAT:
                    case TY_DOUBLE:
                        gen_lit(sp->name);
                        break;
                    }
            }
        }
        gen_end_list();
    }

    /* Now output the (global ...) stuff so that symbols get stored in the global symbol table */
    if (got_decl & 0x01) {
        gen_keywd("global");
        for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
            sp = (symbol_t *) ll_elem(lp);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
            sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
            tm = (sp->s_type & TM_MASK) >> TM_SHIFT;
            ty = (sp->s_type & TY_MASK);

            if (tm != TM_FUNCTION && sc != SC_EXTERN && sc != SC_STATIC) {
                switch (ty) {
                case TY_INT:
                case TY_STRING:
                case TY_ENUMI:
                case TY_LIST:
                case TY_ARRAY:
                case TY_DECLARE:
                case TY_FLOAT:
                case TY_DOUBLE:
                    gen_lit(sp->name);
                    break;
                }
            }
        }
        gen_end_list();
    }

    /* Now output the (static ...) */
    if (got_decl & 0x02) {
        gen_keywd("static");
        for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
            sp = (symbol_t *) ll_elem(lp);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
            sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
            tm = (sp->s_type & TM_MASK) >> TM_SHIFT;
            ty = (sp->s_type & TY_MASK);

            if (tm != TM_FUNCTION && sc == SC_STATIC) {
                switch (ty) {
                case TY_INT:
                case TY_STRING:
                case TY_ENUMI:
                case TY_LIST:
                case TY_ARRAY:
                case TY_DECLARE:
                case TY_FLOAT:
                case TY_DOUBLE:
                    gen_lit(sp->name);
                    break;
                }
            }
        }
        gen_end_list();
    }

    return TRUE;
}


/*
 *  Release of global symbols.
 */
static void
globals_cleanup(void)
{
    if (hd_syms) {
        List_p lp;

        while (NULL != (lp = ll_first(hd_syms))) {
            symbol_t *sp = (symbol_t *) ll_elem(lp);

            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)
            sym_free(sp);
            ll_delete(lp);
        }
    }
}


/*
 *  Generate code for the declaration of arguments to a function
 *
 *  Argument syntax:
 *      type name
 *          Argument is mandatory, and of type string and is referred to as name in the function.
 *
 *      ~type name
 *          Argument is optional (can be omitted in the call or passed as NULL), and is referred
 *          to as name in the function.
 *
 *      type name = constant-expr
 *          Argument is optional. If omitted from the function call, then constant expression
 *          be used as a default value.
 *
 *      ~type
 *          Argument is optional/unnamed and is not directly accessible in the defining
 *          function by name. Typically this is used for place-holder arguments. The
 *          actual argument can be accessed by calling the get_parm() primitive.
 */
int
compile_arglist(Head_p arglist, int flag)
{                                               /* 17/08/08 */
    register List_p lp;
    register node_t *np;
    symtype_t old_ty = 0;
    int arg, dots_seen = FALSE;

    if (NULL == arglist)
        return FALSE;

    if (flag)
        gen_list();
    gen_list();

    /* declarations */
    for (lp = ll_first(arglist); lp; lp = ll_next(lp)) {
        np = (node_t *) ll_elem(lp);

        if (NULL == np) {
            dots_seen = TRUE;

        } else {
            assert(node_type == np->type);

            if (np->right) {
                /*
                 *  named argument
                 */
                const node_t *npsym = np->right;
                symtype_t ty = (np->atom.ival & TY_MASK);

                assert(node_symbol == npsym->type);
                assert(NULL == npsym->left);

                if (TY_ENUMI == ty) {           /* 01/06/09 */
                    assert(npsym->sym);
                    sym_check(npsym->sym);
                    ty = enum_type(npsym->sym);
                }

                if (ty != old_ty) {             /* XXX - check for supported types */
                    if (old_ty)
                        gen_end_list();
                    gen_list();
                    gen_id(symtype_to_str(ty));
                }

                gen_lit(np->right->atom.sval);
                old_ty = ty;
            }
        }
    }

    if (old_ty) {
        gen_end_list();
    }

    /* assignments */
    for (arg = 0, lp = ll_first(arglist); lp; lp = ll_next(lp)) {
        np = (node_t *) ll_elem(lp);

        if (np) {
            assert(node_type == np->type);

            if (np->right) {
                /*
                 *  named argument
                 */
                const node_t *npsym = np->right;

                symtype_t tm = ((np->atom.ival & TM_MASK) >> TM_SHIFT);
                symtype_t qu = ((np->atom.ival & TQ_MASK) >> TQ_SHIFT);

                assert(node_symbol == npsym->type);
                assert(NULL == npsym->left);

                if (TM_REFERENCE & tm) {        /* references, 19/10/08 */
                    gen_keywd("ref_parm");
                    gen_int((accint_t) arg);
                    gen_string(npsym->atom.sval);

                } else {
                    gen_keywd("get_parm");
                    gen_int((accint_t) arg);
                    gen_lit(npsym->atom.sval);

                    if (npsym->right) {         /* 11/10/08 */
                        const node_t *npdef = npsym->right;

                        gen_null();
                        gen_null();
                        switch (npdef->type) {
                        case node_integer:
                            gen_int(npdef->atom.ival);
                            break;
                        case node_float:
                            gen_float(npdef->atom.fval);
                            break;
                        case node_string:
                            gen_string(npdef->atom.sval);
                            break;
                        default:
                            assert(0);
                            gen_null();
                            break;
                        }
                    }
                }
                gen_end_list();

                if (qu & TQ_CONST) {            /*18/01/07*/
                    gen_keywd("const");
                    gen_lit(npsym->atom.sval);
                    gen_end_list();
                }
            }
        }
        ++arg;
    }
    return TRUE;
}


void
compile_list(Head_p hd, int level, int flag, int sep_stmt)
{
    register List_p lp;
    int complex_list = FALSE;
    node_t *np;

    if (NULL == hd)
        return;
    lp = ll_first(hd);
    if (lp) {
        if (ll_next(lp)) {
            complex_list = TRUE;
        } else {
            np = (node_t *) ll_elem(lp);
            if (np && np->type == node_keywd && np->atom.ival == K_NOOP) {
                complex_list = TRUE;
            }
        }
    }

    if (complex_list && !flag) {
        gen_list();
    }

    while ((lp = ll_first(hd)) != NULL) {
        np = (node_t *) ll_elem(lp);

        /* Make sure code like: '1;2;3;' causes each constant to be inside
         * its own set of brackets (sep_stmt == TRUE).
         */
        if (np) {
            if (sep_stmt) {
                switch (np->type) {
                case node_string:
                case node_integer:
                case node_float:
                    gen_list();
                    break;
                default:
                    break;
                }
            }

            compile_node(np, level);

            if (sep_stmt) {
                switch (np->type) {
                case node_string:
                case node_integer:
                case node_float:
                    gen_end_list();
                    break;
                default:
                    break;
                }
            }
        }
        ll_delete(lp);
    }
    ll_free(hd);

    if (complex_list && !flag) {
        gen_end_list();
    }
}


/*
 *  Generate code for a parse tree
 */
void
compile_node(const node_t *tree, int level)
{
    static int deref = 0;

    if (NULL == tree)
        return;

    if (node_keywd == tree->type) {
        switch (tree->atom.ival) {
        case K_INITIALIZER:     /* initialiser */
            gen_list();
            if (0 == x_initialisor_level++)
                gen_id("quote_list");
            compile_node(tree->right, level);
            if (tree->left) {
                gen_keywd("quote_list");
                compile_node(tree->left, level);
            }
            gen_end_list();
            --x_initialisor_level;
            break;

        case K_CONSTRUCTORS:    /* constructor calls */
            compile_node(tree->right, level);
            compile_list((Head_p) tree->left, level, FALSE, FALSE);
            break;

        case K_LVALUE:          /* put_nth */
            funct_push0();
            open_bracket(tree);
            gen_id("put_nth");
            ++deref;
            compile_node(tree->left, level);    /* reference, INDEX [INDEX] */
            --deref;
            compile_node(tree->right, level);   /* VALUE */
            gen_end_list();
            funct_pop();
            funct_type("put_nth", ARG_ANY);
            break;

        case O_OSQUARE: {       /* get_nth */
                int oderef;

                if ((oderef = deref++) == 0) {
                    funct_push0();
                    gen_keywd("nth");
                }

                if (tree->left->type != node_keywd || tree->left->atom.ival != O_OSQUARE)
                    deref = 0;
                compile_node(tree->left, level);

                if (tree->right->type == node_keywd)
                    deref = 0;
                compile_node(tree->right, level);

                if ((deref = oderef) == 0) {
                    gen_end_list();
                    funct_pop();
                    funct_type("nth", ARG_ANY);
                }
            }
            break;

        case K_FUNCALL:         /* function call */
            if (xf_unused && 0 == strcmp(tree->left->atom.sval, "UNUSED")) {
                break;                          /*11/10/08*/
            }
            funct_typeof(tree->left->atom.sval);
            open_bracket(tree);
            gen_id(tree->left->atom.sval);
            funct_push(tree->left->atom.sval, tree->lineno);
            compile_node(tree->right, level);
            funct_check();
            gen_end_list();
            funct_pop();
            break;

        case K_GETPROPERTY:     /* get_property, 27/07/08 */
            funct_push0();
            gen_keywd("get_property");
            compile_node(tree->left, level);
            compile_node(tree->right, level);
            gen_end_list();
            funct_pop();
            funct_typeof("get_property");
            break;

        case K_SETPROPERTY:     /* set_property, 27/07/08 */
            funct_push0();
            gen_keywd("set_property");
            compile_node(tree->left, level);
            compile_node(tree->right, level);
            gen_end_list();
            funct_pop();
            funct_typeof("set_property");
            break;

        case K_COND:            /* (<expr> ? <true> : <false>), 30/07/08 */
            funct_push0();
            gen_keywd("if");
            compile_node(tree->left, level);
            compile_list((Head_p) tree->right->left, level, FALSE, FALSE);
            compile_list((Head_p) tree->right->right, level, FALSE, FALSE);
            gen_end_list();
            funct_pop();
            funct_type("?", ARG_ANY);
            break;

        case K_BLOCK:           /* { <expr> } */
            compile_list((Head_p) tree->right, level, FALSE, TRUE);
            break;

        case K_SWITCH: {        /* switch <value> <cases> <default> */
                const node_t *casep;

                gen_keywd("switch");
                compile_node(tree->left, level);
                if (NULL != (casep = tree->right)) {
                    compile_node(casep->left, level + 1);
                    compile_node(casep->right, level + 1);
                }
                gen_end_list();
            }
            break;

        case K_CASE:            /* case [<value>] <expr> */
            gen_bracket(tree->left, level);
            if (NULL == tree->right) {
                gen_null();                     /* default */
            } else {
                const int iscomplex = (tree->right->left && tree->right->right);

                if (iscomplex)
                    open_bracket(tree);
                compile_list((Head_p) tree->right, level, FALSE, TRUE);
                if (iscomplex)
                    gen_end_list();
            }
            break;

        case K_BREAKSW:         /* switch break */
            gen_keywd("__breaksw");
            gen_end_list();
            break;

        case K_DEFAULT:         /* default */
            gen_null();
            compile_node(tree->right, level);
            break;

        case K_IF:              /* if (xxxx) aaa */
            gen_keywd("if");
            compile_node(tree->left, level);
            compile_list((Head_p) tree->right, level, FALSE, FALSE);
            gen_end_list();
            break;

        case K_ELSE:            /* if <cond> <true> [else <false>] */
            gen_keywd("if");
            compile_node(tree->left->left, level);
            compile_list((Head_p) tree->left->right, level, FALSE, FALSE);
            compile_list((Head_p) tree->right, level, FALSE, FALSE);
            gen_end_list();
            break;

        case K_WHILE:           /* while <cond> <expr> */
            gen_keywd("while");
            compile_node(tree->left, level);
            compile_list((Head_p) tree->right, level, FALSE, TRUE);
            gen_end_list();
            break;

        case K_FOR:             /* for <init> <cond> <post> [<expr>] */
            gen_keywd("for");
            gen_bracket(tree->left, level);
            gen_bracket(tree->right->left, level);
            gen_bracket(tree->right->right->left, level);
            if (tree->right->right->right) {    /* <expr> */
                compile_list((Head_p) tree->right->right->right, level, FALSE, TRUE);
            }
            gen_end_list();
            break;

        case K_FOREACH:         /* foreach <expr> <stmt> <value> <idx> */
            gen_keywd("foreach");
            gen_bracket(tree->left->left, level);
            compile_list((Head_p) tree->left->right, level, FALSE, TRUE);
            gen_bracket(tree->right->left, level);
            gen_bracket(tree->right->right, level);
            gen_end_list();
            break;

        case K_TRY:             /* try <expr> [catch <>] [finally <>] */
            gen_keywd("try");
            gen_bracket(tree->left, level);     /* expr */
            if (tree->right->left) {            /* catch[s] */
                compile_list((Head_p) tree->right->left, level, FALSE, TRUE);
            }
            if (tree->right->right) {           /* finally */
                gen_bracket(tree->right->right, level);
            }
            gen_end_list();
            break;

        case K_CATCH:           /* catch <ident> [<cond>] <expr>, 11/2010 */
            gen_keywd("catch");
            compile_node(tree->left->left, level);
            if (NULL == tree->left->right) {
                gen_null();
            } else {
                compile_node(tree->left->right, level);
            }
            compile_node(tree->right, level);
            gen_end_list();
            break;

        case K_FINALLY:         /* try .. finally <expr>, 11/2010 */
            gen_bracket(tree->left, level);
            gen_bracket(tree->right, level);
            break;

        case K_FLOAT:           /* constants */
        case K_DOUBLE:
        case K_INT:
        case K_STRING:
        case K_LIST:
        case K_ARRAY:
        case K_DECLARE:
        case K_BOOL:
     // case K_HASH:
     // case K_ARRAY:
            goto DEFAULT;

        case K_NOOP:            /* group */
            compile_node(tree->left, level);
            compile_node(tree->right, level);
            break;

        /*
         *  Handle post-inc/decrement operators here because there aren't
         *  any valid tokens for these on input. (They are positional)
         */
        case O_POST_PLUS_PLUS:
        case O_POST_MINUS_MINUS:
            gen_keywd(tree->atom.ival == O_POST_PLUS_PLUS ? "post++" : "post--");
            compile_node(tree->left, level);
            gen_end_list();
            break;

DEFAULT:
        default:
            compile_node2(tree, level);
            break;
        }

    } else {
        compile_node2(tree, level);
    }
}


static void
compile_node2(const node_t *np, int level)
{
    switch (np->type) {
    case node_symbol: {
            const char *sval = np->atom.sval;

            assert(NULL == np->left);
                                                // NULL or null
            if ((sval[0] == 'N' && 0 == strcmp(sval, "NULL")) ||
                (sval[0] == 'n' && 0 == strcmp(sval, "null"))) {
                gen_null();
                funct_type(np->atom.sval, ARG_NULL);

                                                // true
            } else if (sval[0] == 't' && 0 == strcmp(sval, "true")) {
                gen_int(1);
                funct_type(NULL, ARG_INT);
                                                // false
            } else if (sval[0] == 'f' && 0 == strcmp(sval, "false")) {
                gen_int(0);
                funct_type(NULL, ARG_INT);

            } else {                            //  symbol name
                gen_lit(sval);
                if (np->sym) {
                    sym_check(np->sym);
                    funct_symtypeof(sval, np->sym->s_type);
                } else {
                    funct_typeof(sval);
                }
            }
        }
        break;

    case node_integer:
        gen_int(np->atom.ival);
        funct_type(NULL, ARG_INT);
        break;

    case node_float:
        gen_float(np->atom.fval);
        funct_type(NULL, ARG_FLOAT);
        break;

    case node_string:
        gen_string(np->atom.sval);
        funct_type(np->atom.sval, ARG_STRING);
        break;

    case node_keywd:
        funct_push0();
        open_bracket(np);
        if (np->atom.ival != O_COMMA) {
            gen_token((int) np->atom.ival);
        }
        break;

    case node_arglist:
        assert(0);
        return;
    }

    if (np->left) {
        compile_node(np->left, level + 1);
    }

    if (np->right) {
        if (np->type == node_keywd && np->atom.ival == K_WHILE &&
                (np->right->type == node_keywd && np->right->atom.ival == K_NOOP)) {
            gen_list();
            compile_node(np->right, level + 1);
            gen_end_list();
        } else {
            compile_node(np->right, level + 1);
        }
    }

    if (np->type == node_keywd) {
        gen_end_list();
        funct_pop();
        funct_type(NULL, ARG_ANY);
    }
}


/*
 *  Start a new list, and if the debug flag is set, add a call to
 *  the debugger so we can see whats going on
 */
static void
open_bracket(const node_t *np)
{
    gen_list();
    if (xf_debugger) {
        gen_list();
        gen_id("__dbg_trace__");
        gen_int((accint_t) np->lineno);
        gen_string(x_filename);
        gen_string(x_funcname ? x_funcname : "");
        gen_end_list();
    }
}


/*
 *  Generate code for a tree, and enclose it in parenthesis if we
 *  have a complex tree (i.e. more than one statement)
 */
static void
gen_bracket(const node_t *np, int level)
{
    if (NULL == np) {
        gen_null();
    } else if (np->type == node_keywd && K_NOOP == np->atom.ival) {
        gen_list();
        compile_node(np, level);
        gen_end_list();
    } else {
        compile_node(np, level);
    }
}


static void
gen_keywd(const char *str)
{
    gen_list();
    gen_id(str);
}


/*
 *  Function prototype checks (30/07/08)
 */
typedef struct funct {
    const char *        f_name;
    int                 f_level;
    int                 f_lineno;
    unsigned            f_push;
    unsigned            f_argc;
    argtype_t           f_types[32];
    const char *        f_names[32];

#define FUNCT_TYPES     (sizeof(((funct_t *)NULL)->f_types)/sizeof(((funct_t *)NULL)->f_types[0]))

} funct_t;

static int              funct_level = -1;
static funct_t *        funct_data[1024];

#define FUNCT_LEVELS    (sizeof(funct_data)/sizeof(funct_data[0]))

static void
funct_init(void)
{
    while (funct_level >= 0) {
        chk_free(funct_data[funct_level]);
        funct_data[funct_level] = NULL;
        --funct_level;
    }
    assert(funct_level == -1);
    funct_push("_init", 0);
}


static void
funct_push0(void)
{
    assert(funct_level >= 0);
    assert(funct_level < (int)FUNCT_LEVELS);
    assert(funct_data[funct_level]);

    ++funct_data[funct_level]->f_push;
}


static void
funct_push(const char *name, int lineno)
{
    funct_t *funct = chk_alloc(sizeof(funct_t));

    assert(funct_level >= -1);
    assert(funct_level < (int)FUNCT_LEVELS);

    funct->f_name   = name;
    funct->f_level  = ++funct_level;
    funct->f_lineno = lineno;
    funct->f_push   = 0;
    funct->f_argc   = 0;

    assert(NULL == funct_data[funct_level]);
    funct_data[funct_level] = funct;
}


static void
funct_type(const char *name, argtype_t type)
{
    funct_t *funct = funct_data[funct_level];

    if (funct) {                                /* push a type */
        if (0 == funct->f_push) {
            if (funct->f_argc < FUNCT_TYPES) {
                funct->f_names[ funct->f_argc ] = name;
                funct->f_types[ funct->f_argc ] = type;
            }
            ++funct->f_argc;
        }
    }
}


static void
funct_typeof(const char *symbol)
{
    argtype_t type = ARG_SYMBOL;                /* unknown */
    BUILTIN *b;

    if (NULL != (b = builtin_lookup(symbol))) {
        type = b->b_rtntype;                    /* builtin return-type */
    }
    funct_type(symbol, type);
}


static void
funct_symtypeof(const char *name, symtype_t stype)
{
    funct_type(name, symtype_arg(stype));
}


static argtype_t
arg_mask(argtype_t type)
{
    return type & ~(ARG_COND|ARG_REST|ARG_OPT|ARG_LVAL);
}


static const char *
arg_type(unsigned type)
{
    switch (arg_mask(type)) {
    case ARG_ANY:    return "poly";
    case ARG_NUM:    return "numeric";

    case ARG_INT:    return "int";
    case ARG_FLOAT:  return "float";
    case ARG_STRING: return "string";
    case ARG_LIST:   return "list";
    case ARG_ARRAY:  return "array";

    case ARG_VOID:   return "void";
    case ARG_NULL:   return "null";
    case ARG_SYMBOL: return "symbol";
    case ARG_UNDEF:  return "undef";
    }
    return "n/a";
}


static void
funct_check(void)
{
    funct_t *funct = funct_data[funct_level];

    if (funct) {                                /* function call level */
        if (0 == funct->f_push) {

            const BUILTIN *bp;
            const argtype_t *bp_types = NULL;
            symbol_t *sp = sym_lookup(funct->f_name, SYMLK_FUNDEF);
            int is_defined = 0;
            int is_function = 0;
            int is_replacement = 0;
            int bp_argc = 0;

            funct_print();

            /*
             *  determine prototype
             */
            if ((bp = builtin_lookup(funct->f_name)) != NULL) {
                /*
                 *  builtin primitive
                 */
                is_defined = is_function = 1;
                bp_types = bp->b_arg_types;
                bp_argc = bp->b_argc;
            }

            if (NULL != (sp = sym_lookup(funct->f_name, SYMLK_FUNDEF))) {
                /*
                 *  symbol
                 */
                const symtype_t type = sp->s_type;

                is_defined = 1;
                is_function = ((((type & TM_MASK) >> TM_SHIFT) & TM_FUNCTION) ? 1 : 0);
                is_replacement = (((type & SC_MASK) == (SC_REPLACEMENT << SC_SHIFT)) ? 1 : 0);

                if (NULL == bp_types) {
                    /*
                     *  Use symbol definition unless builtin
                     */
                    if (sp->s_arguments) {
                        bp_argc = sp->s_arguments[0];
                        bp_types = sp->s_arguments + 1;
                    } else {
                        bp_argc = 0;
                    }
                }
            }

            /*verify arguments*/
            if (is_defined) {
                if (! is_function) {
                    crwarnx_line(RC_ERROR, funct->f_lineno, "'%s', not a function", funct->f_name);
                }

            } else {                            /* 30/04/10 - undefine, implied prototype */
                crwarnx_line(RC_ERROR, funct->f_lineno,
                    "'%s' undefined; assuming extern return int", funct->f_name);
                sym_implied_function(funct->f_name, funct->f_argc, funct->f_types);
            }

            if (bp_types) {
                const argtype_t *f_types = funct->f_types;
                int f_argc = funct->f_argc;

                int indefinite_args = 0;
                int arg = 1;

                if (bp_argc < 0) {              /* like ... */
                    bp_argc = -bp_argc;
                    indefinite_args = 1;
                }

                while (bp_argc > 0 && f_argc > 0) {
                    if (*bp_types & ARG_REST) {
                        f_argc = 0;             /* like va_start */

                    } else {
                        switch (*f_types) {
                        case ARG_VOID:
                            crwarnx_line(RC_ERROR, funct->f_lineno,
                                "'%s', actual parameter %d has type 'void'", funct->f_name, arg);
                            break;

                        case ARG_NULL:
                            if (is_replacement) {
                                xprintf("\treplacement argument is NULL\n");
                            } else {
                                if (0 == (*bp_types & (ARG_OPT|ARG_LIST))) {
                                    crwarnx_line(RC_ERROR, funct->f_lineno,
                                        "'%s', actual parameter %d is neither list nor optional", funct->f_name, arg);
                                    return;
                                }
                            }
                            break;

                        case ARG_SYMBOL:
                        case ARG_UNDEF:
                            /*TODO - match types*/
                            break;

                        default:
                            if ((*bp_types & arg_mask(*f_types)) == 0) {
                                crwarnx_line(RC_ERROR, funct->f_lineno,
                                    "'%s', different types for formal '%s' and actual '%s' parameter %d",
                                        funct->f_name, arg_type(*bp_types), arg_type(*f_types), arg);
                            }
                            break;
                        }

                        --f_argc; ++f_types;    /* next function call argument */
                    }

                    --bp_argc; ++bp_types;      /* next built argument */
                    ++arg;
                }

                if (! indefinite_args) {
                    if (f_argc > 0) {
                        crwarnx_line(RC_ERROR, funct->f_lineno,
                            "'%s', too many actual parameters, starting with parameter %d", funct->f_name, arg);
                        return;
                    }
                }

                while (bp_argc > 0) {
                    if (*bp_types & (ARG_REST|ARG_VOID)) {
                        break;                  /* none or more ... */
                    }

                    if (0 == (*bp_types & ARG_OPT)) {
                        crwarnx_line(RC_ERROR, funct->f_lineno,
                            "'%s', too few actual parameters, starting at parameter %d",
                            funct->f_name, arg);
                        return;
                    }

                    --bp_argc; ++bp_types;      /* next built argument */
                    ++arg;
                }
            }
        }
    }
}


static void
funct_print(void)
{
    const funct_t *funct = funct_data[funct_level];

    if (funct) {                                /* function call level */
        if (0 == funct->f_push) {
            const char *n = NULL, *p = NULL;
            unsigned i;

            xprintf("%*s%s(", funct->f_level * IDENT_LEVEL, "", funct->f_name);

            for (i = 0; i < funct->f_argc && i < FUNCT_TYPES; ++i) {
                if (p) {
                    xprintf("%s%s%s, ", (n ? n : ""), (n ? " : " : ""), p);
                }
                n = funct->f_names[i];
                p = arg_type(funct->f_types[i]);
            }

            if (p) {
                xprintf("%s%s%s", (n ? n : ""), (n ? " : " : ""), p);
            }
            xprintf(")\n");
        }
    }
}


static void
funct_pop(void)
{
    funct_t *funct;

    assert(funct_level >= 0);
    assert(funct_level < (int)FUNCT_LEVELS);
    assert(funct_data[funct_level]);

    funct = funct_data[funct_level];

    if (0 == funct->f_push--) {
        chk_free(funct_data[funct_level]);
        funct_data[funct_level] = NULL;
        --funct_level;
    }
}
/*end*/
