/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: zoom.cr,v 1.8 2014/10/27 23:28:30 ayoung Exp $
 * Zooms a window to full screen size.
 *
 *
 */

#include "grief.h"

/*
 *  List of windows and buffer information needed to do an unzoom.
 */
static list         zoom_list;


/*
 *  Remember size of screen when zoom was done. If screen size
 *  changed when we want to unzoom, then forget it.
 */
static int          zoom_lines;
static int          zoom_cols;


void
zoom(void)
{
    int curwin = inq_window();
    int curbuf = inq_buffer();
    int win_id, buf_id, lx, by, rx, ty;
    int line, col, top_left;
    int tl, tc, cl, cc;
    int l, c, i;
    string filename;

    /*
     *  If the 'next' window is the same as this one, then we need to unzoom.
     *  Otherwise we need to save the state of all the other windows on the screen
     */
    inq_top_left(tl, tc, NULL, cl, cc);
    next_window();
    if (inq_window() == curwin) {
        unzoom();
        return;
    }
    inq_screen_size(zoom_lines, zoom_cols);

    /*
     *  Ok, so user wants to zoom this window. First save state of all windows.
     */
    zoom_list = NULL;
    set_window(curwin);
    set_buffer(curbuf);
    while (1) {
        inq_window_info(win_id, buf_id, lx, by, rx, ty);
        inq_position(line, col);
        inq_top_left(top_left);
        inq_names(filename);

        zoom_list += lx + " " + by + " " + rx + " " + ty + " " + line
                    + " " + col + " " + top_left + " " + filename;

        /*
         *  Go to the next window, and if we end up back where we started
         *  then we've finished saving the info.
         */
        next_window();
        if (inq_window() == curwin) {
            break;
        }
    }

    /*
     *  Now delete all windows on display.
     */
    while ((i = inq_window()) >= 0) {
        next_window();
        delete_window(i);
    }

    /*
     *  Now create ourselves a full screen window.
     */
    inq_screen_size(l, c);
    create_tiled_window(0, l - 2, c - 1, 0, curbuf);
    set_top_left(tl, tc, NULL, cl, cc);
    message("Re-execute command to unzoom.");
}


/*
 *  This macro restores the window information when we unzoom a window.
 */
void
unzoom(void)
{
    int curwin = inq_window();
    int i, w, b, lines, cols;
    list l;

    /*
     *  Make sure window is still same size.
     */
    inq_screen_size(lines, cols);
    if (lines != zoom_lines || cols != zoom_cols) {
        return;
    }

    /*
     *  If user accidentally calls this macro then ignore it if
     *  we don't have any saved information.
     */
    if (length_of_list(zoom_list) == 0) {
        return;
    }

    /*
     *  Go around creating all the old windows.
     */
    delete_window(curwin);
    curwin = -1;
    for (i = 0; i < length_of_list(zoom_list); i++) {
        /*
         *  Split information up into tokens.
         */
        l = split(zoom_list[i], " ");
        w = create_tiled_window(atoi(l[0]), atoi(l[1]), atoi(l[2]), atoi(l[3]));
        set_window(w);

        if (edit_file(l[7]) >= 0) {
            attach_buffer(inq_buffer());
            if (curwin < 0) {
                curwin = w;
                b = inq_buffer();
            }
            move_abs(atoi(l[4]), atoi(l[5]));
            set_top_left(atoi(l[6]));
        }
    }
    set_window(curwin);
    set_buffer(b);
    attach_buffer(b);
    zoom_list = NULL;
}

/*eof*/
