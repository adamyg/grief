/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_linux.cr,v 1.27 2024/11/18 13:42:10 cvsuser Exp $
 * terminal description file for the xterm window under X11/Linux.
 *
 *
 */

/*  Source:     console_codes(4)

    o Linux Console Private CSI Sequences

        The following sequences are neither ECMA-48 nor native VT102. They
        are native to the Linux console driver. Colors are in SGR parameters:

        0 = black, 1 = red, 2 = green, 3 = brown, 4 = blue, 5 = magenta, 6 = cyan, 7 = white.

        ESC [ 1 ; n ]               Set color n as the underline color
        ESC [ 2 ; n ]               Set color n as the dim color
        ESC [ 8 ]                   Make the current color pair the default attributes.
        ESC [ 9 ; n ]               Set screen blank timeout to n minutes.
        ESC [ 10 ; n ]              Set bell frequency in Hz.
        ESC [ 11 ; n ]              Set bell duration in msec.
        ESC [ 12 ; n ]              Bring specified console to the front.
        ESC [ 13 ]                  Unblank the screen.
        ESC [ 14 ; n ]              Set the VESA powerdown interval in minutes.

    o Additional

        It accepts ESC ] (OSC) for the setting of certain resources. In
        addition to the ECMA-48 string terminator (ST), xterm accepts a BEL to
        terminate an OSC string. These are a few of the OSC control sequences
        recognized by xterm: ESC ] 0 ; txt ST Set icon name and window title to
        txt.

        ESC ] 1 ; txt ST            Set icon name to txt.
        ESC ] 2 ; txt ST            Set window title to txt.
        ESC ] 4 ; num; txt ST       Set ANSI color num to txt.
        ESC ] 10 ; txt ST           Set dynamic text color to txt.
        ESC ] 4 6 ; name ST         Change log file to name (normally disabled by a compile-time option)
        ESC ] 5 0 ; fn ST           Set font to fn.

**/

#include "tty.h"
#include "tty_xterm.h"

