#ifndef GR_BUFFER_H_INCLUDED
#define GR_BUFFER_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_buffer_h,"$Id: buffer.h,v 1.22 2022/09/12 15:57:28 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: buffer.h,v 1.22 2022/09/12 15:57:28 cvsuser Exp $
 * Buffer management.
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

extern void                 buffer_init(void);
extern void                 buffer_shutdown(void);

extern int                  buf_isdirty(const BUFFER_t *bp);
extern BUFFER_t *           buf_first(void);
extern BUFFER_t *           buf_next(BUFFER_t *bp);
extern BUFFER_t *           buf_prev(BUFFER_t *bp);
extern BUFFER_t *           buf_last(void);

extern BUFFER_t *           buf_create(const char *name, const char *encoding, int aflag);
extern int                  buf_name(BUFFER_t *bp, char *fname);

extern BUFFER_t *           buf_lookup(int bufnum);
extern BUFFER_t *           buf_lookupx(int bufnum, const char *caller);
extern BUFFER_t *           buf_argument(int n);

extern int                  buf_kill(int n);
extern int                  buf_anycb(void);
extern BUFFER_t *           buf_find(const char *buffer_name);
extern BUFFER_t *           buf_find_or_create(const char *buffer_name);

extern BUFFER_t *           buf_find2(const char *buffer_name, int cflag, const char *encoding);

extern void                 buf_type_set(BUFFER_t *bp, int type);
extern void                 buf_type_default(BUFFER_t *bp);
extern void                 buf_encoding_set(BUFFER_t *bp, const char *encoding);

extern void                 buf_clear(BUFFER_t *bp);
extern void                 buf_show(BUFFER_t *bp, WINDOW_t *wp);
extern int                  buf_line_length(const BUFFER_t *bp, int marked);
extern void                 buf_change_window(WINDOW_t *wp);
extern int                  buf_imode(const BUFFER_t *bp);
extern void                 buf_mined(BUFFER_t *bp, LINENO start, LINENO end);
extern void                 buf_dirty(BUFFER_t *bp, LINENO start, LINENO end);

extern const char *         buf_type_desc(int type, const char *def);
extern const char *         buf_type_encoding(int type);
extern int                  buf_type_base(int type);
extern int                  buf_type_lookup(const char *encoding);
extern const char *         buf_termtype_desc(int type, const char *def);

extern void                 set_window_parms(WINDOW_t *wp, const BUFFER_t *bp);
extern void                 set_buffer_parms(BUFFER_t *bp, const WINDOW_t *wp);

extern int                  x_imode;            /* insert mode */

__CEND_DECLS

#endif /*GR_BUFFER_H_INCLUDED*/
