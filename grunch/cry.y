/* -*- mode: c; indent-width: 4; -*- */
/* $Id: cry.y,v 1.35 2021/04/05 09:18:23 cvsuser Exp $
 * grunch/crunch grammer, extended c99
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

%{
#define YYDEBUG 1                               /* Enable debug support */
#define YYERROR_VERBOSE 1                       /* verbose syntax error diagnostics */

#include "grunch.h"
%}


/*control structure*/
%union {
  const char *sval;                             /* String/typedef */
  char *xval;                                   /* Allocated string/symbol */
  struct node_leaf *nval;                       /* Node */
  struct func_t *func;                          /* Function */
  struct symbol *sym;                           /* Symbol */
  Head_p arglist;                               /* Argument list */
  accfloat_t fval;                              /* Float */
  accint_t ival;                                /* Integer value */
  symtype_t tval;                               /* Type */
  enum crntypes eval;                           /* Enumeration for debugger usage */
  }


/*internal*/
%token <ival>   K_BLOCK K_LEXICALBLOCK          /* Used as a marker for {..} blocks for the
                                                 * compilation code only - not a syntactic token */
%token <ival>   K_CONSTRUCTORS                  /* local constructors */
%token <ival>   K_CAST                          /* cast */

/*keywords*/
%token <ival>   K_SWITCH K_CASE K_DEFAULT
%token <ival>   K_LIST K_ARRAY K_HASH K_INT K_STRING K_DECLARE K_GLOBAL K_FLOAT K_DOUBLE K_BOOL
%token <ival>   K_NOOP K_FUNCALL K_LVALUE K_DOTS K_GETPROPERTY K_SETPROPERTY
%token <ival>   K_STRUCT K_UNION
%token <ival>   K_CONST K_RESTRICT K_VOLATILE
%token <ival>   K_CHAR K_LONG K_SHORT K_UNSIGNED K_VOID K_SIGNED
%token <ival>   K_SIZEOF

%token <ival>   K_IF K_ELSE K_COND
%token <ival>   K_FOR K_FOREACH K_BREAK K_BREAKSW K_CONTINUE K_WHILE K_DO
%token <ival>   K_RETURN
%token <ival>   K_INITIALIZER K_ENUM K_ENUMVAL
%token <ival>   K_AUTO K_REGISTER K_STATIC K_EXTERN K_LOCAL K_TYPEDEF K_REPLACEMENT K_COMMAND
%token <ival>   K_INLINE K_CDECL K_FORTRAN K_PASCAL
%token <ival>   K_TRY K_CATCH K_FINALLY K_THROW
%token <ival>   K_BLESS

/*tokens*/
%token <ival>   O_OCURLY O_CCURLY O_OROUND O_CROUND O_COMMA O_SEMICOLON O_COLON O_LSHIFT O_RSHIFT
%token <ival>   O_QUESTION
%token <ival>   O_OSQUARE O_CSQUARE             /* [ and ] */
%token <ival>   O_DOT O_ARROW                   /* For C-like structure members */

%token <ival>   O_EQ O_PLUS_EQ O_MINUS_EQ O_MUL_EQ O_DIV_EQ O_MOD_EQ O_AND_EQ O_OR_EQ O_XOR_EQ
%token <ival>   O_LSHIFT_EQ O_RSHIFT_EQ
%token <ival>   O_EQ_OP O_NE_OP O_GT_OP O_GE_OP O_LT_OP O_LE_OP
%token <ival>   O_PLUS O_MINUS O_MUL O_DIV O_MOD O_OR O_AND O_XOR O_NOT O_COMPLEMENT
%token <ival>   O_PLUS_PLUS O_MINUS_MINUS O_POST_PLUS_PLUS O_POST_MINUS_MINUS
%token <ival>   O_CAND O_COR

%token <ival>   O_INTEGER_CONST
%token <xval>   O_STRING_CONST
%token <fval>   O_FLOAT_CONST O_DOUBLE_CONST O_LONG_DOUBLE_CONST
%token <xval>   O_SYMBOL
%token <sval>   O_TYPEDEF_NAME

/*expressions*/
%type <nval>    primary_expression
%type <nval>    postfix_expression
%type <nval>    argument_expression_list
%type <nval>    unary_expression
%type <ival>    unary_operator
%type <nval>    cast_expression
%type <ival>    cast_type_name_primitive
%type <nval>    multiplicative_expression
%type <nval>    additive_expression
%type <nval>    shift_expression
%type <nval>    relational_expression
%type <nval>    equality_expression
%type <nval>    AND_expression
%type <nval>    exclusive_OR_expression
%type <nval>    inclusive_OR_expression
%type <nval>    logical_AND_expression
%type <nval>    logical_OR_expression
%type <nval>    conditional_expression
%type <nval>    assignment_expression
%type <ival>    assignment_operator
%type <nval>    expression
%type <nval>    optional_expression
%type <nval>    constant_expression

/*declarations*/
%type <nval>    declaration
%type <nval>    declaration_specifiers
%type <tval>    decl_specs
%type <nval>    init_declarator_list init_declarator
%type <tval>    storage_class_specifier
%type <tval>    type_specifier
%type <tval>    struct_or_union_specifier
%type <tval>    struct_or_union
%type <nval>    struct_declaration_list
%type <nval>    struct_declaration
%type <tval>    specifier_qualifier_list
%type <nval>    struct_declarator_list
%type <nval>    struct_declarator
%type <tval>    enum_specifier
%type <nval>    enumerator_decl
%type <nval>    enumerator_list
%type <ival>    enumerator_trailing
%type <nval>    enumerator
%type <tval>    type_qualifier
%type <tval>    function_specifier
%type <nval>    declarator
%type <nval>    direct_declarator
%type <nval>    pointer
%type <nval>    pointer_decl
%type <tval>    type_qualifier_list
%type <nval>    parameter_type_list
%type <nval>    parameter_list
%type <nval>    parameter_declaration
%type <tval>    optional_operator
%type <nval>    parameter_default
%type <nval>    identifier_list
//    %type <nval>    type_name
//    %type <nval>    abstract_declarator
//    %type <nval>    direct_abstract_declarator
//    %type <nval>    typedef_name
%type <nval>    initializer
%type <nval>    initializer_list
//    %type <nval>    designation
//    %type <nval>    designator_list
//    %type <nval>    designator

/*statements*/
%type <nval>    statement
%type <nval>    try_statement
%type <ival>    catch_list
%type <ival>    catch_statement
%type <nval>    catch_expression
%type <nval>    finally_statement
%type <nval>    labeled_statement
%type <nval>    compound_statement
%type <nval>    block_item_list
%type <nval>    block_item
%type <nval>    expression_statement
%type <nval>    simple_if selection_statement
%type <nval>    iteration_statement
%type <nval>    jump_statement

/*external*/
//    %type <nval>    program
//    %type <nval>    translation_unit
//    %type <nval>    external_declaration
%type <func>    function_definition
//    %type <nval>    declaration_list

%left           O_COMMA
%right          O_EQ O_PLUS_EQ O_MINUS_EQ O_MUL_EQ O_DIV_EQ O_MOD_EQ O_AND_EQ O_OR_EQ O_XOR_EQ O_LSHIFT_EQ O_RSHIFT_EQ
%right          O_QUESTION O_COLON
%left           O_COR
%left           O_CAND
%left           O_OR
%left           O_XOR
%left           O_AND
%left           O_EQ_OP O_NE_OP
%left           O_LE_OP O_GE_OP O_LT_OP O_GT_OP O_CMP_OP
%left           O_LSHIFT O_RSHIFT
%left           O_PLUS O_MINUS
%left           O_MUL O_DIV O_MOD
%right          O_PLUS_PLUS O_MINUS_MINUS O_COMPLEMENT O_NOT UMINUS UPLUS
%left           O_OROUND O_CROUND O_OSQUARE O_CSQUARE O_DOT O_ARROW

