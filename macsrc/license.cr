/* -*- indent-width: 4; -*- */
/* $Id: license.cr,v 1.20 2025/07/02 19:12:05 cvsuser Exp $
 * License information.
 *
 *
 */

#include "grief.h"

#define LICENSE_FILE    "license.txt"

#define LICENSE_SOURCE  "https://github.com/adamyg/grief/blob/master/COPYING"
#define LICENSE_TEXT    1001

static list             license_import(void);

static int              dialog;                 // dialog handle


void
main()
{
    list license_text = grief_license();

    license_text += license_import();

    int maj, min, edit, rel;
    string verbuf;

    version(maj, min, edit, rel);
    sprintf(verbuf, "%s v%d.%d.%d.%d", APPNAME, maj, min, edit, rel);

    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         verbuf,
            DLGA_CALLBACK,                      "::license_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGC_LIST_BOX,
                    DLGA_ROWS,                  14,
                    DLGA_COLS,                  78,
                    DLGA_LBDUPLICATES,          TRUE,
                    DLGA_LBELEMENTS,            license_text,
                    DLGA_ALLOW_FILLX,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGA_ALIGN_CENTER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Accept",
                    DLGA_NAME,                  "accept",
                    DLGA_ATTACH_LEFT,
                    DLGA_DEFAULT_FOCUS,
                    DLGA_DEFAULT_BUTTON,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Cancel",
                    DLGA_NAME,                  "cancel",
                    DLGA_ATTACH_LEFT,
            DLGC_END
            ));
}


static list
license_import(void)
{
    const int curbuf = inq_buffer();
    string source = help_resolve(LICENSE_FILE);
    string line;
    list text;

    push(text, "");
    if (0 == access(source, 0) &&
            edit_file(EDIT_SYSTEM, source) != -1) {
        while ((line = read()) != "") {         // while (!eof)
            push(text, rtrim(line, "\r\n\t "));
            down();
        }
        set_buffer(curbuf);
        attach_buffer(curbuf);
    } else {
        push(text, "unable to locate license \"" + LICENSE_FILE + "\"");
        push(text, "");
        push(text, "please consult " + LICENSE_SOURCE);
    }
    push(text, "");
    return text;
}


void
license(void)
{
    dialog_run(dialog);
}


static void
license_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "accept":
            dialog_exit();
            break;
        case "cancel":
            dialog_exit();
            exit();
            break;
        }
        break;
    }
}

/*eof*/
