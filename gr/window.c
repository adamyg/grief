#include <edidentifier.h>
__CIDENT_RCSID(gr_window_c,"$Id: window.c,v 1.42 2021/06/10 06:13:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: window.c,v 1.42 2021/06/10 06:13:02 cvsuser Exp $
 * Window basics.
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

#ifndef ED_LEVEL
#define ED_LEVEL 0
#endif

#include <editor.h>
#include "../libchartable/libchartable.h"
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "border.h"                             /* win_...() */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "cmap.h"                               /* default_cmap */
#include "color.h"                              /* ATTR_... */
#include "debug.h"
#include "display.h"
#include "echo.h"                               /* e...() */
#include "eval.h"                               /* get_...()/isa_...() */
#include "file.h"
#include "main.h"
#include "symbol.h"
#include "system.h"
#include "tty.h"
#include "undo.h"
#include "window.h"

static int                  window_sort_before(WINDOW_t *w1, WINDOW_t *w2);

WINDOW_t                    x_window_null = {0};

int                         x_window_nextattr = 0;

int                         x_window_popups = 0;

int                         x_display_top = 0;  /* top line of window arena */

uint32_t                    x_display_ctrl = DC_SHADOW;

int                         x_display_scrollcols = 0;
int                         x_display_scrollrows = 0;
int                         x_display_mincols = 0;
int                         x_display_minrows = 0;
int                         x_display_numbercols = 0;

uint32_t                    w_ctrl_mask = 0;
uint32_t                    w_ctrl_state = 0;
uint32_t                    w_ctrl_stack[WCTRLO_MAX] = {0};

int                         xf_borders = TRUE;

static IDENTIFIER_t         x_windows = 0;      /* next window identifier */

static WINDOWLIST_t         x_windowq;          /* window list */


WINDOW_t *
window_new(const WINDOW_t *clone)
{
    WINDOW_t *wp;

    if (0 == x_windows) {
        TAILQ_INIT(&x_windowq);                 /* initialise window queue */
    }

    if (NULL != (wp = (WINDOW_t *) chk_alloc(sizeof(WINDOW_t)))) {

        if (clone) {
            *wp = *clone;
        } else {
            memset(wp, 0, sizeof(WINDOW_t));
        }

        wp->w_magic = WINDOW_MAGIC;
        wp->w_magic2 = WINDOW_MAGIC;
        wp->w_num = ++x_windows;                /* window identifier 1 ... */
        wp->w_attr = 0;

        if (clone) {
            wp->w_title = NULL;
            wp->w_message = NULL;
            wp->w_dialogp = NULL;
            wp->w_bufp = NULL;

        } else {
            wp->w_cmap = x_default_cmap;
            wp->w_ctrl_state =                  /* default controls */
                (1 << WCTRLO_VERT_SCROLL) | (1 << WCTRLO_HORZ_SCROLL);
        }
    }
    return wp;
}


WINDOW_t *
window_first(void)
{
    WINDOW_t *wp;

    if (NULL != (wp = TAILQ_FIRST(&x_windowq))) {
        assert(WINDOW_MAGIC == wp->w_magic);
        assert(WINDOW_MAGIC == wp->w_magic2);
        assert(wp != &x_window_null);
    }
    return wp;
}


WINDOW_t *
window_next(WINDOW_t *wp)
{
    if (wp) {
        assert(WINDOW_MAGIC == wp->w_magic);
        assert(WINDOW_MAGIC == wp->w_magic2);
        if (NULL != (wp = TAILQ_NEXT(wp, w_node))) {
            assert(WINDOW_MAGIC == wp->w_magic);
            assert(WINDOW_MAGIC == wp->w_magic2);
            assert(wp != &x_window_null);
        }
    }
    return wp;
}


void
window_attr(WINDOW_t *wp)
{
    wp->w_attr = ATTR_WINDOW1 + x_window_nextattr;

    if (++x_window_nextattr >= 16) {
        x_window_nextattr = 0;
    }
}


void
window_append(WINDOW_t *wp)
{
    assert(wp);
    assert(wp != &x_window_null);
    assert(wp->w_num >= 1);

    ED_TRACE(("window_append(%p,num:%d)\n", wp, wp->w_num));

    {/* XXX - temporary test */
        WINDOW_t *twp;
        int found = 0;

        for (twp = window_first(); twp; twp = window_next(twp)) {
            if (twp == wp || twp->w_num == wp->w_num) {
                ++found;
            }
        }
        assert(0 == found);
    }

    TAILQ_INSERT_HEAD(&x_windowq, wp, w_node);

    window_sort();
    window_corners();
}



int
window_create(int type, const char *title, int x, int y, int w, int h)
{
    WINDOW_t *wp = window_new(NULL);

    if (NULL == wp) {
        acc_assign_int((accint_t) -1);          /* error */

    } else {
        wp->w_old_line = wp->w_top_line =
            wp->w_line = wp->w_col = 1;

        acc_assign_int((accint_t) wp->w_num);

        if (W_MENU == type) {
            wp->w_x = 0; 
            wp->w_y = 0;
            wp->w_h = 1; 
            wp->w_w = (uint16_t)(ttcols() - 1);

        } else {
            const int titlelen = (title ? (int)charset_utf8_swidth(title) : 0); /*MCHAR*/

            if (x < 0) {                        /* x within view */
                x = 0;
            } else if (x >= ttcols() - 2) {
                x = ttcols() - 2;
            }

            if (y < 0) {                        /* y window view */
                y = 0;
        //  } else if (y < x_topy) {            /* TABLINE/MENU */
        //      y = topy;
            } else if (y >= ttrows() - 2) {
                y = ttrows() - 2;
            }

            if (x_pt.pt_window_minrows > 0 && h < x_pt.pt_window_minrows) {
                h = x_pt.pt_window_minrows;     /* soft min height */
            } else if (h < 1) {
                h = 1;                          /* hard min height */
            }

            if (x_pt.pt_window_mincols > 0 && w < x_pt.pt_window_mincols) {
                w = x_pt.pt_window_mincols;     /* soft min width/columns */
            } else if (w < titlelen + 4) {
                w = titlelen + 4;               /* MAGIC - hard min width (check display.c) */
            }

            if (y + h >= ttrows() - 2) {        /* trim to screen size */
                if ((h = ttrows() - y - 3) < 1) {
                    if (x_pt.pt_window_minrows > 0) {
                        h =  x_pt.pt_window_minrows;
                    } else {
                        h = 1;
                    }
                }
            }

            wp->w_x = (uint16_t) x;
            wp->w_y = (uint16_t) y;
            wp->w_h = (uint16_t) h;
            wp->w_w = (uint16_t) w;
        }

        wp->w_status = WFHARD;
        wp->w_type = (uint16_t)type;
        wp->w_tab = 0;                          /* TABLINE */

        if (W_TILED == type) {
            if (!xf_borders) window_attr(wp);   /* background colours */

        } else if (W_POPUP == type) {
            /*
             *  enforce popup order
             */
            WINDOW_t *wp2;

            wp->w_priority = 1;                 /* new top priority */
            for (wp2 = window_first(); wp2; wp2 = window_next(wp2))
                if (W_POPUP == wp2->w_type && wp2->w_priority >= wp->w_priority) {
                    wp->w_priority = (uint16_t)(wp2->w_priority + 1);
                }
            ++x_window_popups;
        }

        wp->w_corner_hints[TL_CORNER] = CORNER_3  | CORNER_6;
        wp->w_corner_hints[TR_CORNER] = CORNER_9  | CORNER_6;
        wp->w_corner_hints[BL_CORNER] = CORNER_12 | CORNER_3;
        wp->w_corner_hints[BR_CORNER] = CORNER_12 | CORNER_9;

        window_title(wp, "", (title ? title : ""));
        window_append(wp);
        curwp = wp;                             /* current window */
        set_hooked();
    }
    return (NULL == wp ? -1 : wp->w_num);
}


/*  Function:           window_title
 *      Set a windows title top and bottom.
 *
 *  Parameters:
 *      wp - Window object address.
 *      title - Title line.
 *      message - Message line.
 *
 *  Returns:
 *      nothing
 */
void
window_title(WINDOW_t *wp, const char *title, const char *message)
{
    if (title) {
        if (wp->w_title && strcmp(title, wp->w_title)) {
            chk_free((void *)wp->w_title);
            wp->w_title = NULL;
        }

        if (NULL == wp->w_title && *title) {
            wp->w_title = chk_salloc(title);
        }
    }

    if (message) {
        if (wp->w_message && strcmp(message, wp->w_message)) {
            chk_free((void *)wp->w_message);
            wp->w_message = NULL;
        }

        if (NULL == wp->w_message && *message) {
            wp->w_message = chk_salloc(message);
        }
    }
}


int
window_top_line(WINDOW_t *wp, int top_line)
{
    const BUFFER_t *bp;

    if (NULL != wp) {
        if (top_line < 1) {
            top_line = 1;
        } else if (NULL != (bp = wp->w_bufp) && top_line > bp->b_numlines) {
            top_line = bp->b_numlines;
        }
        if (top_line != wp->w_top_line) {
            wp->w_top_line = top_line;
            window_modify(wp, WFHARD);
            return TRUE;
        }
    }
    return FALSE;
}


int
window_center_line(WINDOW_t *wp, int line)
{
    if (wp)  {
        return window_top_line(wp, line - (wp->w_h / 2));
    }
    return FALSE;
}


void
attach_buffer(WINDOW_t *wp, BUFFER_t *bp)
{
    if (wp && bp) {
        detach_buffer(wp);
        wp->w_bufp = bp;
        ++bp->b_nwnd;

        wp->w_top_line = bp->b_top;
        if ((wp->w_line = bp->b_line) < 1) {
            wp->w_line = 1;
        }
        wp->w_old_line = wp->w_line;
        if ((wp->w_col = bp->b_col) < 1) {
            wp->w_col = 1;
        }
        wp->w_old_col = wp->w_col;
        wp->w_eol_col = 0;

        wp->w_status |= WFHARD;
        window_title(wp, bp->b_title ? bp->b_title : sys_basename(bp->b_fname), NULL);

        curbp = bp;
        set_hooked();
    }
}


void
detach_buffer(WINDOW_t *wp)
{
    BUFFER_t *bp;

    if ((bp = wp->w_bufp) != NULL) {
        --bp->b_nwnd;
        wp->w_bufp = NULL;
        bp->b_line = wp->w_line;
        bp->b_col = wp->w_col;
        bp->b_top = wp->w_top_line;
    }
}


void
window_delete(WINDOW_t *wp)
{
    assert(wp);
    assert(WINDOW_MAGIC == wp->w_magic);
    assert(WINDOW_MAGIC == wp->w_magic2);
    assert(wp != &x_window_null);

    ED_TRACE(("window_delete(%p,num:%d)\n", wp, wp->w_num));

    /*
     *  unhook
     */
//  if (XF_TEST(5)) {
//        WINDOW_t *twp;
//        int found = 0;
//
//        for (twp = window_first(); twp; twp = window_next(twp)) {
//            if (twp == wp) {
//                ++found;
//            }
//        }
//        assert(1 == found);
//  }

    TAILQ_REMOVE(&x_windowq, wp, w_node);

    /*
     *  If we are deleting a window for the current buffer then we need
     *  to save the line/column information.
     */
    if (wp->w_bufp) {
        if (curbp && wp->w_bufp == curbp) {     /* import line/col */
            curbp->b_line = wp->w_line;
            curbp->b_col  = wp->w_col;
            curbp->b_top  = wp->w_top_line;
        }
        --wp->w_bufp->b_nwnd;                   /* decrement reference count */
    }

    /*
     *  release resources etc
     */
    if (wp->w_title)
        chk_free((void *)wp->w_title);
    if (wp->w_message)
        chk_free((void *)wp->w_message);
    if (W_POPUP == wp->w_type)
        --x_window_popups;
    wp->w_magic = ~WINDOW_MAGIC;
    wp->w_magic2 = ~WINDOW_MAGIC;
    chk_free(wp);

    window_sort();
    window_harden();
    window_corners();
}


/*  Function:           window_sort
 *      Function to sort the windows into order on the window list. The list is ordered
 *      with windows higher up the screen nearer the front of the list. 'higher' means
 *      higher and more to the left. This order is assumed by certain bits of code,
 *      e.g. the window resizer on a SIGWINCH and by the display algorithms.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
window_sort(void)
{
    WINDOWLIST_t t_windowq;                     /* temporary window list */
    WINDOW_t *cwp, *nwp, *iwp;                  /* current, next and insert */

    TAILQ_INIT(&t_windowq);                     /* build temporary queue */
    for (cwp = window_first(); cwp; cwp = nwp) {
        nwp = window_next(cwp);
        TAILQ_INSERT_TAIL(&t_windowq, cwp, w_node);
    }

    TAILQ_INIT(&x_windowq);                     /* rebuild window queue */
    for (cwp = TAILQ_FIRST(&t_windowq); cwp; cwp = nwp) {
        assert(WINDOW_MAGIC == cwp->w_magic);
        assert(WINDOW_MAGIC == cwp->w_magic2);
        nwp = TAILQ_NEXT(cwp, w_node);

        for (iwp = window_first(); iwp; iwp = window_next(iwp)) {
            assert(WINDOW_MAGIC == iwp->w_magic);
            assert(WINDOW_MAGIC == iwp->w_magic2);
            if (window_sort_before(cwp, iwp)) {
                TAILQ_INSERT_BEFORE(iwp, cwp, w_node);
                break;
            }
        }
        if (! iwp) {                            /* end of queue, insert */
            TAILQ_INSERT_TAIL(&x_windowq, cwp, w_node);
        }
    }
}


