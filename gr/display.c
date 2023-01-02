#include <edidentifier.h>
__CIDENT_RCSID(gr_display_c,"$Id: display.c,v 1.84 2022/09/28 16:17:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: display.c,v 1.84 2022/09/28 16:17:06 cvsuser Exp $
 * High level display interface.
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

#define ED_LEVEL 3
#define ED_ASSERT

#include <editor.h>
#include <edassert.h>
#include <edconfig.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "anchor.h"
#include "border.h"
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "cmap.h"
#include "color.h"
#include "debug.h"
#include "dialog.h"
#include "display.h"
#include "symbol.h"                             /* system_call() */
#include "system.h"
#include "echo.h"                               /* eeputs() */
#include "eval.h"                               /* get_...()/isa_...() */
#include "getkey.h"
#include "hilite.h"
#include "main.h"
#include "map.h"                                /* linep() */
#include "mchar.h"                              /* mchar_... */
#include "ruler.h"                              /* next_..._stop() */
#include "syntax.h"
#include "tty.h"
#include "vm_alloc.h"
#include "window.h"                             /* win_...() */

#include "m_pty.h"
#include "m_display.h"
#include "m_ruler.h"


#define VF_CHANGE               0x01            /* virtual changes */
#define LMARGIN                 128             /* upper left margin width */

static void                 vtblank(void);
static void                 vtmove(int row, int col);
static int                  vtputb(const vbyte_t c);
static void                 vtpushb(vbyte_t ch, int col);
static void                 vtputs(const unsigned char *s, vbyte_t col);
static int                  vtcursor_position(void);
static void                 vtcursor_draw(void);
static void                 vtwindow(WINDOW_t *wp);

static void                 uscrolldown(int top, int bottom, vbyte_t attr);
static void                 uscrollup(int top, int bottom, vbyte_t attr);
static void                 uflush(void);
static void                 umove(int row, int col);
static void                 uupdate(int row, int eflag);
static void                 uline(const int row, const VCELL_t *vvp, VCELL_t *pvp, int eflag);
static void                 ucopy(const VCELL_t *vvp, VCELL_t *pvp);

static void                 uputchar(const vbyte_t ch);
static void                 uputcell(const VCELL_t *cell);

static void                 vcell_copy(VCELL_t *cell, const VCELL_t *src);
static void                 vcell_set(VCELL_t *c, const vbyte_t ch);
static void                 vcell_push(VCELL_t *cell, const vbyte_t ch);
static int                  vcell_compare(const VCELL_t *a, const VCELL_t *b);
static int                  vcell_space(const VCELL_t *cell, const vbyte_t spaceattr);
static void                 vcell_put(const VCELL_t *cell);

static void                 vthumb_position(WINDOW_t *wp);
static void                 hthumb_position(WINDOW_t *wp);
static void                 vthumb_refresh(WINDOW_t *wp);
static void                 hthumb_refresh(WINDOW_t *wp);

static int                  nexttab(const WINDOW_t *wp, int redge);

static void                 winputch(WINDOW_t *wp, const vbyte_t ch, const vbyte_t attr, int rclip);
static void                 winputmc(WINDOW_t *wp, const cmapchr_t *mc, const vbyte_t attr, int rclip);
#define                     winputm(__c) \
                                (wp->w_disp_indent > 0 ? --wp->w_disp_indent : vtputb(__c))

static __CINLINE vbyte_t    normalcolor(const WINDOW_t *wp);
static __CINLINE vbyte_t    framecolor(const WINDOW_t *wp);
static __CINLINE vbyte_t    scrollcolor(const WINDOW_t *wp);
static __CINLINE vbyte_t    thumbcolor(const WINDOW_t *wp);
static __CINLINE int        hasborders(const WINDOW_t *wp);

#define VTDRAW_SCROLLED         0x01
#define VTDRAW_LAZY             0x02
#define VTDRAW_REPAINT          0x04
#define VTDRAW_DIRTY            0x08

static void                 draw_window(WINDOW_t *wp, int top, LINENO line, int end, const int bottom, int actions);

static void                 draw_title(const WINDOW_t *wp, const int top, const int line);
static int                  draw_hscroll(WINDOW_t *wp, int line);
static void                 draw_ruler(const WINDOW_t *wp, int line, int ledge);
static void                 draw_left(const WINDOW_t *wp);
static void                 draw_right(WINDOW_t *wp, int redge, vbyte_t space);
static void                 draw_shadow(const WINDOW_t *wp, vbyte_t ch);

static vbyte_t              draw_normal_line(const LINENO line, const LINE_t *lp);
static vbyte_t              draw_syntax_line(const LINENO line, const LINE_t *lp);

#define DL_DOEOL                0x0001          /* EOL processing */
#define DL_ISEOF                0x0002          /* Character is EOL */
#define DL_BASECMAP             0x0004          /* use base cmap */
#define DL_ISBINARY             0x0008          /* buffer is binary */

static vbyte_t              draw_line(const vbyte_t attr, const vbyte_t tattr,
                                    const LINECHAR *cp, const LINEATTR *ap, int length, int flags);

#define ANSI_MABOLD             0x0001
#define ANSI_MABLINK            0x0002
#define ANSI_MAREVERSE          0x0004
#define ANSI_MAUNDERLINE        0x0008
#define ANSI_MAITALIC           0x0010
#define ANSI_MACOLORS           0x0100
#define ANSI_MACOLORS256        0x0200
#define ANSI_MANORMAL           0x1000

#define ANSI_FGDEFAULT          WHITE
#define ANSI_BGDEFAULT          BLACK

static int                  ansidecode(const LINECHAR *cp, const LINECHAR *end, vbyte_t *attr, unsigned *flags);
static uint16_t             ansicolor(uint16_t color);

static const vbyte_t        v_corner_map[] = {
    0,                         /* TL_CORNER - CORNER_12 (0) */
    0,                         /* TR_CORNER */
    0,                         /* BL_CORNER */
    CH_BOT_LEFT,               /* BR_CORNER */

    0,                         /* TL_CORNER - CORNER_3  (4) */
    CH_VERTICAL,               /* TR_CORNER */
    CH_TOP_LEFT,               /* BL_CORNER */
    CH_RIGHT_JOIN,             /* BR_CORNER */

    0,                         /* TL_CORNER - CORNER_6  (8) */
    CH_BOT_RIGHT,              /* TR_CORNER */
    CH_HORIZONTAL,             /* BL_CORNER */
    CH_BOT_JOIN,               /* BR_CORNER */

    CH_TOP_RIGHT,              /* TL_CORNER - CORNER_9  (12) */
    CH_LEFT_JOIN,              /* TR_CORNER */
    CH_TOP_JOIN,               /* BL_CORNER */
    CH_CROSS                   /* BR_CORNER */
    };

static int                  vupdated  = FALSE;  /* TRUE if initial update() called. */
static int                  vupdating = FALSE;  /* TRUE if updating. */
static int                  vgarbled  = TRUE;   /* TRUE if screen is garbage. */
static int                  vdelayed  = 0;      /* Delay count. */

static VFLAG_t             *vflags    = NULL;   /* Array of flags. */
static VCELL_t             *vbuffer   = NULL;   /* Actual screen data. */
static VCELL_t            **vscreen   = NULL;   /* Virtual screen */
static VCELL_t            **pscreen   = NULL;   /* Physical screen image. */
static VCELL_t             *vline     = NULL;   /* Current video position. */
static VCELL_t             *vblanks   = NULL;   /* Blank line image. */
static const char          *vtitle    = NULL;   /* Current title. */

static vmpool_t             vmcombined;         /* Combined character pool. */

static int                  vtrow;              /* Terminal cursor. */
static int                  vtcol;
static int                  vtcursorrow = -1;   /* Active 'client' cursor row. */
static int                  vtcursorcol = -1;   /* Active 'client' cursor col. */
static vbyte_t              vtmouse_chr;        /* Old mouse character. */
static int                  vtmouse_row;
static int                  vtmouse_col;

static int                  ucol = -1;
static int                  urow = -1;
                                                /* Display type. */
int                         xf_disptype = DISPTYPE_UNKNOWN;

int                         xf_termcap = FALSE; /* Termcap override (tty driver option). */


/*  Function:           vtinit
 *      Initialise display and any window system.
 *
 *  Parameters:
 *      argcp - Address of command line arguments.
 *      argv - Count.
 *
 *  Returns:
 *      nothing.
 */
void
vtinit(int *argc, char **argv)
{
    trace_log("vtinit:\n");
    ttdefaults();
    if (x_scrfn.scr_init) {
        (*x_scrfn.scr_init)(argc, argv);
    } else {
        ttinit();
    }
}


/*  Function:           vtready
 *      Second stage initialisation.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtready(void)
{
    int nrows, ncols;
    VCELL_t *cursor;
    int idx;

    ttready(TRUE);
    nrows = ttrows();
    ncols = ttcols();
    assert(nrows > 2 && nrows <= 250);
    assert(ncols > 2 && ncols <= 1000);

    if (vbuffer) {
        vm_destroy(&vmcombined);
        chk_free(vbuffer);
        chk_free(vscreen);
        chk_free(pscreen);
        chk_free(vflags);
        chk_free(vblanks);
        chk_free((void *)vtitle);
    }

    vm_init(&vmcombined, sizeof(vbyte_t)  * VCOMBINED_MAX, 0);
    vbuffer = chk_alloc(sizeof(VCELL_t)   * (nrows + 1) * 2 * ncols);
    vscreen = chk_alloc(sizeof(VCELL_t *) * nrows);
    pscreen = chk_alloc(sizeof(VCELL_t *) * nrows);
    vflags  = chk_alloc(sizeof(VFLAG_t)   * nrows);
    vblanks = chk_alloc(sizeof(VCELL_t)   * ncols);
    vtitle  = NULL;

    if (NULL == vbuffer || NULL == vscreen || NULL == pscreen ||
                NULL == vflags || NULL == vblanks) {
        panic("Cannot allocate screen buffer.");
        return;
    }

    memset(vbuffer, 0, sizeof(VCELL_t)   * 2 * (nrows + 1) * ncols);
    memset(vscreen, 0, sizeof(VCELL_t *) * nrows);
    memset(pscreen, 0, sizeof(VCELL_t *) * nrows);
    memset(vblanks, 0, sizeof(VCELL_t)   * ncols);
    memset(vflags, 0,  sizeof(VFLAG_t)   * nrows);

    cursor = vbuffer;
    for (idx = 0; idx < nrows; ++idx) {
        vscreen[idx] = cursor; cursor += ncols;
        pscreen[idx] = cursor; cursor += ncols;
        vflags[idx]  = 0;
    }

    vtblank();
    vline = vscreen[0];
}


/*  Function:           vtinited
 *      Determine whether the video/display is available/initialised.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if the video/display is initialised, otherwise *false*.
 */
int
vtinited(void)
{
    return (NULL != vline);
}


/*  Function:           vtis8bit
 *      Determine whether the video/display supports 8bit characters.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if the display supports 8bit characters.
 */
int
vtis8bit(void)
{
#if defined(HAVE_EIGHTBIT)
    int is8bit = 1;                             /* system default */
#else
    int is8bit = 0;
#endif

    if (DISPTYPE_7BIT == xf_disptype) {         /* not enabled */
        is8bit = 0;

    } else if (xf_disptype >= DISPTYPE_8BIT) {  /* explicit setting */
        is8bit = 1;

    } else if (DISPTYPE_UNKNOWN == xf_disptype) {
        if (x_pt.pt_8bit >= DISPTYPE_8BIT) {    /* auto-detection/terminal profile */
            is8bit = 1;
        } else if (x_pt.pt_encoding[0] && 0 == str_icmp(x_pt.pt_encoding, "8bit")) {
            is8bit = 1;
        }
    }
    return is8bit;
}


/*  Function:           vtisutf8
 *      Determine whether the video/display supports UTF8 characters.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if the display supports UTF-8 characters.
 *
 *  Notes:
 *      Either 8bit or encoding shall be setup by the TTY driver.
 */
int
vtisutf8(void)
{
    int isutf8 = 0;                             /* system default */

    if (DISPTYPE_UTF8 == xf_disptype) {         /* explicit setting */
        isutf8 = 1;

    } else if (DISPTYPE_UNKNOWN == xf_disptype) {
        if (DISPTYPE_UTF8 == x_pt.pt_8bit) {
            xf_disptype = DISPTYPE_UTF8;
            isutf8 = 1;
        }

        if (DISPTYPE_UNKNOWN == xf_disptype) {
            const char *encoding =              /* auto-detection/terminal profile */
                (x_pt.pt_encoding[0] ? x_pt.pt_encoding : sys_get_locale(TRUE));

            if (encoding) {
                if (mchar_locale_utf8(encoding)) {
                    xf_disptype = DISPTYPE_UTF8;
                    isutf8 = 1;
                }
            }
        }
    }
    return isutf8;
}


int
vtisunicode(void)
{
    if (DISPTYPE_UNKNOWN == xf_disptype) {
        vtisutf8();
    }
    return (DISPTYPE_UNICODE == xf_disptype);
}


/*  Function:           vtcharwidth
 *      Display character width.
 *
 *  Notes:
 *      logic *MUST* match winputch().
 *
 *  Parameters:
 *      ch - Character value.
 *      wcwidth - Width.
 *
 *  Returns:
 *      Character width.
 */
int
vtcharwidth(int ch, int wcwidth)
{
    if (DISPTYPE_UTF8 != xf_disptype) {         /* UTF8 */
        if (0 == ch || ch > 0xff) {
            /*
             *      SUBST:  ? or ?? (wcwidth specific)
             *      NCR     U+xxxx
             *      UCN:    \uxxxx
             *      HEX:    Uxxxx
             *      c99:    \uXXXX or \UXXXXXXXX
             *                  ISO/IEC 9899:1999, section 6.4.3
             */
            if (DISPUTF8_HEX & xf_utf8) {
                if (ch <= 0xffff) {
                    return 5;                   /* uxxxx */
                }
                return 7;                       /* Uxxxxxxx */
            } else if ((DISPUTF8_NCR | DISPUTF8_UCN) & xf_utf8) {
                if (ch <= 0xffff) {
                    return 6;                   /* \uxxxx or u+xxxx */
                }
                return 8;                       /* \Uxxxxxxx or U+xxxxxx */
            } else if (DISPUTF8_C99 & xf_utf8) {
                if (ch <= 0xffff) {
                    return 6;                   /* \uxxxx */
                }
                return 10;                      /* \Uxxxxxxxx */
            }
        } else {
            return cmapchr_width(cur_cmap, ch);
        }
    }
    return wcwidth;
}


/*  Function:           vtcolordepth
 *      Retrieve the current color-depth.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Color depth.
 */
int
vtcolordepth(void)
{
    return ttcolordepth();
}


const char *
vtdefaultscheme(void)
{
    return ttdefaultscheme();
}


/*  Function:           vtcolorscheme
 *      Signal the Update of the current color scheme.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtcolorscheme(int now, const char *name)
{
    __CUNUSED(name)
    if (now) {
        if (NULL != vbuffer) {
            vtblank();
            eredraw();
            vtgarbled();                        /* force screen update */
        }
    } else {
        ++vgarbled;                             /* force */
    }
}


/*  Function:           vtmapansi
 *      Map an ANSI 16x8 to its matching attribute.
 *
 *  Parameters:
 *      ansi - ANSI color.
 *
 *  Returns:
 *      Mapped color.
 */
vbyte_t
vtmapansi(vbyte_t ansi)
{
    const vbyte_t fg = PTY_FG_GET(ansi) & (ANSIFG_MAX - 1);
    const vbyte_t bg = PTY_BG_GET(ansi) & (ANSIBG_MAX - 1);

    return ATTR_ANSI128 + ((fg * ANSIBG_MAX) + bg);
}


