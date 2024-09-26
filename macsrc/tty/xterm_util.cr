/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: xterm_util.cr,v 1.13 2024/09/20 12:15:16 cvsuser Exp $
 * Standard Xterm features ...
 *
 *
 */

#include "tty.h"
#include "tty_xterm.h"


/*
 *  main ---
 *      Runtime initialisation.  Load the default xterm keyboard labels.
 */
void
main(void)
{
    extern list kbd_labels;

    if (getenv("BKBD") == "" &&
            (! inq_symbol("kbd_labels") || 0 == length_of_list(kbd_labels))) {
        load_macro("kbd/xterm");
    }
}


/*
 *  xterm_util ---
 *      Retrieve package version.
 */
int
xterm_util(void)
{
    return 1;
}


/*
 *  xterm_graphic ---
 *      standard graphic characters.
 */
void
xterm_graphic(void)
{
    int attributes;

    get_term_feature(TF_ATTRIBUTES, attributes);
    if (attributes & TF_AGRAPHICCHARACTERS) {
        return;                                 /* ACS already set (source from termcap/info) */
    }

    set_term_characters(
        "l",        /* Top left of window. */
        "k",        /* Top right of window. */
        "m",        /* Bottom left of window. */
        "j",        /* Bottom right of window. */
        "x",        /* Vertical bar for window sides. */
        "q",        /* Top and bottom horizontal bar for window. */
        "w",        /* Top join. */
        "v",        /* Bottom join. */
        "n",        /* Window 4-way intersection. */
        "u",        /* Left hand join. */
        "t");       /* Right hand join. */
}


/*
 *  xterm_color ---
 *      Color terminal.
 */
void
xterm_color(void)
{                                                                                                                 
    set_term_feature(TF_COLOR, TRUE);           /* Terminal supports color. */
}


/*
 *  xterm_alt8bit_keys ---
 *      Legacy Alt-X key encoding; not-compatible with UTF8 encoding.
 */
void
xterm_alt8bit_keys(void)
{
    set_term_keyboard(
        //
        //  <Alt-A>,        <Alt-B>,        <Alt-C>,        <Alt-D>,        <Alt-E>,
        //  <Alt-F>,        <Alt-G>,        <Alt-H>,        <Alt-I>,        <Alt-J>,
        //  <Alt-K>,        <Alt-L>,        <Alt-M>,        <Alt-N>,        <Alt-O>,
        //  <Alt-P>,        <Alt-Q>,        <Alt-R>,        <Alt-S>,        <Alt-T>,
        //  <Alt-U>,        <Alt-V>,        <Alt-W>,        <Alt-X>,        <Alt-Y>,
        //  <Alt-Z>
        //
        ALT_A_Z, quote_list(                    /* 8bit-Meta-Alpha/remove */
            "\xE1",         "\xE2",         "\xE3",         "\xE4",         "\xE5",
            "\xE6",         "\xE7",         "\xE8",         "\xE9",         "\xEa",
            "\xEb",         "\xEc",         "\xED",         "\xEe",         "\xEf",
            "\xF0",         "\xF1",         "\xF2",         "\xF3",         "\xF4",
            "\xF5",         "\xF6",         "\xF7",         "\xF8",         "\xF9",
            "\xFa"),

        //
        //  <Alt-0>,        <Alt-1>,        <Alt-2>,        <Alt-3>,        <Alt-4>,
        //  <Alt-5>,        <Alt-6>,        <Alt-7>,        <Alt-8>,        <Alt-9>
        //
        ALT_0_9, quote_list(                    /* 8bit-Meta-Numeric/remove */
            "\xB0",         "\xB1",         "\xC0",         "\xA4",         "\xA4",
            "\xB5",         "\xB6",         "\xB7",         "\xB8",         "\xB9" )
        );
}


/*
 *  xterm_altmetas ---
 *      Escape Alt-X key encoding.
 */
