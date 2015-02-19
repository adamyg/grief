#include <edidentifier.h>
__CIDENT_RCSID(gr_line_c,"$Id: line.c,v 1.42 2015/02/11 23:25:13 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: line.c,v 1.42 2015/02/11 23:25:13 cvsuser Exp $
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

#if !defined(ED_LEVEL)
#define ED_LEVEL 2                              /* debug/trace level */
#endif

#include <editor.h>
#include "../libvfs/vfs.h"
#include "../libchartable/libchartable.h"

#include "accum.h"                              /* acc_...() */
#include "anchor.h"                             /* anchor_...() */
#include "asciidefs.h"                          /* ASCIIDEF_... */
#include "buffer.h"
#include "builtin.h"
#include "chunk.h"
#include "cmap.h"
#include "debug.h"
#include "echo.h"                               /* infof() */
#include "eval.h"                               /* get_...() */
#include "file.h"
#include "hilite.h"
#include "line.h"
#include "main.h"
#include "map.h"
#include "mchar.h"                              /* mchar_...() */
#include "register.h"                           /* trigger() */
#include "ruler.h"                              /* next_..._stop() */
#include "system.h"
#include "undo.h"
#include "window.h"


/*
 *  NBLOCK and NBLOCK1 define the blocking sizes used during line buffer
 *  allocation. Lines are firstly allocated in NBLOCK chunks until NBLOCK1
 *  in size, at which time lines are then allocated on NBLOCK1 chunks.
 */
#define NBLOCK          16                      /* line block chunk size */
#define NBLOCK1         256                     /* line block chunk size for large allocations. */

#define LBLK(x)         ((int) (NBLOCK - 1 + (x)) & ~(NBLOCK-1))
#define LBLK1(x)        ((int) (NBLOCK1 - 1 + (x)) & ~(NBLOCK1-1))

static const char *     x_errmsg = "Cannot allocate %d bytes";

#define LMAGIC_TEXT     0x7f
#define LMAGIC_ATTR     0x42

#define lalloced(lp)    ((lp)->l_size)


/*
 *  Size of lines for binary files.
 */
static LINE_t           static_line;
LINE_t *                x_static_line = &static_line;

static vmpool_t         x_vmlines;

static int              newlinedot(LINE_t *lp, LINENO tdoto);
static void             window_adjust(void);

static int              line_size(LINE_t *lp, LINENO size);


/*  Function:           line_init
 *       Run-time initialisation
 *
 *  Parameters:
 *       void
 *
 *  Returns:
 *      void
 */
void
line_init(void)
{
    trace_log("sizeof(LINE_t)=%d\n", sizeof(LINE_t));
    vm_init(&x_vmlines, sizeof(LINE_t), 0);
}


/*  Function:           line_alloc
 *      Allocate and initialise a LINE_t construct.
 *
 *  Parameters:
 *      size - Required length of line buffer.
 *      hasattrs - Determines whether the line supports attributes.
 *      incore - Storage is core memory (L_INCORE), otherwise file (L_INFILE).
 *
 *  Returns:
 *      Address of allocated line object.
 */
LINE_t *
line_alloc(LINENO size, int hasattrs, int incore)
{
    LINE_t *lp;

    assert(size >= 0);

    if (NULL != (lp = line_new())) {
        lp->l_iflags |= (incore ? LI_INCORE : LI_INFILE);
        if (hasattrs) {
            liflagset(lp, LI_ATTRIBUTES);
            assert(incore);
        }

        if (size > 0) {
            if (! line_size(lp, size)) {
                line_free(lp);
                lp = NULL;
            }
        }
    }
    return lp;
}


/*  Function:           lsize
 *      Verify the line size to be equal or greater then 'size' bytes in length,
 *      resizing the length buffer if so required. Any newly allocated storage shall be
 *      zeroed.
 *
 *  Parameters:
 *      lp - Line pointer.
 *      size - Total line length required.
 *
 *  Returns:
 *      TRUE if successful or line did not need resizing, otherwise FALSE.
 */
int
lsize(LINE_t *lp, LINENO size)
{
    const LINENO length = llength(lp),
            more = (size < length ? 0 : size - length);
    int ret;

    ED_TRACE(("lsize(length:%d,size:%d,more:%d)\n", length, size, more))

    if (FALSE != (ret = ledit(lp, more))) {
        assert(size <= lalloced(lp));
    }
    return ret;
}


/*  Function:           lexpand
 *      Expand the size a line buffer increasing the current length of 'size' bytes.
 *
 *      Data within the line shall be moved to the right creating creating a hole of
 *      'len' bytes starting at the offset 'dot'.
 *
 *  Parameters:
 *      lp - Line pointer.
 *      dot - Insertion point.
 *      size - Length required from insertion point.
 *
 *  Returns:
 *      TRUE if successful or line did not need conversion, otherwise FALSE.
 */
int
lexpand(LINE_t *lp, LINENO dot, LINENO size)
{
    const LINENO length = llength(lp);
    int ret;

    assert(dot >= 0);
    assert(dot <= length);
    assert(size > 0);

    ED_TRACE(("lexpand(length:%d,dot:%d,size:%d)\n", length, dot, size))

    if (FALSE != (ret = ledit(lp, size))) {
        assert(length + size <= lalloced(lp));
        line_move(lp, dot + size, dot, length - dot);
        lp->l_used += size;
    }
    return ret;
}


/*  Function:           ledit
 *      Prepare the specified line object to the edited, convert a line from a FILE buffer
 *      reference to a normal line buffer if needed.
 *
 *  Parameters:
 *      lp - Line pointer.
 *      more - Additional line size required, if any.
 *
 *  Returns:
 *      TRUE if successful or line has already been converted, otherwise FALSE.
 */
int
ledit(LINE_t *lp, LINENO more)
{
    const lineflags_t iflags = liflags(lp);
    const LINENO length = llength(lp), size = lalloced(lp),
            left = (size - length);
    const LINECHAR *text = ltext(lp);
    int ret;

    assert(lp != x_static_line);
    assert(size >= 0);
    assert(size >= length);
    assert(more >= 0);

    ED_TRACE(("ledit(more:%d,size;%d,left:%d)\n", more, size, left))

    if ((more -= left) < 0) {
        more = 0;
    }

    if (0 == (iflags & LI_INFILE)) {
        /*
         *  INCORE image/
         *      expand if needed
         */
        ED_TRACE(("ledit(length:%d,size:%d,more:%d)->CORE\n", length, size, more))

        assert(iflags & LI_INCORE);
        assert(NULL == lp->l_chunk);
        if (text) {
            assert(size > 0);
            assert(length <= size);
            assert(0 == size || text);
            assert(0 == (LI_ATTRIBUTES & iflags) || lp->l_attr);
            assert(LMAGIC_TEXT == lp->l_text[size]);
            assert(NULL == lp->l_attr || LMAGIC_ATTR == lp->l_attr[size]);
        } else {
            assert(NULL == lp->l_attr);
        }
        ret = (more > 0 ? line_size(lp, size + more) : TRUE);

    } else {
        /*
         *  FILE image/
         *      migrate to INCORE and expand if needed
         */
        ED_TRACE(("ledit(length:%d,size:%d,more:%d)->FILE\n", length, size, more))

        assert(0 == (iflags & LI_INCORE));
        assert(0 == (iflags & LI_MODIFIED));
        assert(size == length);
        assert(0 == size || text);
        assert(NULL == lp->l_attr);
        assert(0 == size || NULL != lp->l_chunk);

        liflagclr(lp, LI_INFILE);
        liflagset(lp, LI_INCORE);
        lp->l_size = lp->l_used = 0;
        lp->l_text = NULL;

        if (FALSE == (ret = line_size(lp, length + more))) {
            lp->l_iflags = iflags;
            lp->l_size = length;
            lp->l_used = length;
            lp->l_text = (void *)text;

        } else {
            if (length) {
                line_set(lp, 0, (const char *)text, length, 0, 1);
                lp->l_used = length;
            }

            if (lp->l_chunk) {                  /* unhook chunk */
                chunk_unref(lp->l_chunk, curbp /*NULL*/);
                lp->l_chunk = NULL;
            }
        }
    }
    liflagset(lp, LI_MODIFIED);

    ED_TRACE(("==> length:%d,size:%d = %d\n", llength(lp), lalloced(lp), ret))
    return ret;
}


/*  Function:           lremove
 *      Remove the specified line from a buffer.
 *
 *  Parameters:
 *      buf - Buffer.
 *      line - Line number within the buffer.
 *
 *  Returns:
 *      nothing.
 */
void
lremove(BUFFER_t *bp, LINENO line)
{
    LINE_t *lp;

    lp = linepx(bp, line);
    if (lp) {
        lrelease(bp, lp, line);
    }
}


/*  Function:           lrelease
 *      Release a line from a buffer, given a pointer to the line.
 *
 *  Parameters:
 *      buf - Buffer.
 *      lp - Line pointer.
 *      line - Line number within the buffer.
 *
 *  Returns:
 *      void
 */
void
lrelease(BUFFER_t *bp, LINE_t *lp, LINENO line)
{
    register WINDOW_t *wp;
    LINE_t *next_line;

    /* Update the window line if the cursor is below the line being released. */
    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_bufp == bp) {                 /* XXX - optimise, based on attached count */
            if (wp->w_top_line > line) {
                --wp->w_top_line;
            }
            if (wp->w_line > line) {
                --wp->w_line;
                wp->w_col = 1;
            }
        }

    if (lp == bp->b_maxlinep) {
        bp->b_maxlength = 0;
        bp->b_maxlinep = NULL;
    }

    /* Update the buffer line if the cursor is below the line being released. */
    if (bp->b_line > line) {
        --bp->b_line;
        bp->b_col = 1;                          /* XXX - review, leave unchanged */
    }
    assert(bp->b_numlines >= 1);
    --bp->b_numlines;

    /* Unhook the line */
    next_line = lforw(lp);
    if (lp == bp->b_clinep) {
        assert(bp->b_cline == line);
        bp->b_clinep = next_line;
    }
    TAILQ_REMOVE(&bp->b_lineq, lp, l_node);     /* NEWLINE */

    /* Free storage */
    line_free(lp);
}