/*  Function:           vtblank
 *      Blank the display.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
vtblank(void)
{
    const vbyte_t normal_space = (vbyte_t)(' ' | VBYTE_ATTR(ATTR_NORMAL));
    const int ncols = ttcols(), nrows = ttrows();
    int idx;

    for (idx = 0; idx < ncols; ++idx) {
        vblanks[idx].primary = normal_space;
    }
    ucopy(vblanks, pscreen[nrows - 1]);
    ucopy(vblanks, vscreen[nrows - 1]);
}


/*  Function:           vtgarbled
 *      Set the garbled status.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtgarbled(void)
{
    ++vgarbled;
}


/*  Function:           vtisgarbled
 *      Retrieve the display garbled status.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
int
vtisgarbled(void)
{
    return vgarbled;
}


/*  Function:           vtupdating
 *      Retrieve the display updating status.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
int
vtupdating(void)
{
    return vupdating;
}


/*  Function:           vtiscolor
 *      Retrieve the display color status.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if the terminal/display is color enabled.
 */
int
vtiscolor(void)
{
    if (-1 != xf_color) {
        return xf_color;                        /* user override */
    }
    return x_pt.pt_color;                       /* ... otherwise terminal specific */
}


/*  Function:           vtclose
 *      Shutdown the display.
 *
 *  Parameters:
 *      clear -             if *true* the screen is cleared.
 *
 *  Returns:
 *      nothing.
 */
void
vtclose(int clear)
{
    __CUNUSED(clear)
    ttclose();
    if (vbuffer) {                              /* local resources */
        chk_free(vbuffer); vbuffer = NULL;
        chk_free(vscreen); vscreen = NULL;
        chk_free(pscreen); pscreen = NULL;
        chk_free(vblanks); vblanks = NULL;
        chk_free(vflags); vflags = NULL;
        chk_free((void *)vtitle); vtitle = NULL;
    }
    vupdated = FALSE;
}


/*  Function:           vttitle
 *      Set the console title with local optimisation.
 *
 *  Parameters:
 *      title - Title buffer.
 *
 *  Returns:
 *      nothing.
 */
void
vttitle(const char *title)
{
    char buf[MAX_PATH + 16];

    if (NULL == title || 0 == xf_title) {
        return;
    }
    sxprintf(buf, sizeof(buf), "%s - %s", x_appname /*"cr"*/, title);
    if (NULL != vtitle && 0 == strcmp(vtitle, buf)) {
        return;
    }
    if (NULL == vtitle) {
        vtitle = chk_alloc(sizeof(buf));
    }
    strcpy((char *)vtitle, (const char *)buf);
    tttitle(vtitle);                            /* MCHAR??? */
}


/*  Function;           vtwinch
 *      Function called to scale windows after a window size change (SIGWINCH).
 *
 *      This is called when we run under a windowing system, eg. SunView or X-Windows
 *      and the terminal emulator window is changed. We try and rescale the windows so
 *      that the layout is the same, but we use the real-estate properly. It is not
 *      guaranteed to work under silly window resizes, but hopefully it'll work 99% of
 *      the time.
 *
 *  Parameters:
 *      oncol, onrow -      Previous column and row number.
 *
 *  Returns:
 *      nothing.
 */
void
vtwinch(int oncol, int onrow)
{
    const int   ncols   = ttcols(), nrows = ttrows();
    const int   diffcol = ncols - oncol;
    const float frac    = (float) (nrows - 1) / (float) (onrow - 1);
    WINDOW_t *wp, *last_wp, *next_wp;

    for (wp = window_first(); wp; wp = window_next(wp)) {
        int redge;

        /*
         *  Adjust window width only if its right hand edge touches the right
         *  hand edge of the window.
         */
        redge = win_redge(wp) + 1;
        if (redge >= oncol) {
            wp->w_w = (uint16_t)(wp->w_w + diffcol);
        }
        wp->w_old_line = -1;

        /*
         *  Scale the height of the window so that it keeps approximately
         *  the same fraction of the real estate.
         */
        wp->w_y = (uint16_t)((float)wp->w_y * frac);

        /*
         *  Rescaled tiled, other types are ignored
         */
        if (W_TILED == wp->w_type) {
            wp->w_h = (uint16_t)((float)(wp->w_h + 2) * frac - 2);
        }
    }

    /*
     *  Adjust for overlapping tiled window errors.
     */
    for (wp = window_first(); wp && NULL != (next_wp = window_next(wp)); wp = next_wp) {
        if (W_TILED != wp->w_type || W_TILED != next_wp->w_type) {
            continue;
        }

        /*
         *  If bottom of this window extends past top of next window and windows aren't
         *  side by side then set bottom of this window to line before top of next window.
         */
        if (wp->w_y + wp->w_h + 2 >= next_wp->w_y && wp->w_y != next_wp->w_y) {
            wp->w_h = (uint16_t)(next_wp->w_y - 1 - wp->w_y);
        }
    }

    /*
     *  If we've got some spare blank lines at the bottom of the screen then add the
     *  spare lines all windows which don't quite reach the bottom of the screen.
     */
    last_wp = wp;
    for (wp = window_first(); wp; wp = window_next(wp))
        if (W_TILED == wp->w_type) {
            last_wp = wp;
        }

    if (last_wp) {
        for (wp = window_first(); wp; wp = window_next(wp))
            if (W_TILED == wp->w_type &&
                    (wp->w_y + wp->w_h) == (last_wp->w_y + last_wp->w_h) && wp != last_wp) {
                wp->w_h = (uint16_t)(nrows - 3 - wp->w_y);
            }
        last_wp->w_h = (uint16_t)(nrows - 3 - last_wp->w_y);
    }

    ++vgarbled;
}


/*  Function:           vtmouseicon
 *      Mouse cursor implementation systems which require direct application.
 *
 *  Parameters:
 *      newx, newy - New cursor position.
 *      oldx, oldy - Previous cursor position.
 *
 *  Returns:
 *      none.
 */
void
vtmouseicon(int newy, int newx, int oldy, int oldx)
{
    if (vtmouse_chr) {
        vtmove(oldy, oldx);
        vtputb(vtmouse_chr);
        if (oldy != newy) {
            uupdate(oldy, FALSE);
        }
    }
    vtmove(newy, newx);
    vtmouse_chr = vline[vtcol].primary;
    vtmouse_row = newy;
    vtmouse_col = newx;
    vtputb((vtmouse_chr & 0x00ff) | 0xe300);
    uupdate(newy, FALSE);
    vtupdate_cursor();
}


/*  Function:           vtupdate2
 *      Refresh the display with the current window/buffer details.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtupdate2(int force)
{
    WINDOW_t *wp;
    BUFFER_t *bp;
    int hard = 0;

    if (vupdating || (x_display_enabled < 1) || !tty_open) {
        trace_log("vtupdate: not-ready\n");
        return;
    }
    trace_log("vtupdate:\n");
    ++vupdating;

    /*
     *  Basic optimisation,
     *      Upon characters within the i/o buffer delay current update request.
     *      Note that prompt/echo_line updates are addressed independently.
     */
    if (!force && io_typeahead()) {
        --vupdating;
        return;
    }

    /*
     *  tty resizing,
     *      delayed SIGWINCH as most of the code is not signal safe.
     */
    ttwinch();

    /*
     *  Assign TOP and implied HARD status.
     */
    if (curwp) {
        if (curbp) {
            ANCHOR_t anchor;

            anchor_get(curwp, curbp, &anchor);
            if (MK_COLUMN == anchor.type) {
                curwp->w_status |= WFHARD;      /* update marked region */
            }
            if (curwp->w_bufp == curbp) {
                if (BF2TST(curbp, BF2_CURSOR_ROW) || BF2TST(curbp, BF2_CURSOR_COL)) {
                    curwp->w_status |= WFHARD;  /* XXX - more specific test needed */
                }
            }
        }
        curwp->w_status |= WFTOP;
    }

    line_vstatus_update();                      /* virtual character update and syntax hook */ 

    /*
     *  Walk windows and draw image.
     */
    for (wp = window_first(); wp; wp = window_next(wp)) {
        /*
         *  Optimise refreshs,
         *      process only if updates are required,
         *      being any other flags other then just WFTOP.
         *
         *      o Update window content
         *      o Dirty all windows after this, because this could be a process
         *          window and we may obscure the current window.
         */
        if (vgarbled || hard) {
            wp->w_status |= WFHARD;
        }

        if (0 == WFTST(wp, WF_HIDDEN)) {
            if (wp->w_dialogp) {
                dialog_update(wp->w_dialogp, wp);
            }

            if ((wp->w_status && WFTOP != wp->w_status) ||
                    (wp->w_old_line != wp->w_line || wp->w_old_col != wp->w_col)) {
                if (wp->w_bufp) {
                    vtwindow(wp);
                    hard = 1;
                }
            }
        }

        wp->w_old_line = wp->w_line;
        wp->w_old_col  = wp->w_col;
        wp->w_mined    = wp->w_maxed = 0;
        wp->w_status   = 0;
    }

    /*
     *  Walk buffers, reset status.
     */
    for (bp = buf_first(); bp; bp = buf_next(bp)) {
        bp->b_dirty_min = bp->b_dirty_max = 0;
    }

    uflush();
    vtcursor_position();
    vtcursor_draw();
    ttflush();

    --vupdating;
}


/*  Function:           vtupdate
 *      Refresh the display with the current window/buffer details unless pending
 *      input events exists.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtupdate(void)
{
    vtupdate2(FALSE);
}


/*  Function:           vtupdate_bottom
 *      Refresh the contents of the bottom line.
 *
 *  Parameters:
 *      cursor - Cursor position on completion.
 *
 *  Returns:
 *      nothing.
 */
void
vtupdate_bottom(int cursor)
{
    const int bottom = (ttrows() - 1);

    if (!vupdated || (x_display_enabled < 1) || !tty_open) {
        return;
    }

    if (cursor >= 0) {                          /* prompt active */
        ++x_prompting;
        uupdate(bottom, (vgarbled ? TRUE : FALSE));
        vtcursorcol = cursor;
        vtcursorrow = -1;
        vtcursor_draw();
        --x_prompting;

    } else {                                    /* window active */
        const int lazy = (xf_lazyvt >= 0 ? xf_lazyvt : x_pt.pt_lazyupdate);

        if (vtcursor_position()) {
            if (curwp && curwp->w_bufp == curbp &&
                    (BF2TST(curbp, BF2_CURSOR_ROW) || BF2TST(curbp, BF2_CURSOR_COL))) {
                curwp->w_status |= WFHARD;
                vtupdate();
            }
        }
        if (lazy <= 0 || vgarbled || vdelayed++ > 10) {
            uupdate(bottom, (vgarbled ? TRUE : FALSE));
            vdelayed = 0;
        }
        vtcursor_draw();
    }
    ttflush();
}


/*  Function:           vtupdate_idle
 *      Idle time processing, refresh any dirty line images.
 *
 *  Note:
 *      Primary used to flush hilite updates delayed when vtlazy is enabled.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtupdate_idle(void)
{
    WINDOW_t *wp;

    if (vupdating || (x_display_enabled < 1) || !tty_open || NULL == curwp) {
        return;
    }
    ++vupdating;

    if (vdelayed) {
        uupdate(ttrows() - 1, FALSE);           /* bottom line */
        vdelayed = 0;
    }

    curwp->w_status |= WFTOP;                   /* windows */
    for (wp = window_first(); wp; wp = window_next(wp)) {
        const int top = win_tline(wp);
        const int bottom = win_bline(wp);

        draw_window(wp, top, wp->w_top_line, bottom, bottom, VTDRAW_DIRTY);
    }
    curwp->w_status &= ~WFTOP;

    uflush();
    vtcursor_position();
    vtcursor_draw();
    ttflush();
    --vupdating;
}


/*  Function:           vtupdate_cursor
 *      Refresh the display cursor position.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
vtupdate_cursor(void)
{
    if (vtcursor_position()) {
        vtcursor_draw();
    }
}


/*  Function:           vtmove
 *      Move virtual cursor to a point on the screen yet enforce existing screen size.
 *
 *  Parameters:
 *      row - cursor row.
 *      col - cursor column.
 *
 *  Returns:
 *      nothing.
 */
static void
vtmove(int row, int col)
{
    const int nrows = (ttrows() - 1), ncols = (ttcols() - 1);

    assert(row >= 0);
    assert(col >= 0);
    if (row > nrows) {
        row = nrows;
    }
    if (col > ncols) {
        col = ncols;
    }
    vline = vscreen[row];
    vflags[row] = VF_CHANGE;                    /* move to vputx() ??? */
    vtrow = row;
    vtcol = col;
}


/*  Function:           vpute
 *      Write the specified character 'ch' at the given echo-line column 'col'.
 *
 *  Parameters:
 *      ch - Character value.
 *      col - Cursor position on completion.
 *
 *  Returns:
 *      Resulting cursor location, accounting for wide-characters,
 *      otherwise -1 if off screen.
 */
int
vtpute(vbyte_t ch, int col)
{
    if (vscreen) {
        WChar_t wch = VBYTE_CHAR_GET(ch);
        const int width = (wch > 0xff ? Wcwidth(wch) : 1);

        if (col >= 0 && (col + width) <= ttcols()) {
            vcell_set(vscreen[ttrows() - 1] + col, ch);
            if (width >= 1) {
                ++col;
                if (width > 1) {
                    vcell_set(vscreen[ttrows() - 1] + col, CH_PADDING);
                    ++col;
                }
            }
            return col;
        }
    }
    return -1;  //off-screen
}


/*  Function:           vtputb
 *      Write the specified character 'ch' at the current column.
 *
 *      These ones should not be translated when in binary mode.
 *
 *      Upon the cursor exceeding the right margin, ignore the character.
 *
 *  Parameters:
 *      ch - Character value.
 *
 *  Returns:
 *      Zero if successful, otherwise -1 if beyond the end of line.
 */
static int
vtputb(const vbyte_t ch)
{
    if (vtcol < ttcols()) {
        vcell_set(vline + vtcol, ch);
        ++vtcol;
        return 0;
    }
    return -1;
}


/*  Function:           vtpushb
 *      Write character string to the video buffer.
 *
 *  Parameters:
 *      ch - Character value.
 *      col - Column.
 *
 *  Returns:
 *      nothing.
 */
static void
vtpushb(vbyte_t ch, int col)
{
    if (col < ttcols()) {
        vcell_push(vline + col, ch);
    }
}


/*  Function:           vtputs
 *      Write character string to the video buffer.
 *
 *  Parameters:
 *      s - String buffer.
 *      attr - Colour attribute.
 *
 *  Returns:
 *      nothing.
 */
static void
vtputs(const unsigned char *s, vbyte_t attr)
{
    if (s) {
        while (*s) {
            vtputb(*s++ | attr);
        }
    }
}


/*  Function:           vtcursor_position
 *      Calculate the cursor position.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if the cursor has moved, otherwise *false*.
 */
static int
vtcursor_position(void)
{
    const int orow = vtcursorrow, ocol = vtcursorcol;

    if (! x_prompting && curwp) {
        if (curwp->w_bufp == curbp) {
            vtcursorrow = win_tline(curwp) + (curwp->w_line - curwp->w_top_line);
            if (vtcursorrow < win_tline(curwp) || vtcursorrow > win_bline(curwp)) {
                vtcursorrow = vtcursorcol = -1;
                    /* Note: out-of-bounds, can occur during delayed updates. */

            } else {
                vtcursorcol = win_lcolumn(curwp) + ((curwp->w_col - curwp->w_left_offset) - 1);
                if (vtcursorcol < 0 || vtcursorcol >= ttcols()) {
                    vtcursorrow = vtcursorcol = -1;
                } else {
                    vthumb_position(curwp);
                    hthumb_position(curwp);
                }
            }
        }
    }
    return (orow != vtcursorrow || ocol != vtcursorcol);
}


