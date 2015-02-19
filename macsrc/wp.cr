/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: wp.cr,v 1.20 2014/10/27 23:28:30 ayoung Exp $
 * Word processing features/options.
 *
 *
 */

#include "grief.h"
#include "wp.h"

/* Default left and right margins */
#define LMARGIN         1
#define RMARGIN         70

/* Width of C comment if its indented. */
#define C_IMARGIN       32                      // Idented margin.
#define C_LMARGIN       4                       // Left hand margin after open comment.
#define C_RMARGIN       4                       // Right hand margin before close comment.

#if defined(__PROTOTYPES__)
static void             wp_options_gui(void);
static void             wp_options_text(void);
static void             wp_margins_set(int style, int left, int right);

static void             reg_autowrap(void);
static void             format_list(void);

void                    c_uncomment_block(void);
void                    c_format_block(void);
int                     default_format_block(void);
#endif  //__PROTOTYPES__

static int              wp_autowrap;
static int              wp_autoindent;
//  static int          wp_margins;             // Margin style.
//  static int          wp_marginr;             // Right margin.
//  static int          wp_marginl;             // Left margin.
static int              wp_quiet;

/* Flag set if we have nroff-style file (so we know that dot
 * commands terminate a paragraph).
 */
static int              nroff_file = TRUE;

/* Following variable set to TRUE whilst doing a C comment.
 *  When this is TRUE the right margin moves with the left.
 *  When FALSE, the right margin is a constant.
 */
static int              wp_cstyle = FALSE;


void
main(void)
{
    wp_autowrap   = FALSE;
    wp_autoindent = TRUE;
//  wp_margins_set(FALSE, LMARGIN, RMARGIN);
    set_margins(-1, LMARGIN, RMARGIN, JUSTIFIED);
    wp_quiet      = FALSE;
}


void
wp_options(void)
{
    if (DC_ASCIIONLY & inq_display_mode()) {
        wp_options_text();
    } else {
        wp_options_gui();
    }
}


static void
wp_options_gui(void)
{
    int dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Document Options",
        DLGA_CALLBACK,                          "::wp_callback",

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Auto-wrap",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Auto-indent",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Alignment",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Left margin",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Right margin",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "autowrap",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("Off", "On"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "autoindent",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("Off", "On"),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "margins",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                    DLGA_LBELEMENTS,            quote_list("Justified" /*0*/, "Ragged" /*1*/),
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    2,          // Append
                DLGC_NUMERIC_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_ALIGN_W,
                    DLGA_NAME,                  "marginl",
                    DLGA_COLS,                  16,
                 // DLGA_EDNUMMIN,              1,
                 // DLGA_EDNUMMAX,              999,
                DLGC_NUMERIC_FIELD,
                    DLGA_ATTACH_TOP,
                    DLGA_NAME,                  "marginr",
                    DLGA_COLS,                  16,
                    DLGA_ALIGN_W,
                 // DLGA_EDNUMMIN,              1,
                 // DLGA_EDNUMMAX,              999,
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

    int marginl, marginr, margins;

    inq_margins(NULL, marginl, marginr, margins);
    widget_set(dialog, "autowrap", wp_autowrap, DLGA_LBACTIVE);
    widget_set(dialog, "autoindent", wp_autoindent, DLGA_LBACTIVE);
    widget_set(dialog, "margins", margins, DLGA_LBACTIVE);
    widget_set(dialog, "marginl", marginl);
    widget_set(dialog, "marginr", marginr);
    dialog_run(dialog);
    dialog_delete(dialog);
}


static int
wp_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_BUTTON:
        switch (name) {
        case "done":
            dialog_exit();
            break;
        case "apply": {
                int nautowrap, nautoindent, nmargins, nmarginr, nmarginl;

                nautowrap   = widget_get(NULL, "autowrap", DLGA_LBACTIVE);
                nautoindent = widget_get(NULL, "autoindent", DLGA_LBACTIVE);
                nmargins    = widget_get(NULL, "margins", DLGA_LBACTIVE);
                nmarginl    = atoi(widget_get(NULL, "marginl"));
                nmarginr    = atoi(widget_get(NULL, "marginr"));
//              wp_margins_set(nmargins, nmarginl, nmarginr);
                set_margins(-1, nmarginl, nmarginr, nmargins);
                autowrap(nautowrap ? "y" : "n");
                autoindent(nautoindent ? "y" : "n");
                dialog_exit();
            }
            break;
        case "help":
            execute_macro("explain wp_options");
            break;
        }
    }
    return TRUE;
}


