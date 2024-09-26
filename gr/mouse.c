#include <edidentifier.h>
__CIDENT_RCSID(gr_mouse_c,"$Id: mouse.c,v 1.49 2024/09/15 14:29:51 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mouse.c,v 1.49 2024/09/15 14:29:51 cvsuser Exp $
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
#include <libstr.h>

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
#include "system.h"
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
 *      Initialise the mouse interface using the given device.
 *      Called only if mouse functionality has been enabled.
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

    m.multi = -1;
    if (sys_mousepoll(fds, &m)) {
        mouse_process(&m, NULL);
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
        argv_assign_int(5, (accint_t)(wp->w_left_offset + 1 + x));

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
        The 'process_mouse()' primitive allows an external mouse event to be processed;
        some mice types are handled internally whereas others require macro support.

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
    struct MouseEvent m = {0};

    m.b1 = get_xinteger(1, 0);
    m.b2 = get_xinteger(2, 0);
    m.b3 = get_xinteger(3, 0);
    m.x  = get_xinteger(4, 0);
    m.y  = get_xinteger(5, 0);
    mouse_process(&m, NULL);
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
        [Value  [Definition         [Definition             ]
      ! 0       MOBJ_NOWHERE        Not in any window.

      ! 1       MOBJ_LEFT_EDGE      Left bar of window.
      ! 2       MOBJ_RIGHT_EDGE     Right bar of window.
      ! 3       MOBJ_TOP_EDGE       Top line of window.
      ! 4       MOBJ_BOTTOM_EDGE    Bottom line of window.
      ! 5       MOBJ_INSIDE         Mouse inside window.
      ! 6       MOBJ_TITLE          On title.

      ! 20      MOBJ_VSCROLL        Vertical scroll area.
      ! 21      MOBJ_VTHUMB         Vertical scroll area.
      ! 22      MOBJ_HSCROLL        Horizontal scroll area.
      ! 23      MOBJ_HTHUMB         Horizontal scroll area.

      ! 30      MOBJ_ZOOM           Zoom button.
      ! 31      MOBJ_CLOSE          Close.
      ! 32      MOBJ_SYSMENU        System Menu.
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
        Returns the time since the previous mouse event.

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
        x += wp->w_left_offset + 1;

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
 *      Determine if a mouse status change has occured and generate a key action to match.
 */

struct MouseState {
    mouseevt_t evt;
    unsigned mods;
    WINDOW_t* wp;
};

static uint64_t mouse_time(void);
static unsigned mouse_mods(const struct MouseEvent* me);
static int mouse_press(const struct MouseEvent* me, const struct MouseState* ms);
static int mouse_press_release(const struct MouseEvent* me, const struct MouseState* ms);
static int mouse_ismulti(int click, int x, int y);
static int mouse_motion(const struct MouseEvent *me, const struct MouseState *ms);
static int mouse_release(const struct MouseEvent *me, const struct MouseState *ms, int b1, int b2, int b3);

static struct MouseEvent prev;

int
mouse_process(const struct MouseEvent *me, const char *seq)
{
    struct MouseState ms = {0};
    int msgs = 0;

#if (0)
    char buffer[64];
    sxprintf(buffer, sizeof(buffer), "%s:%u%u%u,%x,%x", seq, me->b1, me->b2, me->b3, me->type, me->ctrl);
    seq = buffer;
#endif

    ms.evt.seq = seq;
    ms.evt.x = me->x;
    ms.evt.y = me->y;
    ms.mods = mouse_mods(me);
    ms.wp = mouse_pos(me->x, me->y, &ms.evt.win, &ms.evt.where);

    if (me->type == MOUSEEVENT_TPRESSRELEASE) {
        /*
         *  Absolute mouse reporting semantics for example win32,
         *  explicit button status events.
         */
        if ((msgs = mouse_press_release(me, &ms)) != 0) {
            return msgs;
        }

    } else if (me->type & (MOUSEEVENT_TPRESS | MOUSEEVENT_TRELEASE)) {
        /*
         *  Advanced mouse reporting semantics,
         *  individual explicit press and release events
         */
        if (me->type & MOUSEEVENT_TRELEASE) {
            return mouse_release(me, &ms, me->b1, me->b2, me->b3);
        }
        msgs = mouse_press(me, &ms);

    } else {
        /*
         *  Simple mouse reporting semantics for example xterm legacy,
         *  individual press event and with a group release event.
         */
        if (me->type & MOUSEEVENT_TRELEASE_ALL) {
            msgs =  mouse_motion(me, &ms);
            msgs += mouse_release(me, &ms, prev.b1, prev.b2, prev.b3);
            return msgs;
        }
        msgs = mouse_press(me, &ms);
    }

    if (0 == msgs) {
        msgs = mouse_motion(me, &ms);
    }

    return msgs;
}


static uint64_t
mouse_time(void)
{
    int ms = 0;
    time_t now = sys_time(&ms);
    return ((uint64_t)now * 1000) + ms;
}


static unsigned
mouse_mods(const struct MouseEvent* me)
{
    unsigned mods = 0;

    if (me->ctrl & MOUSEEVENT_CSHIFT)
        mods |= MOD_SHIFT;
    if (me->ctrl & MOUSEEVENT_CCTRL)
        mods |= MOD_CTRL;
    if (me->ctrl & MOUSEEVENT_CMETA)
        mods |= MOD_META;
    return mods;
}


/*
 *  mouse_press ---
 *      Mouse press logic, common to mouse handlers; unless multi, emumulate double-click
 */
static int
mouse_press(const struct MouseEvent* me, const struct MouseState* ms)
{
    const int x = me->x, y = me->y;
    int b1 = me->b1, b2 = me->b2, b3 = me->b3, click = 0;
    int msgs = 0;

    /*
     *  determine click type.
     */
    if (b1 && 0 == prev.b1) {
        click = 1;
    }  else if (b2 && 0 == prev.b2) {
        click = 2;
    } else if (b3 && 0 == prev.b3) {
        click = 3;
    }

    if (click) {
        int multi = me->multi;

        if (-1 == multi) { /* derive */
            multi = mouse_ismulti(click, x, y);
        }
        if (multi) {
            if (click == 1) {
                ++b1;
            } else if (click == 2) {
                ++b2;
            } else if (click == 3) {
                ++b3;
            }
        }
    }

    /*
     *  publish
     */
    if (!click) {
        return 0;
    }

    trace_log("mouse_press(%d, %d, %d-%d-%d, 0x%x)\n", x, y, b1, b2, b3, ms->mods);
    if (ms->wp) {
        triggerx(REG_MOUSE, "%d %d %d %d %d %d %d %d %d", ms->evt.win, ms->evt.where, x, y, click, b1, b2, b3, ms->mods);
    }

#define BUTTON_PRESS(__s, __b) \
    ((1 == __s ? (BUTTON1_DOWN + __b) : (BUTTON1_DOUBLE + __b)) | ms->mods)

#define BUTTON_MOTION(__b) \
    ((BUTTON1_MOTION + __b) | ms->mods)

    // XXX: should mods be sticky, for example Button1-Ctl- is retained across motion and release?
    if (click) {
        if (b1 && b1 != prev.b1) {
            if (ms->wp) {
                key_cache_mouse(x_push_ref, BUTTON_PRESS(b1, 0), FALSE, &ms->evt);
                ++msgs;
            }
            prev.b1 = b1;
        }

        if (b2 && b2 != prev.b2) {
            if (ms->wp) {
                key_cache_mouse(x_push_ref, BUTTON_PRESS(b2, 1), FALSE, &ms->evt);
                ++msgs;
            }
            prev.b2 = b2;
        }

        if (b3 && b3 != prev.b3) {
            if (ms->wp) {
                key_cache_mouse(x_push_ref, BUTTON_PRESS(b3, 2), FALSE, &ms->evt);
                ++msgs;
            }
            prev.b3 = b3;
        }

        prev.x = x;
        prev.y = y;
    }
    return msgs;
}


/*
 *  mouse_press_release ---
 *      Mouse specific release logic, for example win32-console, system double-click reporting assumed.
 */
static int
mouse_press_release(const struct MouseEvent* me, const struct MouseState* ms)
{
    const int x = me->x, y = me->y,
        b1 = me->b1, b2 = me->b2, b3 = me->b3;
    const int multi = (me->multi > 0 ? 1 : 0);
    int msgs = 0;

#define BUTTON_PRESSRELEASE(__s, __b) \
    ((__s ? (multi ? (BUTTON1_DOUBLE + __b) : (BUTTON1_DOWN + __b)) : (BUTTON1_UP + __b)) | ms->mods)

    if (b1 == prev.b1 && b2 == prev.b2 && b3 == prev.b3) {
        return 0;
    }

    trace_log("mouse_press_release(%d, %d, %d-%d-%d, 0x%x)\n", x, y, b1, b2, b3, ms->mods);
    if (ms->wp) {
        triggerx(REG_MOUSE, "%d %d %d %d %d %d %d %d %d", ms->evt.win, ms->evt.where, x, y, 0, b1, b2, b3, ms->mods);
    }

    if (b1 != prev.b1) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_PRESSRELEASE(b1, 0), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b1 = b1;
    }

    if (b2 != prev.b2) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_PRESSRELEASE(b2, 1), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b2 = b2;
    }

    if (b3 != prev.b3) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_PRESSRELEASE(b3, 2), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b3 = b3;
    }

    return msgs;
}


static int
mouse_ismulti(int click, int x, int y)
{
    static int click_last, click_ypos, click_xpos;
    static uint64_t click_time;
    uint64_t now = mouse_time();

    if (click == click_last && x == click_xpos && y == click_ypos &&
                now < (click_time + sys_doubleclickms())) {
        click_last = 0;
        return 1;                               /* double-click */
    }
    click_last = click;
    click_xpos = x;
    click_ypos = y;
    click_time = now;
    return 0;
}


/*
 *  mouse_motion ---
 *      Mouse motion logic, common to mouse handlers.
 */
static int
mouse_motion(const struct MouseEvent* me, const struct MouseState* ms)
{
    const int x = me->x, y = me->y,
        motion = (x != prev.x) || (y != prev.y) || (me->type & MOUSEEVENT_TMOTION);
    const int b1 = me->b1, b2 = me->b2, b3 = me->b3;
    int msgs = 0;

    if (!motion) {
        return 0;
    }

    trace_log("mouse_motion(%d, %d, %d-%d-%d, 0x%x)\n", x, y, b1, b2, b3, ms->mods);
    if (ms->wp) {
        triggerx(REG_MOUSE, "%d %d %d %d %d %d %d %d %d", ms->evt.win, ms->evt.where, x, y, -1, b1, b2, b3, ms->mods);
    }

    if (ms->wp) {
        if (prev.b1) {
            key_cache_mouse(x_push_ref, BUTTON_MOTION(0), FALSE, &ms->evt);
            ++msgs;
        }

        if (prev.b2) {
            key_cache_mouse(x_push_ref, BUTTON_MOTION(1), FALSE, &ms->evt);
            ++msgs;
        }

        if (prev.b3) {
            key_cache_mouse(x_push_ref, BUTTON_MOTION(2), FALSE, &ms->evt);
            ++msgs;
        }
    }

    prev.x = x;
    prev.y = y;
    return msgs;
}


/*
 *  mouse_release ---
 *      Mouse specific release logic, for example xterm-sgr.
 */
static int
mouse_release(const struct MouseEvent *me, const struct MouseState* ms, int b1, int b2, int b3)
{
    const int x = me->x, y = me->y;
    int msgs = 0;

    if (!((b1 && prev.b1) ||  (b2 && prev.b2) || (b3 || prev.b3))) {
        return 0;
    }

    /*
     *  publish
     */
    trace_log("mouse_release(%d, %d, %d-%d-%d)\n", x, y, b1, b2, b3);
    if (ms->wp) {
        triggerx(REG_MOUSE, "%d %d %d %d %d %d %d %d %d", ms->evt.win, ms->evt.where, x, y, 0, b1, b2, b3, 0);
    }

#define BUTTON_RELEASE(__b) \
    (BUTTON1_UP + __b)

    if (b1 && prev.b1) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_RELEASE(0), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b1 = 0;
    }

    if (b2 && prev.b2) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_RELEASE(1), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b2 = 0;

    }

    if (b3 && prev.b3) {
        if (ms->wp) {
            key_cache_mouse(x_push_ref, BUTTON_RELEASE(2), FALSE, &ms->evt);
            ++msgs;
        }
        prev.b3 = 0;
    }

    if (prev.b1 | prev.b2 | prev.b3) {
        prev.x = x;
        prev.y = y;
    } else {
        prev.x = prev.y = 0;
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
 *
 *  Returns:
 *      Returns window pointer to window where mouse is located, otherwise NULL.
 *      If successful, set win to the window id, and where identifies the type of area the mouse is on.
 */
WINDOW_t *
mouse_pos(int x, int y, int *win, int *where)
{
    register WINDOW_t *wp, *topwp = NULL;
    int t_where = 0;

    /*
     *  Walk window chain,
     *      all windows within the current stack must be checked as the
     *      current (top) popup shall match last.
     */
    if (x) --x;
    if (y) --y;                                  /* mouse 1,1 via window 0,0 origins */

    for (wp = window_first(); wp; wp = window_next(wp))
        if (x >= (win_lcolumn(wp) - win_lborder(wp)) && x <= (win_rcolumn(wp) + win_rborder(wp)) &&
                y >= (win_tline(wp) - win_tborder(wp)) && y <= (win_bline(wp) + win_bborder(wp)) ) {
            topwp = wp;                         /* current match */
        }

    /*
     *  Process matched window
     */
    if (topwp) {
        const int wx = x - topwp->w_x; //, wy = y - topwp->w_y;
        int b;

        if ((b = win_lborder(topwp)) && x == win_lcolumn(topwp) - b) {
            t_where = MOBJ_LEFT_EDGE;

    //  } else if (win_lmargin(topwp)) {
    //      t_where = MOBJ_LEFT_MARGIN;

        } else if ((b = win_rborder(topwp)) && x == win_rcolumn(topwp) + b) {
            // MOBJ_VSCROLL_AREA;
            t_where = MOBJ_RIGHT_EDGE;

        } else if ((b = win_tborder(topwp)) && y == win_tline(topwp) - b) {
            if (wx <= topwp->w_coords.title_end && wx > topwp->w_coords.title_start) {
                t_where = MOBJ_TITLE;
         // } else if (wx <= topwp->w_coords.zoom_end && wx > topwp->w_coords.zoom_start) {
         //     t_where = MOBJ_ZOOM;
            } else if (wx <= topwp->w_coords.close_end && wx > topwp->w_coords.close_start) {
                t_where = MOBJ_CLOSE;
            } else {
                t_where = MOBJ_TOP_EDGE;
            }

        } else if ((b = win_bborder(topwp)) && y == win_bline(topwp) + b) {
            // MOBJ_HSCROLL_AREA;
            t_where = MOBJ_BOTTOM_EDGE;

        } else {
            t_where = MOBJ_INSIDE; 
        }
    }

    if (win)
        *win = (topwp ? topwp->w_num : -1 );

    if (where)
        *where = t_where;

    return topwp;
}

/*end*/
