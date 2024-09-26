/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_mintty.cr,v 1.10 2024/09/21 17:02:02 cvsuser Exp $
 * Mintty terminal profile.
 * See: https://github.com/mintty/mintty/wiki/CtrlSeqs
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
    xterm_256color();                           /* full 256 color is available */

    /*
     *  Terminal map
     */
    set_term_keyboard(
        //  https://github.com/mintty/mintty/wiki/Keycodes#modifier-key-encodings
        //
        //  Function keys:
        //
        //     Key         plain       modified
        //     -------------------------------------------------
        //     F1          ^[OP        ^[[1;mP
        //     F2          ^[OQ        ^[[1;mQ
        //     F3          ^[OR        ^[[1;mR
        //     F4          ^[OS        ^[[1;mS
        //     F5          ^[[15~      ^[[15;m~
        //     F6          ^[[17~      ^[[17;m~
        //     F7          ^[[18~      ^[[18;m~
        //     F8          ^[[19~      ^[[19;m~
        //     F9          ^[[20~      ^[[20;m~
        //     F10         ^[[21~      ^[[21;m~
        //     F11         ^[[23~      ^[[23;m~
        //     F12         ^[[24~      ^[[24;m~
        //
        //  Modifiers:
        //
        //     Modifier                m
        //     -------------------------------------------------
        //     Shift                   1
        //     Alt                     2
        //     Ctrl                    4
        //     Meta (Win key)          8
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

        CTRLSHIFT_F1_F12, quote_list(           /* mintty */
            "\x1b[1;6P",    "\x1b[1;6Q",    "\x1b[1;6R",    "\x1b[1;6S",    "\x1b[15;6~",
            "\x1b[17;6~",   "\x1b[18;6~",   "\x1b[19;6~",   "\x1b[20;6~",   "\x1b[21;6~",
            "\x1b[23;6~",   "\x1b[24;6~"),

        //
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

        //  Cursor keys:
        //
        //     Key         plain       app         modified
        //     -------------------------------------------------
        //     Up          ^[[A        ^[OA        ^[[1;mA
        //     Down        ^[[B        ^[OB        ^[[1;mB
        //     Left        ^[[D        ^[OD        ^[[1;mD
        //     Right       ^[[C        ^[OC        ^[[1;mC
        //     Home        ^[[H        ^[OH        ^[[1;mH
        //     End         ^[[F        ^[OF        ^[[1;mF
        //
        //  Editing keys:
        //
        //
        //     Key         plain       modified
        //     -------------------------------------------------
        //     Ins         ^[[2~       ^[[2;m~
        //     Del         ^[[3~       ^[[3;m~
        //     PgUp        ^[[5~       ^[[5;m~
        //     PgDn        ^[[6~       ^[[6;m~
        //     Home        ^[[1~       ^[[1;m~
        //     End         ^[[4~       ^[[4;m~
        //
        //  Number and symbol keys:
        //
        //     Key         modified    appl keypad modified
        //     -------------------------------------------------
        //     /           ^[[1;mo     ^[Omo
        //     *           ^[[1;mj     ^[Omj
        //     -           ^[[1;mm     ^[Omm
        //     +           ^[[1;mk     ^[Omk
        //     Enter       ^[OmM
        //     ,           ^[[1;ml
        //                             VT220 appl keypad modified
        //     .           ^[[1;mn     ^[Omn
        //     0           ^[[1;mp     ^[Omp
        //     1           ^[[1;mq     ^[Omq
        //     ...
        //     8           ^[[1;mx     ^[Omx
        //     9           ^[[1;my     ^[Omy
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
            NULL,           "\x1bOM",       NULL,           NULL,           NULL,
            NULL),

        //  Ins,            End,            Down,           PgDn,           Left,
        //  5,              Right,          Home,           Up,             PgUp,
        //  Del,            Plus,           Minus,          Star,           Divide,
        //  Equals,         Enter,          Pause,          PrtSc,          Scroll,
        //  NumLock
        //
        SHIFT_KEYPAD_0_9, quote_list(           /* xterm/mintty */
            "\x1b[2;2~",    "\x1b[1;2F",    "\x1b[1;2B",    "\x1b[6;2~",    "\x1b[1;2D",
            NULL,           "\x1b[1;2C",    "\x1b[1;2H",    "\x1b[1;2A",    "\x1b[5;2~",
            "\x1b[3;2~",    NULL,           NULL,           "\x1b[1;2j",    NULL,
            NULL,           "\x1b[1;2M",    NULL,           NULL,           NULL,
            NULL),

        CTRL_KEYPAD_0_9, quote_list(            /* xterm/mintty */
            "\x1b[2;5~",    "\x1b[1;5F",    "\x1b[1;5B",    "\x1b[6;5~",    "\x1b[1;5D",
            NULL,           "\x1b[1;5C",    "\x1b[1;5H",    "\x1b[1;5A",    "\x1b[5;5~",
            "\x1b[3;5~",    NULL,           NULL,           "\x1b[1;5j",    NULL,
            NULL,           "\x1b[1;5M",    "\x1c",         NULL,           NULL,
            NULL),

        CTRLSHIFT_KEYPAD_0_9, quote_list(       /* xterm/mintty */
            "\x1b[2;6~",    "\x1b[1;6F",    "\x1b[1;6B",    "\x1b[6;6~",    "\x1b[1;6D",
            NULL,           "\x1b[1;6C",    "\x1b[1;6H",    "\x1b[1;6A",    "\x1b[5;6~",
            "\x1b[3;6~",    NULL,           NULL,           "\x1b[1;6j",    NULL,
            NULL,           "\x1b[1;6M",    NULL,           NULL,           NULL,
            NULL),

        ALT_KEYPAD_0_9, quote_list(             /* xterm/mintty */
            "\x1b[2;3~",    "\x1b[1;3F",    "\x1b[1;3B",    "\x1b[6;3~",    "\x1b[1;3D",
            NULL,           "\x1b[1;3C",    "\x1b[1;3H",    "\x1b[1;3A",    "\x1b[5;3~",
            "\x1b[3;3~",    NULL,           NULL,           "\x1b[1;3j",    NULL,
            NULL,           "\x1b[1;3M",    NULL,           NULL,           NULL,
            NULL),

        ALTSHIFT_KEYPAD_0_9, quote_list(        /* xterm/mintty */
            "\x1b[2;4~",    "\x1b[1;4F",    "\x1b[1;4B",    "\x1b[6;4~",    "\x1b[1;4D",
            NULL,           "\x1b[1;4C",    "\x1b[1;4H",    "\x1b[1;4A",    "\x1b[5;4~",
            "\x1b[3;4~",    NULL,           NULL,           "\x1b[1;4j",    NULL,
            NULL,           "\x1b[1;4M",    NULL,           NULL,           NULL,
            NULL),

        ALTCTRL_KEYPAD_0_9, quote_list(         /* xterm/mintty */
            "\x1b[2;7~",    "\x1b[1;7F",    "\x1b[1;7B",    "\x1b[6;7~",    "\x1b[1;7D",
            NULL,           "\x1b[1;7C",    "\x1b[1;7H",    "\x1b[1;7A",    "\x1b[5;7~",
            "\x1b[3;7~",    NULL,           NULL,           "\x1b[1;7j",    NULL,
            NULL,           "\x1b[1;7M",    NULL,           NULL,           NULL,
            NULL),

        ALTCTRLSHIFT_KEYPAD_0_9, quote_list(    /* xterm/mintty */
            "\x1b[2;8~",    "\x1b[1;8F",    "\x1b[1;8B",    "\x1b[6;8~",    "\x1b[1;8D",
            NULL,           "\x1b[1;8C",    "\x1b[1;8H",    "\x1b[1;8A",    "\x1b[5;8~",
            "\x1b[3;8~",    NULL,           NULL,           "\x1b[1;8j",    NULL,
            NULL,           "\x1b[1;8M",    NULL,           NULL,           NULL,
            NULL),

        //  Miscellous keys
        //
        BACK_TAB,           "\x1b[Z",
        KEY_BACKSPACE,      "\x7f"
    );
}


void
mintty(void)
{
    /*NOTHING*/
}

/*end*/