%nonassoc       K_IF
%nonassoc       K_ELSE
%{


#define LOOP_STACK 128                          /* Level of loops/switches nesting */

static int      loop_stack[LOOP_STACK];
static int      loop_top = 0;

static unsigned unnamedcnt = 0;                 /* unnamed/anon struct/union/enum counter */

const char *    x_funcname;                     /* current function name for error messages */


/*
 *  The following function keeps track of whether we are in a switch or a loop construct
 *  so that when we see a break we know whether to generate code for it or not.
 */
void
loop_or_switch_enter(int type)
{
    if (loop_top >= LOOP_STACK) {
        crerror(RC_ERROR, "maximum loop/switch nesting exceeded");
    } else {
        loop_stack[loop_top] = type;
    }
    ++loop_top;
}


/*
 *  Exit from a loop or switch construct
 */
void
loop_or_switch_exit(void)
{
    assert(loop_top > 0);
    --loop_top;
}


static int /* Also See: compile_arglist() and predefined_symbol() */
constant_symbol(const node_t *node)
{
    if (node_symbol == node->type) {
        return sym_predefined(node->atom.sval);
    }
    return 0;
}


%}
%start program
%%


/* Expressions.
 *
 *  (6.5.1)     primary-expression:
 *                      identifier
 *                      constant
 *                      string-literal
 *                      ( expression )
 *
 *  (6.5.2)     postfix-expression:
 *                      primary-expression
 *                      postfix-expression [ expression ]
 *                      postfix-expression ( [argument-expression-list] )
 *                      postfix-expression . identifier
 *                      postfix-expression -> identifier
 *                      postfix-expression ++
 *                      postfix-expression --
 *                      ( type-name ) { initializer-list }
 *                      ( type-name ) { initializer-list , }
 *
 *  (6.5.2)     argument-expression-list:
 *                      assignment-expression
 *                      argument-expression-list , assignment-expression
 *
 *  (6.5.3)     unary-expression:
 *                      postfix-expression
 *                      ++ unary-expression
 *                      -- unary-expression
 *                      unary-operator cast-expression
 *                      sizeof unary-expression
 *                      sizeof ( type-name )
 *
 *  (6.5.3)     unary-operator: one of
 *                      & * + -  !
 *
 *  (6.5.4)     cast-expression:
 *                      unary-expression
 *                      ( type-name ) cast-expression
 *
 *  (6.5.5)     multiplicative-expression:
 *                      cast-expression
 *                      multiplicative-expression * cast-expression
 *                      multiplicative-expression / cast-expression
 *                      multiplicative-expression % cast-expression
 *
 *  (6.5.6)     additive-expression:
 *                      multiplicative-expression
 *                      additive-expression + multiplicative-expression
 *                      additive-expression - multiplicative-expression
 *
 *  (6.5.7)     shift-expression:
 *                      additive-expression
 *                      shift-expression << additive-expression
 *                      shift-expression >> additive-expression
 *
 *  (6.5.8)     relational-expression:
 *                      shift-expression
 *                      relational-expression < shift-expression
 *                      relational-expression > shift-expression
 *                      relational-expression <= shift-expression
 *                      relational-expression >= shift-expression
 *
 *  (6.5.9)     equality-expression:
 *                      relational-expression
 *                      equality-expression == relational-expression
 *                      equality-expression != relational-expression
 *
 *  (6.5.10)    AND-expression:
 *                      equality-expression
 *                      AND-expression & equality-expression
 *
 *  (6.5.11)    exclusive-OR-expression:
 *                      AND-expression
 *                      exclusive-OR-expression AND-expression
 *
 *  (6.5.12)    inclusive-OR-expression:
 *                      exclusive-OR-expression
 *                      inclusive-OR-expression | exclusive-OR-expression
 *
 *  (6.5.13)    logical-AND-expression:
 *                      inclusive-OR-expression
 *                      logical-AND-expression && inclusive-OR-expression
 *
 *  (6.5.14)    logical-OR-expression:
 *                      logical-AND-expression
 *                      logical-OR-expression || logical-AND-expression
 *
 *  (6.5.15)    conditional-expression:
 *                      logical-OR-expression
 *                      logical-OR-expression ? expression : conditional-expression
 *
 *  (6.5.16)    assignment-expression:
 *                      conditional-expression
 *                      unary-expression assignment-operator assignment-expression
 *
 *  (6.5.16)    assignment-operator: one of
 *                      = *= /= %= += -= <<= >>= &9= = |=
 *
 *  (6.5.17)    expression:
 *                      assignment-expression
 *                      expression , assignment-expression
 *
 *  (6.6)       constant-expression:
 *                      conditional-expression
 */
primary_expression:
                  O_SYMBOL O_OROUND O_CROUND
                    { $$ = node(K_FUNCALL, new_symbol1($1), NULL); }
                | O_SYMBOL O_OROUND argument_expression_list O_CROUND
                    { $$ = node(K_FUNCALL, new_symbol1($1), $3); }
                | O_SYMBOL
                    {
                        symbol_t *sp = sym_lookup($1, 0);
                        int done = FALSE;

                        if (sp && (TY_ENUMCONST == (sp->s_type & TY_MASK))) {
                            /*
                             *  enumeration (13/05/09)
                             */
                            const node_t *np = sp->s_tree;

                            if (node_keywd == np->type) {
                                const node_t *right = np->right;

                                if (node_integer == right->type) {
                                    $$ = new_number(right->atom.ival);
                                    done = TRUE;

                                } else if (node_string == right->type) {
                                    $$ = new_string(chk_salloc(right->atom.sval));
                                    done = TRUE;

                                } else if (node_float == right->type) {
                                    $$ = new_float(right->atom.fval);
                                    done = TRUE;

                                }
                            }
                        }

                        if (! done) {
                            $$ = new_symbol1($1);
                            $$->sym = sym_reference(sp);
                        }
                    }
                | O_INTEGER_CONST
                    { $$ = new_number($1); }
                | O_FLOAT_CONST
                    { $$ = new_float($1); }
                | O_DOUBLE_CONST
                    { $$ = new_float($1); }
                | O_STRING_CONST
                    { $$ = new_string($1); }    /* 03/08/08 */
                | O_OROUND expression O_CROUND
                    { $$ = $2; }
                ;

postfix_expression:
                  primary_expression
                | postfix_expression O_OSQUARE expression O_CSQUARE
                    { $$ = node(O_OSQUARE, $1, $3); }
                | postfix_expression O_OROUND argument_expression_list O_CROUND
                    { $$ = node(K_FUNCALL, $1, $3); }
                | postfix_expression O_OROUND O_CROUND
                    { $$ = node(K_FUNCALL, $1, NULL); }
                | postfix_expression O_DOT O_SYMBOL     /* 27/07/08, get_property */
                    { $$ = node(K_GETPROPERTY, $1, new_string($3)); }
                | postfix_expression O_ARROW O_SYMBOL
                    { crerror(RC_UNSUPPORTED_POINTER, "pointer dereferencing not supported"); }
                | postfix_expression O_PLUS_PLUS
                    { $$ = node(O_POST_PLUS_PLUS, $1, NULL); }
                | postfix_expression O_MINUS_MINUS
                    { $$ = node(O_POST_MINUS_MINUS, $1, NULL); }
                ;

argument_expression_list:
                  assignment_expression
                | argument_expression_list O_COMMA assignment_expression
                    { $$ = node(K_NOOP, $1, $3); }
                ;

