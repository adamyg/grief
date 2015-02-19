/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: os2.cr,v 1.7 2014/10/22 02:34:40 ayoung Exp $
 * Terminal descriptor for OS/2 vio sessions
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "os2");

    set_term_characters(
        213,                    /* Top left of window */
        184,                    /* Top right of window */
        212,                    /* Bottom left of window */
        190,                    /* Bottom right of window */
        179,                    /* Vertical bar for window sides */
        205,                    /* Top and bottom horizontal bar for window */
        0xd1,                   /* Top join */
        0xcf,                   /* Bottom join */
        0xd8,                   /* Window 4-way intersection */
        0xb5,                   /* Left hand join */
        0xc6                    /* Right hand join */
        );

    set_term_features(
        NULL,                   /* Sequence, clear 'n' spaces. */
        "%c",                   /* Sequence. print characters with top bitset */
        "\x1B[23m",             /* Sequence, insert-mode cursor. */
        "\x1B[24m",             /* Sequence, overwrite-mode cursor. */
        "\x1B[28;10;13m",       /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[28;1;8m",         /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B\x1B",             /* Sequence, print ESCAPE character graphically. */
        "\x1B[%db",             /* Sequence, repeat last character 'n' times. */
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC"              /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        KEY_DEL,        "\xD4",

        KEYPAD_0_9, quote_list(
            "\xCA",     "\xCB",     "\xCC",     "\xCD",     "\xCE",
            "\xCF",     "\xD0",     "\xD1",     "\xD2",     "\xD3"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x0",      "\xD9",     "\x0",      "\xDB",     "\xDC",
            "\x0",      "\xDE",     "\xDF",     "\x0",      "\xE1"),

        ALT_0_9, quote_list(
            "\x0",      "\x0",      "\x0",      "\x0",      "\x0",
            "\x0",      "\x0",      "\x0",      "\x0",      "\x0")
        );

    assign_to_key("<Mouse>", "::mouse");
}


/*
 *  Routines to support a mouse under os/2
 */
extern int popup_level;
extern int sel_warpable;

static int old_b1 = 0, old_b2 = 0, old_b3 = 0;
static int old_x = -1, old_y = -1, old_w = -1;
static int old_wh = -1;
static int mouse_cnt = 0;

static void
button_1_down(int win, int where, int x, int y)
{
    int curwin = inq_window();
    int line, col, buf;
    int lines, cols;
    int total;

    if (win > 0) {                              /* Inside of window region */
        if (win != curwin) {
            if (popup_level)                    /* Stay in context if in popup */
                return;
            if (win >= 0) {                     /* Switch windows */
                buf = inq_window_buf(win);
                set_window(win);
                set_buffer(buf);
            }
        }

        inq_top_left(line, col, win);
        inq_window_size(lines, cols);
        total = inq_lines();
        switch (where) {
        case MOBJ_INSIDE:
            if (win == curwin) {                /* If we clicked in current, move cursor there */
                line += y - 1;                  /* Calc new position in file */
                col += x;
                move_abs(line, col);            /* .. and move there */
                if (sel_warpable)               /* If doing popup do cursor line */
                    sel_warp();
            }                                   /* Otherwise we already changed windows */
            break;

        case MOBJ_TITLE:
            if (popup_level == 0)               /* Click on title, switches to next file */
                edit_next_buffer();
            break;

        case MOBJ_LEFT_EDGE:
            break;

        case MOBJ_RIGHT_EDGE:
            if (win == curwin && total > lines) {
                line = (y * total) / lines;
                move_abs(line, col);
            }
            break;

        case MOBJ_TOP_EDGE:
            break;

        case MOBJ_BOTTOM_EDGE:
            break;
        }
    }
}


static void
button_2_down()
{
}


static void
mouse(int win, int where, int x, int y, int b1, int b2, int b3)
{
    /*
     *  XXX - this is most likely broken as <Mouse> keys are no longer generated.
     *
        Shall need:

            int x, y, win, line, col, where;

            get_mouse_pos(x, y [, buttons]);
            where = translate_pos (x, y, win, line, col);
     */
    ++mouse_cnt;

    if (b1 && b2) {
        help();
    } else if (!old_b1 && b1) {
        button_1_down(win, where, x, y);
    }

    old_w  = win;
    old_x  = x;
    old_y  = y;
    old_b1 = b1;
    old_b2 = b2;
    old_b3 = b3;
    old_wh = where;
    refresh();
}

