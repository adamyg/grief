/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: bufflags.cr,v 1.5 2014/10/27 23:28:18 ayoung Exp $
 * Buffer flags, non UI interface.
 *
 *
 */

#include "grief.h"

#define BFTYP_LIST                              /* these *must* match btypeindex() */ \
    {"UNIX", "DOS", "MAC", "ANSI", "BINARY", "UTF8", "UTF16", "UTF32", "Unknown"}

static list             flags_offon = {
    "Off",
    "On "
    };

static list             flag_fields = {
    "Buffer is executable ............. (ro) ", "exec",
    "Buffer is in binary mode ......... (ro) ", "binary",
    "Buffer is system buffer .......... (ro) ", "sysbuf",
    "Buffer attached to process ....... (ro) ", "process",
    "Buffer creates new file .......... (ro) ", "newfile",
    "Buffer read in ................... (ro) ", "read",
    "Color attributes ................. (ro) ", "attributes",
    "Dialog resource .................. (ro) ", "dialog",

    "Buffer has been modified ......... (rw) ", "changed",
    "Backup not yet backed up ......... (rw) ", "backup",
    "Buffer is readonly ............... (rw) ", "readonly",
    "Buffer is volatile ............... (rw) ", "volatile",
    "Buffer uses hard tabs ............ (rw) ", "tabs",
    "Buffer has no undo ............... (rw) ", "noundo",
    "Buffer has syntax hilighting ..... (rw) ", "syntax",
    "ANSI hilighting enabled .......... (rw) ", "ansi",
    "MAN hilighting enabled ........... (rw) ", "man",
    "Buffer displays ruler ............ (rw) ", "ruler",
    "Spell check ...................... (rw) ", "spell",
//  "Folding .......................... (rw) ", "folding",
    "Cursor row crosshair ............. (rw) ", "cursor_row",
    "Cursor column crosshair .......... (rw) ", "cursor_col",
    "Limit cursor to EOL .............. (rw) ", "eol_cursor",
    "Limit to EOF ..................... (rw) ", "eof_cursor",
    "Has line numbers ................. (rw) ", "line_numbers",
    "Display old lines ................ (rw) ", "line_oldnumbers",
    "Markup modified lines ............ (rw) ", "line_status",
    "Label using full path name ....... (rw) ", "title_full",
    "Scroll title with window ......... (rw) ", "title_scroll",
    "Left justify title ............... (rw) ", "title_left",
    "Right justify title .............. (rw) ", "title_right",
    "Read-only suffix ................. (rw) ", "suffix_readonly",
    "Modified suffix .................. (rw) ", "suffix_modified",
    "Limit hilites to EOL ............. (rw) ", "eol_hilite",
    "<EOF> display .................... (rw) ", "eof",
    "Show <~> at EOF .................. (rw) ", "tilde",
    "Hilite literal characters ........ (rw) ", "hiliteral",
    "Hilite whitespace ................ (rw) ", "hiwhitespace",
    "Hilite modified lines ............ (rw) ", "himodified",
    "Hilite additional lines .......... (rw) ", "hiadditional",
//  "Auto-save ........................ (rx) ", "autosave",
//  "Auto-indent ...................... (rx) ", "autoindent",
//  "Auto-wrap ........................ (rx) ", "autowrap",
//  "Paste mode ....................... (rx) ", "paste_mode",
    "Buffer uses CR/LF newline ........ (rw) ", "ocvt_crmode",
    "Output, trim trailing whitespace . (rw) ", "ocvt_trimwhite"
    };

static list             type_values =
    BFTYP_LIST;

static list             type_fields = {
    "Buffer type: ",  BFTYP_LIST
    };

#if defined(__PROTOTYPES__)
static void             buffer_type_gui(void);
static void             buffer_type_text(void);
static int              btypeindex(void);
static void             btypeset(int idx, int bufnum);
#endif


