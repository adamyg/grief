/* -*- mode: cr; tabs: 4; -*- */
/* $Id: mouse.cr,v 1.9 2024/05/15 08:22:44 cvsuser Exp $
 * Mouse driver macros
 *
 * This file contains code to support the mouse.
 *
 * Refer to the user guide for information on using the mouse. This code is
 * experimental and subject to change but is designed to be more or less usable even if
 * not fully functional.
 *
 *
 */

#include "grief.h"

#define MOUSE_MIDDLE_IGNORE         0x0001
#define MOUSE_WINDOWS_MOUSE_STYLE   0x0002
#define MOUSE_RIGHT_COL_SELECT      0x0004
#define MOUSE_PASTE_AT_CURSOR       0x0008
#define MOUSE_DISABLE_TIME_PANEL    0x0010
#define MOUSE_NO_COPY_CLIPBOARD     0x0020
#define MOUSE_DISABLE_DND           0x0040

#define POINTY_MODE     0x01
#define BORDER_MODE     0x02

#ifndef EDGE_UP
#define EDGE_UP         0
#define EDGE_RIGHT      1
#define EDGE_DOWN       2
#define EDGE_LEFT       3
#endif

/* TRUE if within a selection popup
 */
extern int              sel_warpable;

/* Popup nesting level, 0 if non are active
 */
extern int              popup_level;

       int              mouse_style = 0;
static int              mouse_mode = POINTY_MODE;
//static int            mouse_object;

static int              last_mx, last_my;
static int              last_mwin;
static int              last_line;

static int              num_motion;
static int              dragging;
static int              num_clicks;

/* This variable is essentially read-only. The variable is overloaded in select_buffer()
 * for popups where the current line must always be hilighted.
 */
static void             mouse_move_cursor(int down_button);
static int              delete_window_borders(int type);
static void             try_delete_edge(int e1, int e2, int e3, int e4);

static void             drag_start(void);
static void             drag_release(int is_ctrl);

void
main(void)
{
}


/*
 *  mouse_button_enable ---
 *      This macro is called on startup and by popup macros to define
 *      the mouse key events.
 */
void
mouse_button_enable(void)
{
    assign_to_key("<Button1-Down>",             "::but1_down 1 1");
    assign_to_key("<Button1-Double>",           "::but1_down 2 1");
    assign_to_key("<Button1-Motion>",           "::but1_down 0 1");
    assign_to_key("<Button1-Up>",               "::but1_up 0");

//  assign_to_key("<Shift-Button1-Down>",       "::but1_s_down");
//  assign_to_key("<Shift-Button1-Motion>",     "::but1_s_motion");
//  assign_to_key("<Shift-Button1-Up>",         "::but1_s_up");

//  assign_to_key("<Ctrl-Button1-Down>",        "::but1_down 1 2");
//  assign_to_key("<Ctrl-Button1-Motion>",      "::but1_down 0 2");
//  assign_to_key("<Ctrl-Button1-Up",           "::but1_up 1");

//  assign_to_key("<Alt-Button1-Down>",         "::but1_down 1 3");
//  assign_to_key("<Alt-Button1-Motion>",       "::but1_down 0 3");

    if (mouse_style & MOUSE_MIDDLE_IGNORE) {
        /*
         *  Ignore
         */

    } else if (mouse_style & MOUSE_WINDOWS_MOUSE_STYLE) {
        /*
         *  Windows style
         */
//      assign_to_key("<Button2-Up>",           "::but2_up");
//      assign_to_key("<Button2-Down>",         "::but2_down");
//      assign_to_key("<Button2-Motion>",       "::but2_motion");
//      assign_to_key("<Button3-Down>",         "::button_down 3");

    } else {
        /*
         *  Unix/Motif style
         */
//      assign_to_key("<Button3-Up>",           "::but1_up");
//      assign_to_key("<Button3-Down>",         "::mouse_move_cursor 1");
//      assign_to_key("<Button3-Motion>",       "::mouse_move_cursor 0");
//      assign_to_key("<Button2-Down>",         "::mouse_popup");
    }
}


void
mouse_enable(void)
{
    mouse_button_enable();
}