/*  Function:           window_sort_before
 *      Function used to compare a windows position and determine which window comes
 *      first on the screen. Used by window_sort().
 *
 *  Parameters:
 *      w1 - First window reference.
 *      w2 - Second window reference.
 *
 *  Returns:
 *      Sort order.
 */
static int
window_sort_before(WINDOW_t *w1, WINDOW_t *w2)
{
    /* Hiddens goto bottom of the stack. */
    if (WFTST(w1, WF_HIDDEN) != WFTST(w2, WF_HIDDEN)) {
        return (WFTST(w1, WF_HIDDEN) ? TRUE : FALSE);
    }

    /* Popups have higher priority than tiled windows. */
    if (w1->w_type != w2->w_type) {
        if (W_POPUP == w1->w_type) {
            return FALSE;
        }
        return TRUE;
    }

    /* Window priority */
    if (w1->w_priority != w2->w_priority) {
        return (w1->w_priority > w2->w_priority ? FALSE : TRUE);
    }

    /*
     *  Windows which start at the same place on the screen, are
     *  sorted by age, ie. newest window sits on top of older window.
     */
    if (w1->w_y == w2->w_y && w1->w_x == w2->w_x) {
        return (w1->w_num - w2->w_num);
    }

    if (w1->w_y < w2->w_y) {
        return TRUE;
    }

    if (w1->w_y == w2->w_y) {
        return w1->w_x < w2->w_x;
    }

    return FALSE;
}


