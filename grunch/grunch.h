#ifndef GR_GRUNCH_H_INCLUDED
#define GR_GRUNCH_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_grunch_h,"$Id: grunch.h,v 1.35 2020/06/03 17:18:19 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: grunch.h,v 1.35 2020/06/03 17:18:19 cvsuser Exp $
 * grunch language compiler, structures etc
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

#include <edheaders.h>
#include <edtypes.h>

#include <libmisc.h>
#include <libllist.h>
#include <libsplay.h>
#include <tailqueue.h>
#include <vm_alloc.h>

#include <edopcode.h>

#include "../gr/word.h"
#include "../gr/keywd.h"
#include "crmsg.h"

/* y.tab.c support:
//
//  Macro: YYSTACK_USE_ALLOCA (default = 0)
//      Macro used to control the use of alloca when the deterministic parser in C needs to extend its stacks.
//      If defined to 0, the parser will use malloc to extend its stacks. If defined to 1, the parser will use alloca.
//      Values other than 0 and 1 are reserved for future Bison extensions. If not defined, YYSTACK_USE_ALLOCA defaults to 0.
//
//      In the all-too-common case where your code may run on a host with a limited stack and with unreliable stack-overflow checking,
//      you should set YYMAXDEPTH to a value that cannot possibly result in unchecked stack overflow on any of your target hosts when
//      alloca is called. You can inspect the code that Bison generates in order to determine the proper numeric values.
//      This will require some expertise in low-level implementation details.
//
//  Macro: YYINITDEPTH (default = 200)
//      Macro for specifying the initial size of the parser stack. See Memory Management.
//
//  Macro: YYMAXDEPTH (default = 10000)
//      Macro for specifying the maximum size of the parser stack. See Memory Management.
//
*/
#ifdef  HAVE_STDLIB_H
#if !defined(__GLIBC__)
#define _STDLIB_H
#endif
#endif

#ifdef  HAVE_ALLOCA_H
#if !defined(__GLIBC__)
#define _ALLOCA_H
#endif
#endif

#ifdef  HAVE_STRING_H
#if !defined(__GLIBC__)
#define _STRING_H
#endif
#endif


enum node_types {
    /*0*/ node_keywd,
    /*1*/ node_integer,
    /*2*/ node_float,
    /*3*/ node_string,
    /*4*/ node_symbol,
    /*5*/ node_type,
    /*6*/ node_arglist
};


typedef struct block_t {
    Head_p b_args;
    Head_p b_stmt;
    Head_p b_init;
} block_t;


/* A basic type is:
 *
 *      modifiers (function and type)
 *      storage class
 *      type qualifiers
 *      type1 type2
 *          for example (TY_LONG << TY_SHIFT) | TY_UNSIGNED for a 'unsigned long' type
 *
 *  in a 32 bit field
 */

#define FM_MASK         0x0f000000L
#define TM_MASK         0x00f00000L
#define SC_MASK         0x000f0000L
#define TQ_MASK         0x0000f000L
#define TY_MASK         0x00000fffL

#define TY_HI_MASK      0x00000fc0L             /* 1st */
#define TY_LO_MASK      0x0000003fL             /* 2nd */

#define FM_SHIFT        24                      /* 4 bits */
#define TM_SHIFT        20                      /* 4 bits */
#define SC_SHIFT        16                      /* 4 bits */
#define TQ_SHIFT        12                      /* 4 bits */
#define TY_SHIFT        6                       /* 6 bits for each type (allowing 2) */

typedef uint32_t symtype_t;


/*
 *  Base types
 *  Note, should also match ty_info (64 max)
 */
enum _Ttypes {
    TY_UNDEF            =-1,                    /* special */

/*--export--enumTYPE--*/
    TY_IMPLICIT         =0,
    TY_INT              =1,
    TY_CHAR             =2,
    TY_SHORT            =3,
    TY_VOID             =4,
    TY_LONG             =5,
    TY_FLOAT            =6,
    TY_DOUBLE           =7,

    TY_SIGNED           =8,
    TY_UNSIGNED         =9,

    TY_BOOLEAN          =10,
    TY_LONGCHAR         =11,
    TY_LONGLONG         =12,
    TY_LONGDOUBLE       =13,

    TY_STRUCT           =14,                    /* struct definition */
    TY_UNION            =15,                    /* union definition */
    TY_ENUM             =16,                    /* enum definition */
    TY_ENUMCONST        =17,                    /* enumeration values */