void
mouse_disable(void)
{
    assign_to_key("<Button1-Down>",             "nothing");
    assign_to_key("<Button1-Motion>",           "nothing");
    assign_to_key("<Button1-Up>",               "nothing");
//  assign_to_key("<Shift-Button1-Down>",       "nothing");
//  assign_to_key("<Shift-Button1-Up>",         "nothing");
//  assign_to_key("<Shift-Button1-Motion>",     "nothing");
//  assign_to_key("<Ctrl-Button1-Down>",        "nothing");
//  assign_to_key("<Ctrl-Button1-Motion>",      "nothing");
//  assign_to_key("<Ctrl-Button1-Up>",          "nothing");
//  assign_to_key("<Alt-Button1-Down>",         "nothing");
//  assign_to_key("<Alt-Button1-Motion>",       "nothing");
//  assign_to_key("<Alt-Button1-Up>",           "nothing");
//  assign_to_key("<Alt-Ctrl-Button1-Down>",    "nothing");
//  assign_to_key("<Alt-Ctrl-Button1-Motion>",  "nothing");
//  assign_to_key("<Alt-Ctrl-Button1-Up>",      "nothing");
//  assign_to_key("<Button2-Down>",             "nothing");
//  assign_to_key("<Button2-Motion>",           "nothing");
//  assign_to_key("<Button2-Up>",               "nothing");
//  assign_to_key("<Button3-Down>",             "nothing");
//  assign_to_key("<Button3-Motion>",           "nothing");
//  assign_to_key("<Button3-Up>",               "nothing");
}



/*
 *  but1_down ---
 *      Handle the left hand button being pressed.
 *
 *      If pressed in conjunction with the Ctrl or Alt key, then do a column hilight.
 */
