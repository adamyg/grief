#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyncurses_c,"$Id: ttyncurses.c,v 1.37 2024/10/29 05:21:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyncurses.c,v 1.37 2024/10/29 05:21:04 cvsuser Exp $
 * [n]curses tty driver interface -- alt driver when running under ncurses.
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

#if (!defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBNCURSESW)) || defined(HAVE_LIBTINFO)

#include "tty.h"


/*  Function:           ttcurses
 *      curses(3CURSES)/ncurses(3NCURSES) driver run-time initialisation.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      none.
 */
void
ttcurses(void)
{
    /*IGNORE*/
}

#else   /*libncurses*/

#include <edtermio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED                  /* cchar_t and widechar */
#endif

#if defined(HAVE_LIBNCURSESW)
#if defined(HAVE_NCURSESW_CURSES_H)
#   include <ncursesw/curses.h>
#   include <ncursesw/termcap.h>
#   include <ncursesw/term.h>
#elif defined(HAVE_NCURSESW_H)
#   include <ncursesw.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#elif defined(HAVE_NCURSES_CURSES_H)
#   include <ncurses/curses.h>
#   include <ncurses/termcap.h>
#   include <ncurses/term.h>
#elif defined(HAVE_NCURSES_H)
#   include <ncurses.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#else  /*!HAVE_NCURSESW_CURSES_H || HAVE_NCURSESW_H*/
#error "HAVE_LIBNCURSEW defined yet missing headers, check config"
#endif

#elif defined(HAVE_LIBNCURSES)
#if defined(HAVE_NCURSES_CURSES_H)
#   include <ncurses/curses.h>
#   include <ncurses/termcap.h>
#   include <ncurses/term.h>
#elif defined(HAVE_NCURSES_H)
#   include <ncurses.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#else  /*!HAVE_NCURSES_CURSES_H || HAVE_NCURSES_H*/
#error "HAVE_LIBNCURSE defined yet missing headers, check config"
#endif
#endif

#include "cmap.h"
#include "color.h"
#include "debug.h"                              /* trace_...() */
#include "display.h"
#include "getkey.h"
#include "keyboard.h"
#include "main.h"
#include "system.h"                             /* sys_...() */
#include "tty.h"
#include "ttyutil.h"
#include "ttycfg.h"
#include "ttyrgb.h"
#include "window.h"

#if defined(NCURSES_CONST)
#define CURSES_CAST(__x) (NCURSES_CONST char *)(__x)
#else
#define CURSES_CAST(__x) (char *)(__x)
#endif

#if defined(HAVE_LIBNCURSESW) && defined(NCURSES_WIDECHAR)
#define CURSES_WIDECHAR                         /* wide character support enabled (configure options) and available */
#endif

#if ((NCURSES_VERSION_MAJOR > 6) || ((NCURSES_VERSION_MAJOR == 6) && (NCURSES_VERSION_MINOR >= 1))) && \
            defined(NCURSES_EXT_COLORS)

#define CURSES_DIRECT_COLORS 1                  /* ABI6: direct/truecolor support */
#define CURSES_EXTENDED_COLOR 1                 /* enable extended api */

#define DIRECT_COLORS   16777216                /* direct colors/(0xffffff + 1) */
#define DIRECT_PAIRS    65536                   /* direct pairs */

#define MaxSCALE        1000                    /* input curses ranges 0..1000 */
#define MaxRGB          255                     /* output color ranges 0..255 */
#define ToNCurses(n)    ((NCURSES_COLOR_T)(((n) * MaxSCALE) / MaxRGB))
#endif

#if defined(CURSES_EXTENDED_COLOR)
typedef int nccolor_t;
#else
typedef short nccolor_t;
#endif

#if (NCURSES_VERSION_MAJOR >= 5) && defined(CURSES_WIDECHAR)
extern int _nc_unicode_locale(void);            /* private/exported */
#endif

static void             nc_init(int *argcp, char **argv);
static void             nc_open(scrprofile_t *profile);
static void             nc_ready(int repaint, scrprofile_t *profile);
static void             nc_keybind(void);
static void             nc_display(void);
static void             nc_feature(int ident, scrprofile_t *profile);
static int              nc_control(int action, int param, ...);
static void             nc_winch(int *nrow, int *ncol);
static void             nc_close(void);

static int              nc_cursor(int visible, int imode, int virtual_space);
static void             nc_move(int row, int col);

static void             nc_clear(void);
static void             nc_print(int row, int col, int len, const struct _VCELL *vvp);
static void             nc_putc(const struct _VCELL *cell);
static int              nc_eeol(void);
static void             nc_flush(void);
static int              nc_names(const char *title, const char *icon);
static void             nc_beep(int freq, int duration);

static void             term_identification(void);
static int              istruecolor(void);
static void             term_defaultscheme(void);
static void             term_dump(void);
static void             acs_dump(const char *bp);

static void             term_colors(void);
static int              term_setpair(int attr, const colattr_t *ca);
static void             term_clearpairs(void);
static void             term_setpairs(void);
static nccolor_t        term_color(const colvalue_t *ca, int def, int *attr);
static int              term_attribute(int sf);
static void             term_init(void);
static void             term_start(void);
static void             term_tidy(void);

