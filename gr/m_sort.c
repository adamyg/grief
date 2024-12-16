#include <edidentifier.h>
__CIDENT_RCSID(gr_m_sort_c,"$Id: m_sort.c,v 1.19 2024/12/06 15:46:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_sort.c,v 1.19 2024/12/06 15:46:06 cvsuser Exp $
 * Sort functionality.
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

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "debug.h"                              /* trace */
#include "echo.h"                               /* errorf() */
#include "eval.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "maths.h"                              /* do_com_op */
#include "m_sort.h"

static int              lsort_callback(void *callback, const void *l1, const void *l2);
static int              lsort_compare_fwd(const void *l1, const void *l2);
static int              lsort_compare_bck(const void *l1, const void *l2);
static int              lsort_compare(const LISTV *lvp1, const LISTV *lpv2);


/*  Function:           do_sort_list
 *      sort_list primitive, sorts the list and returns the sorted array value.
 *
 *  Arguments:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *
 *<<GRIEF>>
    Macro: sort_list - Sort list.

        list
        sort_list(list lst,
            [string|int comparator = 0], [int type = 3])

    Macro Description:
        The 'sort_list()' primitive sorts the list and returns the sorted
        array value.

        By default elements are sorted alphabetically yet the sort can
        be modified using the user specified macro or using one of the
        predefined system sort macros.

    Sort Method:
        Grief 2.5.4 and earlier bindly utlilised the system supplied
        quicksort algorithm to implement sort. The characteristics of
        the algorithm could not defined as such was normally unstable, 
        plus may have gone quadratic. (Although quicksort's run time
        is O(NlogN) when averaged over all arrays of length N, the
        time can be O(N**2), quadratic behavior, for some inputs.

        Following Perl and Java in 2.5.5 the default sort
        implementation was replaced with a stable mergesort algorithm
        whose worst-case behavior is O(NlogN). In addition quicksort
        defends against quadratic behaviour by shuffling large arrays
        before sorting.

        A stable sort means that for records that compare equal, the
        original input ordering is preserved. Mergesort is stable, 
        quicksort is not. Stability will matter only if elements that
        compare equal can be distinguished in some other way. That
        means that simple numerical and lexical sorts do not profit
        from stability, since equal elements are indistinguishable.
        However, with a comparison such as

>           int
>           mysort(string a, string b)
>           {
>               return (substr(a, 0, 3) <=> substr(b, 0, 3));
>           }

        stability might matter because elements that compare equal on
        the first 3 characters may be distinguished based on
        subsequent characters. In Perl 5.8 and later, quicksort can
        be stabilized, but doing so will add overhead, so it should
        only be done if it matters.

        The best algorithm depends on many things. On average, 
        mergesort does fewer comparisons than quicksort, so it may be
        better when complicated comparison routines are used.
        Mergesort also takes advantage of pre-existing order, so it
        would be favored for using sort() to merge several sorted
        arrays. On the other hand, quicksort is often faster for
        small arrays, and on arrays of a few distinct values, 
        repeated many times. You can force the choice of algorithm
        with the specification of the third argument. The default
        algorithm is mergesort, which will be stable even if you do
        not explicitly demand it. The stability of the default sort
        is a side-effect that could change in later versions.

>           int
>           mycmp(string a, string b)
>           {
>               return (a <=> b);
>           }

    Note!:
        In the interests of efficiency the normal calling code for
        subroutines is bypassed, with the following effects: elements
        to be compared are passed into 'a' and 'b', are passed by
        reference as such do not modify 'a' and 'b'.

    Macro Parameters:
        list - List to be sorted.

        comparator - Optional string comparison macro or integer
            direction. A string value states the comparison macro to
            be executed. Whereas an integer value states the
            direction, with zero selecting the built
            "sort_list::backward" comparator and a non-zero value
            selecting "sort_list::forward. If omitted the comparator
            defaults to forward.

        type - Optional integer stating the sort type, being

                1 - quicksort.
                2 - mergesort
                3 - heapsort (default).

    Macro Returns:
        Sorted list.

    Macro Portability:
        Second argument allowing either a sort-order or user specified
        callback plus type selection are Grief extensions.

    Macro See Also:
        sort_buffer
 */
void
do_sort_list(void)              /* list (list, [string callback|int order], [int type]) */
{
    const LIST *nextlp, *lp = get_list(1);
    const char *macro = get_xstr(2);            /* NULL | STRING | INT */
    const int type = get_xinteger(3, -1);       /* sort type 1=quicksort,2=mergesort,3=heapsort */
    sortcmp_t cmp = NULL;
    LIST *newlp;
    int atoms, len, i;
    LISTV *lvp;

    /* verify inputs */
    if (lst_isnull(lp)) {
        acc_assign_null();                      /* NULL or empty list */
        return;
    }

    if (macro && macro[0]) {
        trace_log("sort_list: macro=%s\n", macro);

        if (0 == strcmp(macro, "sort_list::forward")) {
            cmp = lsort_compare_fwd;
            macro = NULL;

        } else if (0 == strcmp(macro, "sort_list::backward")) {
            cmp = lsort_compare_bck;
            macro = NULL;

        } else {
            if (! macro_exist(macro, "sort_list")) {
                acc_assign_int(-3);
                return;
            }
            cmp = NULL;
        }
    } else {
        macro = NULL;
        cmp = NULL;
    }

    /* copy */
    atoms = lst_atoms_get(lp);
    if (0 == atoms || NULL == (lvp = chk_alloc(atoms * sizeof(LISTV)))) {
        acc_assign_null();
        return;
    }

    for (i = 0; (nextlp = atom_next(lp)) != lp;) {
        if (!macro)
            if (atom_xlist(lp)) {
                /*
                 *  LIST or RLIST, abort
                 */
                errorf("sort_list: cannot sort a list of lists.");
                acc_assign_null();
                chk_free(lvp);
                return;
            }
        if (atom_xnull(lp)) {                   /* consume NULS */
            lp = nextlp;
            --atoms;
        } else {
            lp += argv_make(lvp + i++, lp);     /* XXX - reference issues */
            assert(lp == nextlp);
        }
    }

    assert(i == atoms);

    /* sort */
    if (atoms) {
        if (NULL == macro && NULL == cmp) {
            cmp = (get_xinteger(2, 0) ?         /* default forward */
                        lsort_compare_bck : lsort_compare_fwd);
        }

        switch (type) {
        case 3:     /* unstable heapsort */
            if (cmp) {
                bsd_heapsort(lvp, atoms, sizeof(LISTV), cmp);
            } else {
                bsd_heapsort_r(lvp, atoms, sizeof(LISTV), (void *)macro, lsort_callback);
            }
            break;
        case 2:     /* stable merge sort - at the cost of memory */
        default:    /* +2.5.5 */
            if (cmp) {
                bsd_mergesort(lvp, atoms, sizeof(LISTV), cmp);
            } else {
                bsd_mergesort_r(lvp, atoms, sizeof(LISTV), (void *)macro, lsort_callback);
            }
            break;
        case 1:     /* unstable qsort */
            if (cmp) {
                bsd_qsort(lvp, atoms, sizeof(LISTV), cmp);
            } else {
                bsd_qsort_r(lvp, atoms, sizeof(LISTV), (void *)macro, lsort_callback);
            }
            break;
        }
    }

    /* return results */
    if (0 >= atoms || NULL == (newlp = argv_list(lvp, atoms, &len))) {
        acc_assign_null();
    } else {
        acc_donate_list(newlp, len);
    }

    chk_free(lvp);
}


/*  Function:           lsort_callback
 *      do_sort_list worker functions.
 *
 *  Parameters:
 *      l1 - First element reference.
 *      l2 - Second element reference.
 *
 *  Returns:
 *      Returns an integer value indicating the relationship between the elements.
 *      A zero value indicates that both are equal.
 *      A value greater then zero indicates the first element is greater then the
 *      second, otherwise a negative value indicates the opposite.
 */
static int
lsort_callback(void *callback, const void *l1, const void *l2)
{
    const LISTV *lvp1 = l1, *lvp2 = l2;
    LIST tmpl[LIST_SIZEOF(3)];
    LIST *newlp, *lp;
    size_t llen;

    /* size arguments */
    llen = sizeof_atoms[F_SYM] + argv_size(lvp1) + argv_size(lvp2) + sizeof_atoms[F_HALT];
    if (llen < sizeof(tmpl)) {                  /* static buffer suitable (general case) */
        newlp = tmpl;
    } else {                                    /* otherwise dynamic */
        if (NULL == (newlp = lst_alloc(llen, 3))) {
            return 0;
        }
    }

    /* build LIST */
    lp  = atom_push_sym(newlp, callback);
    lp += argv_copy(lp, lvp1);
    lp += argv_copy(lp, lvp2);
    atom_push_halt(lp);

    /* execute interface */
    if (newlp != tmpl) {
        lst_check(newlp);
    }
    execute_nmacro(newlp);
    if (newlp != tmpl) {
        lst_free(newlp);
    }
    return (int)acc_get_ival();                 /* -1, 0 or 1 */
}


/*  Function:           lsort_compare_fwd
 *      sort_list primitive forward sort order worker function.
 *
 *  Parameters:
 *      l1 - First element reference.
 *      l2 - Second element reference.
 *
 *  Returns:
 *      Returns an integer value indicating the relationship between the elements.
 *
 *      A zero value indicates that both are equal. A value greater then zero indicates
 *      the first element is greater then the second, otherwise a negative value
 *      indicates the opposite.
 */
static int
lsort_compare_fwd(const void *l1, const void *l2)
{
    return lsort_compare((const LISTV *)l1, (const LISTV *)l2);
}


/*  Function:           lsort_compare_bck
 *      sort_list primitive backwards sort order worker function..
 *
 *  Parameters:
 *      l1 - First element reference.
 *      l2 - Second element reference.
 *
 *  Returns:
 *      Returns an integer value indicating the relationship between the elements.
 *      A zero value indicates that both are equal.
 *      A value greater then zero indicates the second element is greater then the
 *      first, otherwise a negative value indicates the opposite.
 */
static int
lsort_compare_bck(const void *l1, const void *l2)
{
    return lsort_compare((const LISTV *)l2, (const LISTV *)l1);
}


static int
lsort_compare(const LISTV *lvp1, const LISTV *lvp2)
{
    const char *str1;
    accfloat_t float1;
    accint_t int1;
    int val = 0;

    if (listv_int(lvp1, &int1)) {
        accfloat_t float2 = 0;
        accint_t int2 = 0;

        if (listv_float(lvp2, &float2)) {
            int2 = (accint_t) float2;
        } else {
            listv_int(lvp2, &int2);
        }

        if (int1 < int2) {
            val = -1;
        } else if (int1 > int2) {
            val = 1;
        }

    } else if (listv_float(lvp1, &float1)) {
        accfloat_t float2 = 0;
        accint_t int2 = 0;

        if (listv_int(lvp2, &int2)) {
            float2 = (accfloat_t) int2;
        } else {
            listv_float(lvp2, &float2);
        }

        if (float1 < float2) {
            val = -1;
        } else if (float1 > float2) {
            val = 1;
        }

    } else if (listv_str(lvp1, &str1)) {
        const char *str2;

        if (listv_str(lvp2, &str2)) {
            val = strcmp(str1, str2);
        }
    }
    return val;
}

/*end*/
