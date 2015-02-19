/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: rde.cr,v 1.4 2011/11/03 23:42:02 cvsuser Exp $
 * rdetool terminal description
 *
 * This is a terminal description for the legacy 'rdetool' utility available for XView.
 *
 * You will need to source and compile a the copy of rdetool. This terminal
 * description sets the key bindings to be compatible with those defined in that file
 *
 */

#include "tty.h"


void
main()
{
    set_term_feature(TF_NAME, "rdetool");

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
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space).
        NULL,                   /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        FALSE,                  /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE                    /* Boolean,  ESC[K gives us a black erased line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[224z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",
            "\x1B[228z",    "\x1B[229z",    "\x1B[230z",    "\x1B[231z",
            "\x1B[232z",    "\x1B[233z",    "\x1B234z",     "\x1B[235z"),

        SHIFT_F1_F12, quote_list(
            "\x1B[224S",    "\x1B[225S",    "\x1B[226S",    "\x1B[227S",
            "\x1B[228S",    "\x1B[229S",    "\x1B[230S",    "\x1B[231S",
            "\x1B[232S",    "\x1B[233S",    "\x1B[234S",    "\x1B[235S"),

        SHIFT_F1_F12, quote_list(
            "\x8c",         "\x8d",         "\x8e",         "\x8f",
            "\x90",         "\x91",         "\x92",         "\x93",
            "\x94",         "\x95",         "\x1b[SF11~",   "\x1b[SF12~"),

        ALT_F1_F12, quote_list(
            "\x1B[224A",    "\x1B[225A",    "\x1B[226A",    "\x1B[227A",
            "\x1B[228A",    "\x1B[229A",    "\x1B[230A",    "\x1B[231A",
            "\x1B[232A",    "\x1B[233A",    "\x1B[234A",    "\x1B[235A"),

        CTRL_F1_F12, quote_list(
            "\x1B[224C",    "\x1B[225C",    "\x1B[226C",    "\x1B[227C",
            "\x1B[228C",    "\x1B[229C",    "\x1B[230C",    "\x1B[231C",
            "\x1B[232C",    "\x1B[233C",    "x1B[234C",     "\x1B[235C"),

        ALT_A_Z, quote_list(
            "\x1BAa",       "\x1BAb",       "\x1BAc",       "\x1BAd",       "\x1BAe",
            "\x1BAf",       "\x1BAg",       "\x1BAh",       "\x1BAi",       "\x1BAj",
            "\x1BAk",       "\x1BAl",       "\x1BAm",       "\x1BAn",       "\x1BAo",
            "\x1BAp",       "\x1BAq",       "\x1BAr",       "\x1BAs",       "\x1BAt",
            "\x1BAu",       "\x1BAv",       "\x1BAw",       "\x1BAx",       "\x1BAy",
            "\x1BAz"),

        ALT_A_Z, quote_list(
            "\x1BAA",       "\x1BAB",       "\x1BAC",       "\x1BAD",       "\x1BAE",
            "\x1BAF",       "\x1BAG",       "\x1BAH",       "\x1BAI",       "\x1BAJ",
            "\x1BAK",       "\x1BAL",       "\x1BAM",       "\x1BAN",       "\x1BAO",
            "\x1BAP",       "\x1BAQ",       "\x1BAR",       "\x1BAS",       "\x1BAT",
            "\x1BAU",       "\x1BAV",       "\x1BAW",       "\x1BAX",       "\x1BAY",
            "\x1BAZ"),

        KEYPAD_0_9, quote_list(
            "\x1B[2z",      "\x1B[220z",    "\x1B[B",       "\x1B[222z",
            "\x1B[D",       "\x1B[218z",    "\x1B[C",       "\x1B[214z",
            "\x1B[A",       "\x1B[216z"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           "\x1B[220C",    "\x1B[221C",    "\x1B[222C",
            "\x1B[217C",    "\x1B[218C",    "\x1B[219C",    "\x1B[214C",
            "\x1B[215C",    "\x1B[216C"),

        SHIFT_KEYPAD_0, quote_list(
            NULL,           "\x1B[220S",    "\x1B[221S",    "\x1B[222S",
            "\x1B[217S",    "\x1B[218S",    "\x1B[219S",    "\x1B[214S",
            "\x1B[215S",    "\x1B[216S"),

        ALT_0_9, quote_list(
            "\x1bA0",       "\x1bA1",       "\x1bA2",       "\x1bA3",       "\x1bA4",
            "\x1bA5",       "\x1bA6",       "\x1bA7",       "\x1bA8",       "\x1bA9"),

        KEY_COPY,           "\x1B[Ok",
        KEY_CUT,            "\x1B[Om",
        KEY_UNDO,           "\x1B[213z",
        KEY_DEL,            "\x1BKD",

        ALT_KEYPAD_END,     "\x1B[220A",
        ALT_KEYPAD_HOME,    "\x1B[214A",
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
        0xF5,               /* Top left of window. */
        0xF6,               /* Top right of window. */
        0xF4,               /* Bottom left of window. */
        0xF7,               /* Bottom right of window. */
        0xF9,               /* Vertical bar for window sides. */
        0xFA,               /* Top and bottom horizontal bar for window. */
        0xFD,               /* Top join. */
        0xFE,               /* Bottom join. */
        0xF8,               /* Window 4-way intersection. */
        0xFB,               /* Left hand join. */
        0xFC                /* Right hand join. */
        );
}

