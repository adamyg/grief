#include <edidentifier.h>
__CIDENT_RCSID(gr_basic_c,"$Id: basic.c,v 1.29 2015/02/11 23:25:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: basic.c,v 1.29 2015/02/11 23:25:12 cvsuser Exp $
 * Basic cursor movement.
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
#include "basic.h"
#include "border.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "main.h"
#include "map.h"
#include "mchar.h"
#include "ruler.h"                              /* next_..._stop() */
#include "symbol.h"                             /* argv_...() */
#include "tty.h"
#include "undo.h"
#include "window.h"

#define MOVE_OBEY       0x0001                  /* obey cursor rules/column history */
#define MOVE_CLIP       0x0002                  /* clip to buffer */

static void             mov_char(const int n, const int direction);
static void             mov_line(const int n, const int direction);
static void             move2(const LINENO nline, const LINENO ncol, int flags);
static int              move(LINENO nline, LINENO ncol, int flags);


/*  Function:           inq_position
 *      inq_position primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_position - Retrieve current buffer position.

        int 
        inq_position([int &line], [int &col])

    Macro Description:
        The 'inq_position()' primitive retrieves the current cursor
        position in the current buffer.

    Macro Parameters:
        line - Optional integer variable when supplied shall be
            populated with the current buffer line.

        col - Optional integer variable when supplied shall be
            populated with the current buffer column.

    Macro Returns:
        The 'inq_position()' primitive returns 0 if the current
        position is not past the end of the buffer. Otherwise, the
        number of lines between the end of the buffer and the current
        position.

    Macro Portability:
        n/a

    Macro See Also:
        move_abs, move_rel
 */
void
inq_position(void)              /* int ([int &line], [int &column]) */
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    const LINENO numlines = curbp->b_numlines;

    argv_assign_int(1, (accint_t) cline);
    argv_assign_int(2, (accint_t) ccol);
    if (cline <= numlines) {
        acc_assign_int((accint_t) 0);
    } else {                                    /* NEWLINE */
        acc_assign_int((accint_t) (cline - numlines));
    }
}


/*  Function:           do_beginning_of_line
 *      beginning_of_line primitive -- goto beginning of line.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: beginning_of_line - Goto beginning of line.

        int 
        beginning_of_line()

    Macro Description:
        The 'beginning_of_line()' primitive moves the buffer cursor to
        the first character of the current line.

    Macro Parameters:
        none

    Macro Returns:
        The 'beginning_of_line()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        end_of_line
 */
void
do_beginning_of_line(void)      /* int () */
{
    move2(-1, 1, 0);
}


/*  Function:           do_end_of_line
 *      end_of_line primitive -- goto end of line.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: end_of_line - Goto end of line.

        int 
        end_of_line()

    Macro Description:
        The 'end_of_line()' primitive moves the buffer cursor to the
        last character of the current line.

    Macro Parameters:
        none

    Macro Returns:
        The 'end_of_line()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        beginning_of_line
 */
void
do_end_of_line(void)            /* int () */
{
    const int eol = line_column_eol(*cur_line);

    move2(-1, eol, 0);
}


/*  Function:           do_next_char
 *      next_char primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: next_char - Move to the next character.

        int 
        next_char([int characters = 1])

    Macro Description:
        The 'next_char()' primitive moves the current buffer position
        forward to the next character, wrapping around line ends when
        encountered.

        The primitive is similar to <right> except it moves physical
        characters as opposed to logic characters, as the result tabs
        and newlines are both treated as one character.

        Within navigating binary files newlines are not implied, as
        such are not counted within the character count.

    Macro Parameters:
        characters - Optional number of characters to move forward
            in the buffer, which if omitted is 1.

    Macro Returns:
        The 'next_char()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        prev_char, right
 */
void
do_next_char(void)              /* int ([int characters = 1]) */
{
    const int n = get_xinteger(1, 1);

    if (n > 0) {
        move_next_char(n, TRUE);
    } else if (n < 0) {
        move_prev_char(-n);
    } else {
        acc_assign_int(0);
    }
}