/*  Function:           lchange
 *      Register a line status change for the current line.
 *
 *  Parameters:
 *      flag - Status change, being one of the following;
 *
 *          o WFEDIT -  Editing (modification is any form).
 *          o WFDELL -  Deletion.
 *          o WFHARD -  Major change.
 *          o WFINSL -  Insertion.
 *
 *      count - Effected line count.
 *
 *  Returns:
 *      nothing.
 */
void
lchange(int flag, LINENO count)
{
    register WINDOW_t *wp;
    BUFFER_t *bp = curbp;
    LINENO line = *cur_line;

    trace_ilog("lchange(%d/%s)\n", flag,
           (WFHARD == flag ? "HARD" : WFTOP  == flag ? "TOP"  :
                WFPAGE == flag ? "PAGE" : WFMOVE == flag ? "MOVE" :
                    WFEDIT == flag ? "EDIT" : WFDELL == flag ? "DELL" :
                        WFINSL == flag ? "INSL" : "??"));

    /* Clear hilites and Update syntax markers */

    hilite_clear(bp, line /*count, flag*/);     /* clear affected hilites */

    if (WFHARD == flag) {
        line = 1;
        count = bp->b_numlines;
        bp->b_maxlinep = NULL;                  /* clear cache */

    } else {
        const LINE_t *lp;
        LINENO cline, eline;

        if (count < 1) count = 1;

        for (cline = line, eline = line + count; cline < eline; ++cline) {
            if (NULL != (lp = linep(cline))) {
                const LINENO length = llength(lp);

                if (bp->b_maxlinep == lp) {     /* length change? */
                    if (length < bp->b_maxlength) {
                        bp->b_maxlinep   = NULL;
                    } else {
                        bp->b_maxlength  = length;
                    }

                } else if (bp->b_maxlinep) {
                    if (length > bp->b_maxlength) {
                        bp->b_maxlength  = length;
                        bp->b_maxlinep   = (LINE_t *)lp;
                    }
                }
            }
        }
    }

    if (0 == bp->b_syntax_min || line < bp->b_syntax_min) {
        bp->b_syntax_min = line;                /* start of changed region */
    }

    if (0 == bp->b_syntax_max || line > (bp->b_syntax_max + count)) {
        bp->b_syntax_max = line + count;        /* end of changed region */
    }

    /* modification time and status */
    bp->b_ctime = time(NULL);

    if (0 == BFTST(bp, BF_CHANGED)) {
        trigger(REG_BUFFER_MOD);
        BFSET(bp, BF_CHANGED);
    }

    /* update attached window(s) */
    for (wp = window_first(); wp; wp = window_next(wp))
        if (wp->w_bufp == bp) {
            window_modify(wp, wp == curwp ? flag : flag == WFDELL ? WFHARD : flag);
        }
}


