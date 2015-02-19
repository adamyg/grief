/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dumb.cr,v 1.7 2014/10/27 23:28:21 ayoung Exp $
 * Default key bindings for dumb terminals
 * This file contains a set of keybindings to enable GRIEF to be
 * used even in the absence of an ALT or META key on the
 * keyboard. It should be run on startup if you have problems.
 */

void
dumb()
{
    assign_to_key("<Ctrl-A>",   "mark 1");
    assign_to_key("<Ctrl-B>",   "buffer_list 1");
    assign_to_key("<Ctrl-C>",   "mark 2");
    assign_to_key("<Ctrl-D>",   "delete_line");
    assign_to_key("<Ctrl-E>",   "edit_file");
    assign_to_key("<Ctrl-F>",   "display_file_name");
    assign_to_key("<Ctrl-G>",   "goto_line");
    assign_to_key("<Ctrl-H>",   "backspace");
    assign_to_key("<Ctrl-I>",   "self_insert");
    assign_to_key("<Ctrl-J>",   "open_line");
    assign_to_key("<Ctrl-K>",   "delete_to_eol");
    assign_to_key("<Ctrl-L>",   "mark 3");
    assign_to_key("<Ctrl-M>",   "self_insert");
    assign_to_key("<Ctrl-N>",   "edit_next_buffer");
    assign_to_key("<Ctrl-O>",   "output_file");
    assign_to_key("<Ctrl-P>",   "previous_alpha_buffer");
    assign_to_key("<Ctrl-Q>",   "quote");
    assign_to_key("<Ctrl-R>",   "read_file");
    assign_to_key("<Ctrl-S>",   "search__fwd");
    assign_to_key("<Ctrl-T>",   "translate");
    assign_to_key("<Ctrl-U>",   "undo");
    assign_to_key("^V^B",       "set_bottom_of_window");
    assign_to_key("^V^C",       "set_center_of_window");
    assign_to_key("^V^D",       "page_down");
    assign_to_key("^V^G",       "routines");
    assign_to_key("^V^H",       "help");
    assign_to_key("^V^J",       "goto_bookmark");
    assign_to_key("^V^K",       "objects delete_word_left");
    assign_to_key("^V^L",       "objects delete_word_right");
    assign_to_key("^V^N",       "next_error");
    assign_to_key("^V^P",       "next_error 1");
    assign_to_key("^V^R",       "repeat");
    assign_to_key("^V^T",       "set_top_of_window");
    assign_to_key("^V^U",       "page_up");
    assign_to_key("^V^V",       "version");
    assign_to_key("^V^W",       "set_backup");
    assign_to_key("<Ctrl-W>",   "write_buffer");
    assign_to_key("<Ctrl-X>",   "exit");
    assign_to_key("<Ctrl-Y>",   "self_insert");
    assign_to_key("<Ctrl-Z>",   "dos");
}

/*eof*/
