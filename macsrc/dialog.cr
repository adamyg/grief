/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dialog.cr,v 1.13 2014/10/27 23:28:20 ayoung Exp $
 * Macros emulating BRIEF dialog primitives. This file contains
 * macros which emulate BRIEF functions based on the functions
 * available under GRIEF.
 *
 *
 */

#include "grief.h"
#include "dialog.h"

/* Shared dialog globals */
int             _dialog_level,
                _dialog_mode,
                _dialog_picked,
                _dialog_disp_buf,
                _dialog_data_buf,
                _dialog_size,
                _dialog_row;

string          _dialog_action_func;            /* User callback (current level) */

/* Menu specify globals */
int             _menu_old_buf_id,
                _dialog_menu_keymap,
                _dialog_menu_time;

string          _dialog_menu_prefix,
                _dialog_menu_pat;               /* Search pattern (global) */

int             _dialog_menu(int lx, int by, int rx, int ty,
                        ~string, ~string, ~string, ~int, ~string notused, ~int, ~int, ~string);
void            _dialog_menu_func(int funckey);
void            _dialog_menu_insert();
void            _dialog_menu_search( int nextflag, int sizeflag );
int             _menu_highlight();
void            _dialog_menu_home();
void            _dialog_menu_end();
void            _dialog_menu_pgup();
void            _dialog_menu_pgdn();
int             _dialog_menu_down();
int             _dialog_menu_up();
void            _dialog_typeables();
void            _dialog_menu_pick();
string          _dialog_menutext();


void
main(void)
{
    keyboard_push();
    keyboard_typeables();
    assign_to_key("<Down>",             "_dialog_menu_down");
    assign_to_key("<Wheel-Down>",       "_dialog_menu_down");
    assign_to_key("<Space>",            "_dialog_menu_down");
    assign_to_key("<Up>",               "_dialog_menu_up");
    assign_to_key("<Wheel-Up>",         "_dialog_menu_up");
    assign_to_key("<Backspace>",        "_dialog_menu_up");
    assign_to_key("<Enter>",            "_dialog_menu_pick");
    assign_to_key("<Keypad-Enter>",     "_dialog_menu_pick");
    assign_to_key("<Esc>",              "_dialog_esc");
    assign_to_key("<Keypad-Minus>",     "_dialog_back");
    assign_to_key("<Home>",             "_dialog_menu_home");
    assign_to_key("<Ctrl-Home>",        "_dialog_menu_home");
    assign_to_key("<End>",              "_dialog_menu_end");
    assign_to_key("<Ctrl-End>",         "_dialog_menu_end");
    assign_to_key("<PgUp>",             "_dialog_menu_pgup");
    assign_to_key("<PgDn>",             "_dialog_menu_pgdn");
    assign_to_key("<Ins>",              "_dialog_menu_insert");
    assign_to_key("<Keypad-Ins>",       "_dialog_menu_insert");
    assign_to_key("<F1>",               "_dialog_menu_func 1");
    assign_to_key("<F2>",               "_dialog_menu_func 2");
    assign_to_key("<F3>",               "_dialog_menu_func 3");
    assign_to_key("<F4>",               "_dialog_menu_func 4");
    assign_to_key("<F5>",               "_dialog_menu_func 5");
    assign_to_key("<F6>",               "_dialog_menu_func 6");
    assign_to_key("<F7>",               "_dialog_menu_func 7");
    assign_to_key("<F8>",               "_dialog_menu_func 8");
    assign_to_key("<F9>",               "_dialog_menu_func 9");
    assign_to_key("<F10>",              "_dialog_menu_func 10");
    assign_to_key("<Shift-F1>",         "_dialog_menu_func 101");
    assign_to_key("<Shift-F2>",         "_dialog_menu_func 102");
    assign_to_key("<Shift-F3>",         "_dialog_menu_func 103");
    assign_to_key("<Shift-F4>",         "_dialog_menu_func 104");
    assign_to_key("<Shift-F5>",         "_dialog_menu_func 105");
    assign_to_key("<Shift-F6>",         "_dialog_menu_func 106");
    assign_to_key("<Shift-F7>",         "_dialog_menu_func 107");
    assign_to_key("<Shift-F8>",         "_dialog_menu_func 108");
    assign_to_key("<Shift-F9>",         "_dialog_menu_func 109");
    assign_to_key("<Shift-F10>",        "_dialog_menu_func 110");
    _dialog_menu_keymap = inq_keyboard();
    keyboard_pop(TRUE);
}


