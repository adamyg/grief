/* -*- indent-width: 4; -*- */
/* $Id: spell.cr,v 1.10 2014/10/27 23:28:28 ayoung Exp $
 * Spell checker.
 *
 *
 */

#include "grief.h"

static int              sp_callback(int ident, string name, int p1, int p2);
static int              sp_word(int idx);

/*  todo Popup  ...
//
//      Ctrl-S  - Spell menu
//      Ctrl-Q  - Quick menu
//
//      [suggestions ... ]
//      [----------------]
//      Add to dictionary
//      Ignore once
//      Languages       >>
//              [x] English (UK)    en_GB
//              [x] English (US)    en_US
//              [x] French
//              [x] German
//              [x] Italian
//                      :
//      Copy
//      Paste
*/

void
spell()
{
    int dialog = dialog_create( make_list(
        DLGA_TITLE,                     "Spell",
        DLGA_CALLBACK,                  "::sp_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_LEFT,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_NW,
                DLGC_LABEL,
                    DLGA_LABEL,         "Word",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "&Replace",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "&Suggestions",
                    DLGA_COLS,          12,
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_ALIGN_NE,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "word",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                    DLGA_GREYED,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "replace",
                    DLGA_ROWS,          1,
                    DLGA_HOTKEY,        'r',    // Alt-R
                    DLGA_ALLOW_FILLX,
                DLGC_LIST_BOX,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "suggest",
                    DLGA_ROWS,          7,
                    DLGA_COLS,          30,
                    DLGA_HOTKEY,        's',    // Alt-S
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_RIGHT,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&No Ignore",
                DLGA_NAME,              "ignore",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Ignore All",
                DLGA_NAME,              "ignoreall",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Add",
                DLGA_NAME,              "add",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Change",
                DLGA_NAME,              "change",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "Change &Global",
                DLGA_NAME,              "changeglobal",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Done",
                DLGA_NAME,              "done",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Help",
                DLGA_NAME,              "help",
                DLGA_ALLOW_FILLX,
        DLGC_END
        ));
    list errlst;

    message("Checking document ...");
    errlst = spell_buffer(current_line, inq_lines(), TRUE, FALSE);
    if (length_of_list(errlst)) {
        int erridx = -1, curbuf = inq_buffer();

        UNUSED(erridx, curbuf);
        message("check complete, unknown words encountered.");
        dialog_run(dialog);
    } else {
        message("check complete, no errors reported.");
    }
    dialog_delete(dialog);
}


static int
sp_callback(int ident, string name, int p1, int p2)
{
    extern int erridx;

    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        sp_word(++erridx);
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "ignore":
            sp_word(++erridx);
            break;
        case "ignoreall":
            sp_word(++erridx);
            break;
        case "done":
            dialog_exit();
            break;
        case "help":
            execute_macro("explain spell");
            break;
        }
    }
    return TRUE;
}


static int
sp_word(int idx)
{
    extern list errlst;                         // working spell list
    int wordidx = idx * 6;

    widget_set(NULL, "suggest", NULL, DLGA_LBELEMENTS);
    if (wordidx < length_of_list(errlst)) {
        //
        //   0        1                    2         3         4         5
        //  [<word>, <suggest-list|NULL>, <offset>, <column>, <line> [, <count>]]
        //      :       :                    :         :         :         :
        //
        list suglst = spell_suggest(errlst[wordidx]);

        widget_set(NULL, "word", errlst[wordidx]);
        if (is_list(suglst) && length_of_list(suglst)) {
            widget_set(NULL, "replace", suglst[0]);
            widget_set(NULL, "suggest", suglst, DLGA_LBELEMENTS);
            message("<%s> select suggestion and action", errlst[wordidx]);

        } else {
            widget_set(NULL, "replace", "");
            message("<%s> no suggestions", errlst[wordidx]);
        }
        return TRUE;
    }

    widget_set(NULL, "word", "");
    widget_set(NULL, "replace", "");
    message("");
    return FALSE;
}


void
spellbuffer()
{
    int buf, win, curbuf = inq_buffer();
    int tokensize = TRUE, suggest = TRUE;
    list errlst;

    errlst = spell_buffer(1, inq_lines(), tokensize, suggest);
    buf = create_buffer("spell-check", NULL, 1);
    set_buffer(buf);
    if (length_of_list(errlst)) {
        //
        //   0        1                    2         3         4         5
        //  [<word>, <suggest-list|NULL>, <offset>, <column>, <line> [, <count>]]
        //      :       :                    :         :         :         :
        //
        int idx;

        for (idx = 0; idx < length_of_list(errlst); idx += 6) {
            declare suglst = errlst[idx+1];

            insertf("<%s> [%d], line/col:%d-%d, count:%d\n",
                errlst[idx+0], errlst[idx+2], errlst[idx+4], errlst[idx+3], errlst[idx+5]);
            if (is_list(suglst)) {
                int s;

                for (s = 0; s < length_of_list(suglst); s++) {
                    insertf("\t[%s]\n", suglst[s]);
                }
            } else {
                insert("\tno suggestions\n");
            }
        }
    } else {
        insert("no words\n");
    }
    set_buffer(curbuf);
    win = sized_window(inq_lines(), 60);
    select_buffer(buf, win, 0);
    delete_buffer(buf);
}


void
spelltest()
{
    int buf, win, curbuf = inq_buffer();
    list errlst;

    errlst = spell_string(read(), NULL, 1, TRUE);
    buf = create_buffer("spell-check", NULL, 1);
    set_buffer(buf);
    if (length_of_list(errlst)) {
        //
        //   0        1                    2         3
        //  [<word>, <suggest-list|NULL>, <offset>, <column>]
        //      :       :                    :         :
        //
        int idx;

        for (idx = 0; idx < length_of_list(errlst); idx += 4) {
            declare suglst = errlst[idx + 1];

            insertf("<%s> [%d], col:%d\n",
                errlst[idx + 0], errlst[idx + 2], errlst[idx + 3]);
            if (is_list(suglst)) {
                int s;

                for (s = 0; s < length_of_list(suglst); ++s) {
                    insertf("\t[%s]\n", suglst[s]);
                }
            } else {
                insert("\tno suggestions\n");
            }
        }
    } else {
        insert("no words\n");
    }
    set_buffer(curbuf);
    win = sized_window(inq_lines(), 60);
    select_buffer(buf, win, 0);
    delete_buffer(buf);
}

/*eof*/