void
toggle_buffer_flags(void)
{
    list rfields, rvalues, results;
    string field;
    int idx;

    while ((idx = list_each(flag_fields, field, 2)) >= 0) {
        push(rfields, field);
        push(rfields, flags_offon);
        rvalues += (inq_buffer_flags(NULL, flag_fields[(idx * 2) + 1]) ? 1 : 0);
    }
    push(rfields, "Buffer type ...................... (rw) ");
    push(rfields, type_values);
    rvalues += btypeindex();

    results = field_list("Buffer Flags", rvalues, rfields, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        message("No change ..");
    } else {
        while ((idx = list_each(flag_fields, field, 2)) >= 0) {
            if (re_search(SF_NOT_REGEXP, "(rw)", field) > 0) {
                if (rvalues[idx] != results[idx]) {
                    if (results[idx]) {         // set
                        set_buffer_flags(NULL, flag_fields[(idx * 2) + 1], NULL);
                    } else {                    // clear
                        set_buffer_flags(NULL, NULL, flag_fields[(idx * 2) + 1]);
                    }
                }
            }
        }
        idx = length_of_list(rvalues) - 1;
        if (rvalues[idx] != results[idx]) {
            btypeset(results[idx], inq_buffer());
        }
        message("done ..");
    }
}


void
toggle_buffer_type(void)
{
    if (DC_ASCIIONLY & inq_display_mode()) {
        buffer_type_text();
    } else {
        buffer_type_gui();
    }
}


static void
buffer_type_gui(void)
{
    int curbuf = inq_buffer();
    int dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Buffer Type",
        DLGA_CALLBACK,                          "::bt_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Buffer type",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
            DLGC_END,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "buffertype",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBICASESTRINGS,        TRUE,
                    DLGA_LBELEMENTS,            type_values,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_LBINDEXMODE,           TRUE,
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

    UNUSED(curbuf);
    widget_set(dialog, "buffertype", btypeindex(), DLGA_LBACTIVE);
    dialog_run(dialog);
    dialog_delete(dialog);
}


static int
bt_callback(int ident, string name, int p1, int p2)
{
    extern int curbuf;

    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_BUTTON:
        switch(name) {
        case "done":
            dialog_exit();
            break;
        case "apply": {
                int buffertype;

                if ((buffertype = widget_get(NULL, "buffertype", DLGA_LBACTIVE)) >= 0) {
                    btypeset(buffertype, curbuf);
                }
                dialog_exit();
            }
            break;
        case "help":
            execute_macro("explain toggle_buffer_type");
            break;
        }
    }
    return TRUE;
}


static void
buffer_type_text(void)
{
    list results;

    results[0] = btypeindex();
    results = field_list("Buffer Type", results, type_fields, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        message("No change ..");
        return;                                 // <Esc>
    }
    btypeset(results[0], inq_buffer());
}


static int
btypeindex(void)
{
    switch(inq_buffer_type()) {
    case BFTYP_UNIX:   return 0;
    case BFTYP_DOS:    return 1;
    case BFTYP_MAC:    return 2;
    case BFTYP_ANSI:   return 3;
    case BFTYP_BINARY: return 4;
    case BFTYP_UTF8:   return 5;
    case BFTYP_UTF16:  return 6;
    case BFTYP_UTF32:  return 7;
    }
    return 8;
}


static void
btypeset(int idx, int bufnum)
{
    int btype = BFTYP_UNKNOWN;

    switch(idx) {
    case 0: btype = BFTYP_UNIX;   break;
    case 1: btype = BFTYP_DOS;    break;
    case 2: btype = BFTYP_MAC;    break;
    case 3: btype = BFTYP_ANSI;   break;
    case 4: btype = BFTYP_BINARY; break;
    case 5: btype = BFTYP_UTF8;   break;
    case 6: btype = BFTYP_UTF16;  break;
    case 7: btype = BFTYP_UTF32;  break;
    }
    set_buffer_type(bufnum, btype);
}

/*end*/
