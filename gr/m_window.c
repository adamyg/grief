#include <edidentifier.h>
__CIDENT_RCSID(gr_m_window_c,"$Id: m_window.c,v 1.29 2024/12/06 15:46:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_window.c,v 1.29 2024/12/06 15:46:06 cvsuser Exp $
 * Window primitives.
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
#include "../libchartable/libchartable.h"
#include "../libwidechar/widechar.h"
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_window.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "border.h"
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"                            /* execute_str() */
#include "color.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"                               /* get_...() */
#include "m_pty.h"
#include "main.h"
#include "symbol.h"                             /* argv_...() */
#include "tty.h"
#include "undo.h"
#include "window.h"

static int              window_build(int flag, const char *title);

static int              flag_decode(const char *who, int mode, const char *spec, uint32_t *values);
static const struct winflag *flag_lookup(const char *name, int length);

static const struct winflag {
    const char *        f_name;                 /* name/label */
    int                 f_length;
    uint32_t            f_value;                /* flag value */

} winflagnames[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
    { NFIELD("hidden"),             WF_HIDDEN        },     /* Hide the window from view, used to hide nested popups/boss mode etc */
    { NFIELD("no_shadow"),          WF_NO_SHADOW     },     /* Turnoff the popups shadow */
    { NFIELD("no_border"),          WF_NO_BORDER     },     /* Turnoff borders, regardless of the borders() setting */
    { NFIELD("system"),             WF_SYSTEM        },     /* Window is a system window (eg menu) */

    { NFIELD("showanchor"),         WF_SHOWANCHOR    },     /* Show anchor regardless of selection status */
    { NFIELD("selected"),           WF_SELECTED      },     /* Hilite the title regardless of selection status */
    { NFIELD("lazyupdate"),         WF_LAZYUPDATE    },     /* Delay any updates until next refresh() */

    { NFIELD("line_numbers"),       WF_LINE_NUMBERS  },     /* Line numbers */
    { NFIELD("line_status"),        WF_LINE_STATUS   },     /* Line status */
    { NFIELD("eof_display"),        WF_EOF_DISPLAY   },     /* Show <EOF> marker */
    { NFIELD("tilde_display"),      WF_TILDE_DISPLAY },     /* Show <~> marker as EOF marker */

    { NFIELD("himodified"),         WF_HIMODIFIED    },     /* Hilite modified lines */
    { NFIELD("hiadditional"),       WF_HIADDITIONAL  },     /* Hilite additional lines */
    { NFIELD("hichanges"),          WF_HICHANGES     },     /* Hilite inline changes */

    { NFIELD("eol_hilite"),         WF_EOL_HILITE    },     /* Limit hilites to EOL */
    { NFIELD("eol_cursor"),         WF_EOL_CURSOR    }      /* Limit cursor to EOL */
#undef  NFIELD
    };


/*  Function:           do_top_of_window
 *      top_of_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: top_of_window - Goto top of the current window.

        int
        top_of_window()

    Macro Description:
        The 'top_of_window()' primitive moves the buffer cursor to the
        first line of the current window.

    Macro Parameters:
        none

    Macro Returns:
        Returns non-zero if the current cursor position moved, otherwise
        zero if already positioned at the top of the window.

    Macro Portability:
        n/a

    Macro See Also:
        end_of_buffer, inq_position, move_abs, move_rel
 */
void
do_top_of_window(void)          /* void () */
{
    u_dot();
    win_modify(WFMOVE);
    if ((curwp->w_line = curwp->w_top_line) < 1) {
        curwp->w_line = 1;
    }
    curwp->w_col = 1;
    win_modify(WFMOVE);
}


/*  Function:           do_end_of_window
 *      end_of_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: end_of_window - Goto end of the current window.

        int
        end_of_window()

    Macro Description:
        The 'end_of_window()' primitive moves the buffer cursor to the
        last line of the current window.

    Macro Parameters:
        none

    Macro Returns:
        Returns non-zero if the current cursor position moved, otherwise
        zero if already positioned at the end of the window.

    Macro Portability:
        n/a

    Macro See Also:
        top_of_window
 */
void
do_end_of_window(void)          /* void () */
{
    const LINENO line = curwp->w_line;

    u_dot();
    win_modify(WFMOVE);
    curwp->w_line =
        (curwp->w_top_line + win_height(curwp) - 1);
    acc_assign_int((accint_t) (line != curwp->w_line));
    win_modify(WFMOVE);
}


/*  Function:           do_color_index
 *      color_index primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: color_index - Border color background color index.

        int 
        color_index([int index])

    Macro Description:
        The 'color_index()' primitive sets the current value of the color
        index. The index controls the color that shall be assigned as
        the borderless background color to the next window created. On
        assignment the color index shall be automatically incremented
        and contained within the range '0 .. 16'.

        When borders are disabled, this color shall be used as the
        background of the associated window allowing one to distinguish
        between individual views.

    Macro Parameters:
        index - Optional integer index between the ranges of 0 and 16,
            if omitted the current index is returned without effecting
            any change effectively behaving like a 'inq_color_index'
            function.

    Macro Returns:
        Returns the previous value of the color index.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        borders, create_buffer
 */
void
do_color_index(void)            /* int ([int index]) */
{
    int oindex = x_window_nextattr;

    if (isa_integer(1)) {
        x_window_nextattr = get_xinteger(1, 0);
        if (x_window_nextattr < 0 || x_window_nextattr >= 16) {
            x_window_nextattr = 0;              /* out of bounds, reset */
        }
    }
    acc_assign_int((accint_t) oindex);
}


/*  Function:           inq_borders
 *      inq_borders primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_borders - Retrieve the border status.

        int 
        inq_borders()

    Macro Description:
        The 'inq_borders()' primitive retrieves the border status which
        controls whether or not tiled windows are displayed with borders.

        Disabling borders can improve display performance on slow
        systems, yet shall disable scroll bars, title bars and may make
        working with multiple windows difficult.

    Macro Parameters:
        none

    Macro Returns:
        Returns non-zero if windows borders are enabled, otherwise zero
        if disabled.

    Macro Portability:
        n/a

    Macro See Also:
        borders, color_index
 */
void
inq_borders(void)               /* int () */
{
    acc_assign_int((accint_t) xf_borders);
}


/*  Function:           do_borders
 *      borders primitive, set window border on or off.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: borders - Set window border status.

        int 
        borders([int borders])

    Macro Description:
        The 'borders()' primitive either sets or toggles the tiled window
        border status.

        Disabling borders can improve display performance on slow
        systems, yet shall disable scroll bars, title bars and may make
        working with multiple windows difficult.

    Macro Parameters:
        borders - An optional integer stated on (1) or off (0). If
            omitted, the current value is toggled. A value of -1, acts
            the same as inq_borders() only returning the current status
            without effecting any change.

    Macro Returns:
        An integer boolean value representing the previous border state.

    Macro Portability:
        n/a

    Macro See Also:
        inq_borders, _chg_properties

 *<<GRIEF>> [callback]
    Macro: _chg_properties - Property change event.

        void
        _chg_properties()

    Macro Description:
        The '_chg_properties' callback is executed by Grief upon the
        borders configuration changing.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        Callbacks, borders
 */
void
do_borders(void)                /* int ([int borders]) */
{
    const char *sval;
    int oflag = xf_borders;

    if (isa_undef(1)) {
        xf_borders = !xf_borders;               /* no parameter, toggle */

    } else if (get_xinteger(1, 0) != -1) {
        xf_borders = get_xinteger(1, 0) != 0;   /* set or clear */

    } else if (NULL != (sval = get_xstr(1))) {
        switch (*sval) {                        /* extension */
        case 'n': case 'N': xf_borders = 0; break;
        case 'y': case 'Y': xf_borders = 1; break;
        case 'x': case 'X':
        case 0:
            xf_borders = !xf_borders;
            break;
        }
    }

    if (xf_borders != oflag) {                  /* flag being changed */
        WINDOW_t *wp, *active = NULL;

        execute_str("_chg_properties");

        for (wp = window_first(); wp; wp = window_next(wp)) {
            if (W_TILED != wp->w_type) {
                wp->w_attr = 0;                 /* menu|popup */

            } else  {
                if (WFTOP & wp->w_status) {
                    active = wp;                /* top overrides */

                } else {
                    if (NULL == active) {
                        active = wp;            /* first 'active' window */
                    }

                    if (0 == wp->w_attr) {
                        window_attr(wp);        /* assign new background (if none) */
                    }
                }
            }
        }

        if (active) {
            active->w_attr = 0;                 /* first standard window */
        }

        vtgarbled();
    }
    acc_assign_int((accint_t) oflag);           /* previous value */
}


/*  Function:           do_create_title_window
 *      create_title_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: create_tiled_window - Creates a tiled window.

        int 
        create_tiled_window(int lx, int by, int rx, int ty, [int bufnum])

    Macro Description:
        The 'create_tiled_window()' primitive creates a tiled window. The
        stated coordinates (lx, by, rx and ty) represent the total
        window arena irrespective of the <borders> status, with (0, 0)
        being the top left hand corner of the screen.

        Care should be taken not to overlay tiled windows and that all
        of the visible display has been assigned to window.

        Generally tiled windows are created by the end user splitting
        the editor windows. This primitive in primary utilised during
        state restoration to recover the state of the previous edit
        session.

        The created window shall not be visible until the display has
        been enabled using <display_windows>.

    Macro Parameters:
        lx - Coordinate of the left edge.
        by - Coordinate of the bottom edge.
        rx - Coordinate of the right edge.
        ty - Coordinate of the top edge.
        bufnum - Optional buffer identifier to be attached to the newly
            create window, see <attach_buffer>.

    Macro Returns:
        Returns the unique identifier of the new window.

    Macro Portability:
        n/a

    Macro See Also:
        create_window, display_windows, inq_screen_size.
 */
void
do_create_tiled_window(void)    /* (int lx, int by, int rx, int ty, [int bufnum]) */
{
    window_build(W_TILED, "");                  /* common window creation */

    if (isa_integer(5)) {                       /* attach optional buffer */
        const int bufnum = get_xinteger(5, 0);
        BUFFER_t *bp;

        if (NULL == (bp = buf_lookup(bufnum))) {
            ewprintf("create_tiled_window: no such buffer (%d)", bufnum);
        } else {
            attach_buffer(curwp, bp);
        }
    }

    acc_assign_int((accint_t) curwp->w_num);
}


/*  Function:           do_create_window
 *      create_window primitive, creates a popup window.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: create_window - Create a popup window.

        int 
        create_window(int lx, int by,
                int rx, int ty, [string message])

    Macro Description:
        The 'create_window()' primitive creates a popup window resource
        which shall become the current window. The window should be
        suitability sized based on the current screen size, which can be
        determined using <inq_screen_size>.

        Popup windows stack upon any underlying tiled or other popup
        which are located in the same position, the order may be
        controlled using <set_window_priority>,

        On completion at buffer needs to be associated with the newly
        created window using <attach_buffer>.

    Macro Parameters:
        lx - Coordinate of the left edge.
        by - Coordinate of the bottom edge.
        rx - Coordinate of the right edge.
        ty - Coordinate of the top edge.
        message - Optional string containing the message which shall be
            displayed on the bottom frame.

    Macro Returns:
        Returns the unique identifier of the new window.

    Macro Portability:
        Unlike BRIEF the number of windows which may be active at any
        one time is only limit by system resources.

    Macro See Also:
        attach_buffer, create_edge, create_tiled_window, create_menu_window,
            delete_window, inq_screen_size
 */
void
do_create_window(void)          /* (int lx, int by, int rx, int ty, [string message]) */
{
    window_build(W_POPUP, get_str(5));          /* common window creation */
}


/*  Function:           window_build
 *      Common code for creating a tiled or popup window.
 *
 *  Parameters:
 *      type -              Window type.
 *      title -             Title message.
 *
 *  Returns:
 *      int - window identifier otherwise -1 on error.
 */
static int
window_build(int type, const char *title)
{
    const int lx = get_xinteger(1, 0);
    const int by = get_xinteger(2, 1);
    const int rx = get_xinteger(3, 1);
    const int ty = get_xinteger(4, 1);

    return window_create(type, title, lx, ty, (rx - lx - 1), (by - ty - 1));
}


/*  Function:           do_create_menu_window
 *      create_menu_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: create_menu_window - Create the menu window.

        int
        create_menu_window([int create])

    Macro Description:
        The 'create_menu_window()' primitive retrieves and optionally
        creates the menu if not already created. The menu resource is a
        singleton being the top line of display.

    Macro Parameters:
        create - Optional integer flag, if specified as non-zero and
            the menu resource has as yet to be created, it shall be
            built.

    Macro Returns:
        Returns the unique identifier of the menu window resource,
        otherwise -1 if the menu has yet to be created.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_window, create_tiled_window
 */
void
do_create_menu_window(void)     /* ([int create]) */
{
    WINDOW_t *wp, *ocurwp;
    int ret;

    for (wp = window_first(); wp; wp = window_next(wp)) {
        if (W_MENU == wp->w_type) {
            acc_assign_int((accint_t) wp->w_num);
            return;                             /* already exists, return original/singleton */
        }
    }

    if (isa_integer(1) && 0 == get_xinteger(1, 0)) {
        acc_assign_int((accint_t) -1);
        return;
    }

    ocurwp = curwp;                             /* save curwp */
    ret = window_build(W_MENU, "");             /* create menu (top line) */
    if (-1 != ret) {
        for (wp = window_first(); wp; wp = window_next(wp)) {
            if (wp != curwp && x_display_top == wp->w_y) {
                ++wp->w_y, --wp->w_h;           /* remove top line (TABLINE) */
            }
        }
        ++x_display_top;
        vtgarbled();
        curwp = ocurwp;                         /* restore */
    }
}


/*  Function:           inq_window_priority
 *      inq_window_priority primitive, retrieve window priority.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_priority - Retrieve the windows display priority.

        int
        inq_window_priority([int winnum])

    Macro Description:
        The 'inq_window_priority()' primitive retrieves the display
        priority of the specified window, , if omitted the current
        window.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Window priority, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_window_priority
 */
void
inq_window_priority(void)       /* ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);

    acc_assign_int((accint_t) (wp ? wp->w_priority : -1));
}


/*  Function:           do_set_window_priority
 *      set_window_priority primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_window_priority - Set the window display priority.

        int
        set_window_priority(int priority, [int winnum])

    Macro Description:
        The 'set_window_priority()' primitive sets the display priority
        of the specified window, if omitted the current window. The
        priority controls the display order of pop-up windows.

    Macro Parameters:
        priority - Window priority between the range of '0 .. 127'.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Returns the previous window priority, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_window_priority
 */
void
do_set_window_priority(void)    /* (int priority, [int winnum]) */
{
    const int priority = get_xinteger(1, 1);
    WINDOW_t *wp = window_argument(2);

    acc_assign_int((accint_t)(wp ? wp->w_priority : -1));
    if (wp) {
        if (wp->w_priority != (uint8_t)priority) {
            wp->w_priority = (uint8_t)priority;
            window_sort();
        }
    }
}


/*  Function:           inq_window_size
 *      inq_window_size primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_size - Retrieve the window size.

        int
        inq_window_size()

    Macro Description:
        The 'inq_window_size()' primitive determines the size of the
        current window.

    Macro Parameters:
        height - An integer variable which shall be populated with the
            window height in lines.

        width - An integer variable which shall be populated with the
            window width height in lines.

        left_offset - An integer variable which shall be populated with
            the number of columns that the window has been scroll
            horizontally, being the number of columns to the left.

        lmargin - An integer variable which shall be populated with the
            window left margin in lines.

        rmargin - An integer variable which shall be populated with the
            window right margin in lines.

    Macro Returns:
        The 'inq_window_size()' primitive returns the window height in
        rows, otherwise -1 if there is no current window.

    Macro Portability:
        The margins left and right are Grief extensions.

    Macro See Also:
        create_window, inq_window, inq_window_info, inq_window_color
 */
void
inq_window_size(void)           /* ([int &height, int &width], [int &left_offset],
                                        [int &lmargin], [int &rmargin]) */
{
    WINDOW_t *wp = curwp;

    if (NULL == wp) {
        acc_assign_int(-1);
    } else {
        acc_assign_int((accint_t) wp->w_h);
        argv_assign_int(1, (accint_t) win_height(wp));
        argv_assign_int(2, (accint_t) win_width(wp));
        argv_assign_int(3, (accint_t) wp->w_left_offset);
        argv_assign_int(4, (accint_t) wp->w_disp_lmargin);
        argv_assign_int(5, (accint_t) wp->w_disp_rmargin);
    }
}


/*  Function:           inq_window
 *      inq_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window - Retrieve the current window.

        int
        inq_window()

    Macro Description:
        The 'inq_window()' primitive retrieves the window identifier of
        the current window.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_window()' primitive returns the current window
        identifier otherwise -1 if there is no window. This identifier
        can be used to save and restore the active window.

    Macro Portability:
        n/a

    Macro See Also:
        change_window, create_window
 */
void
inq_window(void)
{
    acc_assign_int(curwp && curwp != &x_window_null ? (accint_t) curwp->w_num : -1);
}


/*  Function:           inq_window_info
 *      inq_window_info primitive, function to return information
 *      about the current window.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_info - Retrieve the current window information.

        int
        inq_window_info([int &winnum], [int &bufnum],
                [int &lx], [int &by], [int &rx], [int &ty],
                    [string &title = NULL], [string &message = NULL])

    Macro Description:
        The 'inq_window_info()' primitive retrieves information
        associated with the current windowed.

    Macro Parameters:
        winnum - An integer variable which shall be populated with the
            window identifier, otherwise -1 if no buffer is attached.

        bufnum - An integer variable which shall be populated with the
            buffer identifier of the buffer attached to the specified
            window, otherwise -1 if no buffer is attached.

        lx - An integer variable which shall be populated with the left
            x coordinate of the specified window.

        by - An integer variable which shall be populated with the
            bottom x coordinate of the specified window.

        rx - An integer variable which shall be populated with the
            right x coordinate of the specified window.

        ty - An integer variable which shall be populated with the top
            y coordinate of the specified window.

        title - A string variable, which shall be assigned the
            specified window title value.

        message - A string variable, which shall be assigned the
            specified window message value.

    Macro Returns:
        The 'inq_window_info()' primitive returns zero if the specified
        window is tiled, one if the window is an popup/overlapping
        window and two if the menu window.

    Macro Portability:
        This primitive differs from the BRIEF implementation, in that
        it only returns information associated with the current
        window and update the first argument to reflect the windows
        identifier, also see <inq_window_infox>.

        The 'title' and 'message' parameters are extensions.

    Macro See Also:
        inq_window, create_window, create_tiled_window, create_menu_window

 *<<GRIEF>>
    Macro: inq_window_infox - Retrieve information about a window.

        int
        inq_window_infox([int winnum], [int &bufnum],
                [int &lx], [int &by], [int &rx], [int &ty],
                    [string &title = NULL], [string &message = NULL])

    Macro Description:
        The 'inq_window_infox()' primitive retrieves information
        associated with the specified window 'winnum' or the
        current window if no window is specified.

    Macro Parameters:
        winnum - Optional integer window identifier, if omitted the
            current window is referenced.

        bufnum - An integer variable which shall be populated with the
            buffer identifier of the buffer attached to the specified
            window, otherwise -1 if no buffer is attached.

        lx - An integer variable which shall be populated with the left
            x coordinate of the specified window.

        by - An integer variable which shall be populated with the
            bottom x coordinate of the specified window.

        rx - An integer variable which shall be populated with the
            right x coordinate of the specified window.

        ty - An integer variable which shall be populated with the top
            y coordinate of the specified window.

        title - A string variable, which shall be assigned the
            specified window title value.

        message - A string variable, which shall be assigned the
            specified window message value.

    Macro Returns:
        The 'inq_window_info()' primitive returns zero if the specified
        window is tiled, one if the window is an popup/overlapping
        window and two if the menu window.

    Macro Portability:
        This primitive mirrors the original BRIEF interface presented
        by 'inq_window_info', permitted either the current or an
        explicit window to be referenced.

        The 'title' and 'message' parameters are extensions.

    Macro See Also:
        inq_window, create_window, create_tiled_window, create_menu_window

 */
void
inq_window_info(int isext)      /* ([int &winnum], [int &bufnum], [int &lx], [int &by], [int &rx], [int &ty],
                                        [string &title = NULL], [string &message = NULL]) */
{
    WINDOW_t *wp = (isext ? window_argument(1) : curwp);

    if (NULL == wp) {
        acc_assign_int((accint_t) -1);          /* no current window */

    } else {
        if (!isext) argv_assign_int(1, (accint_t) wp->w_num);

        argv_assign_int(2, (accint_t) (wp->w_bufp ? wp->w_bufp->b_bufnum : -1));
        argv_assign_int(3, (accint_t) wp->w_x);
        argv_assign_int(4, (accint_t) wp->w_y + wp->w_h + 1);
        argv_assign_int(5, (accint_t) wp->w_x + wp->w_w + 1);
        argv_assign_int(6, (accint_t) wp->w_y);

                                                /* extensions */
        argv_assign_str(7, wp->w_title ? wp->w_title : "");
        argv_assign_str(8, wp->w_message ? wp->w_message : "");

        if (W_MENU == wp->w_type) {
            acc_assign_int((accint_t) 2);       /* menu */

        } else if (W_POPUP == wp->w_type) {
            acc_assign_int((accint_t) 1);       /* popup */

        } else {
            acc_assign_int((accint_t) 0);       /* tiled */
        }
    }
}


/*  Function:           do_set_window_flags
 *      set_window_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_window_flags - Set window flags.

        void
        set_window_flags([int winnum],
                [string set|int or_mask], [string clear|int and_mask])

    Macro Description:
        The 'set_window_flags()' primitive modifies the window
        flags of the specified window 'winnum' otherwise if
        omitted the current window.

        If specified one or more flags shall be cleared using the
        'and_mask', in additional one or more flags are set using
        the 'or_mask'.

        Note that 'and_mask' (clear) is applied prior to the application
        of the 'or_mask' (set).

     *Window Flags*

        Available window flags.

(start table,format=basic)
    [Constant           [Name           [Description                                                           ]
    WF_HIDDEN           hidden          Hide the window from view, used to hide nested popup's/boss mode etc.
    WF_NO_SHADOW        no_shadow       Turn off the popup window shadows'
    WF_NO_BORDER        no_border       Turn off borders, regardless of the borders() setting.
    WF_SYSTEM           system          Window is a system window (e.g. menu).
    WF_SHOWANCHOR       showanchor      Show anchor regardless of selection status.
    WF_SELECTED         selected        Highlight the title regardless of selection status.
    WF_LAZYUPDATE       lazyupdate      Delay any updates until next refresh().
    WF_LINE_NUMBERS     line_numbers    Line numbers.
    WF_LINE_STATUS      line_status     Line status.
    WF_EOF_DISPLAY      eof_display     Show <EOF> marker.
    WF_TILDE_DISPLAY    tilde_display   Show <~> marker as EOF marker.
    WF_HIMODIFIED       himodified      Highlight modified lines.
    WF_HIADDITIONAL     hiadditional    Highlight additional lines.
    WF_HICHANGES        hichanges       Highlight in-line changes.
    WF_EOL_HILITE       eol_hilite      Limit highlight to EOL.
    WF_EOL_CURSOR       eol_cursor      Limit cursor to EOL.
(end table)

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

        set_mask - Optional mask of flags to set. May either be an
            integer of AND'ed together flag constants, or alternatively
            a string of comma separated flag names.

        clear_mask - Optional mask of flags to clear. May either be an
            integer of AND'ed together flag constants, or alternatively
            a string of comma separated flag names.

    Macro Returns:
        nothing

    Macro Portability:
        The feature set exposed differs from CRiSPEdit. It is
        therefore advised that the symbolic constants are using
        within a #ifdef construct.

        String flag forms are Grief extensions.

    Macro See Also:
        inq_window_flags
 */
void
do_set_window_flags(void)       /* ([int winnum], [string set|int or_mask], [string clear|int and_mask]) */
{
    static const char who[] = "set_window_flags";
    WINDOW_t *wp = window_argument(1);

    if (NULL != wp) {
        const uint32_t oflags = wp->w_flags;

        /* and/clear */
        if (isa_string(3)) {                    /* extension */
            uint32_t value;

            if (flag_decode(who, 1, get_xstr(3), &value) > 0) {
                wp->w_flags &= value;
            }
        } else if (isa_integer(3)) {
            wp->w_flags &= get_xinteger(3, 0);
        }

        /* or/clear */
        if (isa_string(2)) {                    /* extension */
            uint32_t value;

            if (flag_decode(who, 0, get_xstr(2), &value) > 0) {
                wp->w_flags |= value;
            }
        } else if (isa_integer(2)) {
            wp->w_flags |= get_xinteger(2, 0);
        }

        /* apply changes */
        if ((wp->w_flags & WF_HIDDEN) != (oflags & WF_HIDDEN)) {
            window_sort();
        }
        vtgarbled();
    }
}


/*  Function:           inq_window_flags
 *      inq_window_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_flags - Retrieve window flags.

        int
        inq_window_flags([int winnum], [string flags])

    Macro Description:
        The 'inq_window_flags()' primitive retrieves the window flags of
        the specified window 'winnum' otherwise if omitted the current
        window.

     *Window Flags*

        Available window flags.

(start table,format=basic)
    [Constant           [Name           [Description                                                           ]
    WF_HIDDEN           hidden          Hide the window from view, used to hide nested popup's/boss mode etc.
    WF_NO_SHADOW        no_shadow       Turn off the popup window shadows.
    WF_NO_BORDER        no_border       Turn off borders, regardless of the borders() setting.
    WF_SYSTEM           system          Window is a system window (e.g. menu).
    WF_SHOWANCHOR       showanchor      Show anchor regardless of selection status.
    WF_SELECTED         selected        Highlight the title regardless of selection status.
    WF_LAZYUPDATE       lazyupdate      Delay any updates until next refresh().
    WF_LINE_NUMBERS     line_numbers    Line numbers.
    WF_LINE_STATUS      line_status     Line status.
    WF_EOF_DISPLAY      eof_display     Show <EOF> marker.
    WF_TILDE_DISPLAY    tilde_display   Show <~> marker as EOF marker.
    WF_HIMODIFIED       himodified      Highlight modified lines.
    WF_HIADDITIONAL     hiadditional    Highlight additional lines.
    WF_HICHANGES        hichanges       Highlight in-line changes.
    WF_EOL_HILITE       eol_hilite      Limit highlight to EOL.
    WF_EOL_CURSOR       eol_cursor      Limit cursor to EOL.
(end table)

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

        flags - Optional comma separated list of window flag names, if
            given the value of the specific flags are returned,
            otherwise the full flags is returned.

    Macro Returns:
        Returns the associated window flags.

    Macro Portability:
        The feature set exposed differs from CRiSPEdit. It is
        therefore advised that the symbolic constants are using
        within #ifdef constructs.

        The 'flag' argument is a Grief extension.

    Macro See Also:
        set_window_flags
 */
void
inq_window_flags(void)          /* ([int winnum], [string flags]) */
{
    static const char who[] = "inq_window_flags";
    WINDOW_t *wp = window_argument(1);
    accint_t val = 0;                           /* unknown/error */

    if (wp) {
        if (isa_string(2)) {                    /* extension, by-name */
            uint32_t value = 0;

            val = 0;
            if (flag_decode(who, 0, get_xstr(2), &value) > 0) {
                val = (wp->w_flags & value);
            }
        } else {
            val = (accint_t) wp->w_flags;
        }
    }
    acc_assign_int(val);
}


static int
flag_decode(const char *who, int mode, const char *spec, uint32_t *value)
{
    const char *comma, *cursor = spec;
    uint32_t nvalue = 0;
    const struct winflag *flag;
    int flags = 0;

    while (NULL != (comma = strchr(cursor, ',')) || *cursor) {
        if (NULL != (flag = (NULL == comma ?    /* <value>[,<value>] */
                flag_lookup(cursor, (int)strlen(cursor)) : flag_lookup(cursor, comma - cursor)))) {
            nvalue |= flag->f_value;
            ++flags;

        } else {
            if (comma)  {
                errorf("%s: unknown flag '%*s'.", who, (int)(comma - spec), spec);
            } else {
                errorf("%s: unknown flag '%s'.", who, spec);
            }
            return -1;
        }
        if (NULL == comma) break;
        cursor = comma + 1;
    }

    *value = (1 == mode ? ~nvalue : nvalue);
    trace_log(" == flags=0x%x [ 0x%08x ]\n", flags, *value);
    return flags;
}


static const struct winflag *
flag_lookup(const char *name, int length)
{
    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        trace_ilog("\t %*s\n", length, name);
        for (i = 0; i < (sizeof(winflagnames)/sizeof(winflagnames[0])); ++i)
            if (length == winflagnames[i].f_length &&
                    0 == str_nicmp(winflagnames[i].f_name, name, length)) {
                return winflagnames + i;
            }
    }
    return NULL;
}


