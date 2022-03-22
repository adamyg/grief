#include <edidentifier.h>
__CIDENT_RCSID(gr_tty_c,"$Id: tty.c,v 1.30 2021/06/22 15:52:54 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tty.c,v 1.30 2021/06/22 15:52:54 cvsuser Exp $
 * Common basic tty functionality.
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

#include "asciidefs.h"
#include "cmap.h"
#include "debug.h"
#include "color.h"
#include "display.h"
#include "main.h"
#include "system.h"                             /* sys_... () */
#include "tty.h"

static int                  tty_nrows = -1;     /* screen size */
static int                  tty_ncols = -1;

static int                  tty_ttrow = -1;     /* cursor location */
static int                  tty_ttcol = -1;

int                         tty_open = 0;
int                         tty_egaflag = -1;
int                         tty_needresize = 0;

int                         tty_tceeol = 0;     /* costs */
int                         tty_tcinsl = 0;
int                         tty_tcdell = 0;

features_t                  x_pt = {0};         /* terminal characteristics */

scrfn_t                     x_scrfn = {0};      /* terminal descriptor */

static scrprofile_t         x_scrprofile;       /* terminal profile */

#if defined(SIGWINCH)
static void                 sighandler_winch(int sig);
#endif
static __CINLINE void       boxchar(char *pt, char ch);


/*  Function:           ttdefaults
 *      Configure terminal default parameters.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttdefaults(void)
{
    x_pt.pt_magic = PT_MAGIC;
    x_pt.pt_magic2 = PT_MAGIC;

    x_pt.pt_color = -1;                         /* auto */
    x_pt.pt_schemedark = -1;                    /* auto */
    x_pt.pt_defaultfg = -1;                     /* default foreground */
    x_pt.pt_defaultbg = -1;                     /* default background */
    x_pt.pt_codepage = -1;

    x_pt.pt_xtcompat = -1;                      /* XTERM compatible terminal. */
    x_pt.pt_xtcursor = -1;                      /* XTERM cursor color control. */
    x_pt.pt_xttitle = -1;                       /* XTERM title control. */
    x_pt.pt_xtpalette  = -1;                    /* XTERM palette control. */

    x_pt.pt_vtdatype = -1;                      /* Device attribute type. */
    x_pt.pt_vtdaversion = -1;                   /* Device attribute version. */

    x_pt.pt_screen_rows = -1;                   /* Screen rows. */
    x_pt.pt_screen_cols = -1;                   /* Screen columns. */
    x_pt.pt_tty_fast = -1;                      /* Fast screen optimisations. */
    x_pt.pt_tty_graphicsbox = -1;               /* Box characters require graphics-mode */

    x_pt.pt_lineno_columns = -1;                /* Line-number display columns. */
    x_pt.pt_window_minrows = -1;                /* Window rows limit. */
    x_pt.pt_window_mincols = -1;                /* Window column limit. */
}


/*  Function:           ttopen
 *      Open the console.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttopen(void)
{
    trace_log("ttopen()\n");
    if (0 == tty_open) {
        if (x_scrfn.scr_open) {
            (*x_scrfn.scr_open)(&x_scrprofile);

            if (x_scrprofile.sp_rows > 0 &&
                    x_scrprofile.sp_cols > 0) {
                tty_nrows = x_scrprofile.sp_rows;
                tty_ncols = x_scrprofile.sp_cols;
            }
        }
#if defined(SIGWINCH)
        sys_signal(SIGWINCH, sighandler_winch);
#endif
        tty_open = 1;
    }
}


#if defined(SIGWINCH)
/*  Function:           sighandler_winch
 *      Called upon a SIGWINCH signal telling us the windows changed size.
 *
 *  Parameters:
 *      sig -               Signal number.
 *
 *  Returns:
 *      nothing.
 */
static void
sighandler_winch(int sig)
{
    sig = sig;
    if (! vtupdating()) {
        ttresize();
    } else {
        ++tty_needresize;
    }
    sys_signal(SIGWINCH, sighandler_winch);
}
#endif  /*SIGWINCH*/


/*  Function:           ttwinch
 *      Winch soft action, allowing safe handling of SIGWINCH events.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttwinch(void)
{
    while (tty_needresize > 0) {
        --tty_needresize;
        ttresize();
    }
}


/*  Function:           ttresize
 *      Resize the display, redisplay if the dimensions have changed.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      1 if the display was changed, otherwise 0 or -1 winch support not available.
 */
