/* -*- indent-width: 4; -*-/
 * $Id: dispinfo.cr,v 1.9 2024/10/06 17:01:12 cvsuser Exp $
 * Display information/configuration.
 *
 *
 */

#include "grief.h"

static int                  dialog;             // dialog handle

#define IDENT_BASE          1000


enum {
    // flag field labels order *MUST* match flagset
    IDENT_WINDOW            = IDENT_BASE,
    IDENT_MOUSE,
    IDENT_READONLY,
    IDENT_CHARMODE,
    IDENT_UNICODE,

    IDENT_SHADOW,
    IDENT_SHADOW_SHOWTHRU,
    IDENT_ASCIIONLY,
    IDENT_ROSUFFIX,
    IDENT_MODSUFFIX,
    IDENT_EOF_DISPLAY,
    IDENT_TILDE_DISPLAY,
    IDENT_EOL_HILITE,
    IDENT_HIMODIFIED,
    IDENT_HIADDITIONAL,
};


static list                 flagset = {
    // flag names
    "window",
    "mouse",
    "readonly",
    "charmode",
    "unicode",

    "shadow",
    "showthru",
    "asciionly",
    "rosuffix",
    "modsuffix",
    "eof_display",
    "tilde_display",
    "eol_hilite",
    "himodified",
    "hiadditional",
    };


/*  Function:           main
 *      dispinfo initialisation, build the dispinfo dialog as follows.
 *
 *>         [x] flag            [x] flag          [x] flag
 *>          :                   :                 :
 *>
 *>                 < Done >  < Apply >  < Help >
 *
 *  Returns:
 *      nothing
 */
void
main()
{
    dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Display Information",
        DLGA_CALLBACK,                          "::di_callback",
        DLGA_ALIGN_W,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADY,                          1,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Display",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Font",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "ESC Delay",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Encoding",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Scheme",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "KBProtocol",
                    DLGA_ALIGN_W,
            DLGC_END,
            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "display",
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  50,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "font",
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  50,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "escdelay",
                    DLGA_ROWS,                  1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "encoding",
                    DLGA_ROWS,                  1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "scheme",
                    DLGA_ROWS,                  1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "kbprotocol",
                    DLGA_ROWS,                  1,
                    DLGA_ALLOW_FILLX,
            DLGC_END,
        DLGC_END,

        DLGC_GROUP,
            DLGA_TITLE,                         "Display Attributes",
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGC_GROUP,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "GUI/Window",
                        DLGA_IDENT,             IDENT_WINDOW,
                        DLGA_ALIGN_W,
                        DLGA_GREYED,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Character-mode",
                        DLGA_IDENT,             IDENT_CHARMODE,
                        DLGA_ALIGN_W,
                        DLGA_GREYED,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Mouse",
                        DLGA_IDENT,             IDENT_MOUSE,
                        DLGA_ALIGN_W,
                        DLGA_GREYED,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Read-only",
                        DLGA_IDENT,             IDENT_READONLY,
                        DLGA_ALIGN_W,
                        DLGA_GREYED,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "UNICODE",
                        DLGA_IDENT,             IDENT_UNICODE,
                        DLGA_ALIGN_W,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Shadows",
                        DLGA_IDENT,             IDENT_SHADOW,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Transparent",
                        DLGA_IDENT,             IDENT_SHADOW_SHOWTHRU,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Read-only Suffix",
                        DLGA_IDENT,             IDENT_ROSUFFIX,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Modified Suffix",
                        DLGA_IDENT,             IDENT_MODSUFFIX,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "ASCII Only",
                        DLGA_IDENT,             IDENT_ASCIIONLY,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOL Hilite",
                        DLGA_IDENT,             IDENT_EOL_HILITE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOF Display",
                        DLGA_IDENT,             IDENT_EOF_DISPLAY,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Tilde Marker",
                        DLGA_IDENT,             IDENT_TILDE_DISPLAY,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Hilite Modified Lines",
                        DLGA_IDENT,             IDENT_HIMODIFIED,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Hilite Additional Lines",
                        DLGA_IDENT,             IDENT_HIADDITIONAL,
                        DLGA_ALIGN_W,
                DLGC_END,
            DLGC_END,                           // CONTAINER
        DLGC_END,                               // GROUP

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_GROUP,
                DLGA_TITLE,                     "Scrolling",
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGA_PADY,                      1,
                DLGC_CONTAINER,
                    DLGA_ALIGN_W,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Scroll row/cols:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              18,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "scroll_rows",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              5,
                    DLGC_LABEL,
                        DLGA_LABEL,             "/",
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "scroll_cols",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              5,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_LABEL,             "Visible row/cols:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              18,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "visible_rows",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              5,
                    DLGC_LABEL,
                        DLGA_LABEL,             "/",
                        DLGA_ATTACH_LEFT,
                        DLGA_PADX,              1,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "visible_cols",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              5,
                DLGC_END,
            DLGC_END,
            DLGC_GROUP,
                DLGA_TITLE,                     "Scroll Bars",
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGA_ALIGN_W,
                DLGC_CHECK_BOX,
                    DLGA_LABEL,                 "Vertical",
                    DLGA_ATTACH_TOP,
                    DLGA_ALIGN_W,
                    DLGA_NAME,                  "sb_vert",
                DLGC_CHECK_BOX,
                    DLGA_LABEL,                 "Horizontal",
                    DLGA_ATTACH_TOP,
                    DLGA_ALIGN_W,
                    DLGA_NAME,                  "sb_horz",
            DLGC_END,                           // GROUP
            DLGC_GROUP,
                DLGA_TITLE,                     "Columns",
                DLGA_ATTACH_RIGHT,
                DLGA_ALIGN_W,
                DLGA_PADX,                      1,
                DLGC_CONTAINER,
                     DLGA_ALIGN_W,
                     DLGC_LABEL,
                        DLGA_LABEL,             "Number:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              12,
                     DLGC_EDIT_FIELD,
                        DLGA_NAME,              "number_cols",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              4,
                DLGC_END,
            DLGC_END,                           // GROUP
         DLGC_END,                              // CONTAINER

         DLGC_CONTAINER,
            DLGA_ATTACH_BOTTOM,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Done",
                DLGA_NAME,                      "done",
                DLGA_ATTACH_LEFT,
                DLGA_DEFAULT_BUTTON,
                DLGA_DEFAULT_FOCUS,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Apply",
                DLGA_NAME,                      "apply",
                DLGA_ATTACH_LEFT,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Help",
                DLGA_NAME,                      "help",
                DLGA_ATTACH_LEFT,
        DLGC_END                                // CONTAINER
        ));
}


