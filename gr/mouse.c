#include <edidentifier.h>
__CIDENT_RCSID(gr_mouse_c,"$Id: mouse.c,v 1.41 2023/09/10 16:35:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mouse.c,v 1.41 2023/09/10 16:35:52 cvsuser Exp $
 * Mouse support code.
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
#include <edtermio.h>
#include <edalt.h>

#include "accum.h"
#include "anchor.h"
#include "border.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "keyboard.h"
#include "main.h"
#include "register.h"                           /* trigger() */
#include "symbol.h"
#include "tty.h"
#include "window.h"

#if defined(__OS__) || defined(__MSDOS__)
#ifndef HAVE_MOUSE
#define HAVE_MOUSE
#endif

#else
#ifdef  HAVE_MOUSE
#define NEED_MOUSE_HELPERS
#endif
#endif

#include "mouse.h"

static int              x_mouse_active = 0;

static int              x_mouse_event;
static int              x_mouse_xpos;
static int              x_mouse_ypos;
static int              x_mouse_win;
static int              x_mouse_where;
static time_t           x_mouse_time;
static time_t           x_mouse_prev;


/*
 *  mouse_init ---
 *      Function to try and initialise mouse device. Called only
 *      if -mouse flag set. Called with name of device for
 *      accessing.
 */
int
mouse_init(const char *dev)
{
    __CUNUSED(dev)
#if defined(HAVE_MOUSE)
    x_mouse_active = sys_mouseinit(dev);
#endif
    return x_mouse_active;
}


void
mouse_close(void)
{
#if defined(HAVE_MOUSE)
    if (x_mouse_active) {
        sys_mouseclose();
    }
#endif
    x_mouse_active = 0;
}


int
mouse_active(void)
{
    return x_mouse_active;
}


void
mouse_pointer(int state)
{
    __CUNUSED(state)
#if defined(HAVE_MOUSE)
    sys_mousepointer(state);
#endif
}


#if defined(NEED_MOUSE_HELPERS)
int
mouse_poll(fd_set *fds)
{
    struct MouseEvent m = {0};

    if (sys_mousepoll(fds, &m)) {
        mouse_process(m.x, m.y, m.b1, m.b2, m.b3, m.multi);
    }
    return (0);
}
#endif


/*  Function:           do_translate_pos
 *      translate_pos primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: translate_pos - Convert window coordinates.

        int
        translate_pos(int x, int y,
            [int &winnum], [int &line], [int &col])

    Macro Description:
        The 'translate_pos()' primitive translates the physical
        screen position (x, y) into a window and logical line/col
        position.

    Macro Parameters:
        x, y - Screen coordinates to be translate.
        winnum - Optional an integer variable which shall be
            populated with the window identifier within which
            the (x, y) position falls, otherwise -1 if no window was
            mapped.
        line, col - Optional integer variables which shall be
            populated with the translated window coordinates,
            otherwise -1 if no window was mapped.

    Macro Returns:
        The 'translate_pos()' primitive returns where the mouse
        cursor is located.

(start table)
        [Value              [Definition                     ]
      ! MOBJ_NOWHERE        Not in any window.

      ! MOBJ_LEFT_EDGE      Left bar of window.
      ! MOBJ_RIGHT_EDGE     Right bar of window.
      ! MOBJ_TOP_EDGE       Top line of window.
      ! MOBJ_BOTTOM_EDGE    Bottom line of window.
      ! MOBJ_INSIDE         Mouse inside window.
      ! MOBJ_TITLE          On title.

      ! MOBJ_VSCROLL        Vertical scroll area.
      ! MOBJ_VTHUMB         Vertical scroll area.
      ! MOBJ_HSCROLL        Horz scroll area.
      ! MOBJ_HTHUMB         Horz scroll area.

      ! MOBJ_ZOOM           Zoom button,
      ! MOBJ_CLOSE          Close.
      ! MOBJ_SYSMENU        System Menu.
(end table)

    Macro Portability:
        n/a

    Macro See Also:
        get_mouse_pos, process_mouse
 */
void
do_translate_pos(void)              /* int (int x, int y, int &wid, int &line, int &col) */
{
    int x, y, where, wid;
    WINDOW_t *wp;

    x = get_xinteger(1, 0);
    y = get_xinteger(2, 0);

    wp = mouse_pos(x, y, &wid, &where);
    argv_assign_int(3, (accint_t) wid);
    if (wp) {
        x -= (wp->w_x - !win_lborder(wp)) - 1;
        y -= (wp->w_y - !win_tborder(wp)) - 1;

        argv_assign_int(4, (accint_t)(wp->w_top_line + y));
        argv_assign_int(5, (accint_t)(wp->w_left_offset + x));

    } else {
        argv_assign_int(4, (accint_t) -1);
        argv_assign_int(5, (accint_t) -1);
    }
    acc_assign_int(where);
}


/*  Function:           do_process_mouse
 *      process_mouse primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: process_mouse - Process mouse event.

        void
        process_mouse(
            [int b1], [int b2], [int b3], int x, int y)

    Macro Description:
        The 'process_mouse()' primitive allows an external mouse
        event to be processed; some mice types are handled internally
        whereas others require macro support.

    Macro Parameters:
        b1, b2, b3 - Button states, zero for up otherwise non-zero
            for down.
        x, y - Screen coordinates.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        translate_pos, process_mouse
 */
void
do_process_mouse(void)              /* void (int b1, int b2, int b3, int x, int y) */
{
    const int b1 = get_xinteger(1, 0);
    const int b2 = get_xinteger(2, 0);
    const int b3 = get_xinteger(3, 0);
    const int x  = get_xinteger(4, 0);
    const int y  = get_xinteger(5, 0);

    mouse_process(x, y, b1, b2, b3, 0);
}


/*  Function:           do_get_mouse_pos
 *      get_mouse_pos primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *
 *<<GRIEF>>
    Macro: get_mouse_pos - Retrieve last mouse action.

        void
        get_mouse_pos(
            [int &x], [int &y], [int &winnum],
            [int &line], [int &col],
                [int &where], [int &region], [int &event])

    Macro Description:
        The 'get_mouse_pos()' primitive retrieves the details of the
        last mouse action.

        Note!:
        The details returned are only valid immediately after the
        macro assigned to a button press event. Any subsequent calls
        to <read_char> or <process> will overwrite the internal values.

    Macro Parameters:
        x, y - Screen coordinates.
        winnum - Optional an integer variable which shall be
            populated with the window identifier within which
            the (x, y) position falls, otherwise -1 if no window was
            mapped.
        line, col - Optional integer variables which shall be
            populated with the translated window coordinates,
            otherwise -1 if no window was mapped.
        where - Where the cursor is located within the window.
        region - Cursor position relative to any marked regions.
        event - Associated mouse event.

    Where::

(start table)
        [Value              [Definition                     ]
      ! MOBJ_NOWHERE        Not in any window.

      ! MOBJ_LEFT_EDGE      Left bar of window.
      ! MOBJ_RIGHT_EDGE     Right bar of window.
      ! MOBJ_TOP_EDGE       Top line of window.
      ! MOBJ_BOTTOM_EDGE    Bottom line of window.
      ! MOBJ_INSIDE         Mouse inside window.
      ! MOBJ_TITLE          On title.

      ! MOBJ_VSCROLL        Vertical scroll area.
      ! MOBJ_VTHUMB         Vertical scroll area.
      ! MOBJ_HSCROLL        Horizontal scroll area.
      ! MOBJ_HTHUMB         Horizontal scroll area.

      ! MOBJ_ZOOM           Zoom button,
      ! MOBJ_CLOSE          Close.
      ! MOBJ_SYSMENU        System Menu.
(end table)

    Region::

(start table)
        [Value  [Description                                ]
      ! 0       No region selected.
      ! 1       Cursor is before the region.
      ! 2       Cursor is inside the region.
      ! 3       Cursor is after the region.
      ! 4       Cursor is to the left of a column region.
      ! 5       Cursor is to the right of a column region.
(end table)

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        process_mouse, translate_pos
 */
void
do_get_mouse_pos(void)          /* int ([int x], [int y], [int win]. [int line], [int col],
                                          [int where], [int region], [int event]) */
{
    WINDOW_t *wp = NULL;
    int region = 0;

    argv_assign_int(1, (accint_t) x_mouse_xpos);
    argv_assign_int(2, (accint_t) x_mouse_ypos);
    argv_assign_int(3, (accint_t) x_mouse_win);

    if (x_mouse_win >= 0 && x_mouse_where == MOBJ_INSIDE &&
            NULL != (wp = window_lookup(x_mouse_win))) {
        /*
         *  Active window
         */
        int y = x_mouse_ypos, x = x_mouse_xpos;
        ANCHOR_t a;

        x -= wp->w_x + win_lborder(wp) + 1;     /* normalise to window */
        y -= wp->w_y + win_tborder(wp) + 1;
        y += wp->w_top_line;
        x += wp->w_left_offset;

        argv_assign_int(4, (accint_t) y);       /* line */
        argv_assign_int(5, (accint_t) x);       /* column */

        /*
         *  Determine cursor position within window
         *
         *      0   no region selected.
         *      1   cursor is before the region.
         *      2   cursor is inside the region.
         *      3   cursor is after the region.
         *      4   cursor is to the left of a column region.
         *      5   cursor is to the right of a column region.
         */
        if (anchor_get(wp, NULL, &a)) {
            if (y < a.start_line) {
                region = 1;                     /* before the region. */

            } else if (y > a.end_line) {
                region = 3;                     /* after the region. */

            } else {
                region = 2;                     /* inside. */

                switch (a.type) {
                case MK_NORMAL:
                    if (y == a.start_line && x < a.start_col) {
                        region = 4;             /* left of a column region. */
                    } else if (y == a.end_line && a.end_col) {
                        region = 5;             /* right of a column region. */
                    }
                    break;

                case MK_COLUMN:
                    if (x < a.start_col) {
                        region = 4;             /* left of a column region. */
                    } else if (x > a.end_col) {
                        region = 5;             /* right of a column region. */
                    }
                    break;

                case MK_NONINC:
                    if (y == a.start_line && x < a.start_col) {
                        region = 4;             /* left of a column region. */

                    } else if (y == a.end_line && x >= a.end_col) {
                        region = 5;             /* right of a column region. */
                    }
                    break;
                }
            }
        }
    } else {
        /*
         *  No active window
         */
        argv_assign_int(4, (accint_t) -1);
        argv_assign_int(5, (accint_t) -1);
    }

    argv_assign_int(6, (accint_t) x_mouse_where);
    argv_assign_int(7, (accint_t) region);
    argv_assign_int(8, (accint_t) x_mouse_event);

    acc_assign_int(x_mouse_prev ? (accint_t)(x_mouse_time - x_mouse_prev) : 0);
}


/*
 *  mouse_process ---
 *      Determine if a mouse status change has occured
 *      and generate a key action to match.
 */
int
mouse_process(int x, int y, int b1, int b2, int b3, int multi)
{
    static int click_last, click_ypos, click_xpos;
    static struct MouseEvent oldm;
    static clock_t click_time;

    WINDOW_t *wp;
    int click, win, where;
    int msgs = 0;

    /*
     *  Find out if this is a multi-click
     */
    click = 0;
    if (b1 && !oldm.b1) {
        click = 1;
    } else if (b2 && !oldm.b2) {
        click = 2;
    } else if (b3 && !oldm.b3) {
        click = 3;
    }

    if (click) {
        if (-1 == multi) {
            const clock_t now = clock();

            if (click == click_last && x == click_xpos && y == click_ypos &&
                        now < click_time + (CLOCKS_PER_SEC/3)) {
                multi = 1;                      /* ~330ms */
            } else {
                click_last = click;
                click_xpos = x;
                click_ypos = y;
                click_time = now;
                multi = 0;
            }
        }

        if (multi) {
            if (click == 1) b1++;
            else if (click == 2) b2++;
            else if (click == 3) b3++;
            click_last = 0;
        }
    }

    trace_log("mouse_process(%d, %d, %d-%d-%d-%d)\n", x, y, b1, b2, b3, multi);

    /*
     *  State change, send message
     */
    if ((b1 != oldm.b1) || (b2 != oldm.b2) || (b3 != oldm.b3) || (x != oldm.x) || (y != oldm.y)) {
        if ((wp = mouse_pos(x, y, &win, &where)) != NULL) {
            int motion = ((x != oldm.x) || (y != oldm.y));

#define BUTTON_ACTION(s, b) \
                (s ? (s > 1 ? (BUTTON1_DOUBLE + b) : (BUTTON1_DOWN + b)) : BUTTON1_UP + b)

            /*
             *  Old-style mouse trigger
             */
            triggerx(REG_MOUSE, "%d %d %d %d %d %d %d", win, where, x, y, b1, b2, b3);

            /*
             *  report button changes
             */
            if (b1 != oldm.b1) key_cache_mouse(x_push_ref, BUTTON_ACTION(b1, 0), FALSE, x, y, win, where), ++msgs;
            if (b2 != oldm.b2) key_cache_mouse(x_push_ref, BUTTON_ACTION(b2, 1), FALSE, x, y, win, where), ++msgs;
            if (b3 != oldm.b3) key_cache_mouse(x_push_ref, BUTTON_ACTION(b3, 2), FALSE, x, y, win, where), ++msgs;

            /*
             *  and secondary motion
             */
            if (motion) {
                if (b1) key_cache_mouse(x_push_ref, BUTTON1_MOTION, FALSE, x, y, win, where), ++msgs;
                if (b2) key_cache_mouse(x_push_ref, BUTTON2_MOTION, FALSE, x, y, win, where), ++msgs;
                if (b3) key_cache_mouse(x_push_ref, BUTTON3_MOTION, FALSE, x, y, win, where), ++msgs;
            }
        }

        oldm.x = x; oldm.y = y;
        oldm.b1 = b1; oldm.b2 = b2; oldm.b3 = b3;
    }
    return msgs;
}


/*
 *  mouse_execute ---
 *      Register the execution of a mouse event.
 */
void
mouse_execute(const struct IOEvent *evt)
{
    const struct IOMouse *mouse = &evt->mouse;
    int code = evt->code;

    assert(EVT_MOUSE == evt->type);
    assert(RANGE_BUTTON == (RANGE_MASK & code));
    x_mouse_event = code;
    x_mouse_xpos  = mouse->x;
    x_mouse_ypos  = mouse->y;
    x_mouse_win   = mouse->win;
    x_mouse_where = mouse->where;
    x_mouse_prev  = x_mouse_time;
    x_mouse_time  = mouse->when;
}


/*
 *  mouse_pos ---
 *      Determine on what type of object the mouse is sitting on.
 *      Returns window pointer to window where mouse is. Set win_id to the
 *      window id, and where identifies the type of area the mouse is on.
 */
WINDOW_t *
mouse_pos(int x, int y, int * win, int * where)
{
    register WINDOW_t *wp, *topwp;
    int b;

    --x, --y;                                   /* mouse 1,1 via window 0,0 origins */

    /*
     *  Walk window chain,
     *      all windows within the current stack must be checked as the
     *      current (top) popup shall match last.
     */
    topwp = NULL;
    for (wp = window_first(); wp; wp = window_next(wp))
        if (x >= (win_lcolumn(wp) - win_lborder(wp)) && x <= (win_rcolumn(wp) + win_rborder(wp)) &&
                y >= (win_tline(wp) - win_tborder(wp)) && y <= (win_bline(wp) + win_bborder(wp)) ) {
            topwp = wp;                         /* current match */
        }

    /*
     *  Process matched window
     */
    *where = 0;
    if (NULL == topwp) {
        *win = -1;                              /* no active window */

    } else  {
        *win = topwp->w_num;

        if ((b = win_lborder(topwp)) && x == win_lcolumn(topwp) - b) {
            *where = MOBJ_LEFT_EDGE;

//          *where = MOBJ_ZOOM;
//          *where = MOBJ_CLOSE;

//      } else if (win_lmargin(topwp)) {        /* XXX/TOD0 */
//          *where = MOBJ_LEFT_MARGIN;

        } else if ((b = win_rborder(topwp)) && x == win_rcolumn(topwp) + b) {
//          if (y >= win_tline(topwp) + 1 && y <= win_bline(topwp) - 1) {
//              *where = MOBJ_VSCROLL_AREA;
//          } else {
                *where = MOBJ_RIGHT_EDGE;
//          }

        } else if ((b = win_tborder(topwp)) && y == win_tline(topwp) - b) {
//          if (vscreen[y][x] & 0x80) {
                *where = MOBJ_TOP_EDGE;
//          } else {
//              *where = MOBJ_TITLE;
//          }

        } else if ((b = win_bborder(topwp)) && y == win_bline(topwp) + b) {
//          if (x >= win_lcolumn(topwp)+1 && x <= win_rcolumn(topwp)-1) {
//              *where = MOBJ_HSCROLL_AREA;
//          } else {
                *where = MOBJ_BOTTOM_EDGE;
//          }

        } else {
            *where = MOBJ_INSIDE;               /* must be inside */
        }
    }
    return topwp;
}

