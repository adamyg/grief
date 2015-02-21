#include <edidentifier.h>
__CIDENT_RCSID(gr_syntaxcol_c,"$Id: syntaxcol.c,v 1.18 2015/02/21 22:47:27 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntaxcol.c,v 1.18 2015/02/21 22:47:27 ayoung Exp $
 * Simple column based coloriser.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* margv */
#include "color.h"                              /* ATTR_... */
#include "debug.h"                              /* trace_...() */
#include "echo.h"                               /* errorf() */
#include "eval.h"                               /* get_str() */
#include "lisp.h"                               /* atom_...() */
#include "syntax.h"                             /* syntax basic functionality */
#include "word.h"                               /* LGET_...() */

typedef struct {
    struct SyntaxDriver     sc_driver;
    unsigned                sc_length;
    unsigned *              sc_ruler;
} SyntaxCol_t;

static int                  syncol_select(SyntaxTable_t *st, void *object);
static void                 syncol_destroy(SyntaxTable_t *st, void *object);
static int                  syncol_write(SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end);

static SyntaxCol_t *        syncol_find(int argi, int create);


/*<<GRIEF>>
    Macro: syntax_column_ruler - Column syntax coloriser.

        int
        syntax_column_ruler(
            list ruler, [string attribute], [int|string syntable])

    Macro Description:
        The 'syntax_column_ruler()' primitive sets the column
        originated syntax coloriser.

    Macro Parameters:
        ruler - Ruler specification, represented by a set of
            increasing integer columns plus an optional string
            containing an attribute name (see set_color). If a
            columns trailing attribute is omitted then the
            'default_attr' argument is applied.

            A NULL ruler clears the current ruler.

        default_attr - Optional default attribute specification, if
            omitted "hilite" is assumed.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Macro Returns:
        The 'syntax_column_ruler()' primitive returns the length of
        the resulting ruler, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax
 */
void
do_syntax_column_ruler(void)    /* int (list ruler, [string default_attr = NULL], [int|string syntable]) */
{
    const char *t_attribute,
        *attribute = (isa_string(2) ? get_str(2) : "hilite");
    SyntaxCol_t *col = syncol_find(3, TRUE);
    int attr = -1, ret = 0;

    if (NULL == col) {
        ret = -1;

    } else if ((attr = attribute_value(attribute)) < 0) {
        errorf("syntax_column_ruler: unknown attribute '%s'", attribute);
        ret = -1;

    } else {
        const LIST *collp = get_xlist(1);

        if (NULL != col->sc_ruler) {                /* release current */
            chk_free((void *)col->sc_ruler), col->sc_ruler = NULL;
            col->sc_length = 0;
        }

        if (NULL != collp) {
            accint_t column, prevcolumn = 0;
            const LIST *nextlp, *lp;
            unsigned length = 0;

            /* count list int elements */
            for (lp = collp; lp; lp = nextlp) {
                                                    /* column */
                if (! atom_xint(lp, &column) || column <= prevcolumn) {
                    errorf("syntax_column_ruler: invalid column '%d'", (int)column);
                    break;
                }
                prevcolumn = column;
                nextlp = atom_next(lp);
                                                    /* optional attribute */
                if (nextlp && NULL != (t_attribute = atom_xstr(nextlp))) {
                    if (attribute_value(t_attribute) < 0) {
                        errorf("syntax_column_ruler: unknown attribute '%s'", t_attribute);
                        break;
                    }
                    nextlp = atom_next(nextlp);
                }

                ++length;
            }

            /* build ruler */
            if (length) {
                unsigned t_length = length;
                unsigned *ruler;

                if (NULL != (ruler = chk_alloc((length * 2) * sizeof(*ruler)))) {

                    col->sc_length = length;
                    col->sc_ruler = ruler;

                    for (lp = collp; t_length > 0 && lp; lp = nextlp) {

                        atom_xint(lp, &column);     /* column */
                        *ruler++ = (unsigned) column;
                        nextlp = atom_next(lp);

                                                    /* optional attribute */
                        if (nextlp && NULL != (t_attribute = atom_xstr(nextlp))) {
                            *ruler++ = attribute_value(t_attribute);
                            nextlp = atom_next(nextlp);
                        } else {
                            *ruler++ = (unsigned) attr;
                        }

                        --t_length;
                    }
                    assert(0 == t_length);
                }
            }
            ret = length;
        }
    }
    acc_assign_int((accint_t) ret);
}


static int
syncol_select(SyntaxTable_t *st, void *object)
{
    SyntaxCol_t *col = (SyntaxCol_t *)object;

    __CUNUSED(st)
    if (col->sc_ruler) {
        return 1;
    }
    return 0;
}


static void
syncol_destroy(SyntaxTable_t *st, void *object)
{
    SyntaxCol_t *col = (SyntaxCol_t *)object;

    __CUNUSED(st)
    if (col) {
        if (col->sc_ruler) {
            chk_free((void *)col->sc_ruler), col->sc_ruler = NULL;
            col->sc_length = 0;
        }
    }
}


static int
syncol_write(
    SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end)
{
    SyntaxCol_t *col = (SyntaxCol_t *)object;
    int attr = ATTR_CURRENT;

    assert(cursor <= end);

    if (col->sc_ruler) {
        const LINECHAR *start = cursor-offset;  /* true line buffer start */
        const unsigned *ruler = col->sc_ruler;  /* array of [idx/colour] pairs */
        unsigned length = col->sc_length;

        do {                                    /* foreach (colorset) */
            /*
             *  Ruler:
             *      column/colour
             *          :
             */
            unsigned idx = *ruler++;            /* end of current selection */
            const LINECHAR *t_end = start+idx;  /* buffer address by index */

            if (idx >= offset && cursor < t_end) {
                cursor = syntax_write(st, cursor, (end < t_end ? end : t_end), attr);
            }
            attr = *ruler++;                    /* next colour */

        } while (cursor < end && --length);
    }

    if (cursor < end) {
        syntax_write(st, cursor, end, attr);
    }

    return (0);
}


static SyntaxCol_t *
syncol_find(int argi, int create)
{
    SyntaxTable_t *st;

    if (NULL == (st = syntax_argument(argi, FALSE))) {
        return NULL;
    }

    if (NULL == st->st_drivers[SYNI_COLUMN]) {
        if (create) {
            SyntaxCol_t *col;

            if (NULL == (col = (SyntaxCol_t *)chk_calloc(sizeof(SyntaxCol_t),1))) {
                return NULL;
            }
            col->sc_driver.sd_instance = col;
            col->sc_driver.sd_select   = syncol_select;
            col->sc_driver.sd_write    = syncol_write;
            col->sc_driver.sd_destroy  = syncol_destroy;

            st->st_drivers[SYNI_COLUMN] = &col->sc_driver;
        }
    }

    return (SyntaxCol_t *)st->st_drivers[SYNI_COLUMN]->sd_instance;
}
/*end*/
