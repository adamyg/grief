/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: whelp.cr,v 1.6 2014/10/27 23:28:30 ayoung Exp $
 * Interface to WATCOMS online help system,
 *
 *      Usage: whelp help_file [topic_name]
 */

#include "grief.h"

void
whelp(void)
{
    string libname, keyword;

    /* Retrieve library name */
    if (!get_parm(0, libname, "Library: ", NULL, "") ||
            0 == strlen(libname)) {
        select_list("whelp libraries",
            "", 2,
            quote_list(
                "Programmers Guide",            "whelp PGUIDE",
                "C/C++ Users guide",            "whelp CGUIDE",
                "C Library Reference",          "whelp CLIB",
                "C++ Class LIbrary Reference",  "whelp CPPLIB",
                "Linker guide",                 "whelp LGUIDE",
                "Resource Compiler",            "whelp RESCOMP",
                "Tools",                        "whelp TOOLS",
                "C++ Diagnostic Messages",      "whelp WCCERRS",
                "Watcom debugger",              "whelp WD",
                "C++ Diagnostic Messages",      "whelp WPPERRS",
                "Watcom C/C++ Profiler",        "whelp WPROF" ),
            SEL_CENTER);
        return;
    }

    /* Retrieve (optional) keyword */
    get_parm(1, keyword, "Keyword: ", NULL, "");

    /* Invoke whelp.exe */
    if (strlen(keyword)) {
        keyword = " \"" + keyword + "\"";
    }

    /* Preform WHELP request */
    if (shell("whelp.exe "+libname+keyword+" >whelp.err", 1) > 0) {
        int    error_buf = create_buffer("whelp error", "whelp.err", 1),
               old_buf = set_buffer(error_buf);
        string buf = trim(read());

        error(buf);
        delete_buffer(error_buf);
        set_buffer(old_buf);
    }
}

/*eof*/
