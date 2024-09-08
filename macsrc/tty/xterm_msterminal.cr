/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_msterminal.cr,v 1.2 2024/08/25 06:02:04 cvsuser Exp $
 * msterminal terminal profile.
 * https://learn.microsoft.com/en-us/windows/terminal/
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
    set_term_feature(TF_NAME, "xterm-msterminal");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Basic/common configuration
     */
//  set_term_feature(TF_INIT, "\x1b=");         /* DECKPAM  Enable Keypad Application Mode */
//  set_term_feature(TF_INIT, "\x1b[?1h");      /* DECCKM - Enable Cursor Keys Application Mode */
//  set_term_feature(TF_RESET, "\x1b>");        /* DECKPNM  Enable Keypad Numeric Mode */

//  set_term_feature(TF_INIT, "\x1b[?9001h");   /* enable win32-input-mode */
//  set_term_feature(TF_RESET, "\x1b[?9001l");  /* disable win32-input-mode */

    xterm_graphic();
    xterm_256color();                           /* full 256 color is available */

    /*
     *  Terminal map
     */
    set_term_keyboard(
        //
        //  Function keys
        //
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

        CTRLSHIFT_F1_F12, quote_list(           /* msterminal/mintty */
            "\x1b[1;6P",    "\x1b[1;6Q",    "\x1b[1;6R",    "\x1b[1;6S",    "\x1b[15;6~",
            "\x1b[17;6~",   "\x1b[18;6~",   "\x1b[19;6~",   "\x1b[20;6~",   "\x1b[21;6~",
            "\x1b[23;6~",   "\x1b[24;6~"),

        ALT_F1_F12, quote_list(                 /* msterminal */
            "\x1b[1;3P",    "\x1b[1;3Q",    "\x1b[1;3R",    "\x1b[1;3S",    "\x1b[15;3~",
            "\x1b[17;3~",   "\x1b[18;3~",   "\x1b[19;3~",   "\x1b[20;3~",   "\x1b[21;3~",
            "\x1b[23;3~",   "\x1b[24;3~"),

        //
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

        //
        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        SHIFT_KEYPAD_0_9, quote_list(           /* msterminal */
            NULL,           "\x1b[1;2F",    "\x1b[1;2B",    "\x1b[6;2~",    "\x1b[1;2D",
            NULL,           "\x1b[1;2C",    "\x1b[1;2H",    "\x1b[1;2A",    "\x1b[5;2~",
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* msterminal */
            NULL,           "\x1b[1;5F",    "\x1b[1;5B",    "\x1b[6;5~",    "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1b[1;5H",    "\x1b[1;5A",    "\x1b[5;5~",
            "\x1b[3;5~",    NULL,           NULL,           "\x1b[1;5j",    NULL,
            NULL,           "\x1b[1;5M",    "\x1c",         NULL,           NULL,
            NULL),

        //
        //  Miscellous keys
        //
        ALT_KEYPAD_2,       "\x1b[1;3B",        /* Alt-Down */
        ALT_KEYPAD_4,       "\x1b[1;3D",        /* Alt-Left */
        ALT_KEYPAD_6,       "\x1b[1;3C",        /* Alt-Right */
        ALT_KEYPAD_8,       "\x1b[1;3A",        /* Alt-Up */

        BACK_TAB,           "\x1b[Z",
        KEY_BACKSPACE,      "\x7f"
    );

    xterm_altmeta_keys();
}


void
msterminal(void)
{
    /*NOTHING*/
}

/*end*/