/*  Function:           vtcursor_draw
 *      Update the cursor position.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
vtcursor_draw(void)
{
    if (x_prompting) {                          /* command line */
        if (vtcursorcol >= 0) {
            ttmove(ttrows() - 1, vtcursorcol);
        }
    } else {                                    /* current window */
        if (vtcursorrow >= 0 && vtcursorcol >= 0) {
            if (curwp) {
                vthumb_refresh(curwp);
                hthumb_refresh(curwp);
            }
            ttmove(vtcursorrow, vtcursorcol);
            ecursor(buf_imode(curbp));
        }
    }
}


/*  Function:           vtwindow
 *      Update the specified window, optimising where possible.
 *
 *  Parameters:
 *      wp - Window object address.
 *
 *  Returns:
 *      nothing.
 */
static __CINLINE LINENO
lineno_min(LINENO a, LINENO b)
{
    return (a <= b ? a : b);
}

static __CINLINE LINENO
lineno_max(LINENO a, LINENO b)
{
    return (a >= b ? a : b);
}


static void
vtwindow(WINDOW_t *wp)
{
    const BUFFER_t *bp  = wp->w_bufp;           /* associated buffer */
    const cmap_t *cmap  = wp->w_cmap;           /* user otherwise default_cmp */
    const LINENO old_line = wp->w_old_line;
    const int top       = win_tline(wp);
    const int bottom    = win_bline(wp);
    const int height    = win_height(wp);
    const int height2   = height / 2;
    const vbyte_t nattr = normalcolor(wp);
    int width           = win_width(wp);

    LINENO top_line     = wp->w_top_line;
#define BOTTOMLINE()        ((top_line + height) - 1)

    LINENO bottom_line  = BOTTOMLINE();
    LINENO line         = wp->w_line;
    LINENO left_offset  = wp->w_left_offset;
    uint32_t status     = wp->w_status;
    uint32_t flags      = wp->w_disp_flags;

#define WFD_LINEBITS        0x0001
#define WDF_DIAG            0x0002
#define WDF_OFFSET          0x0004
#define WDF_LITERAL         0x0008

#define WDF_LINENO          0x0010
#define WDF_LINEOLD         0x0020

#define WDF_EOF             0x0100
#define WDF_TILDE           0x0200
#define WDF_EOL_HILITE      0x0400              /* limit hilites to EOL */
#define WDF_DIALOG          0x0800              /* dialog resource */

#define WDF_HIMODIFIED      0x1000              /* hilite modified lines */
#define WDF_HIADDITIONAL    0x2000              /* hilite additional lines */
#define WDF_HICOLUMN        0x4000              /* hilite column active */

    int minrows   = (x_display_minrows <= 0 ? 0 : (x_display_minrows >= height2 ? height2-1 : x_display_minrows));
    int mincols   = (x_display_mincols <= 0 ? 0 : (x_display_mincols >= width/2 ? (width/2)-1 : x_display_mincols));
    int lmargin   = wp->w_disp_lmargin;
    int rmargin   = wp->w_disp_rmargin;
    int scrolled  = 0;
    int diff;

    /*
     *  Build display parameters/
     *      o left margin, auto sized based on line of lines in underlying buffer.
     */
    if (bp) {
        const int has_status = BF2TST(bp, BF2_LINE_STATUS)   || WFTST(wp, WF_LINE_STATUS);
        const int has_lines  = BF2TST(bp, BF2_LINE_NUMBERS)  || WFTST(wp, WF_LINE_NUMBERS);
        const int has_eof    = BFTST(bp,  BF_EOF_DISPLAY)    || WFTST(wp, WF_EOF_DISPLAY);
        const int has_tilde  = BF2TST(bp, BF2_TILDE_DISPLAY) || WFTST(wp, WF_TILDE_DISPLAY);

        flags = 0;
        lmargin = 0;                            /* (re)create left margin */

        if (bp->b_cmap) {
            cmap = bp->b_cmap;                  /* buffer cmap has priority */
        }

        if (has_status) {
            flags |= WFD_LINEBITS;
            ++lmargin;                          /* modification/diff status */
        }

        if (ruler_colorcolumn(bp) > 0) {
            rmargin = ruler_colorcolumn(bp);
            flags |= WDF_HICOLUMN;
        }

        if (BF2TST(bp, BF2_DIALOG)) {
            minrows = mincols = 0;
            flags |= WDF_DIALOG;
        }

        if ((x_dflags || BF2TST(bp, BF2_LINE_SYNTAX)) && (has_lines || has_status)) {
            flags |= WDF_DIAG;
            lmargin += 3;                       /* line flag diagnostics */
        }

        if (has_lines) {
            if (BFTST(bp, BF_BINARY) && bp->b_bin_chunk_size) {
                flags |= WDF_OFFSET;
                lmargin += 8 + 1;               /* hex offset */
            } else {
                if (BF2TST(bp, BF2_LINE_OLDNUMBERS)) {
                    flags |= WDF_LINEOLD;
                }
                flags |= WDF_LINENO;

                if (x_display_numbercols > 0) { /* user */
                    lmargin = x_display_numbercols;

                                                /* terminal default */
                } else if (x_pt.pt_lineno_columns > 0) {
                    lmargin = x_pt.pt_lineno_columns;

                } else  {                       /* otherwise dynamic */
                    const LINENO numlines = bp->b_numlines;

                    if (numlines < 9900) {
                        lmargin += 4;
                    } else if (numlines < 99900) {
                        lmargin += 5;
                    } else {
                        lmargin += 8;
                    }
                }
                ++lmargin;                      /* spacer */
                if (lmargin > (2*(width/3))) {  /* limit 2/3 window width */
                    lmargin = 2*(width/3);
                }
            }
        }

        if (has_eof) {
            flags |= WDF_EOF;                   /* [EOF] */
        } else if (has_tilde) {
            flags |= WDF_TILDE;                 /* ~ */
        }

        if (WFTST(wp, WF_EOL_HILITE)   || BF2TST(bp, BF2_EOL_HILITE)) {
            flags |= WDF_EOL_HILITE;
        }

        if (WFTST(wp, WF_HIMODIFIED)   || BF2TST(bp, BF2_HIMODIFIED)) {
            flags |= WDF_HIMODIFIED;
        }

        if (WFTST(wp, WF_HIADDITIONAL) || BF2TST(bp, BF2_HIADDITIONAL)) {
            flags |= WDF_HIADDITIONAL;
        }
    }

    if (lmargin > LMARGIN) {
        lmargin = LMARGIN;                      /* upper limit */
    }

    if (lmargin != wp->w_disp_lmargin || rmargin != wp->w_disp_rmargin ||
            flags != wp->w_disp_flags || cmap != wp->w_disp_cmap) {
        wp->w_disp_lmargin = lmargin;
        wp->w_disp_rmargin = rmargin;
        wp->w_disp_flags = flags;
        wp->w_disp_cmap = (cmap ? cmap : x_default_cmap);
        status |= WFHARD|WFMARGIN;              /* on change, redraw */
    }

    wp->w_disp_ansicolor = ANSICOLOR_MK(WHITE, BLACK);
    wp->w_disp_ansiflags = ANSI_MANORMAL;
    width -= lmargin;                           /* strip from total width */

    /*
     *  Scroll jump/normalisation
     */
    if (WFMOVE == ((WFPAGE|WFMOVE) & status) && 0 == (WDF_DIALOG & flags)) {
        if (old_line == line) {                 /* horizontal movement */
            if (x_display_scrollcols > 1 && mincols < (width / 3)) {
                const int jump =
                    (x_display_scrollcols >= width/2 ? width/2 : x_display_scrollcols);

                if (wp->w_col == left_offset) {
                    if ((left_offset -= jump) < 0) {
                        left_offset = 0;
                    }
                } else if (wp->w_col == (left_offset + width + 1)) {
                    left_offset += jump;
                }
            }

        } else {                                /* vertical movement */
            if (x_display_scrollrows > 1 && minrows < (height / 3)) {
                const int jump =
                    (x_display_scrollrows >= height2 ? height2 : x_display_scrollrows);

                if (line == ((top_line + minrows) - 1)) {
                    if ((line -= jump) < 1) {
                        line = 1;
                    }
                } else if (line == ((bottom_line - minrows) + 1)) {
                    line += jump;
                }
            }
        }
    }

    /*
     *  Sideways scrolling/boundary check
     */
    if ((wp->w_col - left_offset) > width) {
        if ((left_offset = wp->w_col - (int)width) < 0) {
            left_offset = 0;
        }
    } else if (wp->w_col <= left_offset) {
        if ((left_offset = wp->w_col - 1) < 0) {
            left_offset = 0;
        }
    }

    if (left_offset != wp->w_left_offset) {     /* indent change, redraw */
        wp->w_left_offset = left_offset;
        status |= WFHARD;
    }

    if (top_line < 1) {
        top_line = 1;                           /* normalise to top-of-buffer */
        bottom_line = BOTTOMLINE();
    }

#define FULLSCREEN() \
    (0 == (status & WFHARD) && 0 == x_window_popups && width >= ttcols() - (xf_borders * 2))

    trace_log("vtwindow(title:%s, line:%d, minrows:%d, old_line;%d, top_line:%d, bottom_line:%d, height2:%d)\n",\
        (wp->w_title ? wp->w_title : "n/a"), (int)line, (int)minrows, (int)old_line, \
            (int)top_line, (int)bottom_line, (int)height2);

    /*
     *  Update optimisation scroll up,
     *      optimise the display updates using available scroll commands.
     *      insert scroll and then force the display of the missing lines (diff).
     */
    if (old_line >= top_line && old_line <= (top_line + minrows) &&
            line < (top_line + minrows) && line > (top_line - height2) && top_line > 1 && FULLSCREEN() &&
                ttinsl(top, bottom, diff = ((top_line - line) + (line >= minrows ? minrows : 0)), nattr)) {
        int d;

        trace_log("vtwindow:ttinsl(top:%d, bottom:%d, diff:%d)\n", top, bottom, (int)diff);

        assert(diff >= 1);
        scrolled = diff;                        /* scrolled arena size */
        top_line -= diff;
        for (d = 0; d++ < diff;) {
            uscrolldown(top, bottom, nattr);    /* reflect to physical */
        }
        assert(top_line >= 1);
        status |= WFEDIT;

    /*
     *  Update optimisation scroll down,
     *      optimise the display updates using available scroll commands.
     *      delete scroll and then force the display of the missing lines (diff).
     */
    } else if (old_line >= (bottom_line - minrows) && old_line <= bottom_line &&
            line > (bottom_line - minrows) && line < (bottom_line + height2) && FULLSCREEN() &&
                ttdell(top, bottom, diff = line - (bottom_line - minrows), nattr)) {
        int d;

        trace_log("vtwindow:ttdell(top:%d, bottom:%d, diff:%d)\n", top, bottom, (int)diff);

        assert(diff >= 1);
        scrolled = -diff;                       /* scrolled arena size */
        top_line += diff;
        for (d = 0; d++ < diff;) {              /* reflect to physical */
            uscrollup(top, bottom, nattr);
        }
        assert(top_line >= 1);
        status |= WFEDIT;

    /*
     *  Outside current display arena, reposition the window,
     *      attempt to keep the cursor stationary (ie. relative to top_line)
     *      obey minrow settings force a refresh.
     */
    } else if (line < (top_line + minrows) || line > (bottom_line - minrows)) {
        const LINENO relative = old_line - top_line;

        if (relative < 0 || line <= height2) {
            if ((top_line = (line - minrows)) < 1) {
                top_line = 1;
            }
        } else {                                /* relative adjust, keep cursor stationary */
            if (relative >= (height - minrows)) {
                top_line = line;

            } else if ((top_line = line - relative) < 1) {
                top_line = 1;

            } else if (line < (top_line + minrows)) {
                top_line = (line - minrows);

            } else if (line > (BOTTOMLINE() - minrows)) {
                top_line = (line - height) + minrows + 1;
            }
        }

        trace_log("vtwindow:outside(relative:%d) == top_line:%d\n", (int)relative, (int)top_line);
        assert(top_line >= 1);
        assert(line >= top_line);
        assert(line <= BOTTOMLINE());
        status |= WFHARD;

    /*
     *  Line deletion.
     */
    } else if (WFDELL == (status & (WFDELL|WFHARD))) {
        if (WFDELL == status && FULLSCREEN()) {
            const LINENO relative = old_line - top_line;

            if (relative > 0 && ttdell(top + relative, bottom, 1, nattr)) {
                trace_log("vtwindow:delete(top:%d, bottom:%d, relative:%d)\n", top, bottom, (int)relative);
                uscrollup(top + relative, bottom, nattr);
                draw_window(wp, bottom, bottom_line, bottom, bottom, VTDRAW_SCROLLED);
                return;
            }
        }
        status |= WFHARD;                       /* no ttdel() or compound changes */

    /*
     *  Line insertion.
     */
    } else if (WFINSL == (status & (WFINSL|WFHARD))) {
        /*
         *  case of user hitting <Enter>.
         *
         *      Be very conservative that we capture only an <Enter> and not a combination
         *      of other editing actions, (e.g from a playback macro).
         */
        const LINENO relative = line - top_line;

        if (wp->w_maxed == (wp->w_mined + 1) && 0 == left_offset && relative > 0 &&
                wp->w_line + (wp->w_h / 3) <= wp->w_bufp->b_numlines && relative < (wp->w_h * 3) / 4 &&
                FULLSCREEN() && ttinsl(top + relative, bottom, 1, nattr)) {
            const int ins = top + relative;

            uscrolldown(ins, bottom, nattr);
            draw_window(wp, ins - 1, wp->w_line - 1, ins, bottom, VTDRAW_SCROLLED);
            return;
        }
        status |= WFHARD;
    }

    /*
     *  Generic updates.
     */
    assert(top_line >= 1);
    wp->w_top_line = top_line;
    bottom_line = BOTTOMLINE();
    assert(line <= bottom_line);

    if (WFHARD & status) {                      /* major change/garbled, redraw */
        trace_log("vtwindow:hard(top:%d, bottom:%d\n", top, bottom);
        draw_window(wp, top, top_line, bottom, bottom, VTDRAW_REPAINT);

    } else if ((WFEDIT & status) || bp->b_dirty_min) { /* line edit or scrolled, update mined arena */
        const LINENO diff2 = top - top_line;
        LINENO mined = wp->w_mined, maxed = wp->w_maxed;

        if (bp) {                               /* inherit buffer changes */
            if (bp->b_syntax_min) {
                mined = lineno_min(mined, bp->b_syntax_min);
                maxed = lineno_max(maxed, bp->b_syntax_max);
            }

            if (bp->b_dirty_min) {
                mined = lineno_min(mined, bp->b_dirty_min);
                maxed = lineno_max(maxed, bp->b_dirty_max);
            }
        }

        if (scrolled) {
            if (scrolled > 0) {                 /* redraw top */
                mined = lineno_min(mined, top_line);
                maxed = lineno_max(maxed, top_line + scrolled);
            } else {                            /* redraw bottom */
                mined = lineno_min(mined, bottom_line + scrolled);
                maxed = lineno_max(maxed, bottom_line);
            }
        }

        mined = lineno_max(mined, top_line);
        maxed = lineno_min(maxed, bottom_line);
        assert(mined <= maxed);

        trace_log("vtwindow:draw_window(scrolled:%d, top:%d, bottom:%d)\n", scrolled, mined + diff2, bottom);
        draw_window(wp, mined + diff2, mined, maxed + diff2, bottom, VTDRAW_LAZY);

    } else if (old_line != line) {              /* cursor line change, flush dirty/lazy lines */

        trace_log("vtwindow:draw_lines(top:%d, bottom:%d)\n", top, bottom);
        draw_window(wp, top, top_line, bottom, bottom, VTDRAW_DIRTY);

    }
}


