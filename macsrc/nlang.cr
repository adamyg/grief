/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: nlang.cr,v 1.5 2021/07/11 08:26:12 cvsuser Exp $
 * Macro to allow insertion of national language chars
 *
 *
 */

#include "grief.h"

static list greek =
    {
        0xe0,                       /* alpha - a */
        0xe1                        /* beta - b */
    };

static list graves =
    {
        0x85,                       /* A */
        0x85,                       /* a */
        0x8a,                       /* e */
        0x95,                       /* o */
        0x97                        /* u */
    };

static list acutes =
    {
        0xa0,                       /* A */
        0xa0,                       /* a */
        0x82,                       /* e */
        0xa2,                       /* o */
        0xa3                        /* u */
    };

static list circs =
    {
        0x83,                       /* A */
        0x83,                       /* a */
        0x88,                       /* e */
        0x93,                       /* o */
        0x96                        /* u */
    };

static list umlauts =
    {
        0x8e,                       /* A */
        0x84,                       /* a */
        0x88,                       /* e */
        0x94,                       /* o */
        0x81                        /* u */
    };


/*
 *  Macro to allow insertion of national language characters. Characters are those
 *  compatible with the 8-bit PC character set
 */
void
nlang()                             /*MCHAR???*/
{
    string letters = "Aaeou", ch;
    int accent, letter, i;

    message("Type '[%s] [acgu]', 'g[letter]' (Greek) or x[hexchar].", letters);

    /* Read mode selection charcater */
    while ((letter = read_char()) == -1)
        ;
    if (letter == key_to_int("<Esc>")) {
        message("esc");
        return;
    }

    if (letter == 'x') {
        int hexval = 0, value;
        int done = 0;

        while (1) {
            message("hexval=0x%x", hexval);
            while ((letter = read_char()) == -1)
                ;
            if (letter == key_to_int("<Esc>")) {
                message("esc");
                return;
            }

            if (letter == key_to_int("<Return>")) {
                done = 1;
                break;
            }

            sprintf(ch, "%c", letter);
            if (!isxdigit(letter) || 
                    1 != sscanf(ch, "%x", value)) {
                break;
            }

            hexval <<= 4;
            hexval += value;
            if (hexval > 0x10ffff) {
                done = 1;
                break;
            }
        }

        if (done) {
            message("hexval=0x%x, done", hexval);
            insert(hexval);
            return;
        }

        error("nlang - invalid hexval");
        return;
    }

    /* Accent selection */
    while ((accent = read_char()) == -1)
        ;
    if (accent == key_to_int("<Esc>")) {
        message("esc");
        return;
    }

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
        insert(0xae);
        message("");
        return;

    } else if (letter == 'c' && accent == '"') {
        insert(0xaf);
        message("");
        return;

    } else if (letter == 'c' && accent == 'c') {
        /* Handle C-cedilla. */
        insert(0x87);
        message("");
        return;

    } else if (letter == 'n' && accent == 'n') {
        /* Handle Spanish n with a twiddle on top. */
        insert(0xa4);
        message("");
        return;
    }

    sprintf(ch, "%c", letter);
    if ((i = index(letters, ch) - 1) < 0) {
        error("nlang - unknown character sequence.");
        return;
    }

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
    default:
        error("nlang - unknown character sequence.");
        return;
    }
    message("");
}

/*end*/

