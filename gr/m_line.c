#include <edidentifier.h>
__CIDENT_RCSID(gr_m_line_c,"$Id: m_line.c,v 1.19 2020/04/21 00:01:56 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_line.c,v 1.19 2020/04/21 00:01:56 cvsuser Exp $
 * Line primitives.
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

#include "accum.h"                              /* acc_ .. */
#include "builtin.h"                            /* cur_line .. */
#include "buffer.h"                             /* buf_argument() */
#include "eval.h"                               /* get_xinteger() */
#include "main.h"                               /* curbp */
#include "symbol.h"                             /* argv_ .. */
#include "map.h"                                /* linep() */
#include "m_line.h"                             /* public interface */


static int
flagcount(lineflags_t flags)
{
    int c;
    for (c = 0; flags; ++c) {
        flags &= flags - 1;
    }
    return c;
}


/*  Function:           do_mark_line
 *      mark_line primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
   Macro: mark_line - Create a line marker.

        int
        mark_line(int flag, [int toggleall],
            [int bufnum], [int lineno = 0], [int marker = L_MARKED])

    Macro Description:
        The 'mark_line' primitive controls the value of the user
        defined line marker within the current buffer. This mark
        maybe be utilised by macro developers to maintain a
        collection of lines on which can then can be queried using
        find_marker() for additional processing.

        The 'flags' parameter controls the mark value; if non-zero
        *true* then the marker is set otherwise it is cleared. When
        'toggleall' is stated then the mark status of all lines is
        toggled, ignoring the 'flag' specification.

        'bufnum' and 'lineno' allow explicit buffer and line number
        references to be stated, otherwise if omitted the current
        buffer and/or associated line number shall be used.

        'marker' is the optional marker against which to search, by
        default *L_MARKED*. Only *L_MARKED* or one of the *L_USERx*
        definitions maybe be specified.

        Note!: 
        Markers are only a temporary resource which maybe cleared
        when line are modified, deleted etc.

     Macro Returns:
        The 'mark_line' primitives returns 1 if the marker was
        already set, 0 if the marker was not set, otherwise -1 when
        beyond the end of the buffer.

    Macro Compatibility:
        The options 'bufnum', 'lineno' and 'marker' are Grief extensions.

    Macro See Also:
        find_marker
 */
void
do_mark_line(void)              /* int (int flag, [int toggleall],
                                        [int bufnum], [int lineno = 0], [int marker = L_MARKED]) */
{
    const int flag = get_xinteger(1, 0);
    const int toggleall = (isa_integer(2) ? 1 : 0);
    BUFFER_t *bp = buf_argument(3);
    int lineno = get_xinteger(4, bp ? bp->b_line : 0);
    const lineflags_t marker = get_xinteger(5, L_MARKED);
    int ret = -1;

    if (0 == (L_USER_MASK & marker) || 1 != flagcount(marker)) {
        acc_assign_int(-1);
        return;
    }

    /* current line state and set, unless toggleall */
    if (bp) {
        BUFFER_t *ocurbp = curbp;

        curbp = bp;
        if (lineno >= 1 && lineno <= curbp->b_numlines) {
            LINE_t *lp = linepx(curbp, lineno);

            if (lp) {
                ret = (marker & lp->l_uflags ? 1 : 0);
                if (! toggleall) {
                    if (flag) {                 /* set */
                        lp->l_uflags |= marker;
                    } else {                    /* clear */
                        lp->l_uflags &= ~marker;
                    }
                }
            }
        }
        curbp = ocurbp;
    }

    /* toggleall line mark status */
    if (bp && toggleall) {
        register LINE_t *lp;

        TAILQ_FOREACH(lp, &bp->b_lineq, l_node) {
            if (marker & lp->l_uflags) {        /* clear */
                lp->l_uflags &= ~marker;
            } else {                            /* set */
                lp->l_uflags |= marker;
            }
        }
    }

    acc_assign_int(ret);
}


/*  Function:           do_find_marker
 *      find_marker primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: find_marker - Locate next marker.

        int
        find_marker([int marker = L_MARKED])

    Macro Description:
        The 'find_marker()' primitive positions the cursor at the next line
        which has a user defined marker enabled. On successful completion the
        marker is removed.

        'marker' is the optional marker against which to search, by default
        L_MARKED. Only L_MARKED or one of the L_USERx definitions maybe be
        specified, if other bits or more then one user bit is stated the
        search shall not succeed.

    Macro Returns:
        The 'find_marker()' primitive returns 1 on success and 0 if
        no additional markers exist.

    Macro See Also:
        mark_line and find_line_flags.

    Macro Compatibility:
        The 'marker' parameter is a Grief extension.
 */
void
do_find_marker(void)            /* int ([int marker = L_MARKED]) */
{
    const lineflags_t marker = get_xinteger(1, L_MARKED);
    const LINENO numlines = curbp->b_numlines;
    LINENO line;
    int ret = 0;

    if (0 == (L_USER_MASK & marker) || 1 != flagcount(marker)) {
        acc_assign_int(-1);
        return;
    }

    for (line = *cur_line + 1; line <= numlines; ++line) {  /*NEWLINE*/
        LINE_t *lp = vm_lock_line(line);

        if (marker & lp->l_uflags) {
            lp->l_uflags &= ~marker;            /* clear */
            vm_unlock(line);
            *cur_line = line;
            ret = 1;
            break;
        }
        vm_unlock(line);
    }

    acc_assign_int(ret);
}


/*  Function:           inq_line_flags
 *      inq_line_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_line_flags - Retrieve a lines associated flags.

        int
        inq_line_flags([int bufnum], [int lineno], [int& iflags])

    Macro Description:
        The 'inq_line_flags()' primitive retrieves the flags
        associated with the specified line.

        The line flags was a set of 32 bit values, with the upper 16 bits
        being defined for Grief usage and lower 16 bits for user/macro usage.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        lineno - Line number within the selected buffer, otherwise if
            omitted the current line is referenced.

        iflags - Optional storage for the associated internal flags.

    Macro Returns:
        Associated line flags.

    Macro Compatibility:
        Internal flags are an Grief extension.

    Macro See Also:
        set_line_flags and find_line_flags.
 */
void
inq_line_flags(void)            /* int ([int bufnum], [int lineno], [int& iflags]) */
{
    BUFFER_t *bp = buf_argument(1);
    int lineno = get_xinteger(2, bp ? bp->b_line : 0);
    lineflags_t iflags = 0, uflags = 0;

    if (bp) {
        if (lineno >= 1 && lineno <= bp->b_numlines) {
            const LINE_t *lp = linepx(bp, lineno);

            if (lp) {
                iflags = liflags(lp);
                uflags = lflags(lp);
            }
        }
    }
    argv_assign_int(2, iflags);                 /* extension, internal flags */
    acc_assign_int(uflags);
}


/*  Function:           do_set_line_flags
 *      set_line_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_line_flags - Associate line flags.

        int
        set_line_flags([int bufnum], [int start], [int end], 
                [int and_mask], [int or_value])

    Macro Description:
        The 'set_line_flags' primitive allows the flags of one or
        more line within a specific buffer to be modified.

        The buffer flags was a set of 32 bit values separated into
        two namespaces, with the upper 8 bits being defined for
        user/macro usage and lower bits for system user/macro usage.
        As such only the lower 16 bits maybe affected by this
        primitive.

        The defines *L_USER1* thru *L_USER7* maybe used as manifest
        constants to access the user/macro area.

    Macro Parameters:
        bufnum -    Buffer number, if omitted the current buffer is referenced.
       
        start -     Start line number of region within the selected buffer, 
                    otherwise if omitted the current line is referenced.

        end -       End of the region within the selected buffer, otherwise
                    if omitted the current line is referenced as such one
                    line shall be affected.

        and_mask -  Value AND'ed with the flags of matched lines.
 
        or_value -  Value OR'ed with the flags of matched lines.
 
    Macro Compatiblty:
        Grief enforces two flag namespaces system and user each of 16
        bits with only the lower user 16 bits being read-write, 
        whereas CrispEdit allows read-write to all 32 bits.

    Macro Returns:
        The 'set_line_flags' primitive returns the number at lines
        which were modified.

    Macro see Also:
        inq_line_flags() and find_line_flags().
 */
void
do_set_line_flags(void)         /* int ([int bufnum], [int start], [int end], [int and_mask], [int or_value]) */
{
    BUFFER_t *bp = buf_argument(1);
    int start = get_xinteger(2, bp ? bp->b_line : 0);
    int end = get_xinteger(3, start + 1);
    lineflags_t and_mask = (lineflags_t) get_xinteger(4, -1);
    lineflags_t or_value = (lineflags_t) get_xinteger(5, 0);
    int ret = 0;

    if (bp) {
        int line;

        /* apply filters to mask, guarding syste/critical bits */
#if defined(L_SYSRO_MASK) && (L_SYSRO_MASK)
        if (! BFTST(bp, BF_SYSBUF)) {
            and_mask |= L_SYSRO_MASK;           /* dont allow clear */
            or_value &= ~L_SYSRO_MASK;          /* and dont allow set */
        }
#endif  /*L_SYSRO_MASK*/

        /* apply limits */
        if (start > end) {                      /* swap */
            int t_start = end;
            end = start;
            start = t_start;
        }

        if (end > bp->b_numlines) {
            end = bp->b_numlines;
        }

        for (line = start; line < end; ++line) {
            LINE_t *lp = vm_lock_line(line);

            lp->l_uflags &= and_mask;
            lp->l_uflags |= or_value;
            vm_unlock(line);
            ++ret;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_find_line_flags
 *      find_line_flags primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: find_line_flags - Locate next line with specific flags.

        int
        find_line_flags([int bufnum], [int lineno],
            int mode, int and_mask, [int or_value], [int value])

    Macro Description:
        The 'find_line_flag()' primitive positions the cursor at the
        next line which matches the specified flag value.

        'bufnum' and 'lineno' allow explicit buffer and line number
        references to be stated, otherwise if omitted the current
        buffer and/or associated line number shall be used.

        'flags' defines direction and type of equivalence test to be
        utilised. 'and_mask' and 'or_value' parameterise the
        equivalence expression.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.
 
        lineno - Starting line number.

        mode - Search mode flags.

            LF_FORWARDS -   Forward search (default).
 
            LF_BACKWARDS -  Backwards search.
 
            LF_MATCH_EQ -   Absolute value match against the line flags
                            AND'ed against 'and_mask' and then OR'ed against
                            'or_value'.
 
                                ie. ((flags & and_mask) | or_value) == value)
 
            LF_MATCH_ANY -  Match line flags were any flags contained within
                            the 'and_mask' are set,
 
                                ie. ((flags & and_masK) != 0).
 
        and_mask -      AND mask.
 
        or_value -      OR value during LF_MATCH_EQ operations.
 
        value -         Value being matched during LF_MATCH_EQ operations.
 
    Macro Returns:
        The 'find_line_flag()' primitive returns the matched line
        number, 0 if not suitable match was found, otherwise -1 on
        error, for example invalid parameters.

    Macro See Also:
        find_marker()
 */
void
do_find_line_flags(void)        /* int ([int bufnum], [int lineno],
                                        int mode, int and_mask, [int or_value], [int value]) */
{
    BUFFER_t *bp = buf_argument(1);
    int start = get_xinteger(2, bp ? bp->b_line : 0);
    int mode = get_xinteger(3, 0);
    uint32_t and_mask = (uint32_t) get_xinteger(4, -1);
    uint32_t or_value = (uint32_t) get_xinteger(5, 0);
    uint32_t value = (uint32_t) get_xinteger(6, 0);
    int ret = 0;

#ifndef LF_FORWARDS
#define LF_FORWARDS         1
#define LF_BACKWARDS        2
#define LF_MATCH_EQ         4
#define LF_MATCH_ANY        8
#endif

    if (0 == bp ||
            0 == (mode & (LF_MATCH_EQ|LF_MATCH_ANY)) ||
                 (mode & (LF_MATCH_EQ|LF_MATCH_ANY)) == (LF_MATCH_EQ|LF_MATCH_ANY) ||
                 (mode & (LF_FORWARDS|LF_BACKWARDS)) == (LF_FORWARDS|LF_BACKWARDS) ||
            0 == and_mask) {
        ret = -1;

    } else {
        if (mode & LF_BACKWARDS) {
            /*
             *  backwards
             */
            LINENO line;

            for (line = (uint32_t) start; line; --line) {
                LINE_t *lp = vm_lock_line(line);

                if (((mode & LF_MATCH_EQ)  && (((lp->l_uflags & and_mask) | or_value) == value)) ||
                    ((mode & LF_MATCH_ANY) &&  ((lp->l_uflags & and_mask) != 0))) {
                    /*
                     *  match
                     */
                    vm_unlock(line);
                    ret = (int) line;
                    break;
                }
                vm_unlock(line);
            }

        } else {    /* LF_FORWARD or omitted (default) */
            /*
             *  forward scan
             */
            const LINENO numlines = bp->b_numlines;
            LINENO line;

            for (line = (uint32_t) start; line <= numlines; ++line) {   /* NEWLINE */
                const LINE_t *lp = vm_lock_line(line);

                if (((mode & LF_MATCH_EQ)  && (((lp->l_uflags & and_mask) | or_value) == value)) ||
                    ((mode & LF_MATCH_ANY) &&  ((lp->l_uflags & and_mask) != 0))) {
                    /*
                     *  match
                     */
                    vm_unlock(line);
                    ret = (int) line;
                    break;
                }
                vm_unlock(line);
            }
        }
    }
    acc_assign_int(ret);
}

/*end*/
