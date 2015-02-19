/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: hanoi.cr,v 1.8 2014/10/22 02:34:29 ayoung Exp $
 * Tower of Hanio.
 *
 *
 */

#include "../grief.h"

#define WIDTH     26

static void       hanoi0(int n, int sn, int dn, int hn);
static void       move_piece(int from, int to);
static void       display_disc(void);
static void       replace_string(string str);

static list       msgs = {
   "                                                                                    \n",
   "                                                                                    \n",
   "                                                                                    \n",
   "           !                         !                         !                    \n",
   "          xxx                        !                         !                    \n",
   "         xxxxx                       !                         !                    \n",
   "        xxxxxxx                      !                         !                    \n",
   "       xxxxxxxxx                     !                         !                    \n",
   "      xxxxxxxxxxx                    !                         !                    \n",
   "     xxxxxxxxxxxxx                   !                         !                    \n",
   "    xxxxxxxxxxxxxxx                  !                         !                    \n",
   "   xxxxxxxxxxxxxxxxx                 !                         !                    \n",
   "  xxxxxxxxxxxxxxxxxxx                !                         !                    \n",
   " xxxxxxxxxxxxxxxxxxxxx               !                         !                    \n",
   "xxxxxxxxxxxxxxxxxxxxxxx              !                         !                    \n"
   };



void
hanoi()
{
    int discs = 3;
    int i, buf, new_buf;

    if (get_parm(0, discs, "Number of discs (1-10): ") <= 0 || discs <= 0) {
        discs = 3;
    } else if (discs > 11) {
        discs = 11;
    }
    buf = inq_buffer();
    new_buf = create_buffer("Tower of Hanoi", NULL, 1);
    set_buffer(new_buf);
    attach_buffer(new_buf);
    clear_buffer();

    /****/
    for (i = 0; i < discs + 4; i++) {
        insert(msgs[i]);
    }
    insert("=============================================================================\n");
    hanoi0(discs, 1, 3, 2);
    if (inq_kbd_char()) {
        read_char();
        message("I've had enough of this!");
    }
    message("Press any key to continue");
    read_char();
    message("");
    set_buffer(buf);
    attach_buffer(buf);
}


static void
hanoi0(int n, int sn, int dn, int hn)
{
    if (inq_kbd_char())
        return;

    if (n > 0) {
        hanoi0(n - 1, sn, hn, dn);
        if (inq_kbd_char())
        return;
        move_piece(sn, dn);
        hanoi0(n - 1, hn, dn, sn);
    }
}


static void
move_piece(int from, int to)
{
    local  int width;
    int    i, j, col, col1, col2, lines;
    string blanks, disc;

    top_of_buffer();
    for (i = from; i > 0; --i) {
        re_search(NULL, "!");
        right();
    }
    left();
    inq_position(NULL, col);

    while (read(1) == "!") {
        ++lines;
        down();
    }
    re_search(SF_BACKWARDS, " \\c");
    inq_position(NULL, col1);
    re_search(SF_MAXIMAL, "x@\\c");
    inq_position(NULL, col2);
    refresh();

    move_abs(0, col1);
    width = col2 - col1;
    disc = read(width);
    up();
    move_abs(0, col1);
    blanks = read(width);
    down();

    for (j = lines; j >= 0; --j) {
        replace_string(blanks);
        up();
        replace_string(disc);
        display_disc();
    }

    if (to > from) {
        j = (to - from) * WIDTH;
    } else {
        j = (from - to) * WIDTH;
    }

    for (j /= 2; j > 0; --j) {
        if (to > from) {
            insert("  ");
            inq_position(NULL, col);
            end_of_line();
            left(2);
            delete_char(2);
            move_abs(0, col);
        } else {
            left(2);
            inq_position(NULL, col);
            delete_char(2);
            end_of_line();
            insert("  ");
            move_abs(0, col);
        }
        display_disc();
    }
    save_position();
    replace_string(blanks);
    re_search(NULL, "!");
    delete_char();
    insert(" ");
    restore_position();
    down();
    replace_string(disc);
    display_disc();
    while (1) {
        replace_string(blanks);
        down();
        replace_string(disc);
        display_disc();
        down();
        if (read(1) != " ")
            break;
        up();
    }
}


static void
display_disc(void)
{
    extern int width;

    move_rel(0, width);
    refresh();
    move_rel(0, -width);
}


static void
replace_string(string str)
{
    int col;

    inq_position(NULL, col);
    delete_char(strlen(str));
    insert(str);
    move_abs(0, col);
}