int
ttresize(void)
{
    int nrows = -1, ncols = -1;

    if (NULL == x_scrfn.scr_winch) {
        return -1;
    }
    (*x_scrfn.scr_winch)(&nrows, &ncols);
    return ttwinched(nrows, ncols);
}


/*  Function:           ttwinched
 *      Execute a winch request.
 *
 *  Parameters:
 *      nrows -             New screen rows.
 *      ncols -             Columns.
 *
 *  Returns:
 *      1 if the display was changed, otherwise 0 or -1 winch support not available.
 */
int
ttwinched(int nrows, int ncols)
{
    if (nrows > 0 && ncols > 0) {
        const int orows = ttrows(), ocols = ttcols();

        if (nrows != orows || ncols != ocols) {
            tty_nrows = nrows;
            tty_ncols = ncols;
            vtready();
            vtwinch(ocols, orows);
            vtupdate();
            return 1;
        }
        return 0;
    }
    return -1;
}


/*  Function:           ttready
 *      Ready the terminal interface, run-time (re)initialisation.
 *
 *  Parameters:
 *      repaint -           *true* if the screen should be repainted.
 *
 *  Returns:
 *      nothing.
 */
void
ttready(int repaint)
{
    trace_log("ttready(%d)\n", repaint);
    x_scrprofile.sp_lastsafe = TRUE;            /* assume unless cleared */
    x_scrprofile.sp_rows = x_scrprofile.sp_cols = 0;
    if (x_scrfn.scr_ready) {
        (*x_scrfn.scr_ready)(repaint, &x_scrprofile);
    } else if (repaint) {
        vtgarbled();                            /* force screen update */
    }
}


/*  Function:           ttclose
 *      Close the console.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttclose(void)
{
    trace_log("ttclose()\n");
    if (tty_open) {
        if (x_scrfn.scr_close) {
            (*x_scrfn.scr_close)();
        }
        tty_open = 0;
    }
}


/*  Function:           ttdisplay
 *      Enable the display.
 *
 *      This interface is hooked into the display_windows() primitive which (by default) is
 *      post execution of any terminal specific macros, for example <tty/xterm.cr>
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttdisplay(void)
{
    trace_log("ttdisplay()\n");
    if (x_scrfn.scr_display) {
        (*x_scrfn.scr_display)();
        cmap_init();                            /* reinitialise the character map */
    }
}


/*  Function:           ttfeature
 *      Signal a terminal feature change.
 *
 *  Parameters:
 *      ident -             Feature identifier.
 *
 *  Returns:
 *      nothing.
 */
void
ttfeature(int ident)
{
    trace_log("ttfeature(%d)\n", ident);

    if (x_scrfn.scr_feature) {
        (*x_scrfn.scr_feature)(ident, &x_scrprofile);
    }

    if (TF_EIGHT_BIT == ident || TF_ENCODING == ident) {
        cmap_init();                            /* reinitialise character-map */
    }
}


/*  Function:           ttlastsafe
 *      Return whether it is safe to write to the last character within the bottom column.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      TRUE or FALSE.
 */
int
ttlastsafe(void)
{
    return x_scrprofile.sp_lastsafe;
}


/*  Function:           ttcolordepth
 *      Return the current maximum color depth
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Color depth
 */
int
ttcolordepth(void)
{
    return x_scrprofile.sp_colors;
}


/*  Function:           ttmove
 *      Move the cursor to the specified row and column position, using an origin 0 .
 *
 *      Implementation should try to optimize out extra moves; redisplay may have
 *      left the cursor in the right location last time!
 *
 *  Parameters:
 *      row -               Screen row.
 *      col -               Column.
 *
 *  Returns:
 *      nothing.
 */
void
ttmove(int row, int col)
{
    const int rows = ttrows() - 1, cols = ttcols() - 1,
            ttrow = ttatrow(), ttcol = ttatcol();

    assert(row >= 0);
    assert(row <= rows);
    assert(col >= 0);
    assert(col <= cols);

    if (ttrow != row || ttcol != col) {
        if (x_scrfn.scr_move) {
            (*x_scrfn.scr_move)(row, col);
        }
        ttposset(row, col);
    }
}


