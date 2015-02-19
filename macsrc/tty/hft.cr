/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: hft.cr,v 1.5 2014/10/22 02:34:40 ayoung Exp $
 * Terminal description for IBM High Function Terminal Colour display
 *
 *
 */

#include "tty.h"


void
main()
{
    set_term_feature(TF_NAME, "hft");

    set_term_characters(
        218,                    /* Top left of window */
        191,                    /* Top right of window */
        192,                    /* Bottom left of window */
        217,                    /* Bottom right of window */
        179,                    /* Vertical bar for window sides */
        196,                    /* Top and bottom horizontal bar for window */
        194,                    /* Top join */
        193,                    /* Bottom join */
        197,                    /* Window 4-way intersection */
        195,                    /* Left hand join */
        180                     /* Right hand join */
        );

    set_term_features(
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
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
        FALSE,                  /* Boolean,  ESC[K gives us a black erased line. */
        TRUE                    /* Boolean,  allow scrolling (ins/del). */
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
            "\x1B[087q",    "\x1B[105q",    "\x1B[103q",    "\x1B[089q",    "\x1B[076q",
            "\x1B[090q",    "\x1B[091q",    "\x1B[092q",    "\x1B[081q",    "\x1B[093q",
            "\x1B[094q",    "\x1B[095q",    "\x1B[107q",    "\x1B[106q",    "\x1B[082q",
            "\x1B[083q",    "\x1B[074q",    "\x1B[077q",    "\x1B[088q",    "\x1B[078q",
            "\x1B[080q",    "\x1B[104q",    "\x1B[075q",    "\x1B[102q",    "\x1B[079q",
            "\x1B[101q"),

        KEYPAD_0_9, quote_list(
            "\x1B[139q",    "\x1B[146q",    "\x1B[B",       "\x1B[154q",
            "\x1B[D",       NULL,           "\x1B[C",       "\x1B[H",
            "\x1B[A",       "\x1B[150q"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B[140q",    "\x1B[148q",    "\x1B[165q",    "\x1B[156q",
            "\x1B[159q",    NULL,           "\x1B[168q",    "\x1B[144q",
            "\x1B[162q",    "\x1B[152q"),

        SHIFT_KEYPAD_0, quote_list(
            "\x1B[139q",    "\x1B[147q",    "\x1B[164q",    "\x1B[155q",
            "\x1B[158q",    NULL,           "\x1B[167q",    "\x1B[143q",
            "\x1B[161q",    "\x1B[151q"),

        DEL,                "\x1B[P"
        );
}

