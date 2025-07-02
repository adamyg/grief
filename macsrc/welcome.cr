/* -*- indent-width: 4; -*- */
/* $Id: welcome.cr,v 1.16 2025/07/02 19:12:05 cvsuser Exp $
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
    list welcome_text = grief_license();
    list extra_text = {
            "",
            "See the License for specific details; available via the command",
            "line options 'authors' and 'license'.",
            "",
            "<Alt-H> or <F10>help<Enter> for on-line help.",
            "<F10>sysinfo<Enter> for system information.",
            "<F10>license<Enter> license and distribution conditions."
            };

    welcome_text += extra_text;

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
