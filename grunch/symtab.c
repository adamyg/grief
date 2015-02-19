#include <edidentifier.h>
__CIDENT_RCSID(gr_symtab_c,"$Id: symtab.c,v 1.28 2014/10/22 02:33:30 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: symtab.c,v 1.28 2014/10/22 02:33:30 ayoung Exp $
 * Symbol table.
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

#define issign(x) \
    (TY_SIGNED == (x) || TY_UNSIGNED == (x))

#if !defined(SIZEOF_INT)
#define SIZEOF_INT      sizeof(int)
#define SIZEOF_LONG     sizeof(long)
#endif

static int              sym_size(symbol_t *);
static int              sym_alignment(symbol_t *);
static int              symtype_restricted(symtype_t ty1, symtype_t ty2);

Head_p                  hd_block;               /* List containing chains of statements in a block. */
Head_p                  hd_undef;               /* List of symbols which have been reported as being
                                                 * undefined so we dont keep printing same error message.
                                                 */
Head_p                  hd_struct;              /* Stack of structures which we are defining */

static symqueue_t       x_symqueue;             /* List of current symbols */

/*
 *  Structure giving the size of a basic type and the storage alignments.
 *
 *  XXX - should be made host specific, yet for brief/grief usage these dont matter.
 */
static const struct ty_info {
    const char *        ty_desc;
    int                 ty_size;
    int                 ty_align;
} ty_info[] = {         /* see enum _Ttypes */
        { "<implicit>",     0, 0 },             /* TY_IMPLICIT      */
        { "int",            4, 4 },             /* TY_INT           */
        { "char",           1, 1 },             /* TY_CHAR          */
        { "short",          2, 2 },             /* TY_SHORT         */
        { "void",           4, 4 },             /* TY_VOID          */
        { "long",           4, 4 },             /* TY_LONG          */
        { "float",          8, 4 },             /* TY_FLOAT         */
        { "double",         8, 4 },             /* TY_DOUBLE        */

        { "signed",         4, 4 },             /* TY_SIGNED        */
        { "unsigned",       4, 4 },             /* TY_UNSIGNED      */

        { "boolean",        4, 4 },             /* TY_BOOLEAN       */
        { "long char",      4, 4 },             /* TY_LONGCHAR      */
        { "long long",      8, 4 },             /* TY_LONGLONG      */
        { "long double",    8, 8 },             /* TY_LONGDOUBLE    */

        { "struct",         4, 4 },             /* TY_STRUCT        */
        { "union",          4, 4 },             /* TY_UNION         */
        { "enum",           4, 4 },             /* TY_ENUM          */
        { "enum const",     0, 4 },             /* TY_ENUMCONST     - (psuedo type) */

        { "string",         4, 4 },             /* TY_STRING        */
        { "declare",        4, 4 },             /* TY_DECLARE       */
        { "list",           4, 4 },             /* TY_LIST          */
        { "array",          4, 4 },             /* TY_ARRAY         */
        { "hash",           4, 4 },             /* TY_HASH          */

        { "structi",        0, 4 },             /* TY_STRUCTI       */
        { "unioni",         0, 4 },             /* TY_UNIONI        */
        { "enumi",          0, 4 },             /* TY_ENUMI         */
        };


/* see enum _Tmodifiers */
static const struct tms {
    symtype_t       val;
    const char*     desc;
} ids_tms[] = {
        { TM_FUNCTION,      "function " },
        { TM_POINTER,       "* " },
        { TM_OPTIONAL,      "~ " },
        { TM_REFERENCE,     "& " }
        };


/* see enum _Tstore */
static const struct scs {
    symtype_t       val;
    const char*     desc;
} ids_scs[] = {
        { SC_AUTO,          "" },               /*implied*/
        { SC_REGISTER,      "register " },
        { SC_STATIC,        "static " },
        { SC_EXTERN,        "extern " },
        { SC_TYPEDEF,       "typedef " },
        { SC_LOCAL,         "local " },
        { SC_REPLACEMENT,   "replacement " },
        { SC_PARAM,         "parameter " },
        { SC_PARAMREG,      "parameter-register" }
        };


/* see enum _Tqualif */
static const struct qus {
    symtype_t       val;
    const char*     desc;
} ids_qus[] = {
        { TQ_CONST,         "const " },
        { TQ_VOLATILE,      "volatile " },
        { TQ_RESTRICT,      "restrict " },
        };

/* see enum symprim_t */
static const int    symprim_types[] = {
        K_INT,              /* SYMPRIM_INT */
        K_STRING,           /* SYMPRIM_STRING */
        K_LIST,             /* SYMPRIM_LIST */
        K_ARRAY,            /* SYMPRIM_ARRA */
        K_DECLARE,          /* SYMPRIM_DECLARE */
        K_FLOAT,            /* SYMPRIM_FLOAT */
    //  K_BOOL,
    //  K_HASH,
    //  K_ARRAY
    };


/*
 *  Function to initialise symbol table. May be called multiple times. Each time means
 *  free up data structures from previous usage.
 */
void
symtab_init(void)
{
    assert(FM_MAX <= (FM_MASK >> FM_SHIFT));
    assert(TM_MAX <= (TM_MASK >> TM_SHIFT));
    assert(SC_MAX <= (SC_MASK >> SC_SHIFT));
    assert(TQ_MAX <= (TQ_MASK >> TQ_SHIFT));

    assert(TY_MAX <= TY_LO_MASK);
    assert(TY_MAX+1 == sizeof(ty_info)/sizeof(ty_info[0]));

    assert(SYMPRIM_MAX == sizeof(symprim_types)/sizeof(symprim_types[0]));

    assert(sizeof(symtype_t) <= sizeof(accint_t));  /* atom.ival usage */

    TAILQ_INIT(&x_symqueue);
    hd_syms = NULL;
}


void
symtab_close(void)
{
    symbol_t *sp;

    if (hd_syms) {
        List_p lp;

        while ((lp = ll_first(hd_syms)) != NULL) {
            sp = (symbol_t *) ll_elem(lp);
            sym_free(sp);
            ll_delete(lp);
        }
        ll_free(hd_syms);
        hd_syms = NULL;
    }

    if (TAILQ_FIRST(&x_symqueue)) {
        xprintf("Unresolved symbols\n");

        TAILQ_FOREACH(sp, &x_symqueue, s_node) {
            xprintf("\t%d: %s\n", sp->s_level, sp->name);
        }
    }
    assert(NULL == TAILQ_FIRST(&x_symqueue));
}


/*  Function:           sym_alloc
 *      Allocation symbol storage.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Address of new symbol structure.
 */
symbol_t *
sym_alloc(const char *name)
{
    static symbol_t null_symbol;
    symbol_t *sp;

    sp = (symbol_t *) chk_alloc(sizeof(symbol_t));
    *sp = null_symbol;
    sp->name = chk_salloc(name);
    D_SYMBOL_MAGIC(sp->magic = SYMBOL_MAGIC;)
    sp->s_references = 1;
    TAILQ_INSERT_TAIL(&x_symqueue, sp, s_node);
    return sp;
}


/*  Function:           sym_reference
 *      Increment a symbole reference count.
 *
 *  Parameters:
 *      sp -                Symbol structure.
 *
 *  Returns:
 *      Symbol storage.
 */
symbol_t *
sym_reference(symbol_t *sp)
{
    if (sp) {
        assert(sp);
        D_SYMBOL_MAGIC(assert(sp->magic != 0x5a5a5a5a);)
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)
        assert(sp->s_references >= 1);
        ++sp->s_references;
    }
    return sp;
}


