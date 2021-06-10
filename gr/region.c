#include <edidentifier.h>
__CIDENT_RCSID(gr_region_c,"$Id: region.c,v 1.29 2021/06/10 06:13:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: region.c,v 1.29 2021/06/10 06:13:02 cvsuser Exp $
 * Region primitives.
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

#include "accum.h"
#include "anchor.h"
#include "asciidefs.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"                               /* get_xinteger(), get_str() .. */
#include "file.h"
#include "kill.h"
#include "line.h"
#include "main.h"
#include "map.h"
#include "region.h"
#include "system.h"
#include "undo.h"
#include "window.h"

struct region_process;

typedef int (*region_transfer_t)(struct region_process *rd, const void *buf, unsigned len);

typedef struct region_process {
    region_transfer_t   rd_func;                /* interface function */
    const char *        rd_msg;                 /* optional percentage message */
    char *              rd_base;
    char *              rd_cursor;
    unsigned            rd_length;
} region_process_t;

static void             sys_paste_callback(const char *text, int len);

static int              writestr(region_process_t *rd, const void *buf, unsigned len);
static int              writescrape(region_process_t *rd, const void *buf, unsigned len);
static int              writefile(region_process_t *rd, const void *buf, unsigned len);

static FSIZE_t          region_size(const REGION_t *r);
static int              region_transfer(const REGION_t *r, region_process_t *rd);
static FSIZE_t          region_process(const REGION_t *r, struct region_process *rd);

static int              textstart(const LINE_t *lp, int line, int col);
static int              textend(const LINE_t *lp, int line, int col);


int
region_get(const int del, const int undo, const ANCHOR_t *src, REGION_t *r)
{
    ANCHOR_t a;

    /*
     *  retrieve logical arena
     */
    if (NULL != src) {
        a = *src;                               /* normalize */
    } else {
        if (! anchor_get((curwp && curwp->w_bufp == curbp) ? curwp : NULL, NULL, &a)) {
            errorf("No marked block.");
            return FALSE;
        }
    }

    /*
     *  prime region
     */
    r->r_type      = a.type;
    r->r_savedcol  = *cur_col;
    r->r_startline = a.start_line;
    r->r_endline   = a.end_line;
    r->r_startcol  = a.start_col;
    r->r_endcol    = a.end_col;

    if (a.start_line > a.end_line ||            /* validate */
        (a.start_line == a.end_line &&
            ((a.type == MK_NORMAL && a.start_col > a.end_col) ||
             (a.type == MK_COLUMN && a.start_col > a.end_col) ||
             (a.type == MK_NONINC && a.start_col > a.end_col + 1))))
    {
        errorf("Invalid region (%d = %d/%d, %d/%d).",
            a.type, a.start_line, a.end_line, a.start_col, a.end_col);
        return FALSE;
    }

    /*
     *  prime scape
     */
    if (undo || del) {
        u_scrap();
    }
    if (del) {
        k_delete(r->r_type);
    }

    /*
     *  determine region size, in bytes
     */
    r->r_size = region_size(r);
    trace_ilog("region_get(type:%d, line:%d,col:%d to line:%d,col:%d) = size:%d\n",
        a.type, a.start_line, a.start_col, a.end_line, a.end_col, (int)r->r_size);
    return TRUE;
}


/*  Functon:            region_size
 *      Determine the size of the region, in bytes.
 *
 *  Parameters:
 *      r - Region specification.
 *
 *  Returns:
 *      Region size, in bytes.
 */
static FSIZE_t
region_size(const REGION_t *r)
{
    return region_process(r, NULL);
}


/*  Function:           region_transfer
 *      Transfer the region using the given callback interface.
 *
 *  Parameters:
 *      r - Region specification.
 *      rdata - Processing callback data.
 *
 *  Returns:
 *      Size of the region in bytes.
 */
static int
region_transfer(const REGION_t *r, region_process_t *rdata)
{
    FSIZE_t size;

    assert(r->r_size > 0);
    assert(rdata->rd_func);
    size = region_process(r, rdata);
    assert(size == r->r_size);
    return TRUE;
}


/*  Function:           region_process
 *      Common functionality utilised during region processing.
 *
 *  Parameters:
 *      r - Region speicfication.
 *      rdata - Optional processing callback data.
 *
 *  Returns:
 *      Size of the region in bytes.
 */
static FSIZE_t
region_process(const REGION_t *r, region_process_t *rdata)
{
    region_transfer_t func = (rdata ? rdata->rd_func : NULL);
    const int col = r->r_startcol, end_col = r->r_endcol;
    int line = r->r_startline, end_line = r->r_endline;
    FSIZE_t size = 0;

    ED_TRACE3(("region_process(type:%d,line:%d-%d,col:%d-%d)\n", \
        r->r_type, line, end_line, col, end_col))

    if (MK_COLUMN == r->r_type ||
            (MK_LINE != r->r_type && line == end_line)) {
        /*
         *  column/
         *      foreach(line)
         */
        if (end_line > curbp->b_numlines) {
            end_line = curbp->b_numlines;       /* limit to buffer */
        }

        while (line <= end_line) {
            const LINE_t *lp = vm_lock_line(line);
            const int start = textstart(lp, line, col);
            const int end = textend(lp, line, end_col);
            const int len = end - start;

            size += (len + 1);
            if (func) {
                if (len) {                      /* data */
                    (*func)(rdata, ltext(lp) + start, len);
                }
                (*func)(rdata, NULL, 0);        /* newline */

                if (rdata->rd_msg && r->r_size) {
                    percentage(PERCENTAGE_BYTES,
                        (accuint_t)(r->r_size - size), r->r_size, rdata->rd_msg, "region");
                }
            }
            ED_TRACE3(("-> line(line:%d, len:%d)\n", line, len))
            vm_unlock(line);
            ++line;
        }

    } else {
        /*
         *  NORMAL, NONINC and LINE
         *
         *   a) First-line
         *   b) None or more full lines
         *   c) Last-line
         */
        const int numlines = curbp->b_numlines; /* buffer limit */

        if (MK_LINE != r->r_type) {
            if (line <= numlines) {             /* NEWLINE */
                const LINE_t *lp = vm_lock_line(line);
                const int start = textstart(lp, line, col);
                const int end = llength(lp);
                const int len = end - start;

                size += (len + 1);
                if (func) {
                    if (len) {                  /* data */
                        (*func)(rdata, ltext(lp) + start, len);
                    }
                    (*func)(rdata, NULL, 0);    /* newline */
                }
                ED_TRACE3(("-> first-line(line:%d, len:%d)\n", line, len))
                vm_unlock(line);
            }
            --end_line;
            ++line;
        }
                                                /* NEWLINE */
        while (line <= end_line && line <= numlines) {
            const LINE_t *lp = vm_lock_line(line);
            const int len = llength(lp);

            size += (len + 1);
            if (func) {
                if (len) {                      /* data */
                    (*func)(rdata, ltext(lp), len);
                }

                (*func)(rdata, NULL, 0);        /* newline */

                if (rdata->rd_msg && r->r_size) {
                    percentage(PERCENTAGE_BYTES,
                        (accuint_t)(r->r_size - size), r->r_size, rdata->rd_msg, "region");
                }
            }
            ED_TRACE3(("-> line(line:%d, len:%d)\n", line, len))
            vm_unlock(lp);
            ++line;
        }

        if (MK_LINE != r->r_type) {
            if (line <= numlines) {             /* NEWLINE */
                const LINE_t *lp = vm_lock_line(line);
                const int len = textend(lp, line, end_col);

                size += (len + 1);
                if (func) {
                    /*
                     *  XXX - new-line should be optional/omitted, yet requires multiple macro
                     *      changes for example 'sort' which assumes a new-line is transferred.
                     */
                    if (len) {                  /* data */
                        (*func)(rdata, ltext(lp), len);
                    }
                    (*func)(rdata, NULL, 0);    /* newline */
                }
                ED_TRACE3(("-> last-line(line:%d, len:%d)\n", line, len))
                vm_unlock(line);
                ++line;
            }
        }
    }

    ED_TRACE3(("==> size:%d\n", (int) size))
    return size;
}


static int
textstart(const LINE_t *lp, int line, int col)
{
    return line_offset_const(lp, line, (col > 1 ? col : 1), LOFFSET_FIRSTBYTE);
}


static int
textend(const LINE_t *lp, int line, int col)
{
    return line_offset_const(lp, line, (col > 1 ? col : 1), LOFFSET_LASTBYTE);
}


static int
writestr(region_process_t *rdata, const void *buf, unsigned len)
{
    if (NULL == buf) {                          /* newline */
        *rdata->rd_cursor++ = '\n';

    } else {                                    /* line content */
        (void) memcpy(rdata->rd_cursor, buf, len);
        rdata->rd_cursor += len;
    }
    assert(rdata->rd_base + rdata->rd_length < rdata->rd_cursor);
    return 0;
}


static int
writescrape(region_process_t *rdata, const void *buf, unsigned len)
{
    __CUNUSED(rdata)
    if (NULL == buf) {                          /* newline */
        k_newline();
    } else {                                    /* line content */
        k_write(buf, len);
    }
    return 0;
}


static int
writefile(region_process_t *rdata, const void *buf, unsigned len)
{
    FILE *file = (FILE *) rdata->rd_cursor;

    if (NULL == buf) {                          /* newline */
        fwrite("\n", 1, 1, file);

    } else {                                    /* line content */
        fwrite(buf, len, 1, file);
    }
    return 0;
}


int
region_delete(const REGION_t *region)
{
    if (region->r_size > 0) {
        u_dot();

        *cur_line = region->r_startline;
        *cur_col = region->r_startcol;
        ldelete(region->r_size);
        if (MK_LINE == region->r_type) {
            *cur_col = region->r_savedcol;
        }
    }
    return TRUE;
}


int
region_copy(const char *msg, const REGION_t *r)
{
    region_process_t rdata = {writescrape};

    if (r->r_size <= 0) {
        return FALSE;
    }
    k_seek();
    rdata.rd_msg = msg;
    region_transfer(r, &rdata);
    k_end();
    return TRUE;
}


int
region_write(FILE *fp, const char *msg, const REGION_t *r)
{
    region_process_t rdata = {writefile};

    if (r->r_size <= 0) {
        return 0;
    }
    rdata.rd_cursor = (void *)fp;
    rdata.rd_msg = msg;
    return region_transfer(r, &rdata);
}


/*  Function:           do_transfer
 *      transfer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: transfer - Buffer to buffer transfer.

        int
        transfer(int bufnum,
              int sline, [int scolumn], int eline, [int ecolumn])

    Macro Description:
        The 'transfer()' primitive moves the specified region from the
        buffer 'bufnum' into the current buffer.

        All characters in the source buffer between the start and end
        positions inclusive are included in the block.

        The primitive has two forms determined by the number of
        coordinates arguments being either four or two, as followings:

            o start line/column 'sline', 'scol' and end
                line/column 'eline' and 'ecol'.

            o start line 'sline' and end line 'eline'.

        Unlike the original BRIEF implementation, this version is more
        forgiving and is provided for compatibility; it is usually far
        easier to use the region macros.

    Macro Parameters:
        bufnum - Buffer identifier of the source.
        sline - Starting line.
        scolumn - Starting column.
        eline - Ending line.
        ecolumn - Ending column.

    Macro Returns:
        The 'transfer()' primitive returns non-zero on success,
        otherwise zero or less on error.

    Macro Portability:
        The short form of 'transfer' is an extension yet less error
        prone and more convenient interface.

    Macro See Also:
        copy, cut, paste
 */
void
do_transfer(void)               /* int (int bufnum, int sline, int scolumn, int eline, int ecolumn)
                                    or int (int bufnum, int sline, int eline) */
{
    static const char who[] = "transfer";
    BUFFER_t *bp;
    int ret = 0;

    if (NULL == (bp = buf_lookup(get_xinteger(1, 0)))) {
        ewprintf("%s: no such buffer", who);
        ret = -1;

    } else if (file_rdonly(curbp, who)) {
        ret = -1;

    } else if (curbp != bp) {
        BUFFER_t *saved_scrap, *saved_bp = curbp;
        ANCHOR_t a;
        REGION_t r;

        saved_scrap = k_set(curbp);             /* destination */
        curbp = bp;                             /* source */
        set_hooked();

        if (isa_undef(4) && isa_undef(5)) {     /* line mode */
            a.type = MK_LINE;
            a.start_line = get_xinteger(2, 0);
            a.end_line = get_xinteger(3, 0);
            a.start_col = a.end_col = 0;

        } else {                                /* normal */
            a.type = MK_NORMAL;
            a.start_line = get_xinteger(2, 0);
            a.start_col = get_xinteger(3, 0);
            a.end_line = get_xinteger(4, 0);
            a.end_col = get_xinteger(5, 0);
        }

        if (a.start_line > a.end_line) {        /* .. swap */
            int t = a.end_line; a.end_line = a.start_line; a.start_line = t;
        }

        if (a.start_col > a.end_col) {          /* .. swap */
            int t = a.end_col;  a.end_col = a.start_col;  a.start_col = t;
        }

        if (region_get(FALSE, FALSE, &a, &r)) {
            if (region_copy("Transfer", &r)) {
                ret = 1;
            }
        }

        k_set(saved_scrap);                     /* restore default scrap */
        curbp = saved_bp;
        set_hooked();
    }

    acc_assign_int(ret);
}


/*  Function:           inq_mark_size
 *      inq_mark_size primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *  TODO:
 *      Move to archor.c
 *
 *<<GRIEF>>
    Macro: inq_mark_size - Retrieve size of marked region.

        int
        inq_mark_size()

    Macro Description:
        The 'inq_mark_size()' primitive determines the number of
        characters what are in the currently marked region; this
        represents the the length of a string which would be necessary
        to hold its content, see <get_region>.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_mark_size()' primitive returns the number of
        characters in the currently marked region, otherwise 0 if
        there is no current region.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        mark, drop_anchor, inq_marked
 */
void
inq_mark_size(void)             /* () */
{
    REGION_t r;

    if (NULL == curbp || NULL == curbp->b_anchor ||
            !region_get(FALSE, FALSE, NULL, &r)) {
        acc_assign_int((accint_t) 0);
    } else {
        acc_assign_int((accint_t) r.r_size);
    }
}


/*  Function:           do_paste
 *      paste primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: paste - Insert scrap buffer at cursor location.

        int
        paste([int syspaste = FALSE])

    Macro Description:
        The 'paste()' primitive insert the contents of the scrap
        buffer at the current cursor location.

        The buffer is pasted according to the type of the mark which
        created it:

         o Normal or non-exclusive mark, the scrape is inserted
         before the current position.

         o Line mark, the scrape is inserted before the current line.

    Macro Parameters:
        syspaste - Optional boolean flag, is specified and is
            non-zero, then the text contents of the operating system
            buffer shall (e.g. clipboard under WIN32) shall be
            inserted into the current buffer where the cursor is
            located, if the feature is supported/available.

    Macro Returns:
        The 'paste()' primitive returns 1 on success, otherwise 0 on
        failure.

    Macro Portability:
        n/a

    Macro See Also:
        cut, copy, inq_scrap, set_scrap_info
 */
void
do_paste(void)                  /* ([int syspaste = FALSE]) */
{
    const int syspaste = get_xinteger(1, FALSE);
    int ret = 0;

    if (rdonly()) {
        acc_assign_int((accint_t) 0);
        return;
    }

    lchange(WFEDIT, 0);
    if (syspaste) {
        if ((ret = sys_paste(sys_paste_callback)) >= 0) {
            win_modify(WFEDIT);
        }

    } else {
        int col = *cur_col;
        const char *cp;
        int numlines, mark_type;
        int n, lineno = 0;

        k_seek();
        u_dot();
        if (MK_LINE == (mark_type = k_type())) {
            *cur_col = 1;
        }

        win_modify(WFEDIT);
        numlines = k_numlines();

        while ((n = k_read(&cp)) >= 0) {
            if (ret) {
                lnewline();
            } else {
                ret = 1;
            }
            if (n) {
                linsert(cp, (uint32_t) n, FALSE);
            }
            ++lineno;
            percentage(PERCENTAGE_LINES, lineno, numlines, "Paste", "command");
        }

#if (XXX_SCRAP_NEWLINE)
        if (k_insert_newline() && ret) {
            lnewline();
        }
#endif

        win_modify(WFEDIT);
        if (MK_LINE == mark_type) {
            *cur_col = col;
        }
    }

    if (ret == -1) {
        infof("Scrap paste error.");
    } else if (ret == 0) {
        infof("No scrap to insert.");
    } else {
        infof("Scrap inserted.");
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_copy
 *      copy primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: copy - Copy marked area to scrap.

        int
        copy([int append = FALSE], [int keep = FALSE])

    Macro Description:
        The 'copy()' primitive copies the content of the currently
        marked region to the scrap buffer, and releases the marked
        region on completion.

    Macro Parameters:
        append - Optional integer, if non-zero the copied region
            shall be appended to the scrap instead of replacing the
            scrap content.

        keep - Optional integer, if non-zero the marked region is
            retained otherwise the anchor is released.

    Macro Returns:
        The 'copy()' primitive returns 1 on success, otherwise 0 on
        error.

    Macro Portability:
        n/a

    Macro See Also:
        cut, delete_block, inq_scrap, paste, transfer
 */
void
do_copy(void)                   /* int ([int append = FALSE], [int keep = FALSE], [int syscopy = FALSE]) */
{
    const int append = get_xinteger(1, 0);
    const int keep = get_xinteger(2, FALSE);
 /* const int syscopy = get_xinteger(3, FALSE); TODO */
    REGION_t r;
    int ret = 0;

    if (region_get(!append, TRUE, NULL, &r)) {
        if (region_copy("Copying", &r)) {
            infof("Block copied to scrap.");
            if (!keep) {
                anchor_raise();
            }
            ret = 1;
        }
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_cut
 *      cut primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: cut - Cut marked area to scrap.

        int
        cut([int append = FALSE], [int syscopy = FALSE])

    Macro Description:
        The 'cut()' primitive moves the content of the currently
        marked region to the scrap buffer, and releases the marked
        region on completion.

    Macro Parameters:
        append - Optional integer, if non-zero the copied region
            shall be appended to the scrap instead of replacing the
            scrap content.

        keep - Optional integer, if non-zero the marked region is
            retained otherwise the anchor is released.

    Macro Returns:
        The 'cut()' primitive returns 1 on success, otherwise 0 on
        error.

    Macro Portability:
        n/a

    Macro See Also:
        paste, delete_block, inq_scrap, paste, transfer
 */
void
do_cut(void)                    /* int ([int append = FALSE], [int keep = FALSE], [int syscopy = FALSE]) */
{
    const int append = get_xinteger(1, 0);
    const int keep = get_xinteger(2, FALSE);
 /* const int syscopy = get_xinteger(3, FALSE); TODO */
    REGION_t r;
    int ret = 0;

    if (!rdonly()) {
        if (region_get(! append, TRUE, NULL, &r)) {
            if (region_copy("Cutting", &r)) {
                region_delete(&r);
                infof("Block deleted to scrap.");
                if (!keep) {
                    anchor_raise();
                }
            }
        }
    }
    acc_assign_int((accint_t) ret);
}


/*  Function:           do_get_region
 *      get_region primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_region - Retrieve marked region content.

        int
        get_region([int bufnum])

    Macro Description:
        The 'get_region()' primitive is similar to copy yet is copies
        the content of the current marked region and returns the
        result as a string.

    Macro Parameters:
        bufnum - Optional window identifier, if omitted the current
            buffer is referenced.

    Macro Returns:
        The 'get_region()' primitive returns the content of the current
        marked region as a string, otherwise an empty string.

    Macro Portability:
        n/a

    Macro See Also:
        cut, paste, copy, inq_marked
 */
void
do_get_region(void)             /* string ([int bufnum], [int syscopy = FALSE]) */
{
    BUFFER_t *bp = buf_argument(1);
 /* const int syscopy = get_xinteger(2, FALSE); TODO */
    ANCHOR_t a;
    REGION_t r;
    int ret;

    ret = anchor_get(NULL, bp, &a);
    if (ret) {
        if (region_get(FALSE, FALSE, &a, &r)) {
            region_process_t rdata = {writestr};
            char *cursor;

            cursor = acc_expand(r.r_size + 1);  /* size accumulator */

            rdata.rd_func = writestr;
            rdata.rd_base = cursor;
            rdata.rd_cursor = cursor;
            rdata.rd_length = r.r_size;

            region_transfer(&r, &rdata);
            acc_assign_str(cursor, r.r_size + 1);
        }
    }

    if (!ret) acc_assign_str("", 0);
}


/*  Function:           do_inside_region
 *      inside_region primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF-TODO>>
    Macro: inside_region - Test region location.

        int
        inside_region([int winnum], [int line], [int col])

    Macro Description:
        The 'inside_region()' primitive determines whether the
        specified coordinates correspond to a position either before,
        inside, or after the currently selected region of a given
        window.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

        line - Optional line, if omitted the current line is
            referenced.

        col - Optional column, if omitted the current column is
            referenced.

    Macro Returns:
        The 'inside_region()' primitive returns the location of the
        cursor relative to the active region, if any.

(start table)
        [Value  [Definition                                 ]
        0       no region selected.
        1       cursor is before the region.
        2       cursor is inside the region.
        3       cursor is after the region.
        4       cursor is to the left of a column region.
        5       cursor is to the right of a column region.
(end table)

    Macro See Also:
        inq_marked, get_region
 */
void
do_inside_region(void)          /* int ([int winnum], [int line], [int col]) */
{
#define IR_NONE         0
#define IR_BEFORE       1
#define IR_INSIDE       2
#define IR_AFTER        3
#define IR_LEFT         4
#define IR_RIGHT        5

    WINDOW_t *wp = window_argument(1);
    LINENO line = get_xinteger(2, *cur_line);
    LINENO col = get_xinteger(3, *cur_col);
    int ret = IR_NONE;
    ANCHOR_t a;
    REGION_t r;

    if (anchor_get(wp, NULL, &a)) {
        if (region_get(FALSE, FALSE, &a, &r)) {
            if (line < a.start_line) {
                ret = IR_BEFORE;

            } else if (line > a.end_line) {
                ret = IR_AFTER;

            } else {
                switch (a.type) {
                case MK_NORMAL:
                case MK_NONINC:
                    if (line == a.start_line && col < a.start_col) {
                        ret = IR_BEFORE;
                    } else if (line == a.end_line && col > a.end_col) {
                        ret = IR_AFTER;
                    }
                    break;
                case MK_COLUMN:
                    if (col < a.start_col) {
                        ret = IR_LEFT;
                    } else if (col > a.end_col) {
                        ret = IR_RIGHT;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    acc_assign_int((accint_t) ret);
}


static void
sys_paste_callback(const char *text, int len)
{
    const char *nl;

    len = (len == -1 ? (int)strlen(text) : len);
    while (len > 0) {
        if (NULL != (nl = strchr(text, ASCIIDEF_CR)) ||
                NULL != (nl = strchr(text, ASCIIDEF_LF))) {
            int n = nl - text;

            if (nl[0] == ASCIIDEF_CR && nl[1] == ASCIIDEF_LF) {
                len -= n + 2;
                nl += 2;
            } else {
                len -= n + 1;
                nl += 1;
            }
            if (n) {
                linsert(text, (uint32_t)n, TRUE);
            } else {
                lnewline();
            }
            text = nl;
        } else {
            linsert(text, (uint32_t)len, FALSE);
            len = 0;
        }
    }
}
/*end*/