/*  Function:           llinepad
 *      Function to create new zero-length lines when we are inserting on a line thats past
 *      the end of the buffer.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      void
 */
int
llinepad(void)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    LINENO newlines, numlines = curbp->b_numlines;

    /*
     *  Is padding required ?
     */
    assert(cline > 0);
    assert(numlines >= 0);
    if (cline <= numlines) {
        ED_TRACE(("llinepad(cline:%d, numlines:%d) = 0\n", cline, numlines))
        return FALSE;
    }
    newlines = cline - numlines;
    assert(newlines > 0);

    ED_TRACE(("llinepad(count:%d, cline:%d, numlines:%d)\n", newlines, cline, numlines))

    /*
     *  Write out the undo information for these appended lines
     */
    u_dot();
    *cur_line = (numlines >= 1 ? (numlines + 1) : 1);   /*NEWLINE*/
    *cur_col  = 1;
    u_delete(newlines);
    *cur_col  = ccol;
    *cur_line = cline;

    /*
     *  Append new blank lines, cloning attributes
     */
    while (numlines < cline) {
        LINE_t *newlp;

        if (NULL == (newlp = line_alloc(0, BF2TST(curbp, BF2_ATTRIBUTES), TRUE))) {
            newlines = 0;
            break;
        }
        TAILQ_INSERT_TAIL(&curbp->b_lineq, newlp, l_node);
        ++numlines; --newlines;
    }

    curbp->b_numlines = numlines;
    assert(0 == newlines);
    lchange(WFHARD, 0);

    ED_TRACE(("==> numlines:%d\n", numlines))
    return TRUE;
}


/*  Function:           window_adjust
 *      Adjust window references post a line insertion.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
window_adjust(void)
{
    WINDOW_t *wp;

    for (wp = window_first(); wp; wp = window_next(wp)) {
        if (wp->w_bufp != curbp || wp == curwp) {
            continue;
        }
        if (wp->w_line      >= *cur_line) {
            ++wp->w_line;
        }
        if (wp->w_top_line  >= *cur_line) {
            ++wp->w_top_line;
        }
        if (wp->w_mined     >= *cur_line) {
            ++wp->w_mined;
        }
        if (wp->w_maxed     >= *cur_line) {
            ++wp->w_maxed;
        }
    }
}


/*  Function:           lrenumber
 *      Renumber the lines within the buffer, reordering the l_lineno field
 *      which represents the 'original line number'.
 *
 *      This function is called prior to buffer reload or after buffer
 *      write operations.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing.
 */
void
lrenumber(BUFFER_t *bp)
{
    register LINENO lineno = 0;
    register LINE_t *lp;

    ED_TRACE(("lrenumber()\n"))
    TAILQ_FOREACH(lp, &bp->b_lineq, l_node) {
        lp->l_oldlineno = ++lineno;
        liflagclr(lp, LI_MODIFIED);
    }
    assert(lineno == bp->b_numlines);
}