/*  Function:           do_prev_char
 *      prev_char primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: prev_char - Move to the previous character

        int 
        prev_char([int characters = 1])

    Macro Description:
        The 'prev_char()' primitive moves the current buffer position
        backward to the previous character, wrapping around line ends
        when encountered.

        The primitive is similar to <left> except it moves physical
        characters as opposed to logic characters, as the result tabs
        and newlines are both treated as one character.

        Within navigating binary files newlines are not implied, as
        such are not counted within the character count.

    Macro Parameters:
        characters - Optional number of characters to move backward
            in the buffer, which if omitted is 1.

    Macro Returns:
        The 'prev_char()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        next_char, left
 */
void
do_prev_char(void)              /* int ([int characters = 1]) */
{
    const int n = get_xinteger(1, 1);

    if (n > 0) {
        move_prev_char(n);
    } else if (n < 0) {
        move_next_char(-n, TRUE);
    } else {
        acc_assign_int(0);
    }
}


/*  Function:           do_top_of_buffer
 *      top_of_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: top_of_buffer - Move cursor to start of current buffer.

        int 
        top_of_buffer()

    Macro Description:
        The 'top_of_buffer()' primitive moves the buffer cursor to
        the start of the first line of the current buffer; this is
        eqivalent to using <move_abs> as follows.

>               move_abs(1,1);

    Macro Parameters:
        none

    Macro Returns:
        The 'top_of_buffer()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        end_of_buffer, inq_position, move_abs, move_rel
 */
void
do_top_of_buffer(void)          /* int () */
{
    move2(1, 1, 0);
}


/*  Function:           do_end_of_buffer
 *      end_of_buffer primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: end_of_buffer - Move cursor to end of current buffer.

        int 
        end_of_buffer()

    Macro Description:
        The 'end_of_buffer()' primitive moves the buffer cursor to
        the end of the last line of the current buffer.

    Macro Parameters:
        none

    Macro Returns:
        The 'end_of_buffer()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        top_of_buffer, move_abs
 */
void
do_end_of_buffer(void)          /* int () */
{
    const LINENO numlines = curbp->b_numlines;

    move2((numlines > 1 ? numlines : 1), -1, 0);
    do_end_of_line();
}


/*  Function:           do_page_down
 *      page_down primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: page_down - Move position down a page.

        int 
        page_down([int pages = 1])

    Macro Description:
        The 'page_down()' primitive moves the buffer position down or
        forward one or more pages down, with a page being the current
        window size in lines.

    Macro Parameters:
        pages - If supplied, states the number of pages to move the
            cursor forward, otherwise the cursor is moved 1 page.

    Macro Returns:
        The 'page_down()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        'pages' is a Grief extension.

    Macro See Also:
        page_up, down
 */
void
do_page_down(void)              /* int ([int pages = 1]) */
{
    accint_t pages = get_xinteger(1, 1);        /* extension */
    const int lines = win_height(curwp);

    if (pages < 0) {                            /* half pages */
        mov_forwline(lines > 2 ? (lines / 2) : 1);
    } else if (pages) {
        mov_forwline(pages * lines);
    } else {
        mov_forwline(lines);
    }
}


/*  Function:           do_page_up
 *      page_up primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: page_up - Move position up a page.

        int 
        page_up([int pages = 1])

    Macro Description:
        The 'page_up()' primitive moves the buffer position up or
        backwards one or more pages up, with a page being the current
        window size in lines.

    Macro Parameters:
        pages - If supplied, states the number of pages to move the
            cursor backwards, otherwise the cursor is moved 1 page.

    Macro Returns:
        The 'page_up()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        'pages' is a Grief extension.

    Macro See Also:
        page_down, up
 */
