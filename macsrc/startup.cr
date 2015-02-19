/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: startup.cr,v 1.11 2014/10/27 23:28:29 ayoung Exp $
 * Initialisation, tuning and setup macro.
 *
 *
 */

#include "grief.h"

static int      _compile_loaded = 0;


/*
 *  macro main
 */
void
startup(void)
{
    /* Set screenblanker on, with 30 minute timeout */
    if (getenv("DISPLAY") == "")  {
        scrblank("30");
    }

    /* Tell user what version is running. */
    load_macro("date");
    version();
}


/*
 *  load_indent ---
 *      The following macros are for langauge sensitive editing. If the BPACKAGES
 *      system is load because the .grinit package variable s set and the 'language'
 *      package is loaded, these macros are called as part of file initialisation.
 */
void
load_indent(void)
{
    require("indent");
}


void
load_compile(void)
{
    if (! _compile_loaded++) {
        if (inq_macro("compile", 0x2) == 2) {   //require unsafe with autoload()
            load_macro("compile");
        }
    }
}

/*eof*/
