/* -*- mode: cr; indent-width: 4; -*-
/* $Id: nroff.cr,v 1.9 2014/10/22 02:34:35 ayoung Exp $
 * NROFF programming mode
 *
 *
 */

#include "../grief.h"

#define MODENAME "NROFF"


void
main()
{
    create_syntax(MODENAME);

    /*
     *  Standard syntax engine
     */
    syntax_token(SYNT_COMMENT,    ".\\\"");
    syntax_token(SYNT_QUOTE,      ".");
    syntax_token(SYNT_BRACKET,    "([{", ")]}");
    syntax_token(SYNT_DELIMITER,  "{}[]<>()");
    syntax_token(SYNT_WORD,       ".a-zA-Z");

    /*
     *  Keywords
     */
    define_keywords(SYNK_PRIMARY, ".B.I", 2);
    define_keywords(SYNK_PRIMARY, ".BR.DT.IP.PP.RB.RI.RE.RS.SB.SH.TH.TP.fi.nf", 3);
}


/*
 *  Modeline/package support
 */
string
_nroff_mode()
{
    return "nroff";                             /* return package extension */
}


string
_nroff_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}