unary_expression:
                  postfix_expression
                    { $$ = $1; }
                | O_PLUS_PLUS unary_expression
                    { $$ = node(O_PLUS_PLUS, $2, NULL); }
                | O_MINUS_MINUS unary_expression
                    { $$ = node(O_MINUS_MINUS, $2, NULL); }
                | unary_operator cast_expression
                    {
                        const int op = $1;
                        node_t *np = NULL;

                        /*
                         *  Unary negative and positive.
                         */
                        if (O_MINUS == op || O_PLUS == op) {
                            if (NULL != (np = $2)) {
                                /*inline constant*/
                                if (O_MINUS == op) {/* - <value> */
                                    if (node_integer == np->type) {
                                        np->atom.ival = -np->atom.ival;
                                    } else if (node_float == np->type) {
                                        np->atom.fval = -np->atom.fval;
                                    } else {
                                        np = NULL;
                                    }

                                } else {            /* + <value> */
                                    if (node_integer == np->type || node_float == np->type) {
                                        $$ = np;
                                    } else {
                                        np = NULL;
                                    }
                                }

                                if (NULL == ($$ = np)) {
                                    $$ = np = node_opt(op, (node_t *) new_number(0), $2);
                                                    /* default (op 0 <value>) */
                                }
                            }
                        }

                        if (NULL == np) {
                            $$ = node_opt($1, $2, NULL);
                        }
                    }
                | K_SIZEOF unary_expression
                    { $$ = node(K_SIZEOF, $2, NULL); }
             /* | K_SIZEOF O_OROUND type_name O_CROUND */
                ;

unary_operator:
                  O_PLUS            { $$ = O_PLUS; }
                | O_MINUS           { $$ = O_MINUS; }
             /* | O_AND             { $$ = O_AND; } */
             /* | O_MUL             { $$ = O_MUL; } */
                | O_NOT             { $$ = O_NOT; }
                | O_COMPLEMENT      { $$ = O_COMPLEMENT; }
                ;

cast_expression:
                  unary_expression
                    { $$ = $1; }
             /* | O_OROUND type_name O_CROUND cast_expression */
                | O_OROUND cast_type_name_primitive O_CROUND cast_expression
                    { $$ = node(K_CAST, (node_t *) new_number($2), $4); }
                ;

cast_type_name_primitive:
                  K_CHAR            { $$ = TY_CHAR; }
                | K_SHORT           { $$ = TY_SHORT; }
                | K_INT             { $$ = TY_INT; }
                | K_LONG            { $$ = TY_LONG; }
                | K_FLOAT           { $$ = TY_FLOAT; }
                | K_DOUBLE          { $$ = TY_DOUBLE; }
                | K_SIGNED          { $$ = TY_SIGNED; }
                | K_UNSIGNED        { $$ = TY_UNSIGNED; }
                | K_BOOL            { $$ = TY_BOOLEAN; }
                ;

multiplicative_expression:
                  cast_expression
                | multiplicative_expression O_MUL cast_expression
                    { $$ = node_opt(O_MUL, $1, $3); }
                | multiplicative_expression O_DIV cast_expression
                    { $$ = node_opt(O_DIV, $1, $3); }
                | multiplicative_expression O_MOD cast_expression
                    { $$ = node_opt(O_MOD, $1, $3); }
                ;

additive_expression:
                  multiplicative_expression
                | additive_expression O_PLUS multiplicative_expression
                    { $$ = node_opt(O_PLUS, $1, $3); }
                | additive_expression O_MINUS multiplicative_expression
                    { $$ = node_opt(O_MINUS, $1, $3); }
                ;

shift_expression:
                  additive_expression
                | shift_expression O_LSHIFT additive_expression
                    { $$ = node(O_LSHIFT, $1, $3); }
                | shift_expression O_RSHIFT additive_expression
                    { $$ = node(O_RSHIFT, $1, $3); }
                ;

relational_expression:
                  shift_expression
                | relational_expression O_LT_OP shift_expression
                    { $$ = node(O_LT_OP, $1, $3); }
                | relational_expression O_GT_OP shift_expression
                    { $$ = node(O_GT_OP, $1, $3); }
                | relational_expression O_LE_OP shift_expression
                    { $$ = node(O_LE_OP, $1, $3); }
                | relational_expression O_GE_OP shift_expression
                    { $$ = node(O_GE_OP, $1, $3); }
                | relational_expression O_CMP_OP shift_expression
                    { $$ = node(O_CMP_OP, $1, $3); }
                ;

equality_expression:
                  relational_expression
                | equality_expression O_EQ_OP relational_expression
                    { $$ = node(O_EQ_OP, $1, $3); }
                | equality_expression O_NE_OP relational_expression
                    { $$ = node(O_NE_OP, $1, $3); }
                ;

AND_expression:
                  equality_expression
                | AND_expression O_AND equality_expression
                    { $$ = node_opt(O_AND, $1, $3); }
                ;

exclusive_OR_expression:
                  AND_expression
                | exclusive_OR_expression O_XOR AND_expression
                    { $$ = node_opt(O_XOR, $1, $3); }
                ;

inclusive_OR_expression:
                  exclusive_OR_expression
                | inclusive_OR_expression O_OR exclusive_OR_expression
                    { $$ = node_opt(O_OR, $1, $3); }
                ;

logical_AND_expression:
                  inclusive_OR_expression
                | logical_AND_expression O_CAND inclusive_OR_expression
                    { $$ = node(O_CAND, $1, $3); }
                ;

logical_OR_expression:
                  logical_AND_expression
                | logical_OR_expression O_COR logical_AND_expression
                    { $$ = node(O_COR, $1, $3); }
                ;

conditional_expression:
                  logical_OR_expression
                | logical_OR_expression O_QUESTION expression O_COLON conditional_expression
                    {
                        Head_p hdl = ll_init(), hdr = ll_init();

                        ll_push(hdl, (void *) $3);
                        ll_push(hdr, (void *) $5);
                        $$ = node(K_COND, $1, node(K_NOOP, (void *)hdl, (void *)hdr));
                    }
                ;

assignment_expression:
                  conditional_expression
                    { $$ = $1; }
                | unary_expression assignment_operator assignment_expression
                    { $$ = node_lvalue($2, $1, $3); }
                ;

assignment_operator:
                  O_EQ              { $$ = O_EQ; }
                | O_MUL_EQ          { $$ = O_MUL_EQ; }
                | O_DIV_EQ          { $$ = O_DIV_EQ; }
                | O_MOD_EQ          { $$ = O_MOD_EQ; }
                | O_PLUS_EQ         { $$ = O_PLUS_EQ; }
                | O_MINUS_EQ        { $$ = O_MINUS_EQ; }
                | O_LSHIFT_EQ       { $$ = O_LSHIFT_EQ; }
                | O_RSHIFT_EQ       { $$ = O_RSHIFT_EQ; }
                | O_AND_EQ          { $$ = O_AND_EQ; }
                | O_XOR_EQ          { $$ = O_XOR_EQ; }
                | O_OR_EQ           { $$ = O_OR_EQ; }
                ;

expression:
                  assignment_expression
                    { $$ = $1; }
                | expression O_COMMA assignment_expression
                    { $$ = node(K_NOOP, $1, $3); }
                ;

optional_expression:
                  /*empty*/
                    { $$ = NULL; }
                | expression
                ;

constant_expression:
                  conditional_expression
                ;