void
do_page_up(void)                /* int ([int pages = 1]) */
{
    accint_t pages = get_xinteger(1, 1);        /* extension */
    const int lines = win_height(curwp);

    if (pages < 0) {                            /* half pages */
        mov_backline(lines > 2 ? (lines / 2) : 1);
    } else if (pages) {
        mov_backline(pages * lines);
    } else {
        mov_backline(lines);
    }
}


/*  Function:           do_down
 *      down primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: down - Move position down one line.

        int 
        down([int lines = 1])

    Macro Description:
        The 'down()' primitive moves the cursor down one line to the
        same column on the next line.

    Macro Parameters:
        lines - Optional number of lines to move the cursor;
            may be negative in which case the cursor moves backwards
            behaving like <up>.

    Macro Returns:
        The 'down()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        up, left, right
 */
void
do_down(void)                   /* ([int lines = 1) */
{
    const accint_t lines = get_xinteger(1, 0);

    if (lines < 0) mov_backline((int)(lines * -1));
    else mov_forwline((int)(lines >= 1 ? lines : 1));
}


/*  Function:           do_left
 *      left primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: left - Move position left one charcter.

        int 
        left([int columns = 1], [int wrap = TRUE])

    Macro Description:
        The 'left()' primitive moves the cursor left one column
        retaining the current line; unless at the beginning of the
        line, in which case the cursor moves to the end of the
        previous line.

    Macro Parameters:
        columns - Optional number of columns to move the cursor;
            negative in which case the cursor movement is reversed
            behaving like <right>.

        wrap - Optional boolean value controlling whether the cursor
            wraps when positioned at the beginning of line. If
            *FALSE* line wrapping shall be disabled.

    Macro Returns:
        The 'left()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        Unlike BRIEF, if the cursor is moved past the beginning of
        the current line, then the cursor wraps around to the end of
        the previous line.

        'wrap' is a Grief extension.

    Macro See Also:
        right, up, down
 */
void
do_left(void)                   /* int ([int columns = 1], [int wrap = TRUE) */
{
    const accint_t columns = get_xinteger(1, 0);
    const accint_t wrap = get_xinteger(2, TRUE);

    if (columns < 0) mov_forwchar((int)(columns * -1));
    else mov_backchar((int) (columns >= 1 ? columns : 1), wrap);
}


/*  Function:           do_right
 *      right primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: right - Move position right one character.

        int 
        right([int columns = 1], [int wrap = TRUE])

    Macro Description:
        The 'right()' primitive moves the cursor right one column
        retaining the current line.

    Macro Parameters:
        columns - Optional number of columns to move the cursor;
            negative in which case the cursor movement is reversed
            behaving like <left>.

        wrap - Optional boolean value controlling whether the cursor
            wraps when positioned at the beginning of line. If
            *FALSE* line wrapping shall be disabled, see <left>.

    Macro Returns:
        The 'right()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        Unlike BRIEF, if the cursor is moved past the beginning of
        the current line, then the cursor wraps around to the end of
        the previous line.

        'wrap' is a Grief extension.

    Macro See Also:
        left, up, down
 */
void
do_right(void)                  /* int ([int columns = 1], [int wrap = TRUE]) */
{
    const accint_t columns = get_xinteger(1, 0);
    const accint_t wrap = get_xinteger(2, TRUE);

    if (columns < 0) mov_backchar((int) (columns * -1), wrap);
    else mov_forwchar((int)(columns >= 1 ? columns : 1));
}


/*  Function:           do_up
 *      up primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: up - Move position up one line.

        int 
        up([int lines = 1])

    Macro Description:
        The 'up()' primitive moves the cursor up one line to the same
        column on the previous line.

    Macro Parameters:
        lines - Optional number of lines to move the cursor;
            may be negative in which case the cursor moves forward
            behaving like <down>.

    Macro Returns:
        The 'up()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        down, left, right
 */
