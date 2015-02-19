#ifndef GR_ANCHOR_H_INCLUDED
#define GR_ANCHOR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_anchor_h,"$Id: anchor.h,v 1.20 2014/10/22 02:32:52 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: anchor.h,v 1.20 2014/10/22 02:32:52 ayoung Exp $
 * Buffer anchor management.
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

#include <edtypes.h>

__CBEGIN_DECLS

typedef struct {
    int                 type;                   /* Mark type (MK_xxxx)*/
    LINENO              start_line;             /* Line references */
    LINENO              end_line;
    LINENO              start_col;              /* Column references */
    LINENO              end_col;
} ANCHOR_t;

extern void                 anchor_attach(BUFFER_t *bp);
extern void                 anchor_detach(BUFFER_t *bp);
extern void                 anchor_drop(int type);
extern int                  anchor_raise(void);
extern int                  anchor_adjust(int ins);
extern void                 anchor_zap(BUFFER_t *bp);
extern int                  anchor_get(WINDOW_t *wp, BUFFER_t *bp, ANCHOR_t *anchor);
extern void                 anchor_read(BUFFER_t *bp, ANCHOR_t *anchor);
extern void                 anchor_write(BUFFER_t *bp, const ANCHOR_t *anchor);
extern void                 anchor_dump(void);

extern void                 do_drop_anchor(void);
extern void                 do_end_anchor(void);
extern void                 do_mark(void);
extern void                 do_raise_anchor(void);
extern void                 do_swap_anchor(void);
extern void                 inq_marked(void);
extern void                 inq_marked_size(void);

__CEND_DECLS

#endif /*GR_ANCHOR_H_INCLUDED*/