static void
but1_down(int down_button, int anchor_type)
{
    int tm, x, y, win, buf, line, col, where, region, event;

    tm = get_mouse_pos(x, y, win, line, col, where, region, event);

    /*
     *  Count clicks
     */
    if (1 == down_button) {
        if (tm < 0 || tm > CLICK_TIME_MS) {
            num_clicks = 1;                     /* click window expired */
        } else {
            ++num_clicks;
        }

    } else if (2 == down_button) {
        ++num_clicks;                           /* double click */

    }

    /*
     *  If a popup is active,
     *      validate if the click is within the current bounders otherwise exit.
     */
    if (popup_level && (win != inq_window())) {
        return;
    }

    if (win >= 0) {
        /*
         *  Within a window, make active
         */
        inq_top_left(NULL, NULL, win, NULL, NULL, buf);
        if (buf <= 0)
            return;
        set_buffer(buf);
        set_window(win);
    }

    /* Process event */
//  if (mouse_mode == BORDER_MODE) {            /* border gran in progress */
//      where = mouse_object;
//  }

    switch (where) {
    case MOBJ_INSIDE:
        /*
         *  Are we starting a drag?
         */
        if (down_button) {
            if (region == 2 && !sel_warpable && num_clicks == 1) {
                drag_start();
            }
        }

        /*
         *  Let cursor follow the mouse so we can see where to drop.
         */
        if (dragging) {
            if (inq_marked() == MK_LINE) {
                col = 1;
            }
            move_abs(line, col);
            return;
        }

        /*
         *  If user hits the mouse button and we have a region hilighted,
         *  and cursor is inside the  region then do a drag
         *  operation; otherwise, unhilight it.
         */
        if (down_button && inq_marked()) {
            raise_anchor();

        /*
         *  See if user is dragging the mouse button. If we've just started
         *  dragging it, then drop an anchor.
         */
        } else if (! down_button && ! inq_marked()) {
            drop_anchor(anchor_type);
        }

        /*
         *  Inside a selection box
         */
        if (sel_warpable) {
            move_abs(line, col);                /* move selection */
            sel_warp();
            if (line == last_line && num_clicks == 2) {
                push_back(key_to_int("<Enter>"));
                break;                          /* double-clicks */
            }
            last_line = line;
            return;
        }

        /*
         *  Handle multiple clicks here.
         */
        switch (num_clicks) {
        case 2:
            next_char();
            re_search(SF_UNIX | SF_BACKWARDS, "\\<");
            if (! inq_marked())
                drop_anchor(MK_NONINC);
            re_search(SF_UNIX, "\\>");
            break;
        case 3:
            if (! inq_marked())
                drop_anchor(MK_LINE);
            break;
        case 4:
//          if (inq_marked())
//              raise_anchor();
//          select_all();                       /* TODO */
            break;
        }
        break;

    case MOBJ_TITLE:
        if (down_button) {
            if (popup_level == 0) {
                edit_next_buffer();             /* next buffer */
            } else {
                push_back(key_to_int("<Esc>")); /* exit popup */
            }
        }
        break;

    case MOBJ_LEFT_EDGE:
    case MOBJ_RIGHT_EDGE:
        /*
         *  Split a window, creating a new window boundary where the mouse is.
         *
         *  Its too easy to split a window when you didnt mean to. So
         *  user has to double click to do this.
         */
        if (popup_level == 0 && num_clicks >= 2) {
            int ml, wy, i, j;

            ml = inq_msg_level();               /* quiet now */
            set_msg_level(3);

            if (down_button) {
                /*
                 *  Attempt to create a new edge
                 */
                create_edge(EDGE_DOWN);
                inq_window_info(NULL, NULL, NULL, NULL, NULL, wy);

                j = y - wy + 1;
                i = move_edge(EDGE_UP, j);
                if (j < 0)
                    j = -j;
                if (i != j) {                   /* too close - undo */
                    change_window(EDGE_UP);
                    delete_edge(EDGE_DOWN);
                } else {
                    mouse_mode = BORDER_MODE;
//                  mouse_object = where;
                }

            } else if (mouse_mode == BORDER_MODE) {
                /*
                 *  Drop the edge
                 */
                inq_window_info(NULL, NULL, NULL, NULL, NULL, wy);
                move_edge(EDGE_UP, y - wy + 1);
            }
            set_msg_level(ml);
        }
        break;

    case MOBJ_TOP_EDGE:
    case MOBJ_BOTTOM_EDGE:
        /*
         *  Split a window, creating a new window boundary where the mouse is.
         *
         *  Its too easy to split a window when you didnt mean to. So
         *  user has to double click to do this.
         */
        if (popup_level == 0 && num_clicks >= 2) {
            int ml, wx, i, j;

            ml = inq_msg_level();               /* quiet now */
            set_msg_level(3);

            if (down_button) {
                /*
                 *  Attempt to create a new edge
                 */
                create_edge(EDGE_LEFT);
                inq_window_info(NULL, NULL, NULL, NULL, wx);
                j = x - wx - 1;
                i = move_edge(EDGE_RIGHT, j);
                if (i != j) {                   /* too close - undo */
                    change_window(EDGE_RIGHT);
                    delete_edge(EDGE_LEFT);
                } else {
                    mouse_mode = BORDER_MODE;
//                  mouse_object = where;
                }

            } else if (mouse_mode == BORDER_MODE) {
                /*
                 *  Drop the edge
                 */
                inq_window_info(NULL, NULL, NULL, NULL, wx);
                if (move_edge(EDGE_RIGHT, x - wx - 1) <= 0) {
                    inq_window_info(NULL, NULL, wx);
                    move_edge(EDGE_LEFT, x - wx);
                }
            }
            set_msg_level(ml);
        }
        break;
    }
    last_line = -1;
}


/*
 *  but1_up ---
 *      Handle the left hand button being release.
 */
static void
but1_up(int is_ctrl)
{
    /*
     *  Restore mouse mode so that normal pointy type things can occur.
     */
    mouse_mode = POINTY_MODE;

    /*
     *  Handle where we are going to drag text to.
     */
    if (! inq_marked()) {
        /*
         *  If in column select mode then do the context popup.
         */
        dragging = FALSE;
//      if (is_ctrl) {
//          popup();
//      }
        return;
    }

    if (dragging) {
        drag_release(is_ctrl);
        return;
    }
}


/*
 *  but1_s_down ---
 *      Handle the left hand button being pressed in conjunction with
 *      the Shift key.
 */
