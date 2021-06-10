#include <edidentifier.h>
__CIDENT_RCSID(gr_map_c,"$Id: map.c,v 1.33 2021/06/10 06:13:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: map.c,v 1.33 2021/06/10 06:13:02 cvsuser Exp $
 * High-level character mapping functionality.
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

#if (0)
#define ED_LEVEL 2
#define DO_TRACE_CHARACTER
#define DO_TRACE_LINE
#endif

#include <editor.h>

#include "accum.h"                              /* acc_...() */
#include "asciidefs.h"                          /* ASCII_... */
#include "builtin.h"
#include "cmap.h"                               /* cur_cmap */
#include "color.h"                              /* ATTR_... */
#include "debug.h"                              /* trace_...() */
#include "display.h"                            /* vt */
#include "line.h"                               /* line_...() */
#include "main.h"
#include "map.h"
#include "mchar.h"                              /* mchar_...() */
#include "ruler.h"                              /* next_..._stop() */
#include "undo.h"                               /* u_...() */
#include "window.h"                             /* win_...() */

static __CINLINE int        istab(const LINECHAR ch);
static __CINLINE int        attab(const LINECHAR *cp, const LINECHAR *end);
static int                  spacefill(LINE_t *lp, int ins, int del, int dot, int col);


/*  Function:           character_decode
 *      Decode and return the width of the next physical character within the specified
 *      line buffer for the current buffer.
 *
 *      Logical character encoding other include combination needed to be handled by the caller.
 *
 *  Parameters:
 *      col - Current cursor position.
 *      cp - Buffer cursor.
 *      end - BUffer end reference.
 *      lengthp - Physical length of the character in bytes.
 *      chp - Cooked character value, for example remapping of illegal character values.
 *      rawp - Raw character value.
 *
 *  Returns:
 *      Logical (screen) width of the character.
 *
 *      Note the return width maybe negative denoting the cursor has been moved
 *      backwards due to ANSI back-space processing.
 */
int
character_decode(int pos, const LINECHAR *cp, const LINECHAR *end, int *lenp, int32_t *chp, int32_t *rawp)
{
    const BUFFER_t *bp = curbp;
    mchar_iconv_t *iconv = bp->b_iconv;
    int32_t width, length = 1, ch, wraw;
    const LINECHAR *wcp;                        /* wide character cursor */
    const cmapchr_t *mc;
    int flags;

    if (cp >= end) {
        *lenp = 0;
        if (rawp) *rawp = -1;
        *chp = -1;
        return 1;
    }

    if ((wcp = iconv->ic_decode(iconv, cp, end, &ch, &wraw)) > cp) {
        length = wcp - cp;
        if (ch > 0xff) {                        /* wide character, FIXME - iconv() specific?? */
            width = vtcharwidth(ch, mchar_ucs_width(ch, 1));
            *lenp = length;
            if (rawp) *rawp = wraw;
            *chp = ch;
            return width;
        }
    } else {
        length = 1;
        ch = (unsigned char) *cp;
    }

    assert(ch >= 0 && ch <= 0xff);
    mc = cmapchr_lookup(cur_cmap, ch);
    if (0 == (flags = mc->mc_class)) {          /* normal characters */
        width = mc->mc_width;

    } else {
        if (CMAP_TAB == flags) {                /* <TAB> */
            width = (ruler_next_tab(bp, pos) + 1 - pos);
            ch = '\t';

        } else if (CMAP_BACKSPACE == flags) {   /* <BACKSPACE> */
            if (BFTST(bp, BF_MAN) || BFTST(bp, BF_ANSI)) {
                width = (pos > 1 ? -1 : 0);     /* MAN or ANSI; back one, if valid */
            } else {
                width = 1;
            }

        } else if (CMAP_ESCAPE == flags && BFTST(bp, BF_ANSI)) {
            /*                                  -* ANSI escape sequences *-
             *  <ESC>[<parm><;parm...>m [...]
             *
             *  See Also:
             *      display for details on supported escape sequence syntax.
             */
            if (1 == iconv->ic_unit) {
                const LINECHAR *escend = cp;
                LINECHAR t_ch = 0;

esccombined1:   while (escend < end) {          /* find escape-sequence */
                    t_ch = *++escend;
                    if ('[' == t_ch || ';' == t_ch || isdigit(t_ch)) {
                        continue;
                    }
                    break;
                }

                if ('m' != t_ch) {
                    width = mc->mc_width;
                    ch = -0x1b;                 /* bad/unknown escape */
                } else {
                    if ((escend + 1) < end && ch == escend[1]) {
                        ++escend;
                        goto esccombined1;      /* combine escape sequences */
                    }
                    width = 0;
                    length = (escend - cp)+1;   /* escape length */
                    ch = 0x1b;
                }
            } else {
                const LINECHAR *escend = cp;
                int32_t t_ch = 0;

esccombined2:   while (escend < end) {          /* find escape-sequence */
                    if ((wcp = iconv->ic_decode(iconv, escend, end, &t_ch, &wraw)) > escend) {
                        if ('[' == t_ch || ';' == t_ch || isdigit(t_ch)) {
                            escend = wcp;
                            continue;
                        }
                    }
                    break;
                }

                if ('m' != t_ch) {
                    width = mc->mc_width;
                    ch = -0x1b;                 /* bad/unknown escape */
                } else {
                    length = wcp - cp;          /* escape length */
                    if ((wcp = iconv->ic_decode(iconv, escend, end, &t_ch, &wraw)) > escend) {
                        if (ch == t_ch) {
                            escend = wcp;
                            goto esccombined2;  /* combine escape sequences */
                        }
                    }
                    width = 0;
                    ch = 0x1b;
                }
            }

        } else {                                /* byte-character */
            width = mc->mc_width;
        }
    }

    *lenp = length;
    if (rawp) *rawp = ch;
    *chp = ch;
    return width;
}


const LINE_t *
linep0(LINENO line)
{
    LINE_t *lp;

    if (NULL == curbp || NULL == (lp = linepx(curbp, line))) {
        return x_static_line;                   /* XXX/FIXME- replace with linep2 */
    }
    return lp;
}


LINE_t *
linep2(LINENO line)
{
    LINE_t *lp;

    if (NULL == curbp || NULL == (lp = linepx(curbp, line))) {
        return NULL;
    }
    return lp;
}


/*  Function:           linepx
 *      Return the stated buffers 'buffer' line object associated with the specified 'line'.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      line - Line number.
 *
 *  Returns:
 *      Line object, otherwise NULL.
 */
LINE_t *
linepx(BUFFER_t *bp, LINENO line)
{                                               /* NEWLINE */
    register LINE_t *lp = NULL;
    LINENO numlines;

    assert(line > 0 || 0 == xf_test);           /* TODO - minor fixes needed */
    if (line <= 0) line = 1;

    if (NULL == bp || line > (numlines = bp->b_numlines)) {
        return NULL;                            /* EOF */
    }

    assert(numlines >= 0);

    if (1 == line) {                            /* head */
        lp = lhead(bp);
        assert(lp);

    } else if (line == numlines) {              /* tail */
        lp = ltail(bp);
        assert(lp);

    } else {
        LINE_t *clinep;
        LINENO cline;

        if (NULL == (clinep = bp->b_clinep) ||
                (cline = bp->b_cline) < 1 || cline > numlines) {
            clinep = lhead(bp);                 /* re-seed cursor */
            cline = 1;
        }

        if (line == cline) {                    /* current 'cached' line */
            lp = clinep;
            assert(lp);

        } else {
            /*
             *  determine shortest path and iterate cursor
             */
            register LINENO diff;

            if (cline > line) {                 /* back cursor */
                diff = cline - line;
                if (diff < line) {
                    lp = clinep;
back:;              while (diff-- > 0) {
                        lp = lback(lp);
                    }
                    goto done;
                }
                lp = lhead(bp);
                assert(lp);
                diff = line - 1;

            } else {
                diff = (numlines - line);
                if (diff < line - cline) {
                    lp = ltail(bp);
                    assert(lp);
                    goto back;
                }
                lp = clinep;
                assert(lp);
                diff = line - cline;
            }

            while (diff-- > 0) {                /* forward cursor */
                lp = lforw(lp);
            }
        }
    }

done:;
    assert(lp);
    bp->b_clinep = lp;
    bp->b_cline = line;
    return lp;
}


/*  Function:           linep_flush
 *      Release any current buffer line details, homing the cursor.
 *
 *  Parameters:
 *      bp  -               Buffer object.
 *
 *  Returns:
 *      nothing
 */
void
linep_flush(BUFFER_t *bp)
{
    assert(bp);
    bp->b_clinep = NULL;                        /* lhead(bp); */
    bp->b_cline = 1;
}


/*  Function:           line_sizeregion
 *      Determine the size in characters for the region upto the specified number of
 *      characters or EOL, which ever occurs first.
 *
 *  Parameters:
 *      lp -                Line buffer.
 *      col -               Column.
 *      dot -               Physical line offset.
 *      characters -        Width in characters.
 *      lengthp -           Optional buffer populated with the region size in bytes.
 *      colp -              Optional buffer populated with the resulting column,
 *                          representing the width.
 *
 *  Returns:
 *      Character count consumed.
 */
int
line_sizeregion(const LINE_t *lp, int col, int dot, int characters, LINENO *lengthp, LINENO *colp)
{
    int count = 0, length = 0;

    if (lp && dot < (int)llength(lp)) {
        const LINECHAR *start, *cp, *end;

        start = ltext(lp);
        cp  = start + dot;
        end = start + llength(lp);

        while (count < characters && cp < end) {
            int32_t t_ch;
            int t_length, t_width =
                    character_decode(col, cp, end, &t_length, &t_ch, NULL);

#if defined(DO_TRACE_CHARACTER)
            trace_character(t_ch, t_width, cp, t_length);
#endif
            cp += t_length;
            length += t_length;
            col += t_width;

            if (t_width || (0 == t_length && cp < end)) {
                ++count;                        /* character or EOL */
            }
        }
    }

    if (lengthp) *lengthp = length;
    if (colp) *colp = col;
    return count;
}


/*  Function:           line_current_status
 *      Determine and return the status of the character under the cursor.
 *
 *  Parameters:
 *      values -            Optional character value.
 *
 *  Returns:
 *      *true* if current character is virtual otherwise *false*.
 */
int
line_current_status(int *values, int count)
{
    const int32_t *vchar = NULL;
    unsigned vcombined = 0;
    int vstatus = 0;

    if (curbp) {
        /*
         *  If the active window is current when update when needed,
         *  otherwise just return the current value.
         */
        if (curwp && curwp->w_bufp) {

            if (curbp == curwp->w_bufp) {
                set_hooked();                   /* setup cur_line/cur_col */
                if (-1 == curbp->b_vstatus ||
                        curbp->b_vline != *cur_line ||
                        curbp->b_vcol  != *cur_col) {
                    line_current_offset(LOFFSET_NORMAL);
                }
            }

            vstatus   = curwp->w_bufp->b_vstatus;
            vcombined = curwp->w_bufp->b_vcombined;
            vchar     = curwp->w_bufp->b_vchar;

            ED_ITRACE(("\tcursor1: (line:%d, col:%d, ch:%d/0x%x, virt:%d, width:%d, combined: %d)\n", \
                *cur_line, *cur_col, vchar[0], vchar[0], vstatus, curwp->w_bufp->b_vwidth, vcombined))

        } else {
            vstatus   = curbp->b_vstatus;
            vcombined = curbp->b_vcombined;
            vchar     = curbp->b_vchar;

            ED_ITRACE(("\tcursor2:(line:%d, col:%d, ch:%d/0x%x, virt:%d, width:%d combined: %d)\n", \
                *cur_line, *cur_col, vchar[0], vchar[0], vstatus, curbp->b_vwidth, vcombined))
        }

        /*
         *  optional return of character value(s)
         */
        if (values && vchar) {
            unsigned n;

            for (n = 0; count-- > 0 && n <= vcombined; ++n) {
                if (n < (sizeof(curbp->b_vchar)/sizeof(curbp->b_vchar[0]))) {
                    values[n] = vchar[n];
                }
            }
        }
    }
    return vstatus;
}


int
line_current_offset(int fill)
{
    return line_offset(*cur_line, *cur_col, fill);
}


int
line_offset(const int line, const int col, int fill)
{
    int dot = 0;
    LINE_t *lp;

    if (curbp) {
        if (NULL != (lp = vm_lock_linex(curbp, line))) {
            dot = line_offset_fill(lp, line, col, fill);
            vm_unlock(line);
        } else {
            curbp->b_vline      = line;
            curbp->b_vcol       = col;
            curbp->b_vstatus    = BUFFERVSTATUS_EOF;
            curbp->b_vchar[0]   = 0;
            curbp->b_vcombined  = 0;
            curbp->b_vwidth     = 0;
        }
    }
    return dot;
}


/*  Function:           line_offset_fill
 *      Convert the specified 'column' position into an physical byte offset from the start
 *      of the current line.
 *
 *      If the fill is set and cursor is not on a valid character, the line shall be
 *      tabs/spaces padded upon the specified column based upon the flll mode.
 *
 *  Parameters:
 *      lp -                Line object.
 *      line -              Logical line.
 *      col -               Logical column.
 *      fill -              Fill mode as follows.
 *
 *  Fill Options:
 *      LOFFSET_FIRSTBYTE -
 *              No fill, returning complete character include leading colorization.
 *
 *      LOFFSET_LASTBYTE -
 *              No fill, return offset last byte within the character including any
 *              combined characters.
 *
 *      LOFFSET_NORMAL -
 *              No fill, returning the closest 'normal' character to the specified column.
 *
 *      LOFFSET_NORMAL_MATCH -
 *              No fill, returning the closest 'normal' character to the specified column
 *              plus modify the cursor position to reflect.
 *
 *      LOFFSET_FILL_VSPACE -
 *              Fill if stated column is beyond the end of line or within a virtual space.
 *
 *      LOFFSET_FILL_SPACE  -
 *              As fill LOFFSET_FILL_VSPACE, yet also replacing physical tabs.
 *
 *  Returns:
 *      Line Offset.
 */
int
line_offset_fill(LINE_t *lp, const int line, const int col, int fill)
{
    const int isutf8 = buf_isutf8(curbp);       /* buffer encoding */
    const LINECHAR *start = ltext(lp), 
            *cp = start, *end = cp + llength(lp);
    int length = 0, width = 0;
    int pos = 1, offset = 0;
    int vstatus = 0;
    int32_t ch = 0;

    assert(col >= 1);
    assert(fill >= LOFFSET_LASTBYTE && fill <= LOFFSET_FILL_SPACE);

    ED_ITRACE(("line_offset(line:%d, col:%d, fill:%d, used:%d)\n", line, col, fill, llength(lp)))

    if (col <= 0) {
        offset = 0;
        goto done;
    }

    /*
     *  If ansi escape mode position cursor *after* the escape.
     *
     *  If UTF-8 consume any combined characters under current character.
     */
    while (pos < col && cp < end) {
        width = character_decode(pos, cp, end, &length, &ch, NULL);
#if defined(DO_TRACE_CHARACTER)
        trace_character(ch, width, cp, length);
#endif
        pos += width;
        cp += length;
    }

    ED_ITRACE(("\tco: iterate1 (pos:%d, line:%d, col:%d, ch:%d/0x%x, width:%d, len:%d, cp:%p, start:%p, end:%p)\n", \
        pos, line, col, ch, ch, width, length, cp, start, end))

    if (pos == col && cp < end) {
        const LINECHAR nextch = *cp;

        /*
         *  ANSI mode, unless fill is FIRSTBYTE/LASTBYTE
         */
        if (0x1b == nextch && BFTST(curbp, BF_ANSI)) {
            if (fill >= LOFFSET_NORMAL) {       /* not LASTBYTE/FIRSTBYTE */
                int t_length;
                int32_t t_ch;

                if (0 == character_decode(pos, cp, end, &t_length, &t_ch, NULL) && t_length > 0) {
                    length += t_length;
                    cp += t_length;
                    ch = 0x1b;
                }
                ED_ITRACE(("\tANSI mode = %d", t_length))
            }

        /*
         *  consume all combined characters
         */
        } else if (isutf8 && MCHAR_ISUTF8(nextch)) {
            unsigned combined = 0;

            if (cp > start) {
                do {
                    int32_t t_ch;
                    int t_length, t_width =
                            character_decode(pos, cp, end, &t_length, &t_ch, NULL);

                    if (t_width || t_ch <= 0xff || t_length <= 0) {
                        break;                  /* not combining character */
                    }

#if defined(DO_TRACE_CHARACTER)
                    trace_character(t_ch, t_width, cp, t_length);
#endif
                    length += t_length;
                    cp += t_length;
                    ++combined;
                } while (cp < end);
            }
            ED_ITRACE(("\tcombined mode %u\n", combined))
        }
    }

    /*  Cursor and buffer construction
     *
     *                 pos  logical width
     *                 v    v
     *                 |----|
     *                 V[VVV]
     *
     *          [.....]C[CCC][....]<EOL><EOL2>....
     *          ^start       ^cp   ^end
     *                 |----|
     *                      ^length in bytes
     *          <EOF>
     *
     *  Where:
     *      C   = Character value (one or more bytes).
     *      V   = Visual character, where is a virtual space is any position
     *            after the position of the TAB sequence.
     */
    vstatus = ((pos > col && pos < col + width) ? BUFFERVSTATUS_VIRTUAL :
                    (cp < end ? (length > 1 ? BUFFERVSTATUS_MULTIBYTE : BUFFERVSTATUS_NORMAL) :
                        (col > pos ? BUFFERVSTATUS_PEOL :
                            (curwp && curwp->w_eol_col > 0 ? BUFFERVSTATUS_XEOL : BUFFERVSTATUS_EOL))));

    curbp->b_vstatus    = vstatus;
    curbp->b_vline      = line;
    curbp->b_vcol       = col;
    curbp->b_vchar[0]   = (cp >= end ? 0 : ch);
    curbp->b_vcombined  = 0;
    curbp->b_vwidth     = width;
    curbp->b_vlength    = length;

    assert(cp >= start);
    assert(cp <= end);
    offset = cp - start;

    ED_ITRACE(("\tco: iterate2 (pos:%d, line:%d, col:%d, ch:%d/0x%x, width:%d, len:%d, virt:%d, cp:%p, start:%p, end:%p, off:%d)\n", \
        pos, line, col, ch, ch, width, length, vstatus, cp, start, end, offset))
    ED_TRACE_LINE2(lp)

    /*
     *  Complete or non-fill mode
     */
    if (fill < LOFFSET_FILL_VSPACE ||
            (pos == col &&
                (LOFFSET_FILL_VSPACE == fill ||
                (LOFFSET_FILL_SPACE  == fill && !attab(cp, end))))) {
        unsigned combined = 0;                  /* combined character count */

        /*
         *  past cursor, move back to previous character
         */
        if (pos > col) {
            offset -= length;
            pos -= width;
            cp -= length;
            ED_ITRACE(("\tco: back (pos:%d, col:%d, width:%d, length:%d, cp:%p, start:%p, end:%p)\n", \
                pos, col, width, length, cp, start, end))
            assert(pos <= col);
        }

        /*
         *  encode virtual character
         */
        length = 0;                             /* length of current character, in bytes */
        if (cp < end) {
            while (cp < end) {
                int32_t t_ch, t_raw;
                int t_length, t_width =
                        character_decode(pos, cp, end, &t_length, &t_ch, &t_raw);

                if (t_length <= 0) {
                    break;
                }

                if (0 == combined) {
                    width = t_width;
                } else if (t_width /*|| t_ch <= 0xff*/) {
                    break;                      /* non-combining character */
                }

#if defined(DO_TRACE_CHARACTER)
                trace_character(t_ch, t_width, cp, t_length);
#endif
                if (combined < (sizeof(curbp->b_vchar)/sizeof(curbp->b_vchar[0]))) {
                    curbp->b_vchar[combined] = t_raw;

                    if (t_ch != t_raw) {
                        vstatus |= BUFFERVSTATUS_ILLEGAL;
                    }

                    if (0 == combined) {
                        curbp->b_vwidth = t_width;
                    }

                    ED_ITRACE(("\tco: iterate3[%u] (pos:%d, line:%d, col:%d, ch:%d/0x%x, raw:%d/0x%x, width:%d, len:%d, virt:%d, cp:%p, end:%p)\n", \
                        combined, pos, line, col, t_ch, t_ch, t_raw, t_raw, t_width, t_length, vstatus, cp, end))
                }

                curbp->b_vcombined = combined;
                length += t_length;
                cp += t_length;
                ++combined;                     /* 1 or 2 */
            }

            if (length > 1) {
                vstatus |= BUFFERVSTATUS_MULTIBYTE;
            }

            curbp->b_vstatus = vstatus;
            curbp->b_vwidth  = width;
            curbp->b_vlength = length;
        }

        if (LOFFSET_NORMAL_MATCH == fill) {
            *cur_col = (pos ? pos : 1);         /* match cursor */

        } else if (LOFFSET_LASTBYTE == fill) {
            if (length > 0) {
                offset += (length - 1);         /* end of [combined] character */
            }
        }

        ED_ITRACE(("\tco: no fill (pos:%d, length:%d, combined:%u)\n", pos, length, combined))
        goto done;
    }

    /*
     *  Fill
     */
    assert(LOFFSET_FILL_VSPACE == fill || LOFFSET_FILL_SPACE == fill);
    if (pos < col) {
        /*
         *  End-of-line/
         *      pad with space and/or tabs
         */
        const int endfill = (col - pos);

        assert(cp == end);
        ED_ITRACE(("\tco: fill (end:%d)\n", endfill))
        offset = spacefill(lp, endfill, 0, offset, pos);

    } else if (pos == col) {
        /*
         *  Physical TAB/
         *      replace with space
         */
        const int nwidth = (ruler_next_tab(curbp, pos) + 1 - pos);

        assert(LOFFSET_FILL_SPACE == fill);
        assert(cp >= end || istab(*cp));

        ED_ITRACE(("\tco: fill (replace:%d, width:%d, length:%d)\n", offset, nwidth, length))
        spacefill(lp, nwidth, 1, offset, pos);

    } else {
        /*
         *  Virtual space/
         *      Replace tab with space upto the required column
         *      and optionally forward-fill if not EOL
         */
        const int bckfill = col - (pos - width);
        int fwdfill = 0;

        assert(width > (pos - col));
        if (cp < end) {
            fwdfill = pos - col;
        }
        offset -= length;
        ED_ITRACE(("\tco: fill (back:%d, fwd:%d, length:%d, width:%d)\n", bckfill, fwdfill, length, width))
        spacefill(lp, bckfill + fwdfill, 1, offset, pos - width);
        offset += bckfill;
    }

    curbp->b_vstatus   = -1;                    /* unknown, generally fill preceeds a modification */
    curbp->b_vcombined = 0;
    curbp->b_vwidth    = 0;
    curbp->b_vchar[0]  = 0;

done:;
    ED_ITRACE(("\t==> (pos:%d, col:%d: length:%d) : offset:%d/0x%x\n", \
        pos, col, length, offset, offset))
    return offset;
}


int
line_offset_const(const LINE_t *lp, const int line, const int col, int fill)
{
    assert(fill >= LOFFSET_LASTBYTE && fill <= LOFFSET_NORMAL_MATCH);
    return line_offset_fill((LINE_t *)lp, line, col, fill);
}


static __CINLINE int
istab(const LINECHAR ch)
{
    return (CMAP_TAB == cmapchr_class(cur_cmap, ch));
}


static __CINLINE int
attab(const LINECHAR *cp, const LINECHAR *end)
{
    if (cp < end) {
        if (istab(*cp)) {
            return 1;
        }
    }
    return 0;
}


/*  Function:           spacefill
 *      Fill the space within a specified line buffer, optionally
 *      replacing/deleting the current character.
 *
 *      line_offset2() support function.
 *
 *  Parameters:
 *      lp -                Line buffer.
 *      ins -               Number spaces to fill.
 *      del -               Character to be replaced (0 or 1).
 *      dot -               Physical insertion point starting point within buffer.
 *      col -               Logical column.
 *
 *  Returns:
 *      Final line offset.
 */
static int
spacefill(LINE_t *lp, int ins, int del, int dot, int col)
{
    mchar_iconv_t *iconv = curbp->b_iconv;
    const int characterunit = (int)iconv->ic_unit;
    const int widthof_space = cmapchr_width(cur_cmap, ' ');
    const LINENO ccol = *cur_col;
    int ins_tabs = 0, ins_spaces = ins;
    int tabs = FALSE, size;

    assert(1 == characterunit || 2 == characterunit || 4 == characterunit);
    assert(ins > 0);
    assert(del >= 0 && del <= 1);
    assert(dot >= 0);
    assert(col >= 0);

    __CUNUSED(tabs)

    /*
     *  determine tab/space adjustment
     *      tab fill if the buffer is hard-tabs enabled plus tabs are special
     *      otherwise space-fill.
     */
    if (col && 0 == del && BFTST(curbp, BF_TABS) && istab('\t')) {
        int tcol = col - 1;

        tabs = TRUE;
        while (ins_spaces > 2) {
            const int tab_width = (ruler_next_tab(curbp, tcol + 1) - tcol);

            if (tab_width > ins_spaces) {
                break;
            }
            ++ins_tabs;
            tcol += tab_width;
            ins_spaces -= tab_width;
        }
        ins_spaces /= widthof_space;
        size = ins_spaces + ins_tabs;

    } else {
        ins_spaces /= widthof_space;
        size = ins_spaces;
    }

    /*
     *  apply modification
     */
    assert(size >= 0);
    assert(size == ins_tabs + ins_spaces);
    ED_TRACE_LINE2(lp)

    if (size > 0) {
        const LINEATTR attr =
                (LINEATTR)(curbp ? curbp->b_attrnormal : ATTR_NORMAL);

        assert(size == (ins_tabs + ins_spaces));
        if (! ledit(lp, size * characterunit)) {
            return 0;
        }

        u_dot();
        *cur_col = col;
        if (del) {                              /* replacing character */
            const LINECHAR *text = ltext(lp) + dot;
            const int len = (llength(lp) - dot) - del;

            assert(1 == del);
            assert(characterunit > 1 || '\t' == *text);
            u_replace((const char *)text, (del * characterunit), (size * characterunit));
            line_move(lp, dot + (size * characterunit), dot + (del * characterunit), len);

        } else {                                /* space filling */
            u_delete(size * characterunit);
        }

        if (1 == characterunit) {
            line_set(lp, dot, "\t", 1, attr, ins_tabs);
            line_set(lp, dot + ins_tabs, " ", 1, attr, ins_spaces);
            lp->l_used += (size - del);

        } else {
            char mchbuf[MCHAR_MAX_LENGTH + 1];

            if (ins_tabs)
                line_set(lp, dot, mchbuf, mchar_encode(iconv, '\t', mchbuf), attr, ins_tabs);
            if (ins_spaces)
                line_set(lp, dot + (ins_tabs * characterunit), mchbuf, mchar_encode(iconv, ' ', mchbuf), attr, ins_spaces);
            lp->l_used += (size - del) * characterunit;
        }

        assert(lp->l_used <= lp->l_size);
        u_dot();
        *cur_col = ccol;
    }

    ED_ITRACE(("\tspace_fill(ins:%d, del;%d, dot:%d, col:%d) tab:%c tabs:%d spaces:%d size:%d= %d\n", \
        ins, del, dot, col, (tabs ? 'Y' : 'N'), ins_tabs, ins_spaces, size, dot + size))
    ED_TRACE_LINE2(lp)

    return dot + (size * characterunit);
}


int
line_current_column(int offset)
{
    return line_column(*cur_line, offset);
}


int
line_column_eol(int line)
{
    int column = 0;
    const LINE_t *lp;

    if (NULL != (lp = vm_lock_line2(line))) {
        column = line_column2(lp, line, llength(lp));
        vm_unlock(line);
    }
    return column;
}


int
line_column(const int line, int offset)
{
    int column = 0;
    const LINE_t *lp;

    if (NULL != (lp = vm_lock_line2(line))) {
        column = line_column2(lp, line, offset);
        vm_unlock(line);
    }
    return column;
}


/*  Function:           line_column2
 *      Convert the specified line buffer byte 'offset' into a logical column position
 *      offset within the current line.
 *
 *  Parameters:
 *      lp -                Line object.
 *      offset -            Physical line offset, in bytes.
 *      line -              Logical line.
 *
 *  Returns:
 *      Column value.
 */
int
line_column2(const LINE_t *lp, const int line, int offset)
{
    const LINECHAR *cp, *end;                   /* buffer cursor and end reference */
    int length;
    int col = 1;

    __CUNUSED(line)

    assert(offset >= 0);
    length = llength(lp);
    cp = ltext(lp);
    end = cp + length;

    ED_ITRACE(("line_column(line:%d, offset:%d)\n", line, offset))
    ED_TRACE_LINE2(lp)

    while (offset > 0 && cp < end) {            /* iterate over each 'character' */
        int32_t t_ch;
        int t_length, t_width =
                character_decode(col, cp, end, &t_length, &t_ch, NULL);

#if defined(DO_TRACE_CHARACTER)
        trace_character(t_ch, t_width, cp, t_length);
#endif
        cp += t_length;
        offset -= t_length;
        col += t_width;
    }

    ED_ITRACE(("==> (offset:%d) : %d\n", offset, col))
    return col;
}


/*  Function:           line_tab_backfill
 *      Replace spaces with tabs within the current line up-to the current column.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
line_tab_backfill(void)
{
    const int cline = *cur_line, ccol = *cur_col;
    const LINECHAR *cp, *start, *end;
    char tabbuf[MCHAR_MAX_LENGTH + 1];
    int col, ncol, tablen, spaces, replaced = 0;
    LINE_t *lp;

    ED_TRACE2(("tab_backfill(line:%d,col:%d)\n", cline, ccol))

    lp = vm_lock_line2(cline);
    if (NULL == lp || !ledit(lp, 0)) {
        vm_unlock(cline);
        return;
    }

    col  = 1;
    ncol = ruler_next_tab(curbp, 1) + 1;
    cp   = start = ltext(lp);
    end  = start + llength(lp);
    spaces = 0;

    tablen = mchar_encode((mchar_iconv_t *)curbp->b_iconv, '\t', tabbuf);

    while (col < ccol) {
        int32_t t_ch;
        int t_length, t_width =
                character_decode(col, cp, end, &t_length, &t_ch, NULL);

        if (' ' == t_ch && 1 == t_width) {      /* spaces */
            ++spaces;
        } else {
            spaces = 0;
        }

        ED_TRACE2(("\tch:%d/0x%x,width:%d,spaces:%d,col:%d,ncol:%d\n", \
            t_ch, t_ch, t_width, spaces, col, ncol))
        ED_TRACE_LINE2(lp)

        cp += t_length;
        col += t_width;

        if (col >= ncol) {
            /*
             *  Next TAB stop hit ...
             */
            if (spaces > 1) {                   /* something to backfill */
                const int dot = (cp - start) - spaces;
                int edot = 0;

                ED_TRACE2(("\t==> replacing(col:%d, ncol:%d, spaces:%d, dot:%d\n", \
                    col, ncol, spaces, dot))

                *cur_col = col - spaces;
                lreplacedot(tabbuf, tablen, spaces, dot, &edot);
                replaced += spaces;

                start = ltext(lp);              /* reseed cursor */
                end = cp + llength(lp);
                cp = start + edot;
            }
            spaces = 0;
            ncol = ruler_next_tab(curbp, col) + 1;
        }
    }

    if (replaced) {                             /* restore cursor */
        u_dot();
        *cur_col = ccol;
    }

    vm_unlock(cline);
}

/*end*/