void
do_up(void)                     /* int ([int lines = 1]) */
{
    const accint_t lines = get_xinteger(1, 0);

    if (lines < 0) mov_forwline((int)(lines * -1));
    else mov_backline((int) (lines >= 1 ? lines : 1));
}


/*  Function:           do_move_rel
 *      move_rel primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: move_rel -  Move to a relative location in the buffer.

        int move_rel([int lines = 1], [int cols = 1])

    Macro Description:
        The 'move_rel()' primitive moves the buffer cursor to a new
        position relative to the current position.

        If a parameter is 0 or omitted, the corresponding line or
        column coordinate is unchanged. Positive values move the
        cursor towards the end of the line and/or column, likewise
        negative values move towards the beginning of the line and/or
        column.

    Macro Parameters:
        lines - Optional integer specifying the line number,
            if positive the cursor moves forwards the end of the file, 
            likewise a negative moves to backwards towards the top.

        cols - Optional integer specifying the column number,
            if positive the cursor moves forwards the front of the
            line, likewise a negative moves to backwards towards the
            beginning.

    Macro Returns:
        The 'mov_rel()' primitive returns non-zero on success denoting
        that the cursor moved, otherwise zero if the cursor remained
        unchanged.

    Macro Portability:
        n/a

    Macro See Also:
        move_abs
 */
void
do_move_rel(void)               /* int ([int lines = 1], [int cols = 1]) */
{
    const accint_t lines = get_xinteger(1, 0);
    const accint_t cols = get_xinteger(2, 0);

    move_rel((int)lines, (int)cols);
}


/*  Function:           do_move_abs
 *      move_abs primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: move_abs - Move to an absolute location in the buffer.

        int 
        move_abs([int line = -1], [int column = -1],
                [int bufnum], [int clip = FALSE])

    Macro Description:
        The 'move_abs()' primitive moves the buffer cursor to an
        absolute location, with top of the buffer being position (1,1).

        If a parameter is 0 or omitted, the corresponding line or
        column coordinate is unchanged; positive values set the line
        and/or column.

    Macro Parameters:
        line - Optional integer specifying the line number, if
            positive the cursor is set to the specified line.

        column - Optional integer specifying the column number, 
            if positive the cursor is set to the specified column.

        bufnum - Optional buffer number, if specified the associated
            buffer is affected, otherwise the current buffer.

        clip - Optional int flag, if non-zero the resulting buffer
            cursor shall be clipped to the buffer size.

    Macro Returns:
        The 'move_abs()' primitive returns non-zero on success
        denoting that the cursor moved, otherwise zero if the cursor
        remained unchanged.

    Macro Portability:
        'bufnum' and 'clip' are extensions.

    Macro See Also:
        move_rel
 */
void
do_move_abs(void)               /* int ([int line = -1], [int col = -1], [int bufnum], [int clip = FALSE]) */
{
    const accint_t line = get_xinteger(1, -1);
    const accint_t col  = get_xinteger(2, -1);
    const accint_t clip = get_xinteger(4, FALSE);

    if (line > 0 || col > 0) {
        if (isa_integer(3)) {                   /* optional buffer */
            BUFFER_t *bp = buf_argument(3);

            if (bp && bp != curbp) {
                BUFFER_t *ocurbp = curbp;

                curbp = bp;
                set_hooked();
                move2((LINENO) line, (LINENO) col, (clip ? MOVE_CLIP : 0));
                curbp = ocurbp;
                set_hooked();
                return;
            }
        }
        move2((LINENO) line, (LINENO) col, (clip ? MOVE_CLIP : 0));
    }
}


void
move_abs(const LINENO nline, const LINENO ncol)
{
    trace_ilog("move_abs(nline:%d, ncol:%d, oline:%d, ocol:%d\n", \
        (int)nline, (int)ncol, (int)*cur_line, (int)*cur_col);

    move2(nline, ncol, 0);
}


