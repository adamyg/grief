/* -*- mode: cr; indent-width: 4; -*-
 * $Id: txt.cr,v 1.8 2014/10/22 02:34:36 ayoung Exp $
 * Text support mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "txt"


void
main()
{
    create_syntax(MODENAME);
    syntax_token(SYNT_WORD,         "a-zA-Z", "-0-9a-zA-Z'`_\b");
    syntax_token(SYNT_OPERATOR,     ",;:.!");
    syntax_token(SYNT_BRACKET,      "([{<", ")]}>");
    set_syntax_flags(SYNF_HILITE_WS|SYNF_SPELL_WORD|SYNF_MANDOC);
}


string
_txt_mode()
{
    return "txt";
}


string
_txt_highlight_first(void)
{
    attach_syntax(MODENAME);
    return "";
}