/*  _dialog_esc, _dialog_back ---
 *      Call the action function with an appropriate code and
 *      exit from the dialog box, menu, or entire dialog manager.
 */
void
_dialog_esc(void)
{
    int curr_level;

    execute_macro(_dialog_action_func, DIALOG_ESCAPE);
    curr_level = _dialog_level;
    while (curr_level--) {
        exit();
    }
}


void
_dialog_back(void)
{
    execute_macro(_dialog_action_func, DIALOG_BACK);
    exit();
}


/*
 *  _dialog_menu ---
 *      Creates and processes a menu, calling the user when
 *      interesting events occur.
 */
int
_dialog_menu(int lx, int by, int rx, int ty,
        ~string, ~string, ~string, ~int, ~string notused, ~int, ~int, ~string)
{
    extern int popup_level;
    string  pathname, old_action_func;
    int     old_data_buf_id, old_win_id, old_buf_id,
            old_size, old_mode,
            menu_buf_id, created_buffer,
            screen_lines, screen_cols;

    UNUSED(notused);

    /*  If we are passed an existing buffer id,  we use that
     *  buffer; if not, but we are passed the name of an existing
     *  file, we create the buffer. Otherwise if we get neither
     *  parameter, we fail.
     */
    if (! get_parm(7, menu_buf_id) && get_parm(6, pathname))
        if (exist(pathname)) {
            get_parm(4, old_action_func);
            menu_buf_id = create_buffer(old_action_func, pathname, TRUE);
            created_buffer = TRUE;
        }

    /* Valid buffer */
    if (!menu_buf_id) {
        error("Can't create menu.");
        return (FALSE);
    }
    old_buf_id = inq_buffer();
    old_win_id = inq_window();
    if ( !_dialog_level )
        _menu_old_buf_id = old_buf_id;
    message("Creating menu...");

    /* Increment next levels */
    ++_dialog_level;
    _dialog_menu_prefix = "";
    old_action_func = _dialog_action_func;
    old_mode = _dialog_mode;
    old_data_buf_id = _dialog_data_buf;
    _dialog_mode = DIALOG_MENU_MODE;

    /*  Call the menu creation event in case we want to do any raw
     *  processing on the menu (like adding our own buttons).
     */
    get_parm(8, _dialog_action_func);
    set_buffer(_dialog_data_buf = menu_buf_id);
    inq_names(NULL, NULL, pathname);
    execute_macro(_dialog_action_func, DIALOG_CREATE_MENU, NULL, pathname);

    /* Retrieve the bottom line message. */
    get_parm(5, pathname);

    /*  If rx and ty are both zero, treat lx and by as requested
     *  lines and widt. The forces sizing simular to sized_window(),
     *  that builds a window buffer of a reasonable size taking into
     *  account that we may not be running on a plain 80x25, and gives
     *  as many lines as will fit on the screen whilst taking the
     *  callers request into account.
     *
     *  If either window dimension is zero, size the window automatically.
     *  It has to be at least 1 line x 14 cols  (larger if a long bottom
     *  line message is to be displayed).
     */
    if (rx == 0 && ty == 0) {
        int lines = by;
        int width = lx;

        inq_screen_size(screen_lines, screen_cols);
        lx = (screen_cols - width)/2;           /* center window */
        if (lx < 1)
            lx = 1;
        rx = lx + width;
        --screen_cols;
        if (rx > screen_cols)
            rx = screen_cols;
        ty = 3;                                 /* top line */
        by = ty + lines;
        if (by >= screen_lines - 4)
            by = screen_lines - 5;

    } else if (lx == rx || ty == by) {
        /*
         *  Use the first semicolon, if any, and the number of lines
         *  to get the optimum size.  (No semicolon: use a 14-column window.)
         */
        top_of_buffer();
        if (search_fwd(";", FALSE))
            inq_position(NULL, rx);
        else rx = 15;
        rx += lx;
        end_of_buffer();
        inq_position(by);
        by += ty + 1;
        top_of_buffer();

        /*  Make sure the window will fit on the screen; if it
         *  won't, shrink itm and leave room for the "shadows".
         */
        inq_screen_size(screen_lines, screen_cols);
        screen_lines -= 4;
        screen_cols -= 3;
        if (screen_lines < by)
            by = screen_lines;
        if (screen_cols < rx)
            rx = screen_cols;
    }

    /*  If either dimension is too small, display an error message and
     *  not the window.  Note that we duplicate all the "cleanup" code
     *  here so we can return immediately.  That's because the code size
     *  is less crucial than the maximum nesting depth here, since the
     *  macro is intended to be called recursively and nesting in macros
     *  consumes a ton of stack space.
     */
    screen_cols = strlen(pathname) + 2;
    screen_lines = rx - lx;

    if ((by - ty <= 1 || screen_lines <= 14) || screen_lines <= screen_cols) {
        error("Window would be too small.");
        if (created_buffer && !inq_views())
            delete_buffer(menu_buf_id);
        set_buffer(old_buf_id);
        if (!--_dialog_level) {
            call_registered_macro(1);
            message("");
        }
        _dialog_action_func = old_action_func;
        _dialog_mode = old_mode;
        _dialog_data_buf = old_data_buf_id;
        return (FALSE);
    }
    _dialog_picked = 0;
    old_size = _dialog_size;
    _dialog_size = (rx - lx) - 1;
    keyboard_push(_dialog_menu_keymap);

    /* Now we display the menu window, but we postpone refreshing the
     * display until we highlight the first button.
     */
    create_window(lx, by, rx, ty, pathname);
    attach_buffer(menu_buf_id);
    message("Menu created.");

    /* Go to the top of the buffer, call the init event,
     * highlight the first selectable line, redraw the screen,
     * and go into interactive mode.
     */
    top_of_buffer();
    execute_macro(_dialog_action_func, DIALOG_INIT);
    _dialog_menu_home();

    /* Intercept all normal keys typed so we can do the selection of
     * an item which starts with a letter.
     */
    register_macro(REG_TYPED, "_dialog_typeables", TRUE);

    /* Keep track of the level of nesting. This allows the
     * mousehandler to decide what to do inside popups.
     */
    refresh();
    ++popup_level;
    process();
    --popup_level;

    /* Remove registered macro. */
    unregister_macro(REG_TYPED, "_dialog_typeables", TRUE);

    /* Remove the highlight, then the keyboard, then the window.  */
    raise_anchor();
    execute_macro(_dialog_action_func, DIALOG_TERM);
    keyboard_pop(TRUE);

    /*  Here we set parameter 10 to indicate the button number picked,
     *  set parameter 11 to the text of the picked button, and clear
     *  clear _dialog_picked so that nested menus work properly.
     *
     *  A zero value for parameter 10 indicates the user pressed <Esc>
     *  or <Keypad minus> to abort the menu.
     *
     *  A nonzero value indicates the user pressed <Enter> to select
     *  a button, and the button number becomes the parameter value.
     *  The button text is in parameter 11.
     */
    put_parm(10, _dialog_picked);
    put_parm(11, _dialog_picked ? _dialog_menutext() : "");
    _dialog_picked = 0;

    /* Restore the old current buffer and delete the menu buffer */
    if (created_buffer && !inq_views())
        delete_buffer(menu_buf_id);
    delete_window();
    set_window(old_win_id);
    set_buffer(old_buf_id);
    attach_buffer(old_buf_id);

    /* Decrement the menu level.
     * If it is zero, we're leaving the menu package for good--so
     * re-run the new file macros, and clear the message line.
     */
    if (!--_dialog_level) {
        call_registered_macro(1);
        message("");
    }
    _dialog_action_func = old_action_func;
    _dialog_mode = old_mode;
    _dialog_size = old_size;
    _dialog_data_buf = old_data_buf_id;
    return (TRUE);
}


