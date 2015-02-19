/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dec.cr,v 1.6 2014/10/22 02:34:39 ayoung Exp $
 * terminal description for VT-100/VT-200 type terminals.
 *
 * If you are going to use this then you will need to put the following csh
 * commands in your .login depending on system type:
 *
 *      alias cr cr -medt       # only if you want edt startup
 *      setenv TERM vt200       # or vt100 as appropriate
 *      setenv GRTERM dec
 *
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "dec");

    set_term_characters(
        "l",                /* Top left of window */
        "k",                /* Top right of window */
        "m",                /* Bottom left of window */
        "j",                /* Bottom right of window */
        "x",                /* Vertical bar for window sides */
        "q",                /* Top and bottom horizontal bar for window */
        "w",                /* Top join */
        "v",                /* Bottom join */
        "n",                /* Window 4-way intersection */
        "u",                /* Left hand join */
        "t"                 /* Right hand join */
        );

    set_term_features(
        NULL,               /* Sequence, clear 'n' spaces. */
        NULL,               /* Sequence. print characters with top bitset */
        NULL,               /* Sequence, insert-mode cursor. */
        NULL,               /* Sequence, overwrite-mode cursor. */
        NULL,               /* Sequence, insert-mode cursor (on virtual space). */
        NULL,               /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,               /* Sequence, print ESCAPE character graphically. */
        NULL,               /* Sequence, repeat last character 'n' times. */
        FALSE,              /* Boolean,  ESC [0m resets color. */
        FALSE,              /* Boolean,  terminal supports color. */
        "\x1B[%dC",         /* Sequence, move cursor on same line. */
        TRUE,               /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,              /* Boolean,  allow scrolling (ins/del). */
        "\x1B(0",           /* Sequence, enter graphics mode. */
        "\x1B(B"            /* Sequence, exit graphics mode. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[17~",     "\x1B[18~",     "\x1B[19~",     "\x1B[20~",     "\x1B[21~",
            "\x1B[23~",     "\x1B[24~",     "\x1B[25~",     "\x1B[26~",     "\x1B[31~"),

        SHIFT_F1_F12, quote_list(
            "\x1B[224z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",    "\x1B[228z",
            "\x1B[229z",    "\x1B[230z",    "\x1B[231z",    "\x1B[232z"),

#if defined(XTERM_VT100_KEYS)
        /*
         *  Uncomment these lines if you want support for xterm with VT100 key bindings
         */
        F1_F12, quote_list(
            "\x1B[23~",     "\x1B[24~",     "\x1B[25~",     "\x1B[26~",     "\x1B[28~",
            "\x1B[29~",     "\x1B[31~",     "\x1B[32~",     "\x1B[33~",     "\x1B[34~"),

        SHIFT_F1_F12, quote_list(
            "\x1B[11~",     "\x1B[12~",     "\x1B[13~",     "\x1B[14~",     "\x1B[15~",
            "\x1B[16~",     "\x1B[17~",     "\x1B[18~",     "\x1B[19~",     "\x1B[20~"),
#endif

        ALT_A_Z, quote_list(
            "\x1Ba",        "\x1Bb",        "\x1Bc",        "\x1Bd",        "\x1Be",
            "\x1Bf",        "\x1Bg",        "\x1Bh",        "\x1Bi",        "\x1Bj",
            "\x1Bk",        "\x1Bl",        "\x1Bm",        "\x1Bn",        "\x1Bo",
            "\x1Bp",        "\x1Bq",        "\x1Br",        "\x1Bs",        "\x1Bt",
            "\x1Bu",        "\x1Bv",        "\x1Bw",        "\x1Bx",        "\x1By",
            "\x1Bz"),

        KEYPAD_0_9, quote_list(
            "\x1B[212z",    "\x1B[220z",    "\x1BOB",       "\x1B[6~",      "\x1BOD",
            NULL,           "\x1BOC",       "\x1B[214z",    "\x1BOA",       "\x1B[5~",
            NULL,           "\x1B[213z",    "\x1B[34~"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           NULL,           NULL,           "\x1B[232z",    "\x1B[208z",
            NULL,           "\x1B[209z",    NULL,            NULL,          "\x1B[231z"),

        ALT_0_9, quote_list(
            "\x1B1",        "\x1B2",        "\x1B3",        "\x1B4",        "\x1B5",
            "\x1B6",        "\x1B7",        "\x1B8",        "\x1B9",        "\x1B0")
        );
}



void
xterm_arrow()
{
    set_term_keyboard(
        KEYPAD_0_9, quote_list(
            "\x1B[212z",    "\x1B[220z",    "\x1B[B",       "\x1B[222z",
            "\x1B[D",       NULL,           "\x1B[C",       "\x1B[214z",
            "\x1B[A")
            );
}