/*  Function:           sym_free
 *      Decrement and release the symbol storage.
 *
 *  Parameters:
 *      sp -                Symbol structure.
 *
 *  Returns:
 *      nothing
 */
void
sym_free(symbol_t *sp)
{
    assert(sp);
    D_SYMBOL_MAGIC(assert(sp->magic != 0x5a5a5a5a);)
    D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)
    assert(sp->s_references >= 1);

    if (sp->s_references > 1) {
        --sp->s_references;

    } else {
        D_SYMBOL_MAGIC(sp->magic = 0x5a5a5a5a;)
        TAILQ_REMOVE(&x_symqueue, sp, s_node);

        if (sp->s_defn) {
            sym_free(sp->s_defn);
        }

        chk_free((void *) sp->s_arguments);
        chk_free((void *) sp->name);

        if (sp->s_members) {
            List_p lp;

            while (NULL != (lp = ll_first(sp->s_members))) {
                symbol_t *t_sp = (symbol_t *) ll_elem(lp);
                sym_free(t_sp);
                ll_delete(lp);
            }
            ll_free(sp->s_members);
        }

        chk_free(sp);
    }
}


void
sym_check(symbol_t *sp)
{
    assert(sp);
    D_SYMBOL_MAGIC(assert(sp->magic != 0x5a5a5a5a);)
    D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)
    assert(sp->s_references >= 1);
}


/*
 *  Function to add a symbol to symbol table taking into account current block nesting
 *  level. Also detects duplicate definition errors.
 */
symbol_t *
sym_add(const char *sym, node_t *np, symtype_t type)
{
    return sym_push(&hd_syms, INSERT_AT_FRONT, sym, np, type, (symbol_t *) NULL);
}


/*
 *  Summaries a types pointer components, by walking the left side of the tree.
 */
static symtype_t
pointer_summaries(node_t *np, unsigned *indirect)
{
    unsigned t_indirect = 0;                    /* indirection count */
    symtype_t type = 0;

    if (np) {
        do {
            if (node_keywd == np->type) {
                switch (np->atom.ival) {
                case TO_PTR:
                    type |= (TM_POINTER << TM_SHIFT);
                    ++t_indirect;               /* XXX - encode within type? */
                    break;

                case TO_REF:
                    type |= (TM_REFERENCE << TM_SHIFT);
                    break;
                }
            }
        } while (NULL != (np = np->left));
    }

    if (indirect) {
        *indirect = t_indirect;
    }
    return type;
}


/*
 *  General symbol redefinition information regarding the previous definition
 */
static void
sym_original(int error, symbol_t *sp, const char *name)
{
    if (sp->s_filename && sp->s_line_no != x_lineno) {
        if (error) {
            crerrorx(RC_ERROR, "\r\t%s(%d) : see declaration of '%s'", sp->s_filename, sp->s_line_no, name);
        } else {
            crwarnx(RC_ERROR, "\r\t%s(%d) : see declaration of '%s'", sp->s_filename, sp->s_line_no, name);
        }
    }
}


static void
sym_warning(unsigned first, symbol_t *sp, const char *name, const char *why, ...)
{
    va_list ap;

    va_start(ap, why);
    crwarnv(RC_ERROR, why, ap);
    va_end(ap);

    if (0 == first) {                           /* first error */
        sym_original(FALSE, sp, name);
    }
}


static void
sym_error(unsigned first, symbol_t *sp, const char *name, const char *why, ...)
{
    va_list ap;

    va_start(ap, why);
    crerrorv(RC_ERROR, why, ap);
    va_end(ap);

    if (0 == first) {                           /* first error */
        sym_original(TRUE, sp, name);
    }
}


/*
 *  Function redefinition error support function, generates a labelled error
 *  message of the first error and non-labelled on the remaining.
 */
static void
function_redef(unsigned first, symbol_t *sp, const char *name, const char *why, ...)
{
    char buffer[1024];
    va_list ap;

    va_start(ap, why);
    vsprintf(buffer, why, ap);
    va_end(ap);

    if (0 == first) {                           /* first error */
        crerrorx(RC_ERROR, "function '%s' redefinition; %s", name, buffer);
        sym_original(TRUE, sp, name);

    } else {                                    /* additional */
        crerror(RC_ERROR, buffer);
    }
}


/*
 *  Compare two storage-classes within a type declaration
 */
static int
sc_same(symtype_t type1, symtype_t type2)
{
    type1 = (type1 & SC_MASK) >> SC_SHIFT;
    type2 = (type2 & SC_MASK) >> SC_SHIFT;

    if ((type1 & (symtype_t)~SC_EXTERN) == (type2 & (symtype_t)~SC_EXTERN)) {
        return 1;
    }
    return 0;
}


/*  Function:           sym_push
 *      Push the symbol to the specified symbol table.
 *
 *  Parameters:
 *      hd -                Symbol table.
 *      name -              Symbol name.
 *      np -                Definition (if any).
 *      type -              Type TY_xxxx.
 *      owner -             Owner.
 *
 *  Returns:
 *      Address of symbol object, otherwise NULL on a redefinition error.
 */
