/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tty.cr,v 1.7 2014/10/22 02:34:41 ayoung Exp $
 * Default terminal description file, used when a tty we cannot found.
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "tty");

    set_term_characters(
        '+',                    /* Top left of window. */
        '+',                    /* Top right of window. */
        '+',                    /* Bottom left of window. */
        '+',                    /* Bottom right of window. */
        '|',                    /* Vertical bar for window sides. */
        '-',                    /* Top and bottom horizontal bar for window. */
        '+',                    /* Top join. */
        '+',                    /* Bottom join. */
        '+',                    /* Window 4-way intersection. */
        '+',                    /* Left hand join. */
        '+'                     /* Right hand join. */
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
        FALSE                   /* Boolean,  terminal supports color. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1BOP",       "\x1BOQ",       "\x1BOR",       "\x1BOS",       "\x1BOT",
            "\x1BOU",       "\x1BOV",       "\x1BOW",       "\x1BOX",       "\x1BOY",
            "\x1BOZ",       "\x1BO["),

        SHIFT_F1_F12, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",
            "\x1BOu",       "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy",
            "\x1BOz",       "\x1BO{"),

        CTRL_F1_F12, quote_list(
            "\x1BO\x10",    "\x1BO\x11",    "\x1BO\x12",    "\x1BO\x13",
            "\x1BO\x14",    "\x1BO\x15",    "\x1BO\x16",    "\x1BO\x17",
            "\x1BO\x18",    "\x1BO\x19",    "\x1BO\x1a",    "\x1BO\x1B"),

        ALT_A_Z, quote_list(
            "\x01",         "\x02",         "\x03",         "\x04",         "\x05", /* ALT-A..E */
            "\x06",         "\x07",         NULL,           "\x09",         "\x0a", /* ALT-F..J */
            "\x0b",         "\x0c",         NULL,           "\x0e",         "\x0f", /* ALT-K..O */
            "\x10",         "\x11",         "\x12",         "\x13",         "\x14", /* ALT-P..T */
            "\x15",         NULL,           "\x17",         "\x18",         "\x19", /* ALT-U..Y */
            "\x1a"),

        KEYPAD_0_9, quote_list(
            "\x1B[@",       "\x1B[Y",       "\x1B[B",       "\x1B[U",       "\x1B[D",
            "\x1B[G",       "\x1B[C",       "\x1B[H",       "\x1B[A",       "\x1B[V"),

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

    /*
     *  Following are useful ways to access the BRIEF default CTRL-keys.
     *  We can't do these direct because we substituted the ALT-keys above).
     */
    assign_to_key("^V^B",   "set_bottom_of_window");
    assign_to_key("^V^C",   "set_center_of_window");
    assign_to_key("^V^D",   "page_down");
    assign_to_key("^V^G",   "objects routines");
    assign_to_key("^V^H",   "help");
    assign_to_key("^V^J",   "goto_bookmark");
    assign_to_key("^V^K",   "objects delete_word_left");
    assign_to_key("^V^L",   "objects delete_word_right");
    assign_to_key("^V^N",   "next_error");
    assign_to_key("^V^P",   "next_error 1");
    assign_to_key("^V^R",   "repeat");
    assign_to_key("^V^T",   "set_top_of_window");
    assign_to_key("^V^U",   "page_up");
    assign_to_key("^V^V",   "version");
    assign_to_key("^V^W",   "set_backup");
}