static const struct colormap {                  /* GRIEF -> NCURSES palette */
    nccolor_t       color8;
    int             attr8;
    nccolor_t       color88;
    nccolor_t       color256;
} colormap[] = {
          /*Ident             8 - color/attribute             88          256     */
        { /*BLACK,      */    COLOR_BLACK,        0,          0,          0       },
        { /*BLUE,       */    COLOR_BLUE,         0,          12,         12      },
        { /*GREEN,      */    COLOR_GREEN,        0,          10,         10      },
        { /*CYAN,       */    COLOR_CYAN,         0,          14,         14      },
        { /*RED,        */    COLOR_RED,          0,          9,          9       },
        { /*MAGENTA,    */    COLOR_MAGENTA,      0,          13,         13      },
        { /*BROWN,      */    COLOR_YELLOW,       0,          32,         130     },
        { /*WHITE,      */    COLOR_WHITE,        0,          84,         248     },

        { /*GREY,       */    COLOR_BLACK,        A_BOLD,     7,          7       },
        { /*LTBLUE,     */    COLOR_BLUE,         A_BOLD,     43,         81      },
        { /*LTGREEN,    */    COLOR_GREEN,        A_BOLD,     61,         121     },
        { /*LTCYAN,     */    COLOR_CYAN,         A_BOLD,     63,         159     },
        { /*LTRED ,     */    COLOR_RED,          A_BOLD,     74,         224     },
        { /*LTMAGENTA,  */    COLOR_MAGENTA,      A_BOLD,     75,         225     },
        { /*YELLOW ,    */    COLOR_YELLOW,       A_BOLD,     11,         11      },
        { /*LTWHITE,    */    COLOR_WHITE,        A_BOLD,     1,          15      },

        { /*DKGREY,     */    COLOR_BLACK,        A_DIM,      82,         242     },
        { /*DKBLUE,     */    COLOR_BLUE,         A_DIM,      4,          4       },
        { /*DKGREEN,    */    COLOR_GREEN,        A_DIM,      2,          2       },
        { /*DKCYAN,     */    COLOR_CYAN,         A_DIM,      6,          6       },
        { /*DKRED,      */    COLOR_RED,          A_DIM,      1,          1       },
        { /*DKMAGENTA,  */    COLOR_MAGENTA,      A_DIM,      5,          5       },
        { /*DKYELLOW,   */    COLOR_YELLOW,       A_DIM,      72,         130     },
        { /*LTYELLOW,   */    COLOR_YELLOW,       A_DIM,      78,         229     },
        };

static nccolor_t        tt_pairs        = -1;
static int              tt_colors       = 0;
static int              tt_direct       = 0;
static int              tt_defaultbg    = COLOR_BLACK;
static int              tt_defaultfg    = COLOR_WHITE;
static int              tt_active       = 0;
static int              tt_cursor       = 0;
static WINDOW *         tt_win          = NULL;

static nccolor_t        tt_colormap[COLOR_NONE];


/*  Function:           ttcurses
 *      curses/ncurses(3NCURSES) driver run-time initialisation.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      none.
 */
void
ttcurses(void)
{
    x_scrfn.scr_init    = nc_init;
}


/*  Function:           ttncurses
 *      Initialize the terminal when the editor gets started up.
 *
 *  Calling Sequence:
 *>     ttncurses
 *>     ttopen
 *>         -> nc_open
 *>     ttready
 *>         -> nc_read
 *>     ttkeybind
 *>         -> nc_keybind
 *>     macro tty/<terminal-type>
 *>         [ttfeature]
 *>     ttdisplay
 *>         -> nc_display
 *>         [ttfeature]
 *
 *>     ttfeature
 *>         -> nc_feature
 *>     ttprocess
 *>         -> nc_control
 *
 *>     ttclose
 *>         -> nc_close
 *
 *  Parameters:
 *      none.
 *
 *  Configuration:
 *
 *      Terminal identification.
 *
 *          o TERM -
 *              General terminal identification.
 *          o LINES/COLUMNS
 *              Default terminal size.
 *
 *      Terminal capabilities (ncurses).
 *
 *          o NCURSES_NO_UTF8_ACS -
 *              When running in a UTF-8 locale several terminals (including Linux console and GNU screen)
 *              ignore alternative character selection. If set use unicode box drawing characters in all
 *              cases.
 *          o NCURSES_ASSUMED_COLORS -
 *              Foreground and background colors.
 *          o COLORFGBG -
 *              rxvt/mrxvt terminal default color specification.
 *
 *  Returns:
 *      nothing.
 */
static void
nc_init(int *argcp, char **argv)
{
    __CUNUSED(argcp)
    __CUNUSED(argv)

    x_scrfn.scr_open    = nc_open;
    x_scrfn.scr_ready   = nc_ready;
    x_scrfn.scr_keybind = nc_keybind;
    x_scrfn.scr_display = nc_display;
    x_scrfn.scr_feature = nc_feature;
    x_scrfn.scr_control = nc_control;
    x_scrfn.scr_close   = nc_close;

    x_scrfn.scr_winch   = nc_winch;
    x_scrfn.scr_cursor  = nc_cursor;
    x_scrfn.scr_move    = nc_move;

    x_scrfn.scr_clear   = nc_clear;
    x_scrfn.scr_print   = nc_print;
    x_scrfn.scr_putc    = NULL;
    x_scrfn.scr_flush   = nc_flush;

    x_scrfn.scr_insl    = NULL;
    x_scrfn.scr_dell    = NULL;
    x_scrfn.scr_eeol    = nc_eeol;
    x_scrfn.scr_repeat  = NULL;

    x_scrfn.scr_names   = nc_names;
    x_scrfn.scr_beep    = nc_beep;
}