void
move_rel(LINENO lines, LINENO cols)
{
    const LINENO cline = *cur_line, ccol = *cur_col;

    trace_ilog("move_rel(lines:%d, cols:%d)\n", (int)lines, (int)cols);

    if (lines < 0) {                            /* backwards */
        lines *= -1;
        if (lines >= cline) {
            lines = 1;
        } else {
            lines = cline - lines;
        }
    } else {
        lines += cline;
    }

    if (cols < 0) {                             /* backwards */
        cols *= -1;
        if (cols >= ccol) {
            cols = 1;
        } else {
            cols = ccol - cols;
        }
    } else {
        cols += ccol;
    }

    move2(lines, cols, 0);
}


void
move_next_char(int n, int flags_notused)
{
    const int crlf = (BFTST(curbp, BF_BINARY) ? 0 : 1);
    const LINENO extlines = (ttrows() >= 6 ? ttrows() - 6 : 6),
            bottomline = curbp->b_numlines + extlines;
    LINENO line = *cur_line, col = *cur_col;

    __CUNUSED(flags_notused)

    assert(n > 0);
    if (n <= 0 || *cur_line >= bottomline) {
        acc_assign_int(0);
        return;
    }

    trace_ilog("next_char(line:%d, col:%d, nchars:%d)\n", \
        (int)line, (int)col, (int)n);

    if (crlf /*XXX - ANSI. MAP or MCHAR*/) {
        /*
         *  character
         */
        LINE_t *lp;

        while (n > 0 && line < bottomline) {    /* MCHAR */
            /*
             *  walk line
             *      If <EOL> skip to start of next line
             *      otherwise position within the current line.
             */
            LINENO dot = 0, count = 0;

            lp = vm_lock_line(line);
            dot = (col > 1 ? line_offset2(lp, line, col, LOFFSET_NORMAL) : 0);
            if (dot < (LINENO)llength(lp)) {
                count = line_sizeregion(lp, col, dot, n, NULL, &col);
            }
            vm_unlock(line);
            if ((n -= count) <= 0) {
                goto done;
            }
            ++line, col = 1;
            --n;                                /* EOL */
        }

done:;  trace_ilog("\tdone (line:%d, col:%d)\n", (int)line, (int)col);

    } else {
        /*
         *  binary/8bit
         */
        const int nl = (BFTST(curbp, BF_BINARY) ? 0 : 1);
        int offset, length;
        LINE_t *lp;

        while (n > 0 && line < bottomline) {
            lp = vm_lock_line(line);
            offset = (col > 1 ? line_offset2(lp, line, col, LOFFSET_NORMAL) : 0);
            length = llength(lp);

            if (offset + n <= length) {         /* EOL, move to next line */
                if (offset + n == length) {
                    vm_unlock(line);
                    ++line;
                    col = 1;
                } else {                        /* otherwise, position within current line */
                    col = line_column2(lp, line, offset + n);
                    vm_unlock(line);
                }
                break;                          /* done */
            }
            n -= (length - offset) + nl;        /* remove line, plus optional nl */

            vm_unlock(line);
            ++line, col = 1;
        }
    }

    move2(line, col, 0);
}


