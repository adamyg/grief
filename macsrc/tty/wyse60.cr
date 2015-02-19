/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: wyse60.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * Wyse 60 terminal setup for CRISP. by David MacKenzie.
 *
 * In order to use this entry, you need to modify the keypad keys to use the
 * definitions as specified in this .cr file
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "wyse60");

    set_term_characters(
        213,                    /* Top left of window */
        184,                    /* Top right of window */
        212,                    /* Bottom left of window */
        190,                    /* Bottom right of window */
        179,                    /* Vertical bar for window sides */
        205,                    /* Top and bottom horizontal bar for window */
        "\x1BH0",               /* Top join */
        "\x1BH=",               /* Bottom join */
        "\x1BH8",               /* Window 4-way intersection */
        "\x1BH4",               /* Left hand join */
        "\x1BH9"                /* Right hand join */
        );

    set_term_features(
        NULL,                   /* Sequence, clear 'n' spaces. */
        "%c",                   /* Sequence. print characters with top bitset */
        "\x1b`3",               /* Sequence, insert-mode cursor. */
        "\x1b`5",               /* Sequence, overwrite-mode cursor. */
        "\x1b`4",               /* Sequence, insert-mode cursor (on virtual space). */
        "\x1b`2",               /* Sequence, overwrite-mode cursor (on virtual space). */
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
            "\x16\x01",     "\x16\x02",     "\x16\x03",     "\x16\x04",     "\x16\x05",     /* alt a-e */
            "\x16\x06",     "\x16\x07",     "\x16\x08",     "\x16\x09",     "\x16\x0a",     /* alt f-j */
            "\x16\x0b",     "\x16\x0c",     "\x16\x0d",     "\x16\x0e",     "\x16\x0f",     /* alt k-o */
            "\x16\x10",     "\x16\x11",     "\x16\x12",     "\x16\x13",     "\x16\x14",     /* alt p-t */
            "\x16\x15",     "\x16\x16",     "\x16\x17",     "\x16\x18",     "\x16\x18",     /* alt u-y */
            "\x16\x1a"),                                                                    /* alt-z */

        KEYPAD_0_9, quote_list(
            "\x0b0",        "\x0b1",        "\n",           "\x0b3",        "\007",
            "\x0b5",        "\x0c",         "\x0b7",        "\x0b",         "\x0b9",
            "\x0b.",        "\x0b+",        "\x0b-",        "\x0b*" ),

        CTRL_KEYPAD_0_9, quote_list(
            "\x0bs0",       "\x0bs1",       "\x0bs2",       "\x0bs3",       "\x0bs4",
            "\x0bs5",       "\x0bs6",       "\x0bs7",       "\x0bs8",       "\x0bs9",
            "\x0bs.",       "\x0bs+",       "\x0bs-",       "\x0bs*"),

        BACK_TAB,           "\x1BI"
        );

    assign_to_key("<Ctrl-U>", "undo");
    assign_to_key("<Ctrl-B>", "buffer_list 1");
    assign_to_key("<Ctrl-D>", "delete_line");
    assign_to_key("<Ctrl-E>", "edit_file");
    assign_to_key("<Ctrl-W>", "write_buffer");
}

