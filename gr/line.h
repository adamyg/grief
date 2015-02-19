#ifndef GR_LINE_H_INCLUDED
#define GR_LINE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_line_h,"$Id: line.h,v 1.25 2014/11/16 17:28:38 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: line.h,v 1.25 2014/11/16 17:28:38 ayoung Exp $
 * Line management.
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

extern void                 line_init(void);
extern LINE_t *             line_new(void);
extern LINE_t *             line_alloc(LINENO used, int hasattrs, int incore);
extern void                 line_free(LINE_t *lp);
extern void                 line_move(LINE_t *lp, LINENO dst, LINENO src, LINENO len);
extern void                 line_set(LINE_t *lp, LINENO dst, const char *src, LINENO len, LINEATTR attr, LINENO cnt);
extern void                 line_write(LINE_t *lp, LINENO dst, const char *src, LINENO len, LINEATTR attr);
extern void                 line_copy(LINE_t *lp, LINENO dst, const LINE_t *slp, LINENO src, LINENO len);

extern int                  ledit(LINE_t *lp, LINENO size);
extern int                  lsize(LINE_t *lp, LINENO size);
extern int                  lexpand(LINE_t *lp, LINENO dot, LINENO size);
extern void                 lremove(BUFFER_t *buf, LINENO line);
extern void                 lrelease(BUFFER_t *buf, LINE_t *lp, LINENO line);
extern void                 lchange(int flag, LINENO count);
extern int                  llinepad(void);
extern void                 lrenumber(BUFFER_t *bp);

extern int                  linsertc(int ch);
extern int                  linserts(const char *cp, int cnt);
extern int                  linsert(const char *cp, LINENO len, int nl);
extern int                  lwritec(int ch);
extern int                  lwrite(const char *str, LINENO len, int width);
extern void                 ldeletec(int cnt);
extern void                 ldelete(FSIZE_t cnt);
extern void                 lnewline(void);

extern int                  lreplacedot(const char *str, int ins, int del, int dot, int *edot);
extern void                 ldeletedot(LINENO length, int dot);

extern LINE_t *             x_static_line;

__CEND_DECLS

#endif /*GR_LINE_H_INCLUDED*/