/*  Function:           ttcursor
 *      Set the cursor status.
 *
 *  Parameters:
 *      visible -           Visible status (TRUE, FALSE or -1).
 *      imode -             Insert mode (TRUE, FALSE or -1).
 *      virtual_space -     Virtual space mode (TRUE, FALSE or -1).
 *
 *  Returns:
 *      reports the current visible status, otherwise -1.
 */
int
ttcursor(int visible, int imode, int virtual_space)
{
    trace_log("ttcursor(visible:%d, imode:%d, virtual:%d)\n", visible, imode, virtual_space);
    if (x_scrfn.scr_cursor) {
        return (*x_scrfn.scr_cursor)(visible, imode, virtual_space);
    }
    return -1;
}


/*  Function:           tttitle
 *      Set the console title
 *
 *  Parameters:
 *      title -             Title buffer.
 *
 *  Returns:
 *      nothing.
 */
void
tttitle(const char *title)
{
    if (NULL == title) {
        title = x_appname;
    }
    if (title) {
        trace_log("tttitle(%s)\n", title);
        if (x_pt.pt_xttitle >= 1 && x_scrfn.scr_names) {
            (*x_scrfn.scr_names)(title, NULL);
        }
    }
}


/*  Function:           ttbeep
 *      Terminal beep/visual notice.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttbeep(void)
{
    trace_log("ttbeep()\n");
    if (tty_open) {
        if (x_scrfn.scr_beep) {
            (*x_scrfn.scr_beep)(0, 0);
        }
    }
}


/*  Function:           ttclear
 *      Clear the screen in the terminals nornal color.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttclear(void)
{
    trace_log("ttclear()\n");
    if (tty_open) {
        if (x_scrfn.scr_clear) {
            (*x_scrfn.scr_clear)();
        }
    }
}


/*  Function:           ttcolornormal
 *      Set the current color the terminals normal color.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttcolornormal(void)
{
    trace_log("ttcolornormal()\n");
    if (tty_open) {
        if (x_scrfn.scr_control) {
            (*x_scrfn.scr_control)(SCR_CTRL_NORMAL, 0);
        }
    }
}


/*  Function:           ttflush
 *      Flush the tty buffer/display.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
ttflush(void)
{
    if (tty_open) {
        if (x_scrfn.scr_flush) {
            (*x_scrfn.scr_flush)();
        }
    }
}


/*  Function:           ttinsl
 *      Insert the specified number of blank lines 'lines' onto the screen using the
 *      region 'row' and 'bot', scrolling the last line within the region off the
 *      screen.
 *
 *  Parameters:
 *      row -               Top row of region.
 *      bot -               Bottom of region.
 *      nlines -            Number of lines to be inserted.
 *      fillcolor -         Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
int
ttinsl(int row, int bot, int nlines, vbyte_t fillcolor)
{
    if (x_scrfn.scr_insl) {
        return (*x_scrfn.scr_insl)(row, bot, nlines, fillcolor);
    }
    return FALSE;
}


/*  Function:           ttdell
 *      Delete the specified number of blank lines 'nlines' onto the screen using the region
 *      'row' and 'bot', replacing the last line within a blank line.
 *
 *  Parameters:
 *      row -               Top row of region.
 *      bot -               Bottom of region.
 *      nlines -            Number of lines to be deleted.
 *      fillcolor -         Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
int
ttdell(int row, int bot, int nlines, vbyte_t fillcolor)
{
    if (x_scrfn.scr_dell) {
        return (*x_scrfn.scr_dell)(row, bot, nlines, fillcolor);
    }
    return FALSE;
}


/*  Function:           tteeol
 *      Erase to end of line in the 'current' color.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
int
tteeol(void)
{
    if (x_scrfn.scr_eeol) {
        return (*x_scrfn.scr_eeol)();
    }
    return FALSE;
}


/*  Function:           ttrepeat
 *      Repeat the specified characters.
 *
 *  Parameters:
 *      cnt -               Character count.
 *      fill -              Fill character.
 *      where -             Requested cursor location at end of operation.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
int
ttrepeat(int cnt, vbyte_t fill, int where)
{
    if (x_scrfn.scr_repeat) {
        if (cnt > 0) {
            (*x_scrfn.scr_repeat)(cnt, fill, where);
        }
        return TRUE;
    }
    return FALSE;
}


/*  Function:           ttboxcharacters
 *      Setup box characters.
 *
 *  Parameters:
 *      void
 *
 *  Returns:
 *      nothing.
 */
