/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_mintty.cr,v 1.4 2014/10/22 02:34:43 ayoung Exp $
 * Mintty terminal profile.
 *
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
    set_term_feature(TF_NAME, "xterm-mintty");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Basic/common configuration
     */
    xterm_standard();
    xterm_graphic();
    xterm_colour256();                          /* full 256 color is available */

    /*UTF8*/

    /*
     *  Terminal map
     */
    set_term_keyboard(
        F1_F12, quote_list(                     /* xterm */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

        SHIFT_F1_F12, quote_list(               /* xterm */
            "\x1b[1;2P",    "\x1b[1;2Q",    "\x1b[1;2R",    "\x1b[1;2S",    "\x1b[15;2~",
            "\x1b[17;2~",   "\x1b[18;2~",   "\x1b[19;2~",   "\x1b[20;2~",   "\x1b[21;2~",
            "\x1b[23;2~",   "\x1b[24;2~"),

        CTRL_F1_F12, quote_list(                /* xterm */
            "\x1b[1;5P",    "\x1b[1;5Q",    "\x1b[1;5R",    "\x1b[1;5S",    "\x1b[15;5~",
            "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",   "\x1b[20;5~",   "\x1b[21;5~",
            "\x1b[23;5~",   "\x1b[24;5~"),

        CTRLSHIFT_F1_F12, quote_list(           /* mintty */
            "\x1b[1;6P",    "\x1b[1;6Q",    "\x1b[1;6R",    "\x1b[1;6S",    "\x1b[15;6~",
            "\x1b[17;6~",   "\x1b[18;6~",   "\x1b[19;6~",   "\x1b[20;6~",   "\x1b[21;6~",
            "\x1b[23;6~",   "\x1b[24;6~"),

        //  <Alt-A>,        <Alt-B>,        <Alt-C>,        <Alt-D>,        <Alt-E>,
        //  <Alt-F>,        <Alt-G>,        <Alt-H>,        <Alt-I>,        <Alt-J>,
        //  <Alt-K>,        <Alt-L>,        <Alt-M>,        <Alt-N>,        <Alt-O>,
        //  <Alt-P>,        <Alt-Q>,        <Alt-R>,        <Alt-S>,        <Alt-T>,
        //  <Alt-U>,        <Alt-V>,        <Alt-W>,        <Alt-X>,        <Alt-Y>,
        //  <Alt-Z>
        //
        ALT_A_Z, quote_list(                    /* 8bit (lower case) Meta */
            "\x1ba",        "\x1bb",        "\x1bc",        "\x1bd",        "\x1be",
            "\x1bf",        "\x1bg",        "\x1bh",        "\x1bi",        "\x1bj",
            "\x1bk",        "\x1bl",        "\x1bm",        "\x1bn",        "\x1bo",
            "\x1bp",        "\x1bq",        "\x1br",        "\x1bs",        "\x1bt",
            "\x1bu",        "\x1bv",        "\x1bw",        "\x1bx",        "\x1by",
            "\x1bz" ),

        ALT_A_Z, quote_list(                    /* 8bit (upper case) Meta */
            "\x1bA",        "\x1bB",        "\x1bC",        "\x1bD",        "\x1bE",
            "\x1bF",        "\x1bG",        "\x1bH",        "\x1bI",        "\x1bJ",
            "\x1bK",        "\x1bL",        "\x1bM",        "\x1bN",        "\x1b0",
            "\x1bP",        "\x1bQ",        "\x1bR",        "\x1bS",        "\x1bT",
            "\x1bU",        "\x1bV",        "\x1bW",        "\x1bX",        "\x1bY",
            "\x1bZ"),

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del/.           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* Standard (Application mode) */
            "\x1b[2~",      "\x1bOF",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~",
            "\x1b[3~",      "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM"),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        SHIFT_KEYPAD_0_9, quote_list(           /* mintty */
            NULL,           "\x1b[1;2F",    "\x1b[1;2B",    "\x1b[6;2~",    "\x1b[1;2D",
            NULL,           "\x1b[1;2C",    "\x1b[1;2H",    "\x1b[1;2A",    "\x1b[5;2~",
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* mintty */
            NULL,           "\x1b[1;5F",    "\x1b[1;5B",    "\x1b[6;5~",    "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1b[1;5H",    "\x1b[1;5A",    "\x1b[5;5~",
            "\x1b[3;5~",    NULL,           NULL,           "\x1b[1;5j",    NULL,
            NULL,           "\x1b[1;5M",    "\x1c",         NULL,           NULL,
            NULL),

        //  Miscellous keys
        //
        BACK_TAB,           "\x1b[Z",
        BACKSPACE,          "\x7f"
    );
}


void
mintty(void)
{
    /*NOTHING*/
}

