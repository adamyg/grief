/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: sco.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * Terminal description file for SCO Unix 386 Console Colour display.
 *
 * This file, along with the associated keys and strings files
 * provide support for almost all common crisp keys.
 *
 * Note that the keypad / and enter keys only function if the
 * keyboard is in AT mode (SCO command: kbmode at). All other
 * keys will produce identical scan codes in either mode.
 *
 * The Keys Keypad Equal, Pause, Scroll and Numlock, along with their 
 * ctrl/shift/alt derivatives are not supported as either the keys 
 * do not exist on PC compatible keyboards or they perform other functions.
 *
 * The PrtScr key is also presently unsupported. This key should
 * work but I can't work out how to get it going. Ctrl/shift/alt
 * PrtScr are not supported in any case.
 *
 *
 */

#include  "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "sco");

    set_term_characters(
        213,                    /* Top left of window. */
        184,                    /* Top right of window. */
        212,                    /* Bottom left of window. */
        190,                    /* Bottom right of window. */
        179,                    /* Vertical bar for window sides. */
        205,                    /* Top and bottom horizontal bar for window. */
        0xd1,                   /* Top join. */
        0xcf,                   /* Bottom join. */
        0xd8,                   /* Window 4-way intersection. */
        0xb5,                   /* Left hand join. */
        0xc6                    /* Right hand join. */
        );

    set_term_features(
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
        "\x1B[%dg",             /* Sequence. print characters with top bitset */
        NULL,                   /* Sequence, insert-mode cursor. */
        NULL,                   /* Sequence, overwrite-mode cursor. */
        NULL,                   /* Sequence, insert-mode cursor (on virtual space). */
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B[27g",             /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE,                   /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                   /* Boolean,  allow scrolling (ins/del). */
        NULL,                   /* Sequence, enter graphics mode. */
        NULL,                   /* Sequence, exit graphics mode. */
        "\x1B[0m",
        "\x1B[0m"
        );

    set_term_keyboard(
        F1_F12,
            quote_list(
                "\x1B[M",   "\x1B[N",   "\x1B[O",   "\x1B[P",   "\x1B[Q",
                "\x1B[R",   "\x1B[S",   "\x1B[T",   "\x1B[U",   "\x1B[V",
                "\x1B[W",   "\x1B[X"),

        SHIFT_F1_F12,
            quote_list(
                "\x1B[Y",   "\x1B[Z",   "\x1B[a",   "\x1B[b",   "\x1B[c",
                "\x1B[d",   "\x1B[e",   "\x1B[f",   "\x1B[g",   "\x1B[h",
                "\x1B[i",   "\x1B[j"),

        CTRL_F1_F12,
            quote_list(
                "\x1B[k",   "\x1B[l",   "\x1B[m",   "\x1B[n",   "\x1B[o",
                "\x1B[p",   "\x1B[q",   "\x1B[r",   "\x1B[s",   "\x1B[t",
                "\x1B[u",   "\x1B[v"),

        ALT_F1_F12,
            quote_list(
                "\x1B[w",   "\x1B[x",   "\x1B[y",   "\x1B[z",   "\x1B[@",
                "\x1B[[",   "\x1B[\\",  "\x1B[]",   "\x1B[^",   "\x1B[_",
                "\x1B[`",   "\x1B[{"),

        KEYPAD_0,
            quote_list(
                "\x1B[L",   "\x1B[F",   "\x1B[B",   "\x1B[G",   "\x1B[D",
                "\x1B[E",   "\x1B[C",   "\x1B[H",   "\x1B[A",   "\x1B[I",
                "\x1B[K",   "\x1B[+",   "\x1B[-",   "\x1B[*",   "\x1B[/",
                NULL,       "\x1B[J",   NULL,       "\x1B[}"),

        BACK_TAB,           "\x1B[Z",

        ALT_A_Z,
            quote_list(
                0xe1,       0xe2,       0xe3,       0xe4,       0xe5,       0xe6,       0xe7,
                0xe8,       0xe9,       0xea,       0xeb,       0xec,       0xed,       0xee,
                0xef,       0xf0,       0xf1,       0xf2,       0xf3,       0xf4,       0xf5,
                0xf6,       0xf7,       0xf8,       0xf9,       0xfa),

        ALT_0_9,
            quote_list(
                0xb0,       0xb1,       0xb2,       0xb3,       0xb4,       0xb5,       0xb6,
                0xb7,       0xb8,       0xb9),

        SHIFT_KEYPAD_0,
            quote_list(
                0x80,       0x81,       0x82,       0x83,       0x84,       0x85,       0x86,
                0x87,       0x88,       0x89,       0x8a,       0x8b,       0x8c,       0x8d,
                0x8e,       0x8f),

        CTRL_KEYPAD_0,
            quote_list(
                0xc0,       0xc1,       0xc2,       0xc3,       0xc4,       0xc5,       0xc6,
                0xc7,       0xc8,       0xc9,       0xca,       0xcb,       0xcc,       0xcd,
                0xce,       0xcf),

        __ALT_KEYPAD(0),
            quote_list(
                0xd0,       0xd1,       0xd2,       0xd3,       0xd4,       0xd5,       0xd6,
                0xd7,       0xd8,       0xd9,       0xda,       0xdb,       0xdc,       0xdd,
                0xde,       0xdf)
        );

    assign_to_key("<Ctrl-Del>",     "objects delete_word_right");

    /*
     *  Make Ctrl-L do what it should!
     */
    assign_to_key("<Ctrl-L>",       "self_insert");

    /*
     *  Use Alt-P for print since we can't get PrtScr to work
     */
    assign_to_key("<Alt-P>",        "print");
}


void
vga()
{
    set_term_features(
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
        "\x1B[%dg",             /* Sequence. print characters with top bitset */
        "\x1B[=12;13C",         /* Sequence, insert-mode cursor. */
        "\x1B[=1;13C",          /* Sequence, overwrite-mode cursor. */
        "\x1B[=8;13C",          /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[=1;8C",           /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B[27g",             /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE,                   /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                   /* Boolean,  allow scrolling (ins/del). */
        NULL,                   /* Sequence, enter graphics mode. */
        NULL,                   /* Sequence, exit graphics mode. */
        "\x1B[0m\x1B[=0C",
        "\x1B[0m\x1B[=3C"
        );
}


void
vga50()
{
    set_term_features(
        "\x1B[%dX",             /* Sequence, clear 'n' spaces. */
        "\x1B[%dg",             /* Sequence. print characters with top bitset */
        "\x1B[=6;7C",           /* Sequence, insert-mode cursor. */
        "\x1B[=1;7C",           /* Sequence, overwrite-mode cursor. */
        "\x1B[=4;7C",           /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[=1;4C",           /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B[27g",             /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC",             /* Sequence, move cursor on same line. */
        TRUE,                   /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                   /* Boolean,  allow scrolling (ins/del). */
        NULL,                   /* Sequence, enter graphics mode. */
        NULL,                   /* Sequence, exit graphics mode. */
        "\x1B[0m\x1B[=0C",
        "\x1B[0m\x1B[=3C"
        );
}


