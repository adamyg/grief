/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: setcolor.cr,v 1.14 2014/10/27 23:28:27 ayoung Exp $
 * Basic color configuration.
 *
 *
 */

#include "grief.h"

static void             setcolor_gui(void);
static void             setcolor_text(void);

static list             colors_all = { COLORS_ALL };
static list             color_list;
static int              color_dialog;


void
main(void)
{
    color_list = make_list(
        "Background         : ", colors_all,
        "Normal text        : ", colors_all,
        "Message text       : ", colors_all,
        "Error text         : ", colors_all,
        "Hilite foreground  : ", colors_all,
        "   and background  : ", colors_all,
        "Frame              : ", colors_all,
        "Selected title     : ", colors_all
        );

    color_dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Basic Colors",
        DLGA_CALLBACK,                          "::color_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Background",
                    DLGA_COLS,                  22,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Normal text",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Message text",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Error text",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Hilite foreground",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "and background",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Frame",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Selected title",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "background",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "normal",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "messages",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "errors",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "hi_fg",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "hi_bg",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "frame",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "selected",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            colors_all,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_BOTTOM,
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Done",
                DLGA_NAME,                      "done",
                DLGA_DEFAULT_BUTTON,
                DLGA_DEFAULT_FOCUS,
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Apply",
                DLGA_NAME,                      "apply",
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Help",
                DLGA_NAME,                      "help",
        DLGC_END
        ));
}


void
setcolor(~ string mode)
{
    const int text = (0 == strlen(mode)? (DC_ASCIIONLY & inq_display_mode()) :
                            "text" == mode || "--text" == mode);

    if (text) {
        setcolor_text();
    } else {
        setcolor_gui();
    }
}


static void
setcolor_gui(void)
{
    int background, normal, selected, messages, errors, hi_bg, hi_fg, frame;

    inq_color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);

    if (background < 0 || normal < 0 || messages < 0 ||
            errors < 0 || selected < 0 || hi_fg < 0 || hi_bg < 0 || frame < 0) {
        message("One or more extended colors ecnountered, loading colorscheme editor ...");
        colorscheme();
        return;
    }
    widget_set(color_dialog, "background",  colors_all[background]);
    widget_set(color_dialog, "normal",      colors_all[normal]);
    widget_set(color_dialog, "messages",    colors_all[messages]);
    widget_set(color_dialog, "errors",      colors_all[errors]);
    widget_set(color_dialog, "hi_bg",       colors_all[hi_bg]);
    widget_set(color_dialog, "hi_fg",       colors_all[hi_fg]);
    widget_set(color_dialog, "frame",       colors_all[frame]);
    widget_set(color_dialog, "selected",    colors_all[selected]);
    dialog_run(color_dialog);
}


static int
color_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_BUTTON:
        switch(name) {
        case "done":
            dialog_exit();
            break;
        case "apply": {
                string background, normal, messages, errors, hi_bg, hi_fg, frame, selected;

                background = widget_get(color_dialog, "background");
                normal     = widget_get(color_dialog, "normal");
                messages   = widget_get(color_dialog, "messages");
                errors     = widget_get(color_dialog, "errors");
                hi_fg      = widget_get(color_dialog, "hi_bg");
                hi_bg      = widget_get(color_dialog, "hi_fg");
                frame      = widget_get(color_dialog, "frame");
                selected   = widget_get(color_dialog, "selected");

                if (strlen(background)) {
                    if (background == normal && "none" != normal) {
                        error("Background and normal colors are the same '%s'.", background);
                        return TRUE;
                    }
                    message("colors %s,%s,%s,%s,%s,%s,%s,%s", background, normal, messages, errors, hi_bg, hi_fg, frame, selected);
                    color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);
                }
                dialog_exit();
            }
            break;
        case "help":
            execute_macro("explain setcolor");
            break;
        }
    }
    return TRUE;
}


static void
setcolor_text(void)
{
    list results, field_list;
    int background, normal, selected, messages, errors, hi_bg, hi_fg, frame;

    inq_color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);

    if (background < 0 || normal < 0 || messages < 0 ||
            errors < 0 || selected < 0 || hi_fg < 0 || hi_bg < 0 || frame < 0) {
        message("One or more extended colors ecnountered, loading colorscheme editor ...");
        colorscheme();
        return;
    }

    field_list  = color_list;
    results[0]  = background;
    results[1]  = normal;
    results[2]  = messages;
    results[3]  = errors;
    results[4]  = hi_fg;
    results[5]  = hi_bg;
    results[6]  = frame;
    results[7]  = selected;
    results     = field_list("Basic Colors", results, field_list, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        return;
    }
    background  = results[0];
    normal      = results[1];
    messages    = results[2];
    errors      = results[3];
    hi_fg       = results[4];
    hi_bg       = results[5];
    frame       = results[6];
    selected    = results[7];

    if (background >= 0) {
        if (background == normal && COLOR_NONE != normal) {
            error("Background and normal colors are the same '%d'.", background);
        } else {
            color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);
        }
    }
}

/*end*/