/*  Function:           uscrolldown
 *      Scroll down the screen by one line.
 *
 *  Parameters:
 *      top - Top line.
 *      bottom - Bottom of the scroll region.
 *      attr - Fill colour attribute.
 *
 *  Returns:
 *      nothing.
 */
static void
uscrolldown(int top, int bottom, vbyte_t attr)
{
    const vbyte_t fill = (vbyte_t) (' ' | VBYTE_ATTR(attr));
    const int ncols = ttcols();

    VCELL_t **bp = pscreen + bottom;
    VCELL_t **bv = vscreen + bottom;
    VFLAG_t *fp  = vflags  + bottom;
    VCELL_t *bp1 = *bp;
    VCELL_t *bv1 = *bv;
    int     idx  = bottom;
    WINDOW_t *wp;

    assert(top >= 0);
    assert(top <= bottom);
    assert(bottom < ttrows());

    for (wp = window_first(); wp; wp = window_next(wp)) {
        if (wp->w_vthumb.t_curr) {
            if (wp->w_vthumb.t_curr >= top && wp->w_vthumb.t_curr <= bottom) {
                if (++wp->w_vthumb.t_curr > bottom) {
                    wp->w_vthumb.t_curr = 0;    /* top = scroll wrap? */
                }
            }
        }
    }

    while (idx-- > top) {
        --bp; bp[1] = bp[0];
        --bv; bv[1] = bv[0];
        --fp; fp[1] = fp[0];
    }

    *bp = bp1;
    *bv = bv1;
    *fp = 0;

    for (idx = 0; idx < ncols; ++idx) {
        vcell_set(bp1, fill); ++bp1;
        vcell_set(bv1, fill); ++bv1;
    }
}


/*  Function:           uscrollup
 *      Scroll up the screen by one line.
 *
 *  Parameters:
 *      top - Top line.
 *      bottom - Bottom of the scroll region.
 *      attr - Fill colour attribute.
 *
 *  Returns:
 *      nothing.
 */
static void
uscrollup(int top, int bottom, vbyte_t attr)
{
    const vbyte_t fill = (vbyte_t) (' ' | VBYTE_ATTR(attr));
    const int ncols = ttcols();

    VCELL_t **bp = pscreen + top;
    VCELL_t **bv = vscreen + top;
    VFLAG_t *fp  = vflags  + top;
    VCELL_t *bp1 = *bp;
    VCELL_t *bv1 = *bv;
    int     idx  = top;
    WINDOW_t *wp;

    assert(top >= 0);
    assert(top <= bottom);
    assert(bottom < ttrows());

    for (wp = window_first(); wp; wp = window_next(wp)) {
        if (wp->w_vthumb.t_curr) {
            if (wp->w_vthumb.t_curr >= top && wp->w_vthumb.t_curr <= bottom) {
                if (--wp->w_vthumb.t_curr < top) {
                    wp->w_vthumb.t_curr = 0;    /* bottom = scroll wrap? */
                }
            }
        }
    }

    while (idx++ < bottom) {
        bp[0] = bp[1]; ++bp;
        bv[0] = bv[1]; ++bv;
        fp[0] = fp[1]; ++fp;
    }

    *bp = bp1;
    *bv = bv1;
    *fp = 0;

    for (idx = 0; idx < ncols; ++idx) {
        vcell_set(bp1, fill); ++bp1;
        vcell_set(bv1, fill); ++bv1;
    }
}


/*  Function:           uflush
 *      Flush the current virtual display.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
uflush(void)
{
    const int nrows = (ttrows() - 1);           /* edit arena size */
    register int idx;

    /*
     *  edit arena/
     *      either complete or modified lines.
     */
    if (vgarbled) {
        ttclear();
        if (x_scrfn.scr_control) {
            (*x_scrfn.scr_control)(SCR_CTRL_GARBLED, 0);
        }
        for (idx = 0; idx < nrows; ++idx) {
            uupdate(idx, TRUE);
            vflags[idx] = 0;
        }
        ucopy(vblanks, pscreen[idx]);
        vtmouse_chr = 0;
        eredraw();

    } else {
        for (idx = 0; idx < nrows; ++idx) {
            if (VF_CHANGE & vflags[idx]) {
                uupdate(idx, FALSE);
            }
            vflags[idx] = 0;
        }
        elinecol(0);
    }
    vgarbled = 0;
    vupdated = TRUE;
}


/*  Function:           uputchar
 *      Put a character on the screen, possibly moving cursor first.
 *
 *      Also if cursor is on same line and 'not far' from the current character,
 *      it may be better to paint the characters in between rather than trying
 *      to move the cursor.
 *
 *  Parameters:
 *      c - Character value.
 *
 *  Returns:
 *      nothing.
 */
static void
uputchar(const vbyte_t c)
{
    VCELL_t cell = {0};
    cell.primary = c;
    uputcell(&cell);
}


/*  Function:           uputcell
 *      Put a character cell on the screen, possibly moving cursor first.
 *
 *      Also if cursor is on same line and 'not far' from the current character,
 *      it may be better to paint the characters in between rather than trying
 *      to move the cursor.
 *
 *  Parameters:
 *      cell - Character cell value.
 *
 *  Returns:
 *      nothing.
 */
static void
uputcell(const VCELL_t *cell)
{
    if (CH_PADDING == VBYTE_CHAR_GET(cell->primary)) {
        return;                                 /* wide character padding */
    }

    if (ucol >= 0) {
        int atcol;

        if (ttatrow() == urow &&
                ucol > (atcol = ttatcol()) && ucol < (atcol + 5)) {
            /*
             *  minor cursor change, redraw.
             */
            const VCELL_t *vvp;

            vvp = vscreen[urow] + atcol;
            while (ttatrow() == urow && ttatcol() < ucol) {
                vcell_put(vvp++);
            }
            vcell_put(cell);
            ucol = -1;
            return;
        }
        ttmove(urow, ucol);
    }
    vcell_put(cell);
}


/*  Function:           umove
 *      Set the cursor position.
 *
 *  Parameters:
 *      row - Cursor row.
 *      col - Cursor column.
 *
 *  Returns:
 *      nothing.
 */
static void
umove(int row, int col)
{
    ucol = col;
    urow = row;
}


/*  Function:           uupdate
 *      Routine to update a single row. Called from mouse driver to update
 *      the line where the cursor has moved from/to
 *
 *  Parameters:
 *      row - Cursor row.
 *      eflag - Erase flag.
 *
 *  Returns:
 *      nothing.
 */
static void
uupdate(int row, int eflag)
{
    if (eflag) {
        ucopy(vblanks, pscreen[row]);
    }
    uline(row, vscreen[row], pscreen[row], eflag);
    ucopy(vscreen[row], pscreen[row]);
}


/*  Function:           uline
 *      Export any line changes to the tty driver.
 *
 *  Parameters:
 *      row - Physical row 0...max-1.
 *      vvp - Virtual pointer.
 *      pvp - Physical pointer.
 *      eflag - Erase flag (if the display was garbled we ttclear()
 *          prior to update hence can assume the display is cleared).
 *
 *  Returns:
 *      nothing.
 */
static void
uline(const int row, const VCELL_t *vvp, VCELL_t *pvp, int eflag)
{
    const int bottom_line = (row >= (ttrows() - 1)) ? 1 : 0;
    const int cols        = ttcols();

    const VCELL_t *bp     = pvp;
    const VCELL_t *leftv  = vvp;
    const VCELL_t *rightv;

    vbyte_t spaceattr = (vbyte_t)-1;
    int spacechar, right, left;

    ED_TERM3(("uline(row:%d, bot:%d, eflag:%d)\n", row, bottom_line, eflag))
    assert(pvp != vblanks);

    /*
     *  Look for the left most match.
     */
    left = cols;
    while (left >= 1 && vcell_compare(leftv, bp)) {
        ++leftv, ++bp;
        if (--left <= 0) {
            return;                             /* everything matches */
        }
    }

    /*
     *  Look for the right most match.
     */
    spacechar = -cols;
    rightv = vvp + cols;
    bp = pvp + cols;

    while (vcell_compare(--rightv, --bp)) {
        if (spacechar < 0) {
            if (vcell_space(rightv, spaceattr)) {
                if ((vbyte_t)-1 == spaceattr) {
                    spaceattr = VBYTE_ATTR_GET(rightv->primary);
                }
                ++spacechar;                    /* normal character */
            } else {
                spacechar = -spacechar;         /* last non-normal character */
            }
        }
    }
    ++rightv, ++bp;

    left  = leftv - vvp;
    right = rightv - vvp;

    assert(left  >= 0);
    assert(right <= cols);
    assert(right > left);

    /*
     *  Display hook.
     */
    if (x_scrfn.scr_print) {
        (*x_scrfn.scr_print)(row, left, right - left, vvp + left);
        return;
    }

    /*
     *  Determine space attribute ....
     */
    while (spacechar < 0) {                     /* space not found yet, keep looking */
        if (vcell_space(--rightv, spaceattr)) {
            if ((vbyte_t)-1 == spaceattr) {
                spaceattr = VBYTE_ATTR_GET(rightv->primary);
            }
            ++spacechar;
        } else {
            spacechar = -spacechar;             /* last non-normal character */
        }
    }

    if ((vbyte_t)-1 == spaceattr) spaceattr = ATTR_NORMAL;
    assert(spacechar >= 0 && spacechar <= cols);

    ED_TERM3(("\tuline(left:%d,right:%d,spacechar:%d,spaceattr:0x%x)\n", left, right, spacechar, spaceattr))

    /*
     *  Otherwise attempt to optimise the output operations.
     *      while feasible to perform non-space repeat optimisation, testing has shown little
     *      advantage due to limited terminal-repeat support for regions not using the
     *      default background.
     */
    while (left < right) {
        const VCELL_t *vv = vvp + left;
        VCELL_t *ppend, *pp = pvp + left;
        int oleft = left, bytes = right - left;

        while (bytes > 0) {                     /* next different between virtual and physical image */
            if (! vcell_compare(vv++, pp++)) {
                break;
            }
            --bytes, ++left;
        }

        if (bytes <= 0) {
            break;                              /* none found */
        }

        umove(row, left);
        if (! vcell_space(vvp + left, spaceattr)) {
            uputcell(vvp + left++);             /* not a 'normal' space echo and continue */
            continue;
        }

        oleft = left;
        while (vcell_space(vvp + left, spaceattr) && left < right) {
            ++left;                             /* count consecutive 'normal' spaces we have */
        }

        if (eflag) {
            continue;                           /* already "cleared", nothing todo */
        }

        /*
         *  If minimal sized area then space fill, unless on the bottom line in which case
         *  always erase which shall/may allow us us to "fill" in the last screen column under
         *  terminals with limitations.
         */
        if (! bottom_line) {
            const int repeat = left - oleft;

            if (repeat < 12 || repeat < (cols - right)) {
                const vbyte_t space = (vbyte_t)(' ' | VBYTE_ATTR(spaceattr));

                ED_TERM3(("\tuline:repeat(cnt:%d, left:%d)\n", repeat, left))
                uputchar(VBYTE_ATTR(spaceattr));
                if (ttrepeat(repeat, space, WHERE_DONTCARE)) {
                    while (oleft < left) {
                        vcell_set(pvp + oleft, space);
                        ++oleft;
                    }
                    continue;
                }
            }
        }

        /*
         *  erase to end of line, set 'right' to the right-most non normal-space character.
         */
        right = spacechar;                      /* redraw up-to last non-blank */

        if (bottom_line && ! ttlastsafe()) {
            if (right >= cols) {
                --right;                        /* unsafe, ignore bottom-right corner */
            }
        }

        /*
         *  Erase to the end-of-line in the normal color combination and
         *  update physical buffer to reflect erase-end-of-line operation.
         */
        ED_TERM3(("\tuline:eeol(oleft:%d, left:%d)\n", oleft, left))
        uputchar(VBYTE_ATTR(spaceattr));
        if (tteeol()) {
            const vbyte_t space = (vbyte_t)(' ' | VBYTE_ATTR(spaceattr));

            for (pp = pvp + oleft, ppend = pvp + cols; pp < ppend; ++pp) {
                vcell_set(pp, space);
            }
            umove(row, left);
            eflag = TRUE;                       /* erase-end-of-line done */

        } else {
            while (oleft < left) {
                vcell_put(vvp + oleft);
                ++oleft;
            }
        }
    }

    ucol = -1;                                  /* ??? - combined only */
}


/*  Function:           ucopy
 *      Copy a line content into the specified buffer.
 *
 *  Parameters:
 *      vvp - Source.
 *      pvp - Destination.
 *
 *  Returns:
 *      nothing.
 */
static void
ucopy(const VCELL_t *vvp, VCELL_t *pvp)
{
    const int ncols = ttcols();
    int idx;

    for (idx = 0; idx < ncols; ++idx) {
        vcell_copy(pvp++, vvp++);
    }
}


static void
vcell_set(VCELL_t *cell, const vbyte_t ch)
{
    assert(VBYTE_ATTR_GET(ch) <= ATTR_MAX);

    cell->primary = ch;
    if (cell->combined) {
        if (-1 == vm_unreference(&vmcombined, cell->combined)) {
            vm_free(&vmcombined, cell->combined);
        }
        cell->combined = NULL;
    }
}


static void
vcell_copy(VCELL_t *cell, const VCELL_t *src)
{
    vcell_set(cell, src->primary);
    if (src->combined) {
        if (vm_reference(&vmcombined, src->combined) > 0) {
            cell->combined = src->combined;
        } else {
            if (NULL != (cell->combined = vm_new(&vmcombined))) {
                memcpy(cell->combined, (const void *)src->combined, sizeof(vbyte_t) * VCOMBINED_MAX);
            }
        }
    }
}


static void
vcell_push(VCELL_t *cell, const vbyte_t ch)
{
    assert(cell->primary);
    if (ch) {
        vbyte_t *combined;

        if (NULL == (combined = cell->combined)) {
                                                /* new node, allocate */
            if (NULL != (combined = vm_new(&vmcombined))) {
                cell->combined = combined;
                memset(combined + 1, 0, sizeof(vbyte_t) * (VCOMBINED_MAX - 1));
                combined[0] = ch;
            }

        } else {                                /* exist, push onto combined list */
            unsigned idx;

            for (idx = 0; idx < VCOMBINED_MAX; ++idx) {
                if (0 == combined[idx]) {
                    combined[idx] = VBYTE_CHAR_GET(ch);
                    break;
                }
            }
        }
    }
}


static int
vcell_compare(const VCELL_t *a, const VCELL_t *b)
{
    if (a->primary == b->primary) {
        if (a->combined == b->combined) {
            return TRUE;
        }
    }
    return FALSE;
}


#if defined(NOT_USED)
static int
vcell_repeat(const VCELL_t *cell, const vbyte_t ch)
{
    if (VBYTE_CHAR_GET(ch) < 0x7f &&
            ch == cell->primary && NULL == cell->combined) {
        return TRUE;
    }
    return FALSE;
}
#endif


static int
vcell_space(const VCELL_t *cell, const vbyte_t spaceattr)
{
    const vbyte_t primary = cell->primary;

    if (' ' == VBYTE_CHAR_GET(primary) && NULL == cell->combined) {
        return ((vbyte_t)-1 == spaceattr || spaceattr == VBYTE_ATTR_GET(primary));
    }
    return FALSE;
}


static void
vcell_put(const VCELL_t *cell)
{
    const vbyte_t ch = cell->primary;

    (*x_scrfn.scr_putc)(ch);

    if (cell->combined) {
        const vbyte_t attr = (ch & VBYTE_ATTR_MASK);
        vbyte_t cch;
        unsigned idx;

        for (idx = 0; 0 != (cch = cell->combined[idx]) && idx < VCOMBINED_MAX; ++idx) {
            (*x_scrfn.scr_putc)(cch | attr);
        }
    }
}