static void
wp_options_text(void)
{
    static list wp_list;
    int marginl, marginr, margins;
    list results;

    if (0 == length_of_list(wp_list)) {
        wp_list = make_list(
                    "Auto-wrap      : ", quote_list("Off", "On"),
                    "Auto-indent    : ", quote_list("Off", "On"),
                    "Margins        : ", quote_list("Justified", "Ragged"),
                    "Left Margin    : ", "",
                    "Right Margin   : ", ""
                    );
    }

    inq_margins(-1, marginl, marginr, margins); /* global */
    results[0] = wp_autowrap;
    results[1] = wp_autoindent;
    results[2] = margins;
    results[3] = "" + marginl;
    results[4] = "" + marginr;
    results = field_list("Document Options", results, wp_list, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        return;
    }
    set_margins(-1, results[2], atoi(results[3]), atoi(results[4]));
    autowrap(results[0] ? "y" : "n");
    autoindent(results[1] ? "y" : "n");
}


//  static void
//  wp_margins_set(int style, int left, int right)
//  {
//      wp_margins = style;
//      if (right > 1) {
//          wp_marginr = right;
//          if (left >= 1 && left < right) {
//              wp_marginl = left;
//          }
//      }
//      set_margins(-1, wp_marginl, wp_marginr, wp_margins);
//  }


void
_griset_autowrap(string arg)
{
    autowrap(arg);
}


string
_griget_autowrap(void)
{
    return (wp_autowrap ? "yes" : "no");
}


void
_griset_autoindent(string arg)
{
    autoindent(arg);
}


string
_griget_autoindent(void)
{
    return (wp_autoindent ? "yes" : "no");
}


void
_griset_justification(string arg)
{
    set_margins(-1, NULL, NULL, ("yes" == arg ? JUSTIFIED : RAGGED));
}


string
_griget_justification(void)
{
    int margins;

    inq_margins(-1, NULL, NULL, margins);       /* global */
    return (margins ? "no" : "yes");
}


void
_griset_margin(string to)
{
    list parts = split(to, ",", TRUE);
    int marginl, marginr;

    if (length_of_list(parts) >= 2) {
        marginl = parts[0];
        marginr = parts[1];
    } else if (length_of_list(parts) >= 1) {
        marginl = 1;
        marginr = parts[0];
    }
    set_margins(-1, marginl, marginr, NULL);
}


string
_griget_margin(void)
{
    int marginl, marginr;

    inq_margins(-1, marginl, marginr);          /* global */
    return format("%d,%d", marginl, marginr);
}


int
autowrap(~string arg)
{
    int echo = TRUE, old_autowrap = wp_autowrap;
    int marginr;

    if (strlen(arg)) {
        wp_autowrap = (lower(substr(arg, 1, 1)) == "y") ? 1 : 0;
        echo = FALSE;
    } else {
        wp_autowrap = !wp_autowrap;
    }

    inq_margins(-1, NULL, marginr);             /* global */
    if (wp_autowrap && marginr > 1) {
        if (echo) register_macro(REG_TYPED, "::reg_autowrap", TRUE);
        message("Auto-wrap enabled.");
    } else {
        if (echo) unregister_macro(REG_TYPED, "::reg_autowrap", TRUE);
        message("Auto-wrap disabled");
    }

    return old_autowrap;
}


int
autoindent(~string arg)
{
    int echo = TRUE, old_autoindent = wp_autoindent;

    if (strlen(arg)) {
        wp_autoindent = (lower(substr(arg, 1, 1)) == "y") ? 1 : 0;
        echo = FALSE;
    } else {
        wp_autoindent = !wp_autowrap;
    }

    if (wp_autoindent) {
        assign_to_key("<Enter>", "_indent");
        if (echo) message("Auto-indent enabled");
    } else {
        assign_to_key("<Enter>", "self_insert");
        if (echo) message("Auto-indent disabled");
    }
    return old_autoindent;
}