/* Declarations
 *
 *  (6.7)       declaration:
 *                      declaration-specifiers [init-declarator-list] ;
 *
 *  (6.7)       declaration-specifiers:
 *                      storage-class-specifier [declaration-specifiers]
 *                      type-specifier [declaration-specifiers]
 *                      type-qualifier [declaration-specifiers]
 *                      function-specifier [declaration-specifiers]
 *
 *  (6.7)       init-declarator-list:
 *                      init-declarator
 *                      init-declarator-list , init-declarator
 *
 *  (6.7)       init-declarator:
 *                      declarator
 *                      declarator = initializer
 *
 *  (6.7.1)     storage-class-specifier:
 *                      typedef
 *                      extern
 *                      static
 *                      auto
 *                      register
 *
 *  (6.7.2)     type-specifier:
 *                      void
 *                      char
 *                      short
 *                      int
 *                      long
 *                      float
 *                      double
 *                      signed
 *                      unsigned
 *                      _Bool
 *                      _Complex
 *                      _Imaginary
 *                      struct-or-union-specifier
 *                      enum-specifier
 *                      typedef-name
 *
 *  (6.7.2.1)   struct-or-union-specifier:
 *                      struct-or-union [identifier] { struct-declaration-list }
 *                      struct-or-union identifier
 *
 *  (6.7.2.1)   struct-or-union:
 *                      struct
 *                      union
 *
 *  (6.7.2.1)   struct-declaration-list:
 *                      struct-declaration
 *                      struct-declaration-list struct-declaration
 *
 *  (6.7.2.1)   struct-declaration:
 *                      specifier-qualifier-list struct-declarator-list ;
 *
 *  (6.7.2.1)   specifier-qualifier-list:
 *                      type-specifier [specifier-qualifier-list]
 *                      type-qualifier [specifier-qualifier-list]
 *
 *  (6.7.2.1)   struct-declarator-list:
 *                      struct-declarator
 *                      struct-declarator-list , struct-declarator
 *
 *  (6.7.2.1)   struct-declarator:
 *                      declarator
 *                      [declarator] : constant-expression
 *
 *  (6.7.2.2)   enum-specifier:
 *                      enum [identifier] { enumerator-list }
 *                      enum [identifier] { enumerator-list , }
 *                      enum identifier
 *
 *  (6.7.2.2)   enumerator-list:
 *                      enumerator
 *                      enumerator-list , enumerator
 *
 *  (6.7.2.2)   enumerator:
 *                      enumeration-constant
 *                      enumeration-constant = constant-expression
 *
 *  (6.7.3)     type-qualifier:
 *                      const
 *                      restrict
 *                      volatile
 *
 *  (6.7.4)     function-specifier:
 *                      inline
 *
 *  (6.7.5)     declarator:
 *                      [pointer] direct-declarator
 *
 *  (6.7.5)     direct-declarator:
 *                      identifier
 *                      ( declarator )
 *                      direct-declarator [ [type-qualifier-list] assignment-expressionopt ]
 *                      direct-declarator [ static [type-qualifier-list] assignment-expression ]
 *                      direct-declarator [ type-qualifier-list static assignment-expression ]
 *                      direct-declarator [ [type-qualifier-list] * ]
 *                      direct-declarator ( parameter-type-list )
 *                      direct-declarator ( [identifier-list] )
 *
 *  (6.7.5)     pointer:
 *                      * [type-qualifier-list]
 *                      * [type-qualifier-list] pointer
 *
 *  (6.7.5)     type-qualifier-list:
 *                      type-qualifier
 *                      type-qualifier-list type-qualifier
 *
 *  (6.7.5)     parameter-type-list:
 *                      parameter-list
 *                      parameter-list , ...
 *                      ...                          (brief)
 *
 *  (6.7.5)     parameter-list:
 *                      parameter-declaration
 *                      parameter-list , parameter-declaration
 *
 *  (6.7.5)     parameter-declaration:
 *                      declaration-specifiers declarator
 *                      declaration-specifiers [abstract-declarator]
 *
 *  (6.7.5)     identifier-list:
 *                      identifier
 *                      identifier-list , identifier
 *
 *  (6.7.6)     type-name:
 *                      specifier-qualifier-list [abstract-declarator]
 *
 *  (6.7.6)     abstract-declarator:
 *                      pointer
 *                      [pointer] direct-abstract-declarator
 *
 *  (6.7.6)     direct-abstract-declarator:
 *                      ( abstract-declarator )
 *                      [direct-abstract-declarator] [ assignment-expressionopt ]
 *                      [direct-abstract-declarator] [ * ]
 *                      [direct-abstract-declarator] ( [parameter-type-list] )
 *
 *  (6.7.7)     typedef-name:
 *                      identifier
 *
 *  (6.7.8)     initializer:
 *                      assignment-expression
 *                      { initializer-list }
 *                      { initializer-list , }
 *
 *  (6.7.8)     initializer-list:
 *                      [designation] initializer
 *                      initializer-list , [designation] initializer
 *
 *  (6.7.8)     designation:
 *                      designator-list =
 *
 *  (6.7.8)     designator-list:
 *                      designator
 *                      designator-list designator
 *
 *  (6.7.8)     designator:
 *                      [ constant-expression ]
 *                      . identifier
 */
declaration:
                  declaration_specifiers init_declarator_list O_SEMICOLON
                    {
                        decl_pop();
                    }
                | declaration_specifiers O_SEMICOLON
                    {
                        $$ = $1;
                        decl_pop();
                    }
                | error O_SEMICOLON { $$ = NULL; }
                | error O_CCURLY    { $$ = NULL; }
                ;

declaration_specifiers:
                  decl_specs
                    { decl_push($1); }
                ;

decl_specs:
                  storage_class_specifier
                    { $$ = $1; }
                | storage_class_specifier decl_specs
                    {
                        const symtype_t sc1 = ($1 & SC_MASK) >> SC_SHIFT;
                        const symtype_t sc2 = ($2 & SC_MASK) >> SC_SHIFT;

                        if (sc1 && sc2) {
                            crerror(RC_TYPE_STORAGEDUP, "more then one storage class specified");
                        }
                        $$ = $2 | $1;
                    }
                | type_specifier
                    { $$ = $1; }
                | type_specifier decl_specs
                    { $$ = symtype_coalesce($1, $2); }
                | type_qualifier
                    { $$ = $1; }
                | type_qualifier decl_specs
                    {
                        if (($1 & TQ_MASK) & ($2 & TQ_MASK)) {
                            crerror(RC_TYPE_QUALDUP, "type qualifier specified more then once");
                        }
                        $$ = $2 | $1;
                    }
                | function_specifier
                    { $$ = $1; }
                | function_specifier decl_specs
                    {
                        if ($2 & FM_MASK) {
                            crerror(RC_TYPE_MODDUP, "more then one function modifier specified");
                        }
                        $$ = $2 | $1;
                    }
            /*  | K_DECLSPEC  '(' declspec_list      ')' decl_spec */
            /*  | K_ATTRIBUTE '(' attributespec_list ')' decl_spec */
                ;

init_declarator_list:
                  init_declarator
                    {
                        node_t *np = $1;

                        decl_add(np, 0);
                        if (1 == x_block_level) {
                            block_pop();        /* remove symbols */
                        }
                    }
                | init_declarator_list O_COMMA init_declarator
                    {
                        node_t *np = $3;

                        decl_add(np, 0);
                        if (1 == x_block_level) {
                            block_pop();        /* remove symbols */
                        }
                    }
                ;

init_declarator:
                  declarator
                    { $$ = $1; }
                | declarator O_EQ initializer
                    {
                        node_t *np = node(O_EQ, new_symbol(ident_peek()), (node_t *) $3);

                        if (0 == x_block_level) {
                            /* global symbol */
                            ll_push(hd_globals, (void *) np);

                        } else {                /* c99 style declarations, 12/08/09 */
                            if (NULL == hd_stmt || NULL == ll_first(hd_stmt)) {
                                /* append to initialisation list */
                                list_append(&hd_init, np);

                            } else {
                                /* otherwise within block, append to statements */
                                list_append(&hd_stmt, np);
                            }
                        }
                    }
                ;

storage_class_specifier:
                  K_AUTO            { $$ = (SC_AUTO)        << SC_SHIFT; }
                | K_REGISTER        { $$ = (SC_REGISTER)    << SC_SHIFT; }
                | K_STATIC          { $$ = (SC_STATIC)      << SC_SHIFT; }
                | K_EXTERN          { $$ = (SC_EXTERN)      << SC_SHIFT; }
                | K_LOCAL           { $$ = (SC_LOCAL)       << SC_SHIFT; }
                | K_TYPEDEF         { $$ = (SC_TYPEDEF)     << SC_SHIFT; }
                | K_REPLACEMENT     { $$ = (SC_REPLACEMENT) << SC_SHIFT; }
                | K_COMMAND         { /*$$ = (SC_COMMAND) << SC_SHIFT;*/ }
                ;

