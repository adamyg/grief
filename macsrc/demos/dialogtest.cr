/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dialogtest.cr,v 1.9 2024/10/07 16:23:06 cvsuser Exp $
 *
 *  This file is used when debugging and fixing CRISP to aid in regression
 *  testing - catching bugs introduced inadvertently. This script does not
 *  attempt to exhaustively test CRISP featires, but tests are added whenever
 *  a bug is found, to ensure the conditions do not get overlooked in the future.
 *
 *  The tests in this file are mainly to do with tty dialog interface.
 *
 *  This file can also be run after porting GRIEF, to ensure that these tests
 *  work as expected. If anything doesn't work that should, the porter will
 *  have to check for portability problems. These tests attempt to do things in
 *  order of complexity.
 *
 *
 */

#include "../grief.h"

static int              dialog_tst;

static void             dialogmake();

void
main()
{
    if (! dialog_tst) {
        dialogmake();
    }
}


void
dialogtest()
{
    dialog_run(dialog_tst);
}


void
dialogdel()
{
    dialog_delete(dialog_tst);
    dialog_tst = 0;
}


void
dialogreset()
{
    dialog_delete(dialog_tst);
    dialogmake();
}


static void
dialogmake()
{
    list elements = quote_list(                 // list-box elements
                "apple",        "applepie",     "apricot",      "banana",
                "carrot",       "cherry",       "chili pepper", "coconut",
                "pear",         "pineapple",    "plum",         "pomegranate",
                "cranberry",    "cucumber",     "eggplant",     "fig",
                "blackberry",   "blackcurrant", "blueberry",    "boysenberry",
                "gooseberry",   "gourd",        "grape",        "grapefruit",
                "lime",         "lucuma",       "melon orange", "mulberry",
                "pumpkin",      "raspberry",    "redcurrant",   "strawbery",
                "guava",        "hazelhut",     "kiwifruit",    "lemon",
                "nectarine",    "olive",        "orange",       "peach",
                "tomato"
                );

    dialog_tst =  dialog_create( make_list(
        DLGA_TITLE,                             "dialog test",
        DLGA_CALLBACK,                          "::callback",   // global call-back
        DLGA_STYLES,                            DLGS_CAPTION|DLGS_SYSCLOSE,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGC_GROUP,
                DLGA_NAME,                      "group_1",
                DLGA_ATTACH_LEFT,
                DLGC_RADIO_BUTTON,
                    DLGA_NAME,                  "radio_1",
                    DLGA_LABEL,                 "&radio 1.1",
                    DLGA_LABEL,                 "r&adio 1.2",
                    DLGA_LABEL,                 "ra&dio 1.3",
                    DLGA_LABEL,                 "rad&io 1.4",
            DLGC_END,

            DLGC_GROUP,
                DLGA_NAME,                      "group_2",
                DLGA_ATTACH_RIGHT,
                DLGC_CHECK_BOX,
                    DLGA_NAME,                  "check_1",
                    DLGA_LABEL,                 "c&heck 1",
                DLGC_LABEL,
                    DLGA_NAME,                  "label1",
                    DLGA_LABEL,                 "label&2",
                    DLGA_COLS,                  10,
                DLGC_LABEL,
                    DLGA_NAME,                  "label2",
                    DLGA_LABEL,                 "label&1",
                    DLGA_COLS,                  10,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGC_GROUP,
                DLGA_NAME,                      "group_3",
                DLGA_ATTACH_LEFT,
                DLGC_RADIO_BUTTON,
                    DLGA_ORIENTATION,           1,
                    DLGA_NAME,                  "radio_2",
                    DLGA_LABEL,                 "&radio 2.1",
                    DLGA_LABEL,                 "r&adio 2.2",
                    DLGA_LABEL,                 "ra&dio 2.3",
                    DLGA_LABEL,                 "rad&io 2.4",
            DLGC_END,
        DLGC_END,

        DLGC_LIST_BOX,
            DLGA_NAME,                          "listbox_1",
            DLGA_VALUE,                         "unknown",
            DLGA_ROWS,                          3,
            DLGA_COLS,                          30,
            DLGA_LBELEMENTS,                    elements,
            DLGA_ALLOW_FILLX,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Search:",
                    DLGA_ALIGN_W,
                    DLGA_COLS,                  14,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Replace:",
                    DLGA_ALIGN_W,
                DLGC_SPACER,
                    DLGA_ALIGN_W,
                    DLGA_ROWS,                  1,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Password:",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Line Number:",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "search",
                    DLGA_COLS,                  30,
                    DLGA_ALLOW_FILLX,
                    DLGA_EDPLACEHOLDER,         "<search expression>",
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "replacement",
                    DLGA_COLS,                  30,
                    DLGA_ALLOW_FILLX,
                    DLGA_EDPLACEHOLDER,         "<replacement text>",
                DLGC_SPACER,
                    DLGA_ALIGN_W,
                    DLGA_ROWS,                  1,
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "password",
                    DLGA_COLS,                  30,
                    DLGA_ALLOW_FILLX,
                    DLGA_EDVISIBLITY,           FALSE,
                    DLGA_EDPLACEHOLDER,         "<password>",
                DLGC_NUMERIC_FIELD,
                    DLGA_ALIGN_W,
                    DLGA_NAME,                  "linenumber",
                    DLGA_VALUE,                 0,
                    DLGA_COLS,                  12,
            DLGC_END,
        DLGC_END,

        DLGC_SEPARATOR_HORIZONTAL,
            DLGA_ATTACH_TOP,
            DLGA_COLS,                          40,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Read-only",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Basic Combo",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Append",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "&Suggest",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "AppendSuggest",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Unicode index",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Shortcuts",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_0",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "lemon",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBELEMENTS,            elements,
                    DLGA_LBINDEXMODE,           FALSE,
                    DLGA_LBPAGEMODE,            FALSE,
                    DLGA_CBEDITABLE,            FALSE,      // Unable to modify
                    DLGA_CBAUTOCOMPLETEMODE,    0,          // None

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_1",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "tomato",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBSORTMODE,            1,
                    DLGA_LBELEMENTS,            elements,
                    DLGA_LBINDEXMODE,           FALSE,
                    DLGA_LBPAGEMODE,            FALSE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable
                    DLGA_CBAUTOCOMPLETEMODE,    0,          // None

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_2",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "olive",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBSORTMODE,            -1,
                    DLGA_LBELEMENTS,            elements,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            FALSE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable
                    DLGA_CBAUTOCOMPLETEMODE,    1,          // Append

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_3",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "apple",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBELEMENTS,            elements,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Sugguest

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_4",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "unknown",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBELEMENTS,            elements,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                    DLGA_CBRELAXMODE,           2,          // Any value

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_5",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "two",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBELEMENTS,            quote_list("one", "two", "three", "four", "five", "six", "seven",
                                                    "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                    DLGA_CBRELAXMODE,           2,          // Any value

                DLGC_COMBO_FIELD,
                    DLGA_NAME,                  "combo_6",
                    DLGA_ALIGN_W,
                    DLGA_VALUE,                 "Yes",
                    DLGA_COLS,                  20,
                    DLGA_AUTOMOVE,              TRUE,
                    DLGA_LBELEMENTS,            quote_list("Y\001Yes", "N\001No", "O\001Once", "*\001Maybe"),
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,       // Modifiable

            DLGC_END,
        DLGC_END,

//      DLGC_TAB,
//          DLGA_NAME,                          "tab_1",
//          DLGC_COMBO_FIELD,
//              DLGA_NAME,                      "tab_1.1",
//              DLGA_VALUE,                     "1234",
//              DLGA_LBELEMENTS,                elements,
//              DLGA_COLS,                      5,
//              DLGA_ROWS,                      1,
//              DLGA_CBAUTOCOMPLETEMODE,        1,          // Append
//              DLGA_ALIGN_W,
//          DLGC_COMBO_FIELD,
//              DLGA_NAME,                      "tab_1.2",
//              DLGA_VALUE,                     "123456",
//              DLGA_LBELEMENTS,                elements,
//              DLGA_COLS,                      8,
//              DLGA_ROWS,                      1,
//              DLGA_CBAUTOCOMPLETEMODE,        2,          // Append
//              DLGA_ALIGN_W,
//          DLGC_EDIT_FIELD,
//              DLGA_LABEL,                     "tab_1.3",
//              DLGA_COLS,                      12,
//              DLGA_ROWS,                      1,
//              DLGA_ALIGN_W,
//      DLGC_END,

        DLGC_CONTAINER,
            DLGA_NAME,                          "buttons_1",
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&OK",
                DLGA_NAME,                      "ok",
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "e&Xit",
                DLGA_NAME,                      "exit",
            DLGC_PUSH_BUTTON,
                DLGA_ATTACH_RIGHT,
                DLGA_LABEL,                     "&Cancel",
                DLGA_NAME,                      "cancel",
                DLGA_CANCEL_BUTTON,
                DLGA_DEFAULT_BUTTON,
        DLGC_END
        ));
}


static int
callback(int ident, string name, int p1, int p2)
{
    message("ident=0x%x, name=%s, p1=%d/0x%x, p2=%d/0x%x", ident, name, p1, p1, p2, p2);
    if (name == "exit") {
        dialog_exit();
        return TRUE;
    }
    return FALSE;
}

//end
