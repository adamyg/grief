/* -*- indent-width: 4; -*- */
/* $Id: wininfo.cr,v 1.9 2014/10/27 23:28:30 ayoung Exp $
 * Window Information/Configuration.
 *
 *
 */

#include "grief.h"

static int                  dialog;             // dialog handle

#define IDENT_BASE          1000

enum {      // flag field labels order *MUST* match flagset
    IDENT_EOF_DISPLAY       = IDENT_BASE,
    IDENT_EOL_HILITE,
    IDENT_HIADDITIONAL,
    IDENT_HIMODIFIED,
    IDENT_LINE_NUMBERS,
    IDENT_NO_BORDER,
    IDENT_NO_SHADOW,
    IDENT_SHOWANCHOR,
    IDENT_TILDE_DISPLAY,
    };

static list                 flagset =
    {       // flag names
    "eof_display",
    "eol_hilite",
    "hiadditional",
    "himodified",
    "line_numbers",
    "no_border",
    "no_shadow",
    "showanchor",
    "tilde_display",
    };


/*  Function:       main
 *      wininfo initialise, build the wininfo dialog as follows
 *
 *>     Title:      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *>
 *>         [x] flag            [x] flag          [x] flag
 *>          :                   :                 :
 *>
 *>                   < ok >  < apply >  < help >
 *
 *  Returns:
 *      nothing
 */
void
main()
{
    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         "Window Information",
            DLGA_CALLBACK,                      "::wi_callback",
            DLGA_ALIGN_W,

            DLGC_CONTAINER,
                DLGA_ALIGN_W,
                DLGA_PADY,                      1,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Title:",
                        DLGA_COLS,              12,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGC_EDIT_FIELD,
                        DLGA_ALIGN_E,
                        DLGA_NAME,              "title",
                        DLGA_ROWS,              1,
                        DLGA_COLS,              40,
                        DLGA_ALLOW_FILLX,
                        DLGA_GREYED,
                DLGC_END,
            DLGC_END,

            DLGC_GROUP,
                DLGC_CONTAINER,
                DLGA_PADY,                      1,
                DLGA_ALIGN_W,
                    DLGC_CONTAINER,
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "No Shadow",
                            DLGA_IDENT,         IDENT_NO_SHADOW,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "No Border",
                            DLGA_IDENT,         IDENT_NO_BORDER,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Show Anchor",
                            DLGA_IDENT,         IDENT_SHOWANCHOR,
                            DLGA_ALIGN_W,
                    DLGC_END,
                    DLGC_CONTAINER,
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Line Numbers",
                            DLGA_IDENT,         IDENT_LINE_NUMBERS,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Hilite Modified Lines",
                            DLGA_IDENT,         IDENT_HIMODIFIED,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Hilite Additional Lines",
                            DLGA_IDENT,         IDENT_HIADDITIONAL,
                            DLGA_ALIGN_W,
                    DLGC_END,
                    DLGC_CONTAINER,
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "EOF Marker",
                            DLGA_IDENT,         IDENT_EOF_DISPLAY,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "Tilde Marker",
                            DLGA_IDENT,         IDENT_TILDE_DISPLAY,
                            DLGA_ALIGN_W,
                        DLGC_CHECK_BOX,
                            DLGA_LABEL,         "EOL Hilite",
                            DLGA_IDENT,         IDENT_EOL_HILITE,
                            DLGA_ALIGN_W,
                    DLGC_END,
                DLGC_END,                       // CONTAINER
            DLGC_END,                           // GROUP

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGA_ALIGN_CENTER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Done",
                    DLGA_NAME,                  "done",
                    DLGA_ATTACH_LEFT,
                    DLGA_DEFAULT_BUTTON,
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
}


void
wininfo(void)
{
    string title;
    int curwin = inq_window();

    UNUSED(curwin, title);
    inq_window_info(NULL, NULL, NULL, NULL, NULL, NULL, title);
    dialog_run(dialog);
}


static void
wi_callback(int ident, string name, int p1, int p2)
{
    extern string title;                        // assigned title
    extern int curwin;                          // previous current window

    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT: {
            int i;

            widget_set(NULL, "title", title);
            for (i = 0; i < length_of_list(flagset); ++i) {
                widget_set(NULL, IDENT_BASE + i, inq_window_flags(curwin, flagset[i]));
            }
        }
        break;

    case DLGE_BUTTON:
        switch(name) {
        case "apply": {
                int i;
                                                // set/clear flags
                for (i = 0; i < length_of_list(flagset); ++i) {
                    if (widget_get(NULL, IDENT_BASE+i)) {
                        set_window_flags(curwin, flagset[i], NULL);
                    } else {
                        set_window_flags(curwin, NULL, flagset[i]);
                    }
                }
                redraw();
            }
            break;
        case "done":
            dialog_exit();
            break;
        case "help":
            execute_macro("explain set_window_flags");
            break;
        }
        break;
    }
}

/*eof*/