symbol_t *
sym_push(Head_p *hd, int order, const char *name, node_t *np, symtype_t type, symbol_t *owner)
{
    symtype_t ty = (type & TY_MASK);
    symtype_t sc = (type & SC_MASK) >> SC_SHIFT;
    symtype_t istag = symtype_istag(type);

    int islocal = (sc == SC_LOCAL ? 1 : 0);
    int isfunct = (type & (TM_FUNCTION << TM_SHIFT)) ? 1 : 0;

    argtype_t *arguments = NULL;
    int redef = FALSE;
    List_p lp;
    symbol_t *sp;

    /*convert local to an internal status*/
    if (islocal) {
        sc = 0; type &= (symtype_t) ~SC_MASK;
    }

    /*parse pointers/references*/
    type |= pointer_summaries(np, NULL);
    if (type & (TM_POINTER << TM_SHIFT)) {
        crerror(RC_UNSUPPORTED_POINTER, "use of pointers unsupported");
    }

    /*function argument lists,
     *
     *  np == NULL, if the following construct:
     *
     *      function {
     *      }
     */
    if (! isfunct) {
        if (np && np->type == node_keywd && np->atom.ival == TO_FUNC) {
            /*
             *  prototype ..
             */
            function_arglist(name, np, 0);
            type |= TM_FUNCTION << TM_SHIFT;
            isfunct = 2;
        }
    }

    if (isfunct) {                              /* 16/08/08 */
        unsigned argc = 0;

        xprintf("adding function [%s] 0x%08X %s %s\n",
            (isfunct == 1 ? "definition" : "declaration/prototype"), (unsigned)type, symtype_to_defn(type), name);

        assert(NULL == np || node_keywd == np->type);
        assert(NULL == np || TO_FUNC == np->atom.ival);

        if (type & (TM_REFERENCE << TM_SHIFT)) {
            crerror(RC_UNSUPPORTED_REFFUNC, "reference to function");
        }

        if (np) {
            if (np->right) {
                /*
                 *  function(arglist)
                 */
                Head_p arglist = np->right->atom.arglist;
                List_p lp;

                assert(node_arglist == np->right->type);
                assert(NULL != arglist);

                for (lp = ll_first(arglist); lp; lp = ll_next(lp)) {
                    ++argc;                     /* argument count */
                }

                if (argc) {
                    argtype_t *t_arguments = chk_alloc(sizeof(argtype_t) * (argc + 1));

                    arguments = t_arguments;

                    if (argc > 255) {           /* system limit? */
                        crerrorx(RC_ERROR, "'%s' : argument count exceeds 255.", name);
                        argc = 255;
                    }

                    *t_arguments++ = (argtype_t)argc;

                    for (lp = ll_first(arglist); lp && argc--; lp = ll_next(lp)) {
                        node_t *t_np = (node_t *) ll_elem(lp);

                        if (t_np) {
                            assert(node_type == t_np->type);
                            *t_arguments++ = (argtype_t)symtype_arg((symtype_t)t_np->atom.ival);

                        } else {                /* ... */
                            *t_arguments++ = ARG_REST;
                        }
                    }
                }

            } else {
                /*
                 *  function() treat as
                 *      function(...)
                 */
                arguments = chk_alloc(sizeof(argtype_t) * 2);

                arguments[0] = 1;
                arguments[1] = ARG_REST;        /* no prototype */
            }
        }

    } else {
        xprintf("adding ");
        if (np) {
            xprintf("[");
            node_dprint(np, 0);
            xprintf("] ");
        }
        xprintf("%s 0x%08X %s %s\n",
            (istag ? "tag" : "symbol"), (unsigned)type, symtype_to_defn(type), name);

        if (np) {
            if (np->right && np->right->right) {
                if (type & (TM_REFERENCE << TM_SHIFT)) {
                    crerror(RC_UNSUPPORTED_REFDEFAULT, "'defaulted argument' can not be bound to a reference");
                }
            }
            np->atom.ival = (accint_t)type;
        }
    }

    /*redefinition checks*/
    if (NULL == *hd) {
        *hd = ll_init();
    }

    for (lp = ll_first(*hd); lp; lp = ll_next(lp)) {
        unsigned symerr = 0;
        symtype_t t_ty, t_sc, t_istag;

        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        t_sc = (sp->s_type & SC_MASK) >> SC_SHIFT;
        t_ty = sp->s_type & TY_MASK;
        t_istag = symtype_istag(sp->s_type);

        /*
         *  Match symbol-name and namespace
         */
        if (sp->name[0] != name[0] || 0 != strcmp(sp->name + 1, name + 1)) {
            continue;                           /* symbol-name */
        }

        if (istag != t_istag) {
            continue;                           /* namespace */
        }

        /*
         *  Same type and namespace
         */
        if (! isfunct) {
            if (sp->s_level != x_block_level) {
                if ((sp->s_flags & SF_FUNCTION) || islocal) {
                    /*
                     *  ignore function and 'local' definitions
                     */
                    continue;
                }

                if (sc >= SC_PARAM) {           /* XXX - confirm usage */
                    sym_warning(symerr++, sp, name,
                        "parameter '%s' declaration hides another field or variable.", name);

                } else {
                    sym_warning(symerr++, sp, name,
                        "local '%s' declaration hides another field or variable.", name);
                }
                continue;
            }
        }

        if ((sp->s_type & TM_MASK) != (type & TM_MASK)) {
            sym_error(symerr++, sp, name,
                    "'%s' : redefinition; different type modifiers", name);

        } else if ((sp->s_type & SC_MASK) != (type & SC_MASK) && ! sc_same(sp->s_type, type)) {
            sym_error(symerr++, sp, name,
                    "'%s' : redefinition; different storage class specified", name);

        } else if ((sp->s_type & TY_MASK) != (type & TY_MASK)) {
            sym_error(symerr++, sp, name,
                    "'%s' : redefinition; different types", name);

        } else {
            /*
             *  Forward references
             */
            if (sp->s_flags & SF_FORWARD) {
                if (istag && t_istag == istag) {
                    /*
                     *  Forward reference
                     */
                    return sp;                  /* return original */
                }
            }

            /*
             *  Functions
             */
            if (isfunct) {
                if ((sp->s_flags & SF_BODY) && 1 == isfunct) {
                    /*
                     *  The function has already been defined.
                     */
                    crerrorx(RC_ERROR, "function '%s' already has a body", name);

                } else {
                    unsigned argerrors = 0;     /* argument error count */

                    if (sp->s_arguments != arguments) {
                        const unsigned argc = (arguments ? (unsigned)arguments[0] : 0);
                        const unsigned spc = (sp->s_arguments ? (unsigned)sp->s_arguments[0] : 0);

                        if (argc && spc) {
                            const argtype_t *t_arguments = arguments + 1;
                            const argtype_t *s_arguments = sp->s_arguments + 1;
                            unsigned arg = 1;

                            /*
                             *  Match each and every parameter ..
                             */
                            while (arg <= argc && arg <= spc) {
                                if (*t_arguments != *s_arguments) {
                                    if ((*t_arguments == ARG_REST || *t_arguments == ARG_VOID) &&
                                            (*s_arguments == ARG_REST || *s_arguments == ARG_VOID)) {
                                        /*
                                         *  func([...]) and func(void)
                                         */
                                        sym_warning(symerr++, sp, name,
                                            "formal parameter different from declaration");

                                    } else {
                                        if (1 == isfunct) {
                                            function_redef(symerr|argerrors++, sp, name,
                                                "formal parameter '%d' different from declaration", arg);

                                        } else {
                                            function_redef(symerr|argerrors++, sp, name,
                                                "parameter '%d' declaration different", arg);
                                        }
                                    }
                                }
                                ++t_arguments, ++s_arguments;
                                ++arg;          /* next */
                            }
                        }

                        if (argc != spc) {      /* argument count errors .. */
                            if (spc > argc) {
                                function_redef(symerr|argerrors++, sp, name,
                                    "first formal parameter list longer than the second list (%d/%d)", spc, argc);
                            } else {
                                function_redef(symerr|argerrors++, sp, name,
                                    "second formal parameter list longer than the first list (%d/%d)", spc, argc);
                            }
                        }
                    }

                    if (0 == argerrors) {
                        if (1 == isfunct) {
                            /*
                             *  declaration, override prototype
                             */
                            chk_free(sp->s_arguments);
                            redef = TRUE;
                            goto setup;
                        }
                    }
                    chk_free(arguments);
                    return NULL;
                }

            } else {
                if ((sp->s_type & SC_MASK) == (SC_EXTERN << SC_SHIFT))
                    continue;                   /* ignore externals */

                /*
                 *  XXX - allow dup definitions?
                 */
                sym_error(symerr++, sp, name,
                    "symbol '%s' redefinition error", name);
            }
        }

        if (0 == symerr++) {
            sym_original(TRUE, sp, name);
        }

        chk_free(arguments);
        return NULL;
    }

    /*
     *  warning/errors regarding unsupported types
     */
    if (1 != x_block_level) {
        /*
         *  references unless arguments are not supported (08/05/09)
         */
        if (type & (TM_REFERENCE << TM_SHIFT)) {
            if (0 == x_block_level) {
                crerror(RC_UNSUPPORTED_REFGLOBOL, "global references are not permitted");
            } else {
                crerror(RC_UNSUPPORTED_REFVAR, "reference variables are not supported");
            }
        }
    }

    if (istag) {
        if (1 == x_block_level) {
            /*
             *  Tags within parameter lists
             */
            crwarnx(RC_ERROR, "\"%s %s\" declared inside a parameter list", symtype_to_defn(type), name);
            if (cronceonly(ONCEONLY_TAGPARAMETERSCOPE)) {
                crwarn(0, "its scope is only this definition or declaration, which is probably not what is needed");
            }
        }

    } else if (! isfunct) {
        /*
         *  Unsupported type
         */
        switch (ty) {
        case TY_ENUMI:
        case TY_ENUMCONST:
        case TY_VOID:
            break;
        default:
            if (symtype_map(type) == SYMPRIM_UNKNOWN) {
                crwarnx(RC_ERROR, "use of unsupported type '%s'", symtype_to_str(type));
            }
            break;
        }
    }

    /*
     *  create
     */
    sp = sym_alloc(name);
setup:;
    sp->s_level = (isfunct ? 0:x_block_level);  /* functions all have global scope */
    sp->s_tree = np;
    sp->s_owner = owner;
    sp->s_type = type;
    sp->s_arguments = arguments;                /* function argument summary */
    sp->s_line_no = x_lineno;
    sp->s_filename = filename_cache(x_filename);

    if (islocal) {
        sp->s_flags |= SF_LOCAL;

    } else if (isfunct) {
        sp->s_flags |= SF_FUNCTION;
        if (1 == isfunct) {
            sp->s_flags |= SF_BODY;             /* function body */
        }
    }

    if (! redef) {
        if (INSERT_AT_END == order || isfunct) {
            ll_append(*hd, (char *) sp);

        } else {
            ll_push(*hd, (char *) sp);
        }
    }
    return sp;
}