static void
vthumb_position(WINDOW_t *wp)
{
    int value = window_ctrl_test(wp, WCTRLO_VERT_SCROLL);

    if (value) {
        BUFFER_t *bp;

        if (! hasborders(wp) || NULL == (bp = wp->w_bufp)) {
            value = 0;                          /* not visible */

        } else {
            const int height = win_height(wp);
            const int vmin = win_tline(wp);
            const int vmax = win_bline(wp);

            if (window_ctrl_test(wp, WCTRLO_USER_VTHUMB)) {
                if ((value = wp->w_ctrl_vthumb) < 0) {
                    value = 0;                  /* hide */
                } else {
                    if ((value += vmin) > vmax) {
                        value = vmax;
                    }
                    assert(value < ttrows());
                }

            } else {
                const LINENO lines = bp->b_numlines;

                if (lines <= height) {
                    value = 0;                  /* auto-hide */
                } else {
                    value = vmin +
                        (int)(((accint_t)wp->w_line * height) / (lines + 1));
                    if (value > vmax) {
                        value = vmax;
                    }
                    assert(value > 0);
                    assert(value < ttrows());
                }
            }
        }
    }
    wp->w_vthumb.t_value = (uint16_t)value;
}


static void
hthumb_position(WINDOW_t *wp)
{
    int value = window_ctrl_test(wp, WCTRLO_HORZ_SCROLL);

    if (value) {
        BUFFER_t *bp;

        if (! hasborders(wp) || NULL == (bp = wp->w_bufp) || wp->w_message) {
            value = 0;                          /* not visible */

        } else {
            const int hmin  = win_fcolumn(wp);
            const int hmax  = win_rcolumn(wp);
            const int width = win_uwidth(wp);
            const LINENO length = buf_line_length(bp, FALSE);

            if (length <= width && 0 == wp->w_left_offset) {
                value = 0;                      /* auto-hide */
            } else {
                value = hmin +
                    (int)(((accint_t)wp->w_col * width) / (length + 1));
                if (value >= (ttcols() - 1)) {
                    value = ttcols() - 2;
                } else if (value > hmax) {
                    value = hmax;
                }
            }
        }
    }
    wp->w_hthumb.t_value = (uint16_t)value;
}


static void
vthumb_refresh(WINDOW_t *wp)
{
    const uint16_t value =
            (uint16_t)(wp->w_vthumb.t_active ? wp->w_vthumb.t_value : 0);
    const uint16_t curr  = wp->w_vthumb.t_curr;
    uint16_t prev = wp->w_vthumb.t_prev;

    if (value || prev) {                        /* active value or previous */
        const vbyte_t scrollcol = scrollcolor(wp);
        const vbyte_t thumbcol = thumbcolor(wp);
        const int redge = win_redge(wp);

        if (value && value != curr) {
            if (0 == curr) {                    /* new */
                vtmove(value, redge);
                vtputb(thumbcol  | CH_VTHUMB);
            } else {                            /* update, optimise top->bottom */
                if (value < curr) {
                    vtmove(value, redge);
                    vtputb(thumbcol  | CH_VTHUMB);
                    vtmove(curr,  redge);
                    vtputb(scrollcol | CH_VSCROLL);
                } else {
                    vtmove(curr,  redge);
                    vtputb(scrollcol | CH_VSCROLL);
                    vtmove(value, redge);
                    vtputb(thumbcol  | CH_VTHUMB);
                }
            }
            if (curr == prev || value == prev) {
                wp->w_vthumb.t_prev = prev = 0;
            }
            wp->w_vthumb.t_curr = value;
        }

        if (prev) {                             /* erase old image, i.e. screen scrolls/jumps */
            vtmove(prev, redge);
            vtputb(scrollcol | CH_HSCROLL);
            wp->w_vthumb.t_prev = 0;
        }
    }
}


static void
hthumb_refresh(WINDOW_t *wp)
{
    const uint16_t value =
            (uint16_t)(wp->w_hthumb.t_active ? wp->w_hthumb.t_value : 0);
    const uint16_t curr = wp->w_hthumb.t_curr;

    if (value) {                                /* active value */
        const vbyte_t scrollcol = scrollcolor(wp);
        const vbyte_t thumbcol = thumbcolor(wp);
        const int bedge = win_bedge(wp);

        if (value && value != curr) {
            if (0 == curr) {                    /* new */
                vtmove(bedge, value);
                vtputb(thumbcol  | CH_HTHUMB);
            } else {                            /* update, optimise left->right */
                if (value < curr) {
                    vtmove(bedge, value);
                    vtputb(thumbcol  | CH_HTHUMB);
                    vtmove(bedge, curr);
                    vtputb(scrollcol | CH_HSCROLL);
                } else {
                    vtmove(bedge, curr);
                    vtputb(scrollcol | CH_HSCROLL);
                    vtmove(bedge, value);
                    vtputb(thumbcol  | CH_HTHUMB);
                }
            }
            wp->w_hthumb.t_curr = value;
        }
    }
}


/*  Function:           nexttab
 *      Determine distance to the next tab.
 *
 *  Parameters:
 *      wp - Window object address.
 *      redge - Right edge.
 *
 *  Returns:
 *      Tab column.
 */
static __CINLINE int
nexttab(const WINDOW_t *wp, int redge)
{
    int width, i, x;

    x = win_lcolumn(wp);
    i = vtcol + (wp->w_left_offset - wp->w_disp_indent);
    width = ruler_next_tab(wp->w_bufp, i - x + 1) + x - i;
    if (width + vtcol >= redge) {
        width = redge - vtcol;
    }
    return width;
}


/*  Function:           winputch
 *      Write a character into the specific window respecting the window configuration.
 *
 *          o character map and current indentation for this window.
 *
 *          o UNICODE mapping and replacement for non UTF-8 terminals.
 *
 *          o Tabs/ruler, with the area space filled.
 *
 *  Notes:
 *      Conversion rules within the function must match those within
 *      character_decode() and vtcharwidth() logic.
 *
 *  Parameters:
 *      wp - Window object address.
 *      ch - Character.
 *      attr - Attribute.
 *      rclip - Right column.
 *
 *  Returns:
 *      nothing.
 */
static __CINLINE void
winputch(WINDOW_t *wp, const vbyte_t ch, const vbyte_t attr, int rclip)
{
    if (ch > 0xff) {                            /* Multibyte characters */
        if (xf_disptype >= DISPTYPE_UTF8 ||
                    (ch >= CH_MIN && ch <= CH_MAX)) {
            winputm(ch | attr);                 /* MCHAR or GRAPHIC (dialogs only) */

        } else if (DISPUTF8_SUBST_MASK & xf_utf8) {
            const unsigned char *cp;
            const char *format;
            char buffer[32];

            if (ch <= 0xffff) {
                if (DISPUTF8_HEX & xf_utf8) {
                    format = "u%04x";           /* uxxxx    */
                } else if (DISPUTF8_NCR & xf_utf8) {
                    format = "u+%04x";          /* u+xxxx   */
                } else if (DISPUTF8_C99 & xf_utf8) {
                    format = "\\U%04x";         /* \uxxxx   */
                } else {
                    format = "\\u%04x";         /* \uxxxx   */
                }

            } else {
                if (DISPUTF8_HEX & xf_utf8) {
                    format = "U%06x";           /* Uxxxxxx  */
                } else if (DISPUTF8_NCR & xf_utf8) {
                    format = "U+%06x";          /* U+xxxxxx */
                } else if (DISPUTF8_C99 & xf_utf8) {
                    format = "\\U%08x";         /* \Uxxxxxx */
                } else {
                    format = "\\U%06x";         /* \Uxxxxxx */
                }
            }
            sprintf(buffer, format, ch);
            for (cp = (const unsigned char *)buffer; *cp && vtcol < rclip; ++cp) {
                winputm(*cp | attr);
            }

        } else {                                /* MCHAR??? */
            int wcwidth = Wcwidth(ch);
            while (wcwidth-- > 0) {
                winputm('?' | attr);            /* ? or ??  */
            }
        }

    } else {
        winputm(ch | attr);
    }
}


static __CINLINE void
winputmc(WINDOW_t *wp, const cmapchr_t *mc, const vbyte_t attr, int rclip)
{
    const unsigned char *cp;

    if (NULL != (cp = (unsigned char *)mc->mc_str)) {
        if (VBYTE_ATTR(ATTR_NORMAL) == attr &&
                mc->mc_literal && (WDF_LITERAL & wp->w_disp_flags)) {
            for (;*cp && vtcol < rclip; ++cp) {
                winputm(*cp | VBYTE_ATTR(ATTR_HILITERAL));
            }
        } else {
            for (;*cp && vtcol < rclip; ++cp) {
                winputm(*cp | attr);
            }
        }
    } else {
        if (VBYTE_ATTR(ATTR_NORMAL) == attr &&
                mc->mc_literal && (WDF_LITERAL & wp->w_disp_flags)) {
            winputm(mc->mc_chr | VBYTE_ATTR(ATTR_HILITERAL));
        } else {
            winputm(mc->mc_chr | attr);
        }
    }
}


static __CINLINE vbyte_t
normalcolor(const WINDOW_t *wp)
{
    if (W_TILED == wp->w_type && !xf_borders && wp->w_attr > 0) {
        return VBYTE_ATTR(wp->w_attr);          /* color_index */

    } else {
        const vbyte_t attr =
            (wp->w_bufp && wp->w_bufp->b_attrnormal ? wp->w_bufp->b_attrnormal :
                (W_POPUP == wp->w_type ? ATTR_POPUP_NORMAL : ATTR_NORMAL));
        return VBYTE_ATTR(attr);
    }
}


static __CINLINE vbyte_t
markcolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (W_POPUP == wp->w_type ? VBYTE_ATTR(ATTR_POPUP_HILITE) : VBYTE_ATTR(ATTR_HILITE));
    return ret;
}


static __CINLINE vbyte_t
standoutcolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (W_POPUP == wp->w_type ? VBYTE_ATTR(ATTR_POPUP_STANDOUT)  : VBYTE_ATTR(ATTR_STANDOUT));
    return ret;
}


static __CINLINE vbyte_t
framecolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (wp->w_dialogp ? VBYTE_ATTR(ATTR_DIALOG_FRAME) :
            (W_POPUP == wp->w_type ? VBYTE_ATTR(ATTR_POPUP_FRAME) : VBYTE_ATTR(ATTR_FRAME)));
    if (! ret) {
        return normalcolor(wp);
    }
    return ret;
}


static __CINLINE vbyte_t
titlecolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (wp->w_dialogp ? VBYTE_ATTR(ATTR_DIALOG_TITLE) :
            (W_POPUP == wp->w_type ? VBYTE_ATTR(ATTR_POPUP_TITLE) : VBYTE_ATTR(ATTR_TITLE)));
    if (! ret) {
        return normalcolor(wp);
    }
    return ret;
}


static __CINLINE vbyte_t
scrollcolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (wp->w_dialogp ? VBYTE_ATTR(ATTR_DIALOG_SCROLLBAR) : VBYTE_ATTR(ATTR_SCROLLBAR));
    if (! ret) {
        return normalcolor(wp);
    }
    return ret;
}


static __CINLINE vbyte_t
thumbcolor(const WINDOW_t *wp)
{
    const vbyte_t ret =
        (wp->w_dialogp ? VBYTE_ATTR(ATTR_DIALOG_THUMB) : VBYTE_ATTR(ATTR_SCROLLBAR_THUMB));
    if (! ret) {
        return normalcolor(wp);
    }
    return ret;
}


static __CINLINE int
hasborders(const WINDOW_t *wp)
{
    return (W_POPUP == wp->w_type || (W_TILED == wp->w_type && xf_borders));
}


static __CINLINE int
isdirty(const BUFFER_t *bp, LINENO line)
{
    if (bp->b_dirty_min)
        return (line >= bp->b_dirty_min && line <= bp->b_dirty_max);
    return 0;
}


/*  Function:           draw_window
 *      Draw the specified window using its associated buffer.
 *
 *  Parameters:
 *      wp - Window object address.
 *      top - Top of window arena to update.
 *      line - Line-number at 'top'.
 *      end -  End of window arena to update.
 *      bottom - Physical end of the window arena, used for clipping.
 *      action - Draw actions.
 *
 *          o 0 -           Normal updates.
 *          o SCROLLED -    Normal updates.
 *          o LAZY -        Enable lazy updates.
 *          o REPAINT -     Complete repaint.
 *          o DIRTY -       Only dirty lines.
 *
 *  Returns:
 *      nothing.
 */