    TY_STRING           =18,
    TY_DECLARE          =19,
    TY_LIST             =20,
    TY_ARRAY            =21,
    TY_HASH             =22,

    TY_STRUCTI          =23,                    /* struct instance/declaration */
    TY_UNIONI           =24,                    /* union instance/declaration */
    TY_ENUMI            =25,

/*--end--*/
    TY_MAX              =TY_ENUMI,
};


/*
 *  Storage class specifiers values (max=16)
 */
enum _Tstore {
/*--export--enumTYPE--*/
    SC_AUTO             =0x0,                   /* implicit */
    SC_REGISTER         =0x1,
    SC_STATIC           =0x2,
    SC_EXTERN           =0x3,
    SC_TYPEDEF          =0x4,
    SC_LOCAL            =0x5,
    SC_REPLACEMENT      =0x6,

    SC_PARAM            =0xa,
    SC_PARAMREG         =0xb,
/*--end--*/
    SC_MAX              =SC_PARAMREG
};


/*
 *  Type qualifiers bits (max=4)
 */
enum _Tqualif {
    TQ_CONST            =0x1,
    TQ_VOLATILE         =0x2,
    TQ_RESTRICT         =0x4,

    TQ_MAX              =TQ_RESTRICT
};


/*
 *  Function modifiers bits (max=4)
 */
enum _Fmodifiers {
    FM_INLINE           =0x1,
    FM_CDECL            =0x2,
    FM_FORTRAN          =0x4,
    FM_PASCAL           =0x8,
    FM_MAX              =FM_PASCAL
};


/*
 *  Type modifiers bits (max=4)
 */
enum _Tmodifiers {
    TM_FUNCTION         =0x1,
    TM_POINTER          =0x2,
    TM_OPTIONAL         =0x4,                   /* (~ type) notional */
    TM_REFERENCE        =0x8,
    TM_MAX              =TM_REFERENCE
};


enum _Torg {
/*--export--enumKEYWORD--*/
    TO_NOOP             =0x40,                  /* No-op for tree joining */
    TO_INIT             =0x41,                  /* Declarator + initialiser */
    TO_PTR              =0x42,                  /* Pointer to declarator */
    TO_REF              =0x43,                  /* Reference declarator */
    TO_LIST             =0x44,                  /* Links declarations together */
    TO_SYMBOL           =0x45,                  /* left link points to symbol table */
    TO_ARRAY            =0x46,                  /* Array indexing */
    TO_FUNC             =0x47                   /* Function */
/*--end--*/
};


/*
 *  Enumeration and YYSTYLE
 *
 *      MYYSTYPE *must* match the construct within cry.y; yet shall is no longer be needed under bison 2.x
 */
#include "crntypes.h"                           /* Node type */

#if !defined(YYSTYPE) && !defined(YYSTYPE_IS_DECLARED)
typedef union MYYYSTYPE {
    const char *        sval;                   /* String or symbol buffer */
    char *              xval;                   /* Allocated string or symbol buffer */
    struct node_leaf *  nval;                   /* Node (parser return) */
    struct func_t *     func;                   /* Function */
    struct symbol *     sym;                    /* Symbol */
    Head_p              arglist;                /* Argument list */
    accfloat_t          fval;                   /* float */
    accint_t            ival;                   /* Numeric */
    enum crntypes       eval;                   /* Enumeration for debugger usage */
} MYYYSTYPE;

extern MYYYSTYPE        yylval;
#endif  /*!YYSTYPE && !YYSTYPE_IS_DECLARED*/

#ifndef DO_NODE_MAGIC
#define DO_NODE_MAGIC       2
#endif

#ifdef DO_NODE_MAGIC
#define NODE_MAGIC          0x4e6f6465          /* Node */
#define SYMBOL_MAGIC        0x53796d62          /* Symb */
#endif

#if defined(NODE_MAGIC)
#define D_NODE_MAGIC(x)     x
#else
#define D_NODE_MAGIC(x)
#endif

#if defined(SYMBOL_MAGIC)
#define D_SYMBOL_MAGIC(x)   x
#else
#define D_SYMBOL_MAGIC(x)
#endif

#define IDENT_LEVEL         4
#define IDENT_LINELENGTH    132

