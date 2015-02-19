/* -*- mode: cr; indent-width: 4; -*-
 * $Id: gas.cr,v 1.10 2014/10/27 23:28:33 ayoung Exp $
 * GRIEF macros to support GNU assembler (gas)
 *
 *
 */

#include "../grief.h"

#define MODENAME "GAS"


void
main()
{
    create_syntax(MODENAME);

    /*
     *  operators etc
     */
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    /*
     *  options
     */
    set_syntax_flags(SYNF_COMMENTS_CSTYLE);

    /*
     *  keywords
     */
    define_keywords(SYNK_PRIMARY,   ".if", 3);
    define_keywords(SYNK_PRIMARY,   ".bss.equ.got", 4);
    define_keywords(SYNK_PRIMARY,   ".byte.comm.data.else.endm.endr.long.quad.rept.size.text.type.word", 5);
    define_keywords(SYNK_PRIMARY,   ".align.ascii.asciz.endif.globl.hword.lcomm.macro.short.space",6 );
    define_keywords(SYNK_PRIMARY,   ".rodata", 7);
    define_keywords(SYNK_PRIMARY,   ".section", 8);
    define_keywords(SYNK_PRIMARY,   ".att_syntax", 11);
    define_keywords(SYNK_PRIMARY,   ".intel_syntax", 13);
}


string
_gas_mode()
{
    return "gas";                               /* return package extension */
}


string
_gas_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}

/*end*/