static void
draw_window(WINDOW_t *wp, int top, LINENO line, int end, const int bottom, int actions)
{
    WINDOW_t *saved_wp   = NULL;
    BUFFER_t *saved_bp   = NULL;

    BUFFER_t *bp         = wp->w_bufp;
    const int lazy       = (xf_lazyvt >= 0 ? xf_lazyvt : x_pt.pt_lazyupdate);
    const int iscurrent  = (curwp == wp ? TRUE : FALSE);
    const int ledge      = win_ledge(wp);
    const int redge      = win_redge(wp);
    const int syntax     = (bp && BFTST(bp, BF_SYNTAX) && bp->b_syntax && //MCHAR???
                                bp->b_type != BFTYP_UTF16 && bp->b_type != BFTYP_UTF32);
                                                /* syntax parser not wchar safe/FIXME */
    const vbyte_t nattr  = normalcolor(wp);
    const vbyte_t lattr  = (syntax ? VBYTE_ATTR(ATTR_COLUMN_LINENO) : nattr);
    const vbyte_t sattr  = (syntax ? VBYTE_ATTR(ATTR_COLUMN_STATUS) : nattr);
    const vbyte_t xattr  = (syntax ? VBYTE_ATTR(ATTR_NONBUFFER)     : nattr);

    const uint32_t flags = wp->w_disp_flags;
    LINECHAR lbuffer[ LMARGIN*2 ];              /* left margin formatting buffer */
    vbyte_t space = nattr | ' ';
    ANCHOR_t anchor;

    assert(end <= bottom);

    /* Update the global mark and hilite definition */
    wp->w_disp_vtbase =
            (wp->w_left_offset - win_lmargin(wp)) - win_ledge(wp) + !win_lborder(wp);
    wp->w_disp_anchor = NULL;

    anchor.type = MK_NONE;
    if (bp) {
        if (iscurrent || WFTST(wp, WF_SHOWANCHOR)) {
            if (anchor_get(wp, NULL, &anchor)) {
                wp->w_disp_anchor = &anchor;    /* active anchor */
            }
        }
    }
                                                /* find first hilite, if any */
    if (NULL != (bp->b_hilite = hilite_find(bp, NULL, line, 1, NULL))) {
        if (bp->b_hilite->h_sline > line + (end - top)) {
            bp->b_hilite = NULL;
        }
    }

    /* Update the line syntax status */
    if (0 == (VTDRAW_DIRTY & actions) && syntax) {
        int delta;

        delta = syntax_parse(FALSE);            /* parse buffer syntax */
        if (0 == (VTDRAW_REPAINT & actions)) {
            int diff = end - top;

            if (delta > line + diff) {          /* lower lines also effected */
                int nend = top + (delta - line);

                if (nend > bottom) {            /* limit to display area */
                    nend = bottom;
                }
                if (VTDRAW_LAZY & actions) {    /* limit update */
                    if (lazy > 0 && nend > end + lazy) {
                        nend = end + lazy - 1;
                    }
                }
                end = nend;
            }
        }
    }

    if (end > ttrows() - (xf_borders + 1)) {    /* clip, may cause during resizing */
        end = ttrows() - (xf_borders + 1);
    }

    /*
     *  Thumb positions/
     *      scroll bas should only be visible on the current window,
     *      unless lazy in which case all windows are enabled reducing updates.
     */
    wp->w_vthumb.t_active = (uint16_t) (iscurrent || lazy || WFTST(wp, WF_DIALOG));
    wp->w_hthumb.t_active = (uint16_t) (iscurrent || lazy || WFTST(wp, WF_DIALOG));
    if (VTDRAW_REPAINT & actions) {
        wp->w_vthumb.t_curr = wp->w_vthumb.t_prev = 0;
        wp->w_hthumb.t_curr = wp->w_hthumb.t_prev = 0;
    }
    vthumb_position(wp);
    hthumb_position(wp);

    /*
     *  Titles
     */
    if (VTDRAW_REPAINT & actions) {
        int tedge = win_tedge(wp);

        if (hasborders(wp)) {                   /* window title */
            if (!WFTST(wp, WF_NO_TITLE)) {
                draw_title(wp, TRUE, tedge);
            }
            ++tedge;
        }
        if (BFTST(bp, BF_RULER)) {              /* ruler */
            draw_ruler(wp, tedge, ledge);
            draw_right(wp, redge, space);
        }
        if (iscurrent)                          /* current title */
            if (W_TILED == wp->w_type && !BFTST(bp, BF_SYSBUF)) {
                vttitle(wp->w_title);
            }
    }

    /*
     *  Body
     */
    saved_wp = curwp;                           /* requirement of lower level functionality (draw_line). */
    saved_bp = curbp;
    set_curwpbp(wp, bp);

    for (; top <= end; ++top, ++line) {
        const LINE_t *lp = vm_lock_line2(line);

        if (VTDRAW_DIRTY & actions) {           /* skip clean/blank lines */
            if (NULL == lp || (0 == isdirty(curbp, line) && 0 == lisdirty(lp))) {
                vm_unlock(line);
                continue;
            }
        }

        wp->w_disp_nattr = nattr;

        vtmove(top, ledge);                     /* position cursor */
        draw_left(wp);                          /* left column */

        if (lp) {                               /* line content */
            const LINENO numlines = bp->b_numlines;

            if (line <= numlines) {
                /*
                 *  Extended line attributes
                 */
                if ((WDF_LINENO | WFD_LINEBITS) & flags) {
                    const LINENO lineno = ((WDF_LINEOLD & flags) ? lp->l_oldlineno : line);
                    int lmargin = wp->w_disp_lmargin;
                    LINECHAR *p = lbuffer;

                    /* modification status */
                    if (WFD_LINEBITS & flags) {
                        LINECHAR status = 0;

                        if (0 == lp->l_oldlineno) {
                            status = '+';       /* new */
                        } else if (lismodified(lp)) {
                            status = '!';       /* modified */
                        } else {                /* unmodified */
                            status = ' ';
                        }

                        if (status) {
                            if (sattr != lattr) {
                                draw_line(sattr, sattr, &status, NULL, 1, DL_BASECMAP);
                            } else {
                                *p++ = status;
                            }
                            --lmargin;
                        }
                    }

                    /* line decoder debug */
                    if (WDF_DIAG & flags) {
                        const lineflags_t lsyntax = (lflags(lp) & L_SYNTAX_MASK);

                        p += sprintf((char *)p, "%c%c:",
                                (lincore(lp)                  ? 'C' :
                                (linfile(lp)                  ? 'F' : ' ')),
                                (lsyntax == L_IN_COMMENT      ? '#' :
                                (lsyntax == L_IN_STRING       ? 's' :
                                (lsyntax == L_IN_CHARACTER    ? 'c' :
                                (lsyntax == L_IN_CONTINUE     ? '+' :
                                (lsyntax == L_IN_PREPROC      ? '@' :
                                (lsyntax == L_HAS_EOL_COMMENT ? '-' : ' ')))))));
                        lmargin -= 3;
                    }

                    /* line number or binary-offset (original) */
                    if (WDF_LINENO & flags) {
                        p += sprintf((char *)p, "%*u ", lmargin - 1, (unsigned)lineno);

                    } else if (WDF_OFFSET & flags) {
                        p += sprintf((char *)p, "%08x ", (unsigned)(bp->b_bin_chunk_size * (line - 1)));
                    }

                    /* flush results */
                    if (lbuffer != p) {
                        draw_line(lattr, lattr, lbuffer, NULL, p - lbuffer, DL_BASECMAP);
                    }
                }

                /*
                 *  Line content/
                 *      optimise based on marked type (MK_NORMAL, MK_LINE and MK_NONINC).
                 */
                if (0 == lp->l_oldlineno) {
                    if (WDF_HIADDITIONAL & wp->w_disp_flags) {
                        wp->w_disp_nattr = VBYTE_ATTR(ATTR_ADDITIONAL);
                    }
                } else if (lismodified(lp)) {
                    if (WDF_HIMODIFIED & wp->w_disp_flags) {
                        wp->w_disp_nattr = VBYTE_ATTR(ATTR_MODIFIED);
                    }
                }

                if (!syntax || (MK_NONE != anchor.type && MK_COLUMN != anchor.type &&
                                    line > anchor.start_line && line < anchor.end_line)) {
                    space = draw_normal_line(line, lp);
                } else {
                    space = draw_syntax_line(line, lp);
                }

            } else {
                if (WDF_EOF & flags) {          /* <[EOF]> and <~> markers */
                    static const char eof[] = "[EOF]";
                    const char *cp = eof;

                    while (*cp) {
                        winputch(wp, *cp++, xattr, vtcol + 1);
                    }
                } else if ((WFD_LINEBITS|WDF_TILDE) & flags) {
                    winputch(wp, '~', xattr, vtcol + 1);
                } else {
                    if (line == (numlines + 1)) {
                        const cmapchr_t *mc = cmapchr_lookup(wp->w_disp_cmap, CMAP_EOF);

                        if (mc->mc_str || mc->mc_chr) {
                            winputmc(wp, mc, xattr, vtcol + mc->mc_width);
                        }
                    }
                }
                space = draw_normal_line(line, lp);
            }

            vm_unlock(line);                    /* release lock */
        }

        draw_right(wp, redge, space);           /* right column */
    }

    /*
     *  Repaint the bottom line
     */
    line = win_bedge(wp);

    if ((line <= ttrows() - (xf_borders + 1)) ||
                (W_POPUP == wp->w_type && (line == ttrows() - 1))) {
        if (VTDRAW_REPAINT & actions) {
            /*
             *  Bottom line
             */
            if (hasborders(wp) && !draw_hscroll(wp, line)) {
                if (!WFTST(wp, WF_NO_MESSAGE)) {
                    draw_title(wp, FALSE, line);
                }
            }

            /*
             *  Pop-up draw shadow (right/bottom)
             */
            if (W_POPUP == wp->w_type && !WFTST(wp, WF_NO_SHADOW)) {
                vtmove(line, redge + 1);
                draw_shadow(wp, ' ');

                if (line < ttrows() - (xf_borders + 1)) {
                    int j;

                    vtmove(line + 1, ledge + 1);
                    for (j = win_width(wp) + 1; j > 0; --j) {
                        draw_shadow(wp, ' ');
                    }
                    draw_shadow(wp, '\\');
                }
            }
        }
    }

    wp->w_disp_anchor = NULL;
    set_curwpbp(saved_wp, saved_bp);            /* restore state */
}


/*  Function:           draw_title
 *      Position the cursor and draw the window title.
 *
 *      The following attributes effect title generation.
 *
 *          o DC_ROSUFFIX           Read-only suffix.
 *          o DC_MODSUFFIX          Modified suffix.
 *
 *          o BF2_TITLE_FULL        Label window using full path name.
 *          o BF2_TITLE_SCROLL      Scroll title with window.
 *          o BF2_TITLE_LEFT        Left justify title.
 *          o BF2_TITLE_RIGHT       Right justify title.
 *
 *          o BF2_SUFFIX_RO         Read-only suffix.
 *          o BF2_SUFFIX_MOD        Modified suffix.
 *
 *  Parameters:
 *      wp - Window object address.
 *      top - *true* if top of window, otherwise *false*.
 *      line - Screen line.
 *
 *  Returns:
 *      nothing.
 */
static void
draw_title(const WINDOW_t *wp, const int top, const int line)
{
    const BUFFER_t *bp  = wp->w_bufp;
    const int width     = wp->w_w;
    const int ledge     = win_ledge(wp);
    const int digits    = (((x_dflags && width > 20) || x_applevel > 1) && 0 == line) ? 1 : 0;
    const int closebtn  = (top && W_POPUP == wp->w_type ? window_ctrl_test(wp, WCTRLO_CLOSE_BTN) : FALSE);
//  const int zoombtn   = (top && W_POPUP == wp->w_type ? window_ctrl_test(wp, WCTRLO_ZOOM_BTN) : FALSE);
    const unsigned char *title =
            (unsigned char *)(!top ? wp->w_message :
                    (bp && BF2TST(bp, BF2_TITLE_FULL) && bp->b_fname[0] ? bp->b_fname : wp->w_title));
    const unsigned char *titleend = (title ? title + strlen((const char *)title) : 0);
    const char *suffix  = NULL;
    const vbyte_t col   = framecolor(wp);
    int  titlelen       = (title && *title ? (int)utf8_width(title, titleend) : 0); /*MCHAR*/
    int  left           = 0;
    int  right          = 0;
    char numbuf[20];
    vbyte_t ch;

    /* read-only/modified suffix */
    if (titlelen > 0) {
        if (bp) {
            if (BFTST(bp, BF_RDONLY)) {
                if ((DC_ROSUFFIX & x_display_ctrl)  || BF2TST(bp, BF2_SUFFIX_RO)) {
                    suffix = " (ro)";
                    titlelen += 5;              /* " (ro)" */
                }
            } else if (bp->b_nummod) {
                if ((DC_MODSUFFIX & x_display_ctrl) || BF2TST(bp, BF2_SUFFIX_MOD)) {
                    suffix = " (*)";
                    titlelen += 4;              /* " [*]" */
                }
            }
        }
    }

    /* calc display offset */
    if (titlelen <= 0) {
        left = width;
    } else {
        const int scroll = (bp ? BF2TST(bp, BF2_TITLE_SCROLL) : 0);
        int norm;                               /* title length, include left/right separators (+2) */

        if (scroll) {
            if (titlelen > wp->w_left_offset) {
                titlelen -= wp->w_left_offset;
                title += wp->w_left_offset;
            } else {
                titlelen = 0;
                title = (const unsigned char *)"";
            }
        }

        if ((norm = (width - 2) - titlelen) < 0) {
            titlelen = width - 2;
            left = 0;
        } else {
            if (bp && (scroll || BF2TST(bp, BF2_TITLE_LEFT))) {
                left = 0;
            } else if (bp && BF2TST(bp, BF2_TITLE_RIGHT)) {
                left = width - (titlelen + 2);
            } else {
                left = norm / 2;
            }
            right = width - (left + titlelen + 2);
        }
    }

    numbuf[0] = 0;
    if (digits) {
        const int flags = trace_flags();
        const int len =                         /* debug-flags or level */
            sxprintf(numbuf, sizeof(numbuf), (flags ? " DBG:0x%x" : "%d"), (flags ? flags : x_applevel));
        if (len > 1) {
            right -= (len - 1);
            if (right < 0) right = 0;
        }
    }
    assert(left >= 0);
    assert(right >= 0);

    /* corner */
    vtmove(line, ledge);
    ch = col | v_corner_map[wp->w_corner_hints[top ? TL_CORNER : BL_CORNER]];
    vtputb(ch);

    /* close button */
    if (closebtn && right > 3) {                /* MCHAR??? */
        vtputs((const unsigned char *)"[*]", col);
        right -= 3;
    }

    /* left side of frame */
    ch = col | CH_HORIZONTAL;
    while (left-- > 0) {
        vtputb(ch);
    }

    /* title */
    if (titlelen) {
        const cmap_t *ocmap = wp->w_disp_cmap;
        int title_col = col;

        if (WFTST(wp, WF_SELECTED) || (wp->w_status & WFTOP)) {
            title_col = titlecolor(wp);         /* 'selected' window */
        }

        ch = col | ' ';
        vtputb(ch);
        ((WINDOW_t *)wp)->w_disp_cmap = x_base_cmap;

        while (*title && titlelen > 0) {        /* MCHAR */
            const unsigned char *cend;
            int cwidth;
            int32_t wch;

            if ((cend = charset_utf8_decode_safe(title, titleend, &wch)) > title &&
                    (cwidth = Wcwidth(wch)) >= 0) {
                vtputb(wch | title_col);
                titlelen -= cwidth;
                title = cend;
                continue; //next
            }
            break; //error
        }

        if (titlelen > 0 && suffix) {
            while (*suffix && titlelen-- > 0) {
                vtputb(*suffix++ | title_col);
            }
        }
        ((WINDOW_t *)wp)->w_disp_cmap = ocmap;
        vtputb(ch);
    }

    /* right */
    ch = col | CH_HORIZONTAL;
    while (right-- > 0) {
        vtputb(ch);
    }

    /* window level or corner */
    if (digits) {
        vtputs((const unsigned char *)numbuf, thumbcolor(wp));
    } else {
        ch = col | v_corner_map[wp->w_corner_hints[top ? TR_CORNER : BR_CORNER]];
        vtputb(ch);
    }
}


/*  Function:           draw_hscroll
 *      Position the cursor and draw/update the horizontal scroll bar.
 *
 *  Parameters:
 *      wp - Window object address.
 *      line - Line screen.
 *
 *  Returns:
 *      nothing.
 */
static int
draw_hscroll(WINDOW_t *wp, int line)
{
    const uint16_t value =
            (uint16_t)(wp->w_hthumb.t_active ? wp->w_hthumb.t_value : 0);

    if (value) {
        const vbyte_t framecol  = framecolor(wp);
        const vbyte_t scrollcol = scrollcolor(wp);
        const vbyte_t thumbcol  = thumbcolor(wp);
        int ledge = win_ledge(wp);
        int i, w = wp->w_w;

        vtmove(line, ledge);
        vtputb(framecol | v_corner_map[wp->w_corner_hints[BL_CORNER]]);
        for (i = 1; i <= w; ++i) {
            if (vtcol == value) {
                vtputb(thumbcol | CH_HTHUMB);
                wp->w_hthumb.t_curr = value;
            } else {
                vtputb(scrollcol | CH_HSCROLL);
            }
        }
        vtputb(framecol | v_corner_map[wp->w_corner_hints[BR_CORNER]]);
    }
    return value;
}


/*  Function:           draw_ruler
 *      Position the cursor and draw the window ruler.
 *
 *  Parameters:
 *      wp - Window object address.
 *      line - Screen line.
 *      ledge - Left edge.
 *
 *  Example:
 *      o Format 1, given margins(5, 60) and tabs(5, 9)
 *
 *          ....[...+1..+...+..2+...+...+3..+...+..4+...+...+5..+...+..6+]..+...+7..+
 *
 *  Returns:
 *      nothing.
 */
static void
draw_ruler(const WINDOW_t *wp, int line, int ledge)
{
    const BUFFER_t *bp = wp->w_bufp;
    const vbyte_t rulernormal = VBYTE_ATTR(ATTR_RULER);
    const vbyte_t rulermargin = VBYTE_ATTR(ATTR_RULER_MARGIN);
    const vbyte_t rulerident = VBYTE_ATTR(ATTR_RULER_IDENT);
    const vbyte_t rulermark = VBYTE_ATTR(ATTR_RULER_MARK);
    const LINENO length = buf_line_length(bp, FALSE);
    const int lmargin = (int)wp->w_disp_lmargin;
    const int wpmarginl = ruler_rmargin(bp),
            wpmarginr   = ruler_lmargin(bp),
            colorcolumn = ruler_colorcolumn(bp);
    int indent, col, pos;
    vbyte_t ch;

    pos = wp->w_left_offset + 1;
    indent = ruler_next_indent(bp, pos) + 1;

    vtmove(line, ledge);
    draw_left(wp);

    for (col = 0; col < wp->w_w; ++col) {       /* MCHAR??? */
        if (col < lmargin) {
            ch = rulernormal | ' ';
        } else {
            if (pos == wpmarginl) {
                ch = rulermargin | '[';         /* left margin */

            } else if (pos == wpmarginr) {
                ch = rulermargin | ']';         /* right margin */

            } else if (pos == colorcolumn) {
                ch = rulermargin | '!';         /* color-column */

            } else if (pos == length) {
                ch = rulermargin | '$';         /* line length */

            } else if (pos == indent) {
                ch = rulerident  | '+';         /* indent marker */

            } else if (0 == (pos % 10)) {       /* column */
                ch = rulermark   | "0123456789"[(pos / 10) % 10];

            } else {
                ch = rulernormal | '.';         /* others */
            }
            if (pos > indent) {
                indent = ruler_next_indent(bp, pos) + 1;
            }
            ++pos;
        }
        vtputb(ch);
    }
}


