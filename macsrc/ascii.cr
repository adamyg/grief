/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: ascii.cr,v 1.13 2014/10/27 23:28:16 ayoung Exp $
 * Display an ASCII and UNICODE table display.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
extern void                 ascii_128(void);
extern void                 ascii_256(void);
extern void                 ascii_chart(int radix);
extern void                 unicode_chart(int radix);
static void                 chart(int radix, int unicode);
#endif

#ifdef MSDOS
static int                  ascii_mode = 0x100;
#else
static int                  ascii_mode = 0x80;
#endif
static int                  c0_mode = 1;
static int                  c1_mode = 1;

void
ascii()
{
    int unicode = (DC_UNICODE & inq_display_mode());
    int oascii_mode = ascii_mode;
    int oc1_mode = c1_mode;
    int oc0_mode = c0_mode;
    string option;
    int idx = 0;
    int radix;

    while (get_parm(idx, option, NULL)) {
        if ("/unicode" == option) {
            unicode = 1;                        // use with caution
        } else if ("/256" == option) {
            ascii_mode = 256;
        } else if ("/128" == option) {
            ascii_mode = 128;
        } else if ("/c1" == option) {
            c1_mode = 0;
        } else if ("/c0" == option) {
            c0_mode = 0;                        // use with caution
        } else {
            option = "";
            break;
        }
        ++idx;
    }

    if (get_parm(idx, radix, NULL) &&
            (radix == 8 || radix == 10 || radix == 16)) {
        ascii_chart(radix);

    } else {
        if (unicode) {
            select_list(
                "Character Charts", "", 3,
                quote_list(
                    "Hex Chart", "ascii_chart 16", "ASCII Chart",
                    "Octal Chart", "ascii_chart 8", "ASCII Chart",
                    "Decimal Chart",  "ascii_chart 10", "ASCII Chart",
                    "Unicode Hex Chart", "unicode_chart 16", "Unicode Chart",
                    "Unicode Octal Chart", "unicode_chart 8",  "Unicode Chart",
                    "Unicode Decimal Chart", "unicode_chart 10", "Unicode Chart"),
                SEL_CENTER);

        } else {
            select_list(
                "ASCII Chart", "", 3,
                quote_list(
                    "Hex Chart", "ascii_chart 16", "ASCII Chart",
                    "Octal Chart", "ascii_chart 8", "ASCII Chart",
                    "Decimal Chart", "ascii_chart 10", "ASCII Chart"),
                SEL_CENTER);
        }
    }

    ascii_mode = oascii_mode;
    c1_mode = oc1_mode;
    c0_mode = oc0_mode;
}


void
ascii_128(void)
{
    ascii_mode = 0x80;
}


void
ascii_256(void)
{
    ascii_mode = 0x100;
}


void
ascii_chart(int radix)
{
    chart(radix, FALSE);
}


void
unicode_chart(int radix)
{
    chart(radix, TRUE);
}


