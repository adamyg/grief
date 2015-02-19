/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: telnet.cr,v 1.5 2014/10/22 02:34:41 ayoung Exp $
 * This is the terminal description file for NCSA telnet emulating a vt102. This is a
 * very useful program, but some of the keyboard mappings are decidely odd, and
 * probably differ from a "real" vt102, though I don't have one available for comparison
 *
 * Add the following line to /etc/termcap:
 *     telnet|NCSA telnet emulating vt102:tc=vt102:
 * (or use crisp/utils/termcap [P Fox])
 * Telnet should be configured to send ^H rather than ^?
 * for the backspace key.  Include the following line
 * in CONFIG.TEL:
 *     erase=backspace
 *
 * Unfortunately, the <Home> key also sends ^H, so it cannot be independently
 * recognized, and will behave like another backspace key. But if the backspace key
 * sends ^?, it doesn't seem to work properly on the command line (though it works
 * elsewhere). This configuration is not ideal, but seems to be the best compromise The
 * standard version of NCSA telnet sends the string "\x1Aexit\r\n" when <F9> is
 * pressed. This may be useful under VMS, but its disasterous on a Unix system -- if
 * you have job control, it will suspend the editor and then try to log you out. I've
 * patched our telnet binary to send the string "\x1BWombat" instead. The key map
 * defined here reflects that patch. It will still work for the unpatched version --
 * just don't touch <F9>, whatever you do
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_NAME, "telnet");

    set_term_characters(
        "\x1B(0l\x1B(B",        /* Top left of window. */
        "\x1B(0k\x1B(B",        /* Top right of window. */
        "\x1B(0m\x1B(B",        /* Bottom left of window. */
        "\x1B(0j\x1B(B",        /* Bottom right of window. */
        "\x1B(0x\x1B(B",        /* Vertical bar for window sides. */
        "\x1B(0q\x1B(B",        /* Top and bottom horizontal bar for window. */
        '+',                    /* Top join. */
        '+',                    /* Bottom join. */
        '+',                    /* Window 4-way intersection. */
        '+',                    /* Left hand join. */
        '+'                     /* Right hand join. */
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
            "\x1BOP",       "\x1BOQ",       "\x1BOR",       "\x1BOS",       "\x1BOm",
            "\x1BOl",       NULL,           "\x1BOv",       NULL,           "\x1BOM"),

        ALT_A_Z, quote_list(
            "\x1Ba",        "\x1Bb",        "\x1Bc",        "\x1Bd",        "\x1Be",
            "\x1Bf",        "\x1Bg",        "\x1Bh",        "\x1Bi",        "\x1Bj",
            "\x1Bk",        "\x1Bl",        "\x1Bm",        "\x1Bn",        "\x1Bo",
            "\x1Bp",        "\x1Bq",        "\x1Br",        "\x1Bs",        "\x1Bt",
            "\x1Bu",        "\x1Bv",        "\x1Bw",        "\x1Bx",        "\x1By",
            "\x1Bz"),

        ALT_0_9, quote_list(
            "\x1B0",        "\x1B1",        "\x1B2",        "\x1B3",        "\x1B4",
            "\x1B5",        "\x1B6",        "\x1B7",        "\x1B8",        "\x1B9"),

        KEYPAD_0_9, quote_list(
            NULL,               /* Ins */
            "\x1BOt\x1BOr",     /* End */
            "\x1BOB",           /* Down Arrow */
            "\x1BOt\x1BOx",     /* PgDn */
            "\x1BOD",           /* Left Arrow */
            NULL,               /* 5 (Null) */
            "\x1BOC",           /* Right Arrow */
            NULL,               /* Dont recognize Home key */
            "\x1BOA",           /* Up Arrow */
            "\x1BOu\x1BOx",     /* PgUp */
            "\x1BOn"            /* Del */
            ),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,
            NULL,
            NULL,
            "\x1BOt\x1BOw",     /* Ctrl-PgDn */
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            "\x1BOu\x1BOw"      /* Ctrl-PgUp */
            )
        );

    assign_to_key("#127", "backspace");
}