/*  Function:           linsertc
 *      High-level line interface.
 *
 *      Insert a character into the current buffer.
 *
 *  Parameters:
 *      ch - Character value.
 *
 *  Returns:
 *      TRUE on success otherwise FALSE.
 */
int
linsertc(int ch)
{
    const BUFFER_t *bp = curbp;
    char buffer[MCHAR_MAX_LENGTH + 1];
    mchar_iconv_t *iconv;

    ED_TRACE(("linsertc(%d/0x%x)\n", ch, (unsigned)ch))

    if (rdonly()) {
        return FALSE;
    }

    if ('\n' == ch) {
        lnewline();

    } else if (NULL != (iconv = bp->b_iconv)) {
        int len;                                /* MCHAR */

        if ((len = mchar_encode(iconv, ch, buffer)) > 0) {
            linsert((const char *)buffer, len, FALSE);

        } else {
            *((unsigned char *)buffer) = (unsigned char)ch;
            linsert((const char *)buffer, 1, FALSE);
        }

    } else {
        const int isutf8 = buf_isutf8(bp);      /* legacy/dialog buffer encoding */

        if (isutf8 && MCHAR_ISUTF8(ch)) {
            linsert((const char *)buffer, mchar_ucs_encode(ch, buffer), FALSE);

        } else {                                /* NORMAL */
            *((unsigned char *)buffer) = (unsigned char) ch;
            linsert(buffer, 1, FALSE);
        }
    }
    return TRUE;
}


/*  Function:           linserts
 *      High-level line interface.
 *
 *      Insert a character string into the current buffer, handling any embedded new lines.
 *
 *  Parameters:
 *      buffer - Character buffer.
 *      length - Length of the buffer in bytes.
 *
 *  Returns:
 *      The number of lines inserted.
 */
int
linserts(const char *buffer, int length)
{
    register LINENO len;
    int nline = 0;

    ED_TRACE(("linserts(length:%d) = [", length))
    ED_DATA((buffer, length, "]\n"))

    if (rdonly()) {
        return 0;
    }

    while (length > 0) {
        const char *cpend = buffer + length;
        register const char *cp;
        int nl = FALSE;

        for (cp = buffer; cp < cpend; ++cp) {
            if ('\n' == *cp) {                  /* new-line */
                nl = TRUE;
                break;
            }
        }
        len = cp - buffer;
        length -= len + 1;
        if (FALSE == linsert(buffer, len, nl)) {
            return -1;
        }
        buffer = cp + 1;
        nline += nl;
    }
    return nline;
}


/*  Function:           lnewline
 *      High-level line interface.
 *
 *      Insert a newline at the current cursor position.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      void
 */
void
lnewline(void)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    LINE_t *lp;

    ED_TRACE(("lnewline()\n"))
    if (rdonly()) {
        return;
    }
    llinepad();
    if (NULL != (lp = vm_lock_line(cline))) {
        assert(cline == *cur_line);
        assert(ccol == *cur_col);
        newlinedot(lp, line_offset2(lp, cline, ccol, LOFFSET_NORMAL_MATCH));
        vm_unlock(cline);
    }
}


/*  Function:           lwritec
 *      High-level line interface.
 *
 *      Writes a character value into the current buffer line.
 *
 *  Parameters:
 *      ch - Character value.
 *
 *  Returns:
 *      TRUE on success otherwise FALSE.
 */
int
lwritec(int ch)
{
    const BUFFER_t *bp = curbp;
    char buffer[MCHAR_MAX_LENGTH+1];
    mchar_iconv_t *iconv;
    int ret = TRUE;

    ED_TRACE(("lwritec(%d)\n", ch))

    assert(ch > 0);

    if (rdonly()) {
        return FALSE;
    }

    if ('\n' == ch) {
        lnewline();

    } else if (NULL != (iconv = bp->b_iconv)) {
        int len;                                /* MCHAR */

        if ((len = mchar_encode(iconv, ch, buffer)) > 0) {
            ret = lwrite((const char *)buffer, len, TRUE);

        } else {
            *((unsigned char *)buffer) = (unsigned char)ch;
            ret = lwrite((const char *)buffer, 1, TRUE);
        }

    } else {
        const int isutf8 = buf_isutf8(bp);      /* legacy/dialog buffer encoding */

        if (isutf8 && MCHAR_ISUTF8(ch)) {
            ret = lwrite((const char *)buffer, charset_utf8_encode(ch, buffer), TRUE);

        } else {
            *((unsigned char *)buffer) = (unsigned char)ch;
            ret = lwrite((const char *)buffer, 1, TRUE);
        }
    }
    return ret;
}


/*  Function:           linsert
 *      Low-level line interface.
 *
 *      Insert the specified character buffer into the current buffer line,
 *      optionally forcing a newline at the end of the inserted text.
 *
 *  Parameters:
 *      buffer - Character buffer.
 *      length - Length of the buffer.
 *      nl - Whether a new-line should be inserted.
 *
 *  Returns:
 *      TRUE on success otherwise FALSE.
 */
int
linsert(const char *buffer, LINENO length, int nl)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    LINE_t *lp = NULL;
    int dot;

    ED_TRACE(("linsert(line:%d, col:%d, length:%d, nl:%d)\n", cline, ccol, length, nl))

    assert(nl || length > 0);
    llinepad();
    lp = vm_lock_line(cline);
    assert(lp != x_static_line);

    dot = line_offset2(lp, cline, ccol, LOFFSET_FILL_VSPACE);
    if (length) {
        if (FALSE == lexpand(lp, dot, length)) {
            goto false_exit;
        }
        lchange(WFEDIT, 0);
        u_delete(length);
        line_set(lp, dot, buffer, length, *cur_attr, 1);
    }
    u_dot();
    *cur_col = line_current_column(dot + length);
    if (nl) {
        newlinedot(lp, dot + length);
    }
    vm_unlock(cline);
    return TRUE;