/*  Function:           do_change_window_pos
 *      change_window_pos primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: change_window_pos - Modify window coordinates/size.

        int
        change_window_pos([int topx], [int topy],
                [int width], [int height], [int winnum])

    Macro Description:
        The 'change_window_pos()' primitive modifies the
        coordinates and/or the size of the specified window, if
        omitted the current window.

        Note, care must be taken not position a window outside
        the physical window otherwise unexpected results
        including application crashes may result.

    Macro Parameters:
        topx - Optional integer, if stated sets the 'x' coordinate of the
            top left corner of the window, otherwise the current
            coordinate is taken.

        topy - Optional integer, if stated sets the 'y' coordinate of the
            top left corner of the window, otherwise the current
            coordinate is taken.

        width - Optional integer, if stated sets the new advised width in
            columns; the resulting width shall not be permitted to be
            smaller then the required width to display the current
            window title and/or message content.

        height - Optional integer, if stated sets the new advised
            height in lines.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Returns non-zero if the windows coordinates were modified, 
        otherwise zero if an error occurred.

        To determine the resulting window coordinates and size
        the <inq_window_info> primitive should be used, since
        boundary logic may have resized the window in order to
        obey the limits of the physical display.

    Macro Portability:
        The 'winnum' is a Grief extension.

    Macro See Also:
        inq_window_info, create_window
 */
