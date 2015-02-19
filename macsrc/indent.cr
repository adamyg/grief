/* -*- mode: cr; indent-width: 4; -*-
 * $Id: indent.cr,v 1.14 2014/10/27 23:28:23 ayoung Exp $
 * Language indentation support macro via BPACKAGES
 *
 *
 */

#include "grief.h"

static int _r_keymap;               /* Regular indenting */


/*
 *  main ---
 *      Regular indent support.
 */
void
main(void)
{
    keyboard_push();
    assign_to_key( "<Enter>",       "_r_indent" );
    assign_to_key( "<Tab>",         "_slide_in" );
    assign_to_key( "<Shift-Tab>",   "_slide_out" );
    _r_keymap = inq_keyboard();
    keyboard_pop(1);
}


/*
 *  _regular_first ---
 *      Default regular indent mode.
 */
string
_regular_first()
{
    use_local_keyboard(_r_keymap);
    return "";
}


/*
 *  _r_indent ---
 *      This macro does "standard" style indenting, indenting new lines
 *      to the same column as the previous non-blank line.
 */
void
_r_indent()
{
    int curr_indent_col, following_position, curr_col, curr_line;
    string following_string;

    /*  First, check to see if there are any following characters
     *  on the line. If so, read them into a temporary variable and
     *  delete from the first line. Get column of last non-blank line.
     */
    if (! inq_mode()) {
        end_of_line();
    }

    inq_position(curr_line, curr_col);
    end_of_line();
    inq_position(NULL, following_position);

    if (following_position > curr_col) {
        drop_anchor(MK_NONINC);
        move_abs(0, curr_col);
        following_string = ltrim(read());
        delete_block();
    }

    if (search_back("<[ \\t\\n]+\\c[~ \\t\\n]")) {
        inq_position(NULL, curr_indent_col);
    } else {
        curr_indent_col = 1;
    }
    move_abs(curr_line, curr_col);

    /*  We've determined the last non-blank lines' indent level
     *  -- do return, indent, line split, and clean up.
     */
    if (inq_assignment(inq_command(), 1) == "<Enter>") {
        self_insert(key_to_int("<Enter>"));
    } else {
        self_insert(key_to_int("<Ctrl-M>"));
    }
    move_abs(0, curr_indent_col);

    if (following_string != "") {
        following_string = substr(following_string, 1, strlen(following_string) - 1);
        insert(following_string);
        move_abs(0, curr_indent_col);
    }
}


/*
 *  _slide_in ---
 *      This macros slides a marked block of text in one tab stop per
 *      press of the tab key (-->|).
 */
void
_slide_in()
{
    int start_line, start_col, end_line, mark_type, tab_key;

    tab_key = key_to_int("<Tab>");
    mark_type = inq_marked(start_line, start_col, end_line);

    if (mark_type == MK_NONE) {
        self_insert(tab_key);

    } else {                                    /* marked region */
        int old_mode, cur_line;

        old_mode = inq_mode();
        insert_mode(TRUE);
        save_position();
        if (mark_type != MK_COLUMN) {
            start_col = 1;
        }
        move_abs(start_line, start_col);

        while (start_line <= end_line) {
            re_search(0, "[~ \t]");             /* non-white character */
            inq_position(cur_line);

            if (cur_line != start_line) {       /* skipped a line ? */
                start_line = cur_line;
                move_abs(start_line, start_col);    /* repostion search */
            } else {
                self_insert(tab_key);
                move_abs(++start_line, start_col);  /* next line */
            }
        }

        restore_position();
        insert_mode(old_mode);
    }
}


/*
 *  _slide_out ---
 *      This macros slides a marked block of text out one tab stop per
 *      press of the back-tab key (|<--).
 */
void
_slide_out()
{
    int start_line, start_col, end_line, mark_type;

    mark_type = inq_marked(start_line, start_col, end_line);
    if (mark_type == MK_NONE) {
        _back_tab();

    } else {                                    /* marked region */
        save_position();
        if (mark_type != MK_COLUMN) {
            start_col = 1;
        }
        move_abs(start_line, start_col);

        while (start_line <= end_line) {
            int curr_line, curr_col;

            search_fwd("[~ \t]");
            inq_position(curr_line, curr_col);

            if (curr_line == start_line && curr_col > start_col) {
                drop_anchor(MK_NONINC);
                _back_tab();
                inq_position(curr_line, curr_col);
                if (curr_col < start_col)
                    move_abs(0, start_col);
                delete_block();
            }
            move_abs(++start_line, start_col);
        }

        restore_position();
    }
}


/*
 *  _just_obrace, _just_cbrace, _just_space ---
 *      These functions insert unexpanded and expanded braces and spaces.
 */
void
_just_obrace()
{
    insert("{");
}


void
_just_cbrace()
{
    insert("}");
}


void
_just_space()
{
    insert(" ");
}

/*end*/