false_exit:
    vm_unlock(cline);
    return FALSE;
}


/*  Function:           lwrite
 *      Low-level line interface.
 *
 *      Write the specified character buffer into the current buffer line, replacing
 *      any previous text within the same area.
 *
 *  Parameters:
 *      buffer - Character buffer.
 *      len - Length of the buffer.
 *      characters - Width in characters.
 *
 *  Returns:
 *      TRUE on success otherwise FALSE.
 */
int
lwrite(const char *buffer, LINENO length, int characters)
{
    int ret = TRUE;

    ED_TRACE(("lwrite(characters:%d, linecol:%d/%d) = %d [", characters, *cur_line, *cur_col, length))
    ED_DATA((buffer, length, "]\n"))

    assert(length > 0);
    assert((int)length >= characters);
    if (rdonly()) {
        return FALSE;
    }

    llinepad();

    if (length) {
        const LINENO cline = *cur_line, ccol = *cur_col;
        LINENO dot, count, olength;
        int replace = FALSE;
        LINE_t *lp;

        lp = vm_lock_line(cline);
        dot = line_offset2(lp, cline, ccol, LOFFSET_FILL_SPACE);
        count = line_sizeregion(lp, ccol, dot, characters, &olength, NULL);
        if (olength < length) {
            if (dot + length <= llength(lp)) {
                replace = TRUE;
            }
        } else if (olength > length) {
            replace = TRUE;
        }

        ED_TRACE(("--> characters:%d, count:%d, olength:%d, length:%d, replace:%d\n", \
            characters, count, olength, length, replace))

        if (replace) {                          /* replace */
            ED_TRACE(("--> replace\n"))
            lreplacedot(buffer, length, olength, dot, NULL);

        } else {                                /* overwrite or append */
            const LINENO edoto = dot + length;

            ED_TRACE(("--> overwrite/append\n"))
            if (TRUE == (ret = lsize(lp, edoto))) {
                lchange(WFEDIT, 0);
                u_replace((const char *)(ltext(lp) + dot), olength, length);
                line_set(lp, dot, buffer, length, *cur_attr, 1);
                if (edoto > lp->l_used) {
                    lp->l_used = edoto;
                }
            }
        }
        *cur_col += characters;                 /* XXX - issues when line is filled */
        vm_unlock(cline);
    }
    return ret;
}


void
ldeletec(int cnt)
{
    ED_TRACE(("ldeletec(cnt:%d)\n", cnt))

    if (rdonly()) {
        return;
    }

    while (cnt > 0) {
        const LINENO cline = *cur_line, ccol = *cur_col;
        const LINENO numlines = curbp->b_numlines;
        LINENO count, length, dot;
        LINE_t *lp;

        ED_TRACE(("\tline:%d,col:%d,numlines:%d,cnt:%d\n", cline, ccol, numlines, cnt))

        if (cline > numlines) {                 /* NEWLINE */
            ED_TRACE(("\tEOF"))
            break;
        }

        lp = vm_lock_line(cline);
        dot = line_offset2(lp, cline, ccol, LOFFSET_NORMAL_MATCH);
        if (0 == (count = line_sizeregion(lp, ccol, dot, cnt, &length, NULL))) {
            count = length = 1;                 /* <EOL> */
        }
        ldeletedot(length, dot);
        vm_unlock(cline);
        cnt -= count;
    }
    ED_TRACE(("==> cnt:%d\n", cnt))
}


/*  Function:           ldelete
 *      Delete 'n' characters from the current cursor position. We do check first to
 *      see whether file is read-only and call ldeletedot() with a character offset
 *      rather than a column position.
 *
 *   Parameters:
 *       length - Character count.
 *
 *   Returns:
 *       nothing.
 */
void
ldelete(FSIZE_t length)
{
    const LINENO cline = *cur_line, ccol = *cur_col;

    ED_TRACE(("ldelete(line:%d, col:%d, length:%d)\n", cline, ccol, (int)length))
    if (rdonly()) {
        return;
    }

    if (cline <= curbp->b_numlines) {           /* NEWLINE */
        ldeletedot(length, line_offset(cline, ccol, LOFFSET_NORMAL_MATCH));
    }
}


/*  Function:           lreplacedot
 *      Low-level function to replace text in a buffer.
 *
 *      If the needed operations are optimised into the smallest set of changes delete
 *      plus optional insert, which can be a big win on a global translate as the undo
 *      state is smaller.
 *
 *  Parameters:
 *      str - String to insert.
 *      ins - Number of bytes to insert.
 *      del - Number of bytes to delete.
 *      dot - Position of replacement.
 *
 *  Returns:
 *      Number of lines inserted.
 */