/*  Function:           reg_autowrap
 *      Auto-wrap key handler.
 *
 *      o If not at end of line then don't do anything special.
 *      o Look at the character just inserted,
 *          If not a space, or we havent exceeded the line length then
 *          don't do anything just yet.
 */
static void
reg_autowrap(void)
{
    int marginl, marginr, col;
    string ch;

    if (read(1) != "\n") {
        return;
    }
    prev_char();
    inq_position(NULL, col);
    inq_margins(NULL, marginl, marginr);
    if ((ch = read(1)) != " " || col < marginr) {
        next_char();
        return;
    }
    drop_anchor(MK_LINE);
    wp_quiet = TRUE;
    default_format_block();
    wp_quiet = FALSE;
    up();
    end_of_line();
    insert(" ", marginl);
}


/*
 *  margin ---
 *      Prompt for margin size and turn autowrap on.
 */
void
margin(~string)
{
    string marginnew;
    int marginr;

    inq_margins(NULL, NULL, marginr);
    if (get_parm(0, marginnew, "Right hand margin: ", NULL, marginr) >= 0) {
        set_margins(NULL, NULL, atoi(marginnew), TRUE);
        if (! wp_autowrap) {
            autowrap();
        }
    }
    message("");
}


/*
 *  format ---
 *      Format the current paragraph.
 */
void
format_paragraph(void)
{
    if (inq_marked()) {
        format_block();

    } else {
        int start, end;
        string line;

        drop_anchor(MK_LINE);
        inq_position(start);
        goto_line(start);
        end = start;
        do {
            goto_line(end++);
            line = compress(read(), TRUE);
        } while(strlen(line) > 1);
        up();

        format_region(start, end-1);
    }
}


/*
 *  format_block ---
 *      Format the current selected region.
 */
void
format_block(void)
{
    string macro_name, ext;

    inq_names(NULL, ext);
    macro_name = ext + "_format_block";         /* handler */
    if (inq_macro(macro_name) > 0) {
        execute_macro(macro_name);
    } else {
        format_list();                          /* list available */
    }
}


/*
 *  format_list ---
 *      If no format available for current file, then generate a list of all
 *      format macros and let him have a go at these.
 */
static void
format_list(void)
{
    list  macs, fmt_list;
    int   entry = 0;
    int   cnt = 0;

    macs = macro_list();
    while (1) {
        entry = re_search(NULL, "_format_block>", macs, entry);
        if (entry < 0) {
            break;
        }
        if (macs[entry] != "default_format_block") {
            fmt_list[cnt] = macs[entry];
            cnt++;
        }
        entry++;
    }

    entry = select_list("Format macros", "Select function", 1, fmt_list, SEL_CENTER, NULL);
    if (entry < 0) {
        return;
    }

    execute_macro(fmt_list[entry - 1]);
}


/*
 *  Yacc files have C style comments.
 */
void
y_format_block(void)
{
    c_format_block();
}


void
y_uncomment_block(void)
{
    c_uncomment_block();
}


void
cr_uncomment_block()
{
    c_uncomment_block();
}


void
c_uncomment_block(void)
{
    int start_line, start_col, end_line, end_col;

    save_position();
    inq_marked(start_line, start_col, end_line, end_col);
    message("Stripping existing comments.");
    re_translate(SF_GLOBAL | SF_BLOCK, "<[^A-Za-z0-9]@{[A-Za-z0-9]}", "\\0");
    re_translate(SF_GLOBAL | SF_BLOCK, "\\*{/}@[ \t]@$", "");
    restore_position();
}


void
h_format_block(void)
{
    c_format_block();
}


void
cr_format_block(void)
{
    c_format_block();
}