/*  Function:           sym_implied_function
 *      Define the function 'name' with the specified arguments with the implied type
 *      of 'extern int'.
 *
 *  Parameters:
 *      name -              Function name.
 *      argumentc -         Argument count.
 *      arguments -         Argument vector.
 *
 *  Returns:
 *      Address of the created symbol.
 *
 */
symbol_t *
sym_implied_function(const char *name, int argumentc, const argtype_t *arguments)
{
    symbol_t *sp;

    if (NULL != (sp = sym_alloc(name))) {

        sp->s_level = 0;                        /* functions all have global scope */
        sp->s_tree  = NULL;
        sp->s_owner = NULL;
        sp->s_type  = (TM_FUNCTION << TM_SHIFT) | (SC_EXTERN << SC_SHIFT) | TY_INT;

        if (argumentc > 0 && arguments) {
            argtype_t *t_arguments = chk_alloc(sizeof(argtype_t) * (argumentc + 1));

            if (t_arguments) {
                sp->s_arguments = t_arguments;

                *t_arguments++ = argumentc;
                while (argumentc-- > 0) {
                    *t_arguments++ = *arguments++;
                }
            }
        }

        sp->s_line_no = x_lineno;
        sp->s_filename = filename_cache(x_filename);
        sp->s_flags |= SF_FUNCTION;

        ll_append(hd_syms, (void *) sp);
    }
    return sp;
}


/*  Function:           sym_lookup
 *      Function to check that a symbol has been predefined.
 *
 *  Parameters:
 *      name -              Symbol name.
 *      flags -             Search flags
 *
 *          SYMLK_FUNDEF
 *              allow symbol to be undefined returning NULL, otherwise an
 *              "undefined symbol" error is thrown and a dummy symbol is created.
 *
 *          SYMLK_FTAG
 *              Search tag namespace, otherwise symbol/typedef.
 *
 *  Returns:
 *      Symbol definition, otherwise NULL.
 */
symbol_t *
sym_lookup(const char *name, unsigned flags)
{
    unsigned undef = ((SYMLK_FUNDEF & flags) ? 1 : 0);
    unsigned istag = ((SYMLK_FTAG & flags) ? 1 : 0);

    register List_p lp;
    register symbol_t *sp;

    /*
     *  first symbol in file wont have hd_syms set up. This is ok, since it must be a function name
     */
    if (NULL == hd_syms) {
        if (!undef && 0 != strcmp(name, "NULL")) {
            goto undef_ref;
        }
        return NULL;
    }

    /*
     *  search symbol table inner block to outer block
     */
    for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        if (sp->name[0] == *name && 0 == strcmp(sp->name+1, name+1)) {
            if (istag == (symtype_istag(sp->s_type) ? 1 : 0)) {
                /*
                 *  Mark as referenced and return
                 */
                sp->s_flags |= SF_REF;
                return sp;
            }
        }
    }

    if (undef || 0 == strcmp(name, "NULL")) {
        return NULL;
    }

    /*
     *  undefine, one-shot error
     */
undef_ref:
    if (NULL == hd_undef) {
        hd_undef = ll_init();
    }
    for (lp = ll_first(hd_undef); lp; lp = ll_next(lp)) {
        if (0 == strcmp((const char *) ll_elem(lp), name)) {
            return NULL;
        }
    }
    crerrorx(RC_ERROR, "undefined symbol '%s'", name);
    ll_append(hd_undef, chk_salloc(name));
    return NULL;
}



/*  Function:           sym_typeof
 *      Determine the type of the specified symbol
 *
 *  Parameters:
 *      symbol -            Symbol name.
 *      flags -             Search flags.
 *
 *          SYMLK_FUNDEF
 *              allow symbol to be undefined returning NULL, otherwise an
 *              "undefined symbol" error is thrown and a dummy symbol is created.
 *
 *          SYMLK_FTAG
 *              Search tag namespace, otherwise symbol/typedef.
 *
 *          SYMLK_FBUILTIN
 *              Search builtins
 *
 *  Returns:
 *      Symbol type otherwise TY_UNDEF.
 */
symtype_t
sym_typeof(const char *symbol, unsigned flags)
{
    symtype_t ret = TY_UNDEF;
    symbol_t *sp = NULL;
    BUILTIN *bp = NULL;

    if (NULL != (sp = sym_lookup(symbol, flags))) {
        ret = sp->s_type;

    } else if (SYMLK_FBUILTIN & flags) {
        if (NULL != (bp = builtin_lookup(symbol))) {
            switch (bp->b_rtntype) {
            case ARG_ANY:    ret = TY_DECLARE; break;
            case ARG_NUM:    ret = TY_FLOAT;   break;
            case ARG_INT:    ret = TY_INT;     break;
            case ARG_FLOAT:  ret = TY_FLOAT;   break;
            case ARG_STRING: ret = TY_STRING;  break;
            case ARG_LIST:   ret = TY_LIST;    break;
            case ARG_ARRAY:  ret = TY_ARRAY;   break;
            case ARG_VOID:   ret = TY_VOID;    break;
            case ARG_NULL:   ret = TY_VOID;    break;
            case ARG_SYMBOL:
                assert(ARG_SYMBOL != bp->b_rtntype);
                break;
            case ARG_UNDEF:
                break;
            default:
                assert(0);
                break;
            }
        }
    }
    xprintf("\tsym_typeof(%s)-> %s %u (%s)\n", symbol,
        (sp ? "symbol" : (bp ? "builtin" : "unknown")), (unsigned)ret, symtype_to_str(ret));
    return ret;
}


symtype_t
typedef_type(const char *name)
{
    const symbol_t *sp = sym_lookup(name, 0);

    assert(NULL != sp);
    return sp->s_type & (TQ_MASK | TY_MASK);
}


void
block_enter(void)
{
    block_t *bp = (block_t *) chk_alloc(sizeof(block_t));

    ++x_block_level;
    if (NULL == hd_block) {
        hd_block = ll_init();
    }
    bp->b_args = hd_arglist;
    bp->b_stmt = hd_stmt;
    bp->b_init = hd_init;
    ll_push(hd_block, (char *) bp);
    hd_arglist = NULL;
    hd_stmt = NULL;
    hd_init = NULL;
}


