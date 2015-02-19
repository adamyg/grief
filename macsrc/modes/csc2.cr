/* -*- mode: cr; indent-width: 4; -*-
 * $Id: csc2.cr,v 1.5 2014/10/22 02:34:33 ayoung Exp $
 * csc2 file format support.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "CSC2"

void
main()
{
    create_syntax(MODENAME);
    syntax_token(SYNT_COMMENT,       "//");
    syntax_token(SYNT_BRACKET,       "([{", ")]}");
    syntax_token(SYNT_OPERATOR,      "-%+/&*=<>|!~^");
    syntax_token(SYNT_WORD,          "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,       "-+0-9a-fA-F.xXL");

    set_syntax_flags(SYNF_COMMENTS_CSTYLE|SYNF_COMMENTS_QUOTE);

    define_keywords(SYNK_DEFINITION, "tag,ondisk,default,keys,constants,constraints");
    define_keywords(SYNK_PRIMARY,    "default,dup");
    define_keywords(SYNK_TYPE,       "byte,short,u_short,int,u_int,longlong,u_longlong,float,double");
    define_keywords(SYNK_TYPE,       "cstring,pstring,vutf8");
    define_keywords(SYNK_TYPE,       "blob");
    define_keywords(SYNK_TYPE,       "datetime");
}


string
_csc2_mode(void)
{
    return MODENAME;
}

//end