void
do_change_window_pos(void)      /* ([int x], [int y], [int w], [int h], [int winnum]) */
{
    WINDOW_t *wp = window_argument(5);
    const int vtcols = ttcols(),
            vtrows = ttrows();
    int w1, w2, x, y, h, w;

    if (NULL == wp) {
        acc_assign_int((accint_t)0);            /* no window */
        return;
    }

    x = get_xinteger(1, wp->w_x);
    y = get_xinteger(2, wp->w_y);
    w = get_xinteger(3, wp->w_w);
    h = get_xinteger(4, wp->w_h);

    /* basic x and width boundary checks */
    if (x < 0) {
        x = 0;
    } else if (x >= vtcols) {
        x = vtcols - 1;
    }

    w1 = (wp->w_message ? utf8_swidth(wp->w_message) : 0) + 4; /*MCHAR*/
    w2 = (wp->w_title ? utf8_swidth(wp->w_title) : 0) + 4;
    if (w < w1) w = w1;
    if (w < w2) w = w2;

    if (x + w >= vtcols - 2) {
        w = vtcols - x - 3;
        if (w < 3) w = 3;
    }

    /* basic y and height boundary checks */
    if (y < 0) {
        y = 0;
    } else if (y >= vtrows) {
        y = vtrows - 1;
    }

    if (y + h >= vtrows - 2) {
        h = vtrows - y - 3;
        if (h < 3) h = 3;
    }

    wp->w_x = (uint16_t)x;
    wp->w_y = (uint16_t)y;
    wp->w_w = (uint16_t)w;
    wp->w_h = (uint16_t)h;
    wp->w_status = WFHARD;

    vtgarbled();                                /* must update */

    acc_assign_int((accint_t) 1);               /* success */
}


