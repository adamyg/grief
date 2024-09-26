#ifndef GR_WIDGETS_TTY_H_INCLUDED
#define GR_WIDGETS_TTY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_widgets_tty_h,"$Id: widgets_tty.h,v 1.11 2024/09/25 13:58:06 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: widgets_tty.h,v 1.11 2024/09/25 13:58:06 cvsuser Exp $
 * Widgets, TTY specific functionality.
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

#include <edsym.h>

__CBEGIN_DECLS

    /*  The SETFOCUS message is sent to a widget to determine if it requires
     *  the focus.
     */
#define WIDGET_SETFOCUS     (WIDGET_BASE+0)

    /*  The PAINT message is sent to request the widget content is updated.
     */
#define WIDGET_PAINT        (WIDGET_BASE+1)

    /*  The CARET message is sent to request whether the widget wish to own
     *  the caret.
     */
#define WIDGET_CARET        (WIDGET_BASE+2)

    /*  The KEYDOWN message is posted to the widget with the keyboard
     *  focus when a nonsystem key is pressed. A nonsystem key is a key
     *  that is pressed when the ALT key is not pressed.
     *
     *  An application should return non-zero if it processes this message.
     */
#define WIDGET_KEYDOWN      (WIDGET_BASE+3)

    /*  The HOTKEY message is sent when the focused widget has failed to
     *  process a key press, allowing all other widgets a change at processing
     *  the key stroke.
     */
#define WIDGET_HOTKEY       (WIDGET_BASE+4)

    /*  The COMMAND message is sent when the user selects a command item from
     *  a menu, when a control sends a notification message to its parent
     *  window, or when an accelerator keystroke is translated.
     */
#define WIDGET_COMMAND      (WIDGET_BASE+5)

    /*  The CLOSE message is sent as a signal that a widget should terminate.
     */
#define WIDGET_CLOSE        (WIDGET_BASE+7)

    /*  Input IDLE signal, sent ~1 second to the focused widget.
     */
#define WIDGET_IDLE         (WIDGET_BASE+8)

    /*  The MOUSE message is posted to a dialog when the cursor moves, the
     *  message is posted to the widget that contains the cursor. Otherwise,
     *  the message is posted to the dialog.
     *
     *  p1 -    Cursor coordinates.
     *
     *          The low-order word specifies the x-coordinate of the cursor.
     *          The coordinate is relative to the upper-left corner of the
     *          client area.
     *
     *          The high-order word specifies the y-coordinate of the cursor.
     *          The coordinate is relative to the upper-left corner of the
     *          client area.
     *
     *          Use the following code to obtain the horizontal and vertical
     *          position:
     *
     *              x = GetXParam(p1);
     *              y = GetYParam(p2);
     *
     *  p2 -    Event.
     *
     *          Indicates the mouse event which occurr, represented by one
     *          of the BUTTON_ keys manifest values.
     */
#define WIDGET_MOUSE        (WIDGET_BASE+9)
#define WIDGET_MOUSE_POPUP  (WIDGET_BASE+10)

    /*  Geometry support --
     *      The SIZE message is sent to a widget that the user is resizing. By
     *      processing this message, an application can monitor the size and
     *      position of the drag rectangle and, if needed, change its size or
     *      position.
     */
#define WIDGET_SIZE         (WIDGET_BASE+11)
#define WIDGET_BORDER       (WIDGET_BASE+12)
#define WIDGET_RESIZED      (WIDGET_BASE+13)


    /*  Widget flags
     *
     *  FOCUS ---
     *      Whether the widget has the focus.
     *
     *  HIDDEN ---
     *      Whether the widget has been hidden from view.
     *
     *  TABSTOP ---
     *      Specifies a control that can receive the keyboard focus when the
     *      user presses the TAB key. Pressing the TAB key changes the
     *      keyboard focus to the next control with the TABSTOP style. You
     *      can turn this style on and off to change dialog box navigation.
     *
     *  DONTPROPAGATE ---
     *      Dont propagate widget resizes to the parent.
     *
     *  REPACK ---
     *      A repack is required.
     *
     *  RESIZE ---
     *      A resize is required.
     *
     *  DIRTY ---
     *      Widget is dirty, post a set property.
     *
     *  CLEAR ---
     *      Widget was hidden and the underlying area needs clearing.
     */
#define WTTY_FFOCUS         0x0001
#define WTTY_FHIDDEN        0x0002

#define WTTY_FREPACK        0x0010
#define WTTY_FRESIZE        0x0020

#define WTTY_FDIRTY         0x0100
#define WTTY_FCLEAR         0x0200

#define ClrFocus(w)         w->w_uflags &= ~WTTY_FFOCUS
#define HasFocus(w)         (((WIDGET_t *)(w))->w_uflags & WTTY_FFOCUS)

#define GetXParam(p)        DIALOGARGLO(p)
#define GetYParam(p)        DIALOGARGHI(p)

extern WIDGET_t *           tty_new(uint32_t size, WIDGETCB_t handler);

__CEND_DECLS

#endif /*GR_WIDGETS_TTY_H_INCLUDED*/