typedef struct node_leaf {
    D_NODE_MAGIC(uint32_t magic;)               /* Structure magic */
    struct node_leaf *  left;                   /* Node specific association */
    struct node_leaf *  right;                  /* Node specific association */
    struct node_leaf *  next;                   /* Linked list of allocated nodes */
    int                 type;                   /* Node type (see enum node_types) */
#if defined(YYSTYPE_IS_DECLARED)
    YYSTYPE             atom;                   /* Value */
#else
    MYYYSTYPE           atom;                   /* Value */
#endif
    struct symbol *     sym;                    /* Symbol definition (if any) */
    int                 lineno;                 /* Needed if '-g' specified */
} node_t;


/*
 *  Function control structure
 */
typedef struct func_t {
    symtype_t           f_type;
    const char *        f_name;
    node_t *            f_defn;
    node_t *            f_body;
} func_t;

/*
 *  Switch control structure
 */
typedef struct switch_t {
    Head_p sw_cases;                            /* Pending cases */
    Head_p sw_stmts;                            /* Pending stmts in current case */
    node_t *sw_expr;                            /* Expression node */
    symtype_t sw_type;                          /* Base type of the expression */
    unsigned sw_mixed;                          /* mixed type count */
} switch_t;

#define SF_FORWARD          0x0001              /* definition is a forward reference */
#define SF_DEFINING         0x0002              /* In middle of definition */
#define SF_REF              0x0004              /* Symbol was referenced */
#define SF_LOCAL            0x0008              /* Locally scoped (call/buffer etc) */
#define SF_FUNCTION         0x0010
#define SF_BODY             0x0020              /* Function prototype */
#define SF_INCOMPLETE       0x0040              /* Incomplete definition */

typedef TAILQ_HEAD(symqueue, symbol) symqueue_t;

typedef struct symbol {
    D_SYMBOL_MAGIC(uint32_t magic;)             /* Structure magic */
    TAILQ_ENTRY(symbol) s_node;                 /* Symbol queue, used for memory management */
    unsigned            s_references;           /* Total reference count, released when zero */
    const char *name;                           /* Symbol name */
    int                 s_level;                /* Block level so we can remove things as we come out of blocks */
    int                 s_register;             /* Register assignment */
    symtype_t           s_type;                 /* Data type of symbol */
    argtype_t *         s_arguments;            /* Function argument summary */
    struct symbol *     s_defn;                 /* Pointer to struct/union/enum definition */
                                                /*  ie.  'struct fred x', then s_defn points to 'fred' */
    struct symbol *     s_owner;                /* Pointer to owning structure etc */
    node_t *            s_tree;                 /* Tree describing type modifiers */
    Head_p              s_members;              /* List (in order) of members */
    unsigned short      s_flags;                /* SF_xxx flags */
    unsigned short      s_align;                /* Struct/union base alignment */
    symtype_t           s_enumtype;             /* Emumeration element base type */
    int                 s_size;                 /* Size of variable (type or type * elements) */
    int                 s_line_no;              /* Line where original defined */
    const char *        s_filename;             /* File in which originally defined */
} symbol_t;

typedef struct type_t {
    int                 t_type;
    int                 t_size;
} type_t;

typedef struct decl_t {
    symtype_t           d_type;                 /* type/qualifier/specifier etc */
    symbol_t *          d_symbol;               /* associated symbol */
    symbol_t *          d_struct_sp;            /* struct save/restore */
} decl_t;

typedef struct struct_t {
    symbol_t *          s_symbol;               /* sym tab entry for current structure */
    int                 s_level;                /* nesting level */
    symbol_t *          s_parent;               /* parent */
} struct_t;

/*
 *  Basic (built-in) types
 */
typedef enum {
    SYMPRIM_UNKNOWN = -1,
    SYMPRIM_INT,
    SYMPRIM_STRING,
    SYMPRIM_LIST,
    SYMPRIM_ARRAY,
    SYMPRIM_DECLARE,
    SYMPRIM_FLOAT,
    SYMPRIM_MAX
} symprim_t;

/*
 *  Order in which to insert entries into symbol table
 */
#define INSERT_AT_END       0
#define INSERT_AT_FRONT     1

/*
 *  Following used to keep track of whether we are in a loop or a awitch so that when
 *  we see a break we can figure out what it is applying to
 */
#define INSIDE_LOOP         1
#define INSIDE_SWITCH       2


typedef struct stat_t {     /* memory stats */
    uint32_t s_new_nodes;
    uint32_t s_free_nodes;
} stat_t;