type_specifier:
                  K_VOID            { $$ = TY_VOID; }
                | K_CHAR            { $$ = TY_CHAR; }
                | K_SHORT           { $$ = TY_SHORT; }
                | K_INT             { $$ = TY_INT; }
                | K_LONG            { $$ = TY_LONG; }
                | K_FLOAT           { $$ = TY_FLOAT; }
                | K_DOUBLE          { $$ = TY_DOUBLE; }
                | K_SIGNED          { $$ = TY_SIGNED; }
                | K_UNSIGNED        { $$ = TY_UNSIGNED; }
                | K_STRING          { $$ = TY_STRING; }
                | K_LIST            { $$ = TY_LIST; }
                | K_ARRAY           { $$ = TY_ARRAY; }
                | K_HASH            { $$ = TY_HASH; }
                | K_DECLARE         { $$ = TY_DECLARE; }
                | K_BOOL            { $$ = TY_BOOLEAN; }
             /* | K_COMPLEX         { $$ = TY_COMPLEX; }    */
             /* | K_IMAGINARY       { $$ = TY_IMAGINARY; }  */
                | struct_or_union_specifier
                    {
                        assert(TY_STRUCT == $1 || TY_UNION == $1);
                        $$ = ($1 == TY_STRUCT ? TY_STRUCTI : TY_UNIONI);
                    }
                | enum_specifier
                    {
                        assert(TY_ENUM == $1);
                        $$ = TY_ENUMI;
                    }
                | O_TYPEDEF_NAME
                    { $$ = typedef_type($1); }
                ;

struct_or_union_specifier:
                  struct_or_union O_SYMBOL
                    {                           /* named struct/union */
                        struct_start($2, $1);
                        chk_free($2);
                    }
                  O_OCURLY struct_declaration_list O_CCURLY
                    {
                        struct_end();
                        $$ = $1;
                    }
                | struct_or_union
                    {                           /* unnamed struct/union */
                        char unnamed[32];

                        sprintf(unnamed, "*unnamed_%u*", ++unnamedcnt);
                        struct_start(unnamed, $1);
                    }
                  O_OCURLY struct_declaration_list O_CCURLY
                    {
                        struct_end();
                        $$ = $1;
                    }
                | struct_or_union O_SYMBOL
                    {
                        struct_tag($2, $1);
                        chk_free($2);
                        $$ = $1;
                    }
                ;

struct_or_union:
                  K_STRUCT          { $$ = TY_STRUCT; }
                | K_UNION           { $$ = TY_UNION; }
                ;

struct_declaration_list:
                  struct_declaration
                | struct_declaration_list struct_declaration
                ;

struct_declaration:
                specifier_qualifier_list
                  { decl_push($1); }
                struct_declarator_list O_SEMICOLON
                  { decl_pop(); }
                ;

specifier_qualifier_list:
                  type_specifier specifier_qualifier_list
                    { $$ = $1 | ($2 & ((symtype_t)~TY_MASK)) | (($2 & TY_MASK) << TY_SHIFT); }
                | type_specifier
                    { $$ = $1; }
                | type_qualifier specifier_qualifier_list
                    { $$ = ($1 << TQ_SHIFT) | $2; }
                | type_qualifier
                    { $$ = $1; }
                ;

struct_declarator_list:
                  struct_declarator
                    { struct_member($1); }
                | struct_declarator_list O_COMMA struct_declarator
                    { struct_member($3); }
                ;

struct_declarator:
                  declarator
                    { $$ = $1; }
//              | declarator O_COLON constant_expression
//                  { $$ = $1; }
//              | O_COLON constant_expression
//                  {
//                    static int bit_field = 1;
//                    char buf[64];
//
//                    sprintf(buf, "<bit-field-%d>", bit_field++);
//                    ident_push(buf);
//                    $$ = NULL;
//                  }
                ;

enum_specifier:
                  K_ENUM O_SYMBOL O_OCURLY
                    {                           /* named enumeration */
                        enum_enter($2);
                    }
                  enumerator_decl O_CCURLY
                    {
                        enum_exit($5);
                        $$ = TY_ENUM;
                    }
                | K_ENUM O_OCURLY
                    {                           /* unnamed enumeration */
                        char unnamed[32];

                        sprintf(unnamed, "*unnamed_%u*", ++unnamedcnt);
                        enum_enter(unnamed);
                    }
                  enumerator_decl O_CCURLY
                    {
                        enum_exit($4);
                        $$ = TY_ENUM;
                    }
                | K_ENUM O_SYMBOL
                    {
                        enum_tag($2);
                        $$ = TY_ENUM;
                    }
                ;

enumerator_decl:
                  enumerator_list enumerator_trailing
                    { $$ = $1; }
                ;

enumerator_list:
                  enumerator
                    { $$ = $1; }
                | enumerator_list O_COMMA enumerator
                    {
                        if ($3) {
                            $$ = node(K_ENUM, $1, $3);
                        } else {
                            $$ = $1;
                        }
                    }
                ;

enumerator_trailing:            /* allow optional trailing comma within enumerator_list's */
                  O_COMMA
                | /*empty*/         { $$ = 0; }
                ;

enumerator:
                  O_SYMBOL
                    {
                        accint_t ivalue = 0;

                        if (enum_implicited(&ivalue)) {
                            /*
                             *  integer implicit value
                             */
                            $$ = node(K_ENUMVAL, new_symbol1($1), new_number(ivalue));
                            enum_add($$);

                        } else {
                            /*
                             *  string explicit only
                             */
                            crerror(RC_ERROR, "expected a string literal");
                            $$ = NULL;
                        }
                    }
                | O_SYMBOL O_EQ constant_expression
                    {
                        if (node_integer == $3->type) {
                                                /* integer enumeration */
                            if (! enum_ivalue($3->atom.ival)) {
                                crerror(RC_ERROR, "mixed enumeration types");
                            }
                            $$ = node(K_ENUMVAL, new_symbol1($1), $3);
                            enum_add($$);

                        } else if (xf_grunch && node_string == $3->type) {
                                                /* string enumeration */
                            if (! enum_svalue($3->atom.sval)) {
                                crerror(RC_ERROR, "mixed enumeration types");
                            }
                            $$ = node(K_ENUMVAL, new_symbol1($1), $3);
                            enum_add($$);

                        } else {
                            crerror(RC_ERROR, "expected a constant expression");
                            $$ = NULL;
                        }
                    }
                ;

type_qualifier:
                  K_CONST           { $$ = (TQ_CONST)       << TQ_SHIFT; }
                | K_RESTRICT        { $$ = (TQ_RESTRICT)    << TQ_SHIFT; }
                | K_VOLATILE        { $$ = (TQ_VOLATILE)    << TQ_SHIFT; }
                ;

function_specifier:
                  K_INLINE          { $$ = (FM_INLINE)      << FM_SHIFT; }
                | K_CDECL           { $$ = (FM_CDECL)       << FM_SHIFT; }
                | K_FORTRAN         { $$ = (FM_FORTRAN)     << FM_SHIFT; }
                | K_PASCAL          { $$ = (FM_PASCAL)      << FM_SHIFT; }
                ;

declarator:
                  direct_declarator
                    { $$ = $1; }
                | pointer direct_declarator
                    { $$ = node(K_NOOP, $1, $2); }
                ;

direct_declarator:
                  O_SYMBOL
                    {
                      ident_push($1);
                      $$ = NULL;
                    }
                | O_OROUND declarator O_CROUND
                    { $$ = $2; }
                | direct_declarator O_OSQUARE constant_expression O_CSQUARE
                    { $$ = node(TO_ARRAY, $1, (node_t *) $3); }
                | direct_declarator O_OSQUARE O_CSQUARE
                    { $$ = node(TO_ARRAY, $1, NULL); }
                | direct_declarator O_OROUND
                    {
                        block_enter();
                    }
                  parameter_type_list O_CROUND
                    {
                        $$ = node(TO_FUNC, $1, (node_t *) new_arglist(hd_arglist));
                        hd_arglist = NULL;
                        if (x_block_level > 1) {
                            block_pop();
                        }
                    }
                | direct_declarator O_OROUND
                    { block_enter(); }
                  identifier_list O_CROUND
                    {
                        $$ = node(TO_FUNC, $1, (node_t *) new_arglist(hd_arglist));
                        hd_arglist = NULL;
                        if (x_block_level > 1) {
                            block_pop();
                        }
                    }
                | direct_declarator O_OROUND O_CROUND
                    { $$ = node(TO_FUNC, $1, NULL); }
                ;

