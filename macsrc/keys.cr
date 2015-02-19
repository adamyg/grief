/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: keys.cr,v 1.6 2014/10/27 23:28:23 ayoung Exp $
 * Miscellaneous definitions for keyboard mappings
 *
 *
 */

#include "grief.h"


/*
 *  _back_tab ---
 *      Moves back to the previous tab stop. Deals with the appropriate
 *      special cases as well.
 */
void
_back_tab(void)
{
    int col, start_col, start_dist;

    start_dist = distance_to_indent();
    inq_position(NULL, start_col);

    /* If we were on a tab stop to start with, we back up two
     * characters. Otherwise, just one
     */
    move_rel(0, -1);
    if (distance_to_indent() < start_dist)
        move_rel(0, -1);
    while ((distance_to_indent() != 1) && move_rel(0, -1))
        ;

    /* If the last place we were was the first column, we just stay
     * there. Also, if there were any one space tabs, we know that
     * we should not move back to the original tab stop.
     */
    inq_position(NULL, col);
    if ((col != 1) && ((start_col - col) != 1))
        move_rel(0, 1);
}



/*
 *  _open_line ---
 *      Adds a blank line after the current line, placing the cursor on the
 *      first column of the new line. The normal macro for the Enter key is
 *      used to perform the function after the end of line.
 */
void
_open_line(void)
{
    string enter_macro = inq_assignment("<Enter>");

    end_of_line();
    if (enter_macro == "self_insert" || enter_macro == "_open_line")
        insert("\n");
    else
        execute_macro(enter_macro);
}

/*end*/