typedef struct gen_t {      /* code generator interface */
    void (*g_macro)(void);
    void (*g_list)(void);
    void (*g_id)(const char *id);
    void (*g_sym)(const char *lit);
    void (*g_reg)(const char *name, int index);
    void (*g_end_list)(void);
    void (*g_int)(accint_t);
    void (*g_string)(const char *str);
    void (*g_float)(accfloat_t);
    void (*g_token)(int);
    void (*g_finish)(void);
    void (*g_null)(void);
} gen_t;


extern void             parser_init(void);
extern void             parser_close(void);

extern void             autoload_init(void);
extern int              autoload_open(const char *filename);
extern void             autoload_module(const char *source, time_t mtime);
extern void             autoload_push(const char *name);
extern int              autoload_export(int commit);
extern void             autoload_close(void);

extern void             init_lex(void);
extern void             init_binary(void);
extern void             init_ascii(void);
extern void             init_codegen(void);

typedef struct lexer {
    int               (*get)(struct lexer *);

    unsigned            l_flags;
#define LEX_FNOSALLOC           0x0001

    const char *        l_cursor;
    void *              l_user;
    int                 l_unidx;
    int                 l_unbuffer[4];
} lexer_t;

extern int              yylex(void);
extern int              yyparse(void);
extern int              yylexer(lexer_t *lexer, int expand_symbols);
extern const char *     yysymbol(const char *str, int length);

extern node_t *         node_alloc(int op);
extern void             nodes_free(void);

extern node_t *         node(int op, node_t *left, node_t *right);
extern node_t *         new_node(void);
extern node_t *         new_symbol(const char *name);
extern node_t *         new_symbol1(char *sym);
extern node_t *         new_string(char *str);
extern node_t *         new_number(accint_t ival);
extern node_t *         new_float(accfloat_t fval);
extern node_t *         new_type(symtype_t, node_t *np1, node_t *np2);
extern node_t *         new_arglist(Head_p);

extern node_t *         node_opt(int op, node_t *np1, node_t *np2);
extern node_t *         node_lvalue(int op, node_t *np1, node_t *np2);
extern symtype_t        node_typeof(const node_t *np);

extern int              node_print(const node_t *np, int size);
extern int              node_dprint(const node_t *np, int size);

extern int              list_print(const Head_p hd, int size);
extern int              list_dprint(const Head_p hd, int size);

enum {
    SYMLK_FUNDEF =          0x0001,
    SYMLK_FTAG =            0x0002,
    SYMLK_FBUILTIN =        0x0004
};

extern void             symtab_init(void);
extern void             symtab_close(void);
extern symbol_t *       sym_alloc(const char *name);
extern symbol_t *       sym_reference(symbol_t *sp);
extern void             sym_free(symbol_t *sp);
extern void             sym_check(symbol_t *sp);
extern symbol_t *       sym_add(const char *name, node_t *np, symtype_t type);
extern symbol_t *       sym_push(Head_p *hd, int order, const char *, node_t *np, symtype_t type, symbol_t *sp);
extern symbol_t *       sym_auto_function(const char *name, int argumentc, const argtype_t *arguments);
extern symbol_t *       sym_lookup(const char *name, unsigned flags);
extern int              sym_predefined(const char *name);
extern symtype_t        sym_typeof(const char *name, unsigned flags);
extern int              sym_print(symbol_t *sp, int level);
extern symbol_t *       sym_implied_function(const char *name, int argumentc, const argtype_t *arguments);

extern symtype_t        symtype_make(symtype_t ty1, symtype_t ty2);
extern symtype_t        symtype_sign(symtype_t type);
extern symtype_t        symtype_base(symtype_t type);
extern symtype_t        symtype_promote_int(symtype_t type, int withsign);
extern symtype_t        symtype_promote_long(symtype_t type, int withsign);
extern symtype_t        symtype_promote_double(symtype_t type, int withsign);

extern symtype_t        symtype_coalesce(symtype_t ntype, symtype_t ctype);
extern symtype_t        symtype_istag(symtype_t type);
extern symtype_t        symtype_isinstance(symtype_t type);
extern symprim_t        symtype_map(symtype_t type);
extern unsigned         symtype_arg(symtype_t type);
extern const char *     symtype_to_str(symtype_t type);
extern const char *     symtype_to_defn(symtype_t type);
extern const char *     symtype_to_desc(symtype_t type);

extern void             symtab_dump(void);
extern void             symtab_print(void);

extern void             list_push(Head_p *hd, node_t *np);
extern void             list_append(Head_p *hd, node_t *np);
extern void             list_free(Head_p *hd);

extern const char *     yymap(int);