pointer:
                  pointer_decl
                    {
                        node_t *np = $1;
                        symtype_t type = 0;

                        while (np) {
                            if (node_keywd == np->type) {
                                switch (np->atom.ival) {
                                case TO_PTR:
                                    if (0 == (type & TM_POINTER)) {
                                        if (xf_grunch) {
                                            crerror(RC_ERROR, "unexpected pointer declaration");
                                        }
                                    }
                                    type |= TM_POINTER;
                                    break;

                                case TO_REF:
                                    if (type & TM_REFERENCE) {
                                        crerror(RC_ERROR, "reference to reference is illegal");
                                    } else if ((type & TM_POINTER) && 0 == (type & TM_REFERENCE)) {
                                        crerror(RC_ERROR, "pointer to reference is illegal");
                                    }
                                    type |= TM_REFERENCE;
                                    break;
                                }
                            }
                            np = np->left;
                        }
                        $$ = $1;
                    }
                ;

pointer_decl:
                  O_MUL
                    { $$ = node(TO_PTR, NULL, NULL); }
                | O_MUL pointer_decl
                    { $$ = node(TO_PTR, $2, NULL); }
                | O_MUL type_qualifier_list
                    { $$ = node(TO_PTR, NULL, (node_t *)((size_t) $2)); }
                | O_MUL type_qualifier_list pointer_decl
                    { $$ = node(TO_PTR, $3, (node_t *)((size_t) $2)); }
                | O_AND
                    {
                        if (! xf_grunch) {
                            crerror(RC_ERROR, "unexpected '&'");
                        }
                        $$ = node(TO_REF, NULL, NULL);
                    }
                | O_AND pointer_decl
                    {
                        if (! xf_grunch) {
                            crerror(RC_ERROR, "unexpected '&'");
                        }
                        $$ = node(TO_REF, $2, NULL);
                    }
                ;

type_qualifier_list:
                  type_qualifier
                    { $$ = $1; }
                | type_qualifier_list type_qualifier
                    {
                        if (($1 & TQ_MASK) & ($2 & TQ_MASK))
                            crerror(RC_TYPE_QUALDUP, "type qualifier specified more then once");
                        $$ = $2 | $1;
                    }
                ;

parameter_type_list:
                  parameter_list
                    { $$ = $1; }
                | parameter_list O_COMMA K_DOTS
                    {                           /* 18/08/08 */
                        ll_append(hd_arglist, NULL);
                        $$ = $1;
                    }
                | K_DOTS
                    {
                        if (NULL == hd_arglist) /* 18/08/08 */
                            hd_arglist = ll_init();
                        ll_append(hd_arglist, NULL);

                        if (! xf_grunch) {      /* grunch language only */
                            crerror(RC_ERROR, "ellipsis must be preceded by at least one parameter type");
                        }
                        $$ = NULL;
                    }
                ;

parameter_list:
                  parameter_declaration
                    { $$ = $1; }
                | parameter_list O_COMMA parameter_declaration
                    { $$ = node(K_NOOP, $1, $3); }
                ;

parameter_declaration:
                  optional_operator declaration_specifiers
                    {
                        symtype_t type = decl_peek();
                        node_t *np, *sym = NULL;

                        if ($1 && ! xf_grunch)
                            crerror(RC_ERROR, "invalid parameter declaration");

                        type |= $1;             /* TM_OPTIONAL or 0 */

                        if (type & ((SC_STATIC|SC_REGISTER|SC_LOCAL) << SC_SHIFT)) {
                            crwarn(RC_ERROR, "parameter has bad storage class.");
                            type &= ~((SC_STATIC|SC_REGISTER|SC_LOCAL) << SC_SHIFT);
                        }

                        if (type & ((TQ_VOLATILE) << TQ_SHIFT)) {
                            crwarn(RC_ERROR, "parameter has bad qualifier.");
                            type &= ~((TQ_VOLATILE) << TQ_SHIFT);
                        }

                        np = new_type(type, NULL, sym);
                        list_append(&hd_arglist, np);
                        decl_pop();
                        $$ = NULL;
                    }
                | optional_operator declaration_specifiers declarator parameter_default
                    {
                        const char *name = ident_peek();
                        symtype_t type = decl_peek();
                        symbol_t *sp;
                        node_t *np, *sym;

                        if ($1 && ! xf_grunch) {
                            crerror(RC_ERROR, "invalid parameter declaration");
                        }

                        sym = new_symbol(name);
                        if ($4) {               /* default assignment, 11/10/08 */
                            sym->right = $4;
                            type |= (TM_OPTIONAL << TM_SHIFT);

                            if ($1 & (TM_OPTIONAL << TM_SHIFT)) {
                                crwarn(RC_ERROR,"'~' implied with default argument.");
                            }
                        }

                        type |= $1;             /* TM_OPTIONAL or 0 */

                        if (type & ((SC_STATIC|SC_REGISTER|SC_LOCAL) << SC_SHIFT)) {
                            crwarnx(RC_ERROR, "'%s' has bad storage class.", name);
                            type &= ~((SC_STATIC|SC_REGISTER|SC_LOCAL) << SC_SHIFT);
                        }

                        if (type & ((TQ_VOLATILE) << TQ_SHIFT)) {
                            crwarnx(RC_ERROR, "'%s' has bad qualifier.", name);
                            type &= ~((TQ_VOLATILE) << TQ_SHIFT);
                        }

                        np = new_type(type, $3, sym);
                        sp = decl_add(np, type);
                        sym->sym = sym_reference(sp);
                        list_append(&hd_arglist, np);
                        decl_pop();
                        $$ = NULL;
                    }
             /* | optional_operator declaration_specifiers abstract_declarator */
                ;

              /*
               *  leading tidle (~) indicates that the parameter is optional argument.
               */
optional_operator:
                  O_COMPLEMENT  { $$ = (TM_OPTIONAL << TM_SHIFT); }
                | /*empty*/     { $$ = 0; }
                ;

              /*
               *  trail [= expression] indicates that the parameter is an optional
               *  argument, if the default value of expression.
               */
parameter_default:
                  O_EQ constant_expression
                    {
                        if (node_integer == $2->type || node_string == $2->type || node_float == $2->type ||
                                 constant_symbol($2)) {
                            $$ = $2;
                        } else {
                            crerror(RC_ERROR, "expected a constant expression");
                            $$ = NULL;
                        }
                    }
                | /*empty*/
                    { $$ = 0; }
                ;

identifier_list:
                  O_SYMBOL
                    {
                        char *cp = $1;
                        node_t *np;

                        np = new_type((SC_PARAM << SC_SHIFT)|TY_IMPLICIT, NULL, new_symbol1(cp));
                        list_append(&hd_arglist, np);
                        sym_add((const char *)cp, np, TY_INT);
                        $$ = np;
                    }
                | identifier_list O_COMMA O_SYMBOL
                    {
                        char *cp = $3;
                        node_t *np;

                        np = new_type((SC_PARAM << SC_SHIFT)|TY_IMPLICIT, NULL, new_symbol1(cp));
                        ll_append(hd_arglist, (void *) np);
                        sym_add((const char *)cp, np, TY_INT);
                        $$ = node(K_NOOP, $1, np);
                    }
                ;

