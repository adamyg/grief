#include <edidentifier.h>
__CIDENT_RCSID(gr_anchor_c,"$Id: anchor.c,v 1.46 2021/06/10 06:13:01 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: anchor.c,v 1.46 2021/06/10 06:13:01 cvsuser Exp $
 * Anchor primitives.
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

#include "anchor.h"                             /* public header */

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "mac1.h"
#include "main.h"
#include "map.h"
#include "region.h"
#include "symbol.h"
#include "system.h"
#include "undo.h"
#include "wild.h"
#include "window.h"

typedef struct _anchor {
    MAGIC_t             a_magic;                /* Structure magic */
#define ANCHOR_MAGIC        MKMAGIC('A','n','C','h')
    TAILQ_ENTRY(_anchor)                        /* List nodes */
                        a_bnode, a_gnode;
    int                 a_type;                 /* Mark type (MK_...) */
    IDENTIFIER_t        a_bufnum;               /* Buffer number */
    LINENO              a_line;                 /* Top */
    LINENO              a_offset;
    LINENO              a_eline;                /* Bottom/end */
    LINENO              a_eoffset;
} Anchor_t;

static ANCHORLIST_t     x_anchorq;              /* queue head */

static void             anchor_delete(BUFFER_t *bp, Anchor_t *anchor);
static int              greatest_line(LINENO start_line, LINENO end_line);


/*  Function:           anchor_attach
 *      Buffer anchor management runtime initialiation
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
anchor_attach(BUFFER_t *bp)
{
    TAILQ_INIT(&bp->b_anchors);
    bp->b_anchor = NULL;
}


/*  Function:           anchor_detach
 *      Buffer anchor management runtime shutdown.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
anchor_detach(BUFFER_t *bp)
{
    if (bp) {
        ANCHORLIST_t *banchors = &bp->b_anchors;
        Anchor_t *ap;

        bp->b_anchor = NULL;
        while (NULL != (ap = TAILQ_FIRST(banchors))) {
            anchor_delete(bp, ap);
        }
    }
}


/*  Function:           anchor_zap
 *      Destroy all the associated anchors with the given buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
anchor_zap(BUFFER_t *bp)
{
    if (bp) {
        ANCHORLIST_t *banchors = &bp->b_anchors;
        Anchor_t *ap;

        bp->b_anchor = NULL;
        while (NULL != (ap = TAILQ_FIRST(banchors))) {
            anchor_delete(bp, ap);
        }
    }
}


static void
anchor_delete(BUFFER_t *bp, Anchor_t *ap)
{
    ANCHORLIST_t *banchors = &bp->b_anchors,
            *ganchors = &x_anchorq;

    assert(ANCHOR_MAGIC == ap->a_magic);
    assert(ap->a_bufnum == bp->b_bufnum);
    TAILQ_REMOVE(banchors, ap, a_bnode);
    TAILQ_REMOVE(ganchors, ap, a_gnode);
    if (bp->b_anchor == ap) {
        bp->b_anchor = TAILQ_FIRST(banchors);
    }
    ap->a_magic = 0xDEADBEEF;
    chk_free((void *)ap);
}


/*  Function:           anchor_drop
 *      Drop a anchor at the current position.
 *
 *  Parameters:
 *      type - Anchor type.
 *
 *  Returns:
 *      nothing
 */
void
anchor_drop(int type)
{
    assert(MK_NONE != type);
    if (curbp) {
        Anchor_t *ap = (Anchor_t *) chk_calloc(sizeof(Anchor_t),1);

        if (ap) {
            const int previous_anchor = (NULL != curbp->b_anchor);
            ANCHORLIST_t *banchors = &curbp->b_anchors,
                    *ganchors = &x_anchorq;

            ap->a_magic   = ANCHOR_MAGIC;
            ap->a_type    = type;
            ap->a_bufnum  = curbp->b_bufnum;
            ap->a_line    = *cur_line;
            ap->a_offset  = *cur_col;
            ap->a_eline   = 0;
            ap->a_eoffset = 0;

            u_raise();

            TAILQ_INSERT_HEAD(banchors, ap, a_bnode);
            TAILQ_INSERT_HEAD(ganchors, ap, a_gnode);
            curbp->b_anchor = ap;

            win_modify(previous_anchor ? WFHARD : WFMOVE);
        }
    }
}


/*  Function:           anchor_raise
 *      Raise the current anchor.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      TRUE on success, otherwise FALSE
 */
int
anchor_raise(void)
{
    Anchor_t *ap;

    if (NULL == curbp ||
            NULL == (ap = (Anchor_t *)curbp->b_anchor)) {
        return FALSE;
    }
    assert(ANCHOR_MAGIC == ap->a_magic);
    assert(ap == TAILQ_FIRST(&curbp->b_anchors));
    u_drop();
    anchor_delete(curbp, ap);
    win_modify(WFHARD);
    return TRUE;
}


/*  Function:           anchor_adjust
 *      Adjust the current anchor post a line insertion/deletion.
 *
 *  Parameters:
 *      ins - TRUE if a line insertion otherwise deletion.
 *
 *  Returns:
 *      void
 */
int
anchor_adjust(int ins)
{
    int change = 0;

    if (curbp && curbp->b_anchor) {
        Anchor_t *ap = (Anchor_t *)curbp->b_anchor;

        assert(ANCHOR_MAGIC == ap->a_magic);
        if (ins) {
            /*
             *  Line insertion occurred.
             *
             *  If current buffer has a marked area,
             *      then update the end of region pointer if it ends after the cursor.
             *      ie. A line was inserted into the marked area.
             */
            if (ap->a_line > *cur_line) {
                ++ap->a_line;                   /* move the start by one */
                if (ap->a_eline > *cur_line) {
                    ++ap->a_eline;              /* move the end by one */
                    ++change;
                }
                ++change;
            }
        } else {
            /*
             *  Deletion ...
             */
            /*TODO - anchor mode*/
        }
    }
    return change;
}


/*  Function:           anchor_get
 *      Retrieve the normalised region associated with the current anchor.
 *
 *  Parameters:
 *      wp - Window object address.
 *      bp - Buffer object address, otherwise if NULL 'curbp' is referenced.
 *      a - Anchor object.
 *
 *  Returns:
 *      TRUE on success, otherwise FALSE.
 */
int
anchor_get(WINDOW_t *wp, BUFFER_t *bp, ANCHOR_t *a)
{
    const Anchor_t *ap;
    LINENO tmp;

    if (NULL == bp) bp = curbp;
    if (NULL == wp) {
        ap = (bp ? bp->b_anchor : NULL);
    } else {
        ap = (wp->w_bufp ? wp->w_bufp->b_anchor : NULL);
    }

    if (NULL == ap) {
        /*
         *  no anchor
         *      return the current position to the end-of-buffer.
         */
        a->type       = MK_NONE;
        a->start_line = (curbp == bp ? *cur_line : bp->b_line);
        a->start_col  = 0;
        a->end_line   = (bp && bp->b_numlines ? bp->b_numlines : 1);
        a->end_col    = 0;
        return FALSE;
    }

    assert(ANCHOR_MAGIC == ap->a_magic);
    assert(MK_NONE != ap->a_type);
    a->type       = ap->a_type;
    a->start_line = ap->a_line;
    a->start_col  = ap->a_offset;
    a->end_line   = ap->a_eline > 0 ?  ap->a_eline : (wp ? wp->w_line : *cur_line);
    a->end_col    = ap->a_eoffset > 0 ? ap->a_eoffset : (wp ? wp->w_col : *cur_col);

    if (MK_COLUMN == a->type) {
        if (a->start_line > a->end_line) {
            GR_SWAP(a->start_line, a->end_line, tmp);
        }

        if (a->start_col > a->end_col) {
            GR_SWAP(a->start_col, a->end_col, tmp);
        }
        return TRUE;
    }

    if (a->start_line > a->end_line) {
        GR_SWAP(a->start_line, a->end_line, tmp);
        GR_SWAP(a->start_col, a->end_col, tmp);
    }

    if (a->start_line == a->end_line && a->start_col > a->end_col) {
        GR_SWAP(a->start_col, a->end_col, tmp);
    }

    if (MK_LINE == a->type) {
        a->start_col = 1;
        a->end_col = CURSOR_HUGE_COL;

    } else if (MK_NONINC == a->type) {
        --a->end_col;
    }
    return TRUE;
}


void
anchor_read(BUFFER_t *bp, ANCHOR_t *a)
{
    if (bp && bp->b_anchor) {
        const Anchor_t *ap = (const Anchor_t *)bp->b_anchor;

        assert(ANCHOR_MAGIC == ap->a_magic);
        assert(MK_NONE != ap->a_type);
        a->type = ap->a_type;
        a->start_line = ap->a_line;
        a->start_col = ap->a_offset;
        a->end_line = ap->a_eline;
        a->end_col = ap->a_eoffset;

    } else {
        a->type = MK_NONE;
        a->start_line = 0;
        a->start_col = 0;
        a->end_line = 0;
        a->end_col = 0;
    }
}


void
anchor_write(BUFFER_t *bp, const ANCHOR_t *a)
{
    if (bp && bp->b_anchor) {
        Anchor_t *ap = (Anchor_t *)bp->b_anchor;

        assert(ANCHOR_MAGIC == ap->a_magic);
        assert(MK_NONE != ap->a_type);
        if (MK_NONE == a->type) {
            anchor_delete(bp, ap);

        } else {
            ap->a_type = a->type;
            ap->a_line = a->start_line;
            ap->a_offset = a->start_col;
            ap->a_eline = a->end_line;
            ap->a_eoffset = a->end_col;
        }
    }
}


void
anchor_dump(void)
{
    ANCHORLIST_t *anchors = &x_anchorq;
    const Anchor_t *ap;
    int idx = 0;

    trace_log("anchor dump:\n");
    TAILQ_FOREACH(ap, anchors, a_gnode) {
        trace_log("\t[%2d] %d %d %d/%d %d/%d\n", idx, ap->a_type,
            (int)ap->a_bufnum, (int)ap->a_line, (int)ap->a_offset, (int)ap->a_eline, (int)ap->a_eoffset);
        ++idx;
    }
}


/*  Function:           do_drop_anchor
 *      drop_anchor primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: drop_anchor - Start marking a selection.

        int
        drop_anchor([int type = MK_NORMAL])

    Macro Description:
        The 'drop_anchor()' primitive starts marking a block of the
        specified 'type' at the current position in the current buffer.

        Marked regions are highlighted to stand out on the screen either
        in a different colour or in reverse video dependent on the
        available display capacities.

        The following are the available region types.

(start table,format=nd)
        [Value  [Constant       [Description            ]
      ! 1       MK_NORMAL       A normal mark.
      ! 2       MK_COLUMN       A column mark.
      ! 3       MK_LINE         A line mark.
      ! 4       MK_NONINC       A non-inclusive mark.
(end table)

        MK_NORMAL - A normal mark is a region which encompasses from the
            place where the anchor was dropped up to and including the
            current cursor position.

        MK_COLUMN - A column mark allows a rectangular section of the
            buffer to be marked, highlighting the inclusive columns
            between the left and right boundaries.

        MK_NONINC - A non-inclusive mark, like a normal, is a region
            which encompasses from the place where the anchor was
            dropped up to and but 'does not' include the current cursor
            position.

        MK_LINE - A line mark selects entire lines, and allows for easy
            movement of text from one part of a buffer to another.

        Regions are nestable, in that a 'drop_anchor' may be issued
        without an intervening 'raise_anchor'. Each mark is pushed into
        a LIFO (last-in, first-out) stack, allowing multiple marks to
        exist simultaneously; each mark must however be eventually
        raised. The most recent mark shall be the one displayed by the
        buffer.

        The current active marked region can queried using <inq_marked>.

        The marked region can be cleared by calling <raise_anchor> or
        performing a high level <copy> or <cut> buffer operations plus a
        numebr of the lower level functions, for example <delete_block>.

    Macro Parameters:
        type - Optional anchor type to be dropped Otherwise if omitted a
            *MK_NORMAL* anchor shall created.

    Macro Returns:
        The 'drop_anchor' returns 1 if successful, otherwise 0 on error.

    Macro Portability:
        n/a

    Macro See Also:
        mark, raise_anchor
 */
void
do_drop_anchor(void)            /* ([int type]) */
{
    const int type = get_xinteger(1, MK_NORMAL);
    int ret = 1;

    if (0 == type) {
        anchor_drop(MK_NORMAL);

    } else if (type <= MK_NONINC) {
        anchor_drop(type);

    } else {
        ret = 0;
    }

    acc_assign_int(ret);
}


/*  Function:           do_end_anchor
 *      end_anchor primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: end_anchor - Set the end of the anchor.

        int
        end_anchor([int line], [int column])

    Macro Description:
        The 'end_anchor()' primitive sets the end of the current marked
        region without the need to move the cursor.

    Macro Parameters:
        line - Optional new line, if omitted the line shall be
            unchanged.

        column - Optional new column, if omitted the column shall be
            unchanged.

    Macro Returns:
        The 'end_anchor' returns 1 if successful, otherwise 0 on error.

    Macro Portability:
        n/a

    Macro See Also:
        mark, drop_anchor
 */
void
do_end_anchor(void)             /* int ([int line], [int column]) */
{
    const LINENO line = (isa_undef(1) || 0 == get_xinteger(1, 0) ?
                            *cur_line : get_xinteger(1, 0));
    const LINENO col  = (isa_undef(2) || 0 == get_xinteger(2, 0) ?
                            *cur_col : get_xinteger(2, 0));
    int ret = 0;

    if (curbp && curbp->b_anchor) {
        Anchor_t *ap = (Anchor_t *)curbp->b_anchor;

        u_anchor();
        ap->a_eline = line;
        ap->a_eoffset = col;
        ret = 1;
    }
    acc_assign_int(ret);
}


/*  Function:           do_raise_anchor
 *      raise_anchor primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: raise_anchor - Raise the last dropped mark.

        int
        raise_anchor()

    Macro Description:
        The 'raise_anchor()' primitive raises the last anchor mark that
        was dropped.

        If no mark is dropped, this primitive has no effect.

    Macro Parameters:
        none

    Macro Returns:
        The 'raise_anchor' returns 1 if successful, otherwise 0 on
        error.

    Macro Portability:
        n/a

    Macro See Also:
        mark, drop_anchor
 */
void
do_raise_anchor(void)           /* int () */
{
    acc_assign_int(anchor_raise());
}


/*  Function:           do_mark
 *      mark primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: mark - Toggle the anchor status.

        int
        mark([int type = MK_NORMAL])

    Macro Description:
        The 'mark()' primitive toggles the status of the marked
        region. If the anchor is currently dropped it shall be raised,
        and if raised it shall be dropped.

        To be independent of the current state macros should
        generally utilise <drop_anchor> and <raise_anchor>, with mark
        reserved for keyboard bindings.

    Macro Parameters:
        type - Optional anchor type to be dropped, if omitted a
            MK_NORMAL (1) mark shall be dropped.

(start table,format=nd)
        [Value  [Constant       [Description            ]
      ! 1       MK_NORMAL       A normal mark.
      ! 2       MK_COLUMN       A column mark.
      ! 3       MK_LINE         A line mark.
      ! 4       MK_NONINC       A non-inclusive mark.
(end table)

    Macro Returns:
        The 'mark' returns 1 if successful, otherwise 0 on error.

    Macro Portability:
        Unlike BRIEF the anchor status is always toggled.

        BRIEF logic was equivalent to the following. If 'type' was
        stated and an anchor of a different type current exists, the
        anchor is converted.

>           if (! inq_marked() || type) {
>               drop_anchor(type);
>           } else {
>               raise_anchor();
>           }

    Macro See Also:
        inq_marked, raise_anchor, drop_anchor
 */
void
do_mark(void)                   /* int ([int type = NULL]) */
{
    if (curbp) {

#if defined(DO_BRIEF)
        if (isa_integer(1)) {   // convert current anchor
            do_drop_anchor();
            return;
        }
#endif

        if (curbp->b_anchor) {
            do_raise_anchor();
        } else {
            do_drop_anchor();
        }
    }
}


/*  Function:           inq_marked
 *      inq_marked primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_marked - Determine the current marked region.

        int
        inq_marked([int &start_line], [int &start_col],
                [int &end_line], [int &end_col])

    Macro Description:
        The 'inq_marked()' primitive retrieves the current mark type and
        the associated coordinates of the marked area.

        *Region Types*

(start table,format=nd)
        [Value  [Constant       [Description            ]
    !   0       MK_NONE         No mark is set
    !   1       MK_NORMAL       Normal mark
    !   2       MK_COLUMN       Column mark
    !   3       MK_LINE         Line mark
    !   4       MK_NONINC       Non-inclusive
(end table)

    Macro Parameters:
        start_line - If specified the integer parameter is set to the
            line number at the top of the marked region.

        start_col - If specified the integer parameter is set to the
            column number at the beginning of the marked region.

        end_line - If specified the integer parameter is set to the
            line number marking the bottom of the marked region.

        end_col - If specified the integer parameter is set to the
            column number at the end of the marked region.

    Macro Returns:
        The 'inq_marked()' primitive returns the current region type,
        otherwise 0 if no mark is active.

    Macro Portability:
        n/a

    Macro See Also:
        mark
 */
void
inq_marked(void)                /* int () */
{
    ANCHOR_t a;

    if (anchor_get(NULL, NULL, &a)) {
        argv_assign_int(1, (accint_t) a.start_line);
        argv_assign_int(2, (accint_t) a.start_col);
        argv_assign_int(3, (accint_t) a.end_line);
        if (!isa_undef(4)) {
            if (MK_LINE == a.type) {
                a.end_col = greatest_line(a.start_line, a.end_line);
            }
            sym_assign_int(get_symbol(4), (accint_t) a.end_col);
        }
        acc_assign_int((accint_t) a.type);
    } else {
        acc_assign_int(0);
    }
}



/*  Function:           inq_marked_size
 *      inq_marked primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_marked_size - Size the marked region.

        int
        inq_marked_size()

    Macro Description:
        The 'inq_marked_size()' primitive is reserved for future
        compatibility.

        The 'inq_marked_size()' primitive determines the number of
        characters contained within the current marked region.

    Macro Parameters:

    Macro Returns:
        Returns the character count within the marked region.

    Macro Portability:
        Function is currently a no-op, returning -1.

    Macro See Also:
        inq_marked
 */
void
inq_marked_size(void)           /* int () */
{
    //TODO
    acc_assign_int(-1);
}


static int
greatest_line(LINENO start_line, LINENO end_line)
{
    LINENO line = start_line;
    LINENO maxcol = 0;

    if (end_line > curbp->b_numlines) {
        end_line = curbp->b_numlines;
    }

    while (line <= end_line) {
        const LINE_t *lp = vm_lock_line(line);
        if (lp) {
            const int col = line_column2(lp, line, (int) llength(lp));
            if (col > maxcol) {
                maxcol = col;
            }
            vm_unlock(line);
        }
        ++line;
    }
    return maxcol;
}


/*  Function:           do_swap_anchor
 *      swap_anchor primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: swap_anchor - Swaps the mark with the current position.

        int
        swap_anchor()

    Macro Description:
        The 'swap_anchor()' primitive swaps the current cursor position
        with the start of the marked region, without changing the
        mark type.

    Macro Parameters:
        none

    Macro Returns:
        Returns *true* on success, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        drop_anchor, mark
 */
void
do_swap_anchor(void)            /* int () */
{
    int ret = 0;

    if (curbp && curbp->b_anchor) {
        Anchor_t *ap = (Anchor_t *)curbp->b_anchor;
        const LINENO cline = *cur_line,
                ccol = *cur_col;

        assert(ANCHOR_MAGIC == ap->a_magic);
        if (ap->a_eline <= 0) {                 /* not dropped */
            u_dot();
            u_anchor();
            if (ap->a_line > 0) {
                *cur_line = ap->a_line;
                ap->a_line = cline;
                if (MK_COLUMN != ap->a_type) {
                    *cur_col = ap->a_offset;
                    ap->a_offset = ccol;
                }
            }
            ret = 1;

        } else {                                /* dropped */
            if (cline == ap->a_line) {
                if (cline == ap->a_eline) {
                    if (ccol == ap->a_offset) {
                    } else {
                    }
                } else {
                    /*???*/
                }
            } else {
                /*???*/
            }
        }
    }
    acc_assign_int(ret);
}

/*end*/

