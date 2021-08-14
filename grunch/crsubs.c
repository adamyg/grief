#include <edidentifier.h>
__CIDENT_RCSID(gr_crsubs_c,"$Id: crsubs.c,v 1.29 2021/08/14 17:09:30 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crsubs.c,v 1.29 2021/08/14 17:09:30 cvsuser Exp $
 * Parser ultities.
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

Head_p              hd_case = NULL;             /* List of case's for switch stmt */
Head_p              hd_switch = NULL;           /* List of pending hd_case's for nested switches */
Head_p              hd_stmt = NULL;             /* List of trees representing code for current function*/
Head_p              hd_init = NULL;             /* Pending initialisation statements to be executed
                                                 * after a variable is declared */
Head_p              hd_arglist = NULL;          /* Current argument list */
Head_p              hd_globals = NULL;          /* List of global declarations needed so we
                                                 * can put initialisation code into _init(main) */
Head_p              hd_syms = NULL;             /* Linked list of symbol table declarations
                                                 * Ordered so that we can make symbols come and
                                                 * go as we enter block declarations */
Head_p              hd_decls = NULL;            /* Things declared at innermost level at front of list */

node_t *            hd_nodes = NULL;            /* List of node_t's allocated so we can free them after compilation */

node_t *            x_maintree = NULL;           /* Tree for function main(). Needed so we can
                                                 * delay generating code for main() until we've
                                                 * parsed all declarations */

Head_p              hd_id = NULL;               /* Stack of identifiers seen but not yet finished processing yet */

func_t *            current_func = NULL;        /* Current function */

int                 x_decl_level = 0;           /* Declaration level */

int                 x_block_level = 0;          /* Current block nesting level */

int                 x_break_level = 0;          /* Keep track of whether break is valid */

int                 x_continue_level = 0;       /* Keep track of whether continue is valid */

int                 x_switch_level = 0;         /* Used to detect 'case' statements outside of switches */

int                 x_struct_level = 0;         /* Struct definition nesting level */

static symbol_t *   x_struct_sp;                /* Pointer to symbol table entry for current
                                                 * structure. Used so that we can associate the structure name
                                                 * with a variable of type struct/union.
                                                 */

typedef enum {
    ENUM_UNKNOWN,
    ENUM_INTEGER,
    ENUM_STRING
} enumtype_t;

static enumtype_t   x_enum_type = ENUM_UNKNOWN;
static symbol_t *   x_enum_sp = NULL;
static accint_t     x_enum_value = 0;

static node_t *     stringmultiple(node_t *snode, int count);
static int          case_typecompare(symtype_t ty1, symtype_t ty2);


/*
 *  Initialise variables which are needed by the grunch parser.
 *
 *  May be called multiple times, so only re-initialise data structures if they are NULL
 */
void
parser_init(void)
{
    init_lex();
    symtab_init();
    init_codegen();

    x_decl_level = 0;
    x_block_level = 0;
    x_break_level = 0;
    x_continue_level = 0;
    x_switch_level = 0;
    x_maintree = NULL;
    x_struct_level = 0;
    x_struct_sp = NULL;
    x_enum_type = ENUM_UNKNOWN;
    x_enum_sp = NULL;
    x_enum_value = 0;

    hd_globals = ll_init();
}


void
parser_close(void)
{
    nodes_free();
    list_free(&hd_arglist);
    list_free(&hd_undef);
    list_free(&hd_block);
    list_free(&hd_init);
    list_free(&hd_stmt);
    list_free(&hd_case);
    list_free(&hd_decls);
    list_free(&hd_id);
    list_free(&hd_struct);
    list_free(&hd_switch);
    list_free(&hd_globals);
    symtab_close();
}


node_t *
new_node(void)
{
    static node_t null_node = {0};
    node_t *np = (node_t *) chk_alloc(sizeof(node_t));

    ++x_stats.s_new_nodes;

    *np = null_node;
    D_NODE_MAGIC(np->magic = NODE_MAGIC;)
    np->lineno = x_lineno;

    np->next = hd_nodes;
    hd_nodes = np;
    return np;
}


/*
 *  Routine to free all nodes allocated during the last compilation. We do this in-bulk
 *  because it makes the code so much simpler rather than freeing as we go along.
 */
void
nodes_free(void)
{
    register node_t *np;

    while (hd_nodes) {
        np = hd_nodes;
        hd_nodes = hd_nodes->next;
        np->next = NULL;

        /*
         *  release node specific storage
         */
        if (node_symbol == np->type) {
            assert(np->left == NULL);           /* XXX - checks, 'sym' should be made a leaf */
            if (np->sym) {
                sym_free(np->sym);
            }
            chk_free((void *) np->atom.sval);

        } else if (node_string == np->type) {
            chk_free((void *) np->atom.sval);

        } else if (node_arglist == np->type) {
            if (np->atom.arglist) {
                list_free(&np->atom.arglist);
            }
        }

        /*
         *  release node
         */
        D_NODE_MAGIC(assert(np->magic == NODE_MAGIC);)
        D_NODE_MAGIC(np->magic = 0xa5a5a5a5;)
        chk_free((void *) np);
        --x_stats.s_new_nodes;
    }
}


/*
 *  Routine to free all elements in a list -- just freeing the list atoms.
 *  We leave the freeing of the contents of the atom to some other code.
 */
void
list_push(Head_p *hd, node_t *stmt)
{
    if (NULL == *hd)
        *hd = ll_init();
    assert(NULL != stmt);
    ll_push(*hd, (char *) stmt);
}


void
list_append(Head_p *hd, node_t *stmt)
{
    if (NULL == *hd)
        *hd = ll_init();
    assert(NULL != stmt);
    ll_append(*hd, (char *) stmt);
}


void
list_free(Head_p *hdp)
{
    register List_p lp;

    if (NULL == *hdp)
        return;
    while ((lp = ll_first(*hdp)) != 0) {
        ll_delete(lp);
    }
    ll_free(*hdp);
    *hdp = NULL;
}


/*
 *  Allocate a node containing a symbol type and copy the string containing the symbol name.
 */
node_t *
new_symbol(const char *name)
{
    node_t *np = new_node();

    np->type = node_symbol;
    np->atom.sval = chk_salloc(name);
    return np;
}


/*
 *  Allocate a node containing a symbol type and copy the string containing the symbol
 *  name, same as new_symbol() above but we already have a buffer allocated to the
 *  symbol name.
 */
node_t *
new_symbol1(char *str)
{
    node_t *np = new_node();

    np->type = node_symbol;
    np->atom.sval = str;
    return np;
}


/*
 *  Create an atom in the tree containing an string containing
 *  the string passed as argument.
 */
node_t *
new_string(char *str)
{
    node_t *np = new_node();

    np->type = node_string;
    np->atom.sval = str;
    return np;
}


/*
 *  Create an atom in the tree containing an integer containing
 *  the number passed as argument.
 */
node_t *
new_number(accint_t ival)
{
    node_t *np = new_node();

    np->type = node_integer;
    np->atom.ival = ival;
    return np;
}


/*
 *  Create an atom containing a floating point constant.
 */
node_t *
new_float(accfloat_t fval)
{
    node_t *np = new_node();

    np->type = node_float;
    np->atom.fval = fval;
    return np;
}


node_t *
node_alloc(int op)
{
    node_t *np = new_node();

    np->type = node_keywd;
    np->atom.ival = op;
    return np;
}


node_t *
new_type(symtype_t type, node_t *left, node_t *right)
{
    node_t *np = new_node();

    np->type = node_type;
    np->atom.ival = (accint_t) type;
    np->left = left;
    np->right = right;
    return np;
}


node_t *
new_arglist(Head_p arglist)
{
    node_t *np = new_node();

    np->type = node_arglist;
    np->atom.arglist = arglist;
    return np;
}


/*
 *  Return a new node for the parse tree with 'a' as the type of
 *  the node and b and c being the left and right pointers.
 */
node_t *
node(int op, node_t *left, node_t *right)
{
    node_t *np = new_node();

    np->type = node_keywd;
    np->atom.ival = op;
    np->left = left;
    np->right = right;
    return np;
}


/*  Function:           node_opt
 *      Function is similar to 'node()' but attempts to do some simple constant optimisations.
 *
 *  Parameters:
 *      op -                Operator.
 *      b -                 Left node.
 *      c -                 Right node.
 *
 *  Returns
 *      Modified version of 'b' or a new operator node.
 */
node_t *
node_opt(int op, node_t *b, node_t *c)
{
    if (NULL == b || NULL == c) {
        return node(op, b, c);
    }

    if (b->type == node_integer && c->type == node_integer) {
        /*
         *  integer <op> integer
         *  
         *  XXX/TODO:  over/underflow
         *  Ref: INT32-C. Ensure that operations on signed integers do not result in overflow
         */
        switch (op) {
        case O_PLUS:
            b->atom.ival += c->atom.ival;
            return b;
        case O_MINUS:
            b->atom.ival -= c->atom.ival;
            return b;
        case O_MUL:
            b->atom.ival *= c->atom.ival;
            return b;
        case O_DIV:
            if (0 == c->atom.ival) {
                crwarn(RC_ERROR, "constant expression divides by zero");
                b->atom.ival = 0;
            } else {
                b->atom.ival /= c->atom.ival;
            }
            return b;
        case O_MOD:
            if (0 == c->atom.ival) {
                crwarn(RC_ERROR, "constant expression modulo by zero");
                b->atom.ival = 0;
            } else {
                b->atom.ival %= c->atom.ival;
            }
            return b;
        case O_OR:
            b->atom.ival |= c->atom.ival;
            return b;
        case O_XOR:
            b->atom.ival ^= c->atom.ival;
            return b;
        case O_AND:
            b->atom.ival &= c->atom.ival;
            return b;
        case O_LSHIFT:
            if (0 == c->atom.ival) {
                crwarn(RC_ERROR, "constant expression shift by zero");
            } else if (c->atom.ival < 0 || c->atom.ival >= (sizeof(accint_t) * 8)) {
                crwarn(RC_ERROR, "constant expression shift overflow");
            }
            b->atom.ival <<= c->atom.ival;
            return b;
        case O_RSHIFT:
            if (0 == c->atom.ival) {
                crwarn(RC_ERROR, "constant expression shift by zero");
            } else if (c->atom.ival < 0 || c->atom.ival >= (sizeof(accint_t) * 8)) {
                crwarn(RC_ERROR, "constant expression shift overflow");
            }
            b->atom.ival >>= c->atom.ival;
            return b;
        default:
            break;
        }

#if (TODO)
    } else if (b->type == node_float && c->type == node_float) {
        /*
         *  float <op> integer
         */
        switch (op) {
        case O_PLUS:
            b->atom.fval += c->atom.fval;
            return b;
        case O_MINUS:
            b->atom.fval -= c->atom.fval;
            return b;
        case O_MUL:
            b->atom.fval *= c->atom.fval;
            return b;
        case O_DIV:
            if (0 == c->atom.fval) {
                crwarn(RC_ERROR, "constant expression divides by zero");
                b->atom.fval = 0;
            } else {
                b->atom.fval /= c->atom.fval;
            }
            return b;
        }
#endif

    } else if (b->type == node_string && c->type == node_string) {
        /*
         *  string <op> string
         */
        switch (op) {
        case O_PLUS: {          /*07/08/08, string concat*/
                const int blen = strlen(b->atom.sval), clen = strlen(c->atom.sval);
                char *snew = chk_alloc(blen + clen + 1);

                strcpy(snew, b->atom.sval);
                strcpy(snew + blen, c->atom.sval);
                chk_free((char *) b->atom.sval);
                b->atom.sval = snew;
            }
            return b;
        default:
            break;
        }

    } else if (b->type == node_string && c->type == node_integer) {
        /*
         *  string <op> integer
         */
        if (xf_grunch) {
            switch (op) {
            case O_PLUS:
                return node(op, b, c);
            case O_MUL:
                return stringmultiple(b, c->atom.ival);
            default:
                break;
            }
        }

        switch (op) {
        case O_PLUS:  case O_MINUS:
        case O_MUL:   case O_DIV:
        case O_MOD:   case O_XOR:
        case O_OR:    case O_AND:
            crerror(RC_ERROR, "invalid string operation");
            break;
        default:
            break;
        }

    } else if (b->type == node_integer && c->type == node_string) {
        /*
         *  integer <op> string
         */
        if (xf_grunch) {
            switch (op) {
            case O_PLUS:
                return node(op, b, c);
            case O_MUL:
                return stringmultiple(c, b->atom.ival);
            default:
                break;
            }
        }

        switch (op) {
        case O_PLUS:  case O_MINUS:
        case O_MUL:   case O_DIV:
        case O_MOD:   case O_XOR:
        case O_OR:    case O_AND:
            crerror(RC_ERROR, "invalid string operation");
            break;
        default:
            break;
        }
    }

    return node(op, b, c);
}


/*  Function:           stringmultiple
 *      Constant string multiple
 *
 *  Parameters:
 *      snode -             String node.
 *      count -             Multiply count.
 *
 *  Returns
 *      Modified string node.
 */
static node_t *
stringmultiple(node_t *snode, int count)
{
    const char *sold = snode->atom.sval;
    const int slen = strlen(sold);
    char *snew;

    assert(node_string == snode->type);

    if (0 == slen || count <= 0) {
        chk_free((char *) sold);
        snode->atom.sval = chk_salloc("");

    } else if (NULL != (snew = chk_alloc((slen * count) + 1))) {
        snode->atom.sval = snew;
        while (count-- > 0) {
            memcpy(snew, sold, slen+1);
            snew += slen;
        }
        chk_free((char *) sold);
    }

    return snode;
}


/*
 *  Create a tree as a result of an assignment operator.
 *
 *  We map the "list[expr] = ..." syntax into a
 *
 *      (put_nth expr list ...) macro.
 */
node_t *
node_lvalue(int op, node_t *l, node_t *r)
{
    node_t *lsymbol = l;
    symbol_t *sp = NULL;

    /* find the associated symbol */
    while (lsymbol) {
        if (node_symbol == lsymbol->type) {
            sp = sym_lookup(lsymbol->atom.sval, 0);

            if (sp && (sp->s_type & (TQ_CONST << TQ_SHIFT))) {
                crerrorx(RC_ERROR, "assignment to read-only variable '%s'", sp->name);
            }
            break;
        }
        lsymbol = lsymbol->left;
    }

    if (l->type == node_keywd) {
        if (O_OSQUARE == l->atom.ival) {
            /*
             *  put_nth
             */
            node_t *np = node_alloc(K_LVALUE);

            if (O_EQ != op) {
                crerrorx(RC_ERROR, "%s operation not supported on array elements", yymap((int) op));

            } else if (sp) {
                const symtype_t ty = sp->s_type & TY_MASK;

                switch (ty) {
                case TY_LIST:
                case TY_ARRAY:
                case TY_DECLARE:
                    break;
                default:
                    crerror(RC_ERROR, "[] operation only supported on lists, arrays or polys");
                    break;
                }
            }

            np->left = l;
            np->right = r;
            return (np);

        } else if (l->atom.ival == K_GETPROPERTY) {
            /*
             *  set_property
             */
            node_t *np = node_alloc(K_SETPROPERTY);

            if (O_EQ != op) {
                crerrorx(RC_ERROR, "%s operation not supported on properties", yymap((int) op));
            }

            if (sp) {
                const symtype_t ty = sp->s_type & TY_MASK;

                switch (ty) {
                case TY_INT:                    /* dictionary identifiers */
                case TY_DECLARE:
                    break;
                default:
                    crerror(RC_ERROR, ". operation only supported on identifiers or polys");
                    break;
                }
            }

            np->left = node(K_NOOP, l->left, l->right);
            np->right = r;
            return (np);
        }
    }

    return node(op, l, r);
}


/*
 *  Called at start of a function with the type of function and the parameter list.
 */
int
function_start(symtype_t type, node_t *defn)
{
    func_t *fp = chk_alloc(sizeof(func_t));

    if (0 == type || (type & TY_MASK) == TY_IMPLICIT) {
        crwarn(RC_ERROR, "return-value defaults to `int'");
        type |= TY_INT;
    }

    fp->f_type = type;
    fp->f_name = ident_peek();
    fp->f_defn = defn;
    fp->f_body = NULL;

    function_arglist(fp->f_name, fp->f_defn, 1);

    x_funcname = fp->f_name;
    current_func = fp;
    return 0;
}


func_t *
function_end(node_t *body)
{
    func_t *fp = current_func;

    assert(NULL != current_func);
    fp->f_body = body;
    current_func = NULL;
    return fp;
}


int
function_arglist(const char *name, node_t *defn, int body)
{
    int argc = 0;

    assert(NULL == defn || defn->type == node_keywd);
    assert(NULL == defn || defn->atom.ival == TO_FUNC);

    xprintf("function %s(", name);

    if (defn && defn->right) {
        int is_optional = 0, is_void = 0;
        int seen_dots = FALSE;                  /* variable argument */
        int seen_optional = FALSE;
        Head_p arglist = defn->right->atom.arglist;
        List_p lp;

        assert(node_arglist == defn->right->type);

        for (lp = ll_first(arglist); lp;) {
            node_t *t_np = (node_t *) ll_elem(lp);

            if (seen_dots) {
                crerror(RC_ERROR, "declarations after an ellipsis detected.");
            }

            if (NULL == t_np) {
                seen_dots = TRUE;               /* ... */

            } else {
                assert(node_type == t_np->type);

                is_void = (TY_VOID == (t_np->atom.ival & TY_MASK));
                is_optional = ((TM_OPTIONAL << TM_SHIFT) == (t_np->atom.ival & TM_MASK));

                if (is_optional) {
                    seen_optional = TRUE;       /* ~ type */
                }

                node_dprint(t_np, 0);

                if (t_np->right) {
                    if (is_void) {
                        crerrorx(RC_ERROR, "'%s' : illegal use of void type", t_np->right->atom.sval);
                    }

                } else {
                    if (is_void) {
                        if (argc) {
                            crerror(RC_ERROR, "'void' cannot be an argument type, except for '(void)'");
                        }

                    } else if (! is_optional && body) {
                        if (seen_optional) {
                            crerrorx(RC_ERROR, "'%s' : non-optional parameter missing declaration", name);

                        } else {
                            crerrorx(RC_ERROR, "'%s' : expected formal parameter list, not a type list", name);
                        }
                        body = 0;
                    }
                }

                if (TY_IMPLICIT == (t_np->atom.ival & TY_MASK)) {
                    /*
                    //  A function prototype has formal parameter names but no types for the
                    //  parameters. Each formal parameter must have a type and a trailing
                    //  ellipsis (...) to indicate a variable number of parameters.
                    */
                    crwarnx(RC_ERROR, "'%s' : prototype must have parameter types", name);
                    t_np->atom.ival &= ~TY_MASK;
                    t_np->atom.ival |= TY_INT;
                }
            }

            lp = ll_next(lp);

            if (lp) {
                xprintf(",");
            }

            ++argc;
        }

        if (1 == argc && is_void) {             /* void function */
            argc = 0;
        } else {
            if (strcmp(name, "main") == 0) {
                crwarn(RC_ERROR, "arguments to 'main' are ignored.");
            }
        }
    }

    xprintf(")\n");

    return argc;
}


/*
 *  Simple function to check the return type of a function against the return(expr)' or
 *  'return;' statement. We only handle void vs. non-void
 */
void
function_return(int expr)
{
    int return_type = (current_func ? current_func->f_type & TY_MASK : TY_VOID);

    if (expr && TY_VOID == return_type) {
        crwarn(RC_ERROR, "return with expression in function returning void");

    } else if (!expr && TY_VOID != return_type) {
        crwarn(RC_ERROR, "return with no value");
    }
}


void
decl_push(symtype_t type)
{
    decl_t *dp = (decl_t *) chk_alloc(sizeof(decl_t));

    xprintf("decl push (%u/%s)\n", type, symtype_to_defn(type));

    dp->d_type = type;
    dp->d_struct_sp = NULL;
    dp->d_symbol = NULL;

    switch (type & TY_MASK) {
    case TY_STRUCTI:
    case TY_UNIONI:
        assert(x_struct_sp);
        dp->d_symbol = sym_reference(x_struct_sp);
        break;

    case TY_ENUMI:
        assert(x_enum_sp);
        dp->d_symbol = sym_reference(x_enum_sp);
        break;
    }

    if (NULL == hd_decls)
        hd_decls = ll_init();
    ll_push(hd_decls, (char *) dp);
    ++x_decl_level;
}



symtype_t
decl_peek(void)
{
    List_p lp = ll_first(hd_decls);
    decl_t *dp;

    assert(lp);
    dp = ll_elem(lp);
    return dp->d_type;
}


symbol_t *
decl_add(node_t *np, symtype_t type)
{
    const char *name = ident_pop();
    decl_t *dp = (decl_t *) ll_elem(ll_first(hd_decls));
    symbol_t *sp;

    type |= dp->d_type;
    sp = sym_add(name, np, type);
    if (sp) {
        symbol_t *symsp = dp->d_symbol;         /* 26/05/09 - struct/union/enum */

        assert((sp->s_type & TY_MASK) == (type & TY_MASK));
        switch(type & TY_MASK) {
        case TY_STRUCTI:
        case TY_UNIONI:
            assert(symsp);
            assert((TY_STRUCTI == type && TY_STRUCT == symsp->s_type) || \
                    (TY_UNIONI == type && TY_UNION == symsp->s_type));
            xprintf("%s isa %s %s\n", name,
                (TY_STRUCTI == type ? "struct" : "union"), symsp->name);
            sp->s_defn = sym_reference(symsp);
            break;

        case TY_ENUMI:
            assert(symsp);
            assert(TY_ENUM == symsp->s_type);
            xprintf("%s isa enum %s\n", name, symsp->name);
            sp->s_defn = sym_reference(symsp);
            break;

        default:
            assert(0 == symsp);
            break;
        }
    }
    chk_free((void *) name);
    return sp;
}


void
decl_pop(void)
{
    List_p lp = ll_first(hd_decls);
    decl_t *dp;

    assert(x_decl_level > 0);
    --x_decl_level;

    assert(lp);
    dp = (decl_t *) ll_elem(lp);

    xprintf("decl pop (%u/%s)\n", dp->d_type, symtype_to_defn(dp->d_type));

    if (dp->d_symbol) {
        sym_free(dp->d_symbol);
    }

    chk_free(dp);
    ll_delete(lp);
}


/*  Function:       node_type
 *      Return the underlying type of node as one the TY_xxxx enumerations.
 *
 *  Parameters:
 *      np -            Node pointer.
 *
 *  Return:
 *      Node base type
 */
symtype_t
node_typeof(const node_t *np)
{
    symtype_t type = (symtype_t)TY_UNDEF;

    assert(np);
    switch (np->type) {
    case node_keywd:
        if (K_FUNCALL == np->atom.ival) {
            if (np->left) {
                type = sym_typeof(np->left->atom.sval, SYMLK_FUNDEF|SYMLK_FBUILTIN);
            }
        }
        break;

    case node_integer:
        type = TY_INT;
        break;

    case node_string:
        type = TY_STRING;
        break;

    case node_float:
        type = TY_FLOAT;
        break;

    case node_symbol:
        type = sym_typeof(np->atom.sval, 0);
        break;

    case node_type:
        type = (symtype_t) np->atom.ival;
        break;

    case node_arglist:
        assert(node_arglist != np->type);
        break;

    default:
        assert(0);
        break;
    }

    xprintf("\tnode_typeof(%p)-> %d (%s)\n", np, type, symtype_to_str(type));
    return type;
}


symbol_t *
struct_start(char *name, symtype_t type)
{
    struct_t *stp = (struct_t *) chk_alloc(sizeof(struct struct_t));
    symbol_t *sp;

    xprintf("struct enter (%s, %u/%s)\n", name, type, symtype_to_defn(type));

    if (NULL == hd_struct)
        hd_struct = ll_init();
    sp = sym_add(name, (node_t *) NULL, type);
    sp->s_flags &= ~SF_FORWARD;                 /* definition */
    sp->s_flags |= SF_DEFINING;

    x_struct_sp = stp->s_symbol = sp;
    stp->s_level = ++x_struct_level;

    ll_push(hd_struct, (char *) stp);
    return sp;
}


symbol_t *
struct_end(void)
{
    struct_t *stp = (struct_t *) ll_elem(ll_first(hd_struct));
    Head_p hd = NULL;
    List_p lp, lp1;
    symbol_t *sp;

    assert(x_struct_level > 0);
    --x_struct_level;
    assert(stp);

    sp = stp->s_symbol;
    ll_pop(hd_struct);

    xprintf("struct end (%s)\n", sp->name);

    sp->s_flags &= ~SF_DEFINING;

    /*
     *  Now move all incomplete structure definitions to the beginning of the symbol
     *  table. (This means that when we dump the symbol table, we calculate the size of
     *  structures which are defined inside of other structures and thus get the
     *  structure offsets correct.
     */
    for (lp = ll_first(hd_syms); lp; lp = lp1) {
        symbol_t *s = (symbol_t *) ll_elem(lp);
        const symtype_t ty = s->s_type & TY_MASK;

        lp1 = ll_next(lp);
        if (NULL == s->s_owner &&
                (TY_STRUCT == ty || TY_UNION == ty) && (s->s_flags & SF_DEFINING)) {
            if (NULL == hd)
                hd = ll_init();
            ll_append(hd, (char *) s);
            ll_delete(lp);
        }
    }

    if (hd) {
        while (NULL != (lp = ll_first(hd))) {
            symbol_t *s = (symbol_t *) ll_elem(lp);

            ll_push(hd_syms, (char *) s);
            ll_delete(lp);
        }
        ll_free(hd);
    }

    return sp;
}


void
struct_member(node_t *np)
{
    const char *name = ident_pop();
    struct_t *stp = (struct_t *) ll_elem(ll_first(hd_struct));
    decl_t *dp = (decl_t *) ll_elem(ll_first(hd_decls));
    symtype_t type = dp->d_type;
    symbol_t *sp;

    assert(stp->s_symbol);
    x_struct_sp = stp->s_symbol;                /* reload struct maybe have nested */
    xprintf("memberof %s ", x_struct_sp->name);

    sp = sym_push(&stp->s_symbol->s_members, INSERT_AT_END, name, np, type, x_struct_sp);
    if (sp) {
        symbol_t *symsp = dp->d_symbol;

        assert(sp->s_type == type);
        switch(type & TY_MASK) {
        case TY_STRUCTI:
        case TY_UNIONI:
            assert(symsp);
            assert((TY_STRUCTI == type && TY_STRUCT == symsp->s_type) || \
                    (TY_UNIONI == type && TY_UNION == symsp->s_type));
            xprintf("%s isa %s %s\n", name,
                (TY_STRUCTI == type ? "struct" : "union"), symsp->name);
            sp->s_defn = sym_reference(symsp);
            break;

        case TY_ENUMI:
            assert(symsp);
            assert(TY_ENUM == symsp->s_type);
            xprintf("%s isa enum %s\n", name, symsp->name);
            sp->s_defn = sym_reference(symsp);
            break;

        default:
            assert(0 == symsp);
            break;
        }
    }
    chk_free((void *) name);
}


/*  Function:       struct_tag
 *      Lookup the specified struct tag and create a forward reference if required. The
 *      resulting symbol becomes the current struct tag.
 *
 *  Paremeters:
 *      name -          Struct/union name.
 *      type -          TY_STRUCT or TY_UNION.
 *
 *  Returns:
 *      none
 */
void
struct_tag(const char *name, symtype_t type)
{
    symbol_t *sp;

    assert(TY_STRUCT == type || TY_UNION == type);
    if (NULL == (sp = sym_lookup(name, SYMLK_FUNDEF|SYMLK_FTAG))) {
        /*
         *  add forward reference
         */
        sp = sym_add(name, (node_t *) NULL, type);
        sp->s_flags |= SF_FORWARD;
    }
    x_struct_sp = sp;
}


void
enum_enter(char *name)
{
    symbol_t *sp;

    xprintf("enum enter(%s)\n", name);
    x_enum_type = ENUM_UNKNOWN;
    x_enum_value = 0;
    sp = sym_add(name, (node_t *) NULL, TY_ENUM);
    sp->s_flags &= ~SF_FORWARD;
    sp->s_flags |= SF_DEFINING;
    x_enum_sp = sp;
}


int
enum_ivalue(accint_t ivalue)
{
    int ret = TRUE;

    if (ENUM_STRING == x_enum_type) {
        ret = FALSE;
    } else {
        x_enum_type = ENUM_INTEGER;
        x_enum_value = ivalue + 1;
    }
    xprintf("enum ivalue(%ld) : %s\n", ivalue, (ret ? "TRUE" : "FALSE"));
    return ret;
}


int
enum_svalue(const char *svalue)
{
    int ret = TRUE;

    if (ENUM_INTEGER == x_enum_type) {
        ret = FALSE;
    } else {
        x_enum_type = ENUM_STRING;
    }
    xprintf("enum svalue(%s) : %s\n", svalue, (ret ? "TRUE" : "FALSE"));
    return ret;
}


int
enum_implicited(accint_t *ivalue)
{
    int ret = TRUE;

    if (ENUM_STRING == x_enum_type) {
        ret = FALSE;
    } else {
        x_enum_type = ENUM_INTEGER;
        *ivalue = x_enum_value++;
    }
    xprintf("enum implicited(%ld) : %s\n", *ivalue, (ret ? "TRUE" : "FALSE"));
    return ret;
}


void
enum_add(node_t *np)
{
    const char *name = np->left->atom.sval;
    node_t *right = np->right;

    assert(node_keywd == np->type);
    assert(K_ENUMVAL == np->atom.ival);
    assert(right && (node_integer == right->type || node_string == right->type));

    if (node_integer == right->type) {
        xprintf("enum name(%s) = %ld\n", name, right->atom.ival);

    } else {
        xprintf("enum name(%s) = %s\n", name, right->atom.sval);
    }
    sym_add(name, np, TY_ENUMCONST);
}


void
enum_exit(node_t *__CUNUSEDARGUMENT(enumvalues))
{
    symbol_t *sp = x_enum_sp;

    sp->s_flags &= ~SF_DEFINING;
    sp->s_enumtype = (ENUM_STRING == x_enum_type ? TY_STRING : TY_INT);
    x_enum_type = ENUM_UNKNOWN;
    xprintf("enum exit(%s)\n", sp->name);
}


/*  Function:       enum_type
 *      Return the underlying enumeration type of either TY_INT or TY_STRING. Upon use
 *      of an incomplete enumeration on the first usage an exception is thrown and
 *      TY_INT shall be returned for all references.
 *
 *  Parameters:
 *      sp -            Symbol definition.
 *
 *  Returns:
 *      TY_INT or TY_STRING.
 */
symtype_t
enum_type(symbol_t *sp)
{
    const symtype_t type = sp->s_type & TY_LO_MASK;
    symbol_t *symsp;

    assert(TY_ENUM != type);
    assert(TY_ENUMI == type);

    if (NULL != (symsp = sp->s_defn)) {
        sym_check(symsp);
        if ((symsp->s_flags & SF_FORWARD) == 0 && symsp->s_enumtype) {
            /*
             *  TY_INT or TY_STRING
             */
            const symtype_t enumtype = symsp->s_enumtype;

            assert(TY_INT == enumtype || TY_STRING == enumtype);
            return enumtype;
        }
    }

    if ((sp->s_flags & SF_INCOMPLETE) == 0) {
        /*
         *  Incomplete, throw an error and mark as such.
         */
        crerrorx(RC_ERROR, "'%s' size unknown, enum is incomplete", sp->name);
        sp->s_flags |= SF_INCOMPLETE;
        sp->s_enumtype = TY_INT;
    }
    return TY_INT;
}


/*  Function:       enum_tag
 *      Lookup the specified enum tag and create a forward reference if required. The
 *      resulting symbol becomes the current enum tag.
 *
 *  Paremeters:
 *      name -          Enumeration name.
 *
 *  Returns:
 *      none
 */
void
enum_tag(const char *name)
{
    symbol_t *sp;

    if (NULL == (sp = sym_lookup(name, SYMLK_FUNDEF|SYMLK_FTAG))) {
        /*
         *  add forward reference
         */
        sp = sym_add(name, (node_t *) NULL, TY_ENUM);
        sp->s_flags |= SF_FORWARD;
    }
    x_enum_sp = sp;
}


/*  Function:       ident_push
 *      Push an indentifier on the symbol stack.
 *
 *  Paremeters:
 *      ident -         Identifier
 *
 *  Returns:
 *      none
 */
void
ident_push(char *ident)
{
    if (NULL == hd_id)
        hd_id = ll_init();
    assert(NULL != ident);
    ll_push(hd_id, ident);
}


/*  Function:       ident_peek
 *      Peek at the current identifier stack.
 *
 *  Paremeters:
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
ident_peek(void)
{
    List_p lp = ll_first(hd_id);

    assert(lp);
    return (char *) ll_elem(lp);
}


/*  Function:       ident_peek2
 *      Optionally peek at the current identifier stack.
 *
 *  Paremeters:
 *      none
 *
 *  Returns:
 *      nothing
 */
const char *
ident_peek2(void)
{
    List_p lp = ll_first(hd_id);

    if (NULL != lp) {
        return (const char *) ll_elem(lp);
    }
    return NULL;
}


/*  Function:       ident_pop
 *      Pop the identifier from the stack.
 *
 *  Paremeters:
 *      none
 *
 *  Returns:
 *      Identifier buffer.
 */
char *
ident_pop(void)
{
    List_p lp = ll_first(hd_id);
    char *str;

    assert(lp);
    str = (char *) ll_elem(lp);
    ll_delete(lp);
    return str;
}


/*  Function:       switch_start
 *      Open a new switch block.
 *
 *  Parameters:
 *      np -            Expression node.
 *
 *  Returns:
 *      nothing
 */
void
switch_start(node_t *np)
{
    switch_t *sw = (switch_t *) chk_alloc(sizeof(switch_t));

    xprintf("switch_start(%p)->", np);
    node_dprint(np, 0);
    xprintf("\n");

    ++x_switch_level;
    ++x_break_level;
    if (NULL == hd_switch) {
        hd_switch = ll_init();
    }

    sw->sw_expr = np;
    sw->sw_type = node_typeof(np);
    sw->sw_mixed = 0;

    if (! xf_grunch) {
        if (TY_INT != (sw->sw_type & TY_MASK)) {
            crerror(RC_ERROR, "switch quantity not an integer");
        }
    } else {
        switch (sw->sw_type & TY_MASK) {
        case TY_VOID:
            crerror(RC_ERROR, "switch quantity is void");
            sw->sw_type = (symtype_t)TY_UNDEF;
            break;

        case TY_STRUCT: case TY_STRUCTI:
        case TY_UNION:  case TY_UNIONI:
        case TY_ENUM:   case TY_ENUMI:
        case TY_LIST:
        case TY_ARRAY:
            crerror(RC_ERROR, "switch quantity is either a numeric nor string type");
            sw->sw_type = (symtype_t)TY_UNDEF;
            break;

        default:
            break;
        }
    }
    sw->sw_stmts = hd_stmt;
    sw->sw_cases = hd_case;
    ll_push(hd_switch, (char *) sw);
    hd_case = NULL;
    hd_stmt = NULL;
}


/*  Function:       switch_end
 *      Close the current switch block.
 *
 *  Parameters:
 *      np -            Expression node
 *
 *  Returns:
 *      Switch expresion tree.
 */
node_t *
switch_end(node_t *np)
{
    node_t *case_tree = NULL;
    Head_p hd = ll_init();
    switch_t *sw;
    List_p lp;

    assert(x_switch_level >= 1);
    case_end();
    --x_switch_level;
    --x_break_level;

    /*
     *  Reverse order of cases,
     *      TODO - check for missing enumeration values
     */
    if (hd_case) {  /*non-empty switch*/
        while (NULL != (lp = ll_first(hd_case))) {
            ll_push(hd, ll_elem(lp));
            ll_delete(lp);
        }
    }

    while (NULL != (lp = ll_first(hd))) {
        case_tree = node(K_NOOP, case_tree, (node_t *) ll_elem(lp));
        ll_delete(lp);
    }

    sw = (switch_t *) ll_elem(ll_first(hd_switch));
    assert(sw->sw_expr == np);
    hd_case = sw->sw_cases;
    hd_stmt = sw->sw_stmts;
    ll_pop(hd_switch);
    chk_free(sw);

    return node(K_SWITCH, np, case_tree);
}


/*  Function:       case_start
 *      Open a new case block.
 *
 *  Parameters:
 *      np -            Case expression, NULL if default.
 *
 *  Returns:
 *      0           No closing case.
 *      1           Case without code (is fall thru).
 *      2           Code yet without break/return.
 *      3           Code with complete block (ie. trailing break/return).
 */
void
case_start(node_t *np)
{
    switch_t *sw = (switch_t *) ll_elem(ll_first(hd_switch));
    int typecheck = 0;

    assert(sw);
    xprintf("case_start(%p)->", np);
    node_dprint(np, 0);
    xprintf("\n");

    /*
     *  End any previous dangling case
     */
    if (2 == case_end()) {
        crwarn(RC_ERROR, "case statement falling thru, may not behave as expected.");
    }

    /*
     *  Checks (07/04/10)
     *
     *      Non-constant expressions
     *      Multiple defaults
     *      Duplicate values
     *      Mixed types
     */
    if (np) {
        const symtype_t case_type = node_typeof(np);

        /*
         *  base type checks
         */
        if (! xf_grunch) {
            if (TY_INT != symtype_promote_int(case_type, FALSE)) {
                crerror(RC_ERROR, "case label does not reduce to an integer constant");
            }

        } else {
            if (TY_UNDEF != case_type && TY_DECLARE != case_type) {
                switch (case_type) {
                case TY_VOID:
                    crerror(RC_ERROR, "case value is void");
                    ++typecheck;
                    break;

                case TY_STRUCT: case TY_STRUCTI:
                case TY_UNION:  case TY_UNIONI:
                case TY_ENUM:   case TY_ENUMI:
                case TY_LIST:
                case TY_ARRAY:
                    crerror(RC_ERROR, "case value is neither a numeric nor string type");
                    ++typecheck;
                    break;

                default: {                      /* INT or STRING assumed */
                        const symtype_t sw_type = sw->sw_type;

                        if (TY_UNDEF != sw_type && TY_DECLARE != sw_type)
                            if (! case_typecompare(sw_type, case_type)) {
                                crwarn(RC_ERROR, "incompatible case value type");
                                ++typecheck;
                            }
                    }
                    break;
                }
            }

            if (0 == typecheck)
                switch (np->type) {
                case node_string:
                case node_integer:
                case node_float:
                    break;

                case node_symbol: {             /* check for 'const' symbols */
                        symbol_t *sp;

                        if (NULL != (sp = sym_lookup(np->atom.sval, 0))) {
                            const symtype_t qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;

                            if (qu & TQ_CONST) {
                                break;          /* only const, maybe ignore functions */
                            }
                        }
                    }
                    goto nonconstant;

                case node_keywd:
                    if (K_FUNCALL == np->atom.ival) {
                        if (np->left)           /* special case, others?? */
                            if (0 == strcmp("key_to_int", np->left->atom.sval)) {
                                ++typecheck;
                                break;
                            }
                    }
                    /*FALLTHRU*/

                default:
nonconstant:;       crwarn(RC_ERROR, "non constant case value.");
                    ++typecheck;
                    break;
                }
        }
    }

    if (hd_case) {
        List_p lp;

        xprintf("\tcase walk\n");
        for (lp = ll_first(hd_case); lp; lp = ll_next(lp)) {
            const node_t *t_np = (const node_t *) ll_elem(lp);

            xprintf("\t==> node(%p)->%p/%p\n", t_np, t_np->left, t_np->right);
            assert(node_keywd == t_np->type && K_CASE == t_np->atom.ival);

            if (NULL == t_np->left) {
                /*
                 *  default's
                 */
                if (! np) {
                    crerror(RC_ERROR, "multiple default labels within switch.");
                }

            } else if (np && NULL != (t_np = t_np->left)) {
                /*
                 *  case value's
                 */
                if (np->type == t_np->type) {
                    switch (np->type) {
                    case node_string:
                        if (0 == strcmp(np->atom.sval, t_np->atom.sval)) {
                            crerrorx(RC_ERROR, "duplicate case value '%s'.", np->atom.sval);
                        }
                        break;
                    case node_integer:
                        if (np->atom.ival == t_np->atom.ival) {
                            crerrorx(RC_ERROR, "duplicate case value '%" ACCINT_FMT "'.", np->atom.ival);
                        }
                        break;
                    case node_float:
                        if (np->atom.fval == t_np->atom.fval) {
                            crerrorx(RC_ERROR, "duplicate case value '%" ACCFLOAT_FMT "'.", np->atom.fval);
                        }
                        break;
                    }

                } else {
                    switch (t_np->type) {
                    case node_string:
                    case node_integer:
                    case node_float:
                        if (0 == typecheck && 0 == sw->sw_mixed++) {
                            crwarn(RC_ERROR, "mixed case value types");
                        }
                        break;
                    }
                }
            }
        }
    }

    /*
     *  push new node
     */
    if (NULL == hd_case) {
        hd_case = ll_init();
    }
    ll_push(hd_case, (void *) np);
}


static int
case_typecompare(symtype_t sw_type, symtype_t case_type)
{
    /*
     *  absolute match
     */
    if ((sw_type & TY_MASK) == (case_type & TY_MASK)) {
        return TRUE;
    }

    /*
     *  promote types
     */
    if (symtype_promote_int(sw_type, FALSE) == symtype_promote_int(case_type, FALSE) ||
            symtype_promote_long(sw_type, FALSE) == symtype_promote_long(case_type, FALSE) ||
            symtype_promote_double(sw_type, FALSE) == symtype_promote_double(case_type, FALSE)) {
        return TRUE;
    }

    return FALSE;
}


/*  Function:       case_end
 *      Close the current case statement
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      0           No closing case.
 *      1           Case without code (is fall thru).
 *      2           Code yet without break/return.
 *      3           Code with complete block (ie. trailing break/return).
 */
int
case_end(void)
{
    int rc = 0;

    xprintf("case_end()\n");

    if (hd_case) {
        node_t *np;

        rc = 1;
        if (hd_stmt && ll_last(hd_stmt)) {      /* 07/04/10, enabled */
            np = (node_t *) ll_elem(ll_last(hd_stmt));

            xprintf("\tlast node(%p)->", np);
            node_dprint(np, 0);
            xprintf("\n");

            rc = 2;                             /* fallthru without break/return/throw */
            if (np) {
                if (node_keywd == np->type &&
                        (K_BREAKSW == np->atom.ival || K_RETURN == np->atom.ival || K_THROW == np->atom.ival)) {
                    xprintf("\tis BREAK/RETURN/THROW\n");
                    rc = 3;
                }
            }
        }

        np = (node_t *) ll_elem(ll_first(hd_case));
        ll_pop(hd_case);

        np = node(K_CASE, np, (node_t *) hd_stmt);
        ll_push(hd_case, (void *) np);
        hd_stmt = NULL;
    }
    return rc;
}


/*  Function:       loop_enter
 *      Enter a loop construct (WHILE, FOR etc).
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
loop_enter(void)
{
    ++x_break_level;
    ++x_continue_level;
    loop_or_switch_enter(INSIDE_LOOP);
    block_enter();
}


/*  Function:       loop_exit
 *      Exit a loop construct
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
loop_exit(void)
{
    assert(x_break_level >= 0);
    assert(x_continue_level >= 0);
    --x_break_level;
    --x_continue_level;
    loop_or_switch_exit();
    block_pop();
}

/*end*/