/*  Function:           do_next_window
 *      next_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: next_window - Obtain the next window identifier.

        int
        next_window(int winnum)

    Macro Description:
        The 'next_window()' primitive retrieves the window identifier
        of the next window from the internal window list relative to
        the specified window, if omitted the current window shall be
        referenced.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        The 'next_window()' primitive returns the window identifier
        of the next tiled window from the window list.

        If there is only a single window, then the same identifier as
        the specified shall be returned.

    Macro Example:

        Iterate though all windows

>       int curwin, win;
>
>       curwin = inq_window();
>       if ((win = curwin) != -1) {
>           do {
>
>               // ... process
>
>           } while ((win = next_window(win)) != curwin);
>       }

    Macro Portability:
        n/a

    Macro See Also:
        inq_window, set_window
 */
void
do_next_window(void)            /* int ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);

    if (NULL == wp || wp == &x_window_null) {
        acc_assign_int((accint_t) -1);

    } else {
        WINDOW_t *owp = wp;

        for (;;) {                              /* next window */
            if (NULL == (wp = window_next(wp))) {
                if (NULL == (wp = window_first())) {
                    acc_assign_int((accint_t) -1);
                    return;
                }
            }

            ED_TRACE(("==> %p [%d] menu:%d, \n", wp, wp->w_num));
            if (wp == owp) {                    /* looped? */
                acc_assign_int((accint_t) -2);
                return;
            }
            if (W_TILED == wp->w_type) {
                break;
            }
        }
        buf_change_window(wp);
        acc_assign_int((accint_t) wp->w_num);   /* locate window */
    }
}


