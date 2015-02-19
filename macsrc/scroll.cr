/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: scroll.cr,v 1.6 2014/10/27 23:28:27 ayoung Exp $
 * Scroll locking.
 *
 *
 */

#include "grief.h"

static int        get_window(string msg);

static int        sl_line, sl_col;
static int        sl_line2, sl_col2;
static int        sl_win1, sl_win2;
static int        sl_tline, sl_tcol;

static int        sl_enabled = FALSE;


/*
 *  scroll ---
 *      Function called when ScrollLock key pressed. It toggles the state of
 *      the scroll lock.
 */
void
scroll(void)
{
    int buf;

    if (! sl_enabled) {
        int csrline, csrcol;

        sl_win1 = inq_window();
        inq_top_left(sl_tline, sl_tcol, NULL, csrline, csrcol);
        sl_line = csrline - sl_tline;
        sl_col = csrcol - sl_tcol;
        buf = inq_buffer();

        /*
         *  If we have another window on view, then use that as our
         *  scroll linked window.
         */
        if (next_window() >= 0 && inq_window() != sl_win1) {
            set_window(sl_win1);
            sl_win2 = get_window("Scroll-lock to other window");
            set_window(sl_win1);
            set_buffer(buf);
            if (sl_win2 < 0) {
                return;
            }
            inq_top_left(sl_line2, sl_col2, sl_win2);

        } else {
            sl_win2 = -1;
        }
        message("Scroll Lock enabled.");
        register_macro(REG_KEYBOARD, "scroll_lock");

    } else {
        message("Scroll Lock disabled.");
        unregister_macro(REG_KEYBOARD, "scroll_lock");
    }

    sl_enabled = !sl_enabled;
}


/*
 *  scroll_lock ---
 *      registered macro called whenever crisp is ready to read a key from the
 *      keyboard. We cause the other window and the scroll locked window to stay
 *      where we want them.
 */
int
scroll_lock(void)
{
    int line, col, csrline, csrcol;

    if (sl_win1 == inq_window()) {
        inq_top_left(line, col, NULL, csrline, csrcol);
        line = csrline - sl_line;
        col = csrcol - sl_col;
        set_top_left(line, col);
        if (sl_win2 >= 0) {
            set_top_left(sl_line2 + line - sl_tline, sl_col2 + col - sl_tcol,
                sl_win2, sl_line2 + line - sl_tline, sl_col2 + col - sl_tcol);
        }
        refresh();
    }
    return 0;
}


/*
 *  get_window ---
 *      Get a window pointed to by user.
 */
static int
get_window(string msg)
{
    int curwin = inq_window();
    int win;

    if (change_window(NULL, msg) == 0) {
        return -1;
    }
    win = inq_window();
    set_window(curwin);
    return win;
}

/*end*/
