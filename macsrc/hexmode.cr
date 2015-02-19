/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: hexmode.cr,v 1.8 2014/10/27 23:28:22 ayoung Exp $
 * Hex mode for editing a file in hex or octal
 *
 *  This macro contains code which makes the current window viewable through the
 *  hex character map (see view.cr) and then understands what happens if the user
 *  tries to modify the bytes displayed on the screen.
 *
 *
 *
 */

#include "grief.h"

extern int        hex_cmap;
static int        hex_kbd;

void
main()
{
    keyboard_push();
    hex_kbd = inq_keyboard();
    assign_to_key("0", "::hex_insert 0");
    assign_to_key("1", "::hex_insert 1");
    assign_to_key("2", "::hex_insert 2");
    assign_to_key("3", "::hex_insert 3");
    assign_to_key("4", "::hex_insert 4");
    assign_to_key("5", "::hex_insert 5");
    assign_to_key("6", "::hex_insert 6");
    assign_to_key("7", "::hex_insert 7");
    assign_to_key("8", "::hex_insert 8");
    assign_to_key("9", "::hex_insert 9");
    assign_to_key("a", "::hex_insert 10");
    assign_to_key("b", "::hex_insert 11");
    assign_to_key("c", "::hex_insert 12");
    assign_to_key("d", "::hex_insert 13");
    assign_to_key("e", "::hex_insert 14");
    assign_to_key("f", "::hex_insert 15");
    assign_to_key("A", "::hex_insert 10");
    assign_to_key("B", "::hex_insert 11");
    assign_to_key("C", "::hex_insert 12");
    assign_to_key("D", "::hex_insert 13");
    assign_to_key("E", "::hex_insert 14");
    assign_to_key("F", "::hex_insert 15");
    keyboard_pop(1);
}


void
hexmode()
{
    view("hex");
    use_local_keyboard(hex_kbd);
}


static void
hex_insert(int n)
{
    int col;
    int end_col;
    int ch;

    /*
     *  If user stopped being in hex mode then just insert the character
     */
    if (inq_char_map() != hex_cmap) {
        self_insert();
        return;
    }
    inq_position(NULL, col);
    save_position();
    end_of_line();
    inq_position(NULL, end_col);
    restore_position();

    /*
     *  If user is trying to type in after end of line then append to the
     *  end of the line (dont space or tab fill)
     */
    if (col > end_col)
        end_of_line();

    inq_position(NULL, col);

    switch ((col - 1) % 3) {
    case 0:
        ch = atoi(read(1), 0);
        delete_char();
        insert((n << 4) | (ch & 0x0f));
        break;

    case 1:
        ch = atoi(read(1), 0);
        delete_char();
        insert((ch & 0xf0) | n);
        col++;                  /* Skip over the space that comes after this nibble */
        break;

    case 2:
        /* Ignore entry if user is typing whilst sitting on the space
         * separating characters
         */
        break;
    }

    if (col + 1 >= end_col) {
        down();
        beginning_of_line();
    } else {
        move_abs(NULL, col + 1);
    }
}

/*eof*/
