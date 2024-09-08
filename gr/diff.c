#include <edidentifier.h>
__CIDENT_RCSID(gr_diff_c,"$Id: diff.c,v 1.17 2024/08/25 06:01:53 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: diff.c,v 1.17 2024/08/25 06:01:53 cvsuser Exp $
 * Buffer differ.
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

#include "accum.h"                              // acc_
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "main.h"
#include "map.h"                                // linep etc

#include "diff.h"

static unsigned         diff_flags = 0;         // default flags

//  static void         build_hash(BUFFER_t *bp, unsigned flags);


/*  Function:           do_diff_buffers
 *      diff_buffers primitive
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF-TODO>>
    Macro: diff_buffers - Diff the given buffers.

        int
        diff_buffers(int flags, int buf_id1, int buf_id2,
                [string merge_file], [int new_buf1], [int new_buf2],
                    [list &info])

    Macro Description:
        The 'diff_buffers()' primitive compares two buffers, marking lines
        which have been changed in both buffers, as well as new lines
        based on the comparison logic driven by 'flags'.

(start table,format=nd)
        [Flag   [Meaning                                                    ]

      ! 0x01    If set, then Grief will ignore trailing ^M characters.
                This is useful when comparing a DOS text file against
                a Unix text file.

      ! 0x02    If set, the diff will ignore trailing white space at the
                end of the line when doing the comparison.

      ! 0x04    GRIEF will compress multiple white spaces when doing the
                comparison.

      ! 0x08    All white space is stripped before comparing lines.
(end)

        In order to see these marked lines you will need to enable the
        display of the change-bar margin by setting the 'BF2_HISTATUSLINE'
        in the buffer flags (see set_buffer_flags). Alternatively the
        window flag 'WF_HIMODIFIED', 'WF_HIADDITIONS' and 'WF_HICHANGED'
        to enable line hilited based upon status (see set_window_flags).

        There are essentially three ways to use this primitive. You can
        use it to compare two buffers and determine if the buffers are
        the same; you can create a merged file of differences allowing
        you to manually manage the merging of differences. Or you can
        create one or two new buffers which contain difference lines
        which can convert one file to the other.

        GRIEF will automatically clear the modified, new and deleted
        line flags associated with all lines in both buffers and the
        result of a successful diff will only have the different lines
        in the two buffers displayed (i.e. if you are displaying your
        own modified lines then these markers will be cleared).

    Macro Parameters:

        flags - Integer value being is a set of flags which control how
            the difference is to be performed.

        buf_id1, buf_id2 - Are the buffer IDs of the two buffers to
            compare.

        merge_file - Is the optional name of an output file which will
            be created showing the differences between the two files.
            The style of the output is designed to allow easy
            recognition of changed blocks or for automating the merging
            of the data.

        new_buf1, new_buf2 - Are two optional buffer identifiers. These
            buffers will be created as copies of the input buffer but
            with line markings to show where lines have been inserted, 
            deleted or been modified. The normal line marking mechanism
            cannot show deleted lines in the input buffers, so use these
            arguments if you require this information.

        info - If is specified then a list containing 6 integers are
            returned, containing the number of inserts, deletes and
            updates for each buffer, respectively.

    Macro Returns:
        The 'diff_buffers()' primitive returns -1 if either buffer
        identifier is invalid; otherwise the number of different lines
        in buf_id1 is returned (zero if the two buffers are identical).

    Macro Portability:
        n/a

    Macro See also:
        compare_files, diff_buffers, diff_lines, diff_strings
**/
void
do_diff_buffers(void)
{
//  uint32_t flags = (uint32_t) get_xinteger(1, (accint_t) diff_flags);
    int bn1 = (int) get_xinteger(2, -1);
    int bn2 = (int) get_xinteger(3, -1);
    BUFFER_t *bp1, *bp2;

    /* validate buffer identifiers */
    if (bn1 == -1)
        bp1 = curbp;
    else if ((bp1 = buf_lookup(bn2)) == NULL) {
        acc_assign_int(-1);
        return;
    }

    if (bn2 == -1)
        bp2 = curbp;
    else if ((bp2 = buf_lookup(bn2)) == NULL) {
        acc_assign_int(-1);
        return;
    }

    if (bp1 == bp2) {
        acc_assign_int(0);                      /* same buffers */
        return;
    }

    /* 1. build lines hashs */
//  build_hash(bp1, flags);
//  build_hash(bp2, flags);

    /* 2. compare */
        /*TODO*/

    /* 3. assign results */
    acc_assign_int(0);
}


/*<<GRIEF-TODO>>
    Macro: diff_lines - Diff two buffer lines.

        int
        diff_lines([int flags],
            int lineno1, [int bufnum1], int lineno2, [int bufnum2])

    Macro Description:
        The 'diff_lines()' primitive is reserved for future use.

    Comparison Flags:

        Line comparison flags, which affects the treatment of whitespace
        and character case.

(start table,format=nd)
        [Constant                   [Description                        ]
      ! DIFF_IGNORE_WHITESPACE      Ignore by white-space differences.

      ! DIFF_IGNORE_CASE            Ignore character case, whereby
                                    characters using different case shall
                                    be treated as equivalent.

      ! DIFF_COMPRESS_WHITESPACE    Repeated whitespace characters are
                                    compressed into a single space and
                                    compared as such.

      ! DIFF_SUPPRESS_LEADING       Leading whitespace is ignored.

      ! DIFF_SUPPRESS_TRAILING      Trailing whitespace is ignored.

      ! DIFF_SUPPRESS_LFCR          Line-feed and carriage-return
                                    characters are ignored.
(end table)

    Macro Parameters:
        flags - Optional integer flags defining the rules to be applied
            during their comparison which maybe one or more of the
            predefined flags or'ed together. If omitted or zero
            diff_strings behaves similar to <strcmp>.

        lineno1 - First line number within the buffer 'bufnum1'.

        bufnum1 - Optional first buffer number, if omitted the current
            buffer shall be referenced.

        lineno2 - Second line number within the buffer 'bufnum2'.

        bufnum2 - Optional second buffer number, if omitted the current
            buffer shall be referenced.

    Macro Returns:
        The result of the comparison.

    Macro Portability:
        n/a

    Macro See Also:
        diff_buffers, diff_lines, diff_strings
*/
void
do_diff_lines(void)             /* int([int flags], int line1, [int buffer], 
                                        int line2, [int buffer] */
{
    //TODO
}


/*<<GRIEF>>
    Macro: diff_strings - Compare to strings.

        int
        diff_strings([int flags], string s1, string s2)

    Macro Description:
        The 'diff_strings()' primitive determines whether the specified
        strings are equivalent based on the set of formatting rules
        specified by the comparison flags 'flags'.

    Comparison Flags::

        Line comparison flags, which affects the treatment of whitespace
        and character case.

(start table,format=nd)
        [Constant                   [Description                        ]
      ! DIFF_IGNORE_WHITESPACE      Ignore white-space differences.

      ! DIFF_IGNORE_CASE            Ignore character case, whereby
                                    characters using different case shall
                                    be treated as equivalent.

      ! DIFF_COMPRESS_WHITESPACE    Repeated whitespace characters are
                                    compressed into a single space and
                                    compared as such.

      ! DIFF_SUPPRESS_LEADING       Leading whitespace is ignored.

      ! DIFF_SUPPRESS_TRAILING      Trailing whitespace is ignored.

      ! DIFF_SUPPRESS_LFCR          Line-feed and carriage-return
                                    characters are ignored.
(end table)

    Macro Parameters:
        flags - Optional integer flags defining the rules to be applied
            during their comparison which maybe one or more of the
            predefined flags or'ed together. If omitted or zero
            diff_strings behaves similar to <strcmp>.

        s1 - First string to compare.

        s2 - Second string against which to compare the first.

    Macro Returns:
        The 'diff_strings()' primitive returns zero if the two string are
        equivalent otherwise non-zero.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        diff_buffers, diff_lines, diff_strings, strcmp, strcasecmp
*/
void
do_diff_strings(void)           /* int([int flags], string s1, string s2,
                                            [int &hash1], [int &hash2]) */
{
    uint32_t flags = (uint32_t) get_xinteger(1, (accint_t)diff_flags);
    const char *s1 = get_str(2);
    const char *s2 = get_str(3);
    uint32_t h1, h2;

    h1 = diff_hash((const void *)s1, (uint32_t)strlen(s1), flags);
    h2 = diff_hash((const void *)s2, (uint32_t)strlen(s2), flags);
//  argv_assign_int(4, h1);
//  argv_assign_int(5, h2);
    acc_assign_int(h1 == h2 ? 0 : 1);
}


//  static void
//  build_hash(BUFFER_t *bp, unsigned flags)
//  {
//      BUFFER_t *saved_bp = curbp;
//      uint32_t line, nlines = bp->b_numlines;
//      LINE_t *lp;
//
//      curbp = bp;
//
//      for (lp = linep(1), line = 1; line < nlines; ++line) {
//          uint32_t hash;
//
//          hash = diff_hash(ltext(lp), llength(lp), flags);
//          lp->l_oldlineno = hash;                 // test only
//          lp = lforw(lp);
//      }
//
//      curbp = saved_bp;
//      //TODO - return v;
//  }
/*end*/