void
move_prev_char(int n)
{
    const int crlf = (BFTST(curbp, BF_BINARY) ? 0 : 1);
    LINENO line = *cur_line, col = *cur_col;

    if (n <= 0 || (*cur_col <= 1 && *cur_line <= 1)) {
        acc_assign_int(0);
        return;
    }

    trace_ilog("prev_char(line:%d, col:%d, nchars:%d)\n", line, col, n);

    if (crlf /*XXX - ANSI. MAP or MCHAR*/) {
        /*
         *  character
         */
        const LINECHAR *cp, *start, *end;
        int count, column;
        LINE_t *lp;

        while (n > 0 && line >= 1) {            /* MCHAR */
            /*
             *  setup line references
             */
            lp = vm_lock_line(line);
            cp = start = ltext(lp);
            end = start + llength(lp);
            ED_TRACE_LINE2(lp)

            /*
             *  count characters within current line
             */
            count = 0;
            column = 1;                         /* first column */
            while (cp < end) {
                int prevcolumn, width, length;
                int32_t ch;

                if (col > 0 && column >= col) { /* cursor reached */
                    trace_ilog("\tcursor hit (column:%d, col:%d)\n", column, col);
                    break;
                }

                prevcolumn = column;
                width = character_decode(column, cp, end, &length, &ch, NULL);
                column += width;

                if (1 == n &&                   /* optimise backone() */
                        (col > 0 && column >= col)) {
                    trace_ilog("\tbackchar1(%d)\n", prevcolumn);
                    col = prevcolumn;           /* ... previous location */
                    goto done;
                }
                ++count;
                cp += length;
            }

            /*
             *  position within current line
             */
            if (count >= n) {
                int diff = count - n;

                col = 1;
                cp = start;
                trace_ilog("\tbackchar2(count:%d, n:%d, diff:%d)\n", count, n, diff);
                while (diff > 0 && cp < end) {
                    int width, length;
                    int32_t ch;

                    width = character_decode(col, cp, end, &length, &ch, NULL);
                    col += width;
                    cp += length;
                    --diff;
                }
                break;
            }

            /*
             *  previous line
             */
            trace_ilog("\tprevious line\n");
            if (--line < 1) {
                line = col = 1;                 /* top of buffer */
                break;
            }

            if ((n -= ++count) <= 0) {          /* EOL */
                col = line_column_eol(line);
                break;
            }
            col = 0;
        }
done:;  trace_ilog("==> line:%d, col:%d\n", line, col);

    } else {
        /*
         *  binary/8bit
         */
        const int nl = (BFTST(curbp, BF_BINARY) ? 0 : 1);
        int dot = line_offset(line, col, LOFFSET_NORMAL);

        while (n > 0) {
            if (dot > n) {
                dot -= n;
                n = 0;

            } else if (dot)  {
                n -= dot;
                dot = 0;

            } else {
                if (--line < 1) {
                    line = 1;                   /* top of buffer */
                    break;
                }
                dot = llength(linep(line));     /* EOL */
                n -= nl;
            }
        }
        col = line_column(line, dot);
    }

    move2(line, col, 0);
}


void
mov_forwchar(int n)
{
    assert(n > 0);
    mov_char(n, 1);
}


int
mov_backchar(int n, int wrap)
{
    assert(n > 0);
    if (*cur_col <= 1 && *cur_line > 1) {
        if (wrap) {
            u_dot();
            win_modify(WFMOVE);
            --(*cur_line);
            win_modify(WFMOVE);
            do_end_of_line();
            return TRUE;
        }
        return FALSE;
    }
    mov_char(n, 0);
    return FALSE;
}


void
mov_forwline(int n)
{
    assert(n > 0);
    mov_line(n, 1);
}


void
mov_backline(int n)
{
    assert(n > 0);
    mov_line(n, 0);
}


void
mov_gotoline(int n)
{
    assert(n > 0);

    if (1 != *cur_line || 1 != *cur_col) {
        u_dot();
        win_modify(WFMOVE);
        *cur_col = *cur_line = 1;
        win_modify(WFMOVE);
    }

    if (n > 1) {
        mov_forwline(n - 1);
    }
}


static void
mov_char(const int n, const int direction)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    int ncol;

    assert(n > 0);
    assert(0 == direction || 1 == direction);

    if (direction) {                            /* right */
        if ((ncol = ccol + n) < ccol) {
            ncol = LINEMAX;
        }
    } else {                                    /* left */
        if (n >= ccol) {
            ncol = 1;
        } else {
            ncol = ccol - n;
        }
    }
    move(cline, ncol, MOVE_OBEY);
}


