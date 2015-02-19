/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: misc.cr,v 1.19 2014/10/27 23:28:24 ayoung Exp $
 * Misc macros.
 *
 *
 */

#include "grief.h"

extern string           last_file_edited;

static string FS        = "\t";                 // goto_line() field seperator


/* search_path:
 *
 *    This routine searches a path, such as the PATH or GRPATH
 *    environment variable, for the specified file.  It returns
 *    the null string if the file wasn't found, or the full
 *    path name if it was.
 */
string
search_path(string path, string filename)
{
    string curr_name;
    int semi_loc;

    /*
     *  Get the directory list to check and append a semicolon to it. This means we will
     *  always check the current directory last unless we are told to check it sooner.
     */
    path += CRISP_DIRSEP;

    /*
     *  Search all the directories for the given file name.  As soon as we find the file,
     *  append the name of the file to the given directory and return the full name.
     */
    while (semi_loc = index(path, CRISP_DIRSEP)) {
        curr_name = add_to_path(substr (path, 1, semi_loc - 1), filename);

        /*
         *  If the file exists in the given directory, we return
         *  the full name without bothering to parse anything else.
         */
        if (exist (curr_name)) {
            return (curr_name);
        }
        path = substr (path, semi_loc + 1);
    }
    return "";
}


/*
 *  add_to_path:
 *
 *      This simple routine adds a filename to a path and returns the result.
 */
string
add_to_path(string path, string name)
{
#if defined(MSDOS)                              // TODO - system independent
    if (strlen(path) && !index ("/\\:", substr (path, strlen (path))))
#else
    if (strlen(path) && !index ("/\\", substr (path, strlen (path))))
#endif
        path += "/";
    path += name;
    return path;
}


/*
 *  Macro to delete the current buffer. We can only do this if its not displayed in
 *  another window, and we have another buffer to replace this one with. Prompt user
 *  if its been modified.
 */
void
delete_curr_buffer(void)
{
    string prompt, filename;
    int curbuf;
    int newbuf;

    if (inq_views() != 1) {
        error("Buffer being displayed in another window.");
        return;
    }
    curbuf = inq_buffer();
    newbuf = next_buffer();
    if (newbuf == curbuf) {
        error("No more buffers.");
        return;
    }

    if (inq_modified()) {
        while (1) {
            if (get_parm(NULL, prompt, "Buffer has not been saved.  Delete [ynw]? ", 1) <= 0) {
                message("");
                return;
            }

            if (index("nN", prompt)) {
                message("Buffer not deleted.");
                return;
            }

            if (index("wW", prompt)) {
                int old_msg_level = set_msg_level(0);
                if (write_buffer() <= 0) {
                    set_msg_level(old_msg_level);
                    return;
                }
                set_msg_level(old_msg_level);
                break;
            }

            if (index("yY", prompt)) {
                break;
            }
        }
    }

    set_buffer(newbuf);
    inq_names(filename);
    if (edit_file(filename) != -1) {
        display_file_name();
        delete_buffer(curbuf);
    }
}


/*
 *  Macro executed when user hits <Alt-N>.
 */
void
edit_next_buffer(void)
{
    int curbuf = inq_buffer(), nextbuf = next_buffer();
    string filename;

    inq_names(last_file_edited);

    if (curbuf == nextbuf) {
        error("No more buffers.");
        return;
    }

    set_buffer(nextbuf);
    inq_names(filename);
    if (edit_file(filename) != -1) {
        display_file_name();
    }
}


/*
 *  Macro executed when user hits <^_>.
 */
void
edit_previous_buffer(void)
{
    int curbuf = inq_buffer(), nextbuf = next_buffer();
    string filename;

    inq_names(last_file_edited);

    if (curbuf == nextbuf) {
        error("No more buffers.");
        return;
    }

    while (curbuf != nextbuf) {
        set_buffer(nextbuf);
        nextbuf = next_buffer();
    }

    inq_names(filename);
    if (edit_file(filename) != -1) {
        display_file_name();
    }
}


/*
 *  Macro executed when user hits <Alt-E>
 */