static void
chart(int radix, int unicode)
{
    int     buf, win, curbuf, curwin, len, ch;
    string  name, fmt;
    list    c0, c1;

    curbuf = inq_buffer();
    curwin = inq_window();

    if (unicode) {
        switch (radix) {
        case 8:
            fmt = " %06o ";
            len = 13;
            break;
        case 10:
            fmt = " %6d ";
            len = 13;
            break;
        default:
            radix = 16;
            fmt = " %04x ";
            len = 11;
            break;
        }
        sprintf(name, "UNICODE Chart (%s)",
            (8 == radix  ? "Oct" : 10 == radix ? "Decimal" : "Hex"));

    } else {
        fmt = (8 == radix ? " %03o " : 10 == radix ? " %3d " : " %02x ");
        sprintf(name, "ASCII Chart (%s)",
            (8 == radix  ? "Oct" : 10 == radix ? "Decimal" : "Hex"));
    }

    buf = create_buffer(name, NULL, 1);
    set_buffer(buf);

    if (unicode) {
        message("building chart (radix: %d) ...", radix);
        set_buffer_type(NULL, BFTYP_UTF8);
        ascii_mode = 0x100;
    }

    c0 = quote_list(                            // C0 (0x01 - 0x20)
            "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
            "BS ", "HT ", "NL ", "VT ", "NP ", "CR ", "SO ", "SI ",
            "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
            "CAN", "EM ", "SUB", "ESC", "FS ", "GS ", "RS ", "US ");

    c1 = quote_list(                            // DEL plus C1 (0x80 - 0x9f)
            "DEL",
            "PAD", "HOP", "BPH", "NBH", "IND", "NEL", "SSA", "ESA",
            "HTS", "HTJ", "VTS", "PLD", "PLU", "RI ", "SS2", "SS3",
            "DCS", "PU1", "PU2", "STS", "CCH", "MW ", "SPA", "EPA",
            "SOS", "SGC", "SCI", "CSI", "ST ", "OSC", "PM ", "APC");

    while (ch < ascii_mode) {
        if (0x00 == ch) {                       // c0
            while ((ch < 0x20 && c1_mode) || (ch < 0x01 && !c0_mode)) {
                insertf(fmt, ch);
                insert(c0[ch]);
                if (7 == (ch & 7)) {
                    insert("\n");
                } else {
                    insert(unicode ? " |" : "|");
                }
                ++ch;
            }

        } else if (0x7f == ch && c1_mode) {     // DEL plus c1
            while (ch < 0xa0 && ch < ascii_mode) {
                insertf(fmt, ch);
                insert(c1[ch - 0x7f]);
                if (7 == (ch & 7)) {
                    insert("\n");
                } else {
                    insert(unicode ? " |" : "|");
                }
                ++ch;
            }

        } else {                                // others
            if ('%' == ch) {
                insertf(fmt + " %%", ch);
            } else {
                insertf(fmt + " ", ch);
                insert(ch);
            }
            if (7 == (ch & 7)) {
                insert("\n");
            } else {
                insert(unicode ? "  |" : " |");
            }
            ++ch;
        }
    }

    if (unicode) {
        //
        //  special and undefined character ranges ...
        //
        const list ranges = {
            // Last    Next
                0x0300, 0x036f,                 // Combining diacritical marks
                0x0750, 0x077f,                 // Undefined
                0x07c0, 0x08ff,                 // Undefined
                0x1380, 0x139f,                 // Undefined
                0x18B0, 0x18ff,                 // Undefined
                0x1980, 0x1cff,                 // Undefined
                0x1d80, 0x1dff,                 // Undefined
            //  0x2c00, 0x2e7f,                 // Undefined, UNICODE 6.0 available
                0x2fe0, 0x2fef,                 // Undefined
                0x31c0, 0x31ef,                 // Undefined
                0x9fb0, 0x9fff,                 // Undefined
                0xa4d0, 0xabff,                 // Undefined
                0xd7b0, 0xd7ff,                 // Undefined
            //  0xd800, 0xdbff,                 // Surrogates, high
            //  0xdc00, 0xdfff,                 // Surrogates, low
            //  0xe000, 0xf8ff,                 // Private
                0xd800, -0xf8ff,                // == omitted
                0xfdd0, 0xfdef,                 // Reserved
                0xfe10, 0xfe1f,                 // Undefined
                0xfffd, 0xffff,                 // Completion
                -1
                };
        int idx = 0, last;

        while (1) {
            if ((last = ranges[idx++]) <= 0) {  // last within range
                break;
            }

            while (ch < last) {                 // export characters
                insertf(fmt + " ", ch);
                insert(ch);
                if (7 == (ch & 7)) {
                    insert("\n");
                } else {
                    move_abs(NULL, ((ch & 7) + 1) * len);
                    insert("|");
                }
                ++ch;
            }

            if ((last = ranges[idx++]) < 0) {   // next range, omit or pad
                insertf("\n%*s***" + fmt + " - " + fmt + "***\n\n", len * 3, "", ch, last * -1);
                ch = (last * -1) + 1;
            } else {
                while (ch <= last) {            // pad area
                    insertf(fmt, ch);
                    if (7 == (ch & 7)) {
                        insert("\n");
                    } else {
                        insert("    |");
                    }
                    ++ch;
                }
            }
        }
    }

    win = sized_window(inq_lines(), inq_line_length() + 1);
    select_buffer(buf, win, SEL_NORMAL);
    delete_buffer(buf);
    set_buffer(curbuf);
    set_window(curwin);
    attach_buffer(curbuf);
    redraw();                                   // correct any corruption
}

/*eof*/