/*  Function:           do_set_window
 *      set_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_window - Set the active window.

        int
        set_window(int winnum)

    Macro Description:
        The 'set_window()' primitive set the current window to the
        specified window identifier.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Returns non-zero if the window was changed, otherwise
        zero if specified window was already the current and no
        change occurred.

    Macro Portability:
        Unlike BRIEF the current buffer is not affected, which
        changed the buffer to the one associated with the
        specified window.

    Macro See Also:
        inq_window, next_window, change_window
 */
void
do_set_window(void)             /* ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);
    int ret = 0;

    if (wp && wp != curwp) {
        if (curwp) {
            curwp->w_status |= WFHARD;
        }
        wp->w_status |= WFHARD;
        set_curwp(wp);
        ret = 1;
    }
    acc_assign_int(ret);
}


/*  Function:           do_attach_buffer
 *      attach_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: attach_buffer - Attach a buffer to a window.

        void
        attach_buffer(int bufnum)

    Macro Description:
        The 'attach_buffer()' primitive attaches the specified buffer
        to the current window, so that the window becomes a view port
        into the buffer content.

        This interface is generally used in combination with
        <set_buffer>, <set_window> and/or buffer/window creation.
        Care should be taken to always have both the current buffer
        attached to the current window at end of any macros returning
        control backup, otherwise results are undefined.

        For example, create a buffer and window and then associate
        the two.

>           int buf = create_buffer("buffer);
>           int win = create_window(20, 10, 60, 2);
>           attach_buffer(buf);

        When the specified buffer is attached to the current window, 
        the top title of the window is changed to reflect the
        buffer or file-name associated with the buffer.

    Macro Parameters:
        bufnum - Buffer identifier to be attached.

    Notes:

        A few events automatically affect the attached buffer.

        o An explicit attach_buffer() is performed during
            <create_tiled_window> calls.

        o Deleting an attached buffer, results in all associated
            windows being reassigned the top buffer.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        create_buffer, create_window, inq_buffer
 */