/*  Function:           draw_left
 *      Position the cursor and draw the left column.
 *
 *  Parameters:
 *      wp - Window object address.
 *
 *  Returns:
 *      nothing.
 */
static void
draw_left(const WINDOW_t *wp)
{
    if (win_lborder(wp)) {
        vtputb(framecolor(wp) | CH_VERTICAL);
    }
}


/*  Function:           draw_right
 *       Draw the right hand margin and possibly the shadow to the right of the window,
 *       firstly erasing to end of line within a window.
 *
 *  Parameters:
 *      wp - Window object address.
 *      redge - Right edge.
 *      space - Space character.
 *
 *  Returns:
 *      nothing.
 */
static void
draw_right(WINDOW_t *wp, int redge, vbyte_t space)
{
    register VCELL_t *bp;

    for (bp = vline + vtcol; vtcol < redge;) {
        vcell_set(bp++, space);
        ++vtcol;
    }

    if (win_rborder(wp)) {
        const uint16_t value =
                (uint16_t)(wp->w_vthumb.t_active ? wp->w_vthumb.t_value : 0);

        if (0 == value) {
            vtputb(framecolor(wp)  | CH_VERTICAL);

        } else if (value == vtrow) {
            vtputb(thumbcolor(wp)  | CH_VTHUMB);
            wp->w_vthumb.t_prev = wp->w_vthumb.t_curr;
            wp->w_vthumb.t_curr = value;

        } else {
            vtputb(scrollcolor(wp) | CH_VSCROLL);
        }

        if (vtrow == wp->w_vthumb.t_prev) {
            wp->w_vthumb.t_prev = 0;
        }
    }

    if (W_POPUP == wp->w_type) {
        draw_shadow(wp, ' ');
    }
}


/*  Function:           draw_normal_line
 *      Draw the line using standard colorisation.
 *
 *  Parameters:
 *      line - Line number.
 *      lp - Line buffer.
 *
 *  Returns:
 *      Attribute of last/trailing character.
 */
static vbyte_t
draw_normal_line(const LINENO line, const LINE_t *lp)
{
    WINDOW_t *wp = curwp;
    BUFFER_t *bp = wp->w_bufp;
    const vbyte_t nattr = wp->w_disp_nattr;
    const vbyte_t wattr = (BF2TST(bp, BF2_HIWHITESPACE) ? VBYTE_ATTR(ATTR_WHITESPACE) : nattr);
    vbyte_t space;

    assert(wp->w_bufp == curbp);
    wp->w_disp_line = line;
    wp->w_disp_column = -wp->w_left_offset;
    wp->w_disp_indent = wp->w_left_offset;
    space = draw_line(nattr, wattr, ltext(lp), lattr(lp), llength(lp),
                DL_DOEOL | (lforw(lp) ? 0 : DL_ISEOF) | (BFTST(bp, BF_BINARY) ? DL_ISBINARY : 0));
    wp->w_disp_indent = 0;
    wp->w_disp_column = -1;
    wp->w_disp_line = -1;
    return space;
}


/*  Function:           draw_syntax_line
 *      Draw the line with syntax highlight processing active.
 *
 *  Parameters:
 *      lineno - Line number.
 *      lp - Line buffer.
 *
 *  Returns:
 *      Attribute of last/trailing character.
 */
static vbyte_t
draw_syntax_line(const LINENO line, const LINE_t *lp)
{
    WINDOW_t *wp = curwp;
    BUFFER_t *bp = wp->w_bufp;
    const vbyte_t nattr = wp->w_disp_nattr;
    vbyte_t space;

    assert(wp->w_bufp == curbp);
    wp->w_disp_line = line;
    wp->w_disp_column = -wp->w_left_offset;
    wp->w_disp_indent = wp->w_left_offset;
    syntax_highlight(lp);
    space = draw_line(nattr, nattr, NULL, NULL, 0,
                DL_DOEOL | (lforw(lp) ? 0 : DL_ISEOF) | (BFTST(bp, BF_BINARY) ? DL_ISBINARY : 0));
    wp->w_disp_indent = 0;
    wp->w_disp_column = -1;
    wp->w_disp_line = -1;
    return space;
}


/*  Function:           vtwritehl
 *      Callback from syntax highlighting engine to display characters on the current line.
 *
 *  Parameters:
 *      cp - String.
 *      len - Buffer length, in bytes.
 *      col - Display colour.
 *      dotabs - Whether tabs should be highlighted.
 *
 *  Returns:
 *      nothing.
 */
void
vtwritehl(const LINECHAR *cp, int length, int col, int dotabs)
{
    WINDOW_t *wp = curwp;
    BUFFER_t *bp = wp->w_bufp;
    const vbyte_t nattr = wp->w_disp_nattr;
    const vbyte_t wattr = (dotabs || (BF2TST(bp, BF2_HIWHITESPACE)) ? VBYTE_ATTR(ATTR_WHITESPACE) : nattr);

    draw_line((ATTR_CURRENT == col ? nattr : VBYTE_ATTR(((unsigned)col))), wattr, cp, NULL, length, 0);
}


/*  Function:           draw_line
 *      Render the given text to the current window.
 *
 *  Parameters:
 *      nattr - Normal text color.
 *      wattr - White-space color.
 *      cp - Text buffer.
 *      ap - Attribute buffer, if any.
 *      length - Buffer length.
 *      flags - Control options.
 *
 *  Returns:
 *      Final character attribute.
 */
static vbyte_t
draw_line(const vbyte_t nattr, const vbyte_t wattr,
    register const LINECHAR *cp, register const LINEATTR *ap, int length, int flags)
{
    WINDOW_t *wp            = curwp;
    BUFFER_t *bp            = wp->w_bufp;

    const vbyte_t mattr     = markcolor(wp);
    const vbyte_t sattr     = standoutcolor(wp);

    const cmap_t *ocmap     = wp->w_disp_cmap,
                  *cmap     = ocmap;
    const LINENO line       = wp->w_disp_line;
    const LINENO vtbase     = wp->w_disp_vtbase;

    const int redge         = win_redge(wp);
    const int rclip         = win_rclipped(wp, redge);
    mchar_iconv_t *iconv    = bp->b_iconv;      /* Character conversion */
    const int isansi        = BFTST(bp, BF_ANSI);
    const int isman         = BFTST(bp, BF_MAN);
    const int iscross       = BF2TST(bp, BF2_CURSOR_ROW) || BF2TST(bp, BF2_CURSOR_COL);
    const int lvtcol        = vtcol;            /* left/start column */
    const LINECHAR *end     = cp + length;      /* EOS address */

    vbyte_t hiattr = 0, attr = nattr, ch;       /* attributes and character */
    const ANCHOR_t *anchor = NULL;
    const cmapchr_t *mc;
    int hilite = 0, marked = 0;                 /* hilite status plus marked status */
#define MARKED_NORMAL       1
#define MARKED_LINE         2
#define MARKED_HILITE       3
    int iseol;                                  /* EOL status */
    int32_t wch;

    /*
     *  if end-of-buffer, disable end-of-line marker
     */
    iseol = (DL_DOEOL & flags ? 0 : -1);        /* EOL counter */

    /*
     *  optimise hilite change tests.
     */
    if (line >= 1) {
        if (NULL == (anchor = wp->w_disp_anchor) ||
                line < anchor->start_line || line > anchor->end_line) {
            marked = 0;                         /* no/outside marked region */

        } else if ((MK_LINE   == anchor->type ||
                   (MK_COLUMN != anchor->type &&
                        line > anchor->start_line && line < anchor->end_line))) {
            marked = MARKED_LINE;               /* entire area is marked */

        } else {
            hilite = 0x01;
        }

        if (0 == marked && bp->b_hilite) {
            hilite |= 0x02;                     /* active hilites */
        }
    }

    /*
     *  Decode line
     */
    if (DL_BASECMAP & flags) {                  /* base character map */
        cmap = wp->w_disp_cmap = x_base_cmap;
    }

    while (vtcol < rclip) {
        int wwidth = 0;                         /* wide character end/width */
        int width = 0;                          /* tab width */

        if (cp < end) {                         /* text, character plus optional attribute */
            const LINECHAR *wcp, *ocp = cp;

            wch = 0;
            if ((wcp = mchar_decode_safe(iconv, ocp, end, &wch)) > ocp &&
                    (wwidth = Wcwidth(wch)) >= 0) {
                /*
                 *  apply cursor rules and increment buffers
                 */
                if (ap) {
                    if (ATTR_CURRENT == (attr = *ap)) {
                        attr = nattr;           /* absolute color */
                    } else {                    /* ATTR -> color */
                        attr = VBYTE_ATTR(attr);
                    }
                    ap += wcp - cp;             /* consume attributes */
                }

                cp = wcp;
                ch = wch;
                if (0 == wwidth) {              /* combined character, push onto previous */
                    if (vtisutf8() && (DISPUTF8_COMBINED & xf_utf8)) {
                        if (0 == wp->w_disp_indent && vtcol > 0) {
                            vtpushb(ch, vtcol - 1);
                        }
                    }
                    continue;
                }

                if (wwidth > 1 && (vtcol + wwidth > rclip)) {
                    wwidth = 0;
                    ch = ' ';                   /* exceed width, use replacement */
                } else {
                    width = wwidth = vtcharwidth(wch, wwidth);
                }
            } else {
                ch = *cp++;
                if (ap) {
                    assert(NULL == bp->b_colorizer);
                    if (ATTR_CURRENT == (attr = *ap++)) {
                        attr = nattr;           /* absolute color */
                    } else {                    /* ATTR -> color */
                        attr = VBYTE_ATTR(attr);
                    }
                }
            }

            if (WDF_HICOLUMN & wp->w_disp_flags) {
                if ((vtbase + vtcol) >= wp->w_disp_rmargin) {
                    attr = VBYTE_ATTR(ATTR_RULER_COLUMN);
                }
            }

        } else {
            attr = nattr;
            if (-1 == iseol) {                  /* EOL required ? */
                break;
            }
            ap = NULL;
            if (0 == iseol++) {                 /* first pass ? */
                if (WDF_EOL_HILITE & wp->w_disp_flags) {
                    hilite = marked = 0;
                }
                if ((DL_ISEOF|DL_ISBINARY) & flags) {
                    cmap = wp->w_disp_cmap = x_base_cmap;
                    ch =  ' ';
                } else {                        /* EOL */
                    ch = CMAP_EOL;
                }
            } else {
                cmap = wp->w_disp_cmap = x_base_cmap;
                ch = ' ';
            }
        }

        /*
         *  Character map dependant processing
         */
        if (NULL != (mc = cmapchr_lookup(cmap, ch))) {
            switch (mc->mc_class) {
            case CMAP_ESCAPE:
                if (isansi) {       /* ANSI escape decoding */
                    int inc;

                    if (cp < end && (inc = ansidecode(cp, end, &wp->w_disp_ansicolor, &wp->w_disp_ansiflags)) > 0) {
                        const vbyte_t ansi_color = wp->w_disp_ansicolor,
                                ansi_flags = wp->w_disp_ansiflags;

                        if (ANSI_MANORMAL & ansi_flags) {
                            if (ANSI_MAREVERSE & ansi_flags) {
                                attr = VBYTE_ATTR(ATTR_HILITE);
                            } else if (ANSI_MABOLD & ansi_flags) {
                                attr = VBYTE_ATTR(ATTR_ERROR);      //TODO, ATTR_ANSI_BOLD
                            } else if ((ANSI_MAITALIC|ANSI_MAUNDERLINE) & ansi_flags) {
                                attr = VBYTE_ATTR(ATTR_MESSAGE);    //TODO, ATTR_ANSI_UNDERLINE
                            } else {
                                attr = VBYTE_ATTR(ATTR_NORMAL);
                            }
                        } else {
                            attr = VBYTE_ATTR(ansi_color);
                        }
                        cp += inc;              /* eat character(s) */
                        continue;
                    }
                }
                break;

            case CMAP_BACKSPACE:
                if (isman) {        /* MAN style bold/double-strike */
                    if (vtcol > lvtcol) {
                        if (cp < end) {         /* back */
                            const unsigned char *t_wcp;
                            int32_t t_wch;

                            --vtcol;
                            if ((t_wcp = mchar_decode_safe(iconv, cp, end, &t_wch)) > cp) {
                                ch = t_wch;
                                cp = t_wcp;
                                if (ap) ap += t_wcp - cp;
                            } else {
                                ch = *cp++;
                                if (ap) ++ap;
                            }
                            winputch(wp, ch, (marked ? mattr : sattr), rclip);
                        }
                    }
                    continue;

                } else if (isansi) {
                    if (vtcol > lvtcol) {
                        --vtcol;                /* back, prevwidth? */
                    }
                    continue;
                }
                break;

            case CMAP_TAB:
                width = nexttab(wp, redge);     /* distance to tab */
                ch = CMAP_TABSTART;
                mc = cmapchr_lookup(cmap, ch);
                wwidth = -2;
                break;

            default:
                wwidth = mc->mc_width;
                width = 1;
                break;
            }
        }

        /*
         *  Iterate across character width.
         */
        for (;;) {
            /*
             *  mark and/or highlight?
             */
            if (hilite) {
                marked = 0;

                if (0x01 & hilite) {            /* marked-line, left/inside/right */
                    if ((line == anchor->start_line && line == anchor->end_line) ||
                                MK_COLUMN == anchor->type) {
                        if ((vtbase + vtcol) >= anchor->start_col) {
                            if ((vtbase + vtcol) <= anchor->end_col) {
                                marked = MARKED_NORMAL;
                            } else {
                                hilite &= ~0x01;
                            }
                        }
                                                /* marked-line, left or inside */
                    } else if (line == anchor->start_line) {
                        if ((vtbase + vtcol) >= anchor->start_col) {
                            marked = MARKED_NORMAL;
                        }

                    } else {                    /* marked-line, inside or right */
                        marked = MARKED_NORMAL;
                        if ((vtbase + vtcol) > anchor->end_col) {
                            hilite &= ~0x01;
                            marked = 0;
                        }
                    }
                }

                if (0x02 & hilite) {            /* query highlight */
                    if (0 == marked) {
                        hiattr = 0xffff;
                        if (NULL == (bp->b_hilite =
                                hilite_find(bp, bp->b_hilite, line, vtbase + vtcol, &hiattr))) {
                            hilite &= ~0x02;
                        } else if (0xffff == hiattr) {
                            if (bp->b_hilite->h_sline > line) {
                                hilite &= ~0x02;
                            }
                        } else {
                            marked = (hiattr > 0 ? MARKED_HILITE : MARKED_NORMAL);
                        }
                    }
                }
            }

            /*
             *  Display character,
             *      if marked both <Tab> or normal characters use the 'mattr',
             *      otherwise use 'attr' and 'wattr'.
             */
#define WINPUT(__wp, __mc, __c, __attr) \
            (__mc ? winputmc(__wp, __mc, __attr, rclip) : winputch(__wp, __c, __attr, rclip))

            if (marked) {                       /* marked/highlight area */
                if (MARKED_HILITE == marked) {
                    WINPUT(wp, mc, ch, VBYTE_ATTR(hiattr));
                } else {
                    WINPUT(wp, mc, ch, mattr);
                }
                                                /* tabs */
            } else if (iseol <= 0 && width && -2 == wwidth) {
                WINPUT(wp, mc, ch, wattr);

            } else if (iscross) {               /* cursor cross-hair */
                if (vtcol == vtcursorcol && BF2TST(bp, BF2_CURSOR_COL)) {
                    WINPUT(wp, mc, ch, VBYTE_ATTR(ATTR_CURSOR_COL));
                } else if (vtrow == vtcursorrow && BF2TST(bp, BF2_CURSOR_ROW)) {
                    WINPUT(wp, mc, ch, VBYTE_ATTR(ATTR_CURSOR_ROW));
                } else {
                    WINPUT(wp, mc, ch, attr);
                }

            } else {                            /* normal */
                WINPUT(wp, mc, ch, attr);
            }

#undef  WINPUT

            if (--width <= 0) {
                break;
            }

            if (wwidth > 1) {
                if (vtcol >= rclip) break;
                ch = CH_PADDING;
                mc = NULL;
            } else {
                if (CMAP_TABSTART == ch) {
                    ch = (1 == width ? CMAP_TABEND : CMAP_TABVIRTUAL);
                    mc = cmapchr_lookup(cmap, ch);
                } else if (CMAP_TABVIRTUAL == ch && 1 == width) {
                    ch = CMAP_TABEND;
                    mc = cmapchr_lookup(cmap, ch);
                }
            }
        }
    }

    if (xf_test && cp > end) {
        vtmove(vtrow, rclip);
        vtputb('>');
    }

    wp->w_disp_column = vtbase + vtcol;
    wp->w_disp_cmap = ocmap;                    /* restore character map */

    return (' ' | attr);
}


