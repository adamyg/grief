/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: options.cr,v 1.15 2014/10/27 23:28:25 ayoung Exp $
 * Options interface.
 *
 *
 */

#include "grief.h"


static list         x_option_list = {
    "Options",              "Options",              /* title,[help topic] */
    "Status Line",          "echo_line_options",
    "Tab/Indent",           "indent_options",
    "Document",             "wp_options",           /* see: wp.cr */
    "Searching",            "search_options",       /* see: search.cr */
    "Buffer information",   "bufinfo",              /* see: bufinfo.cr */
    "Window information",   "wininfo",              /* see: wininfo.cr */
    "Display information",  "dispinfo",             /* see: dispinfo.cr */
    "Buffer flags",         "toggle_buffer_flags",  /* see: extra.cr */
    "Buffer type",          "toggle_buffer_type",   /* see: extra.cr */
    };


static list         x_echoline_options = {
    "Time            (HH:MMxm)   : ", {"On", "Off"},
    " as 24 hr clock (HH:MM)     : ", {"On", "Off"},
    "Column          (Col: xxx)  : ", {"On", "Off"},
    "Line            (Line: xxx) : ", {"On", "Off"},
    "File percent    (xx%)       : ", {"On", "Off"},
    "Remember status (RE)        : ", {"On", "Off"},
    "Overtype mode   (OV)        : ", {"On", "Off"},
    "Virtual character indicator : ", {"On", "Off"},
    "Character Value             : ", {"On", "Off"},
    "Format override             : ", {"On", "Off"},
    };


static list         x_indent_options = {
    "Fill with   : ", {"Spaces", "Tabs"},
    "Tab         : ", "",
    "Tabs        : ", "",
    "Indentation : ", "",
    "Ruler       : ", ""
    };


void
options(void)
{
    while (select_feature(x_option_list, 20) >= 0) {
        /*continue*/;
    }
}


void
echo_line_options(void)
{
    list results;
    int options, new_options;

    options = inq_echo_line();
    results[0] = (E_TIME      & options) == 0;
    results[1] = (E_TIME24    & options) == 0;
    results[2] = (E_LINE      & options) == 0;
    results[3] = (E_COL       & options) == 0;
    results[4] = (E_PERCENT   & options) == 0;
    results[5] = (E_REMEMBER  & options) == 0;
    results[6] = (E_CURSOR    & options) == 0;
    results[7] = (E_VIRTUAL   & options) == 0;
    results[8] = (E_CHARVALUE & options) == 0;
    results[9] = (E_FORMAT    & options) == 0;
    results = field_list("Echo-Line Options", results, x_echoline_options, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        message("No changes ...");
        return;                                 // <Esc>
    }
    new_options = (options & E_FROZEN);         // retain FROZEN
    if (0 == results[0]) new_options |= E_TIME;
    if (0 == results[1]) new_options |= E_TIME24;
    if (0 == results[2]) new_options |= E_LINE;
    if (0 == results[3]) new_options |= E_COL;
    if (0 == results[4]) new_options |= E_PERCENT;
    if (0 == results[5]) new_options |= E_REMEMBER;
    if (0 == results[6]) new_options |= E_CURSOR;
    if (0 == results[7]) new_options |= E_VIRTUAL;
    if (0 == results[8]) new_options |= E_CHARVALUE;
    if (0 == results[9]) new_options |= E_FORMAT;
    if (new_options != options) {
        echo_line(new_options);
    }
}


void
indent_options(void)
{
    list results;

    results[0] = use_tab_char(-1);              // Spaces or Tab's
    results[1] = "" + inq_tab();                // Tab setting
    results[2] = inq_tabs(NULL, 2);             // Tabs ruler
    results[3] = "" + inq_indent();             // Indentation
    results[4] = inq_ruler(NULL, 2);            // Indentation ruler

    results = field_list("Tab/Indent Options", results, x_indent_options, TRUE, TRUE);
    if (length_of_list(results) <= 0) {
        return;
    }

    use_tab_char(0 == results[0] ? "n" : "y");
    if (atoi(results[1]) != inq_tab()) {
        set_tab(atoi(results[1]));
    } else {
        tabs(results[2]);
    }
    set_indent(atoi(results[3]));
    set_ruler(NULL, results[4]);
}

/*end*/
