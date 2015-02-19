/* -*- mode: cr; indent-width: 4; -*-
 * $Id: chlog.cr,v 1.13 2015/02/19 00:17:41 ayoung Exp $
 * ChangeLog mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "CHLOG"

void
main()
{
    create_syntax(MODENAME);

    /*
     *  operators etc
     */
    syntax_token(SYNT_STRING,     "\"");
    syntax_token(SYNT_BRACKET,    "<([{", ")]}>");
    syntax_token(SYNT_OPERATOR,   "-+%/*=|~^");
    syntax_token(SYNT_DELIMITER,  ",;.?:!&");
    syntax_token(SYNT_WORD,       "0-9a-zA-Z_-");
    syntax_token(SYNT_NUMERIC,    "-+0-9.xXL");

    /*
     *  options
     */
    set_syntax_flags(SYNF_STRING_ONELINE|SYNF_SPELL_WORD);

    /*
     *  keywords
     */
    define_keywords(SYNK_PRIMARY, "JanFebMarAprMayJunJulAugSepOctNovDec", 3);
    define_keywords(SYNK_PRIMARY, "MonTueWedThuFriSatSun", 3);
    define_keywords(SYNK_PRIMARY, "FridayMondaySunday", 6);
    define_keywords(SYNK_PRIMARY, "Tuesday", 7);
    define_keywords(SYNK_PRIMARY, "Thursday", 8);
    define_keywords(SYNK_PRIMARY, "Wednesday", 9);
}


string
_chlog_mode()
{
    return "chlog";                             /* return package extension */
}


string
_changelog_mode()
{
    return "chlog";                             /* return package extension */
}


string
_chlog_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    use_tab_char(1);
    tabs(9);
    return "";
}