void
ttboxcharacters(int force)
{
    if ('+' != x_pt.pt_top_left[0] &&
            (force || x_pt.pt_tty_graphicsbox)) {
        boxchar(x_pt.pt_top_left,   '+');
        boxchar(x_pt.pt_top_right,  '+');
        boxchar(x_pt.pt_bot_left,   '+');
        boxchar(x_pt.pt_bot_right,  '+');
        boxchar(x_pt.pt_vertical,   '|');
        boxchar(x_pt.pt_horizontal, '-');
        boxchar(x_pt.pt_top_join,   '+');
        boxchar(x_pt.pt_bot_join,   '+');
        boxchar(x_pt.pt_cross,      '+');
        boxchar(x_pt.pt_left_join,  '+');
        boxchar(x_pt.pt_right_join, '+');
        boxchar(x_pt.pt_thumb,      '#');
        boxchar(x_pt.pt_scroll,     0);
        x_pt.pt_tty_graphicsbox = FALSE;
    }
}


static __CINLINE void
boxchar(char *pt, char ch)
{
    pt[0] = ch;
    pt[1] = '\0';
}


/*  Function:           ttspecchar
 *      Remap the special character to the terminal specific value.
 *
 *  Parameters:
 *      ch -                Character value to be mapped.
 *
 *  Returns:
 *      Retrieve special-char mapping.
 */
const char *
ttspecchar(vbyte_t ch)
{
    const char *cp = NULL;

    ch &= ~VBYTE_ATTR_MASK;
    switch (ch) {
    case CH_HTHUMB:
    case CH_VTHUMB:
        cp = (x_pt.pt_thumb[0]  ? x_pt.pt_thumb  : "*");
        break;
    case CH_VSCROLL:
        cp = (x_pt.pt_scroll[0] ? x_pt.pt_scroll : x_pt.pt_vertical);
        break;
    case CH_HSCROLL:
        cp = (x_pt.pt_scroll[0] ? x_pt.pt_scroll : x_pt.pt_horizontal);
        break;
    case CH_HORIZONTAL: cp = x_pt.pt_horizontal;  break;
    case CH_VERTICAL:   cp = x_pt.pt_vertical;    break;
    case CH_TOP_LEFT:   cp = x_pt.pt_top_left;    break;
    case CH_TOP_RIGHT:  cp = x_pt.pt_top_right;   break;
    case CH_TOP_JOIN:   cp = x_pt.pt_top_join;    break;
    case CH_BOT_LEFT:   cp = x_pt.pt_bot_left;    break;
    case CH_BOT_RIGHT:  cp = x_pt.pt_bot_right;   break;
    case CH_BOT_JOIN:   cp = x_pt.pt_bot_join;    break;
    case CH_LEFT_JOIN:  cp = x_pt.pt_left_join;   break;
    case CH_RIGHT_JOIN: cp = x_pt.pt_right_join;  break;
    case CH_CROSS:      cp = x_pt.pt_cross;       break;
    default:
        break;
    }
    return (cp && cp[0] ? cp : NULL);
}


/*  Function:           ttrows
 *      Retrieve the console row number.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Row number.
 */
int
ttrows(void)
{
    assert(tty_nrows > 0);
    return tty_nrows;
}


/*  Function:           ttcols
 *      Retrieve the console column number.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Column number.
 */
int
ttcols(void)
{
    assert(tty_ncols > 0);
    return tty_ncols;
}


int
ttatrow(void)
{
    return tty_ttrow;
}


int
ttatcol(void)
{
    return tty_ttcol;
}


void
ttposset(int row, int col)
{
    assert(row >= 0);
    assert(col >= 0);
    tty_ttrow = row;
    tty_ttcol = col;
}


void
ttposget(int *row, int *col)
{
    assert(tty_ttrow >= 0);
    assert(tty_ttcol >= 0);
    if (row) *row = tty_ttrow;
    if (col) *col = tty_ttcol;
}


void
ttposinvalid(void)
{
    tty_ttrow = -1;
    tty_ttcol = -1;
}


/*  Function:           ttbandw
 *      Black and white coloriser, common to all drivers.
 *
 *   Parameters:
 *      attr -              Attribute.
 *      underline -         Whether underline is supported.
 *      italic -            Whether italic is supported.
 *      blink -             Whether blink is supported.
 *
 *  Returns:
 *      Resulting colorstyle.
 */