/*
 *  Code to remove symbols from symbol table from currently nested block
 */
node_t *
block_exit(void)
{
    node_t *ty_trees[SYMPRIM_MAX], *auto_consts;
    node_t *sty_trees[SYMPRIM_MAX], *static_consts, *static_list;

    register List_p lp;
    register symbol_t *sp;
    node_t *n, *tree = NULL;
    Head_p statics;
    symtype_t ty, sc, qu;
    symprim_t j;

    /* Parse symbols and collect together declarations of the same types */
    statics = ll_init();
    for (j = 0; j < SYMPRIM_MAX; ++j) {
        ty_trees[j] = sty_trees[j] = NULL;
    }

    static_consts = static_list = NULL;
    auto_consts = NULL;

    if (hd_syms)
        while ((lp = ll_first(hd_syms)) != 0) {
            sp = (symbol_t *) ll_elem(lp);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            if (sp->s_level != x_block_level) {
                break;
            }

            ty = (sp->s_type & TY_MASK);
            qu = (sp->s_type & TQ_MASK) >> TQ_SHIFT;
            sc = (sp->s_type & SC_MASK) >> SC_SHIFT;

            /* unreference warning */
            if ((sp->s_flags & SF_REF) == 0)
                if (sc != SC_EXTERN) {          /* ignore extern's, 05/11/07 */
                    switch (ty) {
                    case TY_STRUCT:
                    case TY_UNION:
                    case TY_ENUM:
                    case TY_ENUMCONST:
                        break;
                    default:
                        crwarnx_line(RC_UNUSED, sp->s_line_no, "unused %s %s",
                                1 == sp->s_level ? "parameter" : "variable", sp->name);
                        break;
                    }
                }

            j = symtype_map(ty);

            if (SYMPRIM_UNKNOWN != j && SC_STATIC == sc) {
                /* gather declarations into those of the same type */
                n = new_symbol(sp->name);
                if (NULL == sty_trees[j]) {
                    sty_trees[j] = n;
                } else {
                    sty_trees[j] = node(K_NOOP, sty_trees[j], n);
                }
                ll_push(statics, sp);

                /* const statics */
                if (qu & TQ_CONST) {
                    n = new_symbol(sp->name);
                    if (NULL == static_consts) {
                        static_consts = n;
                    } else {
                        static_consts = node(K_NOOP, static_consts, n);
                    }
                }

                /* build static definition list */
                n = new_symbol(sp->name);
                if (NULL == static_list) {
                    static_list = n;
                } else {
                    static_list = node(K_NOOP, static_list, n);
                }

            } else {
                /* gather declarations into those of the same type */
                if (SYMPRIM_UNKNOWN != j && 0 == sc) {
                    n = new_symbol(sp->name);
                    if (NULL == ty_trees[j]) {
                        ty_trees[j] = n;
                    } else {
                        ty_trees[j] = node(K_NOOP, ty_trees[j], n);
                    }
                }

                /* const auto */
                if (qu & TQ_CONST) {
                    n = new_symbol(sp->name);
                    if (NULL == auto_consts) {
                        auto_consts = n;
                    } else {
                        auto_consts = node(K_NOOP, auto_consts, n);
                    }
                }

                xprintf("removing symbol %s\n", sp->name);
                sym_free(sp);
            }
            ll_delete(lp);
        }

    /* Build a tree containing all the symbols to be defined on entry to the enclosing block. */
    for (j = 0; j < SYMPRIM_MAX; j++)
        if (ty_trees[j] != NULL) {
            n = node(symprim_types[j], ty_trees[j], (node_t *) NULL);
            if (NULL == tree) {
                tree = n;
            } else {
                tree = node(K_NOOP, tree, n);
            }
            ty_trees[j] = NULL;
        }

    /* Build a block for all statics and their associated initialisation (in any). */
    if (ll_first(statics)) {
        node_t *s_tree = NULL;
        Head_p hd = NULL;

        /* declarations */
        for (j = 0; j < (int)SYMPRIM_MAX; j++)
            if (sty_trees[j] != NULL) {
                n = node(symprim_types[j], sty_trees[j], (node_t *) NULL);
                if (NULL == s_tree) {
                    s_tree = n;
                } else {
                    s_tree = node(K_NOOP, s_tree, n);
                }
                sty_trees[j] = NULL;
            }

        /* associated initialisation */
        if (hd_init)
            for (lp = ll_first(hd_init); lp;) {
                List_p slp, nlp;

                nlp = ll_next(lp);
                n = (node_t *) ll_elem(lp);

                for (slp = ll_first(statics); slp; slp = ll_next(slp)) {
                    sp = (symbol_t *) ll_elem(slp);
                    D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

                    assert(node_symbol == n->left->type);
                    if (strcmp(sp->name, n->left->atom.sval) == 0) {
                        s_tree = node(K_NOOP, s_tree, n);
                        ll_delete(lp);          /* remove */
                        break;
                    }
                }
                lp = nlp;                       /* next */
            }

        /* const */
        if (static_consts) {
            s_tree = node(K_NOOP, s_tree,
                        node(K_CONST, static_consts, (node_t *) NULL));
            static_consts = NULL;
        }

        /* static */
        s_tree = node(K_NOOP, s_tree,
                        node(K_STATIC, static_list, (node_t *) NULL));
        static_list = NULL;

        /* enclose within a conditional first_time() block */
        list_append(&hd, s_tree);
        n = node(K_FUNCALL, new_symbol("first_time"), (node_t *) NULL);
        n = node(K_IF, n, (node_t *)hd);
        tree = node(K_NOOP, tree, n);

        /* cleanup */
        while ((lp = ll_first(statics)) != NULL) {
            sp = (symbol_t *) ll_elem(lp);
            xprintf("removing symbol %s\n", sp->name);
            sym_free(sp);
            ll_delete(lp);
        }
    }

    ll_free(statics);

    /* now append the list of initializers for these local variables (if there are any) */
    if (hd_init) {
        tree = node(K_NOOP, tree,
                node(K_CONSTRUCTORS, (node_t *) hd_init, (node_t *) NULL));
        hd_init = NULL;
    }

    /* const */
    if (auto_consts) {
        tree = node(K_NOOP, tree,
                node(K_CONST, auto_consts, (node_t *) NULL));
        auto_consts = NULL;
    }

    block_pop();
    return tree;
}


void
block_pop(void)
{
    List_p lp;
    block_t *bp;

    if (hd_syms) {
        while ((lp = ll_first(hd_syms)) != 0) {
            symbol_t *sp = (symbol_t *) ll_elem(lp);

            if (sp->s_level != x_block_level) {
                break;
            }
            xprintf("removing symbol %s\n", sp->name);
            sym_free(sp);
            ll_delete(lp);
        }
    }

    if (hd_arglist) {
        list_free(&hd_arglist);
    }

    assert(x_block_level >= 1);
    --x_block_level;

    bp = (block_t *) ll_elem(ll_first(hd_block));
    hd_arglist = bp->b_args;
    hd_stmt = bp->b_stmt;
    hd_init = bp->b_init;

    chk_free(bp);
    ll_pop(hd_block);
}


