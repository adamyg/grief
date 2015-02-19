/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: comment.cr,v 1.11 2014/10/27 23:28:19 ayoung Exp $
 * C/C++ block comment/uncomment.
 *
 *
 */

#include "grief.h"
#include "mode.h"

#if defined(__PROTOTYPES__)
static void             _comment_list(string type);
#endif


/*
 *  comment_block ---
 *      Comment out the marked block.
 */
void
comment_block(void)
{
    string macro_name, mode;

    if (inq_marked()) {
        mode = _mode_pkg_get();
        macro_name = mode + "_comment_block";   /* handler */
        if (inq_macro(macro_name) > 0) {
            execute_macro(macro_name);
        } else {
            _comment_list("comment");           /* list available */
        }
    } else {
        error("No marked block.");
    }
}


/*
 *  uncomment_block ---
 *      Remove the comments from the marked block.
 */
void
uncomment_block(void)
{
    string macro_name, mode;

    if (inq_marked()) {
        mode = _mode_pkg_get();
        macro_name = mode + "_uncomment_block"; /* handler */
        if (inq_macro(macro_name) > 0) {
            execute_macro(macro_name);
        } else  {
            _comment_list("uncomment");         /* list available */
        }
    } else {
        error("No marked block.");
    }
}


/*
 *  _comment_list ---
 *      If no routines available for current file, then generate a list of all
 *      (un)comment macros and let him have a go at these.
 *
 */
static void
_comment_list(string type)
{
    list     macs = macro_list();
    list     rtn_list;
    string   what;
    int      entry = 0;

    what = "_" + type + "_block$";
    while (1) {
        entry = re_search(NULL, what, macs, entry);
        if (entry < 0) {
            break;
        }
        entry++;
    }
    entry = select_list(type + " macros", "Select function", 1, rtn_list, SEL_CENTER, NULL);
    if (entry >= 1) {
        execute_macro( rtn_list[entry-1] );
    }
}

/*eof*/
