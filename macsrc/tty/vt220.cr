/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: vt220.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * Terminal description file for vt220
 *
 *
 */

#include "tty.h"


void
main()
{
    set_term_feature(TF_NAME, "vt220");

    set_term_characters(
        "l",                /* Top left of window. */
        "k",                /* Top right of window. */
        "m",                /* Bottom left of window. */
        "j",                /* Bottom right of window. */
        "x",                /* Vertical bar for window sides. */
        "q",                /* Top and bottom horizontal bar for window. */
        "w",                /* Top join. */
        "v",                /* Bottom join. */
        "n",                /* Window 4-way intersection. */
        "u",                /* Left hand join. */
        "t"                 /* Right hand join. */
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
        TRUE,               /* Boolean,  terminal supports color. */
        "\x1B[%dC",         /* Sequence, move cursor on same line. */
        TRUE,               /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,              /* Boolean,  allow scrolling (ins/del). */
        "\x1B(0",           /* Sequence, enter graphics mode. */
        "\x1B(B",           /* Sequence, exit graphics mode. */
        "\x1B[0m",          /* Sequence, Init */
        "\x1B[0m"           /* Sequence, Reset */
        );

    set_term_keyboard(
        KEY_INS,            "[2~",
        KEY_DEL,            "\x1b[3~",
        KEY_HOME,           "\x1b[1~",
        KEY_END,            "\x1b[4~",
        KEY_PAGEUP,         "\x1b[5~",
        KEY_PAGEDOWN,       "\x1b[6~",
        KEY_UP,             "\x1b[A",
        KEY_DOWN,           "\x1b[B",
        KEY_LEFT,           "\x1b[D",
        KEY_RIGHT,          "\x1b[C",
        KEYPAD_ENTER,       "\x1b[M",

        //
        //  Warning: the following strings contain embedded ESC characters
        //
        F1_F12, quote_list(
            "OP",      "OQ",      "OR",      "OS",      "OT",
            "[17~",    "[18~",    "[19~",    "[20~",    "[21~",
            "[35~",    "[36~"),

        SHIFT_F1_F12, quote_list(
            "[23~",    "[24~",    "[25~",    "[26~",    "[28~",
            "[29~",    "[31~",    "[32~",    "[33~",    "[34~",
            "[37~",    "[38~"),

        CTRL_F1_F12, quote_list(
            "Oa",      "Ob",      "Oc",      "Od",      "Oe",
            "Of",      "Og",      "Oh",      "Oi",      "Oj",
            "[39~",    "[40~"),

        ALT_A_Z, quote_list(
            "O!",      "O@",      "O#",      "O$",      "O%",
            "O^",      "O&",      "O*",      "O(",      "O)",
            "O_",      "O+",      "O[",      "O{",      "O]",
            "O}",      "O\\",     "O\"",     "O|",      "O;",
            "O:",      "O'",      "O,",      "O<",      "O.",
            "O>"),

        KEYPAD_0_9, quote_list(
            "Op",      "Oq",      "Or",      "Os",      "Ot",
            "Ou",      "Ov",      "Ow",      "Ox",      "Oy"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,       "\x1bPB",   NULL,       "\x1bPC",   "\x1bO?",
            NULL,       "\x1bPA",   "\x1bPD",   NULL,       "\x1bPE"),

        ALT_0_9, quote_list(
            "O0",      "O1",      "O2",      "[3",      "[4",
            "[5",      "[6",      "[7",      "[8",      "[9"),

        CUT,            "Om",
        COPY,           "Ol",
        DEL,            "On"
        );
}

