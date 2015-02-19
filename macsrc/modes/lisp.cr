/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: lisp.cr,v 1.8 2014/10/22 02:34:34 ayoung Exp $
 * LISP mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "LISP"


/*
 *  main ---
 *      Define 'lisp' mode syntax.
 */
void
main(void)
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,      "%");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([", ")]");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_$");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  Keywords
     */
    define_keywords(0, "eq,if,or", -2);
    define_keywords(0, "let,not", -3);
    define_keywords(0, "setq", -4);
    define_keywords(0, "defuNULL,progNULL,while", -5);
}


/*
 *  Modeline/packag support
 */
string
_lisp_mode()
{
    return "lisp";                              /* return package extension */
}


string
_lisp_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}