int
sym_print(symbol_t *sp, int tab)
{
    const char *tstr;
    symtype_t type, sc;
    int size;

    if (! xf_struct && tab) {
        printf("%*.s", tab, "\t\t\t\t\t\t");
    }

    type = sp->s_type & TY_MASK;
    sc = ((sp->s_type & SC_MASK) >> SC_SHIFT);

    if (xf_struct && !tab) {
        /*
         *  Dont print typedef's in +struct mode
         */
        if (sc == SC_TYPEDEF) {
            return 0;
        }
    }

    tstr = symtype_to_defn(type);
    if (xf_struct) {
        /*
         *  Print struct/union types for global variables if in +struct mode.
         */
        if (! tab) {
            if (TY_STRUCTI != type && TY_UNIONI != type) {
                return 0;
            }

            if (sp->s_defn) {
                printf("%s %s",
                    (type == TY_STRUCTI ? "struct" : "union"), sp->s_defn->name);
            }
        }

    } else {
        printf("%s ", tstr);
        if (symtype_isinstance(type)) {
            if (sp->s_defn) {
                printf("%s ", sp->s_defn->name);

            } else {
                printf("<dont-know> ");
            }
        }
    }

    size = sym_size(sp);
    if (sp->s_tree) {
        size = node_print(sp->s_tree, size);
    }

    if (xf_struct) {
        printf("%s\n", sp->name);
    } else {
        printf("%s;\n", sp->name);
    }
    return size;
}


static int
sym_size(symbol_t *sp)
{
    symtype_t type = sp->s_type & TY_LO_MASK;
    symbol_t *defn;

    switch (type) {
    case TY_STRUCTI:
    case TY_UNIONI:
        if (NULL == (defn = sp->s_defn)) {
            return 0;
        }
        sym_check(defn);
        assert(TY_STRUCT == defn->s_type || TY_UNION == defn->s_type);
        return defn->s_size;

    case TY_ENUMI:
        if (NULL == (defn = sp->s_defn)) {
            return 0;
        }
        sym_check(defn);
        assert(TY_ENUM == defn->s_type);
        type = defn->s_enumtype;
        assert(TY_INT == type || TY_STRING == type || TY_IMPLICIT == type);
        break;

    default:
        if (TY_UNSIGNED == type || TY_SIGNED == type) {
            type = (sp->s_type >> TY_SHIFT) & TY_LO_MASK;
            if (TY_IMPLICIT == type) {
                type = TY_INT;
            }
        }
    }
    return ty_info[type].ty_size;
}


static int
sym_alignment(symbol_t *sp)
{
    symtype_t type = sp->s_type & TY_LO_MASK;
    symbol_t *defn;

    switch (type) {
    case TY_STRUCTI:
    case TY_UNIONI:
        if (NULL == (defn = sp->s_defn)) {
            return 0;
        }
        sym_check(defn);
        assert(TY_STRUCT == defn->s_type || TY_UNION == defn->s_type);
        return defn->s_align;

    case TY_ENUMI:
        if (NULL == (defn = sp->s_defn)) {
            return 0;
        }
        sym_check(defn);
        assert(TY_ENUM == defn->s_type);
        type = defn->s_enumtype;
        assert(TY_INT == type || TY_STRING == type || TY_IMPLICIT == type);
        break;

    default:
        if (issign(type)) {
            type = (sp->s_type >> TY_SHIFT) & TY_LO_MASK;
            if (TY_IMPLICIT == type) {
                type = TY_INT;
            }
        }
    }
    return ty_info[type].ty_align;
}


/*  Function:           symtype_make
 *      Build a data-type.
 *
 *  Parameters:
 *      ty1 -               Type specification.
 *      ty2 -               Secondary type specification.
 *
 *  Returns:
 *      Data type.
 */
symtype_t
symtype_make(symtype_t ty1, symtype_t ty2)
{
    assert(0 == (ty1 & (symtype_t)~TY_MASK));
    assert(0 == (ty2 & (symtype_t)~TY_MASK));

    if (ty1 && ty2) {
        if (issign(ty1)) {                      /* force 'sign' as lower */
            return ty1 | (ty2 << TY_SHIFT);
        }
        return ty2 | (ty1 << TY_SHIFT);
    }
    if (ty1) {
        return ty1;
    }
    return ty2;
}


/*  Function:           symtype_base
 *      Return the base type removing any SIGNED/UNSIGNED component.
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Base data type.
 */
symtype_t
symtype_base(symtype_t type)
{
    const symtype_t ty = (type & TY_MASK);
    const symtype_t ty1 = (type & TY_LO_MASK);

    if (issign(ty1)) {
        const symtype_t ty2 = (ty >> TY_SHIFT); /* ignore signed/unsigned */

        if (0 == ty2) {
            return TY_INT;                      /* signed/unsigned [int] */
        }
        return ty2;
    }
    assert(0 == (ty >> TY_SHIFT));
    return ty1;
}


/*  Function:           symtype_sign
 *      Return the sign of the data-type.
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Sign if present.
 */
symtype_t
symtype_sign(symtype_t type)
{
    const symtype_t ty1 =  (type & TY_LO_MASK);
    const symtype_t ty2 = ((type >> TY_SHIFT) & TY_LO_MASK);

    if (issign(ty1))
        return ty1;
    if (issign(ty2))
        return ty2;
    return 0;
}


/*  Function:           symtype_promote_int
 *      Promote the specified type 'type' to a integer type if suitable.
 *
 *  Parameters:
 *      type -              Type specification.
 *      withsign -          TRUE if the sign should be retained.
 *
 *  Returns:
 *      Promoted data type, otherwise the original type.
 */
symtype_t
symtype_promote_int(symtype_t type, int withsign)
{
    const symtype_t base_type = symtype_base(type);
    const symtype_t sign_type = symtype_sign(type);
    symtype_t ret;

    switch (base_type) {
    case TY_LONG:
        if (! xf_grunch) {
#if (SIZEOF_LONG > SIZEOF_INT)
            break;
#endif
        }
    case TY_BOOLEAN:
    case TY_CHAR:
 /* case TY_LONGCHAR: */
    case TY_SHORT:
    case TY_IMPLICIT:
    case TY_INT:
        ret = symtype_make(TY_INT, withsign ? sign_type : 0);
        break;

    default:
        ret = (withsign ? type : base_type);
        break;
    }

    xprintf("\tsymtype_promote_int(type:%d, withsign:%d)->(base:%d, sign:%d) : %d (%s)\n",
        type, withsign, base_type, sign_type, ret, symtype_to_str(ret));
    return ret;
}


/*  Function:           symtype_promote_long
 *      Promote the specified type 'type' to a long type if suitable.
 *
 *  Parameters:
 *      type -              Type specification.
 *      withsign -          TRUE if the sign should be retained.
 *
 *  Returns:
 *      Promoted data type, otherwise the original type.
 */
symtype_t
symtype_promote_long(symtype_t type, int withsign)
{
    symtype_t base_type = symtype_base(type);
    symtype_t sign_type = (withsign ? symtype_sign(type) : 0);

    switch (base_type) {
    case TY_INT:
#if (SIZEOF_INT > SIZEOF_LONG)
        if (! xf_grunch) {
            break;
        }
#endif
    case TY_BOOLEAN:
    case TY_CHAR:
 /* case TY_LONGCHAR: */
    case TY_SHORT:
    case TY_LONG:
        return symtype_make(TY_LONG, sign_type);
    }
    return (withsign ? type : base_type);
}


/*  Function:           symtype_promote_double
 *      Promote the specified type 'type' to an double type if suitable.
 *
 *  Parameters:
 *      type -              Type specification.
 *      withsign -          TRUE if the sign should be retained.
 *
 *  Returns:
 *      Promoted data type, otherwise the original type.
 */
