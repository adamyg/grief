/* $Id: scrblank.cr,v 1.8 2014/10/27 23:28:26 ayoung Exp $
 * Screen blank utility.
 *
 *
 */

#include "grief.h"

#define BLANK_TIMEOUT      10                   // Blank idle time, in minutes.

#define TIMEOUT            20000                // Time (in msec) between displaying pattern in screen blank window.

#define BLACK              0
#define DARK_GREY          8

static int                 gri_scrblank;        // blankout time setting (< 0 disabled).


void
scrblank(string arg)
{
   int i;

   if (first_time()) {
      register_macro(REG_IDLE, "::screen_blank_idle");
   }
   i = atoi(arg);
   gri_scrblank = (i ? i : BLANK_TIMEOUT);
}


static void
screen_blank_idle(void)
{
   /*
    *  Make sure we've been idle for the time specified before going ahead
    *  and blanking screen.
    */
   if (inq_idle_time() && inq_idle_time() < gri_scrblank * 60) {
      return;
   }
   screen_blank();
}


/*
 *    Actual function to blank the screen.
 *
 *    Separate function allowing testing without needing to wait for an idle timeout.
 */
void
screen_blank(void)
{
   int curwin, curbuf;
   int buf;
   int win;
   int lines, cols;
   int echo_status;
   int background, normal, selected, messages, errors, hi_fg, hi_bg, frame;
   int i = 0;
   int color_screen;
   string esc;

   /* Save colors so we can go for a darker one. */
   inq_color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);

   curwin = inq_window();
   curbuf = inq_buffer();

   /* Save status of echo line flags and clear them from the screen at the same time. */
   echo_status = echo_line(0);

   /* Create a window which overlaps the whole screen, so we can play inside it. */
   color_screen = inq_screen_size(lines, cols);
   buf = create_buffer("Blanker", NULL, TRUE);
   win = create_window(0, lines - 1, cols - 1, 0, NULL);
   set_window(win);
   attach_buffer(buf);
   set_buffer_flags(NULL, BF_ANSI);

   /* Wait for key strokes, and every now and again do something interesting. */
   color(BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK);
   cursor(0);

   do {
      clear_buffer();
      if ((i = ++i % 8) == 0) {
         ++i;
      }
      if (color_screen) {
         sprintf(esc, "\x1b[%dm", 30 + i);      /* FIXME - buffer attributes */
      }
      insert("\n", rand(lines - 2));
      insert(" ", rand(cols - 22));
      insert(esc + "GRIEF Screen Blanker");
   } while (read_char(TIMEOUT) == -1);

   cursor(1);
   delete_window(win);
   delete_buffer(buf);
   set_window(curwin);
   set_buffer(curbuf);
   attach_buffer(curbuf);
   echo_line(echo_status);

   color(background, normal, selected, messages, errors, hi_bg, hi_fg, frame);
}

/*end*/
