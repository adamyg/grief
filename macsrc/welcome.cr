/* -*- indent-width: 4; -*- */
/* $Id: welcome.cr,v 1.14 2022/12/09 15:54:59 cvsuser Exp $
 * Welcome splash dialog.
 *
 *
 */

#include "grief.h"

#define MESSAGE_TEXT    1001

static int              dialog;                 // dialog handle


void
main()
{
    list welcome_text = {
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
            "Copyright (c) 1998 - 2023, Adam Young.",
            "All Rights Reserved.",
            "",
            "Derived from Crisp2.2, by Paul Fox, 1991.",
            "",
            "Please help publish and sponsor " + APPNAME + " development!",
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
            "",
            "<Alt-H> or <F10>help<Enter> for on-line help.",
            "<F10>sysinfo<Enter> for system information.",
            "<F10>license<Enter> license and distribution conditions."
            };

    int maj, min, edit;
    string verbuf;

    version(maj, min, edit);
    sprintf(verbuf, "%s v%d.%d.%d", APPNAME, maj, min, edit);

    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         verbuf,
            DLGA_CALLBACK,                      "::welcome_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGC_LIST_BOX,
                    DLGA_ROWS,                  14,
                    DLGA_COLS,                  65,
                    DLGA_LBDUPLICATES,          TRUE,
                    DLGA_LBELEMENTS,            welcome_text,
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
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Setup",
                    DLGA_NAME,                  "setup",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Help",
                    DLGA_NAME,                  "help",
                    DLGA_ATTACH_LEFT,
            DLGC_END
            ));
}


void
welcome(void)
{
    dialog_run(dialog);
}


static void
welcome_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
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
