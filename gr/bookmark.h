#ifndef GR_BOOKMARK_H_INCLUDED
#define GR_BOOKMARK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_bookmark_h,"$Id: bookmark.h,v 1.11 2014/10/22 02:32:53 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: bookmark.h,v 1.11 2014/10/22 02:32:53 ayoung Exp $
 * Bookmarks.
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

extern void                 bookmark_init(void);
extern void                 bookmark_shutdown(void);

extern void                 do_drop_bookmark(void);
extern void                 do_delete_bookmark(void);
extern void                 do_goto_bookmark(void);
extern void                 do_bookmark_list(void);

__CEND_DECLS

#endif /*GR_BOOKMARK_H_INCLUDED*/
