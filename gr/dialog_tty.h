#ifndef GR_DIALOG_TTY_H_INCLUDED
#define GR_DIALOG_TTY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_dialog_tty_h,"$Id: dialog_tty.h,v 1.10 2020/04/21 00:01:55 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dialog_tty.h,v 1.10 2020/04/21 00:01:55 cvsuser Exp $
 * Dialog manager, TTY specific functionality
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

extern void                 dialog_tty_run(DIALOG_t *d);
extern WIDGET_t            *dialog_tty_current(DIALOG_t *d);
extern int                  dialog_tty_setfocus(DIALOG_t *d, WIDGET_t *w);

extern int                  dialog_tty_popup_create(DIALOG_t *d, const WIDGET_t *w, int rows, int cols, const char *msg);
extern int                  dialog_tty_popup_focus(DIALOG_t *d, int state);
extern int                  dialog_tty_popup_select(DIALOG_t *d, int state);
extern int                  dialog_tty_popup_close(DIALOG_t *d);

__CEND_DECLS

#endif /*GR_DIALOG_TTY_H_INCLUDED*/