static void
but1_s_down(void)
{
    int x, y, win, buf, line, col, where;

    get_mouse_pos(x, y, win, line, col, where);

    last_mx = x;
    last_my = y;
    last_mwin = win;

    num_motion = 0;
    if (win >= 0) {
        inq_top_left(NULL, NULL, win, NULL, NULL, buf);
        set_window(win);
        set_buffer(buf);
        move_abs(line, col);
        if (inq_marked()) {
            num_motion = -1;
        } else {
            drop_anchor(MK_NORMAL);
        }
    }
    last_line = -1;
}


/*
 *  but1_s_down ---
 *      Handle mouse movements with the left hand button being pressed
 *      in conjunction with the Shift key.
 */
static void
but1_s_motion(void)
{
    int x, y, win, buf, line, col, where;

    get_mouse_pos(x, y, win, line, col, where);

    if (num_motion >= 0) {
        ++num_motion;
    }

    if (win == last_mwin && win >= 0) {
        /*
         *  Same window, reposition cursor
         */
        inq_top_left(NULL, NULL, win, NULL, NULL, buf);
        set_window(win);
        set_buffer(buf);
        move_abs(line, col);

    } else {
        /*
         *  Mouse no longer in  same window so we calculate what to do
         *  ourselves. Simply go up or down a line.
         */
        if (y > last_my) {
            down();
        } else if (y  < last_my) {
            up();
        }
    }
    last_mx = x;
    last_my = y;
    last_line = -1;
}


/*
 *  but1_s_up ---
 *      Handle the left hand button being released in conjunction with
 *      the Shift key.
 */
static void
but1_s_up(void)
{
    if (0 == num_motion) {
        raise_anchor();
    }
    mouse_mode = POINTY_MODE;
}


static void
but2_down(void)
{
    if (0 == (mouse_style & MOUSE_MIDDLE_IGNORE)) {
        mouse_move_cursor(1);
    }
}


static void
but2_motion(void)
{
    if (0 == (mouse_style & MOUSE_MIDDLE_IGNORE)) {
        mouse_move_cursor(0);
    }
}


static void
but2_up(void)
{
    if (0 == (mouse_style & MOUSE_MIDDLE_IGNORE)) {
        but1_up(0);
    }
}


/*
 *  mouse_move_cursor ---
 *      Function called when user hits the middle (2nd) mouse button. Just
 *      move cursor keeping hilight if any.
 */
static void
mouse_move_cursor(int down_button)
{
    int x, y, win, buf, line, col, where;

    /*
     *  If in border grab mode then keep dragging the border.
     */
    if (mouse_mode == BORDER_MODE) {
        but1_down(down_button, 1);
        return;
    }

    get_mouse_pos(x, y, win, line, col, where);

    if (popup_level && win != inq_window()) {
        return;                                 /* within popup ? */
    }

    if (win >= 0) {                             /* new window */
        inq_top_left(NULL, NULL, win, NULL, NULL, buf);
        set_window(win);
        set_buffer(buf);
    }

    switch (where) {
    case MOBJ_INSIDE:
        /*
         *  If no region hilighted then hilight it.
         */
        if (inq_marked() == 0) {
            drop_anchor(MK_NORMAL);
        }
        move_abs(line, col);
        if (sel_warpable) {
            sel_warp();
        }
        break;

    case MOBJ_LEFT_EDGE:
    case MOBJ_RIGHT_EDGE:
    case MOBJ_TOP_EDGE:
    case MOBJ_BOTTOM_EDGE:
        delete_window_borders(where);
        break;
    }
    last_line = -1;
}


static void
drag_start(void)
{
    int sline, scol, eline, ecol;

    dragging = TRUE;
    switch (inq_marked(sline, scol, eline, ecol)) {
    case MK_COLUMN:
    case MK_NORMAL:
    case MK_LINE:
        inq_position(current_line, current_col);
        if (current_line == sline && current_col == scol) {
            end_anchor(sline, scol);
        } else {
            end_anchor(eline, ecol);
        }
        break;
    default:
        end_anchor(eline, ecol + 1);
        break;
    }
}


/*
 *  drag_release ---
 *      We started dragging some text so handle the button being released.
 */
