#include <edidentifier.h>
__CIDENT_RCSID(gr_kill_c,"$Id: kill.c,v 1.23 2021/10/18 13:17:45 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kill.c,v 1.23 2021/10/18 13:17:45 cvsuser Exp $
 * Scrap buffer.
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

#include "kill.h"                               /* public header */

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "echo.h"
#include "eval.h"                               /* get_...()/isa_...() */
#include "line.h"
#include "main.h"
#include "map.h"
#include "symbol.h"
#include "undo.h"


#define K_START         BUFFER_t *savedbp = curbp; set_curbp(scrbp)
#define K_END           set_curbp(savedbp)

static BUFFER_t *       scrap_bp = NULL;        /* system scrap buffer */

static int              scrtype;
static int              scrinsnewline = 0;
static BUFFER_t *       scrbp = NULL;           /* current buffer */
static LINE_t *         scrlp;


/*  Function:           inq_scrap
 *      inq_scrap primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_scrap - Obtain the scrap buffer identifier.

        int
        inq_scrap([int &last], [int &type])

    Macro Description:
        The 'inq_scrap()' primitive retrieves the buffer identifier of the
        current scrap buffer.

    Macro Parameters:
        last - Not used; provided for BRIEF compatibility. If *false*
            the last newline in the scrap is not considered part of the
            scrap, otherwise *true* it is considered part of the scrap.

        type - Optional integer variable reference, if stated shall be
            populated with the buffers mark type.

    Macro Returns:
        The 'inq_scrap()' primitive returns the scrap buffer identifier.

    Macro Portability:
        n/a

    Macro See Also:
        copy, cut, paste, inq_scrap, set_scrap_info
 */
void
inq_scrap(void)                 /* int ([int insert_newline], [int type]) */
{
    if (isa_integer(1)) {
        sym_assign_int(get_symbol(1), scrinsnewline);
    }
    if (isa_integer(2)) {
        sym_assign_int(get_symbol(2), scrtype);
    }
    acc_assign_int((accint_t) scrap_bp->b_bufnum);
}


/*  Function:           do_set_scrap_info
 *      set_scrap_info primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_scrap_info - Set the scrap buffer details.

        void
        set_scrap_info(
            [int last], [int type], [int bufnum])

    Macro Description:
        The 'set_scrap_info()' primitive specifies the mark type and
        newline handling to be applied against the scrap buffer.

    Macro Parameters:
        last - Not used; provided for BRIEF compatibility. If *false*
            the last newline in the scrap is not considered part of the
            scrap, otherwise *true* it is considered part of the scrap.

        type - Optional integer value, specifies the mark-type to be
            applied to the scrap buffer.

        bufnum - Optional integer buffer identifier, if stated changes
            the current scrap buffer to the specified buffer. The
            resulting buffer shall be marked with the buffer flag
            *BF_SCRAPBUF*.

    Macro Returns:
        The 'set_scrap_info()' primitive returns 0 on success, otherwise -1
        on error, for example in invalid buffer.

    Macro Portability:
        n/a

    Macro See Also:
        copy, cut, paste, inq_scrap, set_scrap_info
 */
void
do_set_scrap_info(void)         /* int ([int insert_newline], [int type], [int bufnum]) */
{
    int ret = 0;

    if (isa_integer(1)) {
        scrinsnewline = get_xinteger(1, 0);
    }

    if (isa_integer(2)) {
        const int nscrtype = get_xinteger(2, 0);;

        switch (nscrtype) {
        case MK_NORMAL:
        case MK_COLUMN:
        case MK_LINE:
        case MK_NONINC:
            scrtype = nscrtype;
            break;
        case MK_NONE:
        default:
            ret = -1;
            break;
        }
    }

    if (isa_integer(3)) {
        BUFFER_t *bp;

        if (NULL == (bp = buf_lookup(get_xinteger(3, 0)))) {
            ewprintf("set_scrap_info: no such buffer");
            ret = -1;
        } else {
            BFSET(bp, BF_SCRAPBUF);
            k_set(bp);
        }
    }

    acc_assign_int((accint_t) ret);
}


void
k_init(BUFFER_t *bp)
{
    scrap_bp = bp;
    BFSET(scrap_bp, BF_READ);
    BFSET(scrap_bp, BF_SYSBUF);
    BFSET(scrap_bp, BF_SCRAPBUF);
    k_set(NULL);                                /* scrap */
}


/*
 *  k_set ---
 *      set the current scrap buffer.
 */
BUFFER_t *
k_set(BUFFER_t *bp)
{
    BUFFER_t *oscrbp = scrbp;

    scrbp = (bp ? bp : scrap_bp);               /* user or system */
    k_seek();
    return oscrbp;
}


int
k_isscrap(const BUFFER_t *bp)
{
    return (bp == scrap_bp);
}


/*
 *  k_delete ----
 *      delete the scrap buffer content
 */
void
k_delete(int n)
{
    register const LINE_t *lp;
    FSIZE_t nbytes = 0;

    K_START;
    scrtype = n;
    TAILQ_FOREACH(lp, &scrbp->b_lineq, l_node) {
        nbytes += llength(lp) + 1;              /* NEWLINE */
    }
    *cur_line = 1;
    *cur_col = 1;
    if (nbytes > 0)
        --nbytes;
    if (nbytes)
        u_insert(nbytes, 0);
    buf_clear(scrbp);
    K_END;
}


/*
 *  k_write ----
 *      write into the current scrap buffer.
 */
void
k_write(const char *buf, int cnt)
{
    K_START;
    linsert(buf, (uint32_t) cnt, FALSE);
    K_END;
}


/*
 *  k_newline ---
 *      write a newline into the scrap buffer.
 */
void
k_newline(void)
{
    K_START;
    lnewline();
    K_END;
}


/*
 *  k_type ---
 *      return the scrap buffer type.
 */
int
k_type(void)
{
    return scrtype;
}


int
k_insnewline(void)
{
    return scrinsnewline;
}


void
k_seek(void)
{
    scrlp = lhead(scrbp);                       /* NEWLINE */
}


int
k_read(const char **cpp)
{
    int length = -1;

    if (scrlp) {                                /* NEWLINE */
        length = (int) llength(scrlp);
        *cpp = (const char *) ltext(scrlp);
        scrlp = lforw(scrlp);
    }
    return length;
}


/*  Function:           k_numlines
 *      Return the line count within the scrap buffer.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Line number
 */
int
k_numlines(void)
{
    if (scrbp) {
        return scrbp->b_numlines;
    }
    return -1;
}


void
k_undo(void)
{
    K_START;
    do_undo(0);
    K_END;
}


void
k_end(void)
{
    K_START;
    u_chain();
    K_END;
}
/*end*/