void
_dialog_menu_func(int funckey)
{
    if (execute_macro(_dialog_action_func, DIALOG_FUNC_KEY, funckey, _dialog_row)) {
        switch(funckey) {
        case DIALOG_KEY_F5:                     /* <F5> */
            _dialog_menu_search(0, FALSE);
            break;
        case DIALOG_KEY_SF5:                    /* <Shift-F5> */
            _dialog_menu_search(1, FALSE);
            break;
        }
    }
}


/*  _dialog_menu_insert() ---
 *      Inserts the contents of the menu into the buffer of
 *      the file specified by the user on the command line
 */
void
_dialog_menu_insert()
{
    int     menu_buf, menu_row, menu_col;
    string  file_name;

    if (!get_parm(0, file_name, "output file: "))
        return;
    menu_buf = inq_buffer();
    end_of_buffer();
    inq_position(menu_row, menu_col);
    edit_file(file_name);
    transfer(menu_buf, 1, 1, menu_row, menu_col);
    if (read() == "\n")
        delete_line();
    set_buffer(menu_buf);
    _dialog_esc();
}


/*  _dialog_menu_search ---
 *      Search a menu window
 */
void
_dialog_menu_search(int nextflag, int sizeflag)
{
    int cols, col, top;

    /* Use the previous search string, if next... */
    if (!nextflag && get_parm(NULL, _dialog_menu_pat, "Search for: ") == 0)
        return;

    save_position();
    if (nextflag)
        down();
    top = 0;
    inq_window_size(NULL, cols, NULL);
    while (TRUE) {
        /*  Search from current position, but only except matchs if
         *  being displayed within the dialog window  (Searchs use
         *  regular expressions, but are case insensitive)
         */
        while (TRUE) {
            if (search_fwd(_dialog_menu_pat, 1, 0) <= 0)
                break;

            inq_position(NULL, col);
            if ( !sizeflag || col < cols ) {
                /*  Within the window */
                restore_position(0);
                beginning_of_line();
                raise_anchor();
                drop_anchor(3);
                return;
            }
            down();                             /* Skip the last match */
        }

        /* Start search again from the top (if not already tried) */
        if ( !top ) {
            top_of_buffer();
            top = 1;
        } else {
            break;                              /* Top already done */
        }
    }
    restore_position();
    beep();
}