void
c_format_block(void)
{
//  int saved_margin = wp_marginr;
    int start_line, start_col, end_line, end_col;
    int marginr, offset, insert_vert = FALSE;
    int saved_nroff_file = nroff_file;
    string indent_string;
    string s, s1;

    if (inq_marked(start_line, start_col, end_line, end_col) == 0) {
        error("No marked block.");
        return;
    }

    save_position();
    move_abs(start_line, 1);
    s1 = read();
    re_search(NULL, "[^ \t]");
    inq_position(NULL, offset);
    inq_margins(NULL, NULL, marginr);
//  if (--offset) wp_marginr = C_IMARGIN;
    if (--offset || marginr <= 0) marginr = C_IMARGIN;
    move_abs(start_line - 1, 1);
    s = compress(read(), TRUE);
    if (start_line == 1 || s == "" || substr(s, 1, 2) != "/*") {
        insert_vert = TRUE;
    }
    restore_position();

    c_uncomment_block();

    /* Make sure right hand margin moves with the indentation level. */
    wp_cstyle = TRUE;
    nroff_file = FALSE;
    if (default_format_block() < 0) {
        nroff_file = saved_nroff_file;
//      wp_marginr = saved_margin;
        wp_cstyle = FALSE;
        return;
    }

    nroff_file = saved_nroff_file;
    wp_cstyle = FALSE;
    inq_position(end_line);

    /* Now put comment characters around the formatted text. */
    message("Adding C comments.");
    goto_line(start_line);
//  end_col = wp_marginr + C_LMARGIN + 2 + C_RMARGIN;
    end_col = marginr + C_LMARGIN + 2 + C_RMARGIN;

    /* Take the white space from the beginning of the first line
     * of the comment to indicate how much to indent by.
     */
    indent_string = substr(s1, 1, strlen(s1) - strlen(ltrim(s1)));

    if (insert_vert) {
        insert(indent_string);
        insert("/");
        insert("*", end_col - 1);
        insert("/\n");
    }

    while (start_line < end_line) {
        insert(indent_string);
        insert("/*");
        insert(" ", C_LMARGIN);
        move_abs(0, end_col + offset);
        insert("*/");
        down();
        beginning_of_line();
        ++start_line;
    }

    if (insert_vert) {
        insert(indent_string);
        insert("/");
        insert("*", end_col - 1);
        insert("/\n");
    }

//  wp_marginr = saved_margin;
    message("Formatting complete.");
}


/*
 *  default_format_block ---
 *      Macro to format a block of text (justify left and right margins).
 *      Returns -1 if no marked area; 0 if successful.
 */
int
default_format_block(void)
{
    int start_line, start_col, end_line, end_col;

    if (inq_marked(start_line, start_col, end_line, end_col) == 0) {
        error("No marked block.");
        return -1;
    }
    format_region(start_line, end_line);
}


/*
 *  format_region --
 *      Format the specified region.
 */
