/* -*- indent-width: 4; -*-
 * $Id: search_replace.cr,v 1.4 2014/10/27 23:28:27 ayoung Exp $
 * Search and replace ui
 *
 *
 */

#include "grief.h"


/*  Function:           search_replace
 *      Search & replace dialog
 *
 *      Search:     xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *      Replace:    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *
 *           [] Regular Expression       [] Case Sensitive
 *           [] Unix
 *
 *                  < done >  < apply >  < help >
 */
void
search_replace(void)
{
    int dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         "Search & Replace",
            DLGA_CALLBACK,                      "::rp_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGA_ALIGN_NW,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_ALIGN_NW,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Translate: ",
                        DLGA_ALIGN_W,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Replacement: ",
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_RIGHT,
                    DLGA_ALIGN_NE,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "find",
                        DLGA_ROWS,              1,
                        DLGA_COLS,              30,
                        DLGA_ALLOW_FILLX,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "replace",
                        DLGA_ROWS,              1,
                        DLGA_ALLOW_FILLX,
                DLGC_END,
            DLGC_END,

            DLGC_CONTAINER,                     /*spacer*/
                DLGA_ATTACH_TOP,
                DLGA_ROWS,                      1,
            DLGC_END,

            DLGC_GROUP,
                DLGA_ATTACH_TOP,
                DLGA_ALIGN_NW,
                DLGA_ALLOW_FILLX,
                DLGC_CONTAINER,
                    DLGA_PADY,                  1,
                    DLGC_CONTAINER,
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Regular Expression",
                            DLGA_NAME,          "regexp",
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Unix",
                            DLGA_NAME,          "unix",
                            DLGA_ALIGN_W,
                    DLGC_END,
                    DLGC_CONTAINER,
                        DLGA_ATTACH_RIGHT,
                        DLGA_PADX,              1,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Case Sensitive",
                            DLGA_NAME,          "case",
                            DLGA_ALIGN_W,
                    DLGC_END,
                DLGC_END,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Done",
                    DLGA_NAME,                  "done",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Apply",
                    DLGA_NAME,                  "apply",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Help",
                    DLGA_NAME,                  "help",
                    DLGA_ATTACH_LEFT,
            DLGC_END
            ));
    dialog_run(dialog);
    dialog_delete(dialog);
}


static void
rp_callback(int ident, string name, int p1, int p2)
{
    UNUSED(p1, p2);

    switch (p1) {
    case DLGE_INIT: {
            extern int search__regexp, search__syntax, search__case;

            widget_set(ident, "regexp", search__regexp);
            widget_set(ident, "unix", search__syntax);
            widget_set(ident, "case", search__case);
        }
        break;

    case DLGE_BUTTON:
        switch (name) {
        case "done":
            dialog_exit();
            break;
        }
        break;
    }
}

/*end*/
