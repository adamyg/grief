/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: m303.cr,v 1.5 2014/10/22 02:34:40 ayoung Exp $
 * ICLs mono graphics display
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "m303");

    set_term_characters(
        12,                     /* Top left of window */
        24,                     /* Top right of window */
        14,                     /* Bottom left of window */
        15,                     /* Bottom right of window */
        6,                      /* Vertical bar for window sides */
        11,                     /* Top and bottom horizontal bar for window */
        '+',                    /* Top join */
        '+',                    /* Bottom join */
        '+',                    /* Window 4-way intersection */
        '+',                    /* Left hand join */
        '+'                     /* Right hand join */
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
        NULL                    /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1b\x01",     "\x1b\x02",     "\x1b\x03",     "\x1b\x04",     "\x1b\x05",
            "\x1b\x06",     "\x1b\x07",     "\x1b\x08",     "\x1b\x09",     "\x1b\x0a"),

        SHIFT_F1_F12, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",
            "\x1BOu",       "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy",
            "\x1BOz",       "\x1BO{"),

        CTRL_F1_F12, quote_list(
            "\x1BO\x10",    "\x1BO\x11",    "\x1BO\x12",    "\x1BO\x13",
            "\x1BO\x14",    "\x1BO\x15",    "\x1BO\x16",    "\x1BO\x17",
            "\x1BO\x18",    "\x1BO\x19",    "\x1BO\x1a",    "\x1BO\x1B"),

        ALT_A_Z, quote_list(
            "\x1BNa",       "\x1BNb",       "\x1BNc",       "\x1BNd",       "\x1BNe",
            "\x1BNf",       "\x1BNg",       "\x1BNh",       "\x1BNi",       "\x1BNj",
            "\x1BNk",       "\x1BNl",       "\x1BNm",       "\x1BNn",       "\x1BNo",
            "\x1BNp",       "\x1BNq",       "\x1BNr",       "\x1BNs",       "\x1BNt",
            "\x1BNu",       "\x1BNv",       "\x1BNw",       "\x1BNx",       "\x1BNy",
            "\x1BNz"),

        KEYPAD_0_9, quote_list(
            "\x1B[@",       "x1fm",         "x1fj",         "x1fc",         "x1fh",
            "\x1B[G",       "x1fl",         "x1fg",         "x1fk",         "x1fb"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B?0",       "\x1B?1",       "\x1B?2",       "\x1B?3",       "\x1B?4",
            "\x1B?5",       "\x1B?6",       "\x1B?7",       "\x1B?8",       "\x1B?9"),

        ALT_0_9, quote_list(
            "\x1BN0",       "\x1BN1",       "\x1BN2",       "\x1BN3",       "\x1BN4",
            "\x1BN5",       "\x1BN6",       "\x1BN7",       "\x1BN8",       "\x1BN9"),

        CUT,                "\x1B[S",
        COPY,               "\x1B[T",
        BACK_TAB,           "\x1B[Z"
        );
}

