#ifndef GR_WIDGETS_H_INCLUDED
#define GR_WIDGETS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_widgets_h,"$Id: widgets.h,v 1.9 2024/09/08 16:29:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: widgets.h,v 1.9 2024/09/08 16:29:24 cvsuser Exp $
 * Standard widgets.
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

extern WIDGET_t *           group_new(void);
extern WIDGET_t *           container_new(void);
extern WIDGET_t *           tab_new(void);

extern WIDGET_t *           spacer_new(void);
extern WIDGET_t *           separatorh_new(void);
extern WIDGET_t *           separatorv_new(void);
extern WIDGET_t *           checkbox_new(void);
extern WIDGET_t *           combofield_new(void);
extern WIDGET_t *           editfield_new(void);
extern WIDGET_t *           numericfield_new(void);
extern WIDGET_t *           gauge_new(void);
extern WIDGET_t *           label_new(void);
extern WIDGET_t *           listbox_new(void);
extern WIDGET_t *           pushbutton_new(void);
extern WIDGET_t *           radiobutton_new(void);

extern WIDGET_t *           menu_new(void);
extern WIDGET_t *           menu_item_new(void);
extern WIDGET_t *           menu_separator_new(void);

__CEND_DECLS

#endif /*GR_WIDGETS_H_INCLUDED*/
