/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: popup.cr,v 1.2 2024/09/12 17:08:28 cvsuser Exp $
 *
 *  Mouse popup
 */

#include "grief.h"

static list elements =
    quote_list(
        "Copy", "Cut", "Paste",
        "Left-Shift", "Right-Shift",
        "Exit"
        );

void
popup_mouse(~ int x, ~ int y)
{
    int popup_buf;

#if (0) // work-in-progress
    int handle =
        dialog_create( make_list(
            DLGA_CALLBACK,                  "::popup_callback",
            DLGA_STYLES,                    DLGS_BORDER|DLGS_SYSCLOSE,
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
                DLGC_MENU_ITEM,
                    DLGA_NAME,              "Exit",
                    DLGA_CALLBACK,          "::popup_exit"
            ));
#else
    int handle =
        dialog_create( make_list(
            DLGA_CALLBACK,                  "::popup_listbox",
            DLGA_STYLES,                    DLGS_BORDER|DLGS_SYSCLOSE,
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


static int
popup_listbox(int ident, string name, int p1, int p2)
{
    extern int popup_buf;
    string action;

    switch (p1) {
    case DLGE_INIT:
         if (inq_marked() == MK_NONE) {
            widget_set(NULL, "popup", NULL, DLGA_LBREMOVE, 3);
            widget_set(NULL, "popup", NULL, DLGA_LBREMOVE, 3);
        }
        break;
    case DLGE_CHANGE:
        action = widget_get(NULL, "popup", DLGA_LBTEXT, p2);
        switch (action) {
        case "Copy":
            set_buffer(popup_buf);
            copy();
            break;
        case "Cut":
            set_buffer(popup_buf);
            cut();
            break;
        case "Paste":
            set_buffer(popup_buf);
            paste();
            break;
        case "Left-Shift":
            set_buffer(popup_buf);
            if (inq_marked()) {
                shiftl();
            }
            break;
        case "Right-Shift":
            set_buffer(popup_buf);
            if (inq_marked()) {
                shiftr();
            }
            break;
         case "Exit":
            exit();
            break;
        }
        dialog_exit();
        break;
//  case DLGE_SYSCOMMAND:
//      if (p2 == DLSC_CLOSE) {
//          dialog_exit();
//          return TRUE;
//      }
//      break;
    case DLGE_KEYDOWN:
        if (p2 == key_to_int("<Esc>")) {
            dialog_exit();
            return TRUE;
        }
        break;
    default:
        message("ident=0x%x, name=%s, p1=%d/0x%x, p2=%d/0x%x", ident, name, p1, p1, p2, p2);
        break;
    }
    return FALSE; // default action
}

//end

