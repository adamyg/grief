/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xgrief.cr,v 1.7 2014/10/22 02:34:43 ayoung Exp $
 * xterm running as xgrief
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "xgrief");

    set_term_characters(
        "l",                    /* Top left of window. */
        "k",                    /* Top right of window. */
        "m",                    /* Bottom left of window. */
        "j",                    /* Bottom right of window. */
        "x",                    /* Vertical bar for window sides. */
        "q",                    /* Top and bottom horizontal bar for window. */
        "w",                    /* Top join. */
        "v",                    /* Bottom join. */
        "n",                    /* Window 4-way intersection. */
        "u",                    /* Left hand join. */
        "t"                     /* Right hand join. */
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
        COPY,               "\x1BOk",
        CUT,                "\x1BOm",
        BACK_TAB,           "\x1B\t",
        MOUSE_KEY,          "\x1B[M",

        F1_F12, quote_list(
            "\x1B[224z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",
            "\x1B[228z",    "\x1B[229z",    "\x1B[230z",    "\x1B[231z",
            "\x1B[232z",    "\x1B[233z",    "\x1B[234z",    "\x1B[235z"),

        F1_F12, quote_list(
            "\x1B[NL1~",    "\x1B[NL2~",    "\x1B[NL3~",    "\x1B[NL4~",
            "\x1B[NL5~",    "\x1B[NL6~",    "\x1B[NL7~",    "\x1B[NL8~",
            "\x1B[NL9~",    "\x1B[NL10~"),

        SHIFT_F1_F12, quote_list(
            "\x1B[SF1~",    "\x1B[SF2~",    "\x1B[SF3~",    "\x1B[SF4~",
            "\x1B[SF5~",    "\x1B[SF6~",    "\x1B[SF7~",    "\x1B[SF8~",
            "\x1B[SF9~",    "\x1B[SF10~",   "\x1B[SF11~",   "\x1B[SF12~"),

        SHIFT_KEYPAD_2,     "\x1BOr",
        SHIFT_KEYPAD_4,     "\x1BOt",
        SHIFT_KEYPAD_6,     "\x1BOv",
        SHIFT_KEYPAD_8,     "\x1BOx",

        CTRL_F1_F12, quote_list(
            "\x1B[CF1~",    "\x1B[CF2~",    "\x1B[CF3~",    "\x1B[CF4~",
            "\x1B[CF5~",    "\x1B[CF6~",    "\x1B[CF7~",    "\x1B[CF8~",
            "\x1B[CF9~",    "\x1B[CF10~",   "\x1B[CF11~",   "\x1B[CF12~"),

        ALT_F1_F12, quote_list(
            "\x1B[mF1~",    "\x1B[mF2~",    "\x1B[mF3~",    "\x1B[mF4~",
            "\x1B[mF5~",    "\x1B[mF6~",    "\x1B[mF7~",    "\x1B[mF8~",
            "\x1B[mF9~",    "\x1B[mF10~",   "\x1B[mF11~",   "\x1B[mF12~"),

        ALT_A_Z, quote_list(
            "\x1Ba",        "\x1Bb",        "\x1Bc",        "\x1Bd",        "\x1Be",
            "\x1Bf",        "\x1Bg",        "\x1Bh",        "\x1Bi",        "\x1Bj",
            "\x1Bk",        "\x1Bl",        "\x1Bm",        "\x1Bn",        "\x1Bo",
            "\x1Bp",        "\x1Bq",        "\x1Br",        "\x1Bs",        "\x1Bt",
            "\x1Bu",        "\x1Bv",        "\x1Bw",        "\x1Bx",        "\x1By",
            "\x1Bz"),

        ALT_0_9, quote_list(
            "\x1B0",        "\x1B1",        "\x1B2",        "\x1B3",        "\x1B4",
            "\x1B5",        "\x1B6",        "\x1B7",        "\x1B8",        "\x1B9"),

        KEYPAD_0_9, quote_list(
            "\x1B[NR16~",   "\x1B[220z",    "\x1BOB",       "\x1B[222z",
            "\x1BOD",       "\x1B[218z",    "\x1BOC",       "\x1B[214z",
            "\x1BOA",       "\x1B[216z"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B[CR16~",   "\x1B[CR13~",   "\x1B[CR14~",   "\x1B[CR15~",
            "\x1B[CR10~",   "\x1B[CR11~",   "\x1B[CR12~",   "\x1B[CR7~",
            "\x1B[CR8~",    "\x1B[CR9~"),

        ALT_KEYPAD_END,     "\x1B[mR13~",
        ALT_KEYPAD_HOME,    "\x1B[mR7~",
        ALT_KEYPAD_MINUS,   "\x1B[KM-~",
        CTRL_KEYPAD_MINUS,  "\x1B[KC-~",
        CTRL_KEYPAD_PLUS,   "\x1b[KC+~",
        KEYPAD_PRTSC,       "\x1b[209z",
        KEYPAD_SCROLL,      "\x1b[210z",
        SHIFT_KEYPAD_MINUS, "\x1b[SK-~",
        SHIFT_KEYPAD_PLUS,  "\x1b[SK+~"
        );
}


void
nf()
{
    set_term_characters(
        0xF5,                   /* Top left of window */
        0xF6,                   /* Top right of window. */
        0xF4,                   /* Bottom left of window. */
        0xF7,                   /* Bottom right of window. */
        0xF9,                   /* Vertical bar for window sides. */
        0xFA,                   /* Top and bottom horizontal bar for win */
        0xFD,                   /* Top join. */
        0xFE,                   /* Bottom join. */
        0xF8,                   /* Window 4-way intersection. */
        0xFB,                   /* Left hand join. */
        0xFC                    /* Right hand join. */
        );
}

