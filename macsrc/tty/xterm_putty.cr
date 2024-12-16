/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_putty.cr,v 1.6 2024/11/24 12:26:43 cvsuser Exp $
 * Putty terminal profile.
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
    set_term_feature(TF_NAME, "xterm-putty");
    if (inq_macro("xterm_util") <= 0) {
        load_macro("tty/xterm_util", FALSE);
    }

    /*
     *  Basic/common configuration
     */
    xterm_standard();
    xterm_graphic();
    xterm_256color();                           /* full 256 color is available */

    xterm_altmeta_keys();

    /*
     *  Terminal map ---
     *
     *   Supported/advised Keyboard Setup:
     *
     *      Backspace key:
     *         [x] Control-H              [x] Control-? (127)
     *
     *      Home/End:
     *         [x] Standard               [ ] rxvt
     *
     *      Function keys and keypad:
     *
     *         [x] ESC[n~     [x] Linux   [x] Xterm R6    [x] VT400
     *         [ ] VT100+     [ ] SCO     [x] Xterm 216+
     *
     *      Shift/Ctrl/Alt:
     *
     *         [ ] Ctrl toggles appmode   [x] xterm style bitmap
     *
     *	 Keyboard modes:
     *
     *   o  Default mode -- labelled ESC [n~, the function keys generate sequences like ESC [11~,
     *      ESC [12~ and so on. This matches the general behaviour of Digital's terminals.
     *
     *   o  Linux mode -- F6 to F12 behave just like the default mode, but F1 to F5 generate ESC [[A
     *      through to ESC [[E. This mimics the Linux virtual console.
     *
     *   o  Xterm R6 mode --- F5 to F12 behave like the default mode, but F1 to F4 generate ESC OP
     *      through to ESC OS, which are the sequences produced by the top row of the keypad on Digital's
     *      terminals.
     *
     *   o  VT400 mode --- all the function keys behave like the default mode, but the actual top row of
     *      the numeric keypad generates ESC OP through to ESC OS.
     *
     *   o  VT100+ mode -- the function keys generate ESC OP through to ESC O[
     *
     *   o  SCO mode (see sco.cr) -- the function keys F1 to F12 generate ESC [M through to ESC [X. Together with
     *      shift, they generate ESC [Y through to ESC [j. With control they generate ESC [k through to
     *      ESC [v, and with shift and control together they generate ESC [w through to ESC [{
     *
     *   o  Xterm 216 mode -- the unshifted function keys behave the same as Xterm R6 mode. But pressing
     *      a function key together with Shift or Alt or Ctrl generates a different sequence containing
     *      an extra numeric parameter of the form (1 for Shift) + (2 for Alt) + (4 for Ctrl) + 1. For
     *      F1-F4, the basic sequences like ESC OP become ESC [1;bitmapP and similar; for F5 and above,
     *      ESC[index~ becomes ESC[index;bitmap~.
     *
     *  Cursor modes:
     *
     *   This option affects the arrow keys, if you press one with any of the modifier keys Shift, Ctrl or Alt held down.
     *
     *   o  In the default mode, labelled Ctrl toggles app mode, the Ctrl key toggles between the default arrow-key
     *      sequences like ESC [A and ESC [B, and the sequences Digital's terminals generate in "application cursor keys"
     *      mode, i.e. ESC O A and so on. Shift and Alt have no effect.
     *
     *   o  In the "xterm-style bitmap" mode, Shift, Ctrl and Alt all generate different sequences,
     *      with a number indicating which set of modifiers is active.
     */
    set_term_keyboard(
        //  F1,             F2,             F3,             F4,             F5,
        //  F6,             F7,             F8,             F9,             F10,
        //  F11,            F12
        //
        F1_F12, quote_list(                     /* default */
            "\x1b[11~",     "\x1b[12~",     "\x1b[13~",     "\x1b[14~",     "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

            //F11=SF1,F12=SF12: are ambiguous.

        SHIFT_F1_F12, quote_list(
            NULL,           NULL,           "\x1b[25~",     "\x1b[26~",     "\x1b[28~",
            "\x1b[29~",     "\x1b[31~",     "\x1b[32~",     "\x1b[33~",     "\x1b[34~",
            NULL,           NULL),

        ALT_F1_F12, quote_list(                 /* meta-esc */
            "\x1b\x1b[11~", "\x1b\x1b[12~", "\x1b\x1b[13~", "\x1b\x1b[14~", "\x1b\x1b[15~",
            "\x1b\x1b[17~", "\x1b\x1b[18~", "\x1b\x1b[19~", "\x1b\x1b[20~", "\x1b\x1b[21~",
            "\x1b\x1b[23~", "\x1b\x1b[24~"),

        F1_F12, quote_list(                     /* linux: F1..F5, xterm: F6..F12 */
            "\x1b[[A",      "\x1b[[B",      "\x1b[[C",      "\x1b[[D",      "\x1b[[E",
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL),

        ALT_F1_F12, quote_list(                 /* meta-esc, linux: F1..F5, xterm: F6..F12 */
            "\x1b\x1b[[A",  "\x1b\x1b[[B",  "\x1b\x1b[[C",  "\x1b\x1b[[D",  "\x1b\x1b[[E",
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL),

        F1_F12, quote_list(                     /* xterm: F1..F4 */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL),

        ALT_F1_F12, quote_list(                 /* xterm: F1..F4 */
            "\x1b\x1bOP",   "\x1b\x1bOQ",   "\x1b\x1bOR",   "\x1b\x1bOS",   NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL),

        F1_F12, quote_list(                     /* vt100: F1..F4 */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL),

        SHIFT_F1_F12, quote_list(               /* xterm-216: SF1..SF12 */
            "\x1b[1;2P",    "\x1b[1;2Q",    "\x1b[1;2R",    "\x1b[1;2S",    "\x1b[15;2~",
            "\x1b[17;2~",   "\x1b[18;2~",   "\x1b[19;2~",   "\x1b[20;2~",   "\x1b[21;2~",
            "\x1b[23;2~",   "\x1b[24;2~"),

        ALTSHIFT_F1_F12, quote_list(            /* xterm-216: ASF1..ASF12 */
            "\x1b\x1b[1;4P",     "\x1b\x1b[1;4Q",     "\x1b\x1b[1;4R",        "\x1b\x1b[1;4S",     "\x1b\x1b[15;4~",
            "\x1b\x1b[17;4~",    "\x1b\x1b[18;4~",    "\x1b\x1b[19;4~",       "\x1b\x1b[20;4~",    "\x1b\x1b[21;4~",
            "\x1b\x1b[23;4~",    "\x1b\x1b[24;4~"),

        CTRL_F1_F12, quote_list(                /* xterm-216: CF1..CF12 */
            "\x1b[1;5P",    "\x1b[1;5Q",    "\x1b[1;5R",    "\x1b[1;5S",    "\x1b[15;5~",
            "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",   "\x1b[20;5~",   "\x1b[21;5~",
            "\x1b[23;5~",   "\x1b[24;5~"),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        KEYPAD_0_9, quote_list(                 /* default */
            "\x1b[2~",      "\x1b[4~",      "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1b[1~",      "\x1bOA",       "\x1b[5~",
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        KEYPAD_0_9, quote_list(
            NULL,           "\x1bOq",       "\x1bOr",       "\x1bOs",       "\x1bOt",
            "\x1bOu",       "\x1bOv",       "\x1bOw",       "\x1bOx",       "\x1bOy",
            NULL,           "\x1bOl",       NULL,           NULL,           NULL,
            NULL,           "\x1bOM",       NULL,           NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* xterm */
            NULL,           NULL,           "\x1b[B",       NULL,           "\x1b[D",
            NULL,           "\x1b[C",       NULL,           "\x1b[A",       NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        ALT_KEYPAD_0_9, quote_list(             /* meta-esc, xterm */
            "\x1b\x1b[2~",  "\x1b\x1b[4~",  "\x1b\x1bOB",   "\x1b\x1b[6~",  "\x1b\x1bOD",
            "\x1b\x1bOE",   "\x1b\x1bOC",   "\x1b\x1bOH",   "\x1b\x1bOA",   "\x1b\x1b[5~",
            "\x1b\x1b[3~",  NULL,           NULL,           NULL,           NULL,
            NULL,           NULL,           NULL,           NULL,           NULL,
            NULL),

        //  Miscellaneous
        //
        BACK_TAB,           "\x1b[Z",
        KEY_BACKSPACE,      "\x7f"
    );
}


/*
 *  xterm-putty
 */
void
putty(void)
{
    /*NOTHING*/
}

/*end*/