/*  Function:           window_argument
 *      Cook the specified argument index 'n' and return the associated window (if any).
 *
 *  Parameters:
 *      n - Argument index.
 *
 *  Returns:
 *      Window object address, otherwise current-window.
 */
WINDOW_t *
window_argument(int argi)
{
    if (isa_undef(argi)) {                      /* NULL ==> current window */
        assert(!curwp || (WINDOW_MAGIC == curwp->w_magic && WINDOW_MAGIC == curwp->w_magic2));
        return curwp;
    }
    return window_lookup(get_xinteger(argi, 0));
}


/*  Function:           window_argument
 *      Cook the specified argument index 'n' and return the associated window (if any).
 *
 *  Parameters:
 *      n - Argument index.
 *
 *  Returns:
 *      Window object address, otherwise NULL.
 */
WINDOW_t *
window_xargument(int argi)
{
    if (isa_undef(argi)) {                      /* NULL ==> current window */
        return NULL;
    }
    return window_lookup(get_xinteger(argi, 0));
}


/*  Function:           window_lookup
 *      Locate a window by identifier 'winnum'.
 *
 *  Parameters:
 *      num - Window number.
 *
 *  Returns:
 *      WIndow object address, otherwise NULL.
 */
WINDOW_t *
window_lookup(int num)
{
    WINDOW_t *wp;

    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_num == num) {
            return wp;
        }
    return NULL;
}