int
edit__file(~string)
{
    string s;
    int old_msg_level;
    string filename;
    int ret;

    get_parm(0, filename);
    inq_names(s);
    old_msg_level = set_msg_level(0);
    if (filename != "") {
        if ((ret = edit_file(filename))) {
            last_file_edited = s;
        }
    } else {
        if ((ret = edit_file())) {
            last_file_edited = s;
        }
    }
    set_msg_level(old_msg_level);
    if (substr(inq_message(), 1, 4) == "Edit") {
        display_file_name();
    }
    return ret;
}


/*
 *  previous_edited_buffer ---
 *      This macro selects the previous edited buffer. This is the last buffer that was
 *      selected, as distinct from the macro previous_alpha_buffer() which selects the
 *      previous buffer (alphabetically).
 */
void
previous_edited_buffer(void)
{
    string s;

    if (last_file_edited == "") {
        inq_names(last_file_edited);
        edit_next_buffer();
    } else {
        s = last_file_edited;
        inq_names(last_file_edited);
        edit_file(s);
        display_file_name();
    }
}


/*
 *  previous_alpha_buffer ---
 *      This macro selects the previous buffer which is alphabetically before the
 *      current buffer.
 */
void
previous_alpha_buffer(void)
{
    int curbuf = inq_buffer(), nextbuf = next_buffer(NULL, 1);
    string filename;

    inq_names(last_file_edited);
    if (curbuf == nextbuf) {
        error("No more buffers.");
        return;
    }
    set_buffer(nextbuf);
    inq_names(filename);
    if (edit_file(filename) != -1) {
        display_file_name();
    }
}


/*
 *  Macro to perform a redo after an undo.
 */
/*replacement*/
void
redo(void)
{
    undo(0, 0, TRUE);
}


/*
 *  Macro executed when <Alt-G> selected. Allows user to go to a line in the current
 *  buffer. If line is negative, we go to the old line. If line ends with a full-stop,
 *  treat it as a character position.
 */
void
goto__line(void)
{
    string line;
    int colpos = FALSE;
    int l, ln, ln1;

    /*
     *  opens with a '#' go to a particular field in the current line.
     */
    get_parm(NULL, line, "Goto (old) line: ");
    if (substr(line, 1, 1) == "#") {            // TODO - document
        l = cvt_to_object(substr(line, 2));
        beginning_of_line();
        inq_position(ln);
        while (l-- > 0) {
            if (re_search(0, FS + "+\\c") <= 0) {
                break;
            }
        }
        inq_position(ln1);
        if (ln != ln1) {
            goto_line(ln);
            error("Not enough fields on line.");
            return;
        }
        return;
    }

    /*
     *  opens with a '|' make it a column position.
     */
    if (substr(line, 1, 1) == "|") {            // TODO - document
        colpos = TRUE;
        line = substr(line, 2);
    }

    /*
     *  decode and action trailing value
     */
    l = strtol(line);                           // 0xxxx, 0oooo and ddd
    if (colpos) {
        move_abs(NULL, l);
    } else if (substr(line, strlen(line)) == ".") {
        top_of_buffer();
        next_char(l);
    } else if (l < 0) {
        goto_old_line(-l);
    } else if (l) {
        goto_line(l);
    }
}


/*
 *  set_fs ---
 *      Set the field separator.
 */
void
set_fs(void)                                    // TODO - document
{
    if (get_parm(NULL, FS, "Field separator: ", NULL, FS) > 0) {
        if (strlen(FS) > 1) {
            FS = "[" + quote_regexp(FS) + "]";
        }
    }
}


/*
 *  Macro called when the tab key is hit. If a region is
 *  hilighted then shift it otherwise just insert a tab.
 */
void
insert_tab(void)
{
    if (inq_marked()) {
        shift_right();
    } else {
        self_insert();
    }
}


/*
 *  Macro called when <Shift-Tab> key hit. Unindent currently hilighted block
 *  if necessary otherwise go back a tab stop.
 */
void
insert_backtab(void)
{
    if (inq_marked()) {
        shift_left();
    } else {
        previous_tab();
    }
}


/*
 *  Macro to force input into the keyboard input buffer. Argument is a string
 *  in key_to_int() format.
 */