static void
mov_line(const int n, const int direction)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    int nline;

    assert(n > 0);
    assert(0 == direction || 1 == direction);

    if (direction) {                            /* down */
        if ((nline = cline + n) < cline) {
            nline = LINEMAX;
        }
    } else {                                    /* up */
        if (n >= cline) {
            nline = 1;
        } else  {
            nline = cline - n;
        }
    }
    move(nline, ccol, MOVE_OBEY);
}


static void
move2(const LINENO nline, const LINENO ncol, int flags)
{
    const LINENO oline = *cur_line, ocol = *cur_col;

    trace_ilog("move2(nline:%d, ncol:%d, oline:%d, ocol:%d\n", (int)nline, (int)ncol, (int)oline, (int)ocol);

    if (nline > 0 && ncol > 0) {
        if (oline != nline || ocol != ncol) {
            move(nline, ncol, flags);
            return;
        }
    } else if (nline > 0) {
        if (oline != nline) {
            move(nline, ocol, flags);
            return;
        }
    } else if (ncol > 0) {
        if (ocol != ncol) {
            move(oline, ncol, flags);
            return;
        }
    }

    acc_assign_int(0);
    trace_ilog("\t= [line:%d, col:%d] : 0\n", (int)nline, (int)ncol);
}


static int
move(LINENO nline, LINENO ncol, int flags)
{
    const LINENO cline = *cur_line, ccol = *cur_col;
    const LINENO numlines = curbp->b_numlines;
    int ret = 0;

    trace_ilog("move(cline:%d, ccol:%d, nline:%d, ncol:%d]\n",
        (int)cline, (int)ccol, (int)nline, (int)ncol);

    if (ncol < 1) {
        ncol = 1;
    }

    if (nline < 1) {
        nline = 1;
    } else if (nline > numlines) {
        if ((MOVE_CLIP & flags) || BF2TST(curbp, BF2_EOF_CURSOR)) {
            nline = numlines;                   /* clip to buffer */
        } else {
            const LINENO extlines = (ttrows() >= 6 ? ttrows() - 6 : 6);

            if (nline > numlines + extlines) {
                nline = numlines + extlines;
            }
        }
    }

    if (!BFTST(curbp, BF_SYSBUF) && !BF2TST(curbp, BF2_DIALOG)) {

        if (curwp &&                            /* EOL rules */
                (BF2TST(curbp, BF2_EOL_CURSOR) || WFTST(curwp, WF_EOL_CURSOR))) {
            WINDOW_t *wp = curwp;

            if (0 == (MOVE_OBEY & flags)) {
                wp->w_eol_col = 0;              /* disabled (i.e. move_abs) */

            } else if (ncol == ccol && nline != cline) {
                /*
                 *  line change, but no column change
                 */
                const int eol = line_column_eol(nline);

                if (ncol > eol) {               /* pass EOL */
                    if (eol > 0) {
                        if (0 == wp->w_eol_col) {
                            wp->w_eol_col = ncol;
                        }
                        ncol = eol;
                    }
                } else if (wp->w_eol_col &&     /* restore true column */
                                eol >= wp->w_eol_col) {
                    ncol = wp->w_eol_col;
                    wp->w_eol_col = 0;
                }
                flags &= ~MOVE_CLIP;

            } else if (nline == cline && ncol != ccol) {
                /*
                 *  column change, clear history
                 */
                wp->w_eol_col = 0;
            }
        }
    }

    if (MOVE_CLIP & flags) {
        if (ncol != ccol) {
            const int eol = line_column_eol(nline);
            if (ncol > eol) {
                ncol = eol;
            }
        }
    }

    if (cline != nline || ccol != ncol) {       /* cursor change */
        u_dot();
        win_modify(WFMOVE);
        *cur_line = nline;
        *cur_col = ncol;
        win_modify(WFMOVE);
        ret = 1;
    }

    acc_assign_int((accint_t) ret);
    trace_ilog("\t= [line:%d, col:%d] : %d\n", (int)nline, (int)ncol, ret);
    return ret;
}
/*end*/
