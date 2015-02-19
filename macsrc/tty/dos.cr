/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dos.cr,v 1.12 2014/10/22 02:34:39 ayoung Exp $
 * Terminal descriptor for DOS running PC/NFS telnet or Excelan telnet
 *
 *
 */

#include "tty.h"

void
main()
{
    int colordepth, codepage;

    /*
     *  Terminal name
     */
    get_term_feature(TF_COLORDEPTH, colordepth);
    set_term_feature(TF_NAME, (colordepth > 16 ? "xconsole-dos" : "console-dos"));

    /*
     *  Set characters used for extended graphics support when drawing windows.
     */
    get_term_feature(TF_CODEPAGE, codepage);

    if ((codepage >= 1250 && codepage <= 1259) || 819 == codepage) {
        /*
         *  ANSI/MS-Windows ---
         *      http://en.wikipedia.org/wiki/Windows-1250
         */
        set_term_characters(
            '+',                /* Top left of window. */
            '+',                /* Top right of window. */
            '+',                /* Bottom left of window. */
            '+',                /* Bottom right of window. */
            '|',                /* Vertical bar for window sides. */
            '-',                /* Top and bottom horizontal bar for window. */
            '+',                /* Top join. */
            '+',                /* Bottom join. */
            '+',                /* Window 4-way intersection. */
            '+',                /* Left hand join. */
            '+',                /* Right hand join. */
            0,                  /* Scroll (none). */
            '*'                 /* Thumb. */
            );

    } else if (codepage >= 850 && codepage <= 869) {
        /*
         *  Latin1/2 ---
         *      http://en.wikipedia.org/wiki/Code_page_850
         *      http://en.wikipedia.org/wiki/Code_page_852
         */
        set_term_characters(
            218,                /* (0xda) Top left of window */
            191,                /* (0xbf) Top right of window */
            192,                /* (0xc0) Bottom left of window */
            217,                /* (0xd9) Bottom right of window */
            179,                /* (0xb3) Vertical bar for window sides */
            196,                /* (0xc4) Top and bottom horizontal bar for window */
            194,                /* (0xc2) Top join */
            193,                /* (0xc1) Bottom join */
            197,                /* (0xc5) Window 4-way intersection */
            180,                /* (0xb4) Left hand join */
            195,                /* (0xc3) Right hand join */
            176,                /* (0xb0) Scroll */
            178                 /* (0xb2) Thumb */
            );

    } else {
        /*
         *  Default IBM Code Page (437) or PC ---
         *      http://en.wikipedia.org/wiki/Code_page_437
         */
        set_term_characters(
            213,                /* (0xd5) Top left of window */
            184,                /* (0xb8) Top right of window */
            212,                /* (0xd4) Bottom left of window */
            190,                /* (0xbe) Bottom right of window */
            179,                /* (0xb3) Vertical bar for window sides */
            205,                /* (0xcd) Top and bottom horizontal bar for window */
            209,                /* (0xd1) Top join */
            207,                /* (0xcf) Bottom join */
            216,                /* (0xd8) Window 4-way intersection */
            181,                /* (0xb5) Left hand join */
            198,                /* (0xc6) Right hand join */
            176,                /* (0xb0) Scroll */
            178                 /* (0xb2) Thumb */
            );
    }

    /*
     *  Define escape sequences used for special optimisations on output
     */
    set_term_features(
        NULL,                   /* Sequence, clear 'n' spaces. */
        "%c",                   /* Sequence. print characters with top bitset */
        "\x1B[23m",             /* Sequence, insert-mode cursor. */
        "\x1B[24m",             /* Sequence, overwrite-mode cursor. */
        "\x1B[28;10;13m",       /* Sequence, insert-mode cursor (on virtual space). */
        "\x1B[28;1;8m",         /* Sequence, overwrite-mode cursor (on virtual space). */
        "\x1B\x1B",             /* Sequence, print ESCAPE character graphically. */
        "\x1B[%db",             /* Sequence, repeat last character 'n' times. */
        TRUE,                   /* Boolean,  ESC [0m resets color. */
        TRUE,                   /* Boolean,  terminal supports color. */
        "\x1B[%dC"              /* Sequence, move cursor on same line. */
        );
}
