/* -*- mode: cr; tabs: 4; -*- */
/* $Id: mouse.cr,v 1.12 2024/09/08 16:25:51 cvsuser Exp $
 * Mouse support.
 *
 *
 */

#include "grief.h"

#define MODE_NONE       0x00
#define MODE_DROP       0x01                    // cursor dropped/move
#define MODE_DRAG       0x02                    // drag mode
#define MODE_EDGE       0x10                    // edge mode

#ifndef EDGE_UP
#define EDGE_UP         0
#define EDGE_RIGHT      1
#define EDGE_DOWN       2
#define EDGE_LEFT       3
#endif

static int mouse_mode = MODE_NONE;

extern void popup_mouse(~ int x, ~ int y);      // popup.cr


void
main(void)
{
}


/*
 *  mouse_button_enable ---
 *      Mouse startup registration
 *
 *    Mouse Element        Action               Purpose
 *    -------------------------------------------------------------------------------
 *    Left mouse button    Click                Moves the cursor.
 *
 *    Left mouse button    Double-click         Highlights a word.
 *
 *    Left mouse button    Drag                 Drops and drags the associated anchor.
 */
void
mouse_button_enable(void)
{
    assign_to_key("<Button1-Down>",             "::button1_down 1 " + MK_NORMAL);
    assign_to_key("<Button1-Double>",           "::button1_down 2 " + MK_NORMAL);
    assign_to_key("<Button1-Motion>",           "::button1_down 0 " + MK_NORMAL);
    assign_to_key("<Button1-Up>",               "::button1_up");
    assign_to_key("<Button3-Down>",             "::button3_down 1");

    assign_to_key("<Ctrl-Button1-Down>",        "::button1_down 1 " + MK_COLUMN);
    assign_to_key("<Ctrl-Button1-Double>",      "::button1_down 2 " + MK_COLUMN);
    assign_to_key("<Ctrl-Button1-Motion>",      "::button1_down 0 " + MK_COLUMN);
    assign_to_key("<Ctrl-Button1-Up>",          "::button1_up");

    assign_to_key("<Alt-Button1-Down>",         "::button1_down 1 " + MK_LINE);
    assign_to_key("<Alt-Button1-Double>",       "::button1_down 2 " + MK_LINE);
    assign_to_key("<Alt-Button1-Motion>",       "::button1_down 0 " + MK_LINE);
    assign_to_key("<Alt-Button1-Up>",           "::button1_up");
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
    assign_to_key("<Button1-Double",            "nothing");
    assign_to_key("<Button1-Motion>",           "nothing");
    assign_to_key("<Button1-Up>",               "nothing");
    assign_to_key("<Button3-Down>",             "nothing");

    assign_to_key("<Ctrl-Button1-Down>",        "nothing");
    assign_to_key("<Ctrl-Button1-Double>",      "nothing");
    assign_to_key("<Ctrl-Button1-Motion>",      "nothing");
    assign_to_key("<Ctrl-Button1-Up>",          "nothing");

    assign_to_key("<Alt-Button1-Down>",         "nothing");
    assign_to_key("<Alt-Button1-Double>",       "nothing");
    assign_to_key("<Alt-Button1-Motion>",       "nothing");
    assign_to_key("<Alt-Button1-Up>",           "nothing");
}



/*
 *  but1_down ---
 *      Handle the left hand button being pressed.
 *
 *      If pressed in conjunction with the Ctrl or Alt key, then do a column hilight.
 *
 *  Parameters:
 *      click - Active click count, otherwise 0 for up.
 *
 *  Returns:
 *      void
 */
static void
button1_down(int clicks, int anchor_type)
{
    extern int popup_level;                     // grief.cr
    int x, y, win, buf, line, col, where, region, event;

    // modal popup active; ignore.
    get_mouse_pos(x, y, win, line, col, where, region, event);
    if (popup_level && (win != inq_window())) {
        return;
    }

    // within a window, make active.
    if (win >= 0) {
        inq_top_left(NULL, NULL, win, NULL, NULL, buf);
        if (buf <= 0) {
            return;
        }
        set_buffer(buf);
        set_window(win);
    }

    // region operations
    switch (where) {
    case MOBJ_INSIDE:

        // inside window
        switch (clicks) {
        case 0:     // motion
            // active region, motion
            if (mouse_mode == MODE_DROP) {      // initial move
                if (0 == region) {
                    drop_anchor(anchor_type);
                } else {
                    end_anchor();
                }
                mouse_mode = MODE_DRAG;
             }

            if (mouse_mode == MODE_DRAG) {
                if (inq_marked() == MK_LINE) {
                    col = 1;                    // column mode, col=1
                }
                move_abs(line, col);
                end_anchor(line, col);
                return;
            }
            break;

        case 1:     // click
            // initial click, enable drag mode
            if (mouse_mode != MODE_EDGE) {
                if (region) {
                    raise_anchor();
                }
                move_abs(line, col);
                mouse_mode = MODE_DROP;
            }
            break;

        case 2:      // double-click
            // select word
            if (inq_marked()) {
                raise_anchor();
            }
            next_char();
            re_search(SF_UNIX | SF_BACKWARDS, "\\<");
            drop_anchor(MK_NONINC);
            re_search(SF_UNIX, "\\>");
            break;
        }
        break;

    case MOBJ_LEFT_EDGE:
    case MOBJ_RIGHT_EDGE:
        // window edge.
        if (popup_level == 0 && clicks >= 2) {
            int wy, ml = inq_msg_level();

            set_msg_level(3);
            switch (mouse_mode) {
            case MODE_NONE:
                // attempt to create a new edge.
                if (create_edge(EDGE_DOWN) <= 0) {
                     mouse_mode = MODE_EDGE;
                }
                break;

            case MODE_EDGE:
                // move edge.
                inq_window_info(NULL, NULL, NULL, NULL, NULL, wy);
                move_edge(EDGE_UP, y - wy + 1);
                break;
            }
            set_msg_level(ml);
        }
        break;

    case MOBJ_TOP_EDGE:
    case MOBJ_BOTTOM_EDGE:
        // window edge.
        if (popup_level == 0 && clicks >= 2) {
            int wx, ml = inq_msg_level();

            set_msg_level(3);
            switch (mouse_mode) {
            case MODE_NONE:
                // attempt to create a new edge.
                if (create_edge(EDGE_LEFT) <= 0) {
                    mouse_mode = MODE_EDGE;
                }
                break;

            case MODE_EDGE:
                // move edge.
                inq_window_info(NULL, NULL, NULL, NULL, wx, NULL);
                if (move_edge(EDGE_RIGHT, x - wx - 1) <= 0) {
                    inq_window_info(NULL, NULL, wx);
                    move_edge(EDGE_LEFT, x - wx);
                }
                break;
            }
            set_msg_level(ml);
        }
        break;
    }
}


static void
button3_down(int clicks)
{
    extern int popup_level;                     // grief.cr
    int x, y, win, line, col, where;

    // modal popup active; ignore.
    get_mouse_pos(x, y, win, line, col, where);
    if (popup_level && (win != inq_window())) {
        return;
    }

    // region operations
    switch (where) {
    case MOBJ_INSIDE:
        // inside window
        switch (clicks) {
        case 1:     // click
            popup_mouse(x - 1, y - 1);
            break;
        }
    }
}


/*
 *  button1_up ---
 *      Button1 up event handler
 */
static void
button1_up()
{
    mouse_mode = MODE_NONE;
}

/*end*/
