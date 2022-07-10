#include <edidentifier.h>
__CIDENT_RCSID(gr_m_hilite_c,"$Id: m_hilite.c,v 1.17 2022/07/10 13:13:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_hilite.c,v 1.17 2022/07/10 13:13:07 cvsuser Exp $
 * Hilite primitives.
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

#include "m_hilite.h"                           /* public interface */

#include "accum.h"
#include "accum.h"                              /* acc_...() */
#include "buffer.h"                             /* buf_argument() */
#include "builtin.h"                            /* cur_line/cur_col */
#include "color.h"
#include "debug.h"                              /* trace_...() */
#include "eval.h"                               /* get_...() */
#include "hilite.h"
#include "main.h"                               /* curbp */
#include "symbol.h"                             /* argv_assign.. */
#include "window.h"


/*  Function:           do_hilite_create
 *      hilite_create primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: hilite_create - Create a hilite resource.

        int
        hilite_create([int bufnam], [int type], [int timeout],
                [int sline], [int scol], [int eline], [int ecol],
                [string | int attr], [int ident])

    Macro Description:
        The 'hilite_create()' primitive creates a buffer hilite resource
        with the referenced buffer 'bufnum' under the classification
        'type'; the given classification groups hilite resources allowing
        bulk management and removal.

        Similar to the buffer anchors yet they can not be edited and
        there maybe as many as desired hilite's are available for use by
        macros to mark elements within documents, for example search
        results.

        The created resource decorators the region for the duration
        'timeout' between the stated starting position 'sline', 'eline'
        upto the ending position 'eline', 'ecol' using the display
        attribute 'attr'.

    Macro Parameter:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        type - Optional type, default 0; user assignable label.

        timeout - Specifies a timeout in seconds. If specified then the
            hilite shall be automatically deleted upon the timeout
            expiring.

        sline, scol - Start of the hilite region.

        eline, ecol - End for the hilite region.

        attr - Associated attribute.

        ident - User assigned identifier.

    Macro Returns:
        The 'hilite_create()' primitive returns the unique hilite
        identifier, otherwise -1 on error.

    Macro Portability:
        A Grief extension, yet it was noted similar functionality has
        been introduced to CRiSPEdit in parallel; compatibility as
        yet confirmed.

    Macro See Also:
        hilite_destroy, inq_hilite
 */
void
do_hilite_create(void)          /* int ([int bufnam], [int type], [int timeout], [int sline], [int scol],
                                            [int eline], [int ecol], [string | int attribute], [int ident]) */
{
    BUFFER_t *bp = buf_argument(1);
    int ret = -1;

    if (bp) {
        const int     type      = get_xinteger(2, 0);
        const int32_t timeout   = get_xinteger(3, 0);
        const LINENO  sline     = get_xinteger(4, *cur_line);
        const LINENO  scol      = get_xinteger(5, *cur_col);
        const LINENO  eline     = get_xinteger(6, sline);
        const LINENO  ecol      = get_xinteger(7, scol);
        HILITE_t *hp;

        if (NULL != (hp = hilite_create(bp, type, timeout, sline, scol, eline, ecol))) {

            const int ATTRMAX = (1 == sizeof(LINEATTR) ? 0xff : 2048);
            const char *name;
            int attr;

            if (NULL != (name = get_xstr(8))) {
                if ((attr = attribute_value(name)) >= 0) {
                    hp->h_attr = (uint16_t) attr;
                }
            } else if (isa_integer(8)) {
                if ((attr = get_xinteger(8, -1)) >= 0 && attr <= ATTRMAX) {
                    hp->h_attr = (uint16_t) attr;
                }
            }

            hp->h_ident = get_xaccint(9, 0);
            ret = (int) hp->h_seqno;
        }
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: hilite_destroy - Destroy hilite resources.

        int
        hilite_destroy([int bufnum], [int type])

    Macro Description:
        The 'hilite_destroy()' primitive either removes hilite's of the
        specified 'type' otherwise if omitted all hilite's associated
        with the buffer 'bufnum'.

    Macro Parameter:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        type - Optional hilite type, if omitted all buffer hilite's are
            released.

    Macro Returns:
        The 'hilite_destroy()' primitive returns the number of hilite's
        removed, 0 if none were found, otherwise -1 on error.

    Macro Portability:
        A Grief extension, yet it was noted similar functionality has
        been introduced to CRisPEdit in parallel; compatibility as yet
        confirmed.

    Macro See Also:
        hilite_create, inq_hilite
 */
void
do_hilite_destroy(void)         /* int ([int bufnum], [int type]) */
{
    BUFFER_t *bp = buf_argument(1);
    int ret = 0;

    if (bp) {
        if (isa_undef(2)) {
            ret = hilite_zap(bp, TRUE);
        } else {
            ret = hilite_destroy(bp, get_xinteger(2, 0));
        }
        bp->b_hilite = NULL;
        if (ret > 0 && bp == curbp) {
            win_modify(WFHARD);
        }
    } else {
        ret = -1;
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: hilite_delete - Delete a hilite resource.

        int
        hilite_delete([int bufnum], int hilite)

    Macro Description:
        The 'hilite_delete()' primitive removes the stated hilite's
        'hilite' from the associated with the buffer 'bufnum'.

    Macro Parameter:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        hilite - Unique hilite identifier which are returned during the
            hilite's corresponding creation by <hilite_create>.

    Macro Returns:
        The 'hilite_delete()' primitive returns 1 if the hilite existed and
        was removed successfully, 0 if the hilite did not exist,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        hilite_create, hilite_destroy, inq_hilite
 */
void
do_hilite_delete(void)          /* int ([int bufnum], int hilite) */
{
    BUFFER_t *bp = buf_argument(1);
    int ret = 0;

    if (bp) {
        ret = hilite_delete(bp, get_xinteger(2, -1));
        if (ret > 0 && bp == curbp) {
            win_modify(WFHARD);
        }
    } else {
        ret = -1;
    }
    acc_assign_int(ret);
}


/*<<GRIEF>>
    Macro: inq_hilite - Retrieve a hilite definition.

        int
        inq_hilite([int bufnum], [int line], [int column],
            [int &attribute], [int &ident])

    Macro Description:
        The 'inq_hilite()' primitive retrieves details about the
        hilite resource at the specified position.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        line - Optional integer line number within the referenced buffer,
            if omitted the current line number is used.

        column - Optional integer column number within the referenced
            buffer, if omitted the current column number is used.

        attribute - Optional integer variable, if specified shall be
            populated with the hilite assigned attribute.

        indent - Optional integer variable, if specified shall be
            populated with the hilite user assigned user identifier.

    Macro Returns:
        The 'inq_hilite()' primitive returns the type of the active
        hilite and populates 'attribute' and 'ident', otherwise -1 and
        the arguments shall remain unmodified..

    Macro Portability:
        A Grief extension, yet it was noted similar functionality has
        been introduced to CRisPEdit in parallel; compatibility as
        yet confirmed.

    Macro See Also:
        hilite_create, hilite_destroy
 */
void
inq_hilite(void)                /* int ([int bufnum], [int line], [int column],
                                            [int &attribute], [int &ident], [int &hilite]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const int line = get_xinteger(2, *cur_line);
    const int col = get_xinteger(3, *cur_col);
    int ret = -1;

    if (bp) {
        const HILITELIST_t *hilites = &bp->b_hilites;
        HILITE_t *hp;

        TAILQ_FOREACH(hp, hilites, h_node) {
            if (hp->h_sline >= line && line <= hp->h_eline &&
                    (line != hp->h_sline || col >= hp->h_scol) &&
                    (line != hp->h_eline || col <= hp->h_ecol)) {
                /*
                 *  match/
                 *      interleaving/overlapping regions?
                 */
                argv_assign_int(4, hp->h_attr);
                argv_assign_int(5, hp->h_ident);
                argv_assign_int(6, hp->h_seqno);
                ret = hp->h_type;
                break;
            }
        }
    }
    acc_assign_int(ret);
}

/*end*/
