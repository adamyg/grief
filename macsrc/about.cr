/* -*- indent-width: 4; -*- */
/* $Id: about.cr,v 1.11 2021/04/15 14:51:31 cvsuser Exp $
 * About box.
 *
 *
 */

#include "grief.h"

#define MESSAGE_TEXT    1001

#pragma autoload("help_about")

static int              dialog;                 // dialog handle


void
main()
{
    list about_text = {
            "",
            "   Glorious Reconfigurable Interactive Editing Facility",
            "",
            "                __________  ________________",
            "               / ____/ __ \\/  _/ ____/ ____/",
            "              / / __/ /_/ // // __/ / /_",
            "             / /_/ / _, _// // /___/ __/",
            "             \\____/_/ |_/___/_____/_/",
            "",
            "         1000111 1110010 1101001 1100101 1100110",
            "",
            "Copyright (c) 1998 - 2021, Adam Young.",
            "All Rights Reserved.",
            "",
            "Derived from crisp2.2, Paul Fox, 1991.",
            "",
            "Please help publish and sponsor " + APPNAME + " development !",
            "",
            APPNAME + " is open software: you can use, redistribute it",
            "and/or modify it under the terms of the " + APPNAME + " License.",
            "",
            APPNAME + " is distributed in the hope that it will be useful,",
            "but is PROVIDED \"AS IS\" AND WITHOUT ANY EXPRESS OR IMPLIED",
            "WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES",
            "OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.",
            "",
            "See the License for specific details; available via the command",
            "line options 'authors' and 'license'.",
            ""
            };

    string machtype, compiled, extra1, extra2;
    int maj, min, rel, edit;

    version(maj, min, edit, rel, machtype, compiled);

    sprintf(extra1, "%s %s v%d.%d.%d", APPNAME, machtype, maj, min, edit, compiled);
    sprintf(extra2, "Built on %s", compiled);

    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         APPNAME,
            DLGA_CALLBACK,                      "::about_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGC_LIST_BOX,
                    DLGA_NAME,                  "text",
                    DLGA_ROWS,                  14,
                    DLGA_COLS,                  65,
                    DLGA_LBDUPLICATES,          TRUE,
                    DLGA_LBELEMENTS,            make_list("", extra1, extra2),
                    DLGA_LBELEMENTS,            about_text,
                    DLGA_ALLOW_FILLX,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGA_ALIGN_CENTER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Done",
                    DLGA_NAME,                  "done",
                    DLGA_ATTACH_LEFT,
                    DLGA_DEFAULT_FOCUS,
                    DLGA_DEFAULT_BUTTON,
            DLGC_END
            ));
}


void
about(void)
{
    dialog_run(dialog);
}


static void
about_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        widget_set(dialog, "elements", "elements", DLGA_LBINSERT);
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "done":
            dialog_exit();
            break;
        case "setup":
            execute_macro("setup");
            break;
        case "help":
            execute_macro("help");
            break;
        }
        break;
    }
}

/*eof*/