symtype_t
symtype_promote_double(symtype_t type, int withsign)
{
    symtype_t base_type = symtype_base(type);
    symtype_t sign_type = (withsign ? symtype_sign(type) : 0);

    switch (base_type) {
    case TY_BOOLEAN:
    case TY_CHAR:
 /* case TY_LONGCHAR: */
    case TY_SHORT:
    case TY_INT:
    case TY_IMPLICIT:
    case TY_LONG:
 /* case TY_LONGLONG: */
    case TY_FLOAT:
    case TY_DOUBLE:
        return symtype_make(TY_DOUBLE, sign_type);
    }
    return (withsign ? type : base_type);
}


/*  Function:           symtype_coalesce
 *      Merge the current and new basic types.
 *
 *  Parameters:
 *      newtype -           Type specification.
 *      currtype -          Current type value (if any).
 *
 *  Returns:
 *      Resulting type.
 */
symtype_t
symtype_coalesce(symtype_t ntype, symtype_t ctype)
{
    static const struct {
        symtype_t       type1;
        symtype_t       type2;
        symtype_t       conv;
    } conversions[] = {
        { TY_SHORT,     TY_INT,         TY_SHORT      },
        { TY_LONG,      TY_INT,         TY_LONG       },
        { TY_LONG,      TY_CHAR,        TY_LONGCHAR   },
        { TY_LONG,      TY_LONG,        TY_LONGLONG   },
        { TY_LONG,      TY_FLOAT,       TY_DOUBLE     },
        { TY_LONG,      TY_DOUBLE,      TY_LONGDOUBLE },
        { TY_INT,       TY_LONGLONG,    TY_LONGLONG   }
        };

    symtype_t ty1 = (ctype & TY_LO_MASK);
    symtype_t ty2 = (ctype & TY_HI_MASK) >> TY_SHIFT;
    unsigned i;

    xprintf("\tsymtype_coalesce([%u/%s], [ty1=%u/%s, ty2=%u/%s])",
        (unsigned)ntype, symtype_to_desc(ntype), (unsigned)ty1, symtype_to_desc(ty1), (unsigned)ty2, symtype_to_desc(ty2));

    assert((ntype & (symtype_t)~TY_LO_MASK) == 0);

    if (0 == ty1) {
        ty1 = ntype;
        ty2 = 0;
        goto done;
    }

    if (issign(ty1) && issign(ntype)) {
        if (ty1 == ntype) {
            crerrorx(RC_TYPE_SIGNDUP, "duplicate '%s'", (TY_SIGNED == ty1 ? "signed" : "unsigned"));
            return ctype;
        }
        crerror(RC_TYPE_SIGNMIXED, "both signed and unsigned specified");
    }

    for (i = 0; i < sizeof(conversions)/sizeof(conversions[0]); ++i) {
        if (conversions[i].type1 == ntype && conversions[i].type2 == ty1) {
            xprintf("\t  mapped(ty1 ==> %s)", symtype_to_defn(conversions[i].conv));
            ty1 = conversions[i].conv;
            goto done;

        } else if (conversions[i].type2 == ntype && conversions[i].type1 == ty2) {
            xprintf("\t  mapped(ty2 ==> %s)", symtype_to_defn(conversions[i].conv));
            ty2 = conversions[i].conv;
            goto done;
        }
    }

    if (ty1 == ntype || ty2 == ntype) {
        crerrorx(RC_TYPE_DUPLICATE, "data type '%s' specified more then once", symtype_to_desc(ntype));
        return ctype;
    }

    if (symtype_restricted(ty1, ntype) || symtype_restricted(ntype, ty1) ||
            symtype_restricted(ty2, ntype) || symtype_restricted(ntype, ty2)) {
        return ctype;
    }

    if (ty2) {
        crerror(RC_TYPE_MULTIPLE, "two data types in declaration");
        return ctype;
    }

    ty2 = ntype;

done:;
    if (issign(ty2) && ty1) {                   /* force 'sign' as lower */
        ctype = (ctype & (symtype_t)~TY_MASK) | (ty1 << TY_SHIFT) | ty2;

    } else {
        ctype = (ctype & (symtype_t)~TY_MASK) | (ty2 << TY_SHIFT) | ty1;
    }

    xprintf(" = %u/%s\n", ctype, symtype_to_defn(ctype));
    return ctype;
}


static int
symtype_restricted(symtype_t ty1, symtype_t ty2)
{
    const char *restricted = NULL;

    switch (ty1) {
    case TY_VOID:
        if (ty2) {
            crerror(RC_TYPE_VOID, "invalid use of void type");
            return 1;
        }
        break;

    case TY_BOOLEAN:
    case TY_STRING:
    case TY_LIST:
    case TY_ARRAY:
    case TY_HASH:
    case TY_DECLARE:
        switch (ty2) {
        case TY_LONG:
        case TY_SHORT:
        case TY_SIGNED:
        case TY_UNSIGNED:
            restricted = "long, short, signed or unsigned";
            break;
        }
        break;

    case TY_CHAR:
        switch (ty2) {
        case TY_LONG:
        case TY_SHORT:
            restricted = "long, short";
            break;
        }
        break;

    case TY_FLOAT:
    case TY_DOUBLE:
        switch (ty2) {
        case TY_SHORT:
        case TY_SIGNED:
        case TY_UNSIGNED:
            restricted = "short, signed or unsigned";
            break;
        }
        break;
    }
    if (restricted) {
        crerrorx(RC_TYPE_INVALID, "%s cannot be used against %s", restricted, symtype_to_desc(ty1));
        return 1;
    }
    return 0;
}


/*  Function:           symtype_istag
 *      Determine whether the specified type is a struct/union/enum tag.
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Tag type (TY_STRUCT|TY_UNION|TY_ENUM) otherwise 0.
 */
symtype_t
symtype_istag(symtype_t type)
{
    type = type & TY_MASK;

    switch (type) {
    case TY_STRUCT:
    case TY_UNION:
    case TY_ENUM:
        return type & TY_MASK;
    }
    return 0;
}


/*  Function:           symtype_isinstance
 *      Determine whether the specified type is a struct/union/enum instance
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Instance type (TY_STRUCTI|TY_UNIONI|TY_ENUMI) otherwise 0.
 */
symtype_t
symtype_isinstance(symtype_t type)
{
    type = type & TY_MASK;

    switch (type) {
    case TY_STRUCTI:
    case TY_UNIONI:
    case TY_ENUMI:
        return type & TY_MASK;
    }
    return 0;
}


/*  Function:           symtype_map
 *      Map type to a language primitive.
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Type base-type, otherwise -1 if unknown.
 *
 *  Note:
 *      These index values should match (symprim_t) symprim_types[].
 */
symprim_t
symtype_map(symtype_t type)
{
    symtype_t ty1 = (type & TY_LO_MASK);

    type = type & TY_MASK;                      /* mask type */
    if (issign(ty1)) {
        type >>= TY_SHIFT;                      /* ignore signed/unsigned */
        if (! type) {
            type = TY_INT;
        }
    }

    return TY_INT == type ?     SYMPRIM_INT :
           TY_BOOLEAN == type ? SYMPRIM_INT :
           TY_STRING == type ?  SYMPRIM_STRING :
           TY_LIST == type ?    SYMPRIM_LIST :
           TY_ARRAY == type ?   SYMPRIM_ARRAY :
           TY_DECLARE == type ? SYMPRIM_DECLARE :
           TY_FLOAT == type ?   SYMPRIM_FLOAT :
           TY_DOUBLE == type ?  SYMPRIM_FLOAT :
                                SYMPRIM_UNKNOWN;
}


