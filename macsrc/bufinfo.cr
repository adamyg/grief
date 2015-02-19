/* -*- indent-width: 4; -*- */
/* $Id: bufinfo.cr,v 1.16 2014/11/27 15:54:09 ayoung Exp $
 * Buffer Information/Configuration.
 *
 *
 */

#include "grief.h"

#define IDENT_BASE          1000

enum {
        // flag field labels order *MUST* match flagset
        IDENT_ANSI = IDENT_BASE,
        IDENT_AUTOINDENT,
        IDENT_AUTOSAVE,
        IDENT_AUTOWRAP,
        IDENT_BACKUP,
        IDENT_CHANGED,
        IDENT_CURSOR_COL,
        IDENT_CURSOR_ROW,
        IDENT_EOF,
        IDENT_EOF_CURSOR,
        IDENT_EOL_CURSOR,
        IDENT_EOL_HILITE,
        IDENT_HILITERAL,
        IDENT_HIWHITESPACE,
        IDENT_LINE_NUMBERS,
        IDENT_LINE_OLDNUMBERS,
        IDENT_LINE_STATUS,
        IDENT_LOCK,
        IDENT_MAN,
        IDENT_NOUNDO,
        IDENT_OCVT_CRMODE,
        IDENT_OCVT_TRIMWHITE,
        IDENT_READONLY,
        IDENT_RULER,
        IDENT_SPELL,
        IDENT_SUFFIX_MODIFIED,
        IDENT_SUFFIX_READONLY,
        IDENT_SYNTAX,
        IDENT_TABS,
        IDENT_TILDE,
        IDENT_TITLE_FULL,
        IDENT_TITLE_LEFT,
        IDENT_TITLE_RIGHT,
        IDENT_TITLE_SCROLL,
        IDENT_VOLATILE,
        };

static list                 flagset = {
        // flag names *MUST* match above enumeration
        "ansi",
        "autoindent",
        "autosave",
        "autowrap",
        "backup",
        "changed",
        "cursor_col",
        "cursor_row",
        "eof",
        "eof_cursor",
        "eol_cursor",
        "eol_hilite",
        "hiliteral",
        "hiwhitespace",
        "line_numbers",
        "line_oldnumbers",
        "line_status",
        "lock",
        "man",
        "noundo",
        "ocvt_crmode",
        "ocvt_trimwhite",
        "readonly",
        "ruler",
        "spell",
        "suffix_modified",
        "suffix_readonly",
        "syntax",
        "tabs",
        "tilde",
        "title_full",
        "title_left",
        "title_right",
        "title_scroll",
        "volatile",
        };

static int                  dialog;