void
xterm_altmeta_keys(void)
{
    set_term_keyboard(
        //
        //  <Alt-A>,        <Alt-B>,        <Alt-C>,        <Alt-D>,        <Alt-E>,
        //  <Alt-F>,        <Alt-G>,        <Alt-H>,        <Alt-I>,        <Alt-J>,
        //  <Alt-K>,        <Alt-L>,        <Alt-M>,        <Alt-N>,        <Alt-O>,
        //  <Alt-P>,        <Alt-Q>,        <Alt-R>,        <Alt-S>,        <Alt-T>,
        //  <Alt-U>,        <Alt-V>,        <Alt-W>,        <Alt-X>,        <Alt-Y>,
        //  <Alt-Z>
        //
        ALT_A_Z, quote_list(        /*lower case*/
            "\x1ba",        "\x1bb",        "\x1bc",        "\x1bd",        "\x1be",
            "\x1bf",        "\x1bg",        "\x1bh",        "\x1bi",        "\x1bj",
            "\x1bk",        "\x1bl",        "\x1bm",        "\x1bn",        "\x1bo",
            "\x1bp",        "\x1bq",        "\x1br",        "\x1bs",        "\x1bt",
            "\x1bu",        "\x1bv",        "\x1bw",        "\x1bx",        "\x1by",
            "\x1bz" ),

        ALT_A_Z, quote_list(        /*upper case*/
            "\x1bA",        "\x1bB",        "\x1bC",        "\x1bD",        "\x1bE",
            "\x1bF",        "\x1bG",        "\x1bH",        "\x1bI",        "\x1bJ",
            "\x1bK",        "\x1bL",        "\x1bM",        "\x1bN",        "\x1b0",
            "\x1bP",        "\x1bQ",        "\x1bR",        "\x1bS",        "\x1bT",
            "\x1bU",        "\x1bV",        "\x1bW",        "\x1bX",        "\x1bY",
            "\x1bZ"),

        ALT_0_9, quote_list(
            "\x1b0",        "\x1b1",        "\x1b2",        "\x1b3",        "\x1b4",
            "\x1b5",        "\x1b6",        "\x1b7",        "\x1b8",        "\x1b9"),

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
        __ALT('{'),         "\x1b{",
    //  __ALT('['),         "\x1b[",            /* break other keys */
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
        __ALT('\x09'),      "\x1b\x0d"
        );
}


/*
 *  xterm_256color ---
 *      Color terminal.
 *
 *  Usage:
 *      GRTERM=xterm-256color
 */
void
xterm_256color(void)
{
    set_term_feature(TF_COLOR, TRUE);           /* Terminal supports color. */
    set_term_feature(TF_COLORDEPTH, 256);       /* using a colour depth of 256. */
}


/*
 *  xterm_88color
 *      Color terminal.
 *
 *  Usage:
 *      GRTERM=xterm-88colour
 */
void
xterm_88color(void)
{
    set_term_feature(TF_COLOR, TRUE);           /* terminal supports color. */
    set_term_feature(TF_COLORDEPTH, 88);        /* using a colour depth of 88. */
}


/*
 *  xterm_pccolours ---
 *      Enforce a standard (PC style) colour map.
 *
 *  Notes:
 *      see /usr/lib/X11/rbg.txt for a list of valid color name
 */
void
xterm_pccolours(void)
{
    set_term_feature(TF_COLORMAP,               /* color map */
        "black,red3,green3,yellow3,blue3,magenta3,cyan3,gray90,gray30,red,green,yellow,blue,magenta,cyan,white");
}


/*
 *  xterm_pccolors
 *      Color terminal attribute.
 *
 *  Usage:
 *      GRTERM=xterm-pccolors
 */
void
xterm_pccolors(void)
{
    xterm_pccolours();
}


/*
 *  xterm_unicode
 *      Unicode terminal.
 *
 *  Usage:
 *      GRTERM=xterm-unicode
 */
void
xterm_unicode(void)
{
    //TODO
    //set_term_feature(TF_UNICODE_VERSION, "15.0.0");
}


/*
 *  xterm_mono ---
 *      Mono terminal.
 *
 *  Usage:
 *      GRTERM=xterm-mono
 */
void
xterm_mono(void)
{
    set_term_feature(TF_COLOR, FALSE);          /* Terminal supports color. */
}


/*
 *  xterm_standard ---
 *      Standard overrides.
 */
void
xterm_standard(void)
{
    set_term_feature(TF_PRINT_SPACE, "\x1b[%dX");
    set_term_feature(TF_REPEAT_LAST, "\x1b[%db");
    set_term_feature(TF_CURSOR_RIGHT, "\x1b[%dC");
    set_term_feature(TF_CLEAR_IS_BLACK, TRUE);
    set_term_feature(TF_GRAPHIC_MODE, "\x1b(0");
    set_term_feature(TF_TEXT_MODE, "\x1b(B");
    set_term_feature(TF_INIT, "\x1b[?47h[0m");
    set_term_feature(TF_RESET, "\x1b[?47l[0m");
    set_term_feature(TF_DISABLE_INSDEL, TRUE);
}


/*
 *  xterm_arrow ---
 *      Macro to handle xterm running under Sun's OpenWindows product.
 *
 *  Usage:
 *      GRTERM=xterm-arrow
 */
void
xterm_arrow(void)
{
    set_term_keyboard(
        KEYPAD_0_9, quote_list(
            "\x1b[212z",    "\x1b[220z",    "\x1b[B",       "\x1b[222z",
            "\x1b[D",       NULL,           "\x1b[C",       "\x1b[214z",
            "\x1b[A" )
        );
}


