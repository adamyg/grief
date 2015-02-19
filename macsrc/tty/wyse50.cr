/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: wyse50.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * Wyse 50 terminal setup for CRISP. by David MacKenzie
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "wyse50");

    set_term_characters(
        "\x1BH2",               /* Top left of window. */
        "\x1BH3",               /* Top right of window. */
        "\x1BH1",               /* Bottom left of window. */
        "\x1BH5",               /* Bottom right of window. */
        "\x1BH6",               /* Vertical bar for window sides. */
        "\x1BH:",               /* Top and bottom horizontal bar for window. */
        "\x1BH0",               /* Top join. */
        "\x1BH=",               /* Bottom join. */
        "\x1BH8",               /* Window 4-way intersection. */
        "\x1BH4",               /* Left hand join. */
        "\x1BH9"                /* Right hand join. */
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
            "\x01@\x0D",    "\x01A\x0D",    "\x01B\x0D",    "\x01C\x0D",
            "\x01D\x0D",    "\x01E\x0D",    "\x01F\x0D",    "\x01G\x0D",
            "\x01H\x0D",    "\x01I\x0D",    "\x01J\x0D",    "\x01K\x0D"),

        SHIFT_F1_F12, quote_list(
            "\x01`\x0D",    "\x01a\x0D",    "\x01b\x0D",    "\x01c\x0D",
            "\x01d\x0D",    "\x01e\x0D",    "\x01f\x0D",    "\x01g\x0D",
            "\x01h\x0D",    "\x01i\x0D",    "\x01j\x0D",    "\x01k\x0D"),

        ALT_A_Z, quote_list(
            "\x16a",        "\x16b",        "\x16c",        "\x16d",        "\x16e",    /* alt a-e */
            "\x16f",        "\x16g",        "\x16h",        "\x16i",        "\x16j",    /* alt f-j */
            "\x16k",        "\x16l",        "\x16m",        "\x16n",        "\x16o",    /* alt k-o */
            "\x16p",        "\x16q",        "\x16r",        "\x16s",        "\x16t",    /* alt p-t */
            "\x16u",        "\x16v",        "\x16w",        "\x16x",        "\x16y",    /* alt u-y */
            "\x16z"),                                                                   /* alt-z */

        KEY_INS,            "\x1BQ",                /* Keypad 0 Ins (Wyse INS Char) */
        KEY_END,            "\x1B{",                /* Keypad 1 End (Wyse shift-Home) */
        KEY_PAGEDOWN,       "\x1BK",                /* Keypad 3 PgDn (Wyse PAGE Next) */
        KEY_HOME,           "\x1E",                 /* Keypad 7 Home (Wyse Home) */
        KEY_PAGEUP,         "\x1BJ",                /* Keypad 9 PgUp (Wyse PAGE Prev) */
        BACK_TAB,           "\x1BI",
        CTRL_KEYPAD_9,      "\x1BT",                /* PgUp (Wyse CLR Line) */
        CTRL_KEYPAD_3,      "\x1BY",                /* PgDn (Wyse CLR Scrn) */
        CTRL_KEYPAD_1,      "\x1BP",                /* End (Wyse Print) */
        CTRL_KEYPAD_7,      "\x1B7"                 /* Home (Wyse Send) */
        );

    /*
     *  Assign actions to Wyse's stupid choice of cursor keys
     */
    assign_to_key("^H",     "left");
    assign_to_key("^L",     "right");
    assign_to_key("^K",     "up");
    assign_to_key("^J",     "down");

    assign_to_key("#127",   "backspace");
    assign_to_key("\x1Br",  "insert_mode");         /* overtype Repl key */
    assign_to_key("\x1Bq",  "insert_mode 1");       /* insert   Ins  key */
    assign_to_key("\x1BR",  "delete_line");         /* DEL Line key */
    assign_to_key("\x1BW",  "delete_char");         /* DEL Char key */
    assign_to_key("\x1Bf",  "objects word_right");  /* like Emacs */
    assign_to_key("\x1Bb",  "objects word_left");   /* like Emacs */
}