//type_name:
//                specifier_qualifier_list abstract_declarator
//              | specifier_qualifier_list
//              ;
//
//abstract_declarator:
//                pointer
//              | pointer direct_abstract_declarator
//              | direct_abstract_declarator
//              ;
//
//direct_abstract_declarator:
//                O_OROUND abstract_declarator O_CROUND
//              | direct_abstract_declarator O_OSQUARE constant_expression O_CSQUARE
//              | direct_abstract_declarator O_OSQUARE O_CSQUARE
//              | O_OSQUARE constant_expression O_CSQUARE
//              | O_OSQUARE O_CSQUARE
//              | direct_abstract_declarator O_OROUND constant_expression O_CROUND
//              | direct_abstract_declarator O_OROUND O_CROUND
//              | O_OROUND constant_expression O_CROUND
//              | O_OROUND O_CROUND
//              ;

initializer:
                  assignment_expression
                | O_OCURLY initializer_list O_CCURLY
                    { $$ = node(K_INITIALIZER, NULL, $2); }
                | O_OCURLY initializer_list O_COMMA O_CCURLY
                    { $$ = node(K_INITIALIZER, NULL, $2); }
                ;

initializer_list:
                  initializer
                | initializer_list O_COMMA initializer
                    { $$ = node(K_NOOP, $1, $3); }
//              | initializer_list O_COMMA designation initializer
                ;

//designation:                                  /* c99 */
//                designator_list =
//              ;
//
//designator_list:                              /* c99 */
//                designator
//              | designator_list designator
//              ;
//
//designator:                                   /* c99 */
//              | [ constant_expression ]
//              | . O_SYMBOL
//              ;


/* Statements.
 *
 *  (6.8)       statement:
 *                      labeled-statement
 *                      compound-statement
 *                      expression-statement
 *                      selection-statement
 *                      iteration-statement
 *                      jump-statement
 *
 *  (6.8.1)     labeled-statement:
 *                      identifier : statement
 *                      case constant-expression : statement
 *                      default : statement
 *
 *  (6.8.2)     compound-statement:
 *                      { [block-item-list] }
 *
 *  (6.8.2)     block-item-list:
 *                      block-item
 *                      block-item-list block-item
 *
 *  (6.8.2)     block-item:
 *                      declaration
 *                      statement
 *
 *  (6.8.3)     expression-statement:
 *                      [expression] ;
 *
 *  (6.8.4)     selection-statement:
 *                      if (( expression ) statement
 *                      if ( expression ) statement else statement
 *                      switch ( expression ) statement
 *
 *  (6.8.5)     iteration-statement:
 *                      while ( expression ) statement
 *                      do statement while ( expression ) ;
 *                      for ( [expression] ; [expression] ; [expression] ) statement
 *                      for ( declaration [expression] ; [expression] ) statement
 *
 *  (6.8.6)     jump-statement:
 *                      goto identifier ;
 *                      continue ;
 *                      break ;
 *                      return [expression] ;
 */
statement:
                  try_statement        { if ($1) list_append(&hd_stmt, $1); }
                | labeled_statement    { if ($1) list_append(&hd_stmt, $1); }
                | expression_statement { if ($1) list_append(&hd_stmt, $1); }
                | compound_statement   { if ($1) list_append(&hd_stmt, $1); }
                | selection_statement  { if ($1) list_append(&hd_stmt, $1); }
                | iteration_statement  { if ($1) list_append(&hd_stmt, $1); }
                | jump_statement       { if ($1) list_append(&hd_stmt, $1); }
                ;

try_statement:                                  /* try/catch/finally, 22/11/10 */
                  K_TRY
                    {
                        block_enter();
                    }
                  compound_statement catch_list finally_statement
                    {
                        $$ = node(K_TRY, $3, node(K_NOOP, (void *)hd_stmt, $5));
                        block_pop();
                    }
                ;

catch_list:                                     /* one or more catch */
                  catch_statement
                | catch_list catch_statement
                ;

catch_statement:                                /* non-optional catch */
                  K_CATCH O_OROUND
                    {
                        block_enter();
                    }
                  catch_expression O_CROUND compound_statement
                    {
                        block_pop();
                        list_append(&hd_stmt, node(K_CATCH, $4, $6));
                    }
                ;

catch_expression:                               /* catch or conditional-catch */
                  O_SYMBOL
                    {
                        sym_add($1, NULL, TY_DECLARE);
                        $$ = node(K_NOOP, new_symbol1($1), NULL);
                    }
                | O_SYMBOL
                    {
                        sym_add($1, NULL, TY_DECLARE);
                    }
                  K_IF O_OROUND expression O_CROUND
                    {
                        $$ = node(K_NOOP, new_symbol1($1), $5);
                    }
                ;

finally_statement:                              /* optional finally */
                  /*empty*/
                    { $$ = NULL; }
                | K_FINALLY compound_statement
                    { $$ = node(K_FINALLY, NULL, $2); }
                ;

labeled_statement:
                  O_SYMBOL O_COLON statement
                    {
                        $$ = $3;
                        chk_free($1);
                    }
                | K_CASE
                    {
                        if (0 == x_switch_level) {
                            crerror(RC_ERROR, "case not in switch statement");
                        }
                    }
                  expression O_COLON
                    {
                        case_start($3);
                    }
                  statement
                    {
                        $$ = NULL;
                    }
                | K_DEFAULT O_COLON
                    {
                        if (0 == x_switch_level) {
                            crerror(RC_ERROR, "'default' not in switch statement");
                        }
                        case_start(NULL);
                    }
                  statement
                    {
                        $$ = NULL;
                    }
                ;

compound_statement:
                  O_OCURLY O_CCURLY
                    {                           /* empty block */
                        $$ = node(K_BLOCK, NULL, NULL);
                    }
                | O_OCURLY
                    {
                        block_enter();
                    }
                  block_item_list O_CCURLY
                    {
                        int newscope = FALSE;
                        Head_p hd = hd_stmt;
                        node_t *np1;

                        np1 = block_exit1(&newscope);
                        if (NULL == hd) {
                            if (TRUE == newscope) {
                                list_append(&hd, np1);
                                $$ = node(K_LEXICALBLOCK, NULL, (node_t *) hd);
                            } else {
                                $$ = np1;
                            }
                        } else {
                            if (np1) ll_push(hd, (void *) np1);
                            $$ = node((TRUE == newscope ? K_LEXICALBLOCK : K_BLOCK), NULL, (node_t *) hd);
                        }
                    }
                ;

block_item_list:                                /* c99, 27/08/08 */
                  block_item
                | block_item_list block_item
                ;

block_item:                                     /* c99, 27/08/08 */
                  declaration
                | statement
                ;

expression_statement:
                  expression O_SEMICOLON
                    { $$ = $1; }
                | O_SEMICOLON
                    { $$ = NULL; }
                ;

simple_if:
                  K_IF O_OROUND expression O_CROUND
                    {
                        block_enter();
                    }
                  statement
                    {
                        node_t *np;

                        /*  Upon a null <true> statement force a no-op otherwise
                         *  the result shall generate an invalid if-then-else clause
                         *
                         *          if (xxx)
                         *              ;
                         *  and
                         *          if (xxx) {
                         *          }
                         */
                        if (NULL == hd_stmt || NULL == ll_first(hd_stmt) ||
                                NULL == (np = ll_elem(ll_first(hd_stmt)))) {
                            list_append(&hd_stmt, node(K_FUNCALL, new_symbol("nothing"), NULL));

                        } else {
                            if (np->type == node_keywd && np->atom.ival == K_BLOCK && NULL == np->right) {
                                ll_pop(hd_stmt);
                                list_append(&hd_stmt, node(K_FUNCALL, new_symbol("nothing"), NULL));
                            }
                        }
                        $$ = node(K_IF, $3, (node_t *) hd_stmt);
                        block_pop();
                    }
                ;

selection_statement:
                  simple_if K_ELSE
                    {
                        block_enter();
                    }
                  statement
                    {
                        $$ = node(K_ELSE, $1, (node_t *) hd_stmt);
                        block_pop();
                    }
                | simple_if
                    {
                        $$ = $1;
                    }
                | K_SWITCH O_OROUND expression O_CROUND
                    {
                        loop_or_switch_enter(INSIDE_SWITCH);
                        switch_start($3);
                    }
                  statement
                    {
                        loop_or_switch_exit();
                        $$ = switch_end($3);
                    }
                ;

