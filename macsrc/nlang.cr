/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: nlang.cr,v 1.4 2014/10/22 02:34:21 ayoung Exp $
 * Macro to allow insertion of national language chars
 * XXX - needs work
 *
 *
 */

#include "grief.h"

static list greek =
    {
        "\xe0",                     /* alpha - a */
        "\xe1"                      /* beta - b */
    };

static list graves =
    {
        "\x85",                     /* A */
        "\x85",                     /* a */
        "\x8a",                     /* e */
        "\x95",                     /* o */
        "\x97"                      /* u */
    };

static list acutes =
    {
        "\xa0",                     /* A */
        "\xa0",                     /* a */
        "\x82",                     /* e */
        "\xa2",                     /* o */
        "\xa3"                      /* u */
    };

static list circs =
    {
        "\x83",                     /* A */
        "\x83",                     /* a */
        "\x88",                     /* e */
        "\x93",                     /* o */
        "\x96"                      /* u */
    };

static list umlauts =
    {
        "\x8e",                     /* A */
        "\x84",                     /* a */
        "\x88",                     /* e */
        "\x94",                     /* o */
        "\x81"                      /* u */
    };


/*
 *  Macro to allow insertion of national language characters. Characters are those
 *  compatible with the 8-bit PC character set
 */
void
nlang()                             /*MCHAR???*/
{
    string letters = "Aaeou";
    int accent, letter, i;
    string ch;

    message("Type '[%s] [acgu]' or 'g[letter]' (Greek).", letters);

    /* Read in two characters and allow <Esc> to abort selection. */
    while ((letter = read_char()) == -1)
        ;

    if (letter == key_to_int("<Esc>"))
        return;

    while ((accent = read_char()) == -1)
        ;

    if (accent == key_to_int("<Esc>"))
        return;

    /* Handle Greek alphabet. */
    if (letter == 'g') {
        insert(greek[accent - 'a']);
        message("");
        return;
    }

    /* Convert control characters to lower case. */
    if (letter < ' ')
        letter += 0x60;
    if (accent < ' ')
        accent += 0x60;

    /* o" and c" are open and close double angle brackets (<< and >>) */
    if (letter == 'o' && accent == '"') {
        insert("\xae");
        message("");
        return;

    } else if (letter == 'c' && accent == '"') {
        insert("\xaf");
        message("");
        return;

    } else if (letter == 'c' && accent == 'c') {
        /* Handle C-cedilla. */
        insert("\x87");
        message("");
        return;

    } else if (letter == 'n' && accent == 'n') {
        /* Handle Spanish n with a twiddle on top. */
        insert("\xa4");
        message("");
        return;
    }

    sprintf(ch, "%c", letter);
    i = index(letters, ch) - 1;
    if (i < 0)
        return;

    switch (accent) {
    case key_to_int("a"):
        insert(acutes[i]);
        break;
    case key_to_int("c"):
        insert(circs[i]);
        break;
    case key_to_int("g"):
        insert(graves[i]);
        break;
    case key_to_int("u"):
        insert(umlauts[i]);
        break;
    default: {
            error("Unknown character sequence.");
            return;
        }
    }
    message("");
}

/*end*/