/*  Function:           window_hsplit
 *      Split the current window. A window smaller than 3 lines cannot be split.
 *
 *      The only other error that is possible is a "malloc" failure allocating the
 *      structure for the new window.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      New window object.
 */
WINDOW_t *
window_hsplit(void)
{
    WINDOW_t *wp;

    if (curwp->w_h < W_MINHEIGHT) {
        ewprintf("Window would be too small.");
        return NULL;
    }

    wp = window_new(curwp);
    wp->w_type = W_TILED;
    wp->w_bufp = curbp;
    ++curbp->b_nwnd;

    wp->w_h     = (uint16_t)(curwp->w_h / 2 - 1);
    curwp->w_h  = (uint16_t)(curwp->w_h - wp->w_h);
    wp->w_y     = (uint16_t)(curwp->w_y + curwp->w_h);
    --curwp->w_h;

    if (curwp->w_top_line + curwp->w_h < curwp->w_line) {
        curwp->w_top_line = curwp->w_line - (win_height(curwp) / 2);
        if (curwp->w_top_line < 1) {
            curwp->w_top_line = 1;
        }
    }
    curwp->w_old_line = curwp->w_line;
    curwp->w_old_col = curwp->w_col;
    curwp->w_status |= WFHARD;
    if (!xf_borders) window_attr(wp);
    window_append(wp);
    window_title(wp, curbp->b_title ? curbp->b_title : sys_basename(curbp->b_fname), "");

    return wp;
}


