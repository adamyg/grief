/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: acorn.cr,v 1.7 2014/10/22 02:34:38 ayoung Exp $
 * xterm running on an Acorn Archimedes under Unix
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "acorn");

    set_term_characters(
        "l",                    /* Top left of window */
        "k",                    /* Top right of window */
        "m",                    /* Bottom left of window */
        "j",                    /* Bottom right of window */
        "x",                    /* Vertical bar for window sides */
        "q",                    /* Top and bottom horizontal bar for window */
        "w",                    /* Top join */
        "v",                    /* Bottom join */
        "n",                    /* Window 4-way intersection */
        "u",                    /* Left hand join */
        "t"                     /* Right hand join */
        );

    set_term_features(
        NULL,                   /* Sequence, clear 'n' spaces. */
        NULL,                   /* Sequence. print characters with top bitset */
        NULL,                   /* Sequence, insert-mode cursor. */
        NULL,                   /* Sequence, overwrite-mode cursor. */
        NULL,                   /* Sequence, insert-mode cursor (on virtual space). */
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,                   /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        FALSE,                  /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE,                   /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,                  /* Boolean,  allow scrolling (ins/del). */
        "\x1B(0",               /* Sequence, enter graphics mode. */
        "\x1B(B"                /* Sequence, exit graphics mode. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[224z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",
            "\x1B[228z",    "\x1B[229z",    "\x1B[230z",    "\x1B[231z",
            "\x1B[232z",    "\x1B[233z",    "\x1B[234z",    "\x1B[235z"),

        SHIFT_F1_F12, quote_list(
            "\x1B[SF1~",    "\x1B[SF2~",    "\x1B[SF3~",    "\x1B[SF4~",
            "\x1B[SF5~",    "\x1B[SF6~",    "\x1B[SF7~",    "\x1B[SF8~",
            "\x1B[SF9~",    "\x1B[SF10~",   "\x1B[SF11~",   "\x1B[SF12~"),

        CTRL_F1_F12, quote_list(
            "\x1B[CF1~",    "\x1B[CF2~",    "\x1B[CF3~",    "\x1B[CF4~",
            "\x1B[CF5~",    "\x1B[CF6~",    "\x1B[CF7~",    "\x1B[CF8~",
            "\x1B[CF9~",    "\x1B[CF10~",   "\x1B[CF11~",   "\x1B[CF12~"),

        ALT_F1_F12, quote_list(
            "\x1B[mF1~",    "\x1B[mF2~",    "\x1B[mF3~",    "\x1B[mF4~",
            "\x1B[mF5~",    "\x1B[mF6~",    "\x1B[mF7~",    "\x1B[mF8~",
            "\x1B[mF9~",    "\x1B[mF10~",   "\x1B[mF11~",   "\x1B[mF12~"),

        ALT_A_Z, quote_list(
            "\xe6",         "\xe2",         "\xa2",         "\xf0",         "\xe5", /* a-e */
            "\x81",         "\xe7",         "\xe8",         "\xe9",         "\xea", /* f-j */
            "\xeb",         "\xec",         "\xb5",         "\xee",         "\xf8",
            "\xfe",         "\xf1",         "\xb6",         "\xdf",         "\xf4",
            "\xF5",         "\xf6",         "\xf7",         "\xbb",         "\x80",
            "\xab"),

        ALT_0_9, quote_list(
            "\x1B0",        "\x1B1",        "\x1B2",        "\x1B3",        "\x1B4",
            "\x1B5",        "\x1B6",        "\x1B7",        "\x1B8",        "\x1B9"),

        KEYPAD_0_9, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",
            "\x1BOu",       "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy"),

        KEYPAD_2,           "\x1BOB",
        KEYPAD_4,           "\x1BOD",
        KEYPAD_6,           "\x1BOC",
        KEYPAD_8,           "\x1BOA",

        COPY,               "\x1BOk",
        CUT,                "\x1BOm",
        BACK_TAB,           "\x1B\t",
        KEY_INS,            "\x1B[2z",
        KEY_COPY,           "\x1B[4z",
        KEY_PAGEUP,         "\x1B[5z",
        KEY_PAGEDOWN,       "\x1B[6z",
        MOUSE_KEY,          "\x1B[M"
        );
}