static void
nc_open(scrprofile_t *profile)
{
    term_init();

    io_device_add(TTY_INFD);                    /* stream registration */
    sys_initialise();

    term_start();
    term_colors();
    term_defaultscheme();
    term_identification();

    profile->sp_rows = LINES;
    profile->sp_cols = COLS;
}


static void
nc_ready(int repaint, scrprofile_t *profile)
{
    __CUNUSED(repaint)
    __CUNUSED(profile)

#if defined(BADCORNER)
    profile->sp_lastsafe = FALSE;               /* not safe */
#endif

#if !defined(CURSES_WIDECHAR)
    if (DISPTYPE_UTF8 == xf_disptype) {
        trace_log("ttync: UTF8 not supported\n");
        xf_disptype = -1;                       /* down-grade */
    }
#endif

    if (DISPTYPE_UNKNOWN == xf_disptype) {
#if (NCURSES_VERSION_MAJOR >= 5) && defined(CURSES_WIDECHAR)
        int ncunicode;

        if (0 != (ncunicode = _nc_unicode_locale())) {
            xf_disptype = DISPTYPE_UTF8;
        }
        trace_log("ttync: unicode_locale() = %d\n", ncunicode);
#endif

        if (xf_disptype < 0) {
            if (vtis8bit()) {
                if (0 == x_pt.pt_encoding[0]) {
                    strcpy(x_pt.pt_encoding, "latin1");
                }
                xf_disptype = DISPTYPE_8BIT;
            } else {
                if (0 == x_pt.pt_encoding[0]) {
                    strcpy(x_pt.pt_encoding, "us-ascii");
                }
                xf_disptype = DISPTYPE_7BIT;
            }
        }
    }

    if (DISPTYPE_UTF8 == xf_disptype) {
        x_display_ctrl |= DC_UNICODE;

#if defined(NCURSES_VERSION)
    } else if (DISPTYPE_8BIT == xf_disptype) {
        use_legacy_coding(2);                   /* enable 8-bit */

    } else if (DISPTYPE_7BIT == xf_disptype) {
        use_legacy_coding(0);
#endif
    }

#if !defined(CURSES_WIDECHAR)
    x_display_ctrl |= DC_ASCIIONLY;             /* no wide character support */
#endif

    tt_active = 1;
}


static void
nc_keybind(void)
{
    const TermKey_t *ti;
    unsigned count, i;

    if (NULL != (ti = ttcfgkeys(&count))) {     /* load */
        for (i = 0; i < count; ++i) {
            if (ti->key && ti->svalue) {
                key_define_key_seq(ti->key, ti->svalue);
            }
            ++ti;
        }
    }
}


static void
nc_display(void)
{
#if defined(NCURSES_VERSION)
    sxprintf(x_pt.pt_encoding, sizeof(x_pt.pt_encoding), "%s%s", curses_version(),
        (DISPTYPE_UTF8 == xf_disptype ? ".utf-8" : ""));
#else
    sxprintf(x_pt.pt_encoding, sizeof(x_pt.pt_encoding), "curses%s",
        (DISPTYPE_UTF8 == xf_disptype ? ".utf-8" : ""));
#endif

    trace_log("ttync:\n");
    trace_log(" disptype  : %d\n", xf_disptype);
    trace_log(" direct    : %d\n", tt_direct);
    trace_log(" colors    : %d, max=%d, change=%d\n", tt_colors, COLORS, can_change_color());
    trace_log(" pairs     : %d\n", COLOR_PAIRS);

    x_pt.pt_colordepth = tt_colors;
    ttboxcharacters(TRUE);
}


static void
nc_close(void)
{
    term_tidy();
    sys_shutdown();
}