/*  Function:           window_vsplit
 *      Split the current window. A window smaller than 2 lines cannot be split.
 *
 *      The only other error that is possible is a "malloc" failure allocating the
 *      structure for the new window.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      New Window object address, otherwise NULL.
 */
WINDOW_t *
window_vsplit(void)
{
    WINDOW_t *wp = NULL;

    if (curwp->w_w < W_MINWIDTH) {
        ewprintf("Window would be too small.");

    } else {
        int w;

        /* clone window */
        wp = window_new(curwp);
        wp->w_type = W_TILED;
        wp->w_bufp = curbp;
        ++curbp->b_nwnd;

        /* position */
        wp->w_left_offset = curwp->w_left_offset = 0;
        w           = wp->w_w;
        wp->w_w     /= 2;
        curwp->w_w  = (uint16_t)(w - wp->w_w - 1);
        wp->w_x     = (uint16_t)(curwp->w_x + curwp->w_w + 1);

        /* display */
        if (!xf_borders) window_attr(wp);
        window_append(wp);

        curwp->w_status |= WFHARD;
        wp->w_status |= WFHARD;

        window_title(wp, sys_basename(curbp->b_fname), "");
    }
    return wp;
}


void
win_modify(int flag)
{
    window_modify(curwp, flag);
}


void
window_modify(WINDOW_t *wp, int flag)
{
    const LINENO cline = *cur_line;

    if (NULL == wp || wp->w_bufp != curbp) {
        return;
    }

    if ((flag & WFDELL) && (wp->w_status & WFDELL)) {
        flag |= WFHARD;                         /* promote multiple DELS */
    }

    if ((WFMOVE == flag || WFPAGE == flag) &&
                curbp->b_anchor) {              /* XXX - check anchor arena */
        flag |= WFEDIT;                         /* promote moves */
    }

    wp->w_status |= flag;

    if (WFEDIT & wp->w_status) {
        if (0 == wp->w_mined || wp->w_mined > cline) {
            wp->w_mined = cline;
        }
        if (wp->w_maxed < cline) {
            wp->w_maxed = cline;
        }
    }
}


/*
 *  Function to flag all windows as needing hard update after a change which may affect all windows.
 */
void
window_harden(void)
{
    WINDOW_t *wp;

    for (wp = window_first(); wp; wp = window_next(wp)) {
        wp->w_status |= WFHARD;
    }
}



