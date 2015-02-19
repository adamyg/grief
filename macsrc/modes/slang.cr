/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: slang.cr,v 1.10 2014/10/22 02:34:35 ayoung Exp $
 * slang support mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "SLANG"


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
    syntax_token(SYNT_DELIMITER,    ",;:." );
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_$");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  Advanced syntax engine/
     *      used to built a DFA based lexer/parser, which is generally faster.
     */
    syntax_rule("^[ \t]*#", "quick:preprocessor");
    syntax_rule("%.*$", "spell,todo:comment");
    syntax_rule("[A-Za-z_\\$][A-Za-z_0-9\\$]*", "keyword:normal");
    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][\\+\\-]?[0-9]*)?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]*", "number");
    syntax_rule("\"([^\"\\\\]|\\\\.)*\"", "string");
    syntax_rule("\"([^\"\\\\]|\\\\.)*\\\\?$", "string");
    syntax_rule("'([^'\\\\]|\\\\.)*'", "string");
    syntax_rule("'([^'\\\\]|\\\\.)*\\\\?$", "string");
    syntax_rule("[ \t]+", "normal");
    syntax_rule("[\\(\\[{}\\]\\),;\\.\\?:]", "delimiter");
    syntax_rule("[%\\-\\+/&\\*=<>\\|!~\\^]", "operator");
    syntax_rule("!if", "keyword0");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY, "do,if,or", -2);
    define_keywords(SYNK_PRIMARY, "and,chs,for,mod,not,pop,shl,shr,sqr,xor", -3);
    define_keywords(SYNK_PRIMARY, "NULL,_for,case,else,exch,loop,mul2,sign", -4);
    define_keywords(SYNK_PRIMARY, "__tmp,break,using,while", -5);
    define_keywords(SYNK_PRIMARY, "define,orelse,public,return,static,struct,switch", -6);
    define_keywords(SYNK_PRIMARY, "andelse,foreach,forever,private,typedef", -7);
    define_keywords(SYNK_PRIMARY, "continue,variable", -8);
    define_keywords(SYNK_PRIMARY, "EXIT_BLOCK", -10);
    define_keywords(SYNK_PRIMARY, "ERROR_BLOCK", -11);
    define_keywords(SYNK_PRIMARY, "EXECUTE_ERROR_BLOCK", -19);
}


/*
 *  Modeline/packag support
 */
string
_slang_mode()
{
    return "slang";                             /* return package extension */
}


string
_slang_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}