void
dispinfo(void)
{
    dialog_run(dialog);
}


static void
di_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT: {
            string feature, encoding, colorscheme, scheme, kbprotocol;
            int isdark, lines, cols, colordepth, truecolor, escsource;
            int i;

            inq_screen_size(lines, cols);       // display
            get_term_feature(TF_NAME, feature);
            get_term_feature(TF_COLORDEPTH, colordepth);
            get_term_feature(TF_TRUECOLOR, truecolor);
            get_term_feature(TF_ENCODING, encoding);
            get_term_feature(TF_SCHEMEDARK, isdark);
            get_term_feature(TF_COLORSCHEME, colorscheme);
            get_term_feature(TF_KBPROTOCOL, kbprotocol);

            sprintf(feature, "%dx%d-%d%s (%s)", cols, lines, colordepth, (truecolor ? "-truecolor" : ""), feature);
            if (colorscheme == "")
               scheme += (isdark ? "dark" : "light");
            else
               sprintf(scheme, "%s (%s)", colorscheme, (isdark ? "dark" : "light"));

            widget_set(NULL, "display", feature);
            inq_font(feature);
            widget_set(NULL, "font", feature);
            sprintf(feature, "%dms (%d)", inq_char_timeout(escsource), escsource);
            widget_set(NULL, "escdelay", feature);
            widget_set(NULL, "encoding", encoding);
            widget_set(NULL, "scheme", scheme);
            widget_set(NULL, "kbprotocol", kbprotocol);

                                                // scroll parameters, plus line-number column.
            widget_set(NULL, "scroll_cols",  inq_display_mode("scroll_cols"));
            widget_set(NULL, "scroll_rows",  inq_display_mode("scroll_rows"));
            widget_set(NULL, "visible_cols", inq_display_mode("visible_cols"));
            widget_set(NULL, "visible_rows", inq_display_mode("visible_rows"));
            widget_set(NULL, "number_cols",  inq_display_mode("number_cols"));

                                                // scroll bars.
            widget_set(NULL, "sb_vert", inq_ctrl_state(WCTRLO_VERT_SCROLL));
            widget_set(NULL, "sb_horz", inq_ctrl_state(WCTRLO_HORZ_SCROLL));

                                                // flag values.
            for (i = 0; i < length_of_list(flagset); ++i) {
                widget_set(NULL, IDENT_BASE + i, inq_display_mode(flagset[i]));
            }
        }
        break;

    case DLGE_BUTTON:
        switch(name) {
        case "apply": {
                int i;

                display_mode(NULL, NULL,        // scroll parameters, plus line-number column.
                    atoi(widget_get(NULL, "scroll_cols")), atoi(widget_get(NULL, "scroll_rows")),
                    atoi(widget_get(NULL, "visible_cols")), atoi(widget_get(NULL, "visible_rows")),
                    atoi(widget_get(NULL, "number_cols")));

                                                // scroll bars.
                set_ctrl_state(WCTRLO_VERT_SCROLL, widget_get(NULL, "sb_vert") ? WCTRLS_ENABLE : WCTRLS_DISABLE);
                set_ctrl_state(WCTRLO_HORZ_SCROLL, widget_get(NULL, "sb_horz") ? WCTRLS_ENABLE : WCTRLS_DISABLE);

                                                // set/clear flags.
                for (i = 0; i < length_of_list(flagset); ++i) {
                    if (widget_get(NULL, IDENT_BASE+i)) {
                        display_mode(flagset[i], NULL);
                    } else {
                        display_mode(NULL, flagset[i]);
                    }
                }

                redraw();
            }
            break;

        case "done":
            dialog_exit();
            break;

        case "help":
            execute_macro("explain display_mode");
            break;
        }
        break;
    }
}

/*eof*/
