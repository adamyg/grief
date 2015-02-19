/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_gnome.cr,v 1.7 2014/10/22 02:34:43 ayoung Exp $
 * terminal description file for the xterm window under Gnome-terminal.
 *
 *
 */

#include "tty.h"
#include "tty_xterm.h"

void
main(void)
{
    /*
     *  Load support functions
     */
    set_term_feature(TF_NAME, "xterm-gnome");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Set characters used for extended graphics support when
     *  drawing windows.
     */
    xterm_graphic();
    xterm_colour256();          /* full 256 color is available */

    /*
     *  Define escape sequences used for special optimisations on output.
     */
    set_term_features(
        "\x1b[%dX",             /* Sequence, clear 'n' spaces. */
        NULL,                   /* Sequence. print characters with top bitset */
        NULL,                   /* Sequence, insert-mode cursor. */
        NULL,                   /* Sequence, overwrite-mode cursor. */
        NULL,                   /* Sequence, insert-mode cursor (on virtual space). */
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,                   /* Sequence, print ESCAPE character graphically. */
        "\x1b[%db",             /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        NULL,                   /* Sequence, move cursor on same line. */
        FALSE,                  /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                   /* Boolean,  allow scrolling (ins/del). */
        NULL,                   /* Sequence, enter graphics mode. */
        NULL,                   /* Sequence, exit graphics mode. */
        NULL,                   /* Sequence, Init */
        NULL                    /* Sequence, Reset */
        );

    /*
     *  Define keyboard layout for non-ascii characters.
     *  These can only be used for the console.
     */
    set_term_keyboard(
        F1_F12, quote_list(             // kf01 .. kf12
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

        SHIFT_F1_F12, quote_list(       // kf13 .. kf24
            "\x1bO1;2P",    "\x1bO1;2Q",    "\x1bO1;2R",    "\x1bO1;2S",    "\x1b[15;2~",
            "\x1b[17;2~",   "\x1b[18;2~",   "\x1b[19;2~",   "\x1b[20;2~",   "\x1b[21;2~",
            "\x1b[23;2~",   "\x1b[24;2~"),

        CTRL_F1_F12, quote_list(        // kf25 .. kf36
            "\x1bO5P",      "\x1bO1;5Q",    "\x1bO1;5R",    "\x1bO1;5S",    "\x1b[15;5~",
            "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",   "\x1b[20;5~",   "\x1b[21;5~",
            "\x1b[23;5~",   "\x1b[24;5~"),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        KEYPAD_0_9, quote_list(
            "\x1b[2~",      "\x1bOF",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOu",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~",
            "\x08",         "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM",       NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        SHIFT_KEYPAD_0_9, quote_list(
            NULL,           NULL,           "\x1b[1;2B",    NULL,           "\x1b[1;2D",
            NULL,           "\x1b[1;2C",    NULL,           "\x1b[1;2A",    NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        CTRL_KEYPAD_0_9, quote_list(
            NULL,           "\x1bOF",       "\x1b[1;5B",    "\x1b[6;5~",    "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1bOH",       "\x1b[1;5A",    "\x1b[5;5~",
            "\x08",         NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        ALT_KEYPAD_0_9, quote_list(
            "\x1b[2;3~",    NULL,           "\x1b[1;3B",    "\x1b[6;3~",    "\x1b[1;3D",
            NULL,           "\x1b[1;3C",    "\x1bOH",       "\x1b[1;3A",    "\x1b[5;3~",
            "\x08",         "\x1bOk",       "\x1bOm",       NULL,           NULL,
            NULL,           "\x1b\x0a",     NULL,           NULL,           NULL,
            NULL),

        SHIFT_KEYPAD_2,     "\x1bOr",
        SHIFT_KEYPAD_4,     "\x1bOt",
        SHIFT_KEYPAD_6,     "\x1bOv",
        SHIFT_KEYPAD_8,     "\x1bOx",

        KEY_CUT,            "\x18",
        KEY_COPY,           "\x03",
        KEY_HOME,           "\x1b[1~",
        KEY_END,            "\x1b[4~",
        KEY_INS,            "\x1b[2~",
        KEY_DEL,            "\x1b[3~"
        );

    xterm_altmeta_keys();
}


void
gnome(void)
{
    /*gnome*/
}


























