#include <edidentifier.h>
__CIDENT_RCSID(gr_m_terminal_c,"$Id: m_terminal.c,v 1.23 2024/09/25 15:51:54 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_terminal.c,v 1.23 2024/09/25 15:51:54 cvsuser Exp $
 * Terminal screen and keyboard primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_terminal.h"

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "keyboard.h"
#include "lisp.h"
#include "symbol.h"
#include "tty.h"
#include "window.h"
#include "word.h"


struct pt_map {
    int                 ident;                  /* identifier (if any) */
    unsigned            type;                   /* value type */
#define PT_STR                  1   /* string */
#define PT_INT                  2   /* numeric */
#define PT_FLG                  3   /* flag/boolean */

    void *              ptr;                    /* storage pointer */
    size_t              len;                    /* length of storage */
#define PT_MKSTR(x)             PT_STR, (void *)x,  sizeof(x)
#define PT_MKINT(x)             PT_INT, (void *)&x, sizeof(x)
#define PT_MKFLG(x)             PT_FLG, (void *)&x, sizeof(x)

    const char *        desc;                   /* description */
};


static struct pt_map    pt_chars[] = {
    { -1 /*TC_TOP_LEFT*/,       PT_MKSTR(x_pt.pt_top_left),               "top_left" },
    { -1 /*TC_TOP_RIGHT*/,      PT_MKSTR(x_pt.pt_top_right),              "top_ight" },
    { -1 /*TC_BOT_LEFT*/,       PT_MKSTR(x_pt.pt_bot_left),               "bottom_left" },
    { -1 /*TC_BOT_RIGHT*/,      PT_MKSTR(x_pt.pt_bot_right),              "bottom_right" },
    { -1 /*TC_VERTICAL*/,       PT_MKSTR(x_pt.pt_vertical),               "vertical" },
    { -1 /*TC_HORIZONTAL*/,     PT_MKSTR(x_pt.pt_horizontal),             "horizontal" },
    { -1 /*TC_TOP_JOIN*/,       PT_MKSTR(x_pt.pt_top_join),               "top_join" },
    { -1 /*TC_BOT_JOIN*/,       PT_MKSTR(x_pt.pt_bot_join),               "bottom_join" },
    { -1 /*TC_CROSS*/,          PT_MKSTR(x_pt.pt_cross),                  "cross" },
    { -1 /*TC_LEFT_JOIN*/,      PT_MKSTR(x_pt.pt_left_join),              "left_join" },
    { -1 /*TC_RIGHT_JOIN*/,     PT_MKSTR(x_pt.pt_right_join),             "right_join" },
    { -1 /*TC_SCROLL*/,         PT_MKSTR(x_pt.pt_scroll),                 "scroll" },
    { -1 /*TC_THUMB*/,          PT_MKSTR(x_pt.pt_thumb),                  "thumb" },
    { -1,                       0, NULL, 0, NULL }
    };

static struct pt_map    pt_features[] = {
    /*
     *  set_term_features() order specific
     *
     *      o repeat_space -            Sequence, Clear 'n' spaces.
     *      o print_character -         Sequence. Print characters with top bitset.
     *      o insert_cursor -           Sequence, Insert-mode cursor.
     *      o overwrite_cursor -        Sequence, Overwrite-mode cursor.
     *      o virt_insert_cursor -      Sequence, Insert-mode cursor (on virtual space).
     *      o virt_overwrite_cursor -   Sequence, Overwrite-mode cursor (on virtual space).
     *      o print_escape -            Sequence, Print ESCAPE character graphically.
     *      o repeat_last -             Sequence, Repeat last character 'n' times.
     *      o 0m_resets_color -         Boolean,  ESC [0m resets color (defunct).
     *      o color -                   Boolean,  Terminal supports color.
     *      o parm_cursor_right -       Squence,  Move cursor on same line.
     *      o clear_is_black -          Boolean,  ESC[K gives us a black erased line.
     *      o noinsdel -                Boolean,  Allow scrolling (ins/del).
     *      o graphics_mode -           Sequence, Enter graphics mode.
     *      o text_mode -               Sequence, Exit graphics mode.
     *      o reset -                   Sequence, Terminal initialisation.
     *      o init -                    Sequence, Terminal reset.
     *
     *  Most are no longer required since the current generation of curses/ncurses terminfo
     *  databases are complete.  Supplied values shall only be utilised in the event these
     *  are missing from the current terminal description.
     */
    { TF_PRINT_SPACE,           PT_MKSTR(x_pt.pt_repeat_space),           "repeat_space" },
    { TF_PRINT_BITEIGHT,        PT_MKSTR(x_pt.pt_character),              "print_character" },
    { TF_INSERT_CURSOR,         PT_MKSTR(x_pt.pt_icursor),                "insert_cursor" },
    { TF_OVERWRITE_CURSOR,      PT_MKSTR(x_pt.pt_ocursor),                "overwrite_cursor" },
    { TF_VINSERT_CURSOR,        PT_MKSTR(x_pt.pt_vicursor),               "virt_insert_cursor" },
    { TF_VOVERWRITE_CURSOR,     PT_MKSTR(x_pt.pt_vocursor),               "virt_overwrite_cursor" },
    { TF_PRINT_ESCAPE,          PT_MKSTR(x_pt.pt_escape),                 "print_escape" },
    { TF_REPEAT_LAST,           PT_MKSTR(x_pt.pt_repeat_last),            "repeat_last" },
    { TF_0M_RESETS_COLOR,       PT_MKFLG(x_pt.pt_0m),                     "0m_resets_color" },
    { TF_COLOR,                 PT_MKFLG(x_pt.pt_color),                  "color" },
    { TF_CURSOR_RIGHT,          PT_MKSTR(x_pt.pt_cursor_right),           "parm_cursor_right" },
    { TF_CLEAR_IS_BLACK,        PT_MKFLG(x_pt.pt_clrisblack),             "clear_is_black" },
    { TF_DISABLE_INSDEL,        PT_MKFLG(x_pt.pt_noinsdel),               "noinsdel" },
    { TF_GRAPHIC_MODE,          PT_MKSTR(x_pt.pt_graphics_mode),          "graphics_mode" },
    { TF_TEXT_MODE,             PT_MKSTR(x_pt.pt_text_mode),              "text_mode" },
    { TF_RESET,                 PT_MKSTR(x_pt.pt_reset),                  "reset" },
    { TF_INIT,                  PT_MKSTR(x_pt.pt_init),                   "init" },

    /*
     *  non-order specific
     *
     *      o colorset_fgbg -           Sequence, color set control sequence foreground and background.
     *      o colorset_fg -             Sequence, set foreground color.
     *      o colorset_bg -             Sequence, set background color.
     *
     *      o color_depth -             Numeric,  terminals color depth, generally 2, 8, 16, 88 or 256.
     *      o default_fg_color -        Numeric,  terminals default foreground color.
     *      o default_bg_color -        Numeric,  terminals default background color.
     *      o scheme_dark -             Boolean,  *true* if a dark color scheme is desired, otherwise light.
     *
     */
//  { TF_COLORSET_FG,           PT_MKSTR(x_pt.pt_colorsetfg),             "color_set_fg" },
//  { TF_COLORSET_BG,           PT_MKSTR(x_pt.pt_colorsetbg),             "color_set_bg" },
//  { TF_COLORSETRGB_FG,        PT_MKSTR(x_pt.pt_colorsetrgbfg),          "color_setrgb_fg" },
//  { TF_COLORSETRGB_BG,        PT_MKSTR(x_pt.pt_colorsetrgbbg),          "color_setrgb_bg" },

    { TF_COLORDEPTH,            PT_MKINT(x_pt.pt_colordepth),             "color_depth" },
    { TF_DEFAULT_FG,            PT_MKINT(x_pt.pt_defaultfg),              "default_fg_color" },
    { TF_DEFAULT_BG,            PT_MKINT(x_pt.pt_defaultbg),              "default_bg_color" },
    { TF_SCHEMEDARK,            PT_MKFLG(x_pt.pt_schemedark),             "scheme_dark" },
    { TF_COLORRGB,              PT_MKFLG(x_pt.pt_colorrgb),               "color_rgb" },
    { TF_COLORMAP,              PT_MKSTR(x_pt.pt_colormap),               "color_map" },
    { TF_COLORPALETTE,          PT_MKSTR(x_pt.pt_colorpalette),           "color_palette" },
    { TF_COLORSCHEME,           PT_MKSTR(x_pt.pt_colorscheme),            "color_scheme" },

    { TF_EIGHT_BIT,             PT_MKFLG(x_pt.pt_8bit),                   "eight_bit" },
    { TF_MOUSE,                 PT_MKSTR(x_pt.pt_mouse),                  "mouse" },
    { TF_MOUSECLKMS,            PT_MKINT(x_pt.pt_mouseclkms),             "mouse_clickms" },
    { TF_WINDOW_SETSIZE,        PT_MKSTR(x_pt.pt_winsetsize),             "winsetsize" },
    { TF_WINDOW_SETPOS,         PT_MKSTR(x_pt.pt_winsetpos),              "winsetpost" },

    { TF_CODEPAGE,              PT_MKINT(x_pt.pt_codepage),               "codepage" },
    { TF_DISABLE_SCROLL,        PT_MKFLG(x_pt.pt_scroll_disable),         "scroll_disable" },
    { TF_SCROLL_MAX,            PT_MKINT(x_pt.pt_scroll_max),             "scroll_max" },
    { TF_NOALTSCREEN,           PT_MKFLG(x_pt.pt_noaltscreen),            "noaltscreen" },
    { TF_LAZYUPDATE,            PT_MKINT(x_pt.pt_lazyupdate),             "lazy_update" },
    { TF_ATTRIBUTES,            PT_MKINT(x_pt.pt_attributes),             "attributes" },
    { TF_NAME,                  PT_MKSTR(x_pt.pt_name),                   "name" },

    { TF_TTY_FAST,              PT_MKINT(x_pt.pt_tty_fast),               "tty_fast" },
    { TF_TTY_GRAPHICSBOX,       PT_MKINT(x_pt.pt_tty_graphicsbox),        "tty_graphicsbox" },
    { TF_KBPROTOCOL,            PT_MKSTR(x_pt.pt_kbprotocol),             "kbprotocol" },

    { TF_SCREEN_ROWS,           PT_MKINT(x_pt.pt_screen_rows),            "screen_rows" },
    { TF_SCREEN_COLS,           PT_MKINT(x_pt.pt_screen_cols),            "screen_cols" },
    { TF_LINENO_COLUMNS,        PT_MKINT(x_pt.pt_lineno_columns),         "lineno_columns" },
    { TF_WINDOW_MINROWS,        PT_MKINT(x_pt.pt_window_minrows),         "window_minrows" },
    { TF_WINDOW_MINCOLS,        PT_MKINT(x_pt.pt_window_mincols),         "window_mincols" },

    { TF_XTERM_COMPAT,          PT_MKINT(x_pt.pt_xtcompat),               "xterm_compat" },
    { TF_XTERM_CURSOR,          PT_MKINT(x_pt.pt_xtcursor),               "xterm_cursor" },
    { TF_XTERM_TITLE,           PT_MKINT(x_pt.pt_xttitle),                "xterm_title" },
    { TF_XTERM_PALETTE,         PT_MKINT(x_pt.pt_xtpalette),              "xterm_palette" },

    { TF_VT_DATYPE,             PT_MKINT(x_pt.pt_vtdatype),               "vt_datype" },
    { TF_VT_DAVERSION,          PT_MKINT(x_pt.pt_vtdaversion),            "vt_daversion" },
    { TF_VT_DAOPTIONS,          PT_MKINT(x_pt.pt_vtdaoptions),            "vt_daoptions" },

    { TF_ENCODING,              PT_MKSTR(x_pt.pt_encoding),               "encoding" },
    { TF_UNICODE_VERSION,       PT_MKSTR(x_pt.pt_unicode_version),        "unicode_version" },

    { -2,                       0, NULL, 0, "" }
    };

#undef  PT_MKSTR
#undef  PT_MKFLG
#undef  PT_MKINT
#undef  PT_MKCHR

static void             set_list_common(struct pt_map *p, const char *caller);
static void             set_term_assign(struct pt_map *p, const LISTV *result);
static void             get_list_common(const struct pt_map *p);
static void             get_term_retrieve(const struct pt_map *p, int argi);
static void             set_term_key(int key_no, const LISTV *result);


/*  Function:           do_set_term_characters
 *      set_term_characters primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_term_characters - Set terminal special characters.

        int
        set_term_characters(int value0, ....)

    Macro Description:
        The 'set_term_characters()' primitive is one of a set of
        functions which value add to the console interface
        specification. Under Unix(tm) console attributes are derived
        from the system 'termcap' or 'terminfo' databases or similar
        interfaces under non-console and non-unix systems.

        The 'set_term_characters()' primitive configures the set of
        characters which are utilised by the 'tty' console driver to
        represent window borders.

        Each of the character parameters listed below may be either
        an integer or string expression representing the character
        value. An integer value (including character constants) are
        treated as a single character whilst running within
        graphics-mode, whereas a string shall be interpreted as an
        escape sequence. Values can be omitted by supplying a NULL
        parameter against the associated character index.

        This function should generally be invoked prior to the
        display being enabled (see display_windows), otherwise a
        <redraw> is required.

(start table)
    [Index  [Name               [Description                          ]
  ! 0       top_left            Top left of a window.
  ! 1       top_right           Right right of a window.
  ! 2       bottom_left         Bottom left of a window.
  ! 3       bottom_right        Bottom right of a window.
  ! 4       vertical            Vertical window sides.
  ! 5       horizontal          Horizontal window sides.
  ! 5       top_join            Top intersection of window corners.
  ! 6       bottom_join         Bottom intersection of window corners.
  ! 7       cross               Intersection of window corners.
  ! 8       left_join           Left Window intersection.
  ! 9       right_join          Right window intersection.
  ! 10      scroll              Scroll bar arena.
  ! 11      thumb               Scroll bar thumb.
(end table)

    Macro Parameters:
        ... - Integer character value or string escape sequence, one
            for each character within the set.

    Macro Example:

        The following configures a terminal to use simple
        non-graphical characters to represent window borders.

>           set_term_characters(
>               '+',    // 0.  Top left of window.
>               '+',    // 1.  Top right of window.
>               '+',    // 2.  Bottom left of window.
>               '+',    // 3.  Bottom right of window.
>               '|',    // 4.  Vertical bar for window sides.
>               '-',    // 5.  Top and bottom horizontal bar for window.
>               '+',    // 6.  Top join.
>               '+',    // 7.  Bottom join.
>               '+',    // 8.  Window 4-way intersection.
>               '+',    // 9.  Left hand join.
>               '+',    // 10. Right hand join.
>               '*'     // 11. Thumb wheel.
>               );

        Note!:
        For further examples refer to the 'tty' macros, which setup
        the terminal interface for many well known environments.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        set_term_features, set_term_keyboard
 */
void
do_set_term_characters(void)    /* int ([int ident string desc], [string|int] value) */
{
    set_list_common(pt_chars, "term_characters");
}


/*  Function:           do_get_term_feature
 *      set_term_feature primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_term_feature - Set a terminal attribute.

        int
        set_term_feature(
            int|string ident, [string|int value])

    Macro Description:
        The 'set_term_feature()' primitive is one of a set of
        functions which value add to the console interface
        specification. Under Unix(tm) console attributes are derived
        from the system 'termcap' or 'terminfo' databases or similar
        interfaces under non-console and non-unix systems.

        The 'set_term_feature()' primitive allows certain attributes
        about the current display to be modified; for example features
        which are not adequately handled by 'termcap' or 'terminfo'.
        The attribute identified by 'ident' shall be set to 'value'.

    Macro Parameters:
        ident - Either the integer identifier or a string containing
            the name of the terminal attribute to be modified.

        value - Value to be applied against the attribute, if omitted
            or NULL the current value is cleared. The type of the
            value is specific to the attribute being modified, either
            in string or integer form.

    Types:

        Each of the attributes are defined using one of four data
        types.

        Escape  - Terminal specific escape sequence
            being a series of characters that shall be written to the
            attached terminal.

            Controls supporting parameters should use printf style
            '%d' as the place holders for parameters; allowing
            terminfo/termcap independent macro development.

            Most of the sequence attributes are no longer required
            since the current generation of curses/ncurses terminfo
            or terminfo databases are complete. Supplied values shall
            only be utilised in the event these are missing from the
            current terminal description.

        Boolean - Boolean flag, which can be stated as either a
            numeric 'zero/non-zero' value or a string value 'yes/no'.

        Integer - Numeric value, either stated as an integer value
            or string containing a decimal value.

        String  - String value.

    Attributes:

        The following terminal attributes are exposed

(start table)
    [Constant               [Name                   [Type       [Description                            ]

  ! TF_PRINT_SPACE          repeat_space            Escape      Control sequence to print a space
                                                                'n' times.

  ! TF_PRINT_BITEIGHT       print_character         Escape      Control sequence to print eight
                                                                bit characters.

  ! TF_INSERT_CURSOR        insert_cursor           Escape      Control sequence which modifies
                                                                the cursor looks, utilised when
                                                                Grief has insert-mode enabled.

  ! TF_OVERWRITE_CURSOR     overwrite_cursor        Escape      Control sequence which modifies
                                                                the cursor looks, utilised when
                                                                Grief has overwrite-mode enabled.

  ! TF_VINSERT_CURSOR       virt_insert_cursor      Escape      Control sequence which modifies
                                                                the cursor looks, utilised when
                                                                Grief has insert-mode enabled and
                                                                is positioned over a virtual-space.

  ! TF_VOVERWRITE_CURSOR    virt_overwrite_cursor   Escape      Control sequence which modifies
                                                                the cursor looks, utilised when
                                                                Grief has overwrite-mode enabled and
                                                                is positioned over a virtual-space.

  ! TF_PRINT_ESCAPE         print_escape            Escape      Control sequence which prints an
                                                                ESC character graphically.

  ! TF_REPEAT_LAST          repeat_last             Escape      Control sequence for repeating the
                                                                previous character printed 'n'
                                                                times.

  ! TF_0M_RESETS_COLOR      0m_resets_color         Boolean     When non-zero, then whenever an
                                                                ESC[0m is printed, it is assumed
                                                                that the background and
                                                                foreground colours are reset
                                                                (defunct).

  ! TF_COLOR                color                   Boolean     When non-zero, the terminal is
                                                                assumed to support color.

  ! TF_CURSOR_RIGHT         parm_cursor_right       Escape      Control sequence for move the
                                                                cursor 'n' characters within on
                                                                the same line.

  ! TF_CLEAR_IS_BLACK       clear_is_black          Boolean     When *true* states that area
                                                                erase commands set the background
                                                                of the erased line to black,
                                                                otherwise the current background
                                                                is assumed.

  ! TF_DISABLE_INSDEL       noinsdel                Boolean     When *true* disables the use of
                                                                window scrolling when attempting
                                                                to optimise output; on several
                                                                displays its faster to rewrite
                                                                the window.

  ! TF_GRAPHIC_MODE         graphics_mode           Escape      Control sequence to send when
                                                                sending graphics (line drawing)
                                                                characters.

  ! TF_TEXT_MODE            text_mode               Escape      Control sequence to send when
                                                                exiting graphics mode and going
                                                                back to normal character set.

  ! TF_RESET                reset                   Escape      Control sequence to be executed
                                                                during terminal initialisation
                                                                operations.

  ! TF_INIT                 init                    Escape      Control sequence to be executed
                                                                during terminal reset operations.

  ! TF_COLORSETFGBG         colorset_fgbg           Escape      Control sequence when executed
                                                                sets both the foreground and
                                                                backgrounds colours.

  ! TF_COLORSET_FG          colorset_fg             Escape      Control sequence when executed
                                                                sets the foreground colour.

  ! TF_COLORSET_BG          colorset_bg             Escape      Control sequence when executed
                                                                sets the background colour.

  ! TF_COLORDEPTH           color_depth             Integer     Colour depth.

  ! TF_DEFAULT_FG           default_fg_color        Integer     Terminals default foreground color.

  ! TF_DEFAULT_BG           default_bg_color        Integer     Terminals default background color.

  ! TF_SCHEMEDARK           scheme_dark             Boolean     When *true* the dark colour scheme
                                                                variant shall be select over the
                                                                light; if available.

  ! TF_COLORMAP             color_map               String      Xterm colour map specification; a
                                                                comma separated list of colours
                                                                defining the colour palette.

  ! TF_COLORPALETTE         color_palette           String      User defined palette.

  ! TF_EIGHT_BIT            eight_bit               Boolean     When *true* the console supports
                                                                the display of eight bit characters.

  ! TF_MOUSE                mouse                   String      Mouse device.

  ! TF_MOUSECLKMS           mouse_clickms           Integer     Mouse click time in milliseconds.

  ! TF_WINDOW_SETSIZE       winsetsize              Escape      Control sequence to set the window
                                                                size to 'x' by 'y'.

  ! TF_WINDOW_SETPOS        winsetpost              Escape      Control sequence to set the window
                                                                position at 'x' by 'y'.

  ! TF_CODEPAGE             codepage                Integer     Character code page.

  ! TF_DISABLE_SCROLL       scroll_disable          Boolean     When *true* disables use of
                                                                scroll region commands.

  ! TF_SCROLL_MAX           scroll_max              Integer     Upper size of window to be
                                                                scrolled permitting use of scroll
                                                                region commands.

  ! TF_NOALTSCREEN          noaltscreen             Boolean     When *true* denotes no
                                                                alternative screen selection is
                                                                available.

  ! TF_LAZYUPDATE           lazy_update             Integer     When set to a positive value,
                                                                indicates lazy screen updates are
                                                                enabled. Screen updates are
                                                                delayed until the keyboard
                                                                becomes idle upto the stated
                                                                number of lines.

  ! TF_ATTRIBUTES           attributes              Integer     Terminal attribute bitmap;
                                                                see the table below.

  ! TF_NAME                 name                    String      Name of the terminal.

  ! TF_TTY_FAST             tty_fast                Integer     When *true* disables use of pad
                                                                characters.

  ! TF_TTY_GRAPHICSBOX      tty_graphicsbox         Integer     When *true* enables use of graphic
                                                                box characters.

  ! TF_KBPROTOCOL           kbprotocol              String      Active kb-protocol.

  ! TF_SCREEN_ROWS          screen_rows             Integer     Number of screen rows.

  ! TF_SCREEN_COLS          screen_cols             Integer     Number of screen columns.

  ! TF_LINENO_COLUMNS       lineno_columns          Integer     Terminal specific number of
                                                                columns allocated for the display
                                                                of buffer number lines.

  ! TF_WINDOW_MINROWS       window_minrows          Integer     Minimum vertical size of a window.

  ! TF_WINDOW_MINCOLS       window_mincols          Integer     Minimum horizontal size of a window.

  ! TF_XTERM_COMPAT         xterm_compat            Integer     When *true* terminal to treated
                                                                like an xterm.

  ! TF_XTERM_CURSOR         xterm_cursor            Integer     When *true* terminal supports
                                                                xterm cursor control commands.

  ! TF_XTERM_TITLE          xterm_title             Integer     Control sequence when executed
                                                                set the console title.

  ! TF_XTERM_PALETTE        xterm_palette           Integer     Xterm palette selector.

  ! TF_VT_DATYPE            vt_datype               Integer     Terminal type; xterm device
                                                                attribute, known values are.

                                                                    0  - xterm.
                                                                    77 - mintty.
                                                                    83 - screen.
                                                                    82 - rxvt.
                                                                    85 - rxvt unicode.

  ! TF_VT_DAVERSION         vt_daversion            Integer     Terminal version.

  ! TF_VT_DAOPTIONS         vt_daoptions            Integer     Terminal options.

  ! TF_ENCODING             encoding                String      Terminal character encoding.

  ! TF_UNICODE_VERSION      unicode_version         String      UNICODE interface version, for
                                                                example "6.0.1".
(end table)

    TF_ATTRIBUTE:

        The reported *TF_ATTRIBUTE* attribute represents special terminal features mined during
        terminal initialisation. The flag argument can contain none or more of the following
        symbols bitwise OR'ed together.

(start table)
    [Constant               [Description                                ]
  ! TF_AGRAPHICCHARACTERS   Graphic characters (ACS defined)

  ! TF_AFUNCTIONKEYS        F1-F10 function keys.

  ! TF_ACYGWIN              Cygwin native console.

  ! TF_AUTF8ENCODING        UTF8 character encoding, Unicode implied.

  ! TF_AUNICODEENCODING     Unicode character encoding.

  ! TF_AMETAKEY             Meta keys.

  ! TF_AXTERMKEYS           XTerm modifyOtherKeys available.

  ! TF_AKITTYKEYS           Kitty extended keycodes available.
(end table)

        Note!:
        Many of the exposed attributes are specific to the underlying
        terminal, a well known is the 'Xterm Control Sequences'
        available on http://invisible-island.net.

    Macro Returns:
        The 'set_term_feature()' primitive returns 0 on success,
        otherwise 1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_term_features, set_term_characters, set_term_keyboard
 */
void
do_set_term_feature(void)       /* int ([int|string ident], [string|int] value) */
{
    const char *desc = get_xstr(1);
    const int ident = (NULL == desc ? get_xinteger(1, -1) : -1);
    unsigned i;

    for (i = 0; i < (unsigned)(sizeof(pt_features)/sizeof(pt_features[0])); ++i)
        if ((desc && 0 == strcmp(pt_features[i].desc, desc)) ||
                    (ident >= 0 && pt_features[i].ident == ident)) {
            if (!isa_undef(2)) {
                set_term_assign(pt_features + i, margv + 2);
            }
            acc_assign_int(0);
            return;
        }

#if defined(TODO)
#if !defined(USE_VIO_BUFFER)
    if (desc && desc[0] && ':' == desc[1]) {
        /*
         *  [snf]:<tag>=<value>
         */
        const char *tag = desc + 2;
        const char *eq = strchr(tag, '=');

        if (eq) {
            switch (*desc) {
            case 's':           /* string/control sequence */
                ttisetstr(tag, eq - tag, eq + 1);
                return;
            case 'n':           /* numeric value */
                ttisetnum(tag, eq - tag, eq + 1);
                return;
            case 'f':           /* flag value */
                ttisetflag(tag, eq - tag, eq + 1);
                return;
            }
        }
    }
#endif  /*USE_VIO_BUFFER*/
#endif  /*TODO*/

    if (desc) {
        errorf("unknown set_term_feature(%s).", desc);
    } else {
        errorf("unknown set_term_feature(%d).", ident);
    }
    acc_assign_int(1);
}


/*  Function:           do_set_term_features
 *      set_term_features primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_term_features - Define terminal attributes.

        int
        set_term_features(list features)

    Macro Description:
        The 'set_term_features()' primitive is one of a set of
        functions which value add to the console interface
        specification. Under Unix(tm) console attributes are derived
        from the system 'termcap' or 'terminfo' databases or similar
        interfaces under non-console and non-unix systems.

        Similar to the <set_term_feature>, 'set_term_features' allows
        attributes about the current display to be modifed; for
        example features which are not adequately handled by
        'termcap' or 'terminfo'.

        'set_term_features' utilises a list interface with elements
        within the list implied by the TF_XXXX enumeration. This
        permits bulk attribute initialisation at the expense of
        visibility. See <set_term_feature> for further details on
        the exposed attributes.

    Macro Parameters:
        list - Terminal attributes one or each attribute in the order
            implied by the TF_XXXX enumeration.

            Each parameter may be either an integer or string
            expression representing the character value. An integer
            value (including character constants) are treated as a
            single character whilst within graphics-mode, whereas a
            string shall be interpreted as an escape sequence. Values
            can be omitted by supplying a NULL parameter against the
            associated character index.

    Macro Returns:
        nothing

    Macro Example:

>       set_term_features(
>           NULL,           // TF_PRINT_SPACE
>           NULL,           // TF_PRINT_BITEIGHT
>           NULL,           // TF_INSERT_CURSOR
>           NULL,           // TF_OVERWRITE_CURSOR
>           NULL,           // TF_VINSERT_CURSOR
>           NULL,           // TF_VOVERWRITE_CURSOR
>           NULL,           // TF_PRINT_ESCAPE
>           NULL,           // TF_REPEAT_LAST
>           FALSE,          // TF_0M_RESETS_COLOR
>           FALSE,          // TF_COLOR
>           "\x1B[%dC",     // TF_CURSOR_RIGHT
>           TRUE,           // TF_CLEAR_IS_BLACK
>           FALSE,          // TF_DISABLE_INSDEL
>           "\x1B(0",       // TF_GRAPHIC_MODE
>           "\x1B(B"        // TF_TEXT_MODE
>           );

    Macro Portability:
        Many of the return values are only meaningful on systems that
        use a 'tty' based console interface.

    Macro See Also:
        set_term_feature, set_term_characters, set_term_keyboard
 */
void
do_set_term_features(void)      /* void (list) */
{
    set_list_common(pt_features, "term_features");
}


/*  Function:           do_get_term_feature
 *      get_term_feature primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_term_feature - Get value of a terminal feature.

        int
        get_term_feature(
            string | int ident, declare value)

    Macro Description:
        The 'get_term_feature()' primitive retrieves the value
        associated with the specified attribute 'ident'.

    Macro Parameters:
        ident - Either the integer identifier or the a string
            containing the name of the terminal attribute to be
            retrieved.

        value - Variable to populated with the referenced
            attribute.

    Macro Returns:
        The 'set_term_feature()' primitive returns 0 on success,
        otherwise 1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_term_feature
 */
void
do_get_term_feature(void)       /* int (string | int feature, value) */
{
    const char *desc = get_xstr(1);
    const int ident = (NULL == desc ? get_xinteger(1, -1) : -1);
    struct pt_map *p = NULL;
    unsigned i;
                                                /* locate feature */
    for (i = 0; i < (unsigned)(sizeof(pt_features)/sizeof(pt_features[0])); ++i)
        if ((desc && 0 == strcmp(pt_features[i].desc, desc)) ||
                (ident >= 0 && pt_features[i].ident == ident)) {
            p = pt_features + i;
            break;
        }

    if (NULL == p) {                            /* assign */
        if (desc) {
            errorf("unknown get_term_feature(%s).", desc);
        } else {
            errorf("unknown get_term_feature(%d).", ident);
        }
    } else if (!isa_undef(2)) {
        get_term_retrieve(p, 2);
    }

    acc_assign_int(p ? 0 : 1);
}


/*  Function:           do_get_term_features
 *      get_term_features primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_term_features - Retrieve terminate features.

        list
        get_term_features(...)

    Macro Description:
        The 'get_term_features()' primitive retrieves a list of
        string pairs one for each terminal attribute.

    Macro Parameters:
        none

    Macro Returns:
        Without any arguments the 'get_term_features()' primitive
        returns a list of strings, one for each terminal attribute,
        in the form 'name=value'. Otherwise a NULL list is returned.

    Macro Portability:
        Many of the return values are only meaningful on systems that
        use a 'tty' based console interface.

        The CRiSPEdit(tm) version is similar to <get_term_feature>.

    Macro See Also:
        get_term_keyboard, get_term_feature
 */
void
do_get_term_features(void)      /* list ([feature ....]) */
{
    get_list_common(pt_features);
}


/*  Function:           do_get_term_characters
 *      get_term_characters primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_term_characters - Retrieve terminal special characters.

        list
        get_term_characters([top_left], [top_right],
            [bottom_left], [bottom_right], [vertical], [horizontal],
                [top_join], [bottom_join], [cross],
                [left_join], [right_join], [scrol], [thumb])

    Macro Description:
        The 'set_term_characters()' primitive retrieves the set of
        characters which are utilised by the 'tty' console driver to
        represent window borders.

        'set_term_characters()' operators in one of two modes.
        Without arguments a list of strings, one for each character
        is retrieved. Alternatively each parameter is either an
        integer or string variable to be populated with the
        associated character value. Values can be omitted by
        supplying a NULL parameter against the associated character
        index.

        Refer to <set_term_characters> for the order and meaning of
        each of these characters.

    Macro Parameters:
        ... - Integer character value or string escape sequence, one
            for each character within the set.

    Macro Returns:
        Without any arguments the 'get_term_characters' primitive
        returns a list of strings, one for each terminal character,
        in the form 'name=value'. Otherwise a NULL list is returned.

    Macro Portability:
        n/a

    Macro See Also:
        set_term_characters, set_term_feature
 */
void
do_get_term_characters(void)    /* list ([characters ...]) */
{
    get_list_common(pt_chars);
}


/*  Function:           get_list_common
 *      Common handler for set_term_features() and set_term_characters().
 *
 *  Parameters:
 *      p - Feature definition.
 *
 *  returns:
 *      nothing
 */
static void
get_list_common(const struct pt_map *p)
{
    const struct pt_map *tp;
    unsigned i, xattrs = 0, attrs = 0;

    for (tp = p, i = 1; tp->ptr; ++tp, ++i) {   /* arg (1...) */
        if (!isa_undef(i)) {
            get_term_retrieve(tp, i);
            ++xattrs;
        }
        ++attrs;
    }

    if (xattrs) {                               /* extension */
        acc_assign_null();

    } else {                                    /* extension, return list of attribute */
        LIST *newlp;
        const int llen = (attrs * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
        char buffer[128];

        if (NULL != (newlp = lst_alloc(llen, attrs))) {
            LIST *lp = newlp;

            for (tp = p; tp->ptr; ++tp) {
                if (PT_FLG == tp->type || PT_INT == tp->type) {
                    int value;

                    if (sizeof(int) == tp->len) {
                        value = *((int *) tp->ptr);
                    } else {
                        value = *((char *) tp->ptr);
                    }

                    if (PT_FLG == tp->type) {
                        if (-1 == value) {
                            sxprintf(buffer, sizeof(buffer), "%s=default", tp->desc);
                        } else {
                            sxprintf(buffer, sizeof(buffer), "%s=%s", tp->desc, (value ? "yes" : "no"));
                        }
                    } else {
                        sxprintf(buffer, sizeof(buffer), "%s=%d", tp->desc, value);
                    }

                } else if (PT_STR == tp->type) {
                    sxprintf(buffer, sizeof(buffer), "%s=%s", tp->desc, (const char *)tp->ptr);

                } else {
                    sxprintf(buffer, sizeof(buffer), "%s", tp->desc);
                }

                lp = atom_push_str(lp, buffer);
                --attrs;
            }
            assert(0 == attrs);
            atom_push_halt(lp);
            acc_donate_list(newlp, llen);
        }
    }
}


/*  Function:           get_term_retrieve
 *      Retrieve the value of the specified feature.
 *
 *  Parameters:
 *      p - Feature definition.
 *      argi - Argument index.
 *
 *  returns:
 *      nothing
 */
static void
get_term_retrieve(const struct pt_map *p, int argi)
{
    if (PT_FLG == p->type || PT_INT == p->type) {
        if (sizeof(int) == p->len) {
            argv_assign_int(argi, (accint_t) *((int *) p->ptr));
        } else {
            argv_assign_int(argi, (accint_t) *((char *) p->ptr));
        }

    } else if (PT_STR == p->type) {
        argv_assign_str(argi, (const char *)p->ptr);
    }
}


/*  Function:           set_list_common
 *      Common handler for set_term_features() and set_term_characters().
 *
 *  Parameters:
 *      p - Feature definition.
 *      caller - Callers name.
 *
 *  returns:
 *      nothing
 */
static void
set_list_common(struct pt_map *p, const char *caller)
{
    const LIST *nextlp, *lp = get_list(1);

    trace_ilog("set_%s\n", caller);
    if (lp) {
        for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
            LISTV result = {0};

            if (F_ERROR != eval(lp, &result)) {
                set_term_assign(p, &result);
            }
            ++p;
        }
    }
}


/*  Function:           get_term_assign
 *      Assign the value of the specified feature.
 *
 *  Parameters:
 *      p - Feature definition.
 *      result - Result object.
 *
 *  returns:
 *      nothing
 */
static void
set_term_assign(struct pt_map *p, const LISTV *result)
{
    if (result) {
        const char *svalue = NULL;
        accint_t ivalue = 0;

        listv_str(result, &svalue);
        listv_int(result, &ivalue);

        switch (p->type) {
        case PT_FLG:
            if (svalue) {
                if (0 == str_icmp(svalue, "yes") || 0 == str_icmp(svalue, "y")
                        || 0 == str_icmp(svalue, "true")) {
                    ivalue = 1;
                } else if (0 == str_icmp(svalue, "no") || 0 == str_icmp(svalue, "n")
                               || 0 == str_icmp(svalue, "false")) {
                    ivalue = 0;
                } else if (0 == str_icmp(svalue, "default")) {
                    ivalue = -1;
                } else {
                    ivalue = atoi(svalue);
                }
            }

            if (sizeof(int) == p->len) {
                *((int *) p->ptr) = (int) ivalue;
            } else if (sizeof(char) == p->len) {
                *((char *) p->ptr) = (char) ivalue;
            }
            trace_ilog("  %s=%s\n", p->desc, ivalue ? "yes" : "no");
            break;

        case PT_INT:
            if (svalue) {
                ivalue = atoi(svalue);
            }

            if (sizeof(int) == p->len) {
                *((int *) p->ptr) = (int) ivalue;
            } else if (sizeof(char) == p->len) {
                *((char *) p->ptr) = (char) ivalue;
            }
            trace_ilog("  %s=%d\n", p->desc, (int)ivalue);
            break;

        case PT_STR: {
                char *sval = p->ptr;

                if (NULL == svalue) {
                    sval[0] = (char) ivalue;
                    sval[1] = 0;
                } else {
                    ttstringcopy(sval, p->len-1, svalue, '\0');
                }
                trace_ilog("  %s=%s\n", p->desc, sval);
            }
            break;
        }
    }

    /*
     *  Special processing,
     *      note: vtinit0()/vtopen() shall have already been called.
     */
    ttfeature(p->ident);                        /* signal feature change */
}


/*  Function:           do_set_term_keyboard
 *      set_term_keyboard primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_term_keyboard - Define terminal keyboard definitions.

        int
        set_term_keyboard(list kbd)

    Macro Description:
        The 'set_term_keyboard()' primitive is one of a set of
        functions which value add to the console interface
        specification. Under Unix(tm) console attributes are derived
        from the system 'termcap' or 'terminfo' databases or similar
        interfaces under non-unix systems.

        The 'set_term_keyboard' function manages the keyboard
        scan-code (terminal escape sequence) to key-code mapping
        table. For each key definition 'set_term_keyboard' expects a
        pair of arguments, the first being Griefs internal key-code
        (see key_to_int) with the second stating the associated
        escape sequence.

        The mapped escape sequence shall be treated as unique; in
        what only the last instance shall be retained. Whereas
        key-codes can be associated with multiple escape sequences.
        For example, a generic Xterm mapping could support
        alternative function key reports.

        The second argument may takes one of two forms. The first
        being a single string containing the escape sequence, the
        second being a key-list permitting a range of keys to be bound.

        Example simple key associates including non-unique key codes.

>           set_term_keyboard(
>                   :
>
>               KEY_PAGEUP,     "\x1b[5~",  // vt220 mode
>               KEY_PAGEDOWN,   "\x1b[6~",
>               KEY_INS,        "\x1b[2~",
>               KEY_DEL,        "\x7f",
>
>               KEY_PAGEUP,     "\x1b[5z",  // sunFunctionKeys mode
>               KEY_PAGEDOWN,   "\x1b[6z",
>               KEY_INS,        "\x1b[2z",
>
>                   :
>               );

    *Character Ranges*

        The key-lists are simply a list of strings describing each
        consecutive key within a set or range of keys. A constant set
        of keys can be simply quoted using the <quote_list> function,
        alternatively the list can be constructed with the other list
        primitives.

        The following character definitions are generally defined
        using a consecutive set of keys.

            o F1_F12
            o SHIFT_F1_F12
            o CTRL_A_Z
            o CTRL_F1_F12
            o CTRLSHIFT_F1_F12
            o ALT_F1_F12
            o ALT_A_Z
            o ALT_0_9
            o KEYPAD_0_9
            o CTRL_KEYPAD_0_9
            o SHIFT_KEYPAD_0_9
            o ALT_KEYPAD_0_9

        Example of a consecutive definition against a constant set of
        sequences.

>           set_term_keyboard(
>               F1_F12, quote_list(
>                   "\x1bOP",     "\x1bOQ",     "\x1bOR",     "\x1bOS",
>                   "\x1b[15~",   "\x1b[17~",   "\x1b[18~",   "\x1b[19~",
>                   "\x1b[20~",   "\x1b[21~",   "\x1b[23~",   "\x1b[24~" )
>               );

    *Ambiguous*

        Due to the complex nature of terminal escape sequences, many
        key definitions result in ambiguous mappings; for example two
        or more mappings starting with the same sequence of
        characters. The mapping of ambiguous sequences to their
        associated key-code utilise a scan delay mechanism. When an
        ambiguous sequence is detected the driver waits several
        milliseconds before selecting the longest matching sequence,
        see <set_char_timeout> for details.

        Note!:
        Many of the reported escapes are specific to the underlying
        terminal, a well known is the 'Xterm Control Sequences'
        available on http://invisible-island.net.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Example:

        The following is an example from the Linux terminal definition.

>       set_term_keyboard(
>
>               :
>
>           F1_F12,         quote_list(
>               "\x1bOP",     "\x1bOQ",     "\x1bOR",     "\x1bOS",
>               "\x1b[15~",   "\x1b[17~",   "\x1b[18~",   "\x1b[19~",
>               "\x1b[20~",   "\x1b[21~",   "\x1b[23~",   "\x1b[24~"    ),
>
>           SHIFT_F1_F12,   quote_list(
>               NULL,         NULL,         "\x1b[25~",   "\x1b[26~",
>               "\x1b[28~",   "\x1b[29~",   "\x1b[31~",   "\x1b[32~",
>               "\x1b[33~",   "\x1b[34~",   "\x1b[23;2~", "\x1b[24;2~"  ),
>
>           CTRL_F1_F12,    quote_list(
>               "\x1bO5P",    "\x1bO5Q",    "\x1bO5R",    "\x1bO5S",
>               "\x1b[15;5~", "\x1b[17;5~", "\x1b[18;5~", "\x1b[19;5~",
>               "\x1b[20;5~", "\x1b[21;5~", "\x1b[23;5~", "\x1b[24;5~"  ),
>
>               :
>           );

        Note!:
        For further examples refer to the 'tty' macros, which setup
        the terminal interface for many well known environments.

    Macro Portability:
        n/a

    Macro See Also:
        set_term_characters, set_term_features
 */
void
do_set_term_keyboard(void)      /* (list def) */
{
    const LIST *nextlp, *lp = get_list(1);
    LISTV result;
    OPCODE type;
    int keyno;

    while ((nextlp = atom_next(lp)) != lp) {
        const LIST *retlp = NULL;

        if (F_INT != (type = eval(lp, &result))) {
            break;
        }

        lp = nextlp;
        keyno = result.l_int;
        type = eval(lp, &result);
        switch (type) {
        case F_LIST:
            retlp = result.l_list;
            break;
        case F_RLIST:
            retlp = (LIST *) r_ptr(result.l_ref);
            break;
        default:
            set_term_key(keyno, &result);
            break;
        }
        lp = atom_next(lp);

        if (retlp) {
            const LIST *nextretlp;

            while ((nextretlp = atom_next(retlp)) != retlp) {
                const char *str;
                accint_t ival;

                if (NULL != (str = atom_xstr(retlp))) {
                    key_define_key_seq(keyno, str);

                } else if (atom_xint(retlp, &ival)) {
                    char buf[2];

                    buf[0] = (char) ival;
                    buf[1] = '\0';
                    key_define_key_seq(keyno, buf);
                }
                retlp = nextretlp;
                ++keyno;
            }
        }
    }
}


static void
set_term_key(int keyno, const LISTV *result)
{
    const char *sval;
    accint_t ival;

    if (listv_str(result, &sval)) {
        key_define_key_seq(keyno, sval);

    } else if (listv_int(result, &ival)) {
        char buf[2];

        buf[0] = (char) ival;
        buf[1] = '\0';
        key_define_key_seq(keyno, buf);
    }
}


/*  Function:           do_get_term_keyboard
 *      get_term_keyboard primitive.
 *
 *  Parameters:
 *      none
 *
 *  returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_term_keyboard - Retrieve the terminal keyboard definitions.

        list
        get_term_keyboard()

    Macro Description:
        The 'get_term_keyboard()' primitive retrieves the current
        terminal keyboard definition.

        Under Unix(tm) console attributes are derived from the system
        'termcap' or 'terminfo' databases or similar interfaces under
        non-unix systems.

        See 'get_term_keyboard' for further details.

    Macro Parameters:
        none

    Macro Returns:
        The 'get_term_keyboard()' primitive returns a list of key
        binding pairs, each pair consisting of an integer key-code
        plus a string containing the associated escape sequence.

    Macro Portability:
        On systems not utilising a 'tty' based console interface, the
        list shall be empty.

    Macro See Also:
        set_term_features
 */
void
do_get_term_keyboard(void)      /* list () */
{
    SPBLK **array;
    LIST *newlp, *lp;
    int atoms = 0, llen, i;

    if (NULL == (array = key_get_seq_list(&atoms))) {
        acc_assign_null();
        return;
    }

    llen = (atoms * (sizeof_atoms[F_INT] + sizeof_atoms[F_RSTR])) + sizeof_atoms[F_HALT];
    if (0 == atoms || NULL == (newlp = lst_alloc(llen, atoms * 2))) {
        acc_assign_null();
        return;
    }

    for (lp = newlp, i = 0; array[i]; ++i) {
        const keyseq_t *ks = (const keyseq_t *) array[i]->data;

        lp = atom_push_int(lp, ks->ks_code);    /* key code */
        lp = atom_push_str(lp, array[i]->key);  /* escape key */
    }
    atom_push_halt(lp);

    acc_donate_list(newlp, llen);               /* return value */
    chk_free(array);
}

/*end*/





