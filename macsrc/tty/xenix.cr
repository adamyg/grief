/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xenix.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * Terminal description file for Xenix 386 & 286 Console Colour display
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "xenix");

    set_term_characters(
        213,                    /* Top left of window. */
        184,                    /* Top right of window. */
        212,                    /* Bottom left of window. */
        190,                    /* Bottom right of window. */
        179,                    /* Vertical bar for window sides. */
        205,                    /* Top and bottom horizontal bar for window. */
        0xd1,                   /* Top join. */
        0xcf,                   /* Bottom join. */
        0xd8,                   /* Window 4-way intersection. */
        0xb5,                   /* Left hand join. */
        0xc6                    /* Right hand join. */
        );

    set_term_features(
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
        "\x1B[%dg",             /* Sequence. print characters with top bitset */
        "\x1B[=12;13C",         /* Sequence, insert-mode cursor. */
        "\x1B[=1;13C",          /* Sequence, overwrite-mode cursor. */
        "\x1B[=10;13C",         /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[1;8C",            /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B[27g",             /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC"              /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139),

        SHIFT_F1_F12, quote_list(
            140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151),

        CTRL_F1_F12, quote_list(
            152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163),

        ALT_A_Z, quote_list(
            176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187,
            188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
            200, 201),

        KEYPAD_0_9, quote_list(
            202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215),

        CTRL_KEYPAD_0_9, quote_list(
            216, 217, 218, 219, 220, 221, 222, 223, 224, 225),

        ALT_0_9, quote_list(
            231, 232, 233, 234, 235, 236, 237, 238, 239, 240),

        BACK_TAB, 230
        );
}


/*
 *  -mono suffix
 */
void
mono()
{
    set_term_feature(TF_COLOR, FALSE);
}