/*
 *  openwin ---
 *      Macro to handle xterm running under Sun's OpenWindows product.
 *
 *  Usage:
 *      GRTERM=xterm-openwin
 */
void
openwin(void)
{
    set_term_keyboard(
        SHIFT_F1_F12, quote_list(
            "\x1b[224z",    "\x1b[225z",    "\x1b[226z",    "\x1b[227z",
            "\x1b[228z",    "\x1b[229z",    "\x1b[230z",    "\x1b[231z",
            "\x1b[232z"),

        ALT_A_Z,  quote_list(
            "\xe1",         "\xe2",         "\xe3",         "\xe4",         "\xe5",
            "\xe6",         "\xe7",         "\xe8",         "\xe9",         "\xea",
            "\xeb",         "\xec",         "\xed",         "\xee",         "\xef",
            "\xf0",         "\xf1",         "\xf2",         "\xf3",         "\xf4",
            "\xf5",         "\xf6",         "\xf7",         "\xf8",         "\xf9",
            "\xfa")
        );
}


/*
 *  sunview ---
 *      Sunview function keys.
 *
 *  Usage:
 *      GRTERM=xterm-sunview
 */
void
sunview(void)
{
    set_term_keyboard(
        F1_F12, quote_list(
            "\x1b[224z",    "\x1b[225z",    "\x1b[226z",    "\x1b[227z",
            "\x1b[228z",    "\x1b[229z",    "\x1b[230z",    "\x1b[231z",
            "\x1b[232z",    "\x1b[233z"),

        KEYPAD_0_9, quote_list(
            "\x1b[2z",      "\x1b[220z",    "\x1bOB",       "\x1b[222z",
            "\x1bOD",       "\x1b[218z",    "\x1bOC",       "\x1b[214z",
            "\x1bOA",       "\x1b[216z",    "\x1b[P",       "\x1bOk",
            "\x1bOm",       "\x1b[213z"),

        CTRL_KEYPAD_0_9, quote_list(
            NULL,           "\x1b[220C",    "\x1b[221C",    "\x1b[222C",
            "\x1b[217C",    "\x1b[218C",    "\x1b[219C",    "\x1b[214C",
            "\x1b[215C",    "\x1b[216C")
    );
}


/*
 *  vga ---
 *      Set characters used for extended graphics support when drawing windows.
 *
 *  Usage:
 *      GRTERM=xterm-vga
 */
void
vga(void)
{
    set_term_characters(
        213,        /* Top left of window. */
        184,        /* Top right of window. */
        212,        /* Bottom left of window. */
        190,        /* Bottom right of window. */
        179,        /* Vertical bar for window sides. */
        205,        /* Top and bottom horizontal bar for window. */
        0xd1,       /* Top join. */
        0xcf,       /* Bottom join. */
        0xd8,       /* Window 4-way intersection. */
        0xb5,       /* Left hand join. */
        0xc6);      /* Right hand join. */
}


/*  Function:       xterm_mouse
 *      Macro called when a mouse button is hit from an Xterm when the mouse option is
 *      enabled in the window.
 *
 *  Description:
 *      Normal tracking mode sends an escape sequence on both button press and release.
 *      Modifier information is also sent.
 *
 *      It is enabled by sending ESC[?1000h and disabled with ESC[?1000l.
 *
 *      On button press or release, xterm sends "ESC[Mbxy".
 *
 *          The low two bits of b encode button information:
 *
 *              0=MB1 pressed, 1=MB2 pressed, 2=MB3 pressed, 3=release.
 *
 *          The upper bits encode what modifiers were down when the button
 *          was pressed and are added together:
 *
 *              4=Shift, 8=Meta, 16=Control.
 *
 *          xy represent the x and y coordinates of the mouse event. The
 *          upper left corner is (1,1).
 *
 *  Parameters:
 *      none, macro expected to read control sequence directly using read_char().
 *
 *  Returns:
 *      nothing
 */

static int          mouse_cnt = 0;

void
xterm_mouse(void)
{
    int ch1 = read_char(0);
    int ch2 = read_char(0);
    int ch3 = read_char(0);

    message("Mouse hit %d! %s%s%s (%d,%d)", mouse_cnt++,
        ch1 == ' ' ? "o" : "-", ch1 == '!' ? "o" : "-",  ch1 == '"' ? "o" : "-",
        ch2 - ' ', ch3 - ' ');
    process_mouse(ch1 == ' ', ch1 == '!', ch1 == '"', ch2 - ' ' - 1,  ch3 - ' ' - 1 );
}

/*end*/

