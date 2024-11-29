/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_msterminal.cr,v 1.6 2024/11/18 13:42:10 cvsuser Exp $
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
//  set_term_feature(TF_INIT, "\x1b=");         /* DECKPAM - Enable Keypad Application Mode */
//  set_term_feature(TF_INIT, "\x1b[?1h");      /* DECCKM  - Enable Cursor Keys Application Mode */
//  set_term_feature(TF_RESET, "\x1b>");        /* DECKPNM - Enable Keypad Numeric Mode */

    set_term_feature(TF_UNDERSTYLE, "\x1b[4:%dm"); // Smulx, terminfo=\E[4:%p1%dm

    xterm_graphic();
    xterm_256color();                           /* full 256 color is available */

    /*
     *  Terminal map
     */
    xterm_altmeta_keys();

    set_term_keyboard(
        //
        //  Function keys
        //
        //  https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#output-sequences
        //
        //   o Function keys:
        //
        //      Key                 Escape      Modified
        //      -------------------------------------------------
        //      F1                  ^[OP        ^[[1;mP
        //      F2                  ^[OQ        ^[[1;mQ
        //      F3                  ^[OR        ^[[1;mR
        //      F4                  ^[OS        ^[[1;mS
        //      F5                  ^[[15~      ^[[15;m~
        //      F6                  ^[[17~      ^[[17;m~
        //      F7                  ^[[18~      ^[[18;m~
        //      F8                  ^[[19~      ^[[19;m~
        //      F9                  ^[[20~      ^[[20;m~
        //      F10                 ^[[21~      ^[[21;m~
        //      F11                 ^[[23~      ^[[23;m~
        //      F12                 ^[[24~      ^[[24;m~
        //
        F1_F12, quote_list(                     /* xterm */
            "\x1bOP",       "\x1bOQ",       "\x1bOR",       "\x1bOS",       "\x1b[15~",
            "\x1b[17~",     "\x1b[18~",     "\x1b[19~",     "\x1b[20~",     "\x1b[21~",
            "\x1b[23~",     "\x1b[24~"),

        SHIFT_F1_F12, quote_list(               /* xterm/msterminal */
            "\x1b[1;2P",    "\x1b[1;2Q",    "\x1b[1;2R",    "\x1b[1;2S",    "\x1b[15;2~",
            "\x1b[17;2~",   "\x1b[18;2~",   "\x1b[19;2~",   "\x1b[20;2~",   "\x1b[21;2~",
            "\x1b[23;2~",   "\x1b[24;2~"),

        ALT_F1_F12, quote_list(                 /* xterm/msterminal */
            "\x1b[1;3P",    "\x1b[1;3Q",    "\x1b[1;3R",    "\x1b[1;3S",    "\x1b[15;3~",
            "\x1b[17;3~",   "\x1b[18;3~",   "\x1b[19;3~",   "\x1b[20;3~",   "\x1b[21;3~",
            "\x1b[23;3~",   "\x1b[24;3~"),

        ALTSHIFT_F1_F12, quote_list(            /* xterm/msterminal */
            "\x1b[1;4P",    "\x1b[1;4Q",    "\x1b[1;4R",    "\x1b[1;4S",    "\x1b[15;4~",
            "\x1b[17;4~",   "\x1b[18;4~",   "\x1b[19;4~",   "\x1b[20;4~",   "\x1b[21;4~",
            "\x1b[23;4~",   "\x1b[24;4~"),

        CTRL_F1_F12, quote_list(                /* xterm/msterminal */
            "\x1b[1;5P",    "\x1b[1;5Q",    "\x1b[1;5R",    "\x1b[1;5S",    "\x1b[15;5~",
            "\x1b[17;5~",   "\x1b[18;5~",   "\x1b[19;5~",   "\x1b[20;5~",   "\x1b[21;5~",
            "\x1b[23;5~",   "\x1b[24;5~"),

        CTRLSHIFT_F1_F12, quote_list(           /* xterm/msterminal */
            "\x1b[1;6P",    "\x1b[1;6Q",    "\x1b[1;6R",    "\x1b[1;6S",    "\x1b[15;6~",
            "\x1b[17;6~",   "\x1b[18;6~",   "\x1b[19;6~",   "\x1b[20;6~",   "\x1b[21;6~",
            "\x1b[23;6~",   "\x1b[24;6~"),

        ALTCTRL_F1_F12, quote_list(             /* xterm/msterminal */
            "\x1b[1;7P",    "\x1b[1;7Q",    "\x1b[1;7R",    "\x1b[1;7S",    "\x1b[15;7~",
            "\x1b[17;7~",   "\x1b[18;7~",   "\x1b[19;7~",   "\x1b[20;7~",   "\x1b[21;7~",
            "\x1b[23;7~",   "\x1b[24;7~"),

        ALTCTRLSHIFT_F1_F12, quote_list(        /* xterm/msterminal */
            "\x1b[1;8P",    "\x1b[1;8Q",    "\x1b[1;8R",    "\x1b[1;8S",    "\x1b[15;8~",
            "\x1b[17;8~",   "\x1b[18;8~",   "\x1b[19;8~",   "\x1b[20;8~",   "\x1b[21;8~",
            "\x1b[23;8~",   "\x1b[24;8~"),

        //
        //  Ins/0           End/1           Down/2          PgDn/3          Left/4
        //  5               Right/6         Home/7          Up/8            PgUp/9
        //  Del/.           Plus            Minus           Star            Divide
        //  Equals          Enter           Pause           PrtSc           Scroll
        //  NumLock
        //
        //  https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#output-sequences
        //
        //   o Cursor key:
        //
        //      Dependent on mode:
        //
        //      Key                 Normal Mode     Application Mode
        //      -------------------------------------------------------
        //      Up Arrow            ^[[A            ^[OA
        //      Down Arrow          ^[[B            ^[OB
        //      Right Arrow         ^[[C            ^[OC
        //      Left Arrow          ^[[D            ^[OD
        //      Home                ^[[H            ^[OH
        //      End                 ^[[F            ^[OF
        //
        //      Additionally, if Ctrl is pressed with any of these keys, the following sequences are emitted instead,
        //      regardless of the Cursor Keys Mode:
        //
        //      Mod-Up Arrow        ^[[1;mA
        //      Mod-Down Arrow      ^[[1;mB
        //      Mod-Right Arrow     ^[[1;mC
        //      Mod-Left Arrow      ^[[1;mD
        //      Mod-Home            ^[[1;mH
        //      Mod-End             ^[[1;mF
        //
        //  o Numpad & Function Keys
        //
        //      Ins                 ^[[2~           ^[[2;m~
        //      Del                 ^[[3~           ^[[3;m~
        //      PgUp                ^[[5~           ^[[5;m~
        //      PgDn                ^[[6~           ^[[6;m~
        //      Home                ^[[1~           ^[[1;m~
        //      End                 ^[[4~           ^[[4;m~
        //
        //
        //  Modifiers:
        //
        //      Code        Modifier
        //      -------------------------------------------------
        //      1           Normal (1 is optional)
        //      2           Shift
        //      3           Alt
        //      4           Shift + Alt
        //      5           Control
        //      6           Shift + Control
        //      7           Alt + Control
        //      8           Shift + Alt + Control
        //

        KEYPAD_0_9, quote_list(                 /* Standard (Application mode) */
            "\x1b[2~",      "\x1bOF",       "\x1bOB",       "\x1b[6~",      "\x1bOD",
            "\x1bOE",       "\x1bOC",       "\x1bOH",       "\x1bOA",       "\x1b[5~",
            "\x1b[3~",      "\x1bOk",       "\x1bOm",       "\x1bOj",       "\x1bOo",
            NULL,           "\x1bOM"),

        SHIFT_KEYPAD_0_9, quote_list(           /* xterm/msterminal */
            "\x1b[2;2~",    "\x1b[1;2F",    "\x1b[1;2B",    "\x1b[6;2~",    "\x1b[1;2D",
            NULL,           "\x1b[1;2C",    "\x1b[1;2H",    "\x1b[1;2A",    "\x1b[5;2~",
            "\x1b[3;2~",    NULL,           NULL,           "\x1b[1;2j",    NULL,
            NULL,           "\x1b[1;2M",    NULL,           NULL,           NULL,
            NULL),

        ALTSHIFT_KEYPAD_0_9, quote_list(        /* xterm/msterminal */
            "\x1b[2;4~",    "\x1b[1;4F",    "\x1b[1;4B",    "\x1b[6;4~",    "\x1b[1;4D",
            NULL,           "\x1b[1;4C",    "\x1b[1;4H",    "\x1b[1;4A",    "\x1b[5;4~",
            "\x1b[3;4~",    NULL,           NULL,           "\x1b[1;4j",    NULL,
            NULL,           "\x1b[1;4M",    "\x1c",         NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* xterm/msterminal */
            "\x1b[2;5~",    "\x1b[1;5F",    "\x1b[1;5B",    "\x1b[6;5~",    "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1b[1;5H",    "\x1b[1;5A",    "\x1b[5;5~",
            "\x1b[3;5~",    NULL,           NULL,           "\x1b[1;5j",    NULL,
            NULL,           "\x1b[1;5M",    "\x1c",         NULL,           NULL,
            NULL),

        CTRLSHIFT_KEYPAD_0_9, quote_list(       /* xterm/msterminal */
            "\x1b[2;6~",    "\x1b[1;6F",    "\x1b[1;6B",    "\x1b[6;6~",    "\x1b[1;6D",
            NULL,           "\x1b[1;6C",    "\x1b[1;6H",    "\x1b[1;6A",    "\x1b[5;6~",
            "\x1b[3;6~",    NULL,           NULL,           "\x1b[1;6j",    NULL,
            NULL,           "\x1b[1;6M",    NULL,           NULL,           NULL,
            NULL),

        ALT_KEYPAD_0_9, quote_list(             /* xterm/msterminal */
            "\x1b[2;3~",    "\x1b[1;3F",    "\x1b[1;3B",    "\x1b[6;3~",    "\x1b[1;3D",
            NULL,           "\x1b[1;3C",    "\x1b[1;3H",    "\x1b[1;3A",    "\x1b[5;3~",
            "\x1b[3;3~",    NULL,           NULL,           "\x1b[1;3j",    NULL,
            NULL,           "\x1b[1;3M",    NULL,           NULL,           NULL,
            NULL),

        ALTSHIFT_KEYPAD_0_9, quote_list(        /* xterm/msterminal */
            "\x1b[2;4~",    "\x1b[1;4F",    "\x1b[1;4B",    "\x1b[6;4~",    "\x1b[1;4D",
            NULL,           "\x1b[1;4C",    "\x1b[1;4H",    "\x1b[1;4A",    "\x1b[5;4~",
            "\x1b[3;4~",    NULL,           NULL,           "\x1b[1;4j",    NULL,
            NULL,           "\x1b[1;4M",    NULL,           NULL,           NULL,
            NULL),

        ALTCTRL_KEYPAD_0_9, quote_list(         /* xterm/msterminal */
            "\x1b[2;7~",    "\x1b[1;7F",    "\x1b[1;7B",    "\x1b[6;7~",    "\x1b[1;7D",
            NULL,           "\x1b[1;7C",    "\x1b[1;7H",    "\x1b[1;7A",    "\x1b[5;7~",
            "\x1b[3;7~",    NULL,           NULL,           "\x1b[1;7j",    NULL,
            NULL,           "\x1b[1;7M",    NULL,           NULL,           NULL,
            NULL),

        ALTCTRLSHIFT_KEYPAD_0_9, quote_list(    /* xterm/msterminal */
            "\x1b[2;8~",    "\x1b[1;8F",    "\x1b[1;8B",    "\x1b[6;8~",    "\x1b[1;8D",
            NULL,           "\x1b[1;8C",    "\x1b[1;8H",    "\x1b[1;8A",    "\x1b[5;8~",
            "\x1b[3;8~",    NULL,           NULL,           "\x1b[1;8j",    NULL,
            NULL,           "\x1b[1;8M",    NULL,           NULL,           NULL,
            NULL),

        //
        //  Miscellous keys
        //
        BACK_TAB,           "\x1b[Z",
        KEY_BACKSPACE,      "\x7f"
    );
}


void
msterminal(void)
{
    /*NOTHING*/
}

/*end*/