int
lreplacedot(const char *buffer, int ins, int del, int dot, int *edot)
{
    const LINENO cline = *cur_line;
    register LINE_t *lp;
    int diff;

    ED_TRACE(("lreplace(ins:%d, del:%d, dot:%d)\n", ins, del, dot))
    assert(ins > 0 || del > 0);
    assert(dot >= 0);

    /*
     *  should generally be the case *except* when dealing with tabs/virtual
     *  characters, as such disabled for the time being
     *
    assert(*cur_col == (ccol = line_current_column(dot)));
     */

    /*
     *  basic case/
     *      delete only (i.e, replace buffer is zero)
     */
    if (ins <= 0) {
        ldeletedot(del, dot);
        if (edot) {
            *edot = dot;
        }
        return 0;
    }
    assert(buffer && buffer[0]);

    /*
     *  complex case/
     *      (delete-size > insert-size) or new-lines
     */
    diff = del - ins;
    if (diff < 0 || NULL != strchr(buffer, '\n')) {
        int lines = 0 ;

        if (del) ldeletedot(del, dot);
        if (ins) lines = linserts(buffer, ins);
        if (edot) {
            if (0 == lines) {
                *edot = dot + ins;
            } else {
                *edot = line_current_offset(LOFFSET_NORMAL);
            }
        }
        return lines;
    }

    /*
     *  simple case/
     *      insert-size < delete-size and no new-lines
     */
    lp = vm_lock_line(cline);
    if (ledit(lp, 0)) {
        lchange(WFEDIT, 0);
        u_replace((const char *)(ltext(lp) + dot), del, ins);
        if (diff) {
            line_move(lp, dot + ins, dot + del, llength(lp) - dot - del);
            lp->l_used -= diff;
        }
        line_set(lp, dot, buffer, ins, *cur_attr, 1);
    }
    vm_unlock(cline);
    if (edot) {
        *edot = dot + ins;
    }
    return 0;
}


/*  Function:           ldeletedot
 *      Low level primitive to delete characters from a line, possibly spanning lines.
 *
 *      All error checking has been done and we have an index into a character position
 *      into the line.  If the following assert's generally means the region interface
 *      is miss-behaving.
 *
 *  Parameters:
 *      cnt - Byte count.
 *      dot - Position.
 *
 *  Returns:
 *      nothing.
 */
void
ldeletedot(LINENO cnt, int dot)
{
    const LINENO cline = *cur_line;
    LINE_t *lp;
    int ccol;

    ED_TRACE(("ldeletedot(line:%d, col:%d, dot:%d, cnt:%d)\n", *cur_line, *cur_col, dot, cnt))
    assert(cnt > 0);
    if (NULL == (lp = vm_lock_line(cline))) {
        vm_unlock(cline);
        return;
    }

    assert(*cur_col == (ccol = line_column2(lp, cline, dot)));
    u_insert(cnt, dot);

    /*
     *  simple case/
     *      less bytes then contained on the line.
     */
    if (cnt <= (llength(lp) - dot)) {
        ED_TRACE(("--> deleting bytes (cnt:%d)\n", cnt))

        lchange(WFEDIT, 0);
        if (linfile(lp)) {
            /*
             *  optmisation for the following conditions/
             *      deleting from the front-of-line, trim trailing
             *      or for end-of-line trim leading
             */
            if (0 == dot) {
                if (0 == (lp->l_used -= cnt)) { /* leading */
                    lp->l_text = NULL;
                } else {
                    lp->l_text += cnt;
                }
                lp->l_size -= cnt;
                lp = NULL;

            } else if (dot + cnt == llength(lp)) {
                if (0 == (lp->l_used -= cnt)) { /* trailing */
                    lp->l_text = NULL;
                }
                lp->l_size -= cnt;
                lp = NULL;
            }
        }

        if (lp) {                               /* otherwise within line or incore */
            if (ledit(lp, 0)) {
                line_move(lp, dot, dot + cnt, llength(lp) - dot - cnt);
                lp->l_used -= cnt;
            }
        }

    /*
     *  complex case/
     *      delete one or more lines
     */
    } else if (ledit(lp, 0)) {
        LINE_t *nextlp = NULL;

        if (NULL == curbp->b_anchor &&
                0 == dot && cnt == (llength(lp) + 1)) {
            lchange(WFDELL, 0);
        } else {
            lchange(WFHARD, 0);
        }

        cnt -= (llength(lp) - dot);
        lp->l_used = dot;

        ED_TRACE(("--> deleting lines (cnt:%d,lp:%p)\n", cnt, lp))
        ED_TRACE_LINE2(lp)

        while (cnt > 0 && NULL != (nextlp = lforw(lp))) {
            const LINENO curlength = llength(nextlp);

            if (cnt > curlength) {              /* remove line */
                ED_TRACE(("--> deleting line (cnt:%d, curlength:%d)\n", cnt, curlength))
                cnt -= (curlength + 1);         /* length + EOL */
                lrelease(curbp, nextlp, cline + 1);
            //  anchor_adjust(FALSE);

            } else {                            /* sub-line join and remove line */
                const LINENO trailing = (curlength - cnt) + 1;

                ED_TRACE(("--> deleting sub-line (cnt:%d, curlength:%d, trailing:%d)\n", cnt, curlength, trailing))
                if (lsize(lp, dot + trailing)) {
                    line_copy(lp, dot, nextlp, cnt - 1, trailing);
                    lp->l_used += trailing;
                    ED_TRACE_LINE2(lp)
                }
                lrelease(curbp, nextlp, cline + 1);
            //  anchor_adjust(FALSE);
                cnt = 0;
                break;
            }
        }

        ED_TRACE(("--> complete (cnt:%d)\n", cnt))
        if (cnt && 0 == dot && NULL == nextlp) {
            ED_TRACE(("--> deleting trailing\n"))
            assert(1 == cnt);
            lremove(curbp, cline);              /* remove trailing empty last line */
            cnt = 0;
        }

        /*
         *  Fails in cases where ANSI escape are involved, for example subshell
         *  pty terminal emulations needs to be rewritten using ATTR mode buffers.
         *
        assert(cnt <= 1);                       // zero or trailing newline
         */
    }

    vm_unlock(cline);
}