void
main()
{
    /*
     *  Load support functions
     */
    set_term_feature(TF_NAME, "xterm-linux");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Set characters used for extended graphics support when
     *  drawing windows.
     */
    xterm_graphic();

    /*
     *  Define escape sequences used for special optimisations on output.
     */
    xterm_color();                  /* colour by default */
    linux_standard();
//  xterm_pccolors();

    /*
     *  Define keyboard layout for non-ascii characters.
     */
    set_term_keyboard(
        //
        //  Function keys
        //
        F1_F12, quote_list(
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",
            "\x1b[15~",     "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
            "\x1b[20~",     "\x1b[21~",     "\x1b[23~",     "\x1b[24~" ),

        SHIFT_F1_F12, quote_list(   /*XFree*/
            NULL,           NULL,            "\x1b[25~",    "\x1b[26~",
            "\x1b[28~",     "\x1b[29~",      "\x1b[31~",    "\x1b[32~",
            "\x1b[33~",     "\x1b[34~",      "\x1b[23;2~",  "\x1b[24;2~" ),

        SHIFT_F1_F12, quote_list(   /*X.Org*/
            "\x1bO2P",      "\x1bO2Q",       "\x1bO2R",     "\x1bO2S",
            "\x1b[15;2~",   "\x1b[17;2~",    "\x1b[18;2~",  "\x1b[19;2~",
            "\x1b[20;2~",   "\x1b[21;2~",    NULL,           NULL ),

        CTRL_F1_F12, quote_list(
            "\x1bO5P",      "\x1bO5Q",      "\x1bO5R",      "\x1bO5S",
            "\x1b[15;5~",   "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",
            "\x1b[20;5~",   "\x1b[21;5~",   "\x1b[23;5~",   "\x1b[24;5~" ),

        //  <Alt-A>,        <Alt-B>,        <Alt-C>,        <Alt-D>,        <Alt-E>,
        //  <Alt-F>,        <Alt-G>,        <Alt-H>,        <Alt-I>,        <Alt-J>,
        //  <Alt-K>,        <Alt-L>,        <Alt-M>,        <Alt-N>,        <Alt-O>,
        //  <Alt-P>,        <Alt-Q>,        <Alt-R>,        <Alt-S>,        <Alt-T>,
        //  <Alt-U>,        <Alt-V>,        <Alt-W>,        <Alt-X>,        <Alt-Y>,
        //  <Alt-Z>
        //
        ALT_A_Z, quote_list(        /*X.Org (7bit) (lower case)*/
            "\xC3\xA1",     "\xC3\xA2",     "\xC3\xA3",     "\xC3\xA4",     "\xC3\xA5",
            "\xC3\xA6",     "\xC3\xA7",     "\xC3\xA8",     "\xC3\xA9",     "\xC3\xAA",
            "\xC3\xAB",     "\xC3\xAC",     "\xC3\xAD",     "\xC3\xAE",     "\xC3\xAF",
            "\xC3\xB0",     "\xC3\xB1",     "\xC3\xB2",     "\xC3\xB3",     "\xC3\xB4",
            "\xC3\xB5",     "\xC3\xB6",     "\xC3\xB7",     "\xC3\xB8",     "\xC3\xB9",
            "\xC3\xBA" ),

        ALT_A_Z, quote_list(        /*X.Org (7bit) (upper case)*/
            "\xC3\x81",     "\xC3\x82",     "\xC3\x83",     "\xC3\x84",     "\xC3\x85",
            "\xC3\x86",     "\xC3\x87",     "\xC3\x88",     "\xC3\x89",     "\xC3\x8A",
            "\xC3\x8B",     "\xC3\x8C",     "\xC3\x8D",     "\xC3\x8E",     "\xC3\x8F",
            "\xC3\x90",     "\xC3\x91",     "\xC3\x92",     "\xC3\x93",     "\xC3\x94",
            "\xC3\x95",     "\xC3\x96",     "\xC3\x97",     "\xC3\x98",     "\xC3\x99",
            "\xC3\x9A" ),

        ALT_0_9, quote_list(        /*X.Org (7bit)*/
            "\xC2\xB0",     "\xC2\xB1",     "\xC2\xB2",      "\xC2\xB3",    "\xC2\xB4",
            "\xC2\xB5",     "\xC2\xB6",     "\xC2\xB7",      "\xC2\xB8",    "\xC2\xB9" ),

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //
        KEYPAD_0_9, quote_list(
            "\x1b[2~",      "\x1bOF",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~"),

        KEYPAD_DIV,         "\x1bOo",       /* Keypad-/         */
        KEYPAD_STAR,        "\x1bOj",       /* Keypad-*         */
        KEYPAD_MINUS,       "\x1bOm",       /* Keypad--         */
        KEYPAD_PLUS,        "\x1bOk",       /* Keypad-+         */
        KEYPAD_ENTER,       "\x1bOM",       /* Keypad-enter     */

        SHIFT_KEYPAD_2,     "\x1bO2B",      /* down             */
        SHIFT_KEYPAD_4,     "\x1bO2D",      /* left             */
        SHIFT_KEYPAD_6,     "\x1bO2C",      /* right            */
        SHIFT_KEYPAD_8,     "\x1bO2A",      /* up               */

        CTRL_KEYPAD_2,      "\x1bO5B",      /* down             */
        CTRL_KEYPAD_4,      "\x1bO5D",      /* left             */
        CTRL_KEYPAD_6,      "\x1bO5C",      /* right            */
        CTRL_KEYPAD_8,      "\x1bO5A",      /* up               */

        SHIFT_KEYPAD_2,     "\x1b[1;2B",    /* down             */
        SHIFT_KEYPAD_4,     "\x1b[1;2D",    /* left             */
        SHIFT_KEYPAD_6,     "\x1b[1;2C",    /* right            */
        SHIFT_KEYPAD_8,     "\x1b[1;2A",    /* up               */

        ALT_KEYPAD_2,       "\x1b[1;3B",    /* down             */
        ALT_KEYPAD_4,       "\x1b[1;3D",    /* left             */
        ALT_KEYPAD_6,       "\x1b[1;3C",    /* right            */
        ALT_KEYPAD_8,       "\x1b[1;3A",    /* up               */

        CTRL_KEYPAD_0,      "\x1b[2;5~",    /* ins              */
        CTRL_KEYPAD_2,      "\x1b[1;5B",    /* down             */
        CTRL_KEYPAD_3,      "\x1b[6;5~",    /* pagedown         */
        CTRL_KEYPAD_4,      "\x1b[1;5D",    /* left             */
        CTRL_KEYPAD_6,      "\x1b[1;5C",    /* right            */
        CTRL_KEYPAD_8,      "\x1b[1;5A",    /* up               */
        CTRL_KEYPAD_9,      "\x1b[5;5~",    /* pagedown         */
        CTRL_KEYPAD_DEL,    "\x1b[3;5~",    /* delete           */

//      MOUSE_KEY,          "\x1b[M",       /* Mouse event      */

        BACK_TAB,           "\x1b[Z",       /* Shift-Tab        */
//      CTRL_BACKSPACE,     "\x08",         /* Ctrl-Backspace   */
        ALT_BACKSPACE,      "\x1b\x7f",     /* Alt-Backspace    */
        ALT_BACKSPACE,      "\xC2\x88",     /* Alt-Backspace    */

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
linux_standard()
{
    set_term_feature(TF_PRINT_SPACE,    "\x1b[%dX");    /* Sequence to clear 'n' spaces. */
    set_term_feature(TF_CURSOR_RIGHT,   "\x1b[%dC");    /* Sequence to move cursor on same line. */
    set_term_feature(TF_CLEAR_IS_BLACK, TRUE);          /* ESC[K gives us a black erased line. */
    set_term_feature(TF_GRAPHIC_MODE,   "\x1B(0");      /* Enter graphics mode. */
    set_term_feature(TF_TEXT_MODE,      "\x1B(B");      /* Exit graphics mode. */
}

/*end*/