/*  _menu_highlight ---
 *      Calls the menu alter event.  If it succeeds, highlights
 *      the current menu line using a line mark.
 */
int
_menu_highlight()
{
    int line;

    inq_position(line);
    if (execute_macro(_dialog_action_func,
            DIALOG_MOVE_MENU, line, _dialog_menutext())) {
        raise_anchor();
        drop_anchor(3);
        return (TRUE);
    }
    return (FALSE);
}


/*  _dialog_menu_home ---
 *      Moves the selection to the first selectable item on the menu.
 *      Calls the action function with the new line number and menu
 *      button text.
 */
void
_dialog_menu_home()
{
    top_of_buffer();
    while (!_menu_highlight())
        down();
}


/*  _dialog_menu_end ---
 *      Moves to the last selectable item on the menu.   Calls the
 *      action function with the new line number and menu button text.
 */
void
_dialog_menu_end()
{
    end_of_buffer();
    beginning_of_line();
    while (!_menu_highlight())
        up();
}


/*  _dialog_menu_pgup, _dialog_menu_pgdn ---
 *      Pages up or down in a menu.
 */
void
_dialog_menu_pgup()
{
    page_up();
    if (!(_menu_highlight() || _dialog_menu_up()))
        _dialog_menu_down();
}


void
_dialog_menu_pgdn()
{
    int line;

    page_down();
    inq_position(line);
    if (line > inq_lines())
         goto_line(inq_lines());
    if (!(_menu_highlight() || _dialog_menu_down()))
        _dialog_menu_up();
}


