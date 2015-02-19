/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: ftp.cr,v 1.6 2014/10/22 02:34:40 ayoung Exp $
 * description file for VT-100/VT-200 type terminals.
 *
 *  If you are going to use this then you will need to put the following csh commands
 *  in your .login depending on system type:
 *
 *      alias cr cr -medt       # only if you want edt startup
 *      setenv TERM vt200       # or vt100 as appropriate
 *      setenv GRTERM ftp
 *      setenv BKBD pc
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "ftp");

    set_term_characters(
        "\x1B(0l\x1B(B",        /* Top left of window */
        "\x1B(0k\x1B(B",        /* Top right of window */
        "\x1B(0m\x1B(B",        /* Bottom left of window */
        "\x1B(0j\x1B(B",        /* Bottom right of window */
        "\x1B(0x\x1B(B",        /* Vertical bar for window sides */
        "\x1B(0q\x1B(B",        /* Top and bottom horizontal bar for window */
        '+',                    /* Top join */
        '+',                    /* Bottom join */
        '+',                    /* Window 4-way intersection */
        '+',                    /* Left hand join */
        '+'                     /* Right hand join */
        );

    set_term_features(
        NULL,                   /* Sequence, clear 'n' spaces. */
        NULL,                   /* Sequence. print characters with top bitset */
        NULL,                   /* Sequence, insert-mode cursor. */
        NULL,                   /* Sequence, overwrite-mode cursor. */
        NULL,                   /* Sequence, insert-mode cursor (on virtual space). */
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,                   /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        FALSE,                  /* Boolean,  ESC [0m resets color. */
        FALSE,                  /* Boolean,  terminal supports color. */
        "\x1B[%dC"              /* Sequence, move cursor on same line. */
        );

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1BOP",       "\x1BOQ",       "\x1BOR",       "\x1BOS",       "\x1B[17~",
            "\x1B[18~",     "\x1B[19~",     "\x1B[20~",     "\x1B[21~",     "\x1B[22~"),

        ALT_A_Z, quote_list(
            "\x1B[a",       "\x1B[b",       "\x1B[c",       "\x1B[d",
            "\x1B[e",       "\x1B[f",       "\x1B[g",       "\x1B[h",
            "\x1B[i",       "\x1B[j",       "\x1B[k",       "\x1B[l",
            "\x1B[m",       "\x1B[n",       "\x1B[o",       "\x1B[p",
            "\x1B[q",       "\x1B[r",       "\x1B[s",       "\x1B[t",
            "\x1B[u",       "\x1B[v",       "\x1B[w",       "\x1B[x",
            "\x1B[y",       "\x1B[z"),

        KEYPAD_0_9, quote_list(
            "\x1b[33~",     "\x1b[31~",     "\x1b[B",       "\x1bOi",       "\x1b[D",
            NULL,           "\x1b[C",       "\x1b[32~",     "\x1b[A",       "\x1bOj"),

        ALT_0_9, quote_list(
            "\x1B[0",       "\x1B[1",       "\x1B[2",       "\x1B[3",       "\x1B[4",
            "\x1B[5",       "\x1B[6",       "\x1B[7",       "\x1B[8",       "\x1B[9"),

        SHIFT_F1_F12, quote_list(
            NULL,           NULL,           NULL,           NULL,           "\x1b[23~",
            "\x1b[24~",     "\x1b[25~",     NULL,           NULL,           "\x1b[26~"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           "\x1bOa",       NULL,           "\x1bOe",       "\x1bOc",
            NULL,           "\x1bOf",       "\x1bOb",       NULL,           "\x1bOd"),

        COPY,   "\x1b[35~",
        CUT,    "\x1b[34~",
        DEL,    "\x1b[29~"
        );
}


void
xterm_arrow()
{
    set_term_keyboard(
        KEYPAD_0_9, quote_list(
            "\x1B[212z",    "\x1B[220z",    "\x1B[B",       "\x1B[222z",
            "\x1B[D",       NULL,           "\x1B[C",       "\x1B[214z",
            "\x1B[A")
        );
}