void
do_attach_buffer(void)          /* (int bufnum) */
{
    BUFFER_t *bp;

    if (NULL == (bp = buf_lookup(get_xinteger(1, 0)))) {
        ewprintf("attach_buffer: no such buffer");
    } else {
        attach_buffer(curwp, bp);
    }
}


/*  Function:           do_delete_window
 *      delete_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_window - Delete a window.

        void
        delete_window([int winum])

    Macro Description:
        The 'delete_window()' primitive deletes the specified window.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        none

    Macro Portability:
        Unlike BRIEF any window may be deleted.

    Macro See Also:
        create_window, delete_edge
 */
void
do_delete_window(void)          /* void ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);
    int ismenu;

    if (NULL == wp)
        return;

    if (wp == curwp) {
        set_curwp(&x_window_null);
    }

    ismenu = (W_MENU == wp->w_type);

    window_delete(wp);

    if (ismenu) {                               /* foreach(window) */
        for (wp = window_first(); wp; wp = window_next(wp)) {
            if (x_display_top == wp->w_y) {
                --wp->w_y, ++wp->w_h;           /* reposition (TABLINE) */
            }
        }
        --x_display_top;
        assert(x_display_top >= 0);
    }
}


/*  Function:           do_close_window
 *      close_window primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: close_window - Close specified the window.

        void
        close_window([int winum])

    Macro Description:
        The 'close_window()' primitive is reserved for future BRIEF
        compatibility.

        The 'close_window()' primitive closes the specified window
        'winum'. The adjoining windows are enlarged to take up
        the space along the 'best fit' border.

        The 'best fit' border is the one that has a set of
        windows exactly matching the border of the window being
        closed. If there is more than one, the left is eliminated
        first, then right, bottom and top. The top left window
        becomes current.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        none

    Macro Portability:
        Not implemented.

    Macro See Also:
        delete_window
 */
void
do_close_window(void)           /* void ([int winnum]) */
{
    //TODO
    acc_assign_int(-1);
}


/*  Function:           inq_window_buf
 *      inq_window_buf primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_buf - Retrieve the associated buffer.

        int
        inq_window_buf([int winnum])

    Macro Description:
        The 'inq_window_buf()' primitive retrieves the associated buffer
        identifier of the specified window, if omitted the current
        window.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        The associated buffer identifier, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        attach_buffer, inq_window
 */
void
inq_window_buf(void)            /* int ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);

    if (wp && wp->w_bufp){
        acc_assign_int((accint_t) wp->w_bufp->b_bufnum);
    } else {
        acc_assign_int(-1);
    }
}


/*  Function:           do_set_top_left
 *      set_top_left primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_top_left - Manages window view port coordinates.

        int
        set_top_left([int line], [int column],
                [int winnum], [int csrline], [int csrcolumn], [int bufnum])

    Macro Description:
        The 'set_top_left()' primitive manages the view port coordinates,
        setting up the window and buffer positional relationship.

        The window effected can be referenced by one of three means.
        Directly, 'winnum' if specifies states the window identifier
        used. Indirectly, if 'winnum' is omitted the window referenced
        shall be the one attached to the specified buffer identifier
        'bufnum'. If neither are specified then the current window is
        assumed.

        The arguments 'line' and 'column' set the buffer coordinates
        displayed at the top left corner of the window.

        'csrline' and 'csrcolumn' set the buffers cursor position.

    Macro Parameters:
        line - Optional integer, specifies the line within the buffer which
            should be at the top of the window.
        column - Optional integer, specifies the column within the
            buffer which should be at the top of the window
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.
        csrline - Optional integer states the cursor position, if
            stated specifies the line within the buffer on which the
            cursor shall be positioned.
        csrcolumn - Optional integer states the cursor position, if
            stated specifies the column within the buffer on which the
            cursor shall be positioned.
        bufnum - Optional buffer identifier can be used to define a
            window indirectly, if 'winnum' is omitted the window
            referenced shall be the one attached to the specified
            buffer identifier.

    Macro Returns:
        The effected window identifier, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        inq_top_left
 */
