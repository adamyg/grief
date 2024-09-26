/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: cygwin.cr,v 1.14 2024/06/18 16:26:25 cvsuser Exp $
 * terminal description for the cygwin window
 *
 *
 */

#include "tty.h"

void
main()
{
    int codepage;

    set_term_feature(TF_NAME, "console-cygwin");

    /*
     *  Set characters used for extended graphics support when drawing windows.
     */
    get_term_feature(TF_CODEPAGE, codepage);
    if (codepage == 1252 || codepage == 819) {
        /* MS-Windows or Latin1 */
        set_term_characters(
           '+',            /* Top left of window. */
           '+',            /* Top right of window. */
           '+',            /* Bottom left of window. */
           '+',            /* Bottom right of window. */
           '|',            /* Vertical bar for window sides. */
           '-',            /* Top and bottom horizontal bar for window. */
           '+',            /* Top join. */
           '+',            /* Bottom join. */
           '+',            /* Window 4-way intersection. */
           '+',            /* Left hand join. */
           '+' );          /* Right hand join. */

    } else if (codepage == 850) {
        /* IBM Code Page */
        set_term_characters(
           218,            /* Top left of window */
           191,            /* Top right of window */
           192,            /* Bottom left of window */
           217,            /* Bottom right of window */
           179,            /* Vertical bar for window sides */
           196,            /* Top and bottom horizontal bar for window */
           194,            /* Top join */
           193,            /* Bottom join */
           197,            /* Window 4-way intersection */
           180,            /* Left hand join */
           195 );          /* Right hand join */

    } else {
        /* default IBM Code Page (437) or PC */
        set_term_characters(
           213,            /* Top left of window */
           184,            /* Top right of window */
           212,            /* Bottom left of window */
           190,            /* Bottom right of window */
           179,            /* Vertical bar for window sides */
           205,            /* Top and bottom horizontal bar for window */
           0xd1,           /* Top join */
           0xcf,           /* Bottom join */
           0xd8,           /* Window 4-way intersection */
           0xb5,           /* Left hand join */
           0xc6 );         /* Right hand join */
    }

    /*
     *  Define escape sequences used for special optimisations on output.
     */
    set_term_features(
        "\x1b[%dX",         /* Sequence, clear 'n' spaces. */
        NULL,               /* Sequence. print characters with top bitset */
        NULL,               /* Sequence, insert-mode cursor. */
        NULL,               /* Sequence, overwrite-mode cursor. */
        NULL,               /* Sequence, insert-mode cursor (on virtual space). */
        NULL,               /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,               /* Sequence, print ESCAPE character graphically. */
        NULL,               /* Sequence, repeat last character 'n' times. */
        FALSE,              /* Boolean,  ESC [0m resets color. */
        TRUE,               /* Boolean,  terminal supports color. */
        "\x1b[%dC",         /* Sequence, move cursor on same line. */
        TRUE,               /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,               /* Boolean,  allow scrolling (ins/del). */
                            /* Scroll is broken unless the scroll region is
                             * reset, but this then breaks console scrolling
                             * when the virtual screen has more lines then
                             * the physical screen.
                             */
        "\x1B[11m",         /* Sequence, enter graphics mode. */
        "\x1B[10m"          /* Sequence, exit graphics mode. */
        );


    /*
     *  Define keyboard layout for non-ascii characters.
     */
    set_term_keyboard(
        F1_F12, quote_list(
            "\x1b[[A",      "\x1b[[B",      "\x1b[[C",      "\x1b[[D",
            "\x1b[[E",      "\x1b[17~",     "\x1b[18~",     "\x1b[19~",
            "\x1b[20~",     "\x1b[21~",     "\x1b[23~",     "\x1b[24~" ),

        SHIFT_F1_F12, quote_list(
            NULL,           NULL,            "\x1b[25~",    "\x1b[26~",
            "\x1b[28~",     "\x1b[29~",      "\x1b[31~",    "\x1b[32~",
            "\x1b[33~",     "\x1b[34~",      "\x1b[23;2~",  "\x1b[24;2~" ),

        ALT_A_Z, quote_list(
            "\x1ba",        "\x1bb",        "\x1bc",        "\x1bd",        "\x1be",
            "\x1bf",        "\x1bg",        "\x1bh",        "\x1bi",        "\x1bj",
            "\x1bk",        "\x1bl",        "\x1bm",        "\x1bn",        "\x1bo",
            "\x1bp",        "\x1bq",        "\x1br",        "\x1bs",        "\x1bt",
            "\x1bu",        "\x1bv",        "\x1bw",        "\x1bx",        "\x1by",
            "\x1bz" ),

        ALT_A_Z, quote_list(
            "\x1bA",        "\x1bB",        "\x1bC",        "\x1bD",        "\x1bE",
            "\x1bF",        "\x1bG",        "\x1bH",        "\x1bI",        "\x1bJ",
            "\x1bK",        "\x1bL",        "\x1bM",        "\x1bN",        "\x1b0",
            "\x1bP",        "\x1bQ",        "\x1bR",        "\x1bS",        "\x1bT",
            "\x1bU",        "\x1bV",        "\x1bW",        "\x1bX",        "\x1bY",
            "\x1bZ"),

        ALT_0_9, quote_list(
            "\x1b0",        "\x1b1",        "\x1b2",        "\x1b3",        "\x1b4",
            "\x1b5",        "\x1b6",        "\x1b7",        "\x1b8",        "\x1b9" ),

        __ALT('~'),         "\x1b~",
        __ALT('`'),         "\x1b`",
        __ALT('!'),         "\x1b!",
        __ALT('@'),         "\x1b@",
        __ALT('#'),         "\x1b#",
        __ALT('$'),         "\x1b$",
        __ALT('%'),         "\x1b%",
        __ALT('^'),         "\x1b^",
        __ALT('&'),         "\x1b&",
        __ALT('*'),         "\x1b*",
        __ALT('('),         "\x1b(",
        __ALT(')'),         "\x1b)",
        __ALT('_'),         "\x1b_",
        __ALT('-'),         "\x1b-",
        __ALT('+'),         "\x1b+",
        __ALT('='),         "\x1b=",
        __ALT('{'),         "\x1b{",        /* Note: also reports RAW keys */
//      __ALT('['),         "\x1b[",        /* break other keys */
        __ALT('}'),         "\x1b}",
        __ALT(']'),         "\x1b]",
        __ALT('|'),         "\x1b|",
        __ALT('\\'),        "\x1b\\",
        __ALT(':'),         "\x1b:",
        __ALT(';'),         "\x1b;",
        __ALT(';'),         "\x1b;",
        __ALT('"'),         "\x1b\"",
        __ALT('\''),        "\x1b'",
        __ALT('<'),         "\x1b<",
        __ALT(','),         "\x1b,",
        __ALT('>'),         "\x1b>",
        __ALT('.'),         "\x1b.",
        __ALT('?'),         "\x1b?",
        __ALT('/'),         "\x1b/",

        MOUSE_KEY,          "\x1b[M",
        KEY_DOWN,           "\x1B[B",
        KEY_UP,             "\x1B[A",
        KEY_RIGHT,          "\x1B[C",
        KEY_LEFT,           "\x1B[D",
        KEY_HOME,           "\x1b[1~",
        KEY_INS,            "\x1b[2~",
        KEY_DEL,            "\x1b[3~",
        KEY_END,            "\x1b[4~",
        KEY_PAGEUP,         "\x1b[5~",
        KEY_PAGEDOWN,       "\x1b[6~"
        );
}


/*
 *  cygwin_256color ---
 *      Color terminal.
 *
 *  Usage:
 *      GRTERM=cygwin-256color
 */
void
cygwin_256color(void)
{
    set_term_feature(TF_COLOR, TRUE);           /* Terminal supports color. */
    set_term_feature(TF_COLORDEPTH, 256);       /* using a colour depth of 256. */
}



