#include <edidentifier.h>
__CIDENT_RCSID(gr_ruler_c,"$Id: ruler.c,v 1.9 2014/10/22 02:33:17 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ruler.c,v 1.9 2014/10/22 02:33:17 ayoung Exp $
 * Line ruler functionality.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>

#include "builtin.h"
#include "debug.h"
#include "line.h"
#include "ruler.h"

static int              x_tabchar = TRUE;       /* hard_tabs */

/*  Function:           tabchar_set
 *      Configure the Current tab character.
 *
 *  Parameters:
 *      tabs - Tab character.
 *
 *  Returns:
 *      nothing.
 */
void
tabchar_set(int tabs)
{
    x_tabchar = (tabs ? TRUE : FALSE);
}


/*  Function:           tabchar_get
 *      Query the Current tab character.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Current tab character.
 */
int
tabchar_get(void)
{
    return x_tabchar;
}


/*  Function:           ruler_next_stop
 *      Determine the stop, using the given 'ruler'.
 *
 *  Parameters:
 *      ruler - Ruler definition.
 *      column - Current column.
 *
 *  Returns:
 *      Column of the next ruler stop.
 *
 *  Note:
 *      It is assumed the ruler is a -1 terminated array of increasing column positions.
 */
int
ruler_next_stop(const LINENO *ruler, int column)
{
    register const LINENO *tp = ruler;
    int c;

    while (*tp >= 0 && *tp < column) {
        ++tp;                                   /* search ruler */
    }

    if (*tp >= 0) {
        c = *tp;

    } else {                                    /* no match, deal with exceptions */
        LINENO stop = 0;

        if (tp > (ruler + 1)) {                 /* end of ruler, use last elements */
            c = tp[-1], stop = tp[-1] - tp[-2];

        } else if (tp == (ruler + 1)) {         /* start of ruler or single value */
            c = stop = ruler[0];

        } else {                                /* no ruler */
            c = 0;
        }

        if (stop <= 0) stop = 8;                /* default 8 */
        while (c < column) {
            c += stop;                          /* derive */
        }
    }

    return c;
}


/*  Function:           ruler_next_tab
 *       Determine the next tab stop
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      column - Current column.
 *
 *  Returns:
 *      Column of the next tab stop.
 */
int
ruler_next_tab(const BUFFER_t *bp, int column)
{
    return ruler_next_stop(bp->b_tabs, column);
}


/*  Function:           ruler_next_indent
 *      Determine the next indent stop
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      column - Current column.
 *
 *  Returns:
 *      Column of the next indent.
 */
int
ruler_next_indent(const BUFFER_t *bp, int column)
{
    if (bp->b_ruler) {                          /* user defined ruler */
        return ruler_next_stop(bp->b_ruler, column);
    }

    if (bp->b_indent) {                         /* user defined indentation setting */
        LINENO indent;

        if (column-- <= 0) {
            return 1;
        }
        indent = column / bp->b_indent;
        return (indent + 1) * bp->b_indent;
    }

    return ruler_next_tab(bp, column);          /* (default) use tab settings */
}

/*end*/
