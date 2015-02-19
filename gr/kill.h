#ifndef GR_KILL_H_INCLUDED
#define GR_KILL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_kill_h,"$Id: kill.h,v 1.13 2014/10/22 02:32:59 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kill.h,v 1.13 2014/10/22 02:32:59 ayoung Exp $
 * Scrape buffer management.
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

extern void                 k_init(BUFFER_t *bp);
extern BUFFER_t *           k_set(BUFFER_t *bp);
extern int                  k_isscrap(const BUFFER_t *bp);
extern void                 k_delete(int n);
extern void                 k_write(const char *buf, int cnt);
extern int                  k_type(void);
extern int                  k_insnewline(void);
extern void                 k_newline(void);
extern void                 k_seek(void);
extern int                  k_read(const char **cpp);
extern int                  k_numlines(void);
extern void                 k_undo(void);
extern void                 k_end(void);

extern void                 inq_scrap(void);
extern void                 do_set_scrap_info(void);

__CEND_DECLS

#endif /*GR_KILL_H_INCLUDED*/
