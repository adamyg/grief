/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: extra.cr,v 1.21 2014/10/27 23:28:21 ayoung Exp $
 * Extra editing functions.
 *
 *
 */

#include "grief.h"

#define HALF_SEC        500

extern void             make_writeable(void);

extern void             edit_again(void);

extern void             italicise_word(void);
extern void             pipe_region(~string);
extern void             grep_function(void);

extern void             edit_dos_file(void);
extern void             edit_text_file(void);
extern void             edit_binary_file(void);
static void             edit_typed_file(string msg, int flags);

extern void             toggle_buffer_flags(void);

static list             extra_list = {
                "Extras Menu", "",              /* title, help topic */
    "Edit DOS file",            "edit_dos_file",
    "Edit file again",          "edit_again",
    "Edit text file",           "edit_text_file",
    "Edit binary file",         "edit_binary_file",
    "Grep current word",        "grep_function",
    "Italicise word",           "italicise_word",
    "Join next line",           "join_line",
    "Literal display mode",     "literal",
    "Menubar",                  "menubar",
    "Pipe region",              "pipe_region",
    "Toggle buffer flags",      "toggle_buffer_flags",
    "Buffer type",              "toogle_buffer_type",
    "Writeable buffer",         "make_writeable",
    "Zoom window",              "zoom",

    "<F1>",                     "execute_key <F1>",
    "<F2>",                     "execute_key <F2>",
    "<F3>",                     "execute_key <F3>",
    "<F4>",                     "execute_key <F4>",
    "<F5>",                     "execute_key <F5>",
    "<F6>",                     "execute_key <F6>",
    "<F7>",                     "execute_key <F7>",
    "<F8>",                     "execute_key <F8>",
    "<F9>",                     "execute_key <F9>",
    "<F10>",                    "execute_key <F10>"
    };


/*
 *  extra ---
 *      Macro to extend the number of editing keys available. We'll
 *      use ^A as an escape code to get to this macro.
 */
void
extra(void)
{
    int ch;

    if (-1 == (ch = read_char(HALF_SEC))) {
        keyboard_push();
        assign_to_key("<Ctrl-A>", "zoom");
        assign_to_key("<Ctrl-B>", "edit_binary_file");
        assign_to_key("<Ctrl-D>", "edit_text_file");
        assign_to_key("<Ctrl-E>", "edit_again");
        assign_to_key("<Ctrl-F>", "edit_dos_file");
        assign_to_key("<Ctrl-G>", "grep_function");
        assign_to_key("<Ctrl-I>", "italicise_word");
        assign_to_key("<Ctrl-J>", "join_line");
        assign_to_key("<Ctrl-L>", "literal");
        assign_to_key("<Ctrl-N>", "menubar");
        assign_to_key("<Ctrl-P>", "pipe_region");
        assign_to_key("<Ctrl-T>", "toggle_buffer_flags");

        assign_to_key("<Ctrl-1>", "execute_key <F1>");
        assign_to_key("<Ctrl-2>", "execute_key <F2>");
        assign_to_key("<Ctrl-3>", "execute_key <F3>");
        assign_to_key("<Ctrl-4>", "execute_key <F4>");
        assign_to_key("<Ctrl-5>", "execute_key <F5>");
        assign_to_key("<Ctrl-6>", "execute_key <F6>");
        assign_to_key("<Ctrl-7>", "execute_key <F7>");
        assign_to_key("<Ctrl-8>", "execute_key <F8>");
        assign_to_key("<Ctrl-9>", "execute_key <F9>");
        assign_to_key("<Ctrl-0>", "execute_key <F10>");
        select_feature(extra_list, 20);
        keyboard_pop();
        return;
    }

    switch (ch) {
    case key_to_int("<Ctrl-B>"):
        edit_binary_file();
        break;
    case key_to_int("<Ctrl-F>"):
        edit_dos_file();
        break;
    case key_to_int("<Ctrl-E>"):
        edit_again();
        break;
    case key_to_int("<Ctrl-T>"):
        toggle_buffer_flags();
        break;
    case key_to_int("<Ctrl-G>"):
        grep_function();
        break;
    case key_to_int("<Ctrl-I>"):
        italicise_word();
        break;
    case key_to_int("<Ctrl-J>"):
        join_line();
        break;
    case key_to_int("<Ctrl-L>"):
        literal();
        break;
    case key_to_int("<Ctrl-P>"):
        pipe_region();
        break;
    case key_to_int("<Ctrl-D>"):
        edit_text_file();
        break;
    case key_to_int("<Ctrl-W>"):
        make_writeable();
        break;
    case key_to_int("<Ctrl-A>"):
        zoom();
        break;
    case key_to_int("<Ctrl-N>"):
        menubar();
        break;

    case key_to_int("<Ctrl-1>"):
        push_back(key_to_int("<F1"));
        break;
    case key_to_int("<Ctrl-2>"):
        push_back(key_to_int("<F2"));
        break;
    case key_to_int("<Ctrl-3>"):
        push_back(key_to_int("<F3"));
        break;
    case key_to_int("<Ctrl-4>"):
        push_back(key_to_int("<F4"));
        break;
    case key_to_int("<Ctrl-5>"):
        push_back(key_to_int("<F5"));
        break;
    case key_to_int("<Ctrl-6>"):
        push_back(key_to_int("<F6"));
        break;
    case key_to_int("<Ctrl-7>"):
        push_back(key_to_int("<F7"));
        break;
    case key_to_int("<Ctrl-8>"):
        push_back(key_to_int("<F8"));
        break;
    case key_to_int("<Ctrl-9>"):
        push_back(key_to_int("<F9"));
        break;
    case key_to_int("<Ctrl-0>"):
        push_back(key_to_int("<F10"));
        break;
    }
}