void
do_set_top_left(void)           /* ([int line], [int column],
                                        [int winnum], [int csrline], [int csrcol], [int bufnum]) */
{
    WINDOW_t *wp;
    BUFFER_t *bp;

    wp = curwp;
    if (isa_integer(3)) {
        wp = window_lookup(get_xinteger(3, 0));

    } else if (isa_integer(6)) {
        /*
         *  If buffer ID specified, then use window attached to that buffer.
         */
        const int bufnum = get_xinteger(6, 0);

        if (bufnum > 0)
            for (wp = window_first(); wp; wp = window_next(wp)) {
                if (wp->w_bufp && bufnum == wp->w_bufp->b_bufnum) {
                    break;
                }
            }
    }

    if (NULL == wp) {
        acc_assign_int(-1);
        return;
    }

    if (isa_integer(1)) {                       /* line */
        const int line = get_xinteger(1, 1);

        if (line > 0) {
            window_top_line(wp, line);
        }
    }

    if (isa_integer(2)) {                       /* column */
        const int col = get_xinteger(2, 0);

        if (col > 0) {
            wp->w_left_offset = (col - 1);
            window_modify(wp, WFHARD);
        }
    }

    if (NULL == (bp = wp->w_bufp)) {
        acc_assign_int(-1);
        return;
    }

    if (isa_integer(4)) {                       /* csrline */
        const int value4 = get_xinteger(4, 0);

        if (value4 >= 1) {
            wp->w_line = value4;
            if (wp->w_line > bp->b_numlines) {
                if ((wp->w_line = bp->b_numlines) < 1) {
                    wp->w_line = 1;
                }
            }
        }
    }

    if (isa_integer(5)) {                       /* csrcolumn */
        const int value5 = get_xinteger(5, 0);

        if (value5 >= 1) {
            wp->w_col = value5;
        }
    }

    acc_assign_int(wp->w_num);
}


/*  Function:           inq_top_left
 *      inq_top_left primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_top_left - Retrieve window view port coordinates.

        int
        inq_top_left([int &top], [int &indent], [int winnum],
                [int &line], [int &col], [int &bufnum])

    Macro Description:
        The 'inq_top_left()' primitive retrieves the view port coordinates
        of the specified window into their associated buffer, if omitted
        the current window is referenced.

        The variables 'line' and 'column' retrieves the buffer
        coordinates displayed at the top left corner of the window.
        'csrline' and 'csrcolumn' retrieve the buffers cursor position.

    Macro Parameters:
        line - Optional integer, if specified is populated with the
            line at the top of the window.

        column - Optional integer, retrieves the column at the top left
            corner of the window.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

        csrline - Optional integer, if specified is populated with the
            buffer cursor line position.

        csrcolumn - Optional integer, retrieves the buffer cursor
            column position.

        bufnum - Optional integer, if specified is populated with the
            associated buffer identifier.

    Macro Returns:
        The associated window identifier, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        set_top_left, inq_position
 */
void
inq_top_left(void)              /* int ([int &line, [int &column], [int winnum],
                                            [int &csrline], [int &csrcolumn], [int &bufnum]) */
{
    const WINDOW_t *wp = window_argument(3);

    if (wp) {
        argv_assign_int(1, (accint_t) wp->w_top_line);
        argv_assign_int(2, (accint_t) wp->w_left_offset + 1);
        argv_assign_int(4, (accint_t) wp->w_line);
        argv_assign_int(5, (accint_t) wp->w_col);
        argv_assign_int(6, (accint_t) (wp->w_bufp ? wp->w_bufp->b_bufnum : -1));
        acc_assign_int(wp->w_num);

    } else {
        argv_assign_int(1, (accint_t) 0);
        argv_assign_int(2, (accint_t) 0);
        argv_assign_int(4, (accint_t) 0);
        argv_assign_int(5, (accint_t) 0);
        argv_assign_int(6, (accint_t) -1);
        acc_assign_int(-1);
    }
}


/*  Function:           do_window_color
 *	window_color primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: window_color - Set the window attribute.

        int
        window_color([int color|string color], [int winnum])

    Macro Description:
        The 'window_color()' primitive sets the background of the
        specified window otherwise if omitted the current window to the
        stated 'color'.

        When borders are disabled, this color shall be used as the
        background of the associated window allowing one to distinguish
        between individual views.

    Macro Parameters:
        color - Can be either an integer containing the color numeric
            value or a string containing the color name. If omitted,
            the next color within the color index sequence shall be
            assigned, see <color_index> for details.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Returns non-zero if the color was changed successfully,
        otherwise zero.

    Macro Portability:
        n/a

    Macro See Also:
        inq_window_color, color_index
 */