extern int              compile_arglist(Head_p, int flag);
extern void             compile_func(symtype_t type, const char *function, Head_p arglist, node_t *np);
extern void             compile_list(Head_p, int level, int flag, int sepstmt);
extern void             compile_main(node_t *np);
extern void             compile_node(const node_t *np, int level);

extern int              function_start(symtype_t type, node_t *np);
extern int              function_arglist(const char *name, node_t *defn, int body);
extern func_t *         function_end(node_t *np);
extern void             function_return(int);

extern void             block_enter(void);
extern node_t *         block_exit(void);
extern node_t *         block_exit1(int *newscope);
extern void             block_pop(void);

extern symtype_t        typedef_type(const char *name);

extern symbol_t *       decl_add(node_t *np, symtype_t type);
extern void             decl_push(symtype_t type);
extern symtype_t        decl_peek(void);
extern void             decl_pop(void);

extern void             ident_push(char *ident);
extern const char *     ident_peek(void);
extern const char *     ident_peek2(void);
extern char *           ident_pop(void);

extern const char *     filename_cache(const char *filename);

extern symbol_t *       struct_start(char *name, symtype_t type);
extern symbol_t *       struct_end(void);
extern void             struct_lookup(const char *name, int);
extern void             struct_tag(const char *name, symtype_t type);
extern void             struct_member(node_t *np);

extern void             enum_enter(char *name);
extern int              enum_ivalue(accint_t ivalue);
extern int              enum_svalue(const char *svalue);
extern int              enum_implicited(accint_t*ivalue);
extern void             enum_add(node_t *np);
extern void             enum_exit(node_t *enumvalues);
extern symtype_t        enum_type(symbol_t *sp);
extern void             enum_tag(const char *name);

extern void             loop_enter(void);
extern void             loop_exit(void);

extern void             loop_or_switch_enter(int);
extern void             loop_or_switch_exit(void);

extern void             switch_start(node_t *np);
extern node_t *         switch_end(node_t *np);
extern void             case_start(node_t *np);
extern int              case_end(void);

void                    pragma_process(const char *line);

extern int              x_generate_ascii;

extern void             gena_macro(void);
extern void             gena_list(void);
extern void             gena_id(const char *id);
extern void             gena_sym(const char *lit);
extern void             gena_reg(const char *name, int index);
extern void             gena_end_list(void);
extern void             gena_int(accint_t);
extern void             gena_string(const char *str);
extern void             gena_float(accfloat_t);
extern void             gena_token(int);
extern void             gena_finish(void);
extern void             gena_null(void);

extern void             genb_macro(void);
extern void             genb_list(void);
extern void             genb_id(const char *id);
extern void             genb_sym(const char *lit);
extern void             genb_reg(const char *name, int index);
extern void             genb_end_list(void);
extern void             genb_int(accint_t);
extern void             genb_float(accfloat_t);
extern void             genb_string(const char *str);
extern void             genb_token(int);
extern void             genb_finish(void);
extern void             genb_null(void);

extern const char *     x_progname;

extern FILE *           x_bfp;                  /* binary output stream */
extern FILE *           x_afp;                  /* ASCII output stream */
extern FILE *           x_errfp;

extern const char *     x_filename;
extern const char *     x_filename2;
extern const char *     x_funcname;             /* current function name for error messages */

extern char *           yytext;
extern int              x_lineno;
extern int              x_columnno;

extern stat_t           x_stats;

extern int              xf_warnings;
extern int              xf_lexical_scope;
extern int              xf_debug;
extern int              xf_debugger;
extern int              xf_flush;
extern int              xf_grunch;
extern int              xf_struct;
extern int              xf_prototype;
extern int              xf_unused;
extern int              xf_verbose;

extern node_t *         x_maintree;

extern Head_p           hd_syms;
extern Head_p           hd_stmt, hd_init, hd_arglist;
extern Head_p           hd_switch, hd_case;
extern Head_p           hd_globals, hd_undef;
extern Head_p           hd_block, hd_struct;

extern int              x_decl_level;
extern int              x_block_level;
extern int              x_break_level;
extern int              x_continue_level;
extern int              x_switch_level;
extern int              x_struct_level;

extern int              major_version, minor_version, edit_version;

extern const char *     x_version;
extern const char *     x_copyright;
extern const char *     x_compiled;

#include <assert.h>

#ifndef assert
#define assert(x)
#endif

#endif /*GR_GRUNCH_H_INCLUDED*/

