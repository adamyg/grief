#include <edidentifier.h>
__CIDENT_RCSID(gr_hilite_c,"$Id: hilite.c,v 1.18 2022/08/10 15:44:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: hilite.c,v 1.18 2022/08/10 15:44:56 cvsuser Exp $
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

#include <editor.h>

#include "buffer.h"                             /* buf_argument() */
#include "builtin.h"                            /* cur_line/cur_col */
#include "color.h"
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */
#include "hilite.h"
#include "main.h"                               /* curbp */
#include "window.h"

#define HILITE_MAGIC    MKMAGIC('H','i','L','t')

static uint32_t         x_hilite_seqno;

/*  Function:           hilite_attach
 *      Buffer hilite management runtime initialiation.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing.
 */
void
hilite_attach(BUFFER_t *bp)
{
    TAILQ_INIT(&bp->b_hilites);
}


/*  Function:           hilite_detach
 *      Buffer hilite management runtime shutdown.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing.
 */
void
hilite_detach(BUFFER_t *bp)
{
    hilite_zap(bp, FALSE);
}


/*  Function:           hilite_create
 *      Create a region hilite object.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      type - Associated line.
 *      timeout - Timeout in seconds; -1 on next buffer change.
 *      sline - Start line.
 *      scol - Start column.
 *      eline - End line.
 *      ecol - End column.
 *
 *  Returns:
 *      Region hilite object.
 */
HILITE_t *
hilite_create(BUFFER_t *bp, int type, int32_t timeout,
        LINENO sline, LINENO scol, LINENO eline, LINENO ecol)
{
    HILITE_t *hp = NULL;

    if (bp && NULL != (hp = (HILITE_t *) chk_calloc(sizeof(HILITE_t),1))) {
        /*
         *  allocate and queue hilite object.
         */
        HILITELIST_t *hilites = &bp->b_hilites;
        const int first = (NULL == TAILQ_FIRST(hilites));
        register LINENO tmp;

        hp->h_magic     = HILITE_MAGIC;
        hp->h_type      = type;
        hp->h_timeout   = timeout;
        if (hp->h_timeout > 0) {
            hp->h_timeout += (time(NULL) - 1); // expire time.
        } else if (-1 == timeout) {
            hp->h_ctime = bp->b_ctime; // expire on buffer change.
        }
        hp->h_seqno     = ++x_hilite_seqno;

        if (sline > eline) {
            GR_SWAP(sline, eline, tmp);
        }
        if (scol > ecol) {
            GR_SWAP(scol, ecol, tmp);
        }

        hp->h_sline     = sline;
        hp->h_scol      = scol;
        hp->h_eline     = eline;
        hp->h_ecol      = ecol;

        buf_mined(bp, sline, eline);

        trace_log("hilite_create(type:%d, start:%d/%d, end/%d/%d, attr:%d\n",
            (int)hp->h_type, sline, scol, eline, ecol, hp->h_attr);

        if (first) {                            /* sort by line/col */
            TAILQ_INSERT_TAIL(hilites, hp, h_node);
        } else {
            HILITE_t *chp;

            TAILQ_FOREACH(chp, hilites, h_node) {
                assert(HILITE_MAGIC == chp->h_magic);
                if (hp->h_sline < chp->h_sline ||
                        (hp->h_sline == chp->h_sline && hp->h_scol <= chp->h_scol)) {
                    TAILQ_INSERT_BEFORE(chp, hp, h_node);
                    hp = NULL;
                    break;
                }
            }
            if (hp) {
                TAILQ_INSERT_TAIL(hilites, hp, h_node);
            }
        }

        if (bp == curbp) {                      /* redraw event */
            win_modify(WFEDIT);
        }
    }
    return hp;
}


/*  Function:           hilite_expire
 *      Expire hilite elements for the given buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Number of elements expired.
 */
int
hilite_expire(BUFFER_t *bp)
{
    HILITELIST_t *hilites = &bp->b_hilites;
    const time_t now = time(NULL);
    HILITE_t *hp;
    int ret = 0;

    hp = TAILQ_FIRST(hilites);
    while (hp) {
        HILITE_t *next = TAILQ_NEXT(hp, h_node);

        assert(HILITE_MAGIC == hp->h_magic);
        if ((hp->h_timeout > 0 && hp->h_timeout < now) ||
                (hp->h_ctime && hp->h_ctime != bp->b_ctime)) {
            TAILQ_REMOVE(hilites, hp, h_node);
            buf_mined(bp, hp->h_sline, hp->h_eline);
            chk_free((void *)hp);
            ++ret;
        }
        hp = next;
    }
    return ret;
}


/*  Function:           hilite_find
 *      Search the buffer hilite list for the next or active region.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      current - Current region, if any.
 *      line - Line number.
 *      col - Column number.
 *      marked -  Storage populated with the marked attribute, otherwise -1.
 *
 *  Returns:
 *      Next/active region, otherwise NULL.
 */
const HILITE_t *
hilite_find(BUFFER_t *bp, const HILITE_t *current, LINENO line, LINENO col, vbyte_t *marked)
{
    HILITELIST_t *hilites = &bp->b_hilites;
    const HILITE_t *cursor;

    if (NULL == (cursor = current)) {
        hilite_expire(bp);
        cursor = TAILQ_FIRST(hilites);
    }

    while (cursor) {
        assert(HILITE_MAGIC == cursor->h_magic);
        if (line < cursor->h_sline ||           /* next hilite (left side) */
                (line == cursor->h_sline && col < cursor->h_scol)) {
            return cursor;
        }

        if (line < cursor->h_eline ||           /* inside hilite */
                (line == cursor->h_eline && col <= cursor->h_ecol)) {
            if (marked) {
                const HILITE_t *next;
                                                /* 0 == normal, otherwise attribute */
                *marked = (vbyte_t)cursor->h_attr;

                                                /* interleaving/overlapping regions? */
                if (NULL != (next = TAILQ_NEXT(cursor, h_node))) {
                    do {
                        assert(HILITE_MAGIC == next->h_magic);
                        if (line < next->h_sline ||
                                (line == next->h_sline && col < next->h_scol)) {
                            break;
                        }

                        if (line < next->h_eline ||
                                (line == next->h_eline && col <= next->h_ecol)) {
                            if (next->h_eline > cursor->h_eline ||
                                    (next->h_eline == cursor->h_eline && next->h_ecol > cursor->h_ecol)) {
                                cursor = next;  /* ends after parent */
                            }
                            *marked = (vbyte_t)next->h_attr;
                        }
                    } while (NULL != (next = TAILQ_NEXT(next, h_node)));
                }
            }
            return cursor;
        }
        cursor = TAILQ_NEXT(cursor, h_node);    /* next hilite (right side) */
    }

    if (marked) *marked = (vbyte_t)-1;
    return NULL;
}


int
hilite_destroy(BUFFER_t *bp, int type)
{
    int ret = 0;

    if (bp) {
        HILITELIST_t *hilites = &bp->b_hilites;
        HILITE_t *hp;

        hp = TAILQ_FIRST(hilites);
        while (hp) {
            HILITE_t *next = TAILQ_NEXT(hp, h_node);

            assert(HILITE_MAGIC == hp->h_magic);
            if (type == hp->h_type) {
                TAILQ_REMOVE(hilites, hp, h_node);
                buf_mined(bp, hp->h_sline, hp->h_eline);
                if (hp == bp->b_hilite) {
                    bp->b_hilite = NULL;
                }
                chk_free((void *)hp);
                ++ret;
            }
            hp = next;
        }
    }
    return ret;
}


int
hilite_delete(BUFFER_t *bp, int seqno)
{
    if (bp) {
        HILITELIST_t *hilites = &bp->b_hilites;
        HILITE_t *hp;

        hp = TAILQ_FIRST(hilites);
        while (hp) {
            HILITE_t *next = TAILQ_NEXT(hp, h_node);

            if (seqno == (int)hp->h_seqno) {
                TAILQ_REMOVE(hilites, hp, h_node);
                buf_mined(bp, hp->h_sline, hp->h_eline);
                if (hp == bp->b_hilite) {
                    bp->b_hilite = NULL;
                }
                chk_free((void *)hp);
                return 1;
            }
            hp = next;
        }
    }
    return 0;
}


int
hilite_clear(BUFFER_t *bp, LINENO line)
{
    int ret = 0;

    if (bp) {
        HILITELIST_t *hilites = &bp->b_hilites;
        HILITE_t *hp = TAILQ_FIRST(hilites);

        if (hp) {
            do {
                HILITE_t *hpnext = TAILQ_NEXT(hp, h_node);

                assert(HILITE_MAGIC == hp->h_magic);
                if (hp->h_sline >= line) {
                    buf_mined(bp, hp->h_sline, hp->h_eline);
                    TAILQ_REMOVE(hilites, hp, h_node);
                    chk_free((void *)hp);
                    ++ret;
                }
                hp = hpnext;
            } while (hp);
            bp->b_hilite = NULL;
        }
    }
    return ret;
}


int
hilite_zap(BUFFER_t *bp, int update)
{
    int ret = 0;

    if (bp) {
        HILITELIST_t *hilites = &bp->b_hilites;
        HILITE_t *hp;

        while (NULL != (hp = TAILQ_FIRST(hilites))) {
            assert(HILITE_MAGIC == hp->h_magic);
            TAILQ_REMOVE(hilites, hp, h_node);
            if (update) {
                buf_mined(bp, hp->h_sline, hp->h_eline);
            }
            chk_free((void *)hp);
            ++ret;
        }
        bp->b_hilite = NULL;
    }
    return ret;
}

/*end*/