//  File Name:   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//  Buffer Name: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//  Permissions: xxxxxxxxxxxxxxxxxxx       Size:        xxxxxxxxxxxxxxxx
//  Timestamp:   xxxxxxxxxxxxxxxxxxx       Edit:        xxxxxxxxxxxxxxxx
//  File Type:   xxxxxxxxxxxxxxxxxxx       Terminator:  xxxxxxxxxxxxxxxx
//  Char Map:    xxxxxxxxxxxxxxxxxxx
//  Backup:      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//     Versions: xxxxx      Limit: xxx/xxx
//  Margins:     xxxx/xxxx  Style: xxx  Color Column: xxxxxxx
//
//       [] Flag         [] Flag           [] Flag           [] Flag
//            :               :                 :                :
//
//                      < Done >  < Apply >  < Help >
//
void
main()
{
    dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Buffer Information and Options",
        DLGA_CALLBACK,                          "::bi_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_PADX,                          1,
            DLGA_PADY,                          1,
            DLGA_ALIGN_W,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_VALUE,             "File Name:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "buffername",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              60,
                        DLGA_ALLOW_FILLX,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Buffer Name:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "filename",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              60,
                        DLGA_ALLOW_FILLX,
                        DLGA_GREYED,
                DLGC_END,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADX,                          1,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGC_CONTAINER,
                    DLGA_ATTACH_TOP,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Permissions:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "permissions",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              30,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_TOP,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Timestamp:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "disktime",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              30,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_TOP,
                    DLGC_LABEL,
                        DLGA_VALUE,             "File Type:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "filetype",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              30,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_TOP,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Char Map:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "charactermap",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              30,
                        DLGA_GREYED,
                DLGC_END,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Size:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "size",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              18,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Edit:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "edittime",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              18,
                        DLGA_GREYED,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGC_LABEL,
                        DLGA_VALUE,             "Terminator:",
                        DLGA_ATTACH_LEFT,
                        DLGA_COLS,              13,
                    DLGC_EDIT_FIELD,
                        DLGA_NAME,              "terminator",
                        DLGA_ATTACH_LEFT,
                        DLGA_ROWS,              1,
                        DLGA_COLS,              18,
                        DLGA_GREYED,
                DLGC_END,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADX,                          1,
            DLGC_CONTAINER,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Backup:",
                    DLGA_ATTACH_LEFT,
                    DLGA_COLS,                  13,
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "bk_dir",
                    DLGA_ATTACH_LEFT,
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  24,
                    DLGA_ALLOW_FILLX,
                    DLGA_GREYED,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADX,                          2,
            DLGC_CONTAINER,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Versions:",
                    DLGA_ATTACH_LEFT,
                    DLGA_COLS,                  12,
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "bk_versions",
                    DLGA_ATTACH_LEFT,
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  12,
                    DLGA_GREYED,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Limits:",
                    DLGA_ATTACH_LEFT,
                    DLGA_COLS,                  8,
                DLGC_EDIT_FIELD,
                    DLGA_NAME,                  "limits",
                    DLGA_ATTACH_LEFT,
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  16,
                    DLGA_GREYED,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADX,                          1,
            DLGC_LABEL,
                DLGA_VALUE,                     "Margins:",
                DLGA_ATTACH_LEFT,
                DLGA_COLS,                      13,
            DLGC_EDIT_FIELD,
                DLGA_NAME,                      "marginl",
                DLGA_ATTACH_LEFT,
                DLGA_ROWS,                      1,
                DLGA_COLS,                      4,
            DLGC_LABEL,
                DLGA_VALUE,                     " - ",
                DLGA_ATTACH_LEFT,
            DLGC_EDIT_FIELD,
                DLGA_NAME,                      "marginr",
                DLGA_ATTACH_LEFT,
                DLGA_ROWS,                      1,
                DLGA_COLS,                      4,
            DLGC_LABEL,
                DLGA_VALUE,                     " ",
                DLGA_ATTACH_LEFT,
            DLGC_LABEL,
                DLGA_VALUE,                     "Style:",
                DLGA_ATTACH_LEFT,
                DLGA_COLS,                      8,
            DLGC_EDIT_FIELD,
                DLGA_NAME,                      "margins",
                DLGA_ATTACH_LEFT,
                DLGA_ROWS,                      1,
                DLGA_COLS,                      4,
            DLGC_LABEL,
                DLGA_VALUE,                     " ",
                DLGA_ATTACH_LEFT,
            DLGC_LABEL,
                DLGA_VALUE,                     "Color Column:",
                DLGA_ATTACH_LEFT,
                DLGA_COLS,                      14,
            DLGC_EDIT_FIELD,
                DLGA_NAME,                      "colorcolumn",
                DLGA_ATTACH_LEFT,
                DLGA_ROWS,                      1,
                DLGA_COLS,                      4,
        DLGC_END,

        DLGC_GROUP,
            DLGA_ATTACH_TOP,
            DLGC_CONTAINER,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Auto-indent",
                        DLGA_IDENT,             IDENT_AUTOINDENT,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Auto-save",
                        DLGA_IDENT,             IDENT_AUTOSAVE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Auto-wrap",
                        DLGA_IDENT,             IDENT_AUTOWRAP,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Backups",
                        DLGA_IDENT,             IDENT_BACKUP,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "No Undo",
                        DLGA_IDENT,             IDENT_NOUNDO,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Syntax hilite",
                        DLGA_IDENT,             IDENT_SYNTAX,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Hilite Whitespace",
                        DLGA_IDENT,             IDENT_HIWHITESPACE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Hilite Literal",
                        DLGA_IDENT,             IDENT_HILITERAL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOL Hilite",
                        DLGA_IDENT,             IDENT_EOL_HILITE,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Ruler",
                        DLGA_IDENT,             IDENT_RULER,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Line numbers",
                        DLGA_IDENT,             IDENT_LINE_NUMBERS,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Old line numbers",
                        DLGA_IDENT,             IDENT_LINE_OLDNUMBERS,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Line status",
                        DLGA_IDENT,             IDENT_LINE_STATUS,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOF markers",
                        DLGA_IDENT,             IDENT_EOF,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Tilde markers",
                        DLGA_IDENT,             IDENT_TILDE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "ANSI Mode",
                        DLGA_IDENT,             IDENT_ANSI,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "MAN Mode",
                        DLGA_IDENT,             IDENT_MAN,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Hard Tabs",
                        DLGA_IDENT,             IDENT_TABS,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Spell",
                        DLGA_IDENT,             IDENT_SPELL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Read-Only",
                        DLGA_IDENT,             IDENT_READONLY,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Modified",
                        DLGA_IDENT,             IDENT_CHANGED,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Lock",
                        DLGA_IDENT,             IDENT_LOCK,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Volatile",
                        DLGA_IDENT,             IDENT_VOLATILE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Cursor Row",
                        DLGA_IDENT,             IDENT_CURSOR_ROW,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Cursor Col",
                        DLGA_IDENT,             IDENT_CURSOR_COL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOL Cursor",
                        DLGA_IDENT,             IDENT_EOL_CURSOR,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "EOF Cursor",
                        DLGA_IDENT,             IDENT_EOF_CURSOR,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Title Scroll",
                        DLGA_IDENT,             IDENT_TITLE_SCROLL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Title Left",
                        DLGA_IDENT,             IDENT_TITLE_LEFT,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Title Right",
                        DLGA_IDENT,             IDENT_TITLE_RIGHT,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Title Full",
                        DLGA_IDENT,             IDENT_TITLE_FULL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Read-only suffix",
                        DLGA_IDENT,             IDENT_SUFFIX_READONLY,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Modified suffix",
                        DLGA_IDENT,             IDENT_SUFFIX_MODIFIED,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "LF/CR",
                        DLGA_IDENT,             IDENT_OCVT_CRMODE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "Trim whitespace",
                        DLGA_IDENT,             IDENT_OCVT_TRIMWHITE,
                        DLGA_ALIGN_W,
                DLGC_END,
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
bufinfo()
{
    int curbuf = inq_buffer();
    int curwin = inq_window();

    UNUSED(curbuf, curwin);
    dialog_run(dialog);
}


static int
bi_callback(int ident, string name, int p1, int p2)
{
    extern int curbuf, curwin;                  // current buffer

    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT: {
            //
            //  populate tab
            //
            string filename, desc, encoding, termbuf, terminator, charactermap, limits;
            int size, mtime, mode, edittime;
            int buftype, termtype;
            int marginl, marginr, margins, colorcolumn;
            int i;

            // name, permission and time-stamps.
            set_buffer(curbuf);
            inq_names(filename);

            stat(NULL /*filename*/, size, mtime, NULL, NULL, mode);
            widget_set(NULL, "filename", filename);
            widget_set(NULL, "buffername", inq_buffer_title());
            widget_set(NULL, "permissions", mode_string(mode));
            widget_set(NULL, "size", size);

            inq_time(inq_buffer(), edittime);
            widget_set(NULL, "disktime", strftime(NULL, mtime));
            widget_set(NULL, "edittime", (edittime ? strftime(NULL, edittime) : "n.a."));

            // file-type.
            buftype = inq_buffer_type(curbuf, desc, encoding);
            if (strlen(encoding) && encoding != desc) {
                widget_set(NULL, "filetype", desc + " (" + buftype + ") - " + encoding);
            } else {
                widget_set(NULL, "filetype", desc + " (" + buftype + ")");
            }

            // terminator.
            termtype = inq_terminator(curbuf, termbuf);
            if (strlen(termbuf)) {
                for (i = 1; i <= strlen(termbuf); ++i) {
                    terminator += format("0x%02X", characterat(termbuf, i));
                }
                terminator += format(" (%d)", termtype);
            } else {
                terminator = format("NUL (%d)", termtype);
            }
            widget_set(NULL, "terminator", terminator);

            // character-map.
            inq_char_map(curwin, charactermap);
            widget_set(NULL, "charactermap", charactermap);

            // backup.
            widget_set(NULL, "bk_dir", inq_backup_option(BK_DIR));
            widget_set(NULL, "bk_versions", inq_backup_option(BK_VERSIONS));
            sprintf(limits, "%d/%d",
                inq_backup_option(BK_ASK), inq_backup_option(BK_DONT));
            widget_set(NULL, "limits", limits);

            // margins.
            inq_margins(curbuf, marginl, marginr, margins, colorcolumn);
            widget_set(NULL, "marginl", marginl);
            widget_set(NULL, "marginr", marginr);
            widget_set(NULL, "margins", margins);
            widget_set(NULL, "colorcolumn", colorcolumn);

            // flags.
            for (i = 0; i < length_of_list(flagset); ++i) {
                widget_set(NULL, IDENT_BASE + i, inq_buffer_flags(NULL, flagset[i]));
            }
        }
        break;

    case DLGE_BUTTON:
        switch(name) {
        case "apply": {
                int i;

                set_margins(curbuf,             // margins
                    atoi(widget_get(NULL, "marginl")),
                    atoi(widget_get(NULL, "marginr")),
                    atoi(widget_get(NULL, "margins")));
                //  atoi(widget_get(NULL, "colorcolumn")));

                                                // set/clear flags
                for (i = 0; i < length_of_list(flagset); ++i) {
                    if (widget_get(NULL, IDENT_BASE + i)) {
                        set_buffer_flags(curbuf, flagset[i], NULL);
                    } else {
                        set_buffer_flags(curbuf, NULL, flagset[i]);
                    }
                }

                redraw();
            }
            break;

        case "done":
            dialog_exit();
            break;

        case "help":
            execute_macro("explain set_buffer_flags");
            break;
        }
    }
    return TRUE;
}

/*eof*/