/*  Function:           newlinedot
 *      Low-level function to insert a newline at the specified location.
 *
 *  Parameters:
 *      lp - Line pointer.
 *      dot - Insertion point.
 *
 *  Returns:
 *      TRUE if successful, otherwise FALSE.
 */
static int
newlinedot(LINE_t *lp, LINENO dot)
{
    const int incore = lincore(lp);
    const LINENO length = llength(lp),
            newsize = (dot < length ? length - dot : 0);
    const LINENO cline = *cur_line;
    LINE_t *newlp;
    int ccol;

    assert(lp != x_static_line);
    assert(0 == xf_test || *cur_col == (ccol = line_column2(lp, cline, dot)));

    if (NULL == (newlp =                        /* new-line, cloning current storage attributes */
            line_alloc((incore ? newsize : 0), BF2TST(curbp, BF2_ATTRIBUTES), incore))) {
        return FALSE;
    }

    lchange(WFINSL, 0);
    u_delete(1);                                /* newline undo */
    u_anchor();

    if (newsize) {
        if (incore) {                           /* memory image */
            assert(lincore(newlp));
            line_copy(newlp, 0, lp, dot, newsize);
            newlp->l_used = newsize;
            lp->l_used = dot;
        } else {                                /* file-buffer */
            assert(linfile(newlp));
            newlp->l_text  = lp->l_text + dot;
            newlp->l_size  = newlp->l_used = newsize;
            newlp->l_chunk = chunk_ref(lp->l_chunk, curbp);
            lp->l_size = lp->l_used = dot;
        }
    }
                                                /* NEWLINE */
    TAILQ_INSERT_AFTER(&curbp->b_lineq, lp, newlp, l_node);
    ++curbp->b_numlines;

    vm_unlock(cline + 1);

    ++(*cur_line);
    *cur_col = 1;

    lchange(WFINSL, 0);
    window_adjust();
    anchor_adjust(TRUE);
    return TRUE;
}


/*  Function:           line_new
 *      Line buffer operator, allocate a line construction.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Address of new line.
 */
LINE_t *
line_new(void)
{
    LINE_t *lp;

    if (NULL == (lp = (LINE_t *)vm_new(&x_vmlines))) {
        ewprintf(x_errmsg, sizeof(LINE_t));

    } else {
        memset(lp, 0, sizeof(LINE_t));          /* zero the LINE_t storage */
    }
    return (lp);
}


/*  Function:           line_free
 *      Line buffer operator, free a raw line construction.
 *
 *  Returns:
 *      lp - Line object address.
 *
 *  Returns:
 *      nothing.
 */
void
line_free(LINE_t *lp)
{
    const lineflags_t iflags = liflags(lp);

    assert(lp != x_static_line);

    if (0 == (iflags & LI_INFILE)) {
        assert(iflags & LI_INCORE);
        if (lp->l_text) {
            chk_free(lp->l_text);
            if ((LI_ATTRIBUTES & iflags) && lp->l_attr) {
                chk_free(lp->l_attr);
            }
        }
    }

    memset(lp, 0, sizeof(LINE_t));
    vm_free(&x_vmlines, (void *)lp);
}


/*  Function:           line_size
 *      Line buffer operator, size the data storage arena of the specified line to at atleast 'need' bytes.
 *
 *  Returns:
 *      lp - Line object address.
 *      need - Storage requirement.
 *
 *  Returns:
 *      nothing.
 */
static int
line_size(LINE_t *lp, LINENO need)
{
    const LINENO size = lalloced(lp);

    assert(lp != x_static_line);
    assert(lp->l_iflags & LI_INCORE);
    assert(need >= 0);
    assert(llength(lp) <= size);

    ED_TRACE(("\tline_size(need:%d,size:%d)\n", need, size))
    if (need > size) {
        /*
         *  (re)allocation buffer storage
         */
        const LINENO newsize = (need > NBLOCK1 ? LBLK1(need) : LBLK(need));
        LINECHAR *otext;

        assert(newsize >= need);
        if (NULL == (otext = lp->l_text)) {     /* alloc */
            assert(0 == size);
            assert(NULL == lp->l_attr);

            if (NULL == (lp->l_text = chk_alloc(newsize + 1)) ||
                    ((LI_ATTRIBUTES & lp->l_iflags) &&
                            NULL == (lp->l_attr = chk_alloc(sizeof(LINEATTR) * (newsize + 1))))) {
                ewprintf(x_errmsg, newsize);
                chk_free(lp->l_text);
                lp->l_text = NULL;
                return FALSE;
            }

        } else {                                /* expand existing */
            LINEATTR *oattr = lp->l_attr;

            assert(size > 0);
            assert(0 == (LI_ATTRIBUTES & lp->l_iflags) || lp->l_attr);
            assert(LMAGIC_TEXT == lp->l_text[size]);
            assert(NULL == lp->l_attr || LMAGIC_ATTR == lp->l_attr[size]);

            if (NULL == (lp->l_text = chk_realloc(otext, newsize + 1)) ||
                    ((LI_ATTRIBUTES & lp->l_iflags) &&
                            NULL == (lp->l_attr = chk_realloc(oattr, sizeof(LINEATTR) * (newsize + 1))))) {
                lp->l_text = otext;
                lp->l_attr = oattr;
                ewprintf(x_errmsg, newsize);
                return FALSE;
            }
        }

        /*
         *  Assign the new size and initialise new-arena including red zones markers
         */
        lp->l_size = newsize;
        lp->l_text[newsize] = LMAGIC_TEXT;

        if ((LI_ATTRIBUTES & lp->l_iflags) && lp->l_attr) {
            lp->l_attr[newsize] = LMAGIC_ATTR;
        }
    }

    if (lp->l_text) {
        assert(lalloced(lp) > 0);
        assert(llength(lp) <= lalloced(lp));
        assert(LMAGIC_TEXT == lp->l_text[lp->l_size]);
        assert(0 == (LI_ATTRIBUTES & lp->l_iflags) || NULL != lp->l_attr);
        assert(NULL == lp->l_attr || LMAGIC_ATTR == lp->l_attr[lp->l_size]);
    }

    return TRUE;
}