static void
nc_feature(int ident, scrprofile_t *profile)
{
    trace_log("ttync: feature(%d)\n", ident);

    __CUNUSED(profile)
    switch (ident) {
    case TF_COLORDEPTH:
        if (x_pt.pt_colordepth > COLORS) {      // system-limit
            x_pt.pt_colordepth = COLORS;
        }
        if (x_pt.pt_colordepth > 256) {
            x_pt.pt_colordepth = 256;
        }
        break;

    case TF_TRUECOLOR:
        if (! istruecolor()) {
            x_pt.pt_truecolor = 0;              // not-supported; ignore
        }
        break;

    case TF_COLORSCHEME:
        term_setpairs();                        // reassign pairs
        break;

    case TF_COLORMAP: {
            /*
             * Example;
             *      "black,red3,green3,yellow3,blue3,magenta3,cyan3,\
             *          gray90,gray30,red,green,yellow,blue,magenta,cyan,white"
             */
            if (can_change_color() && x_pt.pt_colormap[0]) {
#if (TODO_COLORMAP)
                const char *cursor = x_pt.pt_colormap;
                nccolor_t color = 0, r, g, b;

                while ((cursor = foreach_color(cursor, &r, &g, &b)) {
                    if (r >= 0) {                   /* 0...255 */
                        init_color(color, ToNCurses(r), ToNCurses(g), ToNCurses(b));
                    }
                    ++color;
                }
#endif //TODO
            }
        }
        break;

    case TF_COLORPALETTE: {
            /*
             * Example;
             *      "0,   4,   2,   6,   1,   5, 130, 7,  8,  12,  10,   14,
             *       9,  13,  11,  15,   0,   4, 2,   6,  1,  5,   130,  11"
             */
            const char *cursor = x_pt.pt_colorpalette;
            unsigned col = 0;

            for (col = 0; col < (sizeof(tt_colormap)/sizeof(tt_colormap[0])); ++col) {
                int val = -1;

                if (cursor && *cursor) {
                    if (isdigit(*cursor)) {
                        val = atoi(cursor);
                    }
                    if (NULL != (cursor = strchr(cursor, ','))) {
                        ++cursor;
                    }
                }
                tt_colormap[col] = (nccolor_t)val;
            }
            x_pt.pt_xtpalette = -2;
            term_clearpairs();
        }
        break;

    case TF_DEFAULT_FG:
    case TF_DEFAULT_BG:
        if (x_pt.pt_defaultfg >= 0 && x_pt.pt_defaultbg >= 0) {
            assume_default_colors(x_pt.pt_defaultfg, x_pt.pt_defaultbg);
            tt_defaultfg = tt_defaultbg = -1;
            term_clearpairs();
        }
        break;

    case TF_ENCODING:
        break;
    }
}


static int
nc_control(int action, int param, ...)
{
    switch (action) {
    case SCR_CTRL_NORMAL:       /* normal color */
        attrset(0);
        break;

    case SCR_CTRL_GARBLED:
        redrawwin(tt_win);
        break;

    case SCR_CTRL_SAVE:
        if (param) {
            term_tidy();
        } else {
            meta(tt_win, FALSE);
            keypad(tt_win, FALSE);
            nc_move(LINES - 1, 0);
            wclrtoeol(tt_win);
            wrefresh(tt_win);
        }
        break;

    case SCR_CTRL_RESTORE:
        term_start();
        redrawwin(tt_win);
        wrefresh(tt_win);
        break;

    case SCR_CTRL_COLORS:       /* color change */
        term_clearpairs();
        break;

    default:
        return -1;
    }
    return 0;
}


static void
nc_winch(int *nrow, int *ncol)
{
    endwin();
    term_start();
    doupdate();
    *nrow = LINES;
    *ncol = COLS;
}


static int
nc_cursor(int visible, int imode, int virtual_space)
{
    __CUNUSED(visible)
    __CUNUSED(imode)
    __CUNUSED(virtual_space)
    return tt_cursor;
}


static void
nc_move(int row, int col)
{
    wmove(tt_win, row, col);
    ttposset(row, col);
    wrefresh(tt_win);
}


static void
nc_clear(void)
{
    redrawwin(tt_win);
}


static void
nc_print(int row, int col, int len, const struct _VCELL *vvp)
{
    const int cols = COLS;

    if (row < LINES) {
        wmove(tt_win, row, col);
        while (len-- > 0 && col++ < cols) {
            nc_putc(vvp++);
        }
    }
    ttposinvalid();
}


static void
nc_putc(const struct _VCELL *cell)
{
    const vbyte_t attr = VBYTE_ATTR_GET(cell->primary);
    vbyte_t c = VBYTE_CHAR_GET(cell->primary);

    if (CH_PADDING == c) {                      /* wide character padding */
        return;
    }

    if (tt_colors <= 2) {
#if defined(A_ITALIC)                           /* TODO - runtime attribute checks */
        attrset(term_attribute(ttbandw(attr, 1, 1, 0)));
#else
        attrset(term_attribute(ttbandw(attr, 1, 0, 0)));
#endif

    } else {
        colattr_t ca;
        int a = 0;

        if (color_definition(attr, &ca)) {
            if ((a = ca.val) < 0) {             /* assigned palette index */
                a = term_setpair(attr, &ca);
            }
            attrset(a | term_attribute(ca.sf));
        }
    }

    if (c >= CH_MIN && c <= CH_MAX) {
        if (xf_graph) {
            switch (c) {
            case CH_HORIZONTAL: c = ACS_HLINE;      break;
            case CH_VERTICAL:   c = ACS_VLINE;      break;
            case CH_TOP_LEFT:   c = ACS_ULCORNER;   break;
            case CH_TOP_RIGHT:  c = ACS_URCORNER;   break;
            case CH_BOT_LEFT:   c = ACS_LLCORNER;   break;
            case CH_BOT_RIGHT:  c = ACS_LRCORNER;   break;
            case CH_TOP_JOIN:   c = ACS_TTEE;       break;
            case CH_BOT_JOIN:   c = ACS_BTEE;       break;
            case CH_LEFT_JOIN:  c = ACS_RTEE;       break;
            case CH_RIGHT_JOIN: c = ACS_LTEE;       break;
            case CH_CROSS:      c = ACS_PLUS;       break;
                break;
#if defined(CURSES_WIDECHAR) && defined(CCHARW_MAX)
            case CH_HSCROLL:
            case CH_VSCROLL:
            case CH_HTHUMB:
            case CH_VTHUMB:
            case CH_TOP_LEFT2:
            case CH_TOP_RIGHT2:
            case CH_BOT_LEFT2:
            case CH_BOT_RIGHT2:
            case CH_RADIO_OFF:
            case CH_RADIO_ON:
            case CH_CHECK_OFF:
            case CH_CHECK_ON:
            case CH_CHECK_TRI: {
                    const int cw = cmap_specunicode(c);

                    if (cw > 0) {
                        cchar_t wch = {0};
                        wch.chars[0] = cw;
                        wadd_wch(tt_win, &wch);
                    } else {
                        switch(c) {
                        case CH_HSCROLL:    c = ACS_HLINE;    break;
                        case CH_VSCROLL:    c = ACS_VLINE;    break;
                        case CH_HTHUMB:
                        case CH_VTHUMB:
                            c = ACS_BLOCK;
                            break;
                        case CH_TOP_LEFT2:  c = ACS_ULCORNER; break;
                        case CH_TOP_RIGHT2: c = ACS_URCORNER; break;
                        case CH_BOT_LEFT2:  c = ACS_LLCORNER; break;
                        case CH_BOT_RIGHT2: c = ACS_LRCORNER; break;
                        default:
                            goto nograph;
                        }
                        waddch(tt_win, c);
                    }
                }
                return;
#else
            case CH_HSCROLL:    c = ACS_HLINE;      break;
            case CH_VSCROLL:    c = ACS_VLINE;      break;
            case CH_HTHUMB:
            case CH_VTHUMB:
                c = ACS_BLOCK;
                break;
            case CH_TOP_LEFT2:  c = ACS_ULCORNER;   break;
            case CH_TOP_RIGHT2: c = ACS_URCORNER;   break;
            case CH_BOT_LEFT2:  c = ACS_LLCORNER;   break;
            case CH_BOT_RIGHT2: c = ACS_LRCORNER;   break;
#endif
            default:
                goto nograph;
            }
            waddch(tt_win, c);
            return;
        } else {
nograph:;   const char *cp;
            if (NULL != (cp = ttspecchar(c))) {
                c = (unsigned char)*cp;
            }
        }
    }

    if (c > 0) {
#if defined(CURSES_WIDECHAR) && defined(CCHARW_MAX)
        if ((DISPTYPE_UTF8 == xf_disptype && c <= 0x9f) ||
                (DISPTYPE_UTF8 != xf_disptype && c <= 0xff)) {
            waddch(tt_win, c);
        } else {
            cchar_t wch = {0};

            wch.chars[0] = c;
            if (cell->combined) {
                unsigned idx = 0;
                while (0 != (c = cell->combined[idx]) && idx < VCOMBINED_MAX) {
                    wch.chars[++idx] = c;
                }
            }
            wadd_wch(tt_win, &wch);
        }
#else
        waddch(tt_win, c);
#endif
    }
}


static int
nc_eeol(void)
{
    wclrtoeol(tt_win);
    return TRUE;
}


static void
nc_flush(void)
{
    wrefresh(tt_win);
}


static void
nc_beep(int freq, int duration)
{
    __CUNUSED(freq)
    __CUNUSED(duration)
    beep();
}


static int
nc_names(const char *title, const char *icon)
{
    __CUNUSED(title)
    __CUNUSED(icon)
    return -1;
}


static const char *
ncigetstr(const char *str)
{
    const char *val = tigetstr(str);
    if (val == (const char *)-1) {
        val = NULL;
    }
    return val;
}


/*
 *  term_identification ---
 *      Request the terminal version/DA2 details.
 */
static void
term_identification(void)
{
    const char *RV;

    if (NULL == (RV = ncigetstr("RV")) || *RV == '\0') {
        const char *term = ggetenv("TERM");
        if (NULL == term ||
                (0 == tty_isterm(term, "xterm") && 0 == tty_isterm(term, "kitty"))) {
            return;
        }
    }
    tty_identification(RV, ESCDELAY);
}


static int
istruecolor(void)
{
#if defined(CURSES_DIRECT_COLORS)
    if (COLORS == DIRECT_COLORS) {
        return COLORIF_DIRECT;                  /* direct RGB support. */
    }
#endif
    return 0;
}


/*
 *  term_defaultscheme ---
 *      Determine the terminal deault color-scheme.
 */
static void
term_defaultscheme(void)
{
    if (x_pt.pt_schemedark < 0) {
        int isdark  = -1;

        if (tigetflag("XT") > 0) {              /* OSC-11 supported? */
            isdark = tty_luminance(ESCDELAY);
        }

        if (isdark < 0) {
            isdark = tty_defaultscheme();       /* Legacy */
        }

        x_pt.pt_schemedark = isdark;
    }
}


/*
 *  term_dump ---
 *      Dump the terminal definition.
 */
static void
term_dump(void)
{
    // https://invisible-island.net/ncurses/man/user_caps.5.html
    // https://www.gnu.org/software/screen/manual/screen.html#Termcap-Syntax
    //      screen - special terminal capabilities
    //          XT = terminal understands special xterm sequences (OSC, mouse tracking).
    //
    static const struct {
        const char *name, *fname;
    } extrastr[] = {
        { "E3",     "ud_clear_scrollback" },
        { "RGB",    "ud_rgb_spec" },
        { "XM",     "ud_mouse_enable_disable" },
        { NULL, NULL }
        };

    static const struct {
        const char *name, *fname;
    } extranum[] = {
        { "C0",     "ud_rgb_overlay" },
        { "RGB",    "ud_rgb_num" },
        { "U8",     "ud_unicode_line" },
        { NULL, NULL }
        };

    static const struct {
        const char *name, *fname;
    } extrabool[] = {
        { "AX",     "ud_sgr_default" },
        { "RGB",    "ud_rgb_bool" },
        { "XT",     "ud_xterm_osc" },
        { NULL, NULL }
        };
    const char *fname;
    int i;

    if (0 == trace_flags()) return;

    trace_log("termcap:\n");
    trace_log("  Strings:\n");
    for (i = 0; (fname = strfnames[i]) != NULL; ++i) {
        if (0 != memcmp(fname, "key_", 4)) {    // key_xxx
            const char *name = strnames[i];
            const char *val = tigetstr(CURSES_CAST(name));
            trace_log("\t%-24.24s %-4s %-8s : %s\n",
                fname, strcodes[i], name, (val == NULL ? "" : (val == (void *)-1 ? "NA" : c_string(val))));
            if (0 == strcmp(fname, "acs_chars")) {
                acs_dump(val);
            }
        }
    }
    for (i = 0; (fname = extrastr[i].fname) != NULL; ++i) {
        const char *name = extrastr[i].name;
        const char *val = tigetstr(CURSES_CAST(name));
        trace_log("\t%-24.24s %-4s %-8s : %s\n",
            fname, "", name, (val == NULL ? "" : (val == (void *)-1 ? "NA" : c_string(val))));
    }

    trace_log("  Keys:\n");
    for (i = 0; (fname = strfnames[i]) != NULL; ++i) {
        if (0 == memcmp(fname, "key_", 4)) {    // key_xxx
            const char *name = strnames[i];
            const char *val = tigetstr(CURSES_CAST(name));
            trace_log("\t%-24.24s %-4s %-8s : %s\n",
                fname, strcodes[i], name, (val == NULL ? "" : (val == (void *)-1 ? "NA" : c_string(val))));
        }
    }

    trace_log("  Numeric:\n");
    for (i = 0; (fname = numfnames[i]) != NULL; ++i) {
        const char *name = numnames[i];
        const int val = tigetnum(CURSES_CAST(name));
        trace_log("\t%-24.24s %-4s %-8s : %d\n",
            fname, numcodes[i], name, val);
    }
    for (i = 0; (fname = extranum[i].fname) != NULL; ++i) {
        const char *name = extranum[i].name;
        const int val = tigetnum(CURSES_CAST(name));
        trace_log("\t%-24.24s %-4s %-8s : %d\n",
            fname, "", name, val);
    }

    trace_log("  Boolean/Flags:\n");
    for (i = 0; (fname = boolfnames[i]) != NULL; ++i) {
        const char *name = boolnames[i];
        const int val = tigetflag(CURSES_CAST(name));
        trace_log("\t%-24.24s %-4s %-8s : %d\n",
            fname, boolcodes[i], name, val);
    }
    for (i = 0; (fname = extrabool[i].fname) != NULL; ++i) {
        const char *name = extrabool[i].name;
        const int val = tigetflag(CURSES_CAST(name));
        trace_log("\t%-24.24s %-4s %-8s : %d\n",
            fname, "", name, val);
    }
}


static void
acs_dump(const char *bp)
{
    const static struct {
        unsigned char ident;
        const char *desc;
    } term_characters[] = {                     /* graphic characters */
        { '}', "UK pound sign" },
        { '.', "arrow pointing down" },
        { ',', "arrow pointing left" },
        { '+', "arrow pointing right" },
        { '-', "arrow pointing up" },
        { 'h', "board of squares" },
        { '~', "bullet" },
        { 'a', "checker board (stipple)" },
        { 'f', "degree symbol" },
        { '`', "diamond" },
        { 'z', "greater-than-or-equal-to" },
        { '{', "greek pi" },
        { 'q', "horizontal line" },
        { 'i', "lantern symbol" },
        { 'n', "large plus or crossover" },
        { 'y', "less-than-or-equal-to" },
        { 'm', "lower left corner" },
        { 'j', "lower right corner" },
        { '|', "not-equal" },
        { 'g', "plus/minus" },
        { 'o', "scan line 1" },
        { 'p', "scan line 3" },
        { 'r', "scan line 7" },
        { 's', "scan line 9" },
        { '0', "solid square block" },
        { 'w', "tee pointing down" },
        { 'u', "tee pointing left" },
        { 't', "tee pointing right" },
        { 'v', "tee pointing up" },
        { 'l', "upper left corner" },
        { 'k', "upper right corner" },
        { 'x', "vertical line" }
        };

    const unsigned char *p = (unsigned char *)bp;
    unsigned char ident, ch;
    unsigned i;

    while (*p) {
        const char *desc = "unknown";

            ident = *p++;
        if (! isprint(ident)) continue;
        ch = *p++;

        for (i = 0; i < (sizeof(term_characters)/sizeof(term_characters[0])); ++i)
            if (term_characters[i].ident == ident) {
                desc = term_characters[i].desc;
                break;
            }

        trace_log("\t\t%-30s %c/0x%x : %u/0x%x\n", desc, ident, ident, ch, ch);
    }
}


/*
 *  term_setpair ---
 *      initialise a color-pair.
 */
static int
term_setpair(int attr, const colattr_t *ca)
{
    int attribute = 0;
    const nccolor_t fg = term_color(&ca->fg, tt_defaultfg, &attribute);
    const nccolor_t bg = term_color(&ca->bg, tt_defaultbg, NULL);
    nccolor_t id;
    int ret;

    for (id = tt_pairs; id >= 0; --id) {
        nccolor_t t_fg, t_bg;                   /* search existing pairs 0 .. tt_pairs */

#if defined(CURSES_EXTENDED_COLOR)
        ret = extended_pair_content(id, &t_fg, &t_bg);
#else
        ret = pair_content(id, &t_fg, &t_bg);
#endif
        if (OK == ret && t_fg == fg && t_bg == bg) {
            trace_log("ttync: getpair(id:%3d, fg:%3d/0x%02x, bg:%3d/0x%03x, a:%d/0x%x) : %d\n", \
                id, (int)fg, (int)fg, (int)bg, (int)bg, (int)attribute, (int)attribute, ret);
            break;
        }
    }

    if (id < 0) {                               /* new pair required */
        id = ++tt_pairs;
#if defined(CURSES_EXTENDED_COLOR)
        ret = init_extended_pair(id, fg, bg);
#else
        ret = init_pair(id, fg, bg);
#endif
        trace_log("ttync: setpair(id:%3d, fg:%3d/0x%02x, bg:%3d/0x%03x, a:%d/0x%x) : %d\n", \
            id, (int)fg, (int)fg, (int)bg, (int)bg, (int)attribute, (int)attribute, ret);
        if (OK != ret) {
            --tt_pairs;                         /* out of resources (COLOR_PAIRS) */
            id = 0; //default color
        }
    }

    attribute |= COLOR_PAIR(id);
    color_valueset(attr, attribute);
    return attribute;
}


static void
term_clearpairs(void)
{
    if (tt_pairs != 0) {
        tt_pairs = 0;
        color_valueclr(-1);
#if defined(CURSES_EXTENDED_COLOR)
#if defined(HAVE_RESET_COLOR_PAIRS)
        reset_color_pairs();                    // TODO: not all 6.1 releases
#endif
#endif
    }
}


static void
term_setpairs(void)
{
    colattr_t ca;
    unsigned attr;

    term_clearpairs();
    for (attr = 0; attr < ATTR_MAX; ++attr) {
        color_definition(attr, &ca);
        if (ca.bg.color || ca.fg.color || ca.sf) {
            term_setpair(attr, &ca);
        }
    }
}


/*
 *  term_colors ---
 *      Determine the terminal color depth.
 */
static void
term_colors(void)
{
    int truecolor = 0;

    tt_colors = 2;                              // black&white

#if defined(A_COLOR)
    if (xf_color >= COLORMODE_8 || (COLORMODE_AUTO == xf_color && has_colors()) ||
            x_pt.pt_colordepth > 1) {

        tt_defaultfg = COLOR_WHITE;
        tt_defaultbg = COLOR_BLACK;

        start_color();
        tt_colors = COLORS;

        if (xf_color == COLORMODE_AUTO || xf_color > COLORMODE_256) {
            truecolor = istruecolor();          // enable if supported
        } else if (tt_colors > xf_color) {
            tt_colors = xf_color;               // limit to configuration
        }

        if (tt_colors > 256) {
            tt_colors = 256;                    // base color limit
        }

        if (x_pt.pt_defaultfg >= 0 && x_pt.pt_defaultbg >= 0) {
            assume_default_colors(x_pt.pt_defaultfg, x_pt.pt_defaultbg);
            tt_defaultfg = tt_defaultbg = -1;

        } else if (OK == use_default_colors()) {
            tt_defaultfg = tt_defaultbg = -1;
        }
    }

#if defined(CURSES_DIRECT_COLORS)
    if (COLORS == DIRECT_COLORS) {              // direct, overlap count
        if ((tt_direct = tigetnum(CURSES_CAST("CO"))) <= 0)  {
            tt_direct = 1;
        }
    }
#endif
#endif //A_COLOR

    trace_log("ttync: colors(depth:%d,truecolor:%d,fg:%d,bg:%d)\n", tt_colors, truecolor, tt_defaultfg, tt_defaultbg);

    x_pt.pt_colordepth = tt_colors;
    x_pt.pt_truecolor = truecolor;
    if (tt_colors > 2) {
        x_display_ctrl |= DC_SHADOW_SHOWTHRU;
    }

    term_clearpairs();
}


/*
 *  term_color ---
 *      Map color-attribute to a terminal color.
 */
static nccolor_t
term_color(const colvalue_t *ca, int def, int *attributes)
{
#if defined(CURSES_DIRECT_COLORS)
    const unsigned rgbcolor = ca->rgbcolor;
#endif
    nccolor_t color = (nccolor_t)ca->color;
    int attribute = 0;

#define COLOR_XTERM(_r, _g, _b) \
    (((unsigned)((unsigned char)(_r) << 16) | ((unsigned)(unsigned char)(_g) << 8) | ((unsigned)(unsigned char)(_b) << 0)))

#if defined(CURSES_DIRECT_COLORS)
    if (rgbcolor && tt_direct) {
        nccolor_t r, g, b;

        r = COLOR_RVAL(rgbcolor);
        g = COLOR_GVAL(rgbcolor);
        b = COLOR_BVAL(rgbcolor);
        color = COLOR_XTERM(r, g, b);
        if (color < tt_direct)
            color |= 0x010000;                    /* rgb namespace */
        return color;
    }
#endif //CURSES_DIRECT_COLORS

    if (COLORSOURCE_SYMBOLIC == ca->source) {
        if (color < 0 || color >= COLOR_NONE) {
            color = (nccolor_t)def;
        } else {
            if (tt_colormap[color] >= 0) {
                color = tt_colormap[color];
            } else if (tt_colors >= 256) {
                color = colormap[color].color256;
            } else if (tt_colors >= 88) {
                color = colormap[color].color88;
            } else {
                attribute = colormap[color].attr8;
                color = colormap[color].color8;
            }
        }

    } else {
        if (color >= tt_colors) {
            if (color >= (tt_colors * 2)) {
                color = (nccolor_t)def;
            } else {
                if (8 == tt_colors) {
                    if (color & ~7)
                        attribute = A_BOLD;     /* 8 .. 15 */
                    color &= 7;                 /* 0 .. 7 */
                }
            }
        }
    }

#if defined(CURSES_DIRECT_COLORS)
    if (tt_direct && color >= tt_direct) {
        struct rgbvalue rgb;

        rgb_256xterm(color, &rgb);              /* index to rgb */
        color = COLOR_XTERM(rgb.red, rgb.green, rgb.blue);
        if (color < tt_direct)
            color |= 0x010000;                  /* rgb namespace */
    }
#endif //CURSES_DIRECT_COLORS

    if (attributes) {                           /* foreground; 8-colors */
        *attributes |= attribute;
    }
    return color;
}


/*
 *  term_attribute ---
 *      Map color-style to a terminal attribute.
 */
static int
term_attribute(int sf)
{
    int attr = 0;

    if (sf & COLORSTYLE_BOLD)      attr |= A_BOLD;
    if (sf & COLORSTYLE_DIM)       attr |= A_DIM;
    if (sf & COLORSTYLE_STANDOUT)  attr |= A_STANDOUT;
    if (sf & COLORSTYLE_INVERSE)   attr |= A_REVERSE;
    if (sf & COLORSTYLE_UNDERLINE) attr |= A_UNDERLINE;
    if (sf & COLORSTYLE_BLINK)     attr |= A_BLINK;
    if (sf & COLORSTYLE_REVERSE)   attr |= A_REVERSE;
#if defined(A_ITALIC)
    if (sf & COLORSTYLE_ITALIC)    attr |= A_ITALIC;
#endif
    return attr;
}


/*
 *  term_init ---
 *      Terminal session one-shot initialisation.
 */
static void
term_init(void)
{
    const char *term;
    unsigned col;
    int err = 0;

    tt_win = NULL;
    tt_colors = 0;
    tt_defaultbg = COLOR_BLACK;
    tt_defaultfg = COLOR_WHITE;
    tt_active = FALSE;
    tt_cursor = TRUE;
    for (col = 0; col < (sizeof(tt_colormap)/sizeof(tt_colormap[0])); ++col) {
        tt_colormap[col] = -1;
    }

    if (NULL == (term = ggetenv("TERM"))) {
        fprintf(stderr, "Environment variable TERM not defined!\n");
        exit(1);
    }

    if (setupterm((char *)term, fileno(stdout), &err) != 0) {
        if (0 == err) {
            fprintf(stderr, "Terminal '%s' not found within terminfo database.\n", term);
        } else if (1 == err) {
            fprintf(stderr, "Terminal '%s' is hardcopy, cannot be used for curses applications.\n", term);
        } else {
            fprintf(stderr, "Terminfo database not available.\n");
        }
        exit(1);
    }

    term_dump();

#if defined(CURSES_DIRECT_COLORS)
    if (COLORS > 256) {
        if (COLORS != CURSES_DIRECT_COLORS || COLOR_PAIRS < (DIRECT_PAIRS/2)) {
            printf("Terminal '%s' direct color depth of %d not supported.\n", term, COLORS);
            exit(1);
        }
    }
#endif

    if (x_pt.pt_name[0] == 0) {
        strxcpy(x_pt.pt_name, longname(), sizeof(x_pt.pt_name));
    }
}


/*
 *  term_start ---
 *      Terminal start session operation.
 */
static void
term_start(void)
{
    if (NULL == tt_win) {
        tt_win = initscr();
    }
    nonl();
    noecho();
    idlok(tt_win, TRUE);
    scrollok(tt_win, FALSE);
    meta(tt_win, TRUE);
    keypad(tt_win, TRUE);
}


/*
 *  term_tidy ---
 *      Terminal end session operation.
 */
static void
term_tidy(void)
{
    if (NULL == tt_win) {
        return;
    }
    nc_move(LINES - 1, 0);
    attrset(0);
    wclrtoeol(tt_win);
    endwin();
    tt_active = tt_cursor = 0;
    tt_win = NULL;
}

#endif  /*HAVE_LIBNCURSES*/

/*end*/

