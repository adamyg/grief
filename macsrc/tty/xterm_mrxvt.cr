/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_mrxvt.cr,v 1.6 2024/07/12 16:33:22 cvsuser Exp $
 * mrxvt terminal
 *
 *      The mrxvt program is a terminal emulator for X Window System. It provides DEC VT102
 *      compatible terminals for programs that cannot use the window system directly.
 *
 *      mrxvt is based on rxvt(1) version 2.7.11 CVS, and features most of functionality of rxvt,
 *      with a few major enhancements (namely multiple tabs, and transparency). Like rxvt, mrxvt
 *      aims to be light, fast, flexible and desktop independent, thus KDE or GNOME are not
 *      required.
 *
 *      The primary features of mrxvt include (but are not limited to) multiple tabs, dynamically
 *      changeable tab titles, customisable command for each tab, input broadcasting, true
 *      translucent window, fast pseudo transparency with tinting, user supplied background
 *      images (XPM, JPEG, PNG), off-focus fading, text shadow, multiple style (NeXT, Rxvt,
 *      Xterm, SGI, Plain) scrollbars, XIM, multi-language support (Chinese, Korean, Japanese),
 *      freetype font and logging.
 *
 *  Example Rssource:

!**********************************************************************************
!   mrxvt settings (5.x)
!
|   o Enable <Shift>+<Tab>
|
Mrxvt.macro.Shift+Tab:		Str \e[Z
Mrxvt.macro.Shift+KP_Tab:	Str \e[Z

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
    set_term_feature(TF_NAME, "xterm-mrxvt");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  extended graphics support when drawing windows.
     */
    xterm_graphic();

    /*
     *  display escape sequences
     */
    xterm_color();                              /* colour by default */

    int attributes;

    get_term_feature(TF_ATTRIBUTES, attributes);
    if (0 == (attributes & TF_AFUNCTIONKEYS)) {
        set_term_keyboard(
             F1_F12, quote_list(                /* F1-F12 */
                "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",
                "\x1b[15~",     "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
                "\x1b[20~",     "\x1b[21~",     "\x1b[23~",     "\x1b[24~"));
    } else {
        /* XXX - need inq_term_control()/set_term_control() */
        set_term_keyboard(                      /* F11-F12, generally missing from termcap/info */
            F(11), "\x1b[23~",
            F(12), "\x1b[24~"
            );
    }

    /*
     *  keyboard for non-ascii characters.
     */
    set_term_keyboard(                          /* Function keys */
        //
        //  Function keys
        //
        SHIFT_F1_F12, quote_list(
            "\x1b[11;2~",   "\x1b[12;2~",   "\x1b[13;2~",   "\x1b[14;2~",   "\x1b[15;2~",
            "\x1b[17;2~",   "\x1b[18;2~",   "\x1b[19;2~",   "\x1b[20;2~",   "\x1b[21;2~",
            "\x1b[23;2~",   "\x1b[24;2~"),

        CTRL_F1_F12, quote_list(
            "\x1b[11;5~",   "\x1b[12;5~",   "\x1b[13;5~",   "\x1b[14;5~",   "\x1b[15;5~",
            "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",   "\x1b[20;5~",   "\x1b[21;5~",
            "\x1b[23;5~",   "\x1b[24;5~"),

        ALT_F1_F12, quote_list(
            "\x1b[11;3~",   "\x1b[12;3~",   "\x1b[13;3~",   "\x1b[14;3~",   "\x1b[15;3~",
            "\x1b[17;3~",   "\x1b[18;3~",   "\x1b[19;3~",   "\x1b[20;3~",   "\x1b[21;3~",
            "\x1b[23;3~",   "\x1b[24;3~"),

        ALT_A_Z, quote_list(                    /* Alt-[A-Z] */
            "\x1b\xe1",     NULL,           NULL,           NULL,           "\x1b\xe9",
            NULL,           NULL,           NULL,           "\x1b\xed",     NULL,
            NULL,           NULL,           NULL,           NULL,           "\x1b\xf3",
            NULL,           NULL,           NULL,           NULL,           NULL,
            "\x1b\xfa",     NULL,           NULL,           NULL,           NULL,
            NULL),

        ALT_A_Z, quote_list(                    /* Shifted Alt-[A-Z] */
            "\x1b\xc1",     NULL,           NULL,           NULL,           "\x1b\xc9",
            NULL,           NULL,           NULL,           "\x1b\xcd",     NULL,
            NULL,           NULL,           NULL,           NULL,           "\x1b\xd3",
            NULL,           NULL,           NULL,           NULL,           NULL,
            "\x1b\xda",     NULL,           NULL,           NULL,           NULL,
            NULL),

        KEY_HOME,           "\x1b[7~",          /* cursor */
        KEY_PAGEUP,         "\x1b[5~",
        KEY_END,            "\x1b[8~",
        KEY_PAGEDOWN,       "\x1b[6~",
        KEY_INS,            "\x1b[2~",
        KEY_DEL,            "\x1b[3~",

        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del C           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* Keypad */
            "\x1bOp",       "\x1bOq",       "\x1bOr",       "\x1bOs",       "\x1bOt",
            "\x1bOu",       "\x1bOv",       "\x1bOw",       "\x1bOx",       "\x1bOy",
            "\x1bOn",       "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM",       NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        SHIFT_KEYPAD_0_9, quote_list(           /* Shift keypad */
            "\x1bO5p",      "\x1bO5q",      "\x1bO5r",      "\x1bO5s",      "\x1bO5t",
            "\x1bO5u",      "\x1bO5v",      "\x1bO5w",      "\x1bO5x",      "\x1bO5y",
            "\x1bO5n",      NULL,           NULL,           "\x1bO5j",      "\x1bO5o",
            NULL,           "\x1bO5M",      NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        CTRL_KEYPAD_0_9, quote_list(            /* Ctrl keypad */
            NULL,           "\x1b[8;5~",    "\x1b[1;5B",    NULL,           "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1b[7;5~",    "\x1b[1;5A",    NULL,
            NULL,           "\x1bO5k",      "\x1bO5m",      NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
#if defined(ALT_KEYPAD_0_9)
        ALT_KEYPAD_0_9, quote_list(             /* Alt keypad */
            "\x1b[2;3~",    "\x1b[8;3~",    "\x1b[1;3B",    "\x1b[6;3~",    "\x1b[1;3D",
            NULL,           "\x1b[1;3C",    "\x1b[7;3~",    "\x1b[1;3A",    "\x1b[5;3~",
            "\x1b[3~",      "\x1bO3k",      "\x1bO3m",      "\x1bO3j",      "\x1bO3o",
            NULL,           "\x1bO3M",      NULL,           NULL,           NULL,
            NULL),
#endif

        BACK_TAB,           "\x1b[Z",           /* Shift-Tab, may require Xdefault resource about */
        BACK_TAB,           "\x1b\t",           /* Alt-Tab */
        KEY_BACKSPACE,      "\x7f"
        );

    xterm_altmeta_keys();
}


void
mrxvt()
{
    /*NOTHING*/
}

/*end*/
