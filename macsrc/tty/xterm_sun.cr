/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_sun.cr,v 1.13 2024/07/12 16:33:22 cvsuser Exp $
 * terminal description file for the xterm window under X11 which is a VT-100 like emulator
 *
 *
 */

#include "tty.h"
#include "tty_xterm.h"


void
main()
{
    /*
     *  Load support functions
     */
    set_term_feature( TF_NAME, "xterm-sun" );
    if (inq_macro("xterm_util") <= 0)
        load_macro("tty/xterm_util", FALSE);

    /*
     *  Set characters used for extended graphics support when drawing windows
     */
    xterm_graphic();

    /*
     *  Define escape sequences used for special optimisations on output
     */
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
        FALSE,              /* Boolean,  terminal supports color. */
        "\x1b[%dC",         /* Sequence, move cursor on same line. */
        TRUE,               /* Boolean,  ESC[K gives us a black erased line. */
        FALSE,              /* Boolean,  allow scrolling (ins/del). */
        "\x1b(0",           /* Sequence, enter graphics mode. */
        "\x1b(B"            /* Sequence, exit graphics mode. */
        );

    /*
     *  Define keyboard layout for non-ascii characters
     */
    set_term_keyboard(
        F1_F12, quote_list(
            "\x1b[192z",    "\x1b[193z",    "\x1b[194z",    "\x1b[195z",
            "\x1b[196z",    "\x1b[197z",    "\x1b[198z",    "\x1b[199z",
            "\x1b[200z",    "\x1b[201z",    "\x1b[234z",    "\x1b[235z"),

        F1_F12, quote_list(
            "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",
            "\x1b[15~",     "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
            "\x1b[20~",     "\x1b[21~"),

        ALT_A_Z, quote_list(
            "\x1ba",        "\x1bb",        "\x1bc",        "\x1bd",       "\x1be",
            "\x1bf",        "\x1bg",        "\x1bh",        "\x1bi",       "\x1bj",
            "\x1bk",        "\x1bl",        "\x1bm",        "\x1bn",       "\x1bo",
            "\x1bp",        "\x1bq",        "\x1br",        "\x1bs",       "\x1bt",
            "\x1bu",        "\x1bv",        "\x1bw",        "\x1bx",       "\x1by",
            "\x1bz"),

        KEYPAD_0_9, quote_list(
            "\x1b[2~",      "\x1bO$",       "\x1bOB",       "\x1b[6~",
            "\x1bOD",       "\x1bOu",       "\x1bOC",       "\x1bO\x00",
            "\x1bOA",       "\x1b[5~",      NULL,           "\x1bOk",
            "\x1bOm",       "\x1bOj"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           NULL,           NULL,           "\x1b[232z",
            NULL,           "\x1b[231z"),

        ALT_0_9, quote_list(
            "\x1b1",        "\x1b2",        "\x1b3",        "\x1b4",       "\x1b5",
            "\x1b6",        "\x1b7",        "\x1b8",        "\x1b9",       "\x1b0"),

        SHIFT_KEYPAD_2,     "\x1bOr",
        SHIFT_KEYPAD_4,     "\x1bOt",
        SHIFT_KEYPAD_6,     "\x1bOv",
        SHIFT_KEYPAD_8,     "\x1bOx",

        ALT_0_9, quote_list(
            "\x1bA0",       "\x1bA1",       "\x1bA2",       "\x1bA3",
            "\x1bA4",       "\x1bA5",       "\x1bA6",       "\x1bA7",
            "\x1bA8",       "\x1bA9")
        );
}

/*end*/

