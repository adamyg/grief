/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: objects.cr,v 1.9 2024/05/15 08:22:44 cvsuser Exp $
 * Document object macros.
 *
 *
 */

#include "grief.h"

static string word_chars   = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static string space_chars  = " \t";
static string unword_chars = "]-\"'\t.>=";


/*
 *  macro load
 */
void
main(void)
{
    require("indent");
}



/*
 *  Locate the specific 'object'
 */
void
objects(string function, ~declare arg1)
{
    string ext, macro_name;

    inq_names(NULL, ext, NULL);
    macro_name = ext + "_" + function;
    if (inq_macro(macro_name) <= 0)
        macro_name = "default_" + function;
    execute_macro(macro_name, arg1);
}


/*
 *  Macros to shift left & shift right the currently marked block.
 */
void
shift_right(void)
{
    objects("shift_right");
}


/*
 *  Define synonyms for shift_right to make it easier to type
 */
void
shiftr(void)
{
    shift_right();
}


void
rshift(void)
{
    shift_right();
}


void
shift_left(~declare)
{
    int count = 1;
    declare arg;

    if (get_parm(0, arg) > 0) {
        if (is_string(arg)) {
            count = atoi(arg);
        } else if (is_integer(arg)) {
            count = arg;
        }
    }

    if (count <= 0)
        count = 1;
    while (count-- > 0) {
        objects("shift_left");
    }
}


/*
 *  Define synonyms for shift_left to make it easier to type
 */
void
shiftl(~declare arg)
{
    shift_left(arg);
}


void
lshift(~declare arg)
{
    shift_left(arg);
}


void
default_shift_right(void)
{
    int marked = inq_marked();

    if (marked) {
        _slide_in();                            /* 11/04/10 */

    } else {
        int use_tabs = use_tab_char("y");

        use_tab_char(use_tabs ? "y" : "n");
        if (marked == 0)
            drop_anchor(MK_LINE);
        beginning_of_line();
        re_translate(SF_GLOBAL | SF_BLOCK, "<", use_tabs ? "\t" : " ");
        if (marked == 0)
            raise_anchor();
    }
}


void
default_shift_left(void)
{
    int marked = inq_marked();

    if (marked) {
        _slide_out();                           /* 11/04/10 */

    } else {
        drop_anchor(MK_LINE);
        beginning_of_line();
        re_translate(SF_GLOBAL | SF_BLOCK, "<{\t}|{ }", "");
        raise_anchor();
    }
}


/*
 *  Delete word left/right macros.
 *  Uses the word_left/word_right macros.
 */
void
default_delete_word_right(void)
{
    delete_word(default_word_right());
}


void
default_delete_word_left(void)
{
    delete_word(default_word_left());
}


void
delete_word(~declare)
{
    int i;

    drop_anchor(MK_NONINC);
    get_parm(0, i);                             /* Get argument as a side effect only */
    delete_block();
}


/*
 *  word_left macros
 */
int
default_word_left(void)
{
    return word_left("<|[ .()/\t]\\c[~ .()/\t]");
}


int
word_left(string pat)
{
    int line, col, line1, col1;

    inq_position(line, col);
    re_search(SF_BACKWARDS | SF_MAXIMAL, pat);
    inq_position(line1, col1);
    if (line == line1 && col == col1) {
        prev_char();
        re_search(SF_BACKWARDS | SF_MAXIMAL, pat);
    }
    return 0;
}


/*
 *  word_right macros
 */
void
default_word_right(void)
{
    word_right();
}


/*
 *  Vi compatible word right function
 */
void
word_right(void)
{
    string ch = trim(read(1));
    int col;

    if (ch == "") {
        if (re_search(NULL, "?") <= 0) {
            beep();
            return;
        }
        ch = read(1);
    } else {
        inq_position(NULL, col);
        if (col == 1) {
            next_char();
        }
    }

    if (index(word_chars, ch)) {
        re_search(NULL, "^|[^" + word_chars + "]");
        re_search(NULL, "[^" + space_chars + "]");
    } else if (index(space_chars, ch)) {
        re_search(NULL, "^|[^" + space_chars + "]");
    } else {
        re_search(NULL, "^|[" + word_chars + space_chars + "]");
    }

    if (index(space_chars, read(1))) {
        re_search(NULL, "[^" + space_chars + "]");
    }
}

/*end*/
