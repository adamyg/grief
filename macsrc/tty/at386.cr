/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: at386.cr,v 1.6 2014/10/22 02:34:39 ayoung Exp $
 * Terminal description file for Microport System V/386 Console (Colour display)
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "at386");

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
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
        "\x1B[11m%c\x1B[10m",   /* Sequence. print characters with top bitset */
        "\x1B[23m",             /* Sequence, insert-mode cursor. */
        "\x1B[24m",             /* Sequence, overwrite-mode cursor. */
        "\x1B[28;10;13m",       /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[28;1;8m",         /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B\x1B",             /* Sequence, print ESCAPE character graphically. */
        "\x1B[%db",             /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        FALSE,                  /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,                  /* Boolean,  allow scrolling (ins/del). */
        "\x1B[11m",             /* Sequence, enter graphics mode. */
        "\x1B[10m",             /* Sequence, exit graphics mode. */
        NULL,                   /* Sequence, Init */
        "\x1B[0m"               /* Sequence, Reset */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1BOP",       "\x1BOQ",       "\x1BOR",       "\x1BOS",       "\x1BOT",       "\x1BOU",
            "\x1BOV",       "\x1BOW",       "\x1BOX",       "\x1BOY",       "\x1BOZ",       "\x1BO["),

        SHIFT_F1_F12, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",       "\x1BOu",
            "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy",       "\x1BOz",       "\x1BO{"),

        CTRL_F1_F12, quote_list(
            "\x1BO\x10",    "\x1BO\x11",    "\x1BO\x12",    "\x1BO\x13",    "\x1BO\x14",    "\x1BO\x15",
            "\x1BO\x16",    "\x1BO\x17",    "\x1BO\x18",    "\x1BO\x19",    "\x1BO\x1a",    "\x1BO\x1B"),

        ALT_A_Z, quote_list(
            "\x1BNa",       "\x1BNb",       "\x1BNc",       "\x1BNd",       "\x1BNe",       "\x1BNf",
            "\x1BNg",       "\x1BNh",       "\x1BNi",       "\x1BNj",       "\x1BNk",       "\x1BNl",
            "\x1BNm",       "\x1BNn",       "\x1BNo",       "\x1BNp",       "\x1BNq",       "\x1BNr",
            "\x1BNs",       "\x1BNt",       "\x1BNu",       "\x1BNv",       "\x1BNw",       "\x1BNx",
            "\x1BNy",       "\x1BNz"),

        KEYPAD_0_9, quote_list(
            "\x1B[@",       "\x1B[Y",       "\x1B[B",       "\x1B[U",       "\x1B[D",       "\x1B[G",
            "\x1B[C",       "\x1B[H",       "\x1B[A",       "\x1B[V"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B?0",       "\x1B?1",       "\x1B?2",       "\x1B?3",       "\x1B?4",       "\x1B?5",
            "\x1B?6",       "\x1B?7",       "\x1B?8",       "\x1B?9"),

        ALT_0_9, quote_list(
            "\x1BN0",       "\x1BN1",       "\x1BN2",       "\x1BN3",       "\x1BN4",       "\x1BN5",
            "\x1BN6",       "\x1BN7",       "\x1BN8",       "\x1BN9"),

        CUT,                "\x1B[S",
        COPY,               "\x1B[T",
        BACK_TAB,           "\x1B[Z"
        );
}


