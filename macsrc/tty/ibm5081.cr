/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: ibm5081.cr,v 1.6 2014/10/22 02:34:40 ayoung Exp $
 * Terminal description for IBM 6150 (RT-PC) console with a colour display
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "ibm5081");

    set_term_characters(
        "\x1c\x84",             /* Top left of window */
        "\x1d\xfa",             /* Top right of window */
        "\x1c\x83",             /* Bottom left of window */
        "\x1d\xfc",             /* Bottom right of window */
        179,                    /* Vertical bar for window sides */
        205,                    /* Top and bottom horizontal bar for window */
        NULL,                   /* Top join */
        NULL,                   /* Bottom join */
        NULL,                   /* Window 4-way intersection */
        NULL,                   /* Left hand join */
        NULL                    /* Right hand join */
        );

    set_term_features(          /* Sequence, clear 'n' spaces. */
        "\x1B[%dX",             /* Sequence. print characters with top bitset */
        "\x1B[11m%c\x1B[10m",   /* Sequence, insert-mode cursor. */
        "\x1B[23m",             /* Sequence, overwrite-mode cursor. */
        "\x1B[24m",             /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[28;10;13m",       /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B[28;1;8m",         /* Sequence, print ESCAPE character graphically. */
        "\x1B\x1B",             /* Sequence, repeat last character 'n' times. */
        "\x1B[%db",             /* Boolean,  ESC [0m resets color. */
        FALSE,                  /* Boolean,  terminal supports color. */
        TRUE                    /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1B[001q",    "\x1B[002q",    "\x1B[003q",    "\x1B[004q",
            "\x1B[005q",    "\x1B[006q",    "\x1B[007q",    "\x1B[008q",
            "\x1B[009q",    "\x1B[010q",    "\x1B[011q",    "\x1B[012q"),

        SHIFT_F1_F12, quote_list(
            "\x1B[013q",    "\x1B[014q",    "\x1B[015q",    "\x1B[016q",
            "\x1B[017q",    "\x1B[018q",    "\x1B[019q",    "\x1B[020q",
            "\x1B[021q",    "\x1B[022q",    "\x1B[023q",    "\x1B[024q"),

        CTRL_F1_F12, quote_list(
            "\x1B[025q",    "\x1B[026q",    "\x1B[027q",    "\x1B[028q",
            "\x1B[029q",    "\x1B[030q",    "\x1B[031q",    "\x1B[032q",
            "\x1B[033q",    "\x1B[034q",    "\x1B[035q",    "\x1B[036q"),

        ALT_F1_F12, quote_list(
            "\x1B[037q",    "\x1B[038q",    "\x1B[039q",    "\x1B[040q",
            "\x1B[041q",    "\x1B[042q",    "\x1B[043q",    "\x1B[044q",
            "\x1B[045q",    "\x1B[046q",    "\x1B[047q",    "\x1B[048q"),

        ALT_A_Z, quote_list(
            "\x1B[087q",    "\x1B[105q",    "\x1B[103q",    "\x1B[089q",
            "\x1B[076q",    "\x1B[090q",    "\x1B[091q",    "\x1B[092q",
            "\x1B[081q",    "\x1B[093q",    "\x1B[094q",    "\x1B[095q",
            "\x1B[107q",    "\x1B[106q",    "\x1B[082q",    "\x1B[083q",
            "\x1B[074q",    "\x1B[077q",    "\x1B[088q",    "\x1B[078q",
            "\x1B[080q",    "\x1B[104q",    "\x1B[075q",    "\x1B[102q",
            "\x1B[079q",    "\x1B[101q"),

        KEYPAD_0_9, quote_list(
            "\x1B[139q",    "\x1B[146q",    "\x1B[B",       "\x1B[154q",
            "\x1B[D",       NULL,           "\x1B[C",       "\x1B[H",
            "\x1B[A",       "\x1B[150q",    "\x1B[P",       "\x1B[150q"),

        CTRL_KEYPAD_0_9, quote_list(
            "\\x1B?0",      "\\x1B?1",      "\\x1B?2",      "\\x1B?3",      "\\x1B?4",
            "\\x1B?5",      "\\x1B?6",      "\\x1B?7",      "\\x1B?8",      "\\x1B?9")
            );
}