int
ttbandw(int attr, int underline, int italic, int blink)
{
    int nstyle = 0;

    if (ATTR_NORMAL != attr) {
        if (ATTR_ERROR == attr) {               /* errors */
            nstyle = COLORSTYLE_BOLD;
            if (blink && 2 == xf_underline) {
                nstyle = COLORSTYLE_BLINK;
            }

        } else if (ATTR_TITLE == attr ||        /* active windows */
                        ATTR_POPUP_TITLE == attr ||
                        ATTR_DIALOG_TITLE ==attr) {
            nstyle = COLORSTYLE_BOLD;

        } else if (ATTR_HILITE == attr ||       /* marked */
                        ATTR_POPUP_HILITE == attr ||
                        ATTR_PROMPT_COMPLETE == attr ||
                        ATTR_DIALOG_FOCUS == attr ||
                        ATTR_DIALOG_HILITE == attr ||
                        ATTR_DIALOG_EDIT_COMPLETE == attr) {
            nstyle = COLORSTYLE_STANDOUT;

        } else if (ATTR_STANDOUT == attr ||
                        ATTR_POPUP_STANDOUT == attr ||
                        ATTR_PROMPT == attr) {
            nstyle = COLORSTYLE_BOLD;

        } else if (ATTR_COMMENT == attr ||
                        ATTR_TODO == attr ||
                        ATTR_STRING == attr) {
            if (italic) {
                nstyle = COLORSTYLE_ITALIC;
            } else {
                nstyle = COLORSTYLE_BOLD;
            }
                                                /* keywords */
        } else if ((attr >= ATTR_KEYWORD_MIN && attr <= ATTR_KEYWORD_MAX) ||
                        ATTR_DIALOG_HOTKEY_NORMAL == attr || ATTR_DIALOG_HOTKEY_FOCUS  == attr ||
                        ATTR_DIALOG_BUT_KEY_NORMAL == attr || ATTR_DIALOG_BUT_KEY_FOCUS == attr) {
            if (underline & xf_underline /*-1 or 1*/) {
                nstyle = COLORSTYLE_UNDERLINE;
            } else if (italic) {
                nstyle = COLORSTYLE_ITALIC;
            }

        } else if (ATTR_SHADOW == attr) {
            nstyle = COLORSTYLE_REVERSE;
        }
    }
    return nstyle;
}


/*  Function:           ttstringcopy
 *      Import termcap style strings, converting escaped character sequences.
 *
 *   Parameters:
 *      dp - Destination cursor.
 *      dplen - Length of destination buffer.
 *      bp - Buffer object address.
 *      delim - Token delimiter.
 *
 *  Returns:
 *      Final cursor address.
 */
char *
ttstringcopy(char *dp, int dplen, const char *bp, int delim)
{
#define XDIGIT(x)   \
        (x <= '9' ? x - '0' : (x >= 'a' && x <= 'z') ? x - 'a' + 10 : (x - 'A' + 10))
    int n;

    while (*bp != delim && *bp && --dplen > 0) {
        if ('^' == *bp) {
            *dp++ = *++bp & 0x1f;               /* ESCx */
            bp++;

        } else if ('\\' == *bp) {               /* \<character> */
            switch (*++bp) {
            case 'E': *dp++ = ASCIIDEF_ESC; break;
            case 'r': *dp++ = '\r'; break;
            case 'n': *dp++ = '\n'; break;
            case 't': *dp++ = '\t'; break;
            case 'b': *dp++ = '\b'; break;
            case 'f': *dp++ = '\f'; break;
            case '0': case '1': case '2': case '3':
                n = 0;
                while (*bp >= '0' && *bp <= '7') {
                    n = 8 * n + *bp++ - '0';
                }
                --bp;
                *dp++ = (char) n;
                break;
            case 'x':
                bp++;
                n = XDIGIT(*bp);
                ++bp;
                n = n * 16 + XDIGIT(*bp);
                *dp++ = (char) n;
                break;
            case 0:
                *dp++ = '\0';
                return dp;
            default:
                *dp++ = *bp;
                break;
            }
            ++bp;
        } else {
            *dp++ = *bp++;
        }
    }
    *dp++ = '\0';
    return dp;

#undef  XDIGIT
}
/*end*/
