/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: kermit.cr,v 1.5 2014/10/22 02:34:40 ayoung Exp $
 * kermit terminal emulation mode.
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "kermit");

    set_term_characters(
        "l",                    /* Top left of window */
        "k",                    /* Top right of window */
        "m",                    /* Bottom left of window */
        "j",                    /* Bottom right of window */
        "x",                    /* Vertical bar for window sides */
        "q",                    /* Top and bottom horizontal bar for window */
        "w",                    /* Top join */
        "v",                    /* Bottom join */
        "n",                    /* Window 4-way intersection */
        "u",                    /* Left hand join */
        "t"                     /* Right hand join */
        );

    set_term_features(
        NULL,                   /* Sequence. print characters with top bitset */
        NULL,                   /* Sequence, insert-mode cursor. */
        NULL,                   /* Sequence, overwrite-mode cursor. */
        NULL,                   /* Sequence, insert-mode cursor (on virtual space). */
        NULL,                   /* Sequence, overwrite-mode cursor (on virtual space). */
        NULL,                   /* Sequence, print ESCAPE character graphically. */
        NULL,                   /* Sequence, repeat last character 'n' times. */
        NULL,                   /* Boolean,  ESC [0m resets color. */
        FALSE,                  /* Boolean,  terminal supports color. */
        FALSE,                  /* Sequence, move cursor on same line. */
        "\x1B[%dC",             /* Boolean,  ESC[K gives us a black erased line. */
        TRUE,                   /* Boolean,  allow scrolling (ins/del). */
        FALSE,                  /* Sequence, enter graphics mode. */
        "\x1B(0",               /* Sequence, exit graphics mode. */
        "\x1B(B"                /* Sequence, Init */
        );                      /* Sequence, Reset */

    set_term_keyboard(
        F1_F12, quote_list(
            "\x1BOP",       "\x1BOQ",       "\x1BOR",       "\x1BOS",       "\x1BOT",
            "\x1BOU",       "\x1BOV",       "\x1BOW",       "\x1BOX",       "\x1BOY",
            "\x1BOZ",       "\x1BO["),

        SHIFT_F1_F12, quote_list(
            "\x1BOp",       "\x1BOq",       "\x1BOr",       "\x1BOs",       "\x1BOt",
            "\x1BOu",       "\x1BOv",       "\x1BOw",       "\x1BOx",       "\x1BOy",
            "\x1BOz",       "\x1BO{"),

        ALT_F1_F12, quote_list(
            "\x1BNP",       "\x1BNQ",       "\x1BNR",       "\x1BNS",       "\x1BNT",
            "\x1BNU",       "\x1BNV",       "\x1BNW",       "\x1BNX",       "\x1BNY",
            "\x1BNZ",       "\x1BN["),

        CTRL_F1_F12, quote_list(
            "\x1BNp",       "\x1BNq",       "\x1BNr",       "\x1BNs",       "\x1BNt",
            "\x1BNu",       "\x1BNv",       "\x1BNw",       "\x1BNx",       "\x1BNy",
            "\x1BNz",       "\x1BN{"),

        ALT_A_Z, quote_list(
            "\x1Ba",        "\x1Bb",        "\x1Bc",        "\x1Bd",        "\x1Be",
            "\x1Bf",        "\x1Bg",        "\x1Bh",        "\x1Bi",        "\x1Bj",
            "\x1Bk",        "\x1Bl",        "\x1Bm",        "\x1Bn",        "\x1Bo",
            "\x1Bp",        "\x1Bq",        "\x1Br",        "\x1Bs",        "\x1Bt",
            "\x1Bu",        "\x1Bv",        "\x1Bw",        "\x1Bx",        "\x1By",
            "\x1Bz"),

        KEYPAD_0_9, quote_list(
            "\x1B[@",       "\x05",         "\x1BOB",       "\x16",         "\x1BOD",
            "\x1B[G",       "\x1BOC",       "\x01",         "\x1BOA",       "\x1BV"),

        CTRL_KEYPAD_0_9, quote_list(
            "\x1B?0",       "\x1B\x05",     "\x1B?2",       "\x1B>",        "\x1BB",
            "\x1B?5",       "\x1BF",        "\x1B\x01",     "\x1B?8",       "\x1B<"),

        ALT_0_9, quote_list(
            "\x1B0",        "\x1B1",        "\x1B2",        "\x1B3",        "\x1B4",
            "\x1B5",        "\x1B6",        "\x1B7",        "\x1B8",        "\x1B9"),

        SHIFT_KEYPAD_0, quote_list(
            NULL,           "\x1B[y",       "\x1B[b",       NULL,           "\x1B[d",
            NULL,           "\x1B[c",       "\x1B[h",       "\x1B[a"),

        CUT,            "\x1B[S",
        COPY,           "\x1B[T",
        BACK_TAB,       "\x1B[Z"
        );
}

