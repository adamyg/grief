#include <edidentifier.h>
__CIDENT_RCSID(gr_border_c,"$Id: border.c,v 1.16 2024/08/18 10:49:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: border.c,v 1.16 2024/08/18 10:49:23 cvsuser Exp $
 * Window helper function, dealing with size and border requirements.
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

#include "border.h"         /* local prototypes */
#include "tty.h"            /* ttrows, ttcols */
#include "window.h"         /* xf_borders */

#define LBORDERS(wp)        ((W_POPUP == wp->w_type || xf_borders) ? 1 : 0)
#define TBORDERS(wp)        ((W_POPUP == wp->w_type || xf_borders) ? 1 : 0)
#define RBORDERS(wp)        ((W_POPUP == wp->w_type || xf_borders || \
                                (wp->w_x + wp->w_w + 1) < (ttcols() - 1)) ? 1 : 0)
#define BBORDERS(wp)        ((W_POPUP == wp->w_type || xf_borders || \
                                (wp->w_y + wp->w_h + 1) < (ttrows() - 2)) ? 1 : 0)


/*
 *  win_tborder ---
 *      Top border size.
 */
int
win_tborder(const WINDOW_t *wp)
{
    if (W_MENU == wp->w_type) {
        return 0;
    }
    return TBORDERS(wp) + (wp->w_bufp && BFTST(wp->w_bufp, BF_RULER) ? 1 : 0);
}


/*
 *  win_bborder ---
 *      Bottom border size.
 */
int
win_bborder(const WINDOW_t *wp)
{
    if (W_MENU == wp->w_type) {
        return 0;
    }
    return BBORDERS(wp);
}


/*
 *  win_rborder ---
 *      Right border size.
 */
int
win_rborder(const WINDOW_t *wp)
{
    if (W_MENU == wp->w_type) {
        return 0;
    }
    return RBORDERS(wp);
}


/*
 *  win_lborder ---
 *      Left border size.
 */
int
win_lborder(const WINDOW_t *wp)
{
    if (W_MENU == wp->w_type) {
        return 0;
    }
    return LBORDERS(wp);
}


/*
 *  win_height ---
 *      Return the height of the window, accounting for borders and ruler.
 */
int
win_height(const WINDOW_t *wp)
{
    int height = wp->w_h;

    if (W_MENU != wp->w_type) {
        const int tborder = win_tborder(wp);

        if (tborder >= 2) {                     /* ruler */
            height -= (tborder - 1);
        } else if (0 == tborder) {
            height += 1;
        }
        height += !win_bborder(wp);
    }
    return height;
}


/*
 *  win_width ---
 *      Return the width of the window including any left margin.
 */
int
win_width(const WINDOW_t *wp)
{
    return (wp->w_w + !win_lborder(wp) + !win_rborder(wp));
}


/*
 *  win_uwidth ---
 *      Return the width of the window available for user/content display.
 */
int
win_uwidth(const WINDOW_t *wp)
{
    return (win_width(wp) - wp->w_disp_lmargin);
}


/*
 *  win_tedge ---
 *      Return the top edge of the specified window.
 */
int
win_tedge(const WINDOW_t *wp)
{
    return (wp->w_y);
}


/*
 *  win_bedge ---
 *      Return the bottom edge of the specified window.
 *
 *  Note, the returned value is *not* clipped to the screen, as such can
 *      return a column which is off the physical screen.
 */
int
win_bedge(const WINDOW_t *wp)
{
    return (wp->w_y + wp->w_h + win_bborder(wp));
}


/*
 *  win_ledge ---
 *      Return the left edge of the specified window.
 */
int
win_ledge(const WINDOW_t *wp)
{
    return (wp->w_x);
}


/*
 *  win_redge ---
 *      Return the right edge of the specified window.
 *
 *  Note, the returned value is *not* clipped to the current display,
 *      as such can return a column which is off the physical screen.
 */
int
win_redge(const WINDOW_t *wp)
{
    return (wp->w_x + wp->w_w + !win_rborder(wp)) + 1;
}


int
win_rclipped(const WINDOW_t *wp, int redge)
{
    const int rclipped = ttcols() - RBORDERS(wp);
    if (redge > rclipped) {
        return rclipped;
    }
    return redge;
}


/*
 *  win_tline ---
 *      Return the position of users top window line/row.
 */
int
win_tline(const WINDOW_t *wp)
{
    return (wp->w_y + win_tborder(wp));
}


/*
 *  win_tline ---
 *      Return the position of users bottom window line/row.
 */
int
win_bline(const WINDOW_t *wp)
{
    return (win_tline(wp) + win_height(wp) - 1);
}


/*
 *  win_tline ---
 *      Return the left margin, being the columns within the window allocated
 *      to status and line-number display.
 */
int
win_lmargin(const WINDOW_t *wp)
{
    return (wp->w_disp_lmargin);
}


/*
 *  win_fcolumn ---
 *      Return the first position within window.
 */
int
win_fcolumn(const WINDOW_t *wp)
{
    return (wp->w_x + win_lborder(wp));
}


/*
 *  win_lcolumn ---
 *      Return the position of users first/left column.
 */
int
win_lcolumn(const WINDOW_t *wp)
{
    return win_fcolumn(wp) + win_lmargin(wp);
}


/*
 *  win_rcolumn ---
 *      Return the position of last/right window column's.
 *
 *  Note, the returned value is *not* clipped to the current display,
 *      as such can return a column which is off the physical screen.
 */
int
win_rcolumn(const WINDOW_t *wp)
{
    return (win_fcolumn(wp) + win_width(wp) - 1);
}

/*end*/
