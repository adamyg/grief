/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: johnd.cr,v 1.8 2014/10/22 02:34:37 ayoung Exp $
 * User configuration.
 *
 *
 */

#include "../grief.h"

void
johnd(void)
{
    save_state();                               /* turn on full state saving */

    assign_to_key("\x1b,", "objects word_left");
    assign_to_key("\x1b.", "objects word_right");

    load_macro("scrollfixed");                  /* scroll without cursor movement */
    load_macro("short");
}


void
_highlight_colors(void)                         /* default syntax colors */
{
    set_color_pair("string",         "light-green",   "blue");
    set_color_pair("operator",       "light-cyan",    "blue");
    set_color_pair("number",         "light-green",   "blue");
    set_color_pair("comment",        "magenta",       "blue");
    set_color_pair("preprocessor",   "cyan",          "blue");
    set_color_pair("delimiter",      "light-cyan",    "blue");
    set_color_pair("keyword",        "yellow",        "blue");
    set_color_pair("keyword1",       "yellow",        "blue");
    set_color_pair("keyword2",       "yellow",        "blue");
}
