#include <edidentifier.h>
__CIDENT_RCSID(gr_m_macro_c,"$Id: m_macro.c,v 1.14 2014/11/16 17:28:41 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_macro.c,v 1.14 2014/11/16 17:28:41 ayoung Exp $
 * High-level macro support functionality.
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
#include <patmatch.h>

#include "accum.h"                              /* acc_...() */
#include "debug.h"                              /* trace */
#include "eval.h"                               /* get_...(x) */
#include "keywd.h"                              /* builtin_...x() */
#include "lisp.h"                               /* atom_...(x) */
#include "m_macro.h"                            /* public interface */
#include "macros.h"                             /* macro_...(x) */
#include "main.h"                               /* globals */


/*  Function: do_command_list
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: command_list - Retrieve list of built-in and active macros.

        list
        command_list(
            [int nomacros = FALSE], [string pattern])

    Macro Description:
        The 'command_list()' primitive returns a sorted list of all
        built-in primitives and the optionally all currently defined
        macros.

        If 'nomacros' is non-zero only built-in primitives are
        retrieved. otherwise the list includes all macros and command
        primitives.

        The optional 'pattern' is a regular expression similar to
        that accepted by the command line shells (see file_match), 
        providing an inclusion filter.

    Macro Parameters:
        nomacros - Optional boolean value, if stated as true then only
            built-ins primitives are retrieved, filtering any
            run-time loaded macros.

        pattern - Optional string value containing the macro-name
            pattern to be applied against each name, only returning
            the matching. A macro-name expression is a shell style
            regular expression (see file_match) (e.g. '*' for
            wildcard, '?' for wild-character, and [..] to select a
            range of characters).

    Macro Returns:
        The 'command_list()' primitive returns a list of strings each
        containing the name of a primitive or macro.

    Macro Portability:
        n/a

    Macro See Also:
        macro_list, file_match
 */
void
do_command_list(void)           /* ([int nomacros = FALSE], [string pattern = NULL]) */
{
    const int nomacros = get_xinteger(1, FALSE);
    const char *pattern = get_xstr(2);
    const char **mp, **maclist = NULL;
    LIST *lp, *newlp = NULL;
    unsigned mac_count = 0;
    const BUILTIN *bp;
    unsigned cindex, mindex;
    int atoms = 0, llen;

    if (! nomacros) {                           /* current macro list */
        if (NULL == (maclist = macro_list(&mac_count))) {
            acc_assign_null();
            return;
        }
    }

    if (pattern && *pattern) {                  /* filter */
        bp = builtin;
        for (cindex = 0; cindex < builtin_count; ++cindex, ++bp) {
            if (patmatch(pattern, bp->b_name, 0)) {
                ++atoms;
            }
        }
        if (maclist) {
            mp = maclist;
            for (mindex = 0; mindex < mac_count; ++mindex, ++mp) {
                if (patmatch(pattern, *mp, 0)) {
                    ++atoms;
                }
            }
        }
    } else {                                    /* non-filtered */
        atoms = builtin_count;
        if (maclist) {
            atoms += mac_count;
        }
        pattern = NULL;
    }

    llen = (atoms * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];
    if (0 == atoms ||
            NULL == (newlp = lst_alloc(llen, atoms))) {
        chk_free((void *)maclist);
        acc_assign_null();
        return;
    }

    lp = newlp;
    bp = builtin;
    mp = maclist;
    cindex = mindex = 0;
    atoms = 0;

    if (maclist) {                             /* insert-sort/merge the lists */
        while (cindex < builtin_count && mindex < mac_count) {
            const char *bname = bp->b_name, *mname = *mp;
            int diff;

            if (pattern) {
                if (! patmatch(pattern, bname, 0)) {
                    ++bp, ++cindex;
                    continue;

                } else if (! patmatch(pattern, mname, 0)) {
                    ++mp, ++mindex;
                    continue;
                }
            }

            if (0 == (diff = strcmp(bname, mname))) {
                lp = atom_push_const(lp, bname);
                ++mp, ++mindex;
                ++bp, ++cindex;

            } else if (diff < 0) {
                lp = atom_push_const(lp, bname);
                ++bp, ++cindex;

            } else {
                lp = atom_push_const(lp, mname);
                ++mp, ++mindex;
            }

            ++atoms;
        }
    }

    while (cindex < builtin_count) {            /* remaining built-ins */
        if (NULL == pattern ||
                patmatch(pattern, bp->b_name, 0)) {
            lp = atom_push_const(lp, bp->b_name);
            ++atoms;
        }
        ++bp, ++cindex;
    }

    if (maclist) {
        while (mindex < mac_count) {            /* remaining macros */
            if (NULL == pattern ||
                    patmatch(pattern, *mp, 0)) {
                lp = atom_push_const(lp, *mp);
                ++atoms;
            }
            ++mp, ++mindex;
        }
    }
    atom_push_halt(lp);

    llen = (atoms * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];
    lst_size(newlp, llen, atoms);               /* set the final size */

    acc_donate_list(newlp, llen);
    if (maclist) {
        chk_free((void *)maclist);
    }
}


/*  Function: do_macro_list
 *      macro_list primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: macro_list - Retrieve list of current macros.

        list
        macro_list([string pattern = NULL])

    Macro Description:
        The 'macro_list()' primitive retrieves a sorted list of all
        defined macros.

        The optional 'pattern' is a regular expression similar to
        that accepted by the command line shells (see file_match), 
        providing an inclusion filter.

    Macro Parameters:
        pattern - Optional string value containing the macro-name
            pattern to be applied against each name, only returning
            the matching. A macro-name expression is a shell style
            regular expression (see file_match) (e.g. '*' for
            wildcard, '?' for wild-character, and [..] to select a
            range of characters).

    Macro Returns:
        The 'macro_list()' primitive returns a list of strings each
        containing the name of a macro.

    Macro Portability:
        n/a

    Macro See Also:
        command_list, file_match
 */
void
do_macro_list(void)             /* list ([string pattern = NULL]) */
{
    const char *pattern = get_xstr(1);
    const char **mp, **maclist = NULL;
    unsigned mac_count, mindex;
    LIST *newlp, *lp;
    int atoms = 0, llen;

    if (NULL == (maclist = macro_list(&mac_count))) {
        acc_assign_null();
        return;
    }

    if (pattern && *pattern) {
        mp = maclist;
        for (mindex = 0; mindex < mac_count; ++mindex, ++mp) {
            if (patmatch(pattern, *mp, 0)) {
                ++atoms;
            }
        }
    } else {
        atoms = mac_count;
        pattern = NULL;
    }


    llen = (atoms * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];
    if (0 == atoms ||
            NULL == (newlp = lst_alloc(llen, atoms))) {
        chk_free((void *)maclist);
        acc_assign_null();
        return;
    }

    lp = newlp;
    mp = maclist;
    for (mindex = 0; mindex < mac_count; ++mindex, ++mp) {
        if (NULL == pattern || patmatch(pattern, *mp, 0)) {
            lp = atom_push_const(lp, *mp);
        }
    }
    atom_push_halt(lp);

    acc_donate_list(newlp, llen);
}
/*end*/