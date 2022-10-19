#ifndef GR_HILITE_H_INCLUDED
#define GR_HILITE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_hilite_h,"$Id: hilite.h,v 1.12 2022/08/10 15:44:56 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: hilite.h,v 1.12 2022/08/10 15:44:56 cvsuser Exp $
 * Hilite management.
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

typedef struct _hilite {
    MAGIC_t             h_magic;                /* Structure magic */
    TAILQ_ENTRY(_hilite)
                        h_node;                 /* List node, buffer specific */
#define HILITE_SEARCH           -2
#define HILITE_TRANSLATE        -3
#define HILITE_SYNTAX_MATCH     -4

    accint_t            h_type;                 /* Assigned type */
    accint_t            h_ident;                /* User assigned identifier */
    time_t              h_timeout;              /* Absolute timeout, in seconds */
    time_t              h_ctime;                /* Optional buffer last-change expiry */
    uint32_t            h_seqno;                /* Sequence number */
    uint32_t            h_attr;                 /* Attribute, otherwise 0 == HILITE */
    LINENO              h_sline;                /* Position */
    LINENO              h_scol;
    LINENO              h_eline;
    LINENO              h_ecol;
} HILITE_t;

extern void             hilite_attach(BUFFER_t *bp);
extern void             hilite_detach(BUFFER_t *bp);
extern int              hilite_expire(BUFFER_t *bp);
extern HILITE_t *       hilite_create(BUFFER_t *bp, int type, int32_t timeout, LINENO sline, LINENO scol, LINENO eline, LINENO ecol);
extern const HILITE_t * hilite_find(BUFFER_t *bp, const HILITE_t *cursor, LINENO line, LINENO col, vbyte_t *mark);
extern int              hilite_destroy(BUFFER_t *bp, int type);
extern int              hilite_delete(BUFFER_t *bp, int seqno);
extern int              hilite_clear(BUFFER_t *bp, LINENO line);
extern int              hilite_zap(BUFFER_t *bp, int update);

__CEND_DECLS

#endif /*GR_HILITE_H_INCLUDED*/