void
force_input(string str)
{
    int i;
    string fn;

    while (str != "") {
        if (substr(str, 1, 1) == "<") {
            i = index(str, ">");
            fn = substr(str, 1, i);
            str = substr(str, i + 1);
            push_back(key_to_int(fn));
        } else {
            push_back(key_to_int(substr(str, 1, 1)));
            str = substr(str, 2);
        }
    }
}


/*
 *  Macro to delete all blank lines from cursor position to end of buffer.
 */
void
delete_blank_lines(void)
{
    int num = 0;

    save_position();
    while (re_search(NULL, "^$") > 0) {
        delete_line();
        num++;
    }
    restore_position();
    message("%d blank line%s deleted.", num, num == 1 ? "" : "s");
}


/*
 *  Function to delete trailing white space to all lines in buffer.
 */
void
delete_trailing_spaces(void)
{
    save_position();
    top_of_buffer();
    re_translate(SF_GLOBAL, "[ \t]+$", "");
    restore_position();
}


/*
 *  Following macro turns off the undo flag for the current buffer. When performing
 *  big editing jobs stops us running out of disk space and speeds things up a bit.
 */
void
noundo(void)
{
    int flags = inq_buffer_flags();

    if (flags & BF_NO_UNDO) {
        set_buffer_flags(NULL, NULL, ~BF_NO_UNDO);
        message("Undo enabled.");
    } else {
        set_buffer_flags(NULL, BF_NO_UNDO);
        message("Undo disabled.");
    }
}


/*
 *  Function to turn on/off ANSI mode setting for current buffer.
 */
void
ansi(void)
{
    int flags = inq_buffer_flags();

    if (flags & BF_ANSI) {
        set_buffer_flags(NULL, NULL, ~BF_ANSI);
        message("ANSI disabled.");
    } else {
        set_buffer_flags(NULL, BF_ANSI);
        message("ANSI enabled.");
    }
    refresh();
}


void
_indent(void)
{
    int col;

    if (inq_buffer_flags() & BF_READONLY) {
        down();
        beginning_of_line();
        return;
    }

    inq_position(NULL, col);
    insert("\n");

    if (col <= 1 || trim(read()) != "") {
        /*
         *  If user hits <Enter> at beginning of line or theres stuff to
         *  the right of the cursor then don't autoindent.
         */
        return;
    }
    save_position();

    if (search_back("[~ \t]") <= 0) {
        restore_position();
        return;
    }

    beginning_of_line();
    search_fwd("[~ \t]");
    inq_position(NULL, col);
    restore_position();
    move_abs(NULL, col);
    tab_to_col(col);
}


/*
 *  Macro to move the cursor back to the previous tab stop. This macro will not move
 *  the cursor beyond the beginning of the current line.
 */
void
previous_tab(void)
{
    int col, num, prev_num;

    inq_position(NULL, col);
    if (col <= 1) {
        return;                                 /* If we are already in column 1, dont go back any further. */
    }
    left();
    prev_num = distance_to_indent();
    while (1) {
        num = distance_to_indent();
        inq_position(NULL, col);
        if (num < prev_num) {
            right();
            break;
        }
        if (col <= 1) {
            break;                              /* start of line */
        }
        prev_num = num;
        left();
    }
}


void
tab_to_col(int col)
{
    int curcol, hard_tabs;

    beginning_of_line();
    hard_tabs = use_tab_char("y");
    use_tab_char(hard_tabs ? "y" : "n");

    if (! hard_tabs) {
        insert(" ", col - 1);
        return;
    }

    while (1) {
        inq_position(NULL, curcol);
        if (curcol >= col) {
            break;
        }
        insert("\t");
    }

    if (curcol > col) {
        backspace();
        inq_position(NULL, curcol);
        insert(" ", col - curcol);
    }
}


void
display_file_name(void)
{
    string filename;
    int cols, len;

    inq_names(filename);
    inq_screen_size(NULL, cols);
    cols -= 43;
    len = strlen(filename);
    if (len > cols) {
        filename = substr(filename, len - cols);
        filename = "..." + filename;
    }
    message("File: %s%s", filename, inq_modified() ? "*" : "");
}


