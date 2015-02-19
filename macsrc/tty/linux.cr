/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: linux.cr,v 1.8 2014/10/22 02:34:40 ayoung Exp $
 * Terminal description for the Linux console colour display
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "linux");

    set_term_characters(
        "l",                        /* Top left of window. */
        "k",                        /* Top right of window. */
        "m",                        /* Bottom left of window. */
        "j",                        /* Bottom right of window. */
        "x",                        /* Vertical bar for window sides. */
        "q",                        /* Top and bottom horizontal bar for window.*/
        "w",                        /* Top join. */
        "v",                        /* Bottom join. */
        "n",                        /* Window 4-way intersection. */
        "u",                        /* Left hand join. */
        "t"                         /* Right hand join. */
        );

    set_term_features(
        "\x1B[%dX",                 /* Sequence, clear 'n' spaces. */
        "\x1B[11m%c\x1B[10m",       /* Sequence. print characters with top bitset */
        "\x1B[23m",                 /* Sequence, insert-mode cursor. */
        "\x1B[24m",                 /* Sequence, overwrite-mode cursor. */
	     "\x1B[28;10;13m",           /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[28;1;8m",             /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B\x1B",                 /* Sequence, print ESCAPE character graphically. */
        "\x1B[%db",                 /* Sequence, repeat last character 'n' times. */
        FALSE,                      /* Boolean,  ESC [0m resets color. */
        TRUE,                       /* Boolean,  terminal supports color. */
        "\x1B[%dC",                 /* Sequence, move cursor on same line. */
        FALSE,                      /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                       /* Boolean,  allow scrolling (ins/del). */
        "\x1B(0",                   /* Sequence, enter graphics mode. */
        "\x1B(B"                    /* Sequence, exit graphics mode. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[[A",      "\x1B[[B",      "\x1B[[C",      "\x1B[[D",      "\x1B[[E",
            "\x1B[17~",     "\x1B[18~",     "\x1B[19~",     "\x1B[20~",     "\x1B[21~",
            "\x1B[23~",     "\x1B[24~" ),

        SHIFT_F1_F12, quote_list(
            NULL,           NULL,           "\x1B[25~",     "\x1B[26~",     "\x1B[28~",
            "\x1B[29~",     "\x1B[31~",     "\x1B[32~",     "\x1B[33~",     "\x1B[34~",
            NULL,           NULL),

        SHIFT_F1_F12, quote_list(
            "\x1B[SF1~",    "\x1B[SF2~",    "\x1B[SF3~",    "\x1B[SF4~",    "\x1B[SF5~",
            "\x1B[SF6~",    "\x1B[SF7~",    "\x1B[SF8~",    "\x1B[SF9~",    "\x1B[SF10~",
            "\x1B[SF11~",   "\x1B[SF12~"),

        CTRL_F1_F12, quote_list(
            "\x1B[CF1~",    "\x1B[CF2~",    "\x1B[CF3~",    "\x1B[CF4~",
            "\x1B[CF5~",    "\x1B[CF6~",    "\x1B[CF7~",    "\x1B[CF8~",
            "\x1B[CF9~",    "\x1B[CF10~",   "\x1B[CF11~",   "\x1B[CF12~"),

        ALT_A_Z, quote_list(
            "\x1Ba",        "\x1Bb",        "\x1Bc",        "\x1Bd",        "\x1Be",
            "\x1Bf",        "\x1Bg",        "\x1Bh",        "\x1Bi",        "\x1Bj",
            "\x1Bk",        "\x1Bl",        "\x1Bm",        "\x1Bn",        "\x1Bo",
            "\x1Bp",        "\x1Bq",        "\x1Br",        "\x1Bs",        "\x1Bt",
            "\x1Bu",        "\x1Bv",        "\x1Bw",        "\x1Bx",        "\x1By",
            "\x1Bz"),

        KEYPAD_0_9, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",
            "\x1BOu",       "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy"),

        KEYPAD_0_9, quote_list(
            "\x1B[2~",      "\x1B[4~",      "\x1B[B",       "\x1B[6~",      "\x1B[D",
            "\x1B[G",       "\x1B[C",       "\x1B[1~",      "\x1B[A",       "\x1B[5~"),

        KEY_DEL,            "\x1B[3~",
        CTRL_H,             "\x1B",
        CUT,                "\x1BOS",
        COPY,               "\x1BOl",
        KEYPAD_STAR,        "\x1BOR",
        KEYPAD_DEL,         "\x1BOn",
        KEYPAD_DEL,         "\x1B[3~",
        0x08,               "\x7f",

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           NULL,           NULL,           NULL,           "\x1B[232z",
            NULL,           "\x1B[231z"),

        SHIFT_KEYPAD_0_9, quote_list(
            "\x1b[SR16~",    "\x1b[SR13~",   "\x1b[SR14~",   "\x1b[SR15~",
            "\x1b[SR10~",    "\x1b[SR11~",   "\x1b[SR12~",   "\x1b[SR7~",
            "\x1b[SR8~",     "\x1b[SR9~"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B[?CR16~",  "\x1B[?CR13~",  "\x1B[?CR14~",  "\x1B[?CR15~",
            "\x1B[?CR10~",  "\x1B[?CR11~",  "\x1B[?CR12~",  "\x1B[?CR7~",
            "\x1B[?CR8~",   "\x1B[?CR9~"),

        ALT_0_9, quote_list(
            "\x1B0",        "\x1B1",        "\x1B2",        "\x1B3",
            "\x1B4",        "\x1B5",        "\x1B6",        "\x1B7",
            "\x1B8",        "\x1B9")
        );
}