int
format_region(int start_line, int end_line)
{
    string prefix, trailing, remaining;
    string line, newline;
    int marginl, marginr, margins;              // margin left/right/style
    int space, comma;                           // cursors
    int paranum, linenum;                       // statistics
    int pwidth;                                 // paragraph width, minus left/right margins
    int wpad, rpad;                             // padding

    /*
     *  Strip off any leading tabs and remember how much first
     *  line was indented so we can re-indent later.
     */
    inq_margins(NULL, marginl, marginr, margins);
    save_position();
    goto_line(start_line);
    prefix = "";
    while (index(" \t", read(1))) {
        prefix += read(1);
        next_char();
    }
    pwidth = marginr;                           // paragraph width, minus left/right margins
    if (! wp_cstyle) {
        inq_position(NULL, wpad);
        pwidth -= wpad;                         // remove prefix width
    }
    restore_position();
    if (prefix != "") {
        re_translate(SF_GLOBAL | SF_BLOCK, "<[ \t]+", "");
    }
    raise_anchor();

    if (pwidth < 8) {                           // fail-safe margin
        pwidth = 8;
    }

    /*
     *  For each line seperated area
     */
    wpad = 0, rpad = 0;
    while (start_line <= end_line) {
        line = "";
        goto_line(start_line);
        linenum = start_line;

        ++paranum;
        if (! wp_quiet) {
            message("Formatting paragraph %d (%d).", paranum, end_line - start_line);
        }

        /*
         *  Read the section into a single image, removing white space etc.
         */
        space = 250;                            // stop run-away reads
        while (linenum <= end_line) {
            if (space-- <= 0) {
                if (! wp_quiet) {
                    beep();                     // warning
                }
                break;
            }
            newline = read();
            if (strlen(newline) < 2 ||         // blank or nroff marker
                    (nroff_file && substr(newline, 1, 1) == ".")) {
                break;
            }
            line += " " + newline;
            delete_line();
            --end_line;
        }
        start_line = linenum;
        line = compress(line, TRUE);

        //  While line is greater then margin,
        //      keep chopping the line and inserting the formatted remains.
        //
        while (strlen(line) > pwidth) {

            //  large (non-breakable) tokens
            //
            comma = index(line, ',');

            if ((space = index(line, ' ')) >= pwidth &&
                    (comma <= 0 || comma >= pwidth)) {
                                                // split at first comma
                if (comma > 1 && comma < space) {
                    trailing = substr(line, comma + 1);
                    line = substr(line, 1, comma);

                } else {                        // split at first space
                    trailing = substr(line, space + 1);
                    line = substr(line, 1, space - 1);
                }

                insert(prefix + line + "\n");
                line = ltrim(trailing);
                continue;

            } else if (space <= 0) {
                if (comma >= pwidth) {          // split at first comma
                    trailing = substr(line, comma + 1);
                    line = substr(line, 1, comma);
                    insert(prefix + line + "\n");
                    line = ltrim(trailing);
                    continue;

                } else if (comma <= 0) {        // no suitable break
                    insert(prefix + line + "\n");
                    line = "";
                    break;

                }
            }

            //  Seperate next line fragment
            //
            //  o truncate to the parameter width
            //  o locate last comma or space
            //  o set fragment to the greater.
            //
            remaining = substr(line, pwidth);
            line = substr(line, 1, pwidth - 1);

            space = rindex(line, ' ');          // TODO ... lastof(line, " ,");
            comma = rindex(line, ',');

            if (space > 0 &&                    // split on last space
                    (comma <= 0 || space > comma)) {
                trailing = substr(line, space) + remaining;
                line = substr(line, 1, space - 1);

            } else {                            // split on last comma
                trailing = substr(line, comma + 1) + remaining;
                line = substr(line, 1, comma);
            }

            if (JUSTIFIED == margins) {         // right justification, cal padding
                int delims, padding = pwidth - strlen(line);

                delims  = string_count(line, ' ');
                delims += string_count(line, ',');
                wpad    = padding / (delims + 1);
                rpad    = padding - (delims * wpad);
            }

            //  foreach(word)
            //
            insert(prefix);                     // leading indentation

        //  message("%d, <%s><%s>", pwidth, line, trailing);
            while (1) {

                space = index(line, ' ');       // TODO ... firstof(string, ", ");
                comma = index(line, ',');

                if (space <= 0 && comma <= 0) {
                    break;                      // complete
                }

                if (space > 0 &&                // split on first space
                        (comma <= 0 || space < comma)) {
                    remaining = substr(line, space + 1);
                    line = substr(line, 1, space);

                } else {                        // split on first comma
                    remaining = substr(line, comma + 1);
                    line = substr(line, 1, comma);
                    if (characterat(remaining, 1) != ' ') {
                        line += " ";            // push space after comma, optional???
                        --rpad;
                    }
                }

                insert(line);
                insert(" ", wpad);              // word padding
                if (rpad-- > 0) {
                    insert(" ");                // remaining
                }
                line = remaining;
            }

            insert(line + "\n");                // push remaining
            line = ltrim(trailing);
            ++end_line;
            ++start_line;
        }

        if (strlen(line)) {                     // trailing
            insert(prefix + line + "\n");
            ++end_line;
            ++start_line;
        }

        ++start_line;
        refresh();
    }

    if (! wp_quiet) {
        message("");
    }
    return 0;
}


/*
 *  center ---
 *    Function to center lines of text.
 */
void
center(~int)
{
    int num_lines, cur_line;
    int start_line, end_line;
    int marginl, marginr;
    int width, len;
    string line;

    inq_position(cur_line);
    inq_margins(NULL, marginl, marginr);        // buffer margins
    width = marginr - marginl;

    if (get_parm(0, num_lines) > 0) {           // explicit number of lines
        start_line = cur_line;
        end_line = cur_line + num_lines - 1;
                                                // marked lines
    } else if (0 == inq_marked(start_line, NULL, end_line)) {
        start_line = cur_line;                  // otherwise current only
        end_line = cur_line;
    }

    save_position();
    while (start_line <= end_line) {
        move_abs(start_line, 1);
        line = trim(read());
        delete_to_eol();
        if ((len = width - strlen(line)) > 0) {
            move_rel(NULL, marginl + (len / 2));
        }
        insert(line);
        start_line++;
    }
    restore_position();
}

/*eof*/
