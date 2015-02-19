/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: window.cr,v 1.10 2014/10/27 23:28:30 ayoung Exp $
 * Window support.
 *
 *
 */


/*
 *  Macro to go to the left edge of the current window (because
 *  it may be indented).
 */
void
goto_left_edge(void)
{
    int shift;

    inq_window_size(NULL, NULL, shift);
    move_abs(0, shift + 1);
}


/*
 *  Macro to go to the right edge of the curent window (because
 *  it may be indented).
 */
void
goto_right_edge(void)
{
    int shift, cols;

    inq_window_size(NULL, cols, shift);
    move_abs(0, shift + cols);
}


void
set_top_of_window(void)
{
    int cur_line;

    inq_position(cur_line);
    set_top_left(cur_line);
}


/*
 *  Macro to move current line to bottom of window if its possible. If buffer size < size
 *  of window, then we make first line of buffer at top of window.
 */
void
set_bottom_of_window(void)
{
    int cur_line;

    inq_position(cur_line);

    if (inq_lines() <= inq_window_size()) {
        set_top_left(1);
    } else {
        set_top_left(cur_line - inq_window_size() + 1);
    }
}


/*
 *  Macro to move current line to center of window (if possible).
 */
void
set_center_of_window(void)
{
    int cur_line;

    inq_position(cur_line);
    set_top_left(cur_line - inq_window_size() / 2);
}


/*
 *  This macro may be used to change the title of an Xterm or SunView window by sending
 *  an appropriate escape sequence to the screen.
 *
 *      FIXME - move into ./tty.
 */
void
set_win_label(string mess)
{
    string grterm = getenv("GRTERM");

    if (substr(grterm, 1, 5) == "xterm") {
        printf("\033]2;%s\007", mess);
    } else {
        printf("\033]l%s\033\134", mess);
    }
}


/*
 *  Routine to set the title of an icon. Can be used with Xterms and Sunview
 *  compatible windows.
 *
 *      FIXME - move into ./tty.
 */
void
set_icon_label(string mess)
{
    string grterm = getenv("GRTERM");

    if (substr(grterm, 1, 5) == "xterm") {
        printf("\033]1;%s\007", mess);
    } else {
        printf("\033]L%s\033\134", mess);
    }
}

/*eof*/
