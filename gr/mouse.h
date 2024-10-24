#ifndef GR_MOUSE_H_INCLUDED
#define GR_MOUSE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_mouse_h,"$Id: mouse.h,v 1.25 2024/08/27 12:44:33 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mouse.h,v 1.25 2024/08/27 12:44:33 cvsuser Exp $
 * Mouse support.
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

/*  Definitions for where the mouse is on the screen
 *
 *    Window:
 *
 *            +-- SYSMENU                +--- REDUCE
 *            |                          | +- ZOOM
 *           ||+--------+- TITLE -+------+|+|
 *           +-+--------+---TOP---+-------+-+
 *           |                            +-+
 *      LEFT.|                            | |.. RIGHT
 *           |        INSIDE_WINDOW       | |
 *           |                            | |.. VSCROLL
 *           |                            +-+
 *           +-+------------------------+-+-+
 *           +-+----------BOTTOM--------+-+-+
 *                       - HSCROLL
 *
 *   Scroll bars:
 *
 *           +---+----------+-+---------+---+
 *           +-4-+---1------+2+---3-----+-5-+
 *
 *           1 - Left/top of thumb.
 *           2 - On the thumb marker.
 *           3 - Right/left of thumb.
 *           4 - Up marker.
 *           5 - Down marker.
 *
 */

enum {
/*--export--enum--*/
/*
 *  Mouse position
 */
    MOBJ_NOWHERE            = 0,                /* Not in any window */

    MOBJ_LEFT_EDGE          = 1,                /* Left bar of window */
    MOBJ_RIGHT_EDGE         = 2,                /* Right bar of window */
    MOBJ_TOP_EDGE           = 3,                /* Top line of window */
    MOBJ_BOTTOM_EDGE        = 4,                /* Bottom line of window */
    MOBJ_INSIDE             = 5,                /* Mouse inside window */
    MOBJ_TITLE              = 6,                /* On title */

    MOBJ_VSCROLL            = 20,               /* Vertical scroll area */
    MOBJ_VTHUMB             = 21,               /* Vertical scroll area */
    MOBJ_HSCROLL            = 22,               /* Horz scroll area */
    MOBJ_HTHUMB             = 23,               /* Horz scroll area */

    MOBJ_ZOOM               = 30,               /* Zoom button */
    MOBJ_CLOSE              = 31,               /* Close */
    MOBJ_SYSMENU            = 32                /* System Menu */
/*--end--*/
};

struct IOEvent;

struct MouseEvent {
    int b1, b2, b3;                             /* Button state */
    int multi;                                  /* Multiple button click's */
    int x, y;                                   /* Coordinates */
#define MOUSEEVENT_CSHIFT 1
#define MOUSEEVENT_CCTRL 2
#define MOUSEEVENT_CMETA 4
    int ctrl;                                   /* Optional Ctrl keys */
#define MOUSEEVENT_TPRESS 1                     // single press 
#define MOUSEEVENT_TRELEASE 2                   // signle release
#define MOUSEEVENT_TRELEASE_ALL 4               // release all buttons
#define MOUSEEVENT_TPRESSRELEASE 8              // button delta
#define MOUSEEVENT_TMOTION 16
#define MOUSEEVENT_TWHEELED 32
    int type;                                   /* Event, otherwise 0 if undefined */
};

extern int                  mouse_init(const char *dev);
extern void                 mouse_close(void);
extern int                  mouse_active(void);
extern void                 mouse_pointer(int state);
extern WINDOW_t *           mouse_pos(int x, int y, int *win, int *where);
extern int                  mouse_process(const struct MouseEvent *m, const char *seq);
extern void                 mouse_execute(const struct IOEvent *evt);

#if defined(HAVE_MOUSE)
extern int                  sys_mouseinit(const char *dev);
extern void                 sys_mouseclose(void);
extern void                 sys_mousepointer(int on);

extern void                 mouse_pointer(int state);
extern void                 mouse_draw_icon(int y, int x, int oldy, int oldx);
extern int                  mouse_poll(fd_set *fds);
extern int                  sys_mousepoll(fd_set *fds, struct MouseEvent *me);
#endif

extern unsigned             sys_doubleclickms(void);

extern void                 do_process_mouse(void);
extern void                 do_translate_pos(void);

#endif /*GR_MOUSE_H_INCLUDED*/