iteration_statement:
                  K_WHILE O_OROUND optional_expression O_CROUND
                    { loop_enter(); }
                  statement
                    {
                        /*
                         *  while (<cond> = 1) <body>
                         */
                        if (NULL == $3) $3 = new_number(1);
                        $$ = node(K_WHILE, $3, (node_t *) hd_stmt);
                        loop_exit();
                    }
                | K_DO
                    { loop_enter(); }
                  statement
                    {
                        $3 = (node_t *) hd_stmt;
                        loop_exit();
                    }
                  K_WHILE O_OROUND expression O_CROUND O_SEMICOLON
                    {
                        /*
                         *  Convert do statements of the form:
                         *      do <stmts> while <expr>
                         *
                         *  to the following a while loop of the form:
                         *      while (1) <body> if (!(expr))break;
                         */
                        Head_p hd_body = (Head_p)$3;  /* statement */
                        Head_p hd_break = NULL;

                        list_push(&hd_break, node_alloc(K_BREAK));
                        list_append(&hd_body, node(K_IF, node(O_NOT, $7, NULL), (node_t *) hd_break));
                        $$ = node(K_WHILE, new_number(1), (node_t *) hd_body);
                    }
                | K_FOR O_OROUND optional_expression O_SEMICOLON
                            optional_expression O_SEMICOLON optional_expression O_CROUND
                    { loop_enter(); }
                  statement     /* hd_stmt */
                    {
                        /*
                         *  for (<init>; <expr> = 1; <post>) <body>
                         */
                        if (NULL == $5) $5 = new_number(1);
                        if (NULL == $3 && NULL == $7) {
                            $$ = node(K_WHILE, $5, (node_t *) hd_stmt);
                        } else {
                            $$ = node(K_FOR, $3, node(K_NOOP, $5, node(K_NOOP, $7, (node_t *) hd_stmt)));
                        }
                        loop_exit();
                    }
                /*
                 *  foreach(<expr>; <symbol> [; [<idx>]] )
                 */
                | K_FOREACH O_OROUND expression O_SEMICOLON O_SYMBOL O_CROUND
                    { loop_enter(); }
                  statement     /* hd_stmt */
                    {
                        $$ = node(K_FOREACH,
                                node(K_NOOP, $3, (node_t *) hd_stmt),
                                node(K_NOOP, new_symbol($5), NULL));
                        loop_exit();
                    }
                | K_FOREACH O_OROUND expression O_SEMICOLON O_SYMBOL O_SEMICOLON O_CROUND
                    { loop_enter(); }
                  statement     /* hd_stmt */
                    {
                        $$ = node(K_FOREACH,
                                node(K_NOOP, $3, (node_t *) hd_stmt),
                                node(K_NOOP, new_symbol($5), NULL));
                        loop_exit();
                    }
                | K_FOREACH O_OROUND expression O_SEMICOLON O_SYMBOL O_SEMICOLON O_SYMBOL O_CROUND
                    { loop_enter(); }
                  statement     /* hd_stmt */
                    {
                        $$ = node(K_FOREACH,
                                node(K_NOOP, $3, (node_t *) hd_stmt),
                                node(K_NOOP, new_symbol($5), new_symbol($7)));
                        loop_exit();
                    }
                ;

jump_statement:
                  K_BREAK O_SEMICOLON
                    {
                        int type = K_BREAKSW;   /* switch */

                        if (x_break_level <= 0) {
                            crerror(RC_ERROR, "'break' not inside loop/switch statement");

                        } else if (loop_top < LOOP_STACK) {
                            if (INSIDE_LOOP == loop_stack[loop_top-1]) {
                                type = K_BREAK; /* loop */

                            } else if (!hd_case) {  /* 06/04/10 */
                                crerror(RC_ERROR, "'break' outside of case statement");
                            }
                        }
                        $$ = node_alloc(type);
                    }
                | K_CONTINUE O_SEMICOLON
                    {
                        if (x_continue_level <= 0) {
                            crerror(RC_ERROR, "'continue' not inside loop statement");
                        }
                        $$ = node_alloc(K_CONTINUE);
                    }
                | K_RETURN optional_expression O_SEMICOLON
                    {
                        $$ = node(K_RETURN, $2, NULL);
                        function_return($2 ? TRUE : FALSE);
                    }
                | K_THROW expression O_SEMICOLON
                    {                           /* 22/11/10 */
                        $$ = node(K_THROW, $2, NULL);
                    }
                ;

/* External definitions.
 *
 *  (6.9)       translation-unit:
 *                      external-declaration
 *                      translation-unit external-declaration
 *
 *  (6.9)       external-declaration:
 *                      function-definition
 *                      declaration
 *
 *  (6.9.1)     function-definition:
 *                      declaration-specifiers declarator [declaration-list] compound-statement
 *
 *  (6.9.1)     declaration-list:
 *                      declaration
 *                      declaration-list declaration
 */
program:
                  /*empty*/
                    {
                        crerror(RC_ERROR, "empty source file");
                    }
                | /*non-empty*/
                    {
                        unnamedcnt = 0;
                        x_funcname = NULL;
                        loop_top = 0;
                    }
                  translation_unit
                ;

translation_unit:
                  external_declaration
                | translation_unit external_declaration
                ;

external_declaration:
                  function_definition
                    {
                        func_t *fp = $1;
                        const char *name = ident_pop();
                        Head_p arglist = NULL;
                        symtype_t type = ((symtype_t)fp->f_type) | (TM_FUNCTION << TM_SHIFT);

                      //assert(fp->f_body);     /* may occur if a syntax error */
                        if (fp->f_defn) {
                            if (fp->f_defn->right) {
                                assert(node_arglist == fp->f_defn->right->type);
                                arglist = fp->f_defn->right->atom.arglist;
                            }
                        }
                        sym_add(name, fp->f_defn, type);

                        if (0 == strcmp(name, "main")) {
                            if ((type & TY_MASK) != TY_VOID) {
                                crwarn(RC_MAIN_VOID, "'main' expected to be a 'void' function.");
                            }
                            x_maintree = fp->f_body;
                        } else {
                            if (0 == strcmp(name, "_init")) {
                                crerror(RC_INIT_RESERVED, "'_init' is a reserved function.");
                            }
                            compile_func(type, name, arglist, fp->f_body);
                        }
                        x_funcname = NULL;

                        chk_free(fp);
                        chk_free((void *)name);
                        list_free(&hd_undef);   /* reset undefined symbols */
                    }
                | declaration
                    {
                    }
                ;

function_definition:
                  declaration_specifiers declarator
                    {
                        function_start((symtype_t)$1, $2);
                        block_enter();
                        decl_pop();
                    }
                  compound_statement
                    {
                        block_exit();
                        if (x_block_level)
                            block_exit();
                        $$ = function_end($4);
                    }
                | declarator
                    {
                        function_start(0, $1);
                        block_enter();
                    }
                  compound_statement
                    {
                        block_exit();
                        if (x_block_level)
                            block_exit();
                        $$ = function_end($3);
                    }
//              | declaration_specifiers declarator
//                  {
//                    start_function($1, $2);
//                    block_enter();
//                    decl_pop();
//                  }
//               declaration_list compound_statement
//                  {
//                    block_exit();
//                    if (x_block_level)
//                      block_exit();
//                    $$ = function_end($5);
//                  }
//              | declarator
//                  {
//                    start_function(0, $1);
//                    block_enter();
//                  }
//                declaration_list compound_statement
//                  {
//                    block_exit();
//                    if (x_block_level)
//                      block_exit();
//                    $$ = function_end($4);
//                  }
                ;

//declaration_list:
//                declaration
//              | declaration_list declaration
//              ;

/*end*/