/*  Function:           line_move
 *      Line buffer operator, move data contained within a specified line buffer.
 *
 *  Parameters:
 *      lp - Line buffer.
 *      dst - Destination offset within the buffer.
 *      src - Source offset within the buffer.
 *      len - Length of the region to move.
 *
 *  Returns:
 *      nothing.
 */
void
line_move(LINE_t *lp, LINENO dst, LINENO src, LINENO len)
{
    assert(lp != x_static_line);
    assert(lp->l_iflags & LI_INCORE);
    assert(0 == len || lp->l_text);
    assert(src >= 0);
    assert(dst >= 0);
    assert(src + len <= llength(lp));
    assert(dst + len <= lalloced(lp));

    if (len) {
        memmove(lp->l_text + dst, (const void *)(lp->l_text + src), len);

        if ((LI_ATTRIBUTES & lp->l_iflags) && lp->l_attr) {
            memmove(lp->l_attr + dst, (const void *)(lp->l_attr + src), sizeof(LINEATTR) * len);
        }
    }
}


/*  Function:           line_copy
 *      Line buffer operator, copy data between two specified line buffers.
 *
 *  Parameters:
 *      dlp - Destination Line buffer.
 *      dst - Destination offset within the buffer.
 *      slp - Source line buffer.
 *      src - Source offset within the buffer.
 *      len - Length of the region to copy.
 *
 *  Returns:
 *      nothing.
 */
void
line_copy(LINE_t *dlp, LINENO dst, const LINE_t *slp, LINENO src, LINENO len)
{
    assert(slp != x_static_line);
    assert(dlp != x_static_line);
    assert(dlp->l_iflags & LI_INCORE);
    assert(0 == len || slp->l_text);
    assert(0 == len || dlp->l_text);
    assert((LI_ATTRIBUTES & slp->l_iflags) == (dlp->l_iflags & LI_ATTRIBUTES));
    assert(src >= 0);
    assert(dst >= 0);
    assert(src + len <= llength(slp));
    assert(dst + len <= lalloced(dlp));

    if (len) {
        memmove(dlp->l_text + dst, (const void *)(slp->l_text + src), len);

        if ((LI_ATTRIBUTES & slp->l_iflags) && slp->l_attr && dlp->l_attr) {
            memmove(dlp->l_attr + dst, (const void *)(slp->l_attr + src), sizeof(LINEATTR) * len);
        }
    }
}


/*  Function:           line_set
 *      Line buffer operator, set the line buffer to the character buffer/attribute a specified number of times.
 *
 *  Parameters:
 *      lp - Destination Line buffer.
 *      dst - Destination offset within the buffer.
 *      ch - Character value.
 *      len - Length of the character value.
 *      cnt - Count of buffers to be written.
 *
 *  Returns:
 *      nothing.
 */
void
line_set(LINE_t *lp, LINENO dst, const char *src, LINENO len, LINEATTR attr, LINENO cnt)
{
    assert(lp != x_static_line);
    assert(lp->l_iflags & LI_INCORE);
    assert(0 == len || NULL != lp->l_text);
    assert(dst >= 0);
    assert(dst + (cnt * len) <= lalloced(lp));
    assert(cnt >= 0);

    if (len) {
        if (1 == cnt) {
            memmove(lp->l_text + dst, src, len);
        } else if (1 == len) {
            memset(lp->l_text + dst, *src, cnt);
        } else {
            LINECHAR *cursor = lp->l_text + dst;
            LINENO acc;

            for (acc = 0; acc < cnt; ++acc) {
                memmove(cursor, src, len);
                cursor += len;
            }
        }

        if ((LI_ATTRIBUTES & lp->l_iflags) && lp->l_attr) {
            cnt *= len;
            if (1 == sizeof(LINEATTR)) {
                memset(lp->l_attr + dst, (unsigned char)attr, cnt);
            } else {
                register LINEATTR *cursor = lp->l_attr + dst;
                while (cnt-- > 0) {
                    *cursor++ = attr;
                }
            }
        }
    }
}
/*end*/
