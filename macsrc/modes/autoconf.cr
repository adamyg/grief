/* -*- mode: cr; indent-width: 4; -*-
 * $Id: autoconf.cr,v 1.10 2024/07/30 16:29:20 cvsuser Exp $
 * Autoconf support.
 *
 *
 */

#include "../grief.h"

#define MODENAME "autoconf"

void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine/
     *      used for basic line processing.
     */
    syntax_token(SYNT_COMMENT,	    "#");
    syntax_token(SYNT_COMMENT,	    "dnl");
    syntax_token(SYNT_QUOTE,	    "\\");
    syntax_token(SYNT_STRING,	    "\"");
    syntax_token(SYNT_LITERAL,	    "\'");
    syntax_token(SYNT_BRACKET,	    "([{", ")]}");
    syntax_token(SYNT_DELIMITER,    ",;:");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_KEYWORD,      "a-zA-Z_", "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,	    "-+.0-9_xa-fA-F");

    /*
     *  Options,
     *      Hilite whitespace
     *      Hilite (trailing) line-cont whitespace.
     */
    set_syntax_flags(SYNF_HILITE_WS|SYNF_HILITE_LINECONT);

    /*
     *  Advanced syntax engine/
     *      used to built a DFA based lexer/parser, which are generally faster.
     */
    syntax_rule("dnl.*$", "spell,todo,quick:comment");

    syntax_rule("[A-Za-z_][A-Za-z_0-9.]*", "keyword:normal");

    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][-+]?[0-9]*)?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]*", "number");
    syntax_rule("[0-9]+", "number");

    syntax_rule("\"(\\\\\"|[^\"])*\"", "string");
    syntax_rule("\"(\\\\\"|[^\"])*\\$", "string");

    syntax_rule("\'(\\\\\'|[^\'])*\'", "string");
    syntax_rule("\'(\\\\\'|[^\'])*\\$", "string");

    syntax_rule("[()\\[\\]{},;.?:]", "delimiter");
    syntax_rule("[-%+/&*=<>|!~^]", "operator");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY,   "cddofiifin", 2);
    define_keywords(SYNK_PRIMARY,   "forletpwdset", 3);
    define_keywords(SYNK_PRIMARY,   "casedoneechoelifelseesacevalexitifeqreadtestthentype", 4);
    define_keywords(SYNK_PRIMARY,   "aliasbreakendifendswifdefifneqlocalshiftumaskunsetuntilwhile", 5);
    define_keywords(SYNK_PRIMARY,   "exportifndefreturnsetenvsourceswitch", 6);
    define_keywords(SYNK_PRIMARY,   "breaksw", 7);
    define_keywords(SYNK_PRIMARY,   "continuefunction", 8);
}


/*
 *  Modeline support
 */
string
_autoconf_mode()
{
    return "autoconf";
}


/*
 *  Package support
 */
string
_autoconf_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}