void
do_window_color(void)           /* ([int color], [int winnum]) */
                                /* ([string color], [int winnum]) */
{
    WINDOW_t *wp = window_argument(2);
    int ret = -1;

    if (wp) {
        if (isa_undef(1)) {                     /* assign next in order */
            window_attr(wp);

	} else if (isa_string(1)) {
            /*
	     *  extension - foreground,background
             */
            const char *sval = get_xstr(1),
                *comma = strchr(sval, ',');

            if (comma) {
                const int fg = color_enum(sval, comma - sval),
                    bg = color_enum(comma + 1, strlen(comma) - 1);

                if (fg >= 0 && bg >= 0) {
                    const vbyte_t ansi = (vbyte_t)(PTY_FG_SET(fg & 0x0f)|PTY_BG_SET(bg & 0x0f));

                    wp->w_attr = vtmapansi(ansi);
                    window_modify(wp, WFHARD);
                    ret = wp->w_attr;
                }
            }

        } else if (isa_integer(1)) {
            /*
             *  top nibble background, bottom is the foreground.
	     *
	     *	FIXME - interface is no longer suitable.
	     *
	     *	    window_color([bg_color], [fg_color], [winnum])
             */
            const accint_t color = get_xinteger(1, 0);

            if (color < 0) {
                wp->w_attr = (vbyte_t)-1;	/* disabled */
            } else {
                const vbyte_t ansi = (vbyte_t)(PTY_FG_SET(color & 0x0f)|PTY_BG_SET((color >> 4) & 0x0f));
                wp->w_attr = vtmapansi(ansi);
            }
            window_modify(wp, WFHARD);
            ret = wp->w_attr;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           inq_window_color
 *       inq_window_color primitive, look up value of window color.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_window_color - Retrieve the window attribute.

        int
        inq_window_color([int winnum])

    Macro Description:
        The 'inq_window_color' returns the background and foreground
        colors of a window. If winnum is not specified then the colors
        of the current window are returned.

        If the window color is set to < 0, then the default background
        color will be used. If the color is set to >= 0 then the
        specified color will be used.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        Returns the current window attribute, otherwise -1 on error.

    Macro Portability:
        Unlike BRIEF the assigned attributes is returned, whereas BRIEF
        returned an encoded color the foreground were the lower 4 bits
        (nibble) and the background was the upper 4 bits.

    Macro See Also:
        get_color_pair, window_color
 */
void
inq_window_color(void)          /* ([int winnum]) */
{
    WINDOW_t *wp = window_argument(1);

    if (NULL == wp) {
        acc_assign_int(-1);                     /* error */

    } else {
     /* acc_assign_int((accint_t) (wp->w_fgcolor | (wp->w_color << 4))); */
        acc_assign_int((accint_t) wp->w_attr);
    }
}


/*  Function:           do_set_ctrl_state
 *      Set the state of a window control.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_ctrl_state - Set the state of a window control.

        void
        set_ctrl_state(int ctrl, int state, [int winnum])

    Macro Description:
        The 'set_ctrl_state()' primitive sets the state of a window
        control of the specific window, if omitted the current window.

    Macro Parameters:
        ctrl - Control identifier.

            WCTRLO_CLOSE_BTN -      Close button.

            WCTRLO_ZOOM_BTN -       Zoom button.

            WCTRLO_VERT_SCROLL -    Vertical scroll.

            WCTRLO_HORZ_SCROLL -    Horizontal scroll.

            WCTRLO_VERT_THUMB  -    Vertical thumb.

            WCTRLO_HORZ_THUMB -     Horizontal thumb.

        state - An integer specifying the desired state.

            WCTRLS_ENABLE -         Enable the control all windows.

            WCTRLS_DISABLE -        Disable the control all windows.

            WCTRLS_HIDE -           Used to temporarily hide object for
                                    either the specified window or all
                                    windows, if window is omitted.

            WCTRLS_SHOW -           Restore the show status of a hidden
                                    control. HIDE/SHOW calls nests,
                                    hence for a hidden object to be
                                    displayed the number of HIDE
                                    operations must be matched by the
                                    same number of SHOW operations.

            WCTRLS_ZOOMED -         Display the zoomed button.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        inq_ctrl_state
 */
void
do_set_ctrl_state(void)         /* (int ctrl, int state, [int winnum]) */
{
    const int ctrl = get_xinteger(1, 0);
    const int state = get_xinteger(2, 0);
    WINDOW_t *wp = window_xargument(3);
    const uint32_t bit = (1 << ctrl);

    if (ctrl >= 0 && ctrl <= WCTRLO_MAX) {
        switch (state) {
        case WCTRLS_ENABLE:         /* Enable for all windows */
            if (0 == (w_ctrl_mask & bit)) {
                int change = 0;

                if (0 == w_ctrl_stack[ctrl]) {
                    w_ctrl_state |= bit;
                    ++change;
                }

                for (wp = window_first(); wp; wp = window_next(wp)) {
                    if (0 == wp->w_ctrl_stack[ctrl]) {
                        if (change) {
                            wp->w_status |= WFHARD;
                        }
                        wp->w_ctrl_state |= bit;
                    }
                }

                w_ctrl_mask |= bit;
            }
            break;

        case WCTRLS_DISABLE:        /* Disable for all windows */
            if (w_ctrl_mask & bit) {
                int change = 0;

                if (w_ctrl_state & bit) {
                    w_ctrl_state &= ~bit;
                    ++change;
                }

                for (wp = window_first(); wp; wp = window_next(wp)) {
                    if (wp->w_ctrl_state & bit) {
                        if (change) {
                            wp->w_status |= WFHARD;
                        }
                        wp->w_ctrl_state &= ~bit;
                    }
                }
                w_ctrl_mask &= ~bit;
            }
            break;

        case WCTRLS_SHOW:           /* Show the specified control */
            /*
             *  Pop current level (if any)
             *  If top level, enable
             */
            if (NULL != wp) {
                if (wp->w_ctrl_stack[ctrl]) {
                    --wp->w_ctrl_stack[ctrl];
                }

                if (w_ctrl_mask & bit) {
                    if (0 == wp->w_ctrl_stack[ctrl]) {
                        if (w_ctrl_state & bit) {
                            wp->w_status |= WFHARD;
                        }
                        wp->w_ctrl_state |= bit;
                    }
                }

            } else {
                if (w_ctrl_stack[ctrl]) {
                    --w_ctrl_stack[ctrl];
                }

                if (w_ctrl_mask & bit) {
                    if (0 == w_ctrl_stack[ctrl]) {
                        if (curwp && (curwp->w_ctrl_state & bit)) {
                            curwp->w_status |= WFHARD;
                        }
                        w_ctrl_state |= bit;
                    }
                }
            }
            break;

        case WCTRLS_HIDE:           /* Hide the specified control */
            if (NULL != wp) {
                if (0 == wp->w_ctrl_stack[ctrl]) {
                    if (w_ctrl_state & bit) {
                        wp->w_status |= WFHARD;
                    }
                    wp->w_ctrl_state &= ~bit;
                }
                ++wp->w_ctrl_stack[ctrl];

            } else {
                if (0 == w_ctrl_stack[ctrl]) {
                    if (curwp && (curwp->w_ctrl_state & bit)) {
                        curwp->w_status |= WFHARD;
                    }
                    w_ctrl_state &= ~bit;
                }
                ++w_ctrl_stack[ctrl];
            }
            break;

        case WCTRLS_ZOOMED:
            break;
        }
    }
}


/*  Function:           inq_ctrl_state
 *       Retrieve the state of a window control.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_ctrl_state - Retrieve the state of a window control.

        int
        inq_ctrl_state(int ctrl, [int winnum])

    Macro Description:
        The 'inq_ctrl_state()' primitive retrieves the state of a window
        control of the specific window, if omitted the current window.

    Macro Parameters:
        ctrl - Control identifier; see <set_ctrl_state> for details.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        The return value depends on the specified control.

        o CLOSE_BTN, ZOOM_BTN, VERT_SCROLL, and HORZ_SCROLL

           0  - Disabled.
           1  - Enabled.
           -1 - Hidden globally.
           -2 - Hidden explicitly for this window.
           -3 - Hidden globally and explicitly.

        o VERT_THUMB and HORZ_THUMB,

           -1 - Disabled.
           n  - The position (percentage) of the thumb on the scroll bar,
                with a value 0 thru to 100.

    Macro Portability:
        n/a

    Macro See Also:
        set_ctrl_state
 */
void
inq_ctrl_state(void)            /* (int ctrl, [int winnum]) */
{
    const int ctrl = get_xinteger(1, 0);
    const uint32_t bit = (1 << ctrl);
    int ret = 0;                                /* disabled */

    if ((ctrl >= 0 && ctrl <= WCTRLO_MAX) && (w_ctrl_mask & bit)) {

        if (isa_undef(2)) {
            ret = 1;                            /* enabled */

        } else {
            const WINDOW_t *wp = window_argument(2);

            if (wp) {
                if ((wp->w_ctrl_state & bit) && (w_ctrl_state & bit)) {

                    switch (ctrl) {
                    case WCTRLO_VERT_THUMB:     /* XXX - value needs conversion */
                        ret = wp->w_vthumb.t_value;
                        break;

                    case WCTRLO_HORZ_THUMB:     /* XXX - value needs conversion */
                        ret = wp->w_hthumb.t_value;
                        break;

                    default:
                        ret = 1;                /* enabled */
                        break;
                    }

                } else if ((wp->w_ctrl_state & bit) != 0 && (w_ctrl_state & bit) == 0) {
                    ret = -1;                   /* hidden globally */

                } else if ((wp->w_ctrl_state & bit) == 0 && (w_ctrl_state & bit) != 0) {
                    ret = -2;                   /* hidden explicitly */

                } else {
                    ret = -3;                   /* hidden both */
                }
            }
        }
    }
    acc_assign_int((accint_t) ret);
}

/*end*/
