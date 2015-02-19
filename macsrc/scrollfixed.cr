/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: scrollfixed.cr,v 1.3 2014/10/27 23:28:27 ayoung Exp $
 * Scroll with reduced cursor movement
 *
 *
 */

#include "grief.h"

static void             scrolldown(void);
static void             scrollup(void);
static void             scrollup_cursordown(void);
static void             scrolldown_cursorup(void);


void
main(void)
{
    assign_to_key("<Ctrl-Up-Arrow>",    "::scrollup");
    assign_to_key("<Ctrl-Down-Arrow>",  "::scrolldown");
    assign_to_key("<Alt-Up-Arrow>",     "::scrollup_cursordown");
    assign_to_key("<Alt-Down-Arrow>",   "::scrolldown_cursorup");
}


/*
 *  scrolldown ---
 *      Move down one line, retaining the cursors current screen position.
 */
static void
scrolldown(void)
{
    int line1, line2, top;

    inq_position(line1);
    down();
    inq_position(line2);
    if (line1 != line2) {
      inq_top_left(top);
      set_top_left(top+1);
    }
}


/*
 *  scrollup ---
 *      Move up one line, retaining the cursors current screen position.
 */
static void
scrollup(void)
{
    int line1, line2, top;

    inq_position(line1);
    up();
    inq_position(line2);
    if (line1 != line2) {
        inq_top_left(top);
        set_top_left(top-1);
    }
}


static void
scrollup_cursordown(void)
{
   int topline, cursorline, windowsize;

   inq_top_left(topline);
   if (topline > 1) {
      inq_position(cursorline);
      inq_window_size(windowsize);
      if (cursorline == topline + (windowsize-1)) {
         up();
      }
      set_top_left(topline-1);
   }
}


static void
scrolldown_cursorup(void)
{
    int topline, cursorline;

    inq_top_left(topline);
    inq_position(cursorline);
    if ( cursorline == topline ) {
        down();
    }
    set_top_left(topline+1);
}

/*end*/
