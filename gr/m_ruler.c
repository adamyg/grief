#include <edidentifier.h>
__CIDENT_RCSID(gr_m_ruler_c,"$Id: m_ruler.c,v 1.17 2022/12/03 16:40:17 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_ruler.c,v 1.17 2022/12/03 16:40:17 cvsuser Exp $
 * Ruler primitives.
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

#include "m_ruler.h"                            /* public interface */

#include "accum.h"                              /* acc_...() */
#include "buffer.h"
#include "builtin.h"
#include "echo.h"                               /* ereply() */
#include "eval.h"
#include "lisp.h"                               /* lst_...()/atom_...() */
#include "main.h"                               /* curbp */
#include "ruler.h"                              /* next_..._stop() */
#include "symbol.h"                             /* argv_...() */
#include "window.h"
#include "word.h"                               /* LPUT/LGET */

#define RULER_NTABS     80
#define RULER_MARGINL   1
#define RULER_MARGINR   70

static int              ruler_import(const char *what, int argi, LINENO *ruler, int maxtabs);
static void             ruler_export(const LINENO *ruler, int maxtabs, int mincnt, int aslist);

static int              x_wpmarginr     = RULER_MARGINR;
static int              x_wpmarginl     = RULER_MARGINL;
static int              x_wpmargins     = 0;
static int              x_colorcolumn   = 0;

/*  Function:           ruler_rmargin
 *      ruler_rmargin determines the active right margin of the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Column of the buffers right margin, otherwise 0.
 */
int
ruler_rmargin(const BUFFER_t *bp)
{
    if (bp && bp->b_marginr > 0) {
        return bp->b_marginr;
    }
    return x_wpmarginr;
}


/*  Function:           ruler_lmargin
 *      ruler_lmargin determines the active left margin of the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Column of the buffers left margin, otherwise 0.
 */
int
ruler_lmargin(const BUFFER_t *bp)
{
    if (bp && bp->b_marginl > 0) {
        return bp->b_marginl;
    }
    return x_wpmarginl;
}


/*  Function:           ruler_colorcolumn
 *      ruler_colorcolumn determines the active color-column of the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Column of the buffers color-column, otherwise 0.
 */
int
ruler_colorcolumn(const BUFFER_t *bp)
{
    if (bp && bp->b_colorcolumn > 0) {
        return bp->b_colorcolumn;
    }
    return x_colorcolumn;
}


/*  Function:           do_use_tab_char
 *      use_tab_char primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: use_tab_char - Configure use of hard/soft tabs.

        int
        use_tab_char([int|string yesno], [int global = FALSE])

    Macro Description:
        The 'use_tab_char()' primitive controls whether hard or soft
        tabs shall be utilised within buffers. When enabled
        hard/physical tab characters shall be inserted into the
        buffer otherwise one or more space characters are inserted
        up-to the next indentation/tab stop.

        The effected tabs setting shall either be the current
        buffer if 'global' is given as *FALSE* or omitted,
        otherwise the global configuration setting which is
        referenced during buffer creation defining the default of
        subsequent buffers.

        The argument 'yesno' states the new tab setting value as
        either a string or an integer flag. If omitted the user
        shall be prompted as follows:

>           Fill with tab chars [yes,no]?

    Macro Parameters:
        yesno - Optional string or integer containing the required status.
            A string value of 'yes' enables with 'no' disabling. An
            integer of '1' enables, '0' disables and '-1' returns
            the current with effecting any change.

        global - Optional integer flag states the effected resource, if
            *FALSE* or omitted then the current buffer otherwise
            the global buffer default.

    Macro Returns:
        The 'use_tab_char()' primitive returns the previous tab
        setting of the selected resource, either the current buffer
        or the global.

    Macro Portability:
        n/a

    Macro See Also:
        tabs, inq_tabs
 */
void
do_use_tab_char(void)           /* int ([int|string yesno], [int global = FALSE]) */
{
    const int global_flag = get_xinteger(2, FALSE);
    char buf[MAX_CMDLINE] = {0};
    const char *cp;
    int value = -1;                             /* new value */

    if (isa_integer(1)) {
        value = get_xinteger(1, 0);             /* <0, returns current */

    } else {                                    /* string or NULL */
        if (NULL == (cp = get_arg1("\001Fill with tab chars [^yes,^no]?", buf, sizeof(buf))))  {
            if (global_flag) {                  /* previous value */
                acc_assign_int((accint_t) tabchar_get());
            } else {
                acc_assign_int((accint_t) BFTST(curbp, BF_TABS));
            }
            return;
        }
                                                /* y[es] or Y[es] */
        value = ('y' == *cp || 'Y' == *cp) ? TRUE : FALSE;
    }

    if (global_flag) {
        acc_assign_int((accint_t) tabchar_get());
    } else {
        acc_assign_int((accint_t) BFTST(curbp, BF_TABS));
    }

    if (value >= 0) {
        if (global_flag) {                      /* global value (default) */
            tabchar_set(value);
        } else {                                /* buffer specific */
            if (value) {
                BFSET(curbp, BF_TABS);
            } else {
                BFCLR(curbp, BF_TABS);
            }
        }
    }
}


/*  Function:           do_tabs
 *      tabs primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: tabs - Set buffer tab stops.

        int
        tabs([string tabs | list tabs | int tab, ...])

    Macro Description:
        The 'tabs()' primitive configures the tabs of the current
        buffer to the positions specified within 'tabs'.

        The primitive supports a number of alternative
        specification forms being a set of integer parameters, a
        single string parameter containing space/comma separated
        numbers or a single list of integers. If omitted the user
        shall be prompted for each of the tab points, with an empty
        reply terminating the sequence as follows:

>           Enter tab stop (return terminates):

        Regardless of the form each should be a sequence of columns
        in ascending order. Tabs for the reminder of the line are set
        using the difference between the last two tabs stated, 
        starting at the last specified.

      Example::

        The following sets the first tab at four spaces and all sequence
        tabs to three resulting in the tabs at (5, 8, 11, 14 ...)

>           tabs(5, 8);

        As the tab primitive allows a number of specification forms, 
        all the following are equivalent;

>           tabs("5 8");
>           tabs("5,8");
>           list ttabs = {5, 8};
>           tabs(ttabs);

    Macro Parameters:
        tabs - Optional tabs specification, being the sequence of columns
            in ascending order otherwise the user is prompted.

    Macro Returns:
        The 'tabs()' primitive returns the number of applied tab points
        otherwise if the user was prompted and they aborted -1 is
        returned.

    Macro Portability:
        BRIEF limited the number of unique tab stops at 8, under
        Grief this limit is 80.

    Macro See Also:
        inq_tabs, set_indent, distance_to_tab, distance_to_indent
 */
void
do_tabs(void)                   /* int ([string tabs | list tabs | int tab, ...]) */
{
    LINENO newtabs[BUFFER_NTABS + 1] = {0};
    int tabi = 0;

    tabi = ruler_import("tabs", 1, newtabs, BUFFER_NTABS);
    if (tabi >= 0 && curbp) {
        memcpy(curbp->b_tabs, (const void *)newtabs, sizeof(curbp->b_tabs));
    }
    win_modify(WFHARD);
    acc_assign_int((accint_t)tabi);
}


/*  Function:           do_inq_tabs
 *      inq_tabs primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_tabs - Retrieves the buffer tab specification.

        string|list
        inq_tabs([int bufnum], [int min_count], [int aslist = FALSE]))

    Macro Description:
        The 'inq_tabs()' primitive retrieves the effective tabs
        specification of the current buffer.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        min_count - Optional integer, allows the specification of the
            minimum number of tab points which shall be presented within
            the returned specification.

        aslist - Optional integer boolean flags, if *TRUE* the tab
            specification is returned in the form of a list of integers,
            otherwise by default as a string specification.

    Macro Returns:
        The 'inq_tabs()' primitive either returns a space separated
        string or an integer list containing the current tabs
        specification; both are suitable for use by <tabs>.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tabs, set_indent, distance_to_tab, distance_to_indent
 */
void
inq_tabs(void)                  /* string|list ([int bufnum], [int min_count], [int aslist = FALSE])) */
{
    BUFFER_t *bp = buf_argument(1);
    const LINENO *ruler = (bp ? bp->b_tabs : NULL);

    ruler_export(ruler, BUFFER_NTABS, get_xinteger(2, 0), get_xinteger(3, FALSE));
}


/*  Function:           do_set_tab
 *      set_tab primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_tab - Derive the buffer tab stops.

        int
        set_tab([int increment], [int bufnum])

    Macro Description:
        The 'set_tab()' primitive derives a <tabs> configuration from
        the specified tab increment 'increment'. If omitted the
        user shall be prompted for the tab increment.

>           Enter tab amount:

    Macro Parameters:
        increment - Optional positive integer, stating the tab increment
            if omitted the user shall be prompted.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        Returns the applied tab increment, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tabs
 */
void
do_set_tab(void)                /* int ([int increment], [int bufnum]) */
{
    BUFFER_t *bp = buf_argument(2);
    int newtab = -1;

    if (isa_undef(1)) {                         /* prompt */
        if (bp) {
            char buf[MAX_CMDLINE] = {0};

            if (ereply("Enter tab amount: ", buf, sizeof(buf)) != TRUE || !isdigit(*buf)) {
                acc_assign_int(-1);
                return;
            }
            newtab = atoi(buf);
        }
    } else {
        newtab = get_xinteger(1, -1);
    }

    if (newtab >= 0 && bp) {                    /* define ruler */
        memset(bp->b_tabs, 0, sizeof(bp->b_tabs));
        bp->b_tabs[0] = (newtab > 0 ? newtab : -1);
        bp->b_tabs[1] = -1;
    }

    acc_assign_int(newtab);
}


/*  Function:           inq_tab
 *      inq_tab primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_tab - Derive the tab increment.

        int
        inq_tab([int bufnum])

    Macro Description:
        The 'inq_tab()' primitive derives the tab increment in force
        at the current cursor position.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        Returns the current tab increment, otherwise the default of
        8 if none is active.

    Macro Portability:
        An Grief extension.

    Macro See Also:
        tabs, set_indent, set_ruler
 */
void
inq_tab(void)                   /* int ([int bufnum]) */
{
    BUFFER_t *bp = buf_argument(1);
    int tab = 8;

    if (bp) {
        const LINENO *ruler = bp->b_tabs, *cursor = ruler;
        int cnt = BUFFER_NTABS;

        while (*cursor >= 0 && cnt-- > 0)
            ++cursor;
        if (cursor > (ruler + 1)) {             /* end of ruler */
            tab = cursor[-1] - cursor[-2];
        } else if (cursor == (ruler + 1)) {
            tab = ruler[0];                     /* single value */
        }
        if (tab <= 0) tab = 8;
    }
    acc_assign_int((accint_t)tab);
}


/*  Function:           do_distance_to_tab
 *      distance_to_tab primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: distance_to_tab - Calculate distance to next tab.

        int
        distance_to_tab([int column])

    Macro Description:
        The 'distance_to_tab()' primitive calculates the distance in
        characters to the next tab from either the specified
        'column' otherwise if omitted the current cursor position.

    Macro Parameters:
        column - Optional column if omitted the current buffer position
            is referenced.

    Macro Returns:
        Returns the number of columns/characters between the referenced
        column and the next tab stop. If the referenced column is on a
        tab stop, then the number of characters to the next tab stop
        shall be returned.

    Macro Portability:
        n/a

    Macro See Also:
        tabs, distance_to_indent
 */
void
do_distance_to_tab(void)        /* int ([int column]) */
{
    accint_t col = get_xinteger(1, *cur_col);

    if (col < 1) col = 1;
    acc_assign_int((accint_t) (ruler_next_tab(curbp, col) - col + 1));
}


/*  Function:           do_distance_to_ident
 *      distance_to_ident primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: distance_to_indent - Calculate distance to next indent.

        int
        distance_to_indent([int column])

    Macro Description:
        The 'distance_to_indent()' primitive calculates the distance
        in characters to the next active indentation from either
        the specified 'column' otherwise if omitted the current
        cursor position.

        The active indentation information is sourced from the
        first available specification in order from the following:

            o Ruler specification (see set_ruler).
            o Buffer indentation (see set_indent).
            o Tab specification (see tabs).

    Macro Parameters:
        column - Optional column if omitted the current buffer position
            is referenced.

    Macro Returns:
        Returns the number of columns/characters between the referenced
        column and the next indentation. If the referenced column is on
        a tab stop, then the number of characters to the next tab stop
        shall be returned.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        distance_to_tab, set_ruler, set_indent, tabs
 */
void
do_distance_to_indent(void)     /* int ([int column]) */
{
    accint_t col = get_xinteger(1, *cur_col);

    if (col < 1) col = 1;
    acc_assign_int((accint_t) (ruler_next_indent(curbp, col) - col + 1));
}


/*  Function:           do_set_indent
 *      set_indent primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_indent - Set the buffers default indentation.

        int
        set_indent([int indent], [int bufnum])

    Macro Description:
        The 'set_indent()' primitive configures the indentation value
        for the specified buffer, representing the buffers default
        ruler. Indentation stops are set every 'indent' stops after
        the last stop, with the first column within a line being
        column 1.

        Indenting does not change the size represented by physical
        tabbing, it determines the buffers default indentation when
        a tab-character is self_inserted(), backfilling with either
        spaces and/or physical tabs dependent on whether or not
        hard=tabs are enabled (see use_tab_char).

        An indent value of 0, shall disable the buffers indentation
        setting defaulting to the current tab stop (see tabs)
        unless a ruler is also in effect. If omitted the user shall
        be prompted for a new value as follows:

>           Enter indent amount:

        Note that any user specified ruler (see set_ruler) shall have
        priority over both this setting and the tabs configuration.

    Macro Parameters:
        indent - Optional buffer indentation, if omitted the user shall
            be prompted.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'set_indent()' primitive returns the applied indentation value,
        otherwise if the user was prompted and they aborted -1 is
        returned.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_indent, set_ruler, tabs
 */
void
do_set_indent(void)             /* int ([int indent], [int bufnum]) */
{
    BUFFER_t *bp = buf_argument(2);
    int newindent = -1;

    if (isa_undef(1)) {                         /* prompt */
        if (bp) {
            char buf[MAX_CMDLINE] = {0};

            if (ereply("Enter indent amount: ", buf, sizeof(buf)) != TRUE ||
                    !isdigit(*((unsigned char *)buf))) {
                acc_assign_int(-1);
                return;
            }
            newindent = atoi(buf);
        }
    } else {
        newindent = get_xinteger(1, -1);
    }

    if (newindent >= 0 && bp) {
        bp->b_indent = newindent;
    }
    acc_assign_int(newindent);
}


/*  Function:           inq_ident
 *      inq_ident primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_indent - Get current indentation settings.

        int
        inq_indent([int bufnum])

    Macro Description:
        The 'inq_indent()' primitive retrieves the current buffer
        indentation of the specified buffer 'bufnum'.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        Returns the non-zero indentation value if indentation is
        active, otherwise 0.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_indent
 */
void
inq_indent(void)                /* int ([int bufnum]) */
{
    BUFFER_t *bp = buf_argument(1);
    acc_assign_int((accint_t)(bp ? bp->b_indent : -1));
}


/*  Function:           do_set_ruler
 *      set_ruler primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_ruler - Configure the buffer ruler.

        int
        set_ruler([int bufnum], [list|string|int ...])

    Macro Description:
        The 'set_ruler()' primitive configures the indentation ruler
        of the current buffer to the positions specified within
        'ruler'.

        The primitive supports a number of alternative specification
        forms being either a set of integer parameters, a single
        string parameter containing space/comma separated numbers or
        a single list of integers. If omitted the ruler is cleared.

        Regardless of the form each should be a sequence of columns
        in ascending order. The indentations for the reminder of the
        line are set using the difference between the last two stated
        positions, starting at the last specified.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current
            buffer shall be referenced.

        ruler - Optional ruler specification, being the sequence of
            columns in ascending order otherwise the ruler is cleared.

    Macro Returns:
        The 'set_ruler()' primitive returns the number of applied
        ruler points, 0 is the ruler was cleared otherwise -1 on
        error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        tabs, inq_ruler
 */
void
do_set_ruler(void)              /* ([int bufnum], [list|string|int ruler = NULL]) */
{
    BUFFER_t *bp = buf_argument(1);
    LINENO newruler[RULER_NTABS + 1] = {0};
    int cnt = 0;

    if (NULL == bp) {
        cnt = -1;                               /* invalid buffer */

    } else {
        if (isa_undef(2) ||                     /* clear */
                0 == (cnt = ruler_import("set_ruler", 2, newruler, RULER_NTABS))) {
            chk_free((void *)bp->b_ruler);
            bp->b_ruler = NULL;

        } else if (cnt > 0) {                   /* replace */
            const size_t size = (cnt + 1) * sizeof(LINENO);
            LINENO *ruler;

            if (NULL != (ruler = chk_alloc(size))) {
                chk_free((void *)bp->b_ruler);
                memcpy(ruler, (const void *)newruler, size);
                bp->b_ruler = ruler;
            }
        }
    }
    acc_assign_int((accint_t)cnt);
}


/*  Function:           inq_ruler
 *      inq_ruler primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_ruler - Retrieves the ruler specification.

        string|list
        inq_ruler([int bufnum], [int min_count], [int aslist = FALSE])

    Macro Description:
        The 'inq_ruler()' primitive retrieves the effective
        indentation specification of the current buffer.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        min_count - Optional integer, allows the specification of the
            minimum number of tab points which shall be presented within
            the returned specification.

        aslist - Optional integer boolean flags, if *TRUE* the tab
            specification is returned in the form of a list of integers,
            otherwise by default as a string specification.

    Macro Returns:
        The 'inq_ruler()' primitive either returns a space separated
        string or an integer list containing the current indentation
        specification; both are suitable for use by <set_ruler>.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_ruler
 */
void
inq_ruler(void)                 /* string|list (int bufnum, [int min_count], [int aslist]) */
{
    const BUFFER_t *bp = buf_argument(1);
    const LINENO *ruler = (bp && bp->b_ruler ? bp->b_ruler : NULL);

    ruler_export(ruler, RULER_NTABS, get_xinteger(2, 0), get_xinteger(3, FALSE));
}


/*  Function:           do_set_margins
 *      set_margins primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_margins - Set buffer formatting margins.

        int
        set_margins([int bufnum],
                [int left = NULL], [int right = NULL],
                [int style = NULL], [int colorcolumn = NULL])

    Macro Description:
        The 'set_margins()' primitive configures one or more of the
        specified buffers 'bufnum' margins.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced. A negative bufnum (e.g. -1) shall
            set the global margin parameters, which are applied
            when no buffer specific margin has been set.

        left - Optional integer left margin. A non-positive value
            shall clear the buffer specific margin.

        right - Optional integer right margin. A non-positive value
            shall clear the buffer speific margin.

        style - Optional justification style.

        colorcolumn - Optional colour column.

    Macro Returns:
        The 'set_margins()' primitive returns 0 on success otherwise
        -1 on error.

    Macro Portability:
       A Grief extension.

    Macro See Also:
        inq_margins
 */
void
do_set_margins(void)            /* int ([int bufnum], [int left = NULL], [int right = NULL],
                                            [int style = NULL], [int colorcolumn = NULL]) */
{
    const int bufnum = get_xinteger(1, 0);
    BUFFER_t *bp = (bufnum >= 0 ? buf_argument(1) : NULL);

    if (bufnum >= 0) {
        if (NULL == bp) {
            acc_assign_int((accint_t)-1);
            return;
        }
    }

    if (isa_integer(2)) {                   /* <left> */
        const int value = get_xinteger(2, 0);
        if (bp) {
            bp->b_marginl = (value >= 0 ? value : 0);
        } else {
            x_wpmarginl = (value >= 0 ? value : RULER_MARGINL);
        }
    }

    if (isa_integer(3)) {                   /* <right> */
        const int value = get_xinteger(3, 0);
        if (bp) {
            bp->b_marginr = (value >= 0 ? value : 0);
        } else {
            x_wpmarginr = (value >= 0 ? value : RULER_MARGINR);
        }
    }

    if (isa_integer(4)) {                   /* <style> */
        const int value = get_xinteger(4, 0);
        if (bp) {
            bp->b_margins = value;
        } else {
            x_wpmargins = value;
        }
    }

    if (isa_integer(5)) {                   /* <color-column> */
        const int value = get_xinteger(4, 0);
        if (bp) {
            bp->b_colorcolumn = value;
        } else {
            x_colorcolumn = value;
        }
    }

    if (bp) {
        if (bp->b_marginl > bp->b_marginr) {
            int omarginl = bp->b_marginl;   /* reorder (left < right) */

            bp->b_marginl = bp->b_marginr;
            bp->b_marginr = omarginl;
                                            /* unless margins, clear both */
        } else if (bp->b_marginl == bp->b_marginr) {
            bp->b_marginl = bp->b_marginr = 0;
        }

    } else {
        if (x_wpmarginl > x_wpmarginr) {
            int omarginl = x_wpmarginl;     /* reorder (left < right) */

            x_wpmarginl = x_wpmarginr;
            x_wpmarginr = omarginl;
                                            /* unless margins, reset both */
        } else if (x_wpmarginl == x_wpmarginr) {
            x_wpmarginl = RULER_MARGINL;
            x_wpmarginr = RULER_MARGINR;
        }
    }

    acc_assign_int((accint_t)0);
}


/*  Function:           inq_margins
 *      inq_margins primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_margins - Retrieve buffer formatting margins.

        int
        inq_margins([int bufnum],
            [int &left], [int &right], [int &style], [int &colorcolumn],
            [int global = TRUE])

    Macro Description:
        The 'inq_margins()' primitive retrieves one or more of the
        specified buffers 'bufnum' current margins.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced. A negative bufnum (e.g. -1) shall
            retrieve the global margin parameters, which are applied
            when no buffer specific margin has been set.

        left - Optional left margin.

        right - Optional right margin.

        style - Optional justification style.

        colorcolumn - Optional colour column.

        global - Optional integer flag, if given as FALSE when
            retrieving buffer margins the global settings shall not
            be applied when no buffer specific value is available.

    Macro Returns:
        The 'inq_margins()' primitive returns 0 on success, otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_margins
 */
void
inq_margins(void)               /* int ([int bufnum], [int &left], [int &right],
                                            [int &style], [int &colorcolumn], [int global = 1]) */
{
    const int bufnum = get_xinteger(1, 0);
    BUFFER_t *bp = (bufnum >= 0 ? buf_argument(1) : NULL);
    int ret = -1;

    switch (bufnum)  {
    case -1:        /* global */
        argv_assign_int(2, (accint_t) x_wpmarginl);
        argv_assign_int(3, (accint_t) x_wpmarginr);
        argv_assign_int(4, (accint_t) x_wpmargins);
        argv_assign_int(5, (accint_t) x_colorcolumn);
        ret = 0;
        break;

    default:        /* buffer specific */
        if (bp) {                              
            const int global = get_xinteger(6, TRUE);

            if (global) {                       /* apply globals */
                argv_assign_int(2, (accint_t) bp->b_marginl > 0 ? bp->b_marginl : x_wpmarginl);
                argv_assign_int(3, (accint_t) bp->b_marginr > 0 ? bp->b_marginr : x_wpmarginr);
                argv_assign_int(4, (accint_t) bp->b_margins > 0 ? bp->b_margins : x_wpmargins);
                argv_assign_int(5, (accint_t) bp->b_colorcolumn > 0 ? bp->b_colorcolumn : x_colorcolumn);

            } else {                            /* buffer specific, otherwise -1 */
                argv_assign_int(2, (accint_t) bp->b_marginl > 0 ? bp->b_marginl : -1);
                argv_assign_int(3, (accint_t) bp->b_marginr > 0 ? bp->b_marginr : -1);
                argv_assign_int(4, (accint_t) bp->b_margins > 0 ? bp->b_margins : -1);
                argv_assign_int(5, (accint_t) bp->b_colorcolumn > 0 ? bp->b_colorcolumn : -1);
            }
            ret = 0;
        }
        break;
    }
    acc_assign_int((accint_t) ret);
}


static int
ruler_import(const char *what, int argi, LINENO *ruler, int maxtabs)
{
    int tabi = 0;

    if (isa_undef(argi)) {
        /*
         *  NULL, prompt.
         */
        char buf[MAX_CMDLINE], *end;
        int current, previous = 1;

        while (tabi < BUFFER_NTABS) {
            if (ereply("Enter tab stop (return terminates): ", buf, sizeof(buf)) != TRUE) {
                return -1;
            }
            if (0 == *buf) {
                break;
            }
            errno = 0;
            if ((current = strtol(buf, &end, 10)) <= previous ||
                    NULL == end || errno || '\0' != *end) {
                errorf("%s: invalid ruler specification list <%s>.", what, buf);
                break;
            }
            ruler[tabi++] = (uint16_t)(current - 1);
            previous = current;
        }

    } else if (isa_list(argi)) {
        /*
         *  list of integer values.
         */
        accint_t previous = 0;
        const LIST *lp, *nextlp;

        for (lp = get_xlist(argi); (nextlp = atom_next(lp)) != lp && tabi < maxtabs; lp = nextlp) {
            accint_t current = -1;

            if (! atom_xint(lp, &current) || current <= previous) {
                errorf("%s: invalid ruler specification list <%d>, index (%d).",
                    what, (int) current, tabi);
                break;
            }
            ruler[tabi++] = (LINENO)(current - 1);
            previous = current;
            ++argi;
        }

    } else if (isa_integer(argi)) {
        /*
         *  integer values, max of 10.
         */
        accint_t previous = 0;

        while (tabi < maxtabs && !isa_undef(argi)) {
            accint_t current = -1;

            if (!isa_integer(argi) ||
                    (current = get_xinteger(argi, 0)) <= previous) {
                errorf("%s: invalid ruler specification value <%d>, index (%d).",
                    what, (int) current, tabi);
                break;
            }
            ruler[tabi++] = (LINENO)(current - 1);
            previous = current;
            ++argi;
        }

    } else if (isa_string(argi)) {
        /*
         *  space and/or comma separated integer values.
         */
        const char *str = get_str(argi), *cursor = str;
        accint_t current = -1, previous = 0;
        char *end;

        errno = 0;
        while (tabi < maxtabs && *cursor) {
            if ((current = strtol(cursor, &end, 10)) <= previous ||
                    NULL == end || errno || ('\0' != *end && ' ' != *end && ',' != *end)) {
                errorf("%s: invalid ruler specification <%s>, index (%d).",
                    what, str, tabi);
                break;
            }
            ruler[tabi++] = (LINENO)(current - 1);
            previous = current;
            cursor = end;
            while (' ' == *cursor || ',' == *cursor)
                ++cursor;
        }
    }

    ruler[tabi] = -1;
    return tabi;
}


static void
ruler_export(const LINENO *ruler, int maxtabs, int mincnt, int aslist)
{
    int col = 1, cnt = 0;

    if (ruler) {
        while (cnt < maxtabs && ruler[cnt] >= 0) {
            ++cnt;
        }
        if (cnt < mincnt) {
            cnt = mincnt;                       /* min count */
        }
    }

    if (aslist) {
        LIST *newlp, *lp;
        int len;
                                                /* allocate list storage */
        len = (cnt * sizeof_atoms[F_INT]) + sizeof_atoms[F_HALT];
        if (!ruler || cnt <= 0 ||
                NULL == (lp = newlp = lst_alloc(len, cnt))) {
            acc_assign_null();
            return;
        }
                                                /* build ruler list and donate result */
        cnt = 0;
        while (cnt < maxtabs && ruler[cnt] >= 0) {
            col = ruler[cnt++] + 1;
            lp = atom_push_int(lp, col);
        }
        while (cnt < mincnt) {
            col = ruler_next_stop(ruler, col) + 1;
            lp = atom_push_int(lp, col);
            ++cnt;
        }
        atom_push_halt(lp);
        acc_donate_list(newlp, len);

    } else {
        char *buf = NULL;

        if (ruler && cnt > 0 && NULL != (buf = chk_alloc((cnt * 6) + 1))) {
            char *p = buf;

            cnt = 0;
            while (cnt < maxtabs && ruler[cnt] >= 0) {
                col = ruler[cnt] + 1;
                p += sprintf(p, "%s%d", (cnt ? " " : ""), col);
                ++cnt;
            }
            while (cnt < mincnt) {
                col = ruler_next_stop(ruler, col) + 1;
                p += sprintf(p, "%s%d", (cnt ? " " : ""), col);
                ++cnt;
            }
        }
        acc_assign_str(buf ? buf : "", -1);
        chk_free((void *)buf);
    }
}

/*end*/