static void
drag_release(int is_ctrl)
{
    int x, y, win, line, col, where;
    int type, sline, scol, eline, ecol;

    type = inq_marked(sline, scol, eline, ecol);
    get_mouse_pos(x, y, win, line, col, where);

    /* If in line mode, make sure we paste at the start of the line. */
    if (type == MK_LINE)
        col = 1;

    if (where == MOBJ_INSIDE) {
        int i, i0;

        i0 = i = inside_region(NULL, line, col);        /*???*/
        if (type == MK_COLUMN && i == 3)
            i = 5;

        switch (i) {
          case 0:
            /***********************************************/
            /*   No region.                                */
            /***********************************************/
            break;

          case 1:
            /***********************************************/
            /*   Before  the  region.  Need to delete and  */
            /*   then paste.                               */
            /***********************************************/
            if (is_ctrl)
                copy();
            else
                cut();

            set_window(win);
            set_buffer(inq_window_buf());
            move_abs(line, col);

            paste();
            break;

          case 2:
            /***********************************************/
            /*   Inside the region - ignore the drop.      */
            /***********************************************/
            if (num_clicks == 1)
                raise_anchor();
            break;

          case 3:
            /***********************************************/
            /*   After  the  region.  The  following code  */
            /*   attempts  handle  the  fact  that we are  */
            /*   trying  to move a block of code possible  */
            /*   earlier on in the same line, and keeping  */
            /*   the   final  cursor  position  where  we  */
            /*   expect it to be.                  */
            /***********************************************/
            if (is_ctrl) {
                copy();
                set_window(win);
                set_buffer(inq_window_buf());
                move_abs(line, col);
                paste();
                break;
                }

            {string s = get_region();
            int mtype = inq_marked();

            if (mtype == MK_NONINC)
                end_anchor(eline, ecol + 1);
            else
                end_anchor(eline, ecol);

            set_window(win);
            set_buffer(inq_window_buf());
            move_abs(line, col);
            if (inq_marked() != MK_LINE)
                insert("\n");
            drop_bookmark(1000, "y");
            insert(s);
            delete_block();
            goto_bookmark(1000);
            delete_bookmark(1000);
            if (mtype != MK_LINE) {
                backspace();
                next_char(strlen(s));
                }
            }
            break;
          case 4:
            /***********************************************/
            /*   Pasting to the left.              */
            /***********************************************/
            if (is_ctrl)
                copy();
            else
                cut();
            set_window(win);
            set_buffer(inq_window_buf());
            move_abs(line, col);
            paste();
            break;

          case 5:
            /***********************************************/
            /*   Pasting to the right.             */
            /***********************************************/
            if (is_ctrl)
                copy();
            else
                cut();
            set_window(win);
            set_buffer(inq_window_buf());
            if (i0 == 3)
                move_abs(line, col - (ecol - scol) - 1);
            else
                move_abs(line, col);
//          do_paste(2);
            break;
          }
    }

    dragging = FALSE;
}


/*
 *  delete_window_borders ---
 *      Routine to delete a window border depending on where we clicked.
 */
static int
delete_window_borders(int where)
{
    int msg_level;

    msg_level = inq_msg_level();
    set_msg_level(3);

    switch (where) {
    case MOBJ_LEFT_EDGE:
    case MOBJ_RIGHT_EDGE:
        if (! popup_level)
            try_delete_edge(EDGE_UP, EDGE_DOWN, EDGE_LEFT, EDGE_RIGHT);
        set_msg_level(msg_level);
        return 1;

    case MOBJ_TOP_EDGE:
    case MOBJ_BOTTOM_EDGE:
        if (! popup_level)
            try_delete_edge(EDGE_LEFT, EDGE_RIGHT, EDGE_UP, EDGE_DOWN);
        set_msg_level(msg_level);
        return 1;
    }

    set_msg_level(msg_level);
    return 0;
}


/*
 *  try_delete_edge ---
 *      Try and delete an edge. Because we may be at an intersection
 *      determining the 'correct' edge may be tricky so try in all
 *      directions based on a preference scale.
 */
static void
try_delete_edge(int e1, int e2, int e3, int e4)
{
    if (delete_edge(e1))
    if (delete_edge(e2))
    if (delete_edge(e3))
        delete_edge(e4);
}

/*end*/
