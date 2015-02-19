/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: os2mouse.h,v 1.4 2012/06/21 22:51:13 cvsuser Exp $
 * support a mouse under os/2
 *
 *
 */

#include "../crunch.h"

#define MOBJ_LEFT_EDGE      1
#define MOBJ_RIGHT_EDGE     2
#define MOBJ_TOP_EDGE       3
#define MOBJ_BOTTOM_EDGE    4
#define MOBJ_INSIDE         5
#define MOBJ_TITLE	    6

int	old_b1		    = 0,
	old_b2		    = 0,
	old_b3		    = 0;
int	old_x		    = -1,
	old_y		    = -1,
	old_w		    = -1;
int	mouse_cnt	    = 0;


void
button_1_down(int winid, int where)
{
    int curwin = inq_window();

    switch (where) {
    case MOBJ_LEFT_EDGE:
	break;
    case MOBJ_RIGHT_EDGE:
	break;
    case MOBJ_TOP_EDGE:
	break;
    case MOBJ_BOTTOM_EDGE:
	break;
    case MOBJ_INSIDE:
	if (curwin != winid) {
	    set_window(winid);
	    set_buffer(inq_buffer(winid));
	    refresh();
	}
	break;
    case MOBJ_TITLE:
	break;
    }
}


void
button_2_down()
{
}


void
mouse(int winid, int where, int x, int y, int b1, int b2, int b3)
{
    ++mouse_cnt;
    message("mouse X=%d Y=%d %c%c%c", x, y, b1 + '0', b2 + '0', b3 + '0');
    if (!old_b1 && b1) {
	button_1_down(winid, where);
    }
    old_w  = winid;
    old_x  = x;
    old_y  = y;
    old_b1 = b1;
    old_b2 = b2;
    old_b3 = b3;
}