/*  Function:           draw_shadow
 *      Function to display a character forming a part of the background shadow of a
 *      window.
 *
 *      If the show thru option is on, then display the reverse of the character at this
 *      character position, otherwise display the character passed.
 *
 *  Parameters:
 *      wp - Window object address.
 *      ch - Shadow character.
 *
 *  Returns:
 *      Remaining buffer left in the buffer.
 */
static void
draw_shadow(const WINDOW_t *wp, vbyte_t ch)
{
    if (0 == (DC_SHADOW & x_display_ctrl)) {
        return;                                 /* shadows enabled? */
    }

    if (WFTST(wp, WF_NO_SHADOW)) {
        return;                                 /* explicit NO_SHADOW */
    }

    if (vtcol < ttcols()) {
        if (DC_SHADOW_SHOWTHRU & x_display_ctrl) {
            ch = VBYTE_CHAR_GET(vline[vtcol].primary);
        }
        vline[vtcol].primary = (vbyte_t)(ch | VBYTE_ATTR(ATTR_SHADOW));
        ++vtcol;
    }
}


/*  Function:           ansidecode
 *      Cook an ANSI/xterm style color escape sequence.
 *
 *  Parameters:
 *      cp - Buffer pointer.
 *      end - End of escape.
 *      color - Resulting color.
 *      flags - Attributes.
 *
 *  Returns:
 *      Remaining buffer left in the buffer.
 */
static int
ansidecode(const LINECHAR *cp, const LINECHAR *end, vbyte_t *color, unsigned *flags)
{
    const LINECHAR *start = cp;

    if ('[' == *cp++) {
        uint16_t args[DISPLAY_ESCAPELEN] = {0}, no = 0;

        while (cp < end) {
            const LINECHAR ch = *cp;

            if (';' == ch) {                    /* empty parameter = 0 */
                ++cp, ++no;

            } else if (isdigit(ch)) {           /* parameter numeric value */
                LINECHAR *cursor;
                unsigned val = (unsigned)strtoul((const char *)cp, (char **)&cursor, 10);

                if (no < DISPLAY_ESCAPELEN) {
                    args[no++] = (uint16_t)val;
                }
                cp = (';' == *cursor ? cursor + 1 : cursor);

            } else if  ('m' == ch) {            /* command */
                uint16_t fgcol  = (uint16_t)ANSICOLOR_FG(*color);
                uint16_t bgcol  = (uint16_t)ANSICOLOR_BG(*color);
                unsigned nflags = *flags;
              //unsigned normal = 0;
                unsigned idx;

                if (fgcol >= 8) {
                    nflags |= ANSI_MABOLD;
                }

                for (idx = 0; idx < no; ++idx) {
                    if (0 == args[idx]) {       /* normal */
                        fgcol  = ANSI_FGDEFAULT;
                        bgcol  = ANSI_BGDEFAULT;
                        nflags = ANSI_MANORMAL;

                    } else {
                        switch(args[idx]) {
                        case 1:                 /* bold */
                            nflags |= ANSI_MABOLD;
                            break;
                        case 2:                 /* faint */
                            break;
                        case 3:                 /* italic */
                            nflags |= ANSI_MAITALIC;
                            break;
                        case 4:                 /* underline */
                            nflags |= ANSI_MAUNDERLINE;
                            break;
                        case 5:                 /* bright background */
                            break;
                        case 6:                 /* blink */
                            nflags |= ANSI_MABLINK;
                            break;
                        case 7:                 /* reverse */
                            nflags |= ANSI_MAREVERSE;
                            break;
                        case 8:                 /* conceal */
                            break;
                        case 9:                 /* crossed-out */
                            break;
                                                /* fonts */
                        case 10: case 11: case 12: case 13: case 14:
                        case 15: case 16: case 17: case 18: case 19:
                            break;

                        case 22:                /* clear bold */
                            nflags &= ~ANSI_MABOLD;
                            break;
                        case 23:                /* clear italic */
                            nflags &= ~ANSI_MAITALIC;
                            break;
                        case 24:                /* clear underline */
                            nflags &= ~ANSI_MAUNDERLINE;
                            break;
                        case 25:                /* clear blink */
                            nflags &= ~ANSI_MABLINK;
                            break;
                        case 27:                /* clear reverse */
                            nflags &= ~ANSI_MAREVERSE;
                            break;
                        case 28:                /* clear conceal */
                            break;
                        case 29:                /* clear crossed-out */
                            break;
                                                /* foreground color */
                        case 30: case 31: case 32: case 33:
                        case 34: case 35: case 36: case 37:
                            nflags &= ~ANSI_MANORMAL;
                            nflags |=  ANSI_MACOLORS;
                            fgcol = ansicolor((uint16_t)(args[idx] - 30));
                            break;
                                                /* background */
                        case 40: case 41: case 42: case 43:
                        case 44: case 45: case 46: case 47:
                            nflags &= ~ANSI_MANORMAL;
                            nflags |=  ANSI_MACOLORS;
                            bgcol = ansicolor((uint16_t)(args[idx] - 40));
                            break;
                        case 38:                /* foreground color - 256 color mode */
                            if ((idx + 2) < no && 5 == args[idx + 1]) {
                                nflags &= ~ANSI_MANORMAL;
                                nflags |=  ANSI_MACOLORS | ANSI_MACOLORS256;
                                fgcol = ansicolor(args[idx + 2]);
                                idx += 2;       /* 38;5;<color> */
                            }
                            break;
                        case 48:                /* background color - 256 color mode */
                            if ((idx + 2) < no && 5 == args[idx + 1]) {
                                nflags &= ~ANSI_MANORMAL;
                                nflags |=  ANSI_MACOLORS | ANSI_MACOLORS256;
                                bgcol = ansicolor(args[idx + 2]);
                                idx += 2;       /* 48;5;<color> */
                            }
                            break;
                        case 39:                /* set default background color */
                            nflags &= ~ANSI_MANORMAL;
                            bgcol = ANSI_BGDEFAULT;
                            break;
                        case 49:                /* set default foreground color */
                            nflags &= ~ANSI_MANORMAL;
                            fgcol = ANSI_FGDEFAULT;
                            break;

                        case 51:                /* framed */
                        case 52:                /* encircled */
                        case 53:                /* overlined */
                        case 54:                /* not framed or encircled */
                        case 55:                /* not overlined */
                            break;

                        case 60:                /* ideogram underline or right side line */
                        case 61:                /* ideogram double underline or double line on the right side */
                        case 62:                /* ideogram overline or left side line */
                        case 63:                /* ideogram double overline or double line on the left side */
                        case 64:                /* ideogram stress marking */
                            break;
                                                /* set foreground color, high intensity (aixterm) */
                        case 90:  case 91:  case 92:  case 93:  case 94:
                        case 95:  case 96:  case 97:  case 98:  case 99:
                            break;
                                                /* set background color, high intensity (aixterm) */
                        case 100: case 101: case 102: case 103: case 104:
                        case 105: case 106: case 107: case 108: case 109:
                            break;

                        default:                /* unknown/unsupported */
                            break;
                        }
                    }
                }

                if ((ANSI_MABOLD & nflags) && fgcol < 8) {
                    fgcol = (uint16_t)(fgcol + 8);
                }

                if (ANSI_MAREVERSE & nflags) {
                    *color = ANSICOLOR_MK(bgcol, (fgcol % ANSIBG_MAX));
                } else {
                    *color = ANSICOLOR_MK(fgcol, bgcol);
                }

                *flags = (unsigned) nflags;
                return (cp - start) + 1;

            } else {
                break;                          /* other, exit */
            }
        }
    }
    return 0;
}


/*  Function:           ansicolor
 *      Map an ANSI color specification into its equivalent supported color.
 *
 *      For 8 and 16 colour modes this is simple one-to-mapping, otherwise for
 *      256 modes map the RBG values to something close.
 *
 *  Parameters:
 *      color - Color value.
 *
 *  Returns:
 *      Mapped color value.
 */
static uint16_t
ansicolor(uint16_t color)
{
    static const uint16_t coloransimap[8] = {   /* ANSI colors -> BRIEF */
        BLACK, RED, GREEN, BROWN, BLUE, MAGENTA, CYAN, WHITE
        };

    if (color < 16) {                           /* classic colors */
        return coloransimap[color % 8];

    } else if (color >= 244) {                  /* upper grey-scale */
        return WHITE;

    } else if (color >= 232) {                  /* lower grey-scale */
        return BLACK;

    } else {                                    /* RGB 6x6x6 color cube */
        color = (uint16_t)(color - 16);
        {
            const uint16_t r =  color / 36;
            const uint16_t g = (color / 6) % 6;
            const uint16_t b = (color % 6);

            if (r < g) {                        /* green? */
                if (g < b)  return BLUE;
                if (g > b)  return GREEN;
                return CYAN;
            } else if (r > g) {                 /* red? */
                if (r < b)  return BLUE;
                if (r > b)  return RED;
                return MAGENTA;
            } else {                            /* yellow? (brown) */
                if (g < b)  return BLUE;
                if (g > b)  return BROWN;
                if (r >= 3) return WHITE;
            }
        }
    }
    return BLACK;
}


/*  Function:           do_display_windows
 *      display_windows primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: display_windows - Control window display.

        int
        display_windows([int mode])

    Macro Description:
        The 'display_window()' primitive control the state of the display
        driver.

        This primitive should be called prior to the creation of any
        windows using <create_tiled_window> enabling the display
        driver. If any tiled windows exist when enabled the following
        error shall be echoed, denoting incorrect system initialisation.

>           display_window: overlapping window exist

        The original functionality was intended to initialise a set of
        tiled windows, between two successive calls; firstly with
        disable (0) which cleared all windows and secondary on the
        completion of the window creation with enable (1). This
        implementation only obeys the first enable command, with any
        disable requests silently ignored.

    Macro Parameters:
        mode - Optional integer, if specified states the new display
            mode, otherwise if omitted the current display mode is
            toggled.

    Macro Returns:
        The 'display_window()' primitive returns the previous status.

    Macro Portability:
        Unlike BRIEF existing tiled windows are not destroyed upon
        the initial mode change.

    Macro See Also:
        create_tiled_window
 */
void
do_display_windows(void)        /* void ([int mode]) */
{
    const int omode = (x_display_enabled > 0 ? TRUE : FALSE);

    x_display_enabled = (get_xinteger(1, !omode) ? TRUE : FALSE);
    if (omode != x_display_enabled) {
        if (x_display_enabled > 0) {
            WINDOW_t *wp;

            ttdisplay();                        /* final initialisation/defaults */

            for (wp = window_first(); wp; wp = window_next(wp)) {
                if (W_TILED != wp->w_type) {
                    eeputs("display_window: overlapping window exist");
                    break;
                }
            }
        }
    }
    acc_assign_int((accint_t) omode);
}


/*  Function:           do_screen_dump
 *      screen_dump primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: screen_dump - Dump an image of the screen.

        int
        screen_dump(string filename)

    Macro Description:
        The 'screen_dump()' primitive dumps a text representation of the
        current screen image to the specified file. The resulting image
        omits both attribute and character-map associations with the
        view, with frame characters mapped to their ASCII equivalent.

    Macro Parameters:
        filename - Optional string containing the full path of the
            output filename, if omitted the path '/tmp/griefscreen.###'
            shall be used; where ### represents the next available
            sequential filename.

    Macro Returns:
        Returns 0 on success, otherwise -1 on error. When an error
        has occurred, <errno> contains a value indicating the type of
        error that has been detected.

    Macro Portability:
        n/a

    Macro See Also:
        print
 */
void
do_screen_dump(void)            /* int ([string filename], [string encoding]) */
{
    const char *filename = get_str(1);
  /*const char *encoding = get_xstr(2); TODO*/
    const int nrows = (ttrows() - 1);           /* edit arena */
    const int ncols = ttcols();
    register int row, col;
    char path[MAX_PATH];
    FILE *fp;

    if (NULL == filename || '\0' == filename[0]) {
        strxcpy(path, GRDUMP_MKSTEMP, sizeof(path));
        if (sys_mkstemp(path) >= 0) {
            filename = path;
        } else {
            filename = GRDUMP_DEFAULT;          /* default */
        }
    }

    if (NULL == (fp = fopen(filename, "w"))) {
        system_call(-1);
        acc_assign_int(-1);
        return;
    }

    for (row = 0; row < nrows; ++row) {
        /*
         *  dump visible rows
         */
        const VCELL_t *rp = vscreen[row];

        for (col = 0; col < ncols; ++col) {
            vbyte_t c = VBYTE_CHAR_GET(rp[col].primary);

            if (c >= CH_MIN && c <= CH_MAX) {
                switch (c) {                    /* convert special characters */
                case CH_VSCROLL:    case CH_HSCROLL:
                    c = ':';
                    break;
                case CH_VTHUMB:     case CH_HTHUMB:
                    c = '*';
                    break;
                case CH_HORIZONTAL:
                    c = '-';
                    break;
                case CH_VERTICAL:
                    c = '|';
                    break;
                case CH_TOP_LEFT:   case CH_TOP_RIGHT:
                case CH_BOT_LEFT:   case CH_BOT_RIGHT:
                case CH_TOP_JOIN:   case CH_BOT_JOIN:
                case CH_LEFT_JOIN:  case CH_RIGHT_JOIN:
                case CH_CROSS:
                    c = '+';
                    break;
                case CH_TOP_LEFT2:  case CH_TOP_RIGHT2:
                case CH_BOT_LEFT2:  case CH_BOT_RIGHT2:
                    c = '+';
                    break;
                case CH_RADIO_OFF:
                case CH_CHECK_OFF:
                case CH_PADDING:
                    c = ' ';
                    break;
                case CH_RADIO_ON:
                case CH_CHECK_ON:
                    c = 'x';
                    break;
                case CH_CHECK_TRI:
                    c = 'o';
                    break;
                }
            }
            fputc((int)c, fp);                  /* encoding/MCHAR??? */
        }
        fputc('\n', fp);
    }
    acc_assign_int(0);
    fclose(fp);
}

/*end*/