/*  Function:           symtype_arg
 *      Map type to an argument type.
 *
 *  Parameters:
 *      type -              Type specification.
 *
 *  Returns:
 *      Argument base-type, otherwise -1 if unknown.
 */
unsigned
symtype_arg(symtype_t stype)
{
    int is_optional = ((TM_OPTIONAL << TM_SHIFT) == (stype & TM_MASK));
    unsigned ret = 0;

    switch (symtype_map(stype)) {
    case SYMPRIM_INT:       ret = ARG_INT;     break;
    case SYMPRIM_STRING:    ret = ARG_STRING;  break;
    case SYMPRIM_LIST:      ret = ARG_LIST;    break;
    case SYMPRIM_ARRAY:     ret = ARG_ARRAY;   break;
    case SYMPRIM_DECLARE:   ret = ARG_ANY;     break;
    case SYMPRIM_FLOAT:     ret = ARG_FLOAT;   break;
    case SYMPRIM_UNKNOWN:
    default:
        if ((stype & TY_MASK) == TY_VOID) {
            ret = ARG_VOID;
        } else {
            ret = ARG_SYMBOL;
        }
    }
    if (is_optional) {
        ret |= ARG_OPT;
    }
    return ret;
}


const char *
symtype_to_str(symtype_t type)
{
    static char buf[80];
    symtype_t t;

    type &= TY_MASK;
    t = type & TY_LO_MASK;
    if (t > TY_MAX) {
        strcpy(buf, "<type?>");
    } else {
        strcpy(buf, ty_info[t].ty_desc);
    }

    t = (type >> TY_SHIFT) & TY_LO_MASK;
    if (t) {
        if (t > TY_MAX) {
            strcat(buf, " <type?>");
        } else {
            strcat(buf, " ");
            strcat(buf, ty_info[t].ty_desc);
        }
    }
    return buf;
}


const char *
symtype_to_desc(symtype_t type)
{
    type &= TY_MASK;
    if (type > TY_MAX) {
        return "<type?>";
    }
    return ty_info[type].ty_desc;
}


const char *
symtype_to_defn(symtype_t type)
{
    static char buf[128];

    symtype_t tm = (type & TM_MASK) >> TM_SHIFT;
    symtype_t qu = (type & TQ_MASK) >> TQ_SHIFT;
    symtype_t sc = (type & SC_MASK) >> SC_SHIFT;
    int i;

    buf[0] = 0;
    for (i = 0; i < (int)(sizeof(ids_tms)/sizeof(ids_tms[0])); ++i)
        if (tm & ids_tms[i].val) {
            strcat(buf, ids_tms[i].desc);
        }

    if (sc) {
        for (i = (int)(sizeof(ids_scs)/sizeof(ids_scs[0]))-1; i >= 0; --i)
            if (sc == ids_scs[i].val) {
                strcat(buf, ids_scs[i].desc);
                break;
            }
        if (i < 0) {
            strcat(buf, "<class?>");
        }
    }

    if (qu) {
        for (i = (sizeof(ids_qus)/sizeof(ids_qus[0]))-1; i >= 0; --i)
            if (qu & ids_qus[i].val) {
                strcat(buf, ids_qus[i].desc);
                qu &= ~ids_qus[i].val;
            }
        if (qu) {
            printf("<qual?>");
        }
    }

    strcat(buf, symtype_to_str(type));
    return buf;
}


/*  Function:           symtype_dump
 *      Dump the symbol table when the -S switch is used. This is used so that we can
 *      get the structure offset information. This isn't strictly a part of the grunch
 *      language, but since we more or less have a full C syntax interpreter, we can
 *      use grunch for things not otherwise related to brief/grief.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
symtab_dump(void)
{
    register List_p lp, lp1;
    register symbol_t *sp, *sp1;
    symtype_t type;
    Head_p hd_str;
    int offset;
    int max;

    if (NULL == hd_syms || NULL == ll_first(hd_syms)) {
        return;
    }
    hd_str = ll_init();

    /* make a list of all structures -- in the order they were defined. */
    for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        type = sp->s_type & TY_MASK;
        if (type != TY_STRUCT && type != TY_UNION)
            continue;
        if (sp->s_owner)
            continue;
        ll_push(hd_str, (char *) sp);
    }

    /* print out all structure definitions first. */
    for (lp = ll_first(hd_str); lp; lp = ll_next(lp)) {
        const char *cp;

        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        type = sp->s_type & TY_MASK;
        if (sp->s_type == TY_STRUCT) {
            cp = "struct";
        } else {
            cp = "union";
        }

        if (xf_struct) {
            printf("\ndefine %s\n", sp->name);
        } else {
            printf("%s %s {\n", cp, sp->name);
        }

        /* calculate base alignment of this structure from first element inside of it */
        sp->s_align = 0;
        if (sp->s_members && ll_first(sp->s_members)) {
            sp1 = (symbol_t *) ll_elem(ll_first(sp->s_members));
            if (sp1) {
                D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)
                sp->s_align = sym_alignment(sp1);
            } else {
                sp->s_align = 0;
            }
        }

        offset = 0;
        max = 0;

        for (lp1 = sp->s_members ? ll_first(sp->s_members) : NULL; lp1;) {
            int align;

            sp1 = (symbol_t *) ll_elem(lp1);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            if ((sp->s_type & TY_MASK) == TY_UNION) {
                if (offset > max) {
                    max = offset;
                }
                offset = 0;
            }

            if (xf_struct) {
                printf("0x%x=", offset);
            } else  {
                int len = printf("   /* %u/0x%x */", offset, offset);
                if (len < 16) {
                    printf("%*s", len-16, "");
                }
            }

            offset += sym_print(sp1, 1);
            if (NULL == (lp1 = ll_next(lp1))) {
                break;
            }

            sp1 = (symbol_t *) ll_elem(lp1);
            D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

            type = sp1->s_type & TY_MASK;
            align = sym_alignment(sp1);

            if (sp1->s_tree && sp1->s_tree->type == node_keywd &&
                    (sp1->s_tree->atom.ival == TO_PTR || sp1->s_tree->atom.ival == TO_REF)) {
                align = 4;
            }

            if (align && offset & (align - 1)) {
                offset = (offset | (align - 1)) + 1;
            }
        }

        if ((sp->s_type & TY_MASK) == TY_UNION && offset < max) {
            offset = max;
        }

        sp->s_size = offset;

        if (! xf_struct) {
            printf("\t}; /* size=%u/0x%x align=%u/0x%x */\n", offset, offset, sp->s_align, sp->s_align);
        }
    }

    /* print all non-structures and structure members. */
    printf("\n");
    for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
        sp = (symbol_t *) ll_elem(lp);
        D_SYMBOL_MAGIC(assert(sp->magic == SYMBOL_MAGIC);)

        if (! symtype_istag(sp->s_type) && !sp->s_owner) {
            sym_print(sp, 0);
        }
    }
}


/*  Function:           symtab_print
 *      Dump the symbol table.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
symtab_print(void)
{
    register List_p lp;
    register symbol_t *sp;

    if (hd_syms) {
        for (lp = ll_first(hd_syms); lp; lp = ll_next(lp)) {
            sp = (symbol_t *) ll_elem(lp);
            printf("%d: %s\n", sp->s_level, sp->name);
        }
    }
}
/*end*/
