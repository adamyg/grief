#include <edidentifier.h>
__CIDENT_RCSID(gr_position_c,"$Id: position.c,v 1.6 2014/10/22 02:33:14 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: position.c,v 1.6 2014/10/22 02:33:14 ayoung Exp $
 * Buffer position/status.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "position.h"                           /* public header */

#include "basic.h"                              /* mov_...() */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"                            /* cur_line */
#include "debug.h"                              /* trace_...() */
#include "echo.h"                               /* errorf() */
#include "main.h"                               /* curbp */
#include "window.h"                             /* window_...() */

typedef struct pos {
    MAGIC_t             p_magic;                /* Structure magic */
#define POS_MAGIC               MKMAGIC('P','o','S','p')
    TAILQ_ENTRY(pos)    p_node;                 /* List node */
    IDENTIFIER_t        p_bufnum;               /* Buffer number */
    IDENTIFIER_t        p_winnum;               /* Window number */
    LINENO              p_top_line;
    LINENO              p_cur_line;
    LINENO              p_cur_col;
    char                p_name[16];             /* first characters of the buffer name */
} Position_t;

typedef TAILQ_HEAD(PositionList, pos)
                        POSITIONLIST_t;

static POSITIONLIST_t   x_positionq;            /* queue head */


void
position_init(void)
{
    TAILQ_INIT(&x_positionq);
}


void
position_save(void)
{
    struct pos *pos =
            (struct pos *) chk_alloc(sizeof(struct pos));

    if (curwp) {                                /* window */
        pos->p_winnum = curwp->w_num;
    } else {
        pos->p_winnum = 0;
    }

    pos->p_top_line =
        ((curwp && curwp->w_bufp == curbp) ? curwp->w_top_line : -1);

    pos->p_cur_line = *cur_line;
    pos->p_cur_col = *cur_col;

    if (curbp) {                                /* buffer */
        pos->p_bufnum = curbp->b_bufnum;
        strxcpy(pos->p_name, curbp->b_title ? curbp->b_title : "n/s", sizeof(pos->p_name));
    } else {
        pos->p_bufnum = 0;
        strxcpy(pos->p_name, "n/a", sizeof(pos->p_name));
    }

    pos->p_magic = POS_MAGIC;
    TAILQ_INSERT_TAIL(&x_positionq, pos, p_node);
}


int
position_restore(int what)
{
    POSITIONLIST_t *positions = &x_positionq;
    struct pos *pos;

    if (NULL == (pos = TAILQ_LAST(positions, PositionList))) {
        errorf("restore_position: no saved position.");
        return 0;
    }
    TAILQ_REMOVE(positions, pos, p_node);
    assert(POS_MAGIC == pos->p_magic);

    if (what > 0) {

        if (what <= 4) {
            BUFFER_t *saved_bp = curbp;
                                                /* restore buffer */
            if (NULL == (curbp = buf_lookup(pos->p_bufnum))) {
                errorf("restore_position: no such buffer (%d).", pos->p_bufnum);
                curbp = saved_bp;

            } else {
                                                /* restore window */
                if (what >= 4 && pos->p_winnum) {
                    WINDOW_t *saved_wp = curwp;
                                                /* if unknown, restore and continue */
                    if (NULL == (curwp = window_lookup(pos->p_winnum))) {
                        errorf("restore_position: no such window (%d).", pos->p_winnum);
                        curwp = saved_wp;
                    }
                }

                if (-1 != pos->p_top_line) {    /* cursor */
                    curwp->w_top_line = pos->p_top_line;
                }

                set_hooked();
                move_abs(pos->p_cur_line, pos->p_cur_col);

                if (1 == what) {                /* position restore, but not buffer */
                    curbp = saved_bp;
                    set_hooked();
                } else {
                    attach_buffer(curwp, curbp);
                }
            }

        } else {
            errorf("restore_position: unknown operation %d.", what);
        }
    }

    pos->p_magic = 0;
    chk_free(pos);
    return 1;
}


/*  Function:           position_dump
 *      Dump the position stack, used during system diagnostics debugging.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
position_dump(void)
{
    POSITIONLIST_t *positions = &x_positionq;
    const struct pos *pos;
    int idx = 0;

    trace_log("save_position dump:\n");

    TAILQ_FOREACH(pos, positions, p_node) {
        const BUFFER_t *bp =
                (pos->p_bufnum ? buf_lookup(pos->p_bufnum) : NULL);

        trace_log("\t[%2d] %d%d %d/%d ", idx, pos->p_winnum, pos->p_bufnum, pos->p_cur_line, pos->p_cur_col);
        if (bp) {
            trace_log(" active \"%s ...\"\n", bp->b_title ? bp->b_title : "n.s.");
        } else {
            trace_log(" closed \"%s ...\"\n", pos->p_name);
        }
        ++idx;
    }
}


/*  Function:           position_shutdown
 *      System shutdown.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
position_shutdown(void)
{
    POSITIONLIST_t *positions = &x_positionq;
    struct pos *pos;

    while (NULL != (pos = TAILQ_FIRST(positions))) {
        TAILQ_REMOVE(positions, pos, p_node);
        pos->p_magic = 0;
        chk_free(pos);
    }
}
/*end*/
