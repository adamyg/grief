/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: suncmd.cr,v 1.4 2024/09/20 12:15:15 cvsuser Exp $
 * This file can be used for a variety of Sun based applications.
 *
 *  The GRTERM entry should start with 'sun', and then you can append 'type3' if you have an old
 *  type 3 keyboard or various other things. Refer to config guide for more info On a sun-3 keyboard,
 *  you should type: 'setkeys nosunview' so that we can trap the L* keys. (We cannot get at L1 because
 *  that seems to be hardwired.
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "suncmd");

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
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE                    /* Boolean,  ESC[K gives us a black erased line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[234z",    "\x1B[234z",    "\x1B[203z",    "\x1B[235z",
            "\x1B[204z",    "\x1B[236z",    "\x1B[205z",    "\x1B[237z",
            "\x1B[206z",    "\x1B[238z"),

        SHIFT_F1_F12, quote_list(
            "\x1B[202z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",
            "\x1B[228z",    "\x1B[229z",    "\x1B[230z",    NULL,
            NULL),

        ALT_A_Z, quote_list(
            "\xE1",         "\xE2",         "\xE3",         "\xE4",         "\xE5",     /* ALT-A..E */
            "\xE6",         "\xE7",         "\xE8",         "\xE9",         "\xEa",     /* ALT-F..J */
            "\xEb",         "\xEc",         "\xED",         "\xEe",         "\xEf",     /* ALT-K..O */
            "\xF0",         "\xF1",         "\xF2",         "\xF3",         "\xF4",     /* ALT-P..T */
            "\xF5",         "\xF6",         "\xF7",         "\xF8",         "\xF9",     /* ALT-U..Y */
            "\xFa"),

        KEYPAD_0_9, quote_list(
            "\x1B[212z",    "\x1B[220z",    "\x1B[B",       "\x1B[222z",
            "\x1B[D",       "\x1B[218z",    "\x1B[C",       "\x1B[214z",
            "\x1B[A",       "\x1B[216z",    "\x1B[P",       "\x1B[213z",
            "\x1B[210z"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           NULL,           NULL,           "\x1B[232z",
            "\x1B[208z",    NULL,           "\x1B[209z",    NULL,
            NULL,           "\x1B[231z"),

        ALT_0_9, quote_list(
            "\xB0",         "\xB1",         "\xB2",         "\xB3",         "\xB4",     /* ALT-0..4 */
            "\xB5",         "\xB6",         "\xB7",         "\xB8",         "\xB9")     /* ALT-5..9 */
    );
}


/*
 *  -type3 suffix
 */
void
type3()
{
    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[202z",    "\x1B[3~",      "\x1B[4~",      "\x1B[5~",
            "\x1B[17~",     "\x1B[18~",     "\x1B[19~"),

        F1_F12, quote_list(
            NULL,           "\x1B[23~",     "\x1B[203z",    "\x1B[24~",     "\x1B[204z",
            "\x1B[25~",     "\x1B[205z",    "\x1B[26~",     "\x1B[206z",    "\x1B[28~")
        );
}


/*
 *  -type4 suffix
 */
void
type4()
{
    set_term_keyboard(
        KEYPAD_0_9, quote_list(
            "\x1B[247z",    "\x1B[220z",    "\x1B[B",       "\x1B[222z",
            "\x1B[D",       "\x1B[218z",    "\x1B[C",       "\x1B[214z",
            "\x1B[A",       "\x1B[216z",    "\x1B[249z",    "\x1B[253z",
            "\x1B[254z",    "\x1B[213z")
        );

    assign_to_key("\x1B[250z",  "insert \"\n\"");
    assign_to_key("\x1B[211z",  "objects word_left");
    assign_to_key("\x1B[212z",  "objects word_right");
    assign_to_key("<Keypad-5>", "search_next");

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[234z",    "\x1B[225z",    "\x1B[226z",    "\x1B[227z",
            "\x1B[228z",    "\x1B[229z",    "\x1B[230z",    "\x1B[231z",
            "\x1B[232z",    "\x1B[233z"));
}


/*
 *  The following macro is used when running from inside the crttool program, which
 *  supports full color VT220 emulation, yet remaps the keyboard the way a VT220 does
 */
void
crttool()
{
    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[2~",      "\x1B[3~",      "\x1B[4~",      "\x1B[5~",
            "\x1B[6~",      "\x1B[17~",     "\x1B[18~",     "\x1B[19~",
            "\x1B[20~",     "\x1B[21~",     "\x1B[23~",     "\x1B[24~")
        );

    set_term_keyboard(
        KEYPAD_0_9, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",
            "\x1BOt",       "\x1BOu",       "\x1BOv",       "\x1BOw",
            "\x1BOx",       "\x1BOy"),

        KEY_COPY,           "\x1BOl",
        KEY_CUT,            "\x1BOS",
        KEY_UNDO,           "\x1BOR",
        KEY_DEL,            "\x1BOn",

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B?0",       "\x1B?1",       "\x1B?2",       "\x1B?3",       "\x1B?4",
            "\x1B?5",       "\x1B?6",       "\x1B?7",       "\x1B?8",       "\x1B?9"),

        SHIFT_F1_F12, quote_list(
            "\x1B$1",       "\x1B$2",       "\x1B$3",       "\x1B$4",       "\x1B$5",
            "\x1B$6",       "\x1B$7",       "\x1B$8",       "\x1B$9"),

        SHIFT_KEYPAD_2,     "\x1B0B",
        SHIFT_KEYPAD_4,     "\x1B0D",
        SHIFT_KEYPAD_6,     "\x1B0C",
        SHIFT_KEYPAD_8,     "\x1B0A"
        );

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
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE,                   /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,                  /* Boolean,  allow scrolling (ins/del). */
        "\x1B(0",               /* Sequence, enter graphics mode. */
        "\x1B(B"                /* Sequence, exit graphics mode. */
        );

    assign_to_key("#127", "backspace");
}


/*
 *  -mono suffix
 */
void
mono()
{
    set_term_feature(TF_COLOR, FALSE);
}


/*
 *  xmodmap running (in crisp/utils/xmodmaprc)
 */
void
xmodmap()
{
    set_term_keyboard(
        ALT_A_Z, quote_list(
            "\xb0",         "\xb1",         "\xb2",         "\xb3",         "\xf1",     /* ALT-A..E */
            "\xb5",         "\xb6",         "\xb7",         "\xf2",         "\xb9",     /* ALT-F..J */
            "\xba",         "\xbb",         "\xbc",         "\xbd",         "\xbe",     /* ALT-K..O */
            "\xbf",         "\xc0",         "\xc1",         "\xc2",         "\xc3",     /* ALT-P..T */
            "\xc4",         "\xc5",         "\xc6",         "\xc7",         "\xc8",     /* ALT-U..Y */
            "\xc9"),

        ALT_0_9, quote_list(
            "\xe7",         "\xe8",         "\xe9",         "\xea",         "\xeb",
            "\xec",         "\xed",         "\xee",         "\xef",         "\xf0"),

        KEY_UNDO,           "\x1B[213z"
        );
}

