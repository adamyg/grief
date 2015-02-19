/* -*- mode: cr; indent-width: 4; -*-
/* $Id: awk.cr,v 1.9 2014/10/22 02:34:32 ayoung Exp $
 * AWK programming mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "AWK"

void
main(void)
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,    "#");
    syntax_token(SYNT_QUOTE,      "\\");
    syntax_token(SYNT_STRING,     "\"");
    syntax_token(SYNT_CHARACTER,  "\'");
    syntax_token(SYNT_BRACKET,    "([{", ")]}");
    syntax_token(SYNT_OPERATOR,   "-%+/&*=<>|!~^`");
    syntax_token(SYNT_DELIMITER,  ";,");
    syntax_token(SYNT_WORD,       "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,    "-+0-9a-fA-F.xX");

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY, "do,if,in,or", -2);
    define_keywords(SYNK_PRIMARY, "and,cos,exp,for,int,log,sin,sub,xor", -3);
    define_keywords(SYNK_PRIMARY, "else,exit,gsub,next,rand,sqrt", -4);
    define_keywords(SYNK_PRIMARY, "asort,atan2,break,close,compl,index,match,print,split,srand,while", -5);
    define_keywords(SYNK_PRIMARY, "delete,fflush,gensub,length,lshift,mktime,printf,return,rshift,substr,system", -6);
    define_keywords(SYNK_PRIMARY, "getline,sprintf,systime,tolower,toupper", -7);
    define_keywords(SYNK_PRIMARY, "continue,function,nextfile,strftime,strtonum", -8);
    define_keywords(SYNK_PRIMARY, "dcgettext", -9);
    define_keywords(SYNK_PRIMARY, "bindtextdomain", 14);

    define_keywords(1, "FSNFNRRSRT", -2);
    define_keywords(1, "ENDFNROFSORS", -3);
    define_keywords(1, "ARGCARGVLINTOFMT", -4);
    define_keywords(1, "BEGINERRNO", -5);
    define_keywords(1, "ARGINDRSTARTSUBSEP", -6);
    define_keywords(1, "BINMODECONVFMTENVIRONRLENGTH", -7);
    define_keywords(1, "FILENAME", -8);
    define_keywords(1, "IGNORECASETEXTDOMAIN", -10);
    define_keywords(1, "FIELDWIDTHS", -11);
}


/*
 * Modeline/package support
 */
string
_awk_mode(void)
{
    return "awk";                               /* return package extension */
}


string
_awk_highlight_first(void)
{
    attach_syntax(MODENAME);
    return "";
}