void
repeat(void)
{
    int count, ch;
    string macro_name;

    count = 0;
    while (1) {
        message("Repeat count = %d", count);
        while ((ch = read_char()) == -1) {
            nothing();
        }

        if (ch >= '0' && ch <= '9') {
            count = count * 10 + ch - '0';
            continue;
        }

        if (int_to_key(ch) == "<Esc>") {
            message("Repeat aborted.");
            return;
        }

        if (int_to_key(ch) == "<Ctrl-r>") {
            if (count == 0) {
                count = 1;
            }
            count *= 4;
            continue;
        }
        break;
    }

    macro_name = inq_assignment(int_to_key(ch));
    while (count > 0) {
        execute_macro(macro_name);
        --count;
    }
}


/*
 *  home ---
 *      context specific <home> key.
 */
void
home(void)
{
    int state = 0;

    if (inq_macro_history(1) == "home") {       /* home, home */
        ++state;
        if (inq_macro_history(2) == "home") {   /* home, home, home */
            ++state;
            if (inq_macro_history(3) == "home") {
                return;
            }
        }
    }

    switch (state) {
    case 2:
        top_of_buffer();
        break;
    case 1:
        top_of_window();
        break;
    default:
        beginning_of_line();
        break;
    }
}


/*
 *  end ---
 *      context specific <end> key.
 */
void
end(void)
{
    int state = 0;

    if (inq_macro_history(1) == "end") {
        ++state;
        if (inq_macro_history(2) == "end") {
            ++state;
            if (inq_macro_history(3) == "end") {
                return;
            }
        }
    }

    switch (state) {
    case 2:
        end_of_buffer();
        break;
    case 1:
        end_of_window();
        end_of_line();
        break;
    default:
        end_of_line();
        break;
    }
}


/*
 *  sub ---
 *      awk-like sub macro.
 */
string
sub(string r, string s, string t)
{
    return re_translate(SF_AWK, r, s, t);
}


/*
 *  qsub ---
 *      awk-like gsub macro.
 */
string
gsub(string r, string s, string t)
{
    return re_translate(SF_GLOBAL | SF_AWK, r, s, t);
}


/*
 *  cd ---
 *      Overloaded 'cd', when executed, sets window title to the current directory.
 */
replacement string
cd()
{
    string __dir;

    if (inq_called() != "")
        return cd();
    if (get_parm(0, __dir) > 0) {
        cd(__dir);
        getwd(NULL, __dir);
        set_wm_name(__dir);
    } else {
        cd();
    }
}


/*
 *  quote ---
 *      quote character.
 */
void
quote(void)
{
    int key = -1;

    while (key < 0) {
        key = read_char(NULL, TRUE);
    }
    insert(key);
}


/*
 *  join_line ---
 *      Macro to append the next line to the end of the current line. This is similar
 *      in operation to the 'vi' 'J' command. Leading spaces at the beginning of the
 *      next line are removed.
 */
void
join_line(void)
{
    down();
    beginning_of_line();

    /* Delete white space at beginning of next line. */
    while (index(" \t", read(1))) {
        delete_char();
    }

    /* Delete white space at end of current line. */
    up();
    drop_anchor(MK_LINE);
    re_translate(SF_GLOBAL | SF_BLOCK, "[ \t]+$", "");
    raise_anchor();
    end_of_line();
    insert(" ");
    delete_char();
}


void
delete_character(void)
{
    if (inq_called() != "" || inq_marked() == 0) {
        delete_char();
        return;
    }

    if (inq_marked() == MK_COLUMN) {
        block_delete();
        message("Block deleted.");
    } else {
        delete_block();
    }
}


/*
 *  tiny ---
 *      Create a tiny movable popup window. This is an experimental macro. It is not
 *      designed to be usable until some real problems are resolved concerning the
 *      display manager.
 */
void
tiny(void)
{
    int win;

    win = sized_window(10, 20, NULL);
    set_window(win);
    attach_buffer(inq_buffer());
}


/*
 *  hex ---
 *      Print decimal number as HEX.
 */
void
hex(~int)
{
    int n;

    get_parm(0, n, "decimal number: ");
    message("hex: %x", n);
}


/*
 *  dec ---
 *      Print decimal number as HEX.
 */
void
dec(~string)
{
    string s;
    int v;

    get_parm(0, s, "hexadecimal number: ");
    sscanf(s, "%x", v);
    message("dec: %d", v);
}

/*end*/
