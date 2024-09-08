/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: popup.cr,v 1.1 2024/09/08 16:25:51 cvsuser Exp $
 *
 *  Mouse popup
 */

#include "grief.h"

#define POPUP_COPY 0
#define POPUP_CUT 1
#define POPUP_PASTE 2
#define POPUP_SHIFT_LEFT 3
#define POPUP_SHIFT_RIGHT 4

static list elements = 
    quote_list(
        "Copy", "Cut", "Paste",
        "Left-Shift", "right-Shift"
    );

void
popup_mouse(~ int x, ~ int y)
{
    int popup_buf;

#if (0) // work-in-progress
    int handle =
        dialog_create( make_list(
            DLGA_CALLBACK,                  "::popup_callback",
            DLGC_MENU,
                DLGA_NAME,                  "popup",
                DLGC_MENU_ITEM,
                    DLGA_ICON,              0x2398,     // Next Page
                    DLGA_NAME,              "Copy",
                    DLGA_CALLBACK,          "::popup_copy",
                DLGC_MENU_ITEM,
                    DLGA_ICON,              0x2702,     // Black Scissors
                    DLGA_NAME,              "Cut",
                    DLGA_CALLBACK,          "::popup_cut",
                DLGC_MENU_SEPARATOR,
                DLGC_MENU_ITEM,
                    DLGA_ICON,              0x1f4cb,    // Clipboard
                    DLGA_NAME,              "Paste",
                    DLGA_CALLBACK,          "::popup_paste"
                DLGC_MENU_ITEM,
                    DLGA_NAME,              "Left-Shift",
                    DLGA_CALLBACK,          "::popup_shift"
                    DLGA_GREYED,
                DLGC_MENU_ITEM,
                    DLGA_NAME,              "Right-Shift",
                    DLGA_CALLBACK,          "::popup_shift"
                    DLGA_GREYED,
            ));
#else
    int handle =
        dialog_create( make_list(
            DLGA_CALLBACK,                  "::popup_lbcallback",
            DLGC_LIST_BOX,
                DLGA_NAME,                  "popup",
                DLGA_KEYDOWN,               1,
                DLGA_ROWS,                  length_of_list(elements),
                DLGA_COLS,                  12,
                DLGA_LBELEMENTS,            elements
            ));
#endif

    if (get_parm(0, x) <= 0) x = -1;
    if (get_parm(1, y) <= 0) y = -1;

    popup_buf = inq_buffer();
    dialog_run(handle, x, y);
    dialog_delete(handle);
}


static void
popup_lbcallback(int ident, string name, int p1, int p2)
{
    extern int popup_buf;

    switch (p1) {
     case DLGE_INIT:
         if (inq_marked() == MK_NONE) {
            widget_set(NULL, "popup", NULL, DLGA_LBREMOVE, POPUP_SHIFT_RIGHT);
            widget_set(NULL, "popup", NULL, DLGA_LBREMOVE, POPUP_SHIFT_LEFT);
        }
        break;
    case DLGE_CHANGE:
        switch (p2) {
        case POPUP_COPY:
            set_buffer(popup_buf);
            copy();
            break;
        case POPUP_CUT:
            set_buffer(popup_buf);
            cut();
            break;
        case POPUP_PASTE:
            set_buffer(popup_buf);
            paste();
            break;
        case POPUP_SHIFT_LEFT:
            set_buffer(popup_buf);
            if (inq_marked()) {
                shiftl();
            }
            break;
        case POPUP_SHIFT_RIGHT:
            set_buffer(popup_buf);
            if (inq_marked()) {
                shiftr();
            }
            break;
        }
        dialog_exit();
        break;
    case DLGE_KEYDOWN:
        if (p2 == key_to_int("<Esc>")) {
            dialog_exit();
        }
        break;
    default:
        message("ident=0x%x, name=%s, p1=%d/0x%x, p2=%d/0x%x", ident, name, p1, p1, p2, p2);
        break;
    }
}

//end