/*  _dialog_menu_down ---
 *      Moves the current selection down, stopping at next selectable item; if the
 *      bottom of the menu is encountered, returns to the previous position.
 *
 *      Returns whether or not it succeeded in moving.  Calls the action function
 *      with the new line number and menu text as parameters.
 */
int
_dialog_menu_down()
{
    int failed, line;

    save_position();
    while (TRUE) {
        down();
        inq_position(line);
        if (line > inq_lines()) {
            failed++;
            break;
        } else if (_menu_highlight())
            break;
    }
    restore_position (failed);
    return (!failed);
}


/*  _dialog_menu_up ---
 *      Moves the current selection up to the previous selectable
 *      item, if there are any.  Returns TRUE if it succeeded.
 */
int
_dialog_menu_up()
{
    int moved;

    save_position();
    while (up())
        if (_menu_highlight()) {
            moved++;
            break;
        }
    if (!moved)
        set_top_left(1, 1);
    restore_position(!moved);
    return (moved);
}


/*  _dialog_typeables ---
 *      Reads a character (that was just inserted; _dialog_typeables is
 *      called as registered macro) from the buffer and deletes it, then
 *      looks for a matching button before the end of the menu. If there
 *      is none, looks for one starting with the first item.  If there
 *      is still none, stays put.  (If a selectable line is found,
 *      move to it and highlights it).
 */
void
_dialog_typeables()
{
    string  pattern;
    int     end_line, after_line,
            start_line, start_col,
            failed;

    /*
     *  Figure out what key was pressed.
     */
    prev_char();
    pattern = read(1);
    delete_char();
    message("alpha(%s)", pattern);
    pattern = quote_regexp(pattern);
    pattern = "<[ \\t]@" + (_dialog_menu_prefix += pattern);

    /*
     *  Find the last line of the window; if the search takes us
     *  outside it, we want to center the line in the window.
     */
    inq_position(start_line, start_col);
    end_of_window();
    inq_position(end_line);
    move_abs(start_line, start_col);

    /*
     *  If we are searching for a one-character button prefix
     *  we search the current line last, not first.
     */
    if (strlen(_dialog_menu_prefix) == 1)
        down();

    /*
     *  Search from the current position to EOF, breaking if a
     *  selectable line is found.  If the pattern is not found,
     *  start searching from the top of the file down.
     */
    while (!failed) {
        if (search_fwd(pattern, TRUE, FALSE) <= 0) {
            move_abs(start_line, start_col);
            drop_anchor(3);
            top_of_buffer();
            while (! failed) {
                if (search_fwd(pattern, TRUE, FALSE, TRUE) <= 0)
                    ++failed;

                else if (_menu_highlight()) {
                    inq_position (after_line);
                    return;
                } else {
                    down();
                }
            }

            /*
             *  If we can't find the current multi-character prefix in the
             *  menu, but the last two characters were the same, we throw
             *  away the second of those and look for the next prefix that
             *  matches.
             */
            if ((failed = strlen(_dialog_menu_prefix)) > 1 &&
                    substr(_dialog_menu_prefix, failed, 1) == substr(_dialog_menu_prefix, failed - 1, 1)) {
                _dialog_menu_prefix = substr(_dialog_menu_prefix, 1, failed - 1);
                pattern = "<[ \\t]@" + _dialog_menu_prefix;
                move_abs(start_line, start_col);
                down();
                failed = FALSE;
            } else {
                failed = TRUE;
            }
            raise_anchor();

        } else if (_menu_highlight()) {
            inq_position(after_line);
            return;

        } else {
            down();
        }
    }
    move_abs(start_line, start_col);
}


/*  _dialog_menu_pick ----
 *          Processes the menu line that is selected when the user
 *          presses Enter.
 */
void
_dialog_menu_pick()
{
    int line;

    inq_position(line);
    if (execute_macro(_dialog_action_func,
               DIALOG_PICK_MENU, line, _dialog_menutext())) {
        _dialog_picked = line;
    }
}


/*  _dialog_menutext ----
 *      Returns the text of the current menu button, with leading
 *      and trailing white space removed.
 */
string
_dialog_menutext()
{
    return (trim(read()));
}

/*eof*/