/*  Function:           window_corners
 *      Function to set flags for the display code for background windows so that
 *      abutting corners are drawn properly.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
window_corners(void)
{
    WINDOW_t *wp, *wp1;
    int c;

#define TL(w)   (w->w_y * ttcols() + w->w_x)
#define TR(w)   (w->w_y * ttcols() + w->w_x + w->w_w + 1)
#define BL(w)   ((w->w_y + w->w_h + 1) * ttcols() + w->w_x)
#define BR(w)   ((w->w_y + w->w_h + 1) * ttcols() + w->w_x + w->w_w + 1)

    for (wp = window_first(); wp; wp = window_next(wp)) {
        /* Only process tited (ie background) windows, skipping popup's and menu's. */
        if (W_TILED != wp->w_type) {
            continue;
        }

        /* Top left corner of window. */
        c = TL(wp);
        wp->w_corner_hints[TL_CORNER] = CORNER_3 | CORNER_6;
        for (wp1 = window_first(); wp1; wp1 = window_next(wp1)) {
            if (wp1 == wp || W_TILED != wp1->w_type) {
                continue;
            }

            if (c == TR(wp1)) {
                wp->w_corner_hints[TL_CORNER] |= CORNER_9;

            } else if (c == BL(wp1)) {
                wp->w_corner_hints[TL_CORNER] |= CORNER_12;

            } else if (c == BR(wp1)) {
                wp->w_corner_hints[TL_CORNER] |= CORNER_9 | CORNER_12;
                break;
            }
        }

        /* Top right corner of window. */
        c = TR(wp);
        wp->w_corner_hints[TR_CORNER] = CORNER_9 | CORNER_6;
        for (wp1 = window_first(); wp1; wp1 = window_next(wp1)) {
            if (wp1 == wp || W_TILED != wp1->w_type)
                continue;

            if (c == BR(wp1)) {
                wp->w_corner_hints[TR_CORNER] |= CORNER_12;

            } else if (c == TR(wp1)) {
                wp->w_corner_hints[TR_CORNER] |= CORNER_3;

            } else if (c == BL(wp1)) {
                wp->w_corner_hints[TR_CORNER] |= CORNER_12 | CORNER_3;
                break;
            }
        }

        /* Bottom left corner of window. */
        c = BL(wp);
        wp->w_corner_hints[BL_CORNER] = CORNER_12 | CORNER_3;
        for (wp1 = window_first(); wp1; wp1 = window_next(wp1)) {
            if (wp1 == wp || W_TILED != wp1->w_type)
                continue;

            if (c == BR(wp1)) {
                wp->w_corner_hints[BL_CORNER] |= CORNER_9;

            } else if (c == TR(wp1)) {
                wp->w_corner_hints[BL_CORNER] |= CORNER_6;

            } else if (c == TL(wp1)) {
                wp->w_corner_hints[BL_CORNER] |= CORNER_9 | CORNER_6;
                break;
            }
        }

        /* Bottom right corner of window. */
        c = BR(wp);
        wp->w_corner_hints[BR_CORNER] = CORNER_12 | CORNER_9;
        for (wp1 = window_first(); wp1; wp1 = window_next(wp1)) {
            if (wp1 == wp || W_TILED != wp1->w_type)
                continue;

            if (c == BL(wp1)) {
                wp->w_corner_hints[BR_CORNER] |= CORNER_3;

            } else if (c == TR(wp1)) {
                wp->w_corner_hints[BR_CORNER] |= CORNER_6;

            } else if (c == TL(wp1)) {
                wp->w_corner_hints[BR_CORNER] |= CORNER_3 | CORNER_6;
                break;
            }
        }
    }
}


int
window_ctrl_set(WINDOW_t *wp, uint32_t ctrl)
{
    const uint32_t bit = (1 << ctrl);
    int value = 0;

    if (NULL != wp) {
        value = (bit & wp->w_ctrl_state) ? 1 : 0;
        wp->w_ctrl_state |= bit;
    }
    return value;
}


void
window_ctrl_clr(WINDOW_t *wp, uint32_t ctrl)
{
    const uint32_t nbit = ~(1 << ctrl);

    if (NULL != wp) {
        wp->w_ctrl_state &= nbit;
    }
}


/*  Function:           window_ctrl_test
 *      Determine whether the specified control is active for the given window.,
 *
 *  Parameters:
 *      wp - Window object address.
 *      ctrl - Control object.
 *
 *  Returns:
 *      *true* or *false*
 */
int
window_ctrl_test(const WINDOW_t *wp, uint32_t ctrl)
{
    const uint32_t bit = (1 << ctrl);

    if (NULL != wp) {
        if (bit & WCTRL0_INTERNAL) {
            return (0 != (wp->w_ctrl_state & bit) ? 1 : 0);
        }
        return (0 != (wp->w_ctrl_state & w_ctrl_state & bit) ? 1 : 0);
    }
    return ((w_ctrl_state & bit) != 0 ? 1 : 0);
}
/*end*/
