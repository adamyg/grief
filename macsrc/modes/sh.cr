/* -*- mode: cr; indent-width: 4; -*-
 * $Id: sh.cr,v 1.11 2024/07/30 16:29:20 cvsuser Exp $
 * 'sh' programming languages mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "sh"

void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,    "#");
    syntax_token(SYNT_QUOTE,      "\\");
    syntax_token(SYNT_STRING,     "\"");
    syntax_token(SYNT_LITERAL,    "\'");
    syntax_token(SYNT_BRACKET,    "([{", ")]}");
    syntax_token(SYNT_DELIMITER,  ",;:");
    syntax_token(SYNT_OPERATOR,   "-%+/&*=<>|!~^");
    syntax_token(SYNT_KEYWORD,    "a-zA-Z_", "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,    "-+.0-9_xa-fA-F");

    define_keywords(SYNK_PRIMARY, "cddofiifin", 2);
    define_keywords(SYNK_PRIMARY, "forletpwdset", 3);
    define_keywords(SYNK_PRIMARY, "casedoneechoelifelseesacevalexitifeqreadtestthentype", 4);
    define_keywords(SYNK_PRIMARY, "aliasbreakendifendswifdefifneqlocalshiftumaskunsetuntilwhile", 5);
    define_keywords(SYNK_PRIMARY, "exportifndefreturnsetenvsourceswitch", 6);
    define_keywords(SYNK_PRIMARY, "breaksw", 7);
    define_keywords(SYNK_PRIMARY, "continuefunction", 8);

    /* options,
     *      Hilite line-cont whitespace.
     */
    set_syntax_flags(SYNF_HILITE_LINECONT);
}


/*
 *  Modeline support
 */
string
_sh_mode()
{
    return "sh";
}


/*
 *  Package support
 */
string
_sh_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}