/*
 *  execute_key ---
 *      Execute the given function-key.
 */
void
execute_key(string key)
{
    const int keycode = key_to_int(key);

    if (keycode > 0) {
        push_back(keycode);
    }
}


/*
 *  make_writeable ---
 *      Clears the Read-only flag for the current buffer.
 */
void
make_writeable(void)
{
    set_buffer_flags(NULL, NULL, ~BF_READONLY);
    message("Buffer now writable.");
}


/*
 *  edit_again ---
 *      Macro to re-read in a file. Used when a file has changed on
 *      disk and we want to see the latest version.
 */
void
edit_again(void)
{
    string file_name, arg;
    int curbuf, line;
    int l, c, ret;

    /* If buffer being viewed in more than one window then we can't do it  */
    if (inq_views() > 1) {
        error("Buffer being viewed in %d other windows.", inq_views() - 1);
        return;
    }

    /* Give user a last chance to lose the edits  */
    if (inq_modified()) {
        get_parm(0, arg, "Buffer modified -- are you sure (y/n) ? ");
        if (upper(substr(arg, 1, 1)) != "Y") {
            error("Aborted.");
            return;
        }
    }
    message("reloading.");

    inq_top_left(l, c);
    inq_names(file_name);
    inq_position(line);
    curbuf = inq_buffer();
    ret = edit_file(EDIT_AGAIN, file_name);
    if (inq_buffer() != curbuf) {
        delete_buffer(curbuf);
        set_top_left(l, c);
        goto_line(line);
    }
}


/*
 *  italicise_word ---
 *      Macro to italicise a word in nroff/troff format.
 */
void
italicise_word(void)
{
    default_word_left();
    insert("\\fI");
    re_search(NULL, "[ \t]|$");
    insert("\\fR");
}


void
bold_word(void)
{
    default_word_left();
    insert("\\fB");
    re_search(NULL, "[ \t]|$");
    insert("\\fR");
}


/*
 *  grep_function ---
 *      Do a grep on all .[ch] files in current directory on word
 *      where the cursor is. (Similar to a tags function)
 */
void
grep_function(void)
{
    string function;
    int i;

    save_position();
    re_search(SF_BACKWARDS, "<|{[~_A-Za-z0-9]\\c}");
    function = trim(read());
    i = re_search(NULL, "[~_A-Za-z0-9]", function);
    if (i > 0) {
        function = substr(function, 1, i - 1);
    }
    restore_position();
    if (function == "") {
        beep();
        return;
    }
    grep(function, "*.[ch]");
}


/*
 *  pipe_region ---
 *      This macro is used to pipe a region to an external command and replace the
 *      region with the output from the command.
 *
 *      This is similar to the !! feature of vi.
 */
void
pipe_region(~string)
{
    string tempbase = format("%s/gr%dXXXXX.tmp", inq_tmpdir(), getpid());
    string temp = mktemp(tempbase), command;
    int mark_set;

    get_parm(NULL, command, "Pipe to: ");
    if ("" == command) {
        message("");
        return;
    }

    /* If no marked region, then do the whole buffer */
    mark_set = inq_marked();
    if (!mark_set) {
        save_position();
        top_of_buffer();
        drop_anchor(MK_LINE);
        end_of_buffer();
        refresh();
    }

    write_block("| " + command + ">" + temp, NULL, TRUE);

    /* Handle deletion of column markers */
    if (inq_marked() == MK_COLUMN) {
        block_delete();
    } else {
        delete_block();
    }

    read_file(temp);
    remove(temp);

    if (! mark_set) {
        raise_anchor();
        restore_position();
    }
}


void
edit_dos_file(void)
{
    edit_typed_file("DOS", EDIT_CR);
}


void
edit_text_file(void)
{
    edit_typed_file("text", EDIT_ASCII);
}


void
edit_binary_file(void)
{
    edit_typed_file("binary", EDIT_BINARY);
}


static void
edit_typed_file(string msg, int flags)
{
    string filename;
    int ml = inq_msg_level();

    if (get_parm(NULL, filename, "Edit " + msg + " file: ") <= 0 || filename == "") {
        return;
    }

    set_msg_level(0);
    if (edit_file(flags, filename) != -1) {
        set_msg_level(ml);
        if (substr(inq_message(), 1, 4) == "Edit") {
            display_file_name();
        }
    }
}

/*end*/
