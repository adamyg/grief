#include <edidentifier.h>
__CIDENT_RCSID(gr_search_c,"$Id: search.c,v 1.51 2014/11/27 18:56:53 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: search.c,v 1.51 2014/11/27 18:56:53 ayoung Exp $
 * Search interface.
 *
 *  TODO:
 *      https://lua-toolbox.com/
 *          lrexlib-pcre
 *          and http://rrthomas.github.io/lrexlib/
 *      http://laurikari.net/tre/
 *
 *  JavaScript:
 *      http://ftp.mozilla.org/pub/mozilla.org/js/
 *      http://adaptive-enterprises.com/~d/software/see/
 *      http://www.duktape.org/
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

#define ED_ASSERT
#include <editor.h>
#include <edassert.h>
#include <edalt.h>

#include "search.h"
#include "m_search.h"

#include "accum.h"
#include "anchor.h"
#include "basic.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "line.h"
#include "lisp.h"
#include "main.h"
#include "map.h"
#include "position.h"
#include "symbol.h"
#include "undo.h"
#include "window.h"

#include "regprog.h"                            /* Local regular expression implementation */
#include "regrpl.h"

#if (1)
#define SRCH_ENABLED
#define SRCH_TRACE(x)   search_trace x;
#define SRCH_TRACE2(x)  search_trace x;
#else
#define SRCH_TRACE(x)
#define SRCH_TRACE2(x)
#endif

struct re_state {
    struct regopts      regopts;                /* Options. */
    struct regprog      prog;                   /* Working program. */
    struct regrpl       rpl;                    /* Replacement expression. */

    char                replbuf[MAX_CMDLINE];   /* Replacement input buffer. */

    int                 beg_match;              /* TRUE if matching from start-of-line. */
    int                 end_match;              /* TRUE if matching from end-of-line. */

    const char *        replacement;            /* Replacement specification (if any) */

    LINENO              start_line;             /* First line of marked area. */
    LINENO              start_col;              /* First column of marked area (not implemented). */

    LINENO              end_line;               /* Last line of marked area. */
    LINENO              end_col;                /* Last column (not implemented). */

    LINENO              search_line;            /* Current line. */
    LINENO              search_offset;          /* Offset within line. */

    int                 search_result;          /* Length of the result. */
    int                 search_result2;         /* Optional length of non-repositioned result. */
    };

static const struct regopts defoptions = {
    0,
    TRUE,                                       /* Case sensitive search. */
    TRUE,                                       /* Regular expression characters. */
    RE_BRIEF,                                   /* BRIEF. */
    TRUE,                                       /* Forward search. */
    };

#define GLOBAL_PROMPT   -1
#define GLOBAL_ONCE     0
#define GLOBAL_ALL      1

#define xlatprompt(_g)  ((_g) <  0)
#define xlatonce(_g)    ((_g) == 0)
#define xlatall(_g)     ((_g) >  0)

static int              get_reflags(int argi);

static int              search_flags(int rei, int casei, int blocki);
static int              search_start(struct re_state *rs, int dir, int flags, int pati, int repi);
static int              search_arguments(struct re_state *rs, int pati, int repi);

static int              search_comp(struct re_state *rs, const char *pattern, int patlen);
static int              search_load(struct re_state *rs, int rxhandle);
static __CINLINE int    search_exec(struct regprog *prog, const char *buf, int buflen, int offset);
static const char *     search_capture(const struct regprog *prog, int group, const char **end);
static __CINLINE void   search_end(struct re_state *rs);
static __CINLINE void   search_free(struct re_state *rs);

static int              re_exec(struct regprog *prog, const char *buf, int buflen, int offset);
static const char *     re_capture(const struct regprog *prog, int group, const char **end);
static void             re_destroy(struct regprog *prog);

static int              on_exec(struct regprog *prog, const char *buf, int buflen, int offset);
static const char *     on_capture(const struct regprog *prog, int group, const char **end);
static void             on_destroy(struct regprog *prog);

#if defined(HAVE_LIBTRE)
static int              tre_exec(struct regprog *prog, const char *buf, int buflen, int offset);
static const char *     tre_capture(const struct regprog *prog, int group, const char **end);
static void             tre_destroy(struct regprog *prog);
#endif

static int              search_string(struct re_state *rs, int flags, int patii, int stri);
static int              search_list(struct re_state *rs, int flags, int pati, int listi, int starti);
static int              search_buf(struct re_state *rs, int dir, int flags, int pati);

static int              buffer_search(struct re_state *re, int cursor);

static void             translate_string(int flags, const char *sstr, int slen, int pati, int repi);
static void             translate_buf(int dir, int global, int flags, int pati, int repi);

static int              trans_response(int size, int ntranslations);
static void             trans_hilite(int size);
static void             trans_unhilite(int size);

static const char *     replacement_prompt(struct re_state *rs);
static int              replace_buffer(struct re_state *rs, int interactive);

#if defined(SRCH_ENABLED)
static void             search_trace(const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
#endif

static struct re_state *x_reglast = NULL;
static int              x_regopts_capture = 0;
static int              x_regopts_case_sense = TRUE;
static int              x_regopts_mode = RE_BRIEF;


/*  Function:           search_init
 *      Search engine run-time initialisation.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 */
void
search_init(void)
{
    onig_init();
}


/*  Function:           do_quote_regexp
 *      quote_regexp primitive.
 *
 *      This macro takes a string and quote it so that it can be used in a search
 *      without the regular expression characters causing problems.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: quote_regexp - Quote regexp special characters.

        string
        quote_regexp(string text)

    Macro Description:
        The 'quote_regexp()' primitive quotes (i.e. prefixes with a
        backslash) the following set of predefined characters in the
        specified string 'text', representing most of the regular
        expression special characters.

        The predefined characters are;

            o asterisk (*)
            o backslash (\)
            o caret (^)
            o curvy brackets ({})
            o dollar sign ($)
            o parenthesis (())
            o percentage (%)
            o period (.)
            o pipe (|)
            o plus sign (+)
            o question mark (?)
            o square brackets ([])

    Macro Parameters:
        text - String containing the text to quote, protecting
            special regular expression characters..

    Macro Returns:
        The 'quote_regexp()' primitive returns the resulting quoted
        string.

    Macro Portability:
        n/a

    Macro See Also:
        search_fwd, re_search
 */
void
do_quote_regexp(void)           /* string (string text) */
{
    static const char *rech = "+%@[]<>^$*?{}|"; /* UNIX and BRIEF magic */
    register const char *cp = get_str(1);
    char *acc, *end, *bp;
#define ESCAPESPACE     4
    size_t alen = 0x80;

    bp = acc = acc_expand(alen);                /* use accumulator in 128 byte chunks */
    end = (acc + alen) - ESCAPESPACE;

    while (*cp) {
        if (bp >= end) {                        /* expand accumulator */
            const size_t len = bp - acc;

            if (NULL == (acc = acc_expand(alen += 0x80))) {
                break;
            }
            bp = acc + len;
            end = (acc + alen) - ESCAPESPACE;
        }

        if ('\\' == *cp) {
            *bp++ = *cp++;
            *bp++ = *cp++;
        } else {
            if (strchr(rech, *cp)) {
                *bp++ = '\\';
            }
            *bp++ = *cp++;
        }
    }
    *bp = '\0';

    assert(bp < (end + ESCAPESPACE));
    acc_assign_strlen(bp - acc);
}


/*  Function:           do_search_case
 *      search_case primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: search_case - Set the search pattern case mode.

        int
        search_case([int case])

    Macro Description:
        The 'search_case()' primitive sets or toggles the global
        search case mode, if omitted the current mode is toggled.

        On completion the resulting mode is echo on the command
        prompt, either as

>           Case sensitivity on

        or

>           Case sensitivity off

        By default all searches are case sensitive, that is 'A' and
        'a' are not equivalent, by setting the case mode to a zero
        value, case sensitivity will be ignored when performing
        matches.

        'search_case' affects the default case sensitivity of the
        follow primitives.

            o <search_string>
            o <search_list>
            o <search_fwd> and <search_back>
            o <translate>

    Macro Parameters:
        case - Optional integer value, specifying the new state of
            the case mode. If case is omitted, the current value is
            toggled.

    Macro Returns:
        The 'search_case()' primitive returns the previous value of
        the case mode.

    Macro Portability:
        n/a

    Macro See Also:
        search_back, search_fwd, translate, re_syntax
 */
void
do_search_case(void)            /* ([int case]) */
{
    acc_assign_int(x_regopts_case_sense);         /* previous value */

    x_regopts_case_sense =                        /* set/toggle */
            get_xinteger(1, !x_regopts_case_sense);

    infof("Case sensitivity %s.", x_regopts_case_sense ? "on" : "off");
}


/*  Function:           do_re_syntax
 *      re_syntax primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_syntax - Set the regular expression mode.

        int
        re_syntax([int re], [int case], [int capture])

    Macro Description:
        The 're_syntax()' primitive configures the global regular
        expression syntax mode to the mode 're'. By default, the mode
        is set to the *RE_BRIEF* regular expression syntax.

        're_syntax' affects the defaults of the following primitives;

            o <search_string>
            o <search_list>
            o <search_fwd> and <search_back>
            o <translate>

        The supported regular expression syntax modes.

(start table,format=nd)
        [Value  [Constant       [Description            ]
      ! 0       RE_BRIEF        BRIEF/Crisp Edit mode.
      ! 1       RE_UNIX         Basic Unix syntax.
      ! 2       RE_EXTENDED     POSIX Extended Unix.
      ! 3       RE_PERL         Perl.
      ! 4       RE_RUBY         Ruby style syntax.
(end table)

    Macro Parameters:
        re - Optional integer value, specifying the new regular
            expression syntax to by applied. If omitted, the
            current value is unchanged.

        case - Optional integer value, specifying the new state of
            the global case mode. If 'case' is specified and is zero,
            then searches are performed as case insensitive,
            otherwise when non-zero they shall be case sensitive. If
            omitted, the current value is unchanged.

        capture - Optional integer value, specifying the new state of
            the global replacement pattern reference mode. If
            'RE_PERLVARS' then Perl style capture references shall be
            utilised, when 'RE_AWKVARS' AWK style capture references
            are utilised, otherwise then '0' the style is dependent
            on the 're' mode; see <translate>. If omitted, the
            current value is unchanged.

    Macro Returns:
        The 're_syntax()' primitive returns the current/resulting
        regular expression syntax mode.

    Macro Portability:
        The 'case' and 're' arguments are Grief extensions.

    Macro See Also:
        search_case, re_search
 */
void
do_re_syntax(void)              /* ([int re], [int case], [int capture]) */
{
    if (isa_integer(1)) {                       /* regression expression syntax */
        const int re = get_xinteger(1, RE_BRIEF);

        if (re >= 0) {
            switch (re) {
#if defined(HAVE_LIBTRE)
            case RE_TRE:        /*5*/
#endif
            case RE_RUBY:       /*4*/
            case RE_PERL:       /*3*/
            case RE_EXTENDED:   /*2*/
            case RE_UNIX:       /*1*/
            case RE_BRIEF:      /*0*/
                x_regopts_mode = re;
                break;
            }
        }
    }

    if (isa_integer(2)) {                       /* extension, case-fold */
        const int casesense = get_xinteger(2, FALSE);

        if (casesense >= 0) {
            x_regopts_case_sense = (casesense ? 1 : 0);
        }
    }

    if (isa_integer(3)) {                       /* extension, capture mode */
        const int capture = get_xinteger(3, -1);

        if (capture >= 0) {
            switch (capture) {
            case RE_PERLVARS:   /*2*/
            case RE_AWKVARS:    /*1*/
            case 0:             /*0*/
                x_regopts_capture = capture;
                break;
            }
        }
    }

    acc_assign_int(x_regopts_mode);             /* current value */
}


/*  Function:           do_search_string
 *      search_string primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: search_string - Searches for a pattern in a string.

        int
        search_string(string pattern, string text,
                [int &length], [int re], [int case])

    Macro Description:
        The 'search_string()' primitive searches the string 'text'
        against the expression 'pattern'. The treatment of the
        matched pattern may either be a regular-expression or
        constant string dependent on 're' and with case folding
        based on 'case'.

        Note!:
        The 'search_string' primitive is provided primary for BRIEF
        compatibility. Since 're' and 'case' can reference the current
        global search states, inconsistent and/or unexpected results
        may result between usage; as such it is advised that the
        state-less <re_search> primitive is used by new macros.

    Macro Parameters:
        pattern - A string containing the pattern to be matched.

        text - A string containing the text to be search for the
            specified 'pattern'.

        length - Optional integer variable reference. If ths search
            is successful and specified is populated with the
            length of the matched text.

        re - Optional integer value. If 're' is specified and is
            zero, then 'pattern' is treated as a literal string not
            as a regular expression; behaviour being the same as
            <index>. Otherwise the string shall be treated as a
            regular expression using the current global syntax, see
            <re_syntax> and <search_back> for additional information.

        case - Optional integer value specifying whether case folding
            should occur. If 'case' is specified and is zero, then
            the search is done with case insensitive. If omitted the
            current global setting is referenced.

    Macro Returns:
        The 'search_string()' primitive returns the starting
        character in string where the match was found, or zero if
        the match failed.

        Alternatively, if a '\\c' anchor was encountered within the
        pattern, it returns the length of the text from the position
        of the marker to the end of the matched text plus one.


    Macro Portability:
        n/a

    Macro See Also:
        re_search
 */
void
do_search_string(void)          /* (string pattern, string text, [int &length], [int re], [int case]) */
{
    const int flags = search_flags(/*re*/4, /*case*/5, /*block*/-1);
    struct re_state rs = {0};
    int val = 0;

    if ((val = search_string(&rs, flags, 1, 2)) >= 0) {
        if (val) {
            argv_assign_int(3, rs.search_result);
        }
        search_end(&rs);
    }
    acc_assign_int(val);
}


/*  Function:           do_search_list
 *      search_list primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: search_list - Search list contents.

        int
        search_list([int start], string pattern,
                list expr, [int re], [int case])

    Macro Description:
        The 'search_list()' primitive searches the list 'expr'
        against the expression 'pattern'. The treatment of the
        matched pattern may either be a regular-expression or
        constant string dependent on 're' and case folding based
        on 'case'.

        Note!:
        The 'search_string' primitive is provided primary for Crisp
        compatibility. Since 're' and 'case' can reference the current
        global search states, inconsistent and/or unexpected results
        may result between usage; as such it is advised that the
        state-less <re_search> primitive is used by new macros.

    Macro Parameters:
        start - Optional integer index, states the element offset at
            which search operation shall start. If omitted, the
            search is started from the beginning of the list.

        pattern - A string containing the pattern to be matched.

        expr - A list expression containing the list to be search for
            the specified 'pattern'.

        re - Optional integer value. If 're' is specified and is
            zero, then 'pattern' is treated as a literal string not
            as a regular expression; behaviour being the same as
            <index>. Otherwise the string shall be treated as a
            regular expression using the current global syntax, see
            <re_syntax> and <search_back> for additional information.

        case - Optional integer value specifying whether case folding
            should occur. If 'case' is specified and is zero, then
            the search is done with case insensitive. If omitted the
            current global setting is referenced.

    Macro Returns:
        The 'search_list()' primitive returns the index of the
        matched element, otherwise -1 if no match.

    Macro Portability:
        n/a

    Macro See Also:
        list, re_search
 */
void
do_search_list(void)            /* int ([int start], string pattern, list expr, [int re], [int case]) */
{
    const int flags = search_flags(/*re*/4, /*case*/5, -1);
    struct re_state rs = {0};
    int result;

    if ((result = search_list(&rs, flags, 2, 3, 1)) >= -1) {
        search_end(&rs);
    } else {
        result = -1;
    }
    acc_assign_int(result);
}


/*  Function:           do_search_buf
 *      search_fwd and search_back buffer search primitives.
 *
 *  Parameters:
 *      dir - Direction flag.
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: search_fwd - Buffer search.

        int
        search_fwd(string pattern, [int re], [int case],
                [int block], [int length])

    Macro Description:
        The 'search_fwd()' primitive searches forward within the
        active buffer from the current cursor position to the buffer
        end (or block) against the expression 'pattern'. The
        treatment of the matched pattern may either be a
        regular-expression or constant string dependent on 're' and
        case folding based on 'case'.

        Note!:
        The 'search_fwd' primitive is provided primary for BRIEF
        compatibility. Since 're' and 'case' can reference the
        current global search states, inconsistent and/or unexpected
        results may result between usage; as such it is advised that
        the state-less <re_search> primitive is used by new macros.

    Macro Parameters:
        pattern - A string containing the pattern to be matched.

        re - Optional integer value. If 're' is specified and is
            zero, then 'pattern' is treated as a literal string not
            as a regular expression; behaviour being the same as
            <index>. Otherwise the string shall be treated as a
            regular expression using the current global syntax, see
            <re_syntax> and <search_back> for additional information.

        case - Optional integer value specifying whether case folding
            should occur. If 'case' is specified and is zero, then
            the search is done with case insensitive. If omitted the
            current global setting is referenced.

        block - Optional integer flag. If 'block' is specified and is
            non-zero, the search operations are limited to the
            current marked region.

        length - Optional inter flag. If 'length' is specified and is
            non-zero, indicates that the total length of the text
            should be returned; regardless of any '\\c' anchors in
            the pattern.

    Macro Returns:
        The 'search_fwd()' primitive returns the length of the
        matched text plus one otherwise zero or less if no match was
        found.

        Alternatively, if 'length' is specified the total length of
        the matched text is returned. Otherwise if a '\\c' anchor was
        encountered within the pattern, it returns the length of the
        text from the position of the marker to the end of the
        matched text plus one.

    Macro Portability:
        n/a

    Macro See Also:
        search_back

 *<<GRIEF>>
    Macro: search_back - Backwards buffer search.

        int
        search_back(string pattern, [int re], [int case],
                [int block], [int length])

    Macro Description:
        The 'search_back()' primitive searches backwards within the
        active buffer from the current cursor position to the buffer
        top (or block) against the expression 'pattern'. The
        treatment of the matched pattern may either be a
        regular-expression or constant string dependent on 're' and
        case folding based on 'case'.

        Note!:
        The 'search_back' primitive is provided primary for BRIEF
        compatibility. Since 're' and 'case' can reference the
        current global search states, inconsistent and/or unexpected
        results may result between usage; as such it is advised that
        the state-less <re_search> primitive is used by new macros.

    Macro Parameters:
        pattern - A string containing the pattern to be matched.

        re - Optional integer value. If 're' is specified and is
            zero, then 'pattern' is treated as a literal string not
            as a regular expression; behaviour being the same as
            <index>. Otherwise the string shall be treated as a
            regular expression using the current global syntax, see
            <re_syntax> and <search_back> for additional information.

        case - Optional integer value specifying whether case folding
            should occur. If 'case' is specified and is zero, then
            the search is done with case insensitive. If omitted the
            current global setting is referenced.

        block - Optional integer flag. If 'block' is specified and is
            non-zero, the search operations are limited to the
            current marked region.

        length - Optional inter flag. If 'length' is specified and is
            non-zero, indicates that the total length of the text
            should be returned; regardless of any '\\c' anchors in
            the pattern.

    Macro Returns:
        The 'search_back()' primitive returns the length of the
        matched text plus one otherwise zero or less if no match was
        found.

        Alternatively, if 'length' is specified the total length of
        the matched text is returned. Otherwise if a '\\c' anchor was
        encountered within the pattern, it returns the length of the
        text from the position of the marker to the end of the
        matched text plus one.

    Macro Portability:
        n/a

    Macro See Also:
        search_fwd
 */
void
do_search_buf(int dir)          /* int (string pattern, [int re], [int case], [int block], [int length]) */
{
    const int flags = search_flags(/*re*/2, /*case*/3, /*block*/4);
    const int length = get_xinteger(/*length*/5, 0);
    struct re_state rs = {0};
    int result;

    if ((result = search_buf(&rs, dir, flags, /*pattern - optional*/-1)) >= 0) {
        if (result) {
            if (length && rs.search_result2 > 0) {
                result = rs.search_result2;
            } else {
                result = rs.search_result;
            }
            ++result;                           /* +1 */
        }
        search_end(&rs);

    } else {
        result = -1;
    }

    acc_assign_int(result);
}


/*  Function:           re_search
 *      re_search primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_search - Search for a string.

        int
        re_search([int flags], [string pattern],
            [declare object], [int start], [int lensym])

    Macro Description:
        The 're_search()' primitive is a generalised search interface,
        combining the functionality of the more specialised search
        primitives but presenting consistent and stateless search
        capabilities.

        In addition both the 're' and 'case' modes are explicit based
        upon the flags values.

    Macro Parameters:
        flags - Optional integer flags, one or more of the 'SF_XXX'
            flags OR'ed together control the options to be applied
            during the search.

        pattern - A string containing the pattern to be matched.

        object - Optional object to be searched. The search depends
            on the type of 'object'. If 'object' is a string, then a
            string search is performed. If 'object' is a list then a
            list search is done. Ifn an integer the buffer associated
            with the stated buffer number is searched, otherwise if
            omitted a search on the current buffer is performed.

        start - Optional integer index for list objects, states the
            element offset at which search operation shall start. If
            omitted, the search is started from the beginning of the
            list.

        lensym - Optional integer reference for string objects is
            populated with the length of the matched region.

    Flags::

        General control flags.

(start table)
        [Constant           [Description                                ]
      ! SF_BACKWARDS        Search in backwards direction. By default
                            searchs are performed in a forward
                            direction; only meaningful for buffer
                            searches or translates.
      ! SF_IGNORE_CASE      Ignore/fold case.
      ! SF_BLOCK            Restrict search to current block.
      ! SF_LINE             Line mode.
      ! SF_LENGTH           Return length of match.
      ! SF_MAXIMAL          BRIEF maximal search mode.
      ! SF_CAPTURES         Capture elements are retained.
      ! SF_QUIET            Do not display progress messages.
      ! SF_GLOBAL           Global translate (*); <re_translate> only.
      ! SF_PROMPT           Prompt for translate changes;
                                <re_translate> only.
      ! SF_AWK              awk(1) style capture references.
      ! SF_PERLVARS         perl(1) style capture references.
(end table)

        (*) PROMPT is given priority over GLOBAL.

        Syntax selection, one of the following mutually exclusive
        options can be stated.

(start table)
        [Constant           [Description                                ]
      ! SF_NOT_REGEXP       Treat as plain test, not as a regular
                            expression.
      ! SF_BRIEF/SF_CRISP   BRIEF/CRiSP/GRIEF expressions (default).
      ! SF_UNIX             Unix regular expressions.
      ! SF_EXTENDED         Extended Unix expressions.
      ! SF_PERL             PERL syntax.
      ! SF_RUBY             Ruby syntax.
(end table)

    Macro Returns:
        The 're_search()' primitive return is dependent on the type
        of the object searched; matching the corresponding
        specialised primitive <search_fwd>, <search_list> and
        <search_string>. These are summarised below.

(start table)
        [Object Type        [return                                     ]
      ! buffer              on success the length of the matched text;
                            otherwise 0 if no match or -1 on error.
      ! list                on success index of the matched element,
                            otherwise -1 if on match.
      ! string              on success the index of first character
                            of match, otherwise 0 if no match.
(end table)

        On error 're_search' returns -1.

    Macro Portability:
        n/a

    Macro See Also:
        re_translate, search_fwd, search_back, search_string, search_list, re_syntax
 */
void
do_re_search(void)              /* int ([int flags], [string pattern], [declare object], [int start], [int lensym]) */
{
    const int flags = get_reflags(1);
    BUFFER_t *bp = (isa_undef(3) ? curbp :
                        (isa_integer(3) ? buf_argument(3) : (BUFFER_t *)-1));
    struct re_state t_rs = {0}, *rs = &t_rs;

    if (x_reglast) {                            /* destroy previous capture */
        search_free(x_reglast);
        x_reglast = NULL;
    }

    if ((BUFFER_t *)-1 != bp) {
        /*
         *  buffer search (current or specified)
         */
        const int length = (flags >= 0 && (SF_LENGTH & flags));
        int result;

        if (NULL == bp) {
            result = -1;                        /* bad buffer */
        } else {
            BUFFER_t *saved_bp = NULL;

            if (bp != curbp) {
                saved_bp = curbp;
                curbp = bp;
                set_hooked();
            }

            if (SF_CAPTURES & flags) {          /* captures, globalise state */
                x_reglast = rs = chk_calloc(sizeof(struct re_state), 1);
            }

            if ((result = search_buf(rs, TRUE, flags, /*pattern - optional*/-2)) >= 0) {
                if (result) {
                    if (length && rs->search_result2 > 0) {
                        result = rs->search_result2;
                    } else {
                        result = rs->search_result;
                    }
                    ++result;                   /* +1 */
                }
                if (rs != x_reglast) {
                    search_end(rs);
                }
            } else {
                if (rs == x_reglast) {
                    search_free(x_reglast);
                    x_reglast = NULL;
                }
                result = -1;
            }

            if (saved_bp) {
                curbp = saved_bp;
                set_hooked();
            }
        }
        acc_assign_int(result);

    } else if (isa_string(3)) {
        /*
         *  search string
         */
        int result = 0;

        if (SF_CAPTURES & flags) {              /* captures, globalise state */
            x_reglast = rs = chk_calloc(sizeof(struct re_state), 1);
        }

        if ((result = search_string(rs, flags, 2, 3)) >= 0) {
            if (result) {
                argv_assign_int(5, rs->search_result);
            }
            if (rs != x_reglast) {
                search_end(rs);
            }
        } else {
            if (rs == x_reglast) {
                search_free(x_reglast);
                x_reglast = NULL;
            }
        }
        acc_assign_int(result);

    } else if (isa_list(3)) {
        /*
         *  search list
         */
        int result = 0;

        if (SF_CAPTURES & flags) {              /* captures, global state */
            x_reglast = rs = chk_calloc(sizeof(struct re_state), 1);
        }

        if ((result = search_list(rs, flags, 2, 3, 4)) >= -1) {
            if (result >= 0) {
                argv_assign_int(5, rs->search_result);
            }
            if (rs != x_reglast) {
                search_end(rs);
            }
        } else {
            if (rs == x_reglast) {
                search_free(x_reglast);
                x_reglast = NULL;
            }
            result = -1;                        /* general error */
        }
        acc_assign_int(result);

    } else {
        /*
         *  unknown
         */
        acc_assign_int(-1);
    }
}


/*  Function:           do_re_result
 *      capture retrieval.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_result - Retrieve search captures.

        int
        re_result([int capture],
                [string &value], [int &offset], [int &length])

    Macro Description:
        The 're_result()' primitive retrieves the results of the last
        regular expression <re_search> operation. In addition the
        prior search must have had *SF_CAPTURES* enabled.

        Warning!:
        The captured regions reference the original searched object,
        as such the original buffer, list or string *must *not* be
        modified prior to executing 're_result'.

    Macro Parameters:
        capture - Integer capture index, see details below.

        value - String variable reference, is populated with the
            value of the referenced capture.

        offset - Optional integer variable reference, is populated
            with the starting offset of the capture within the
            original source.

    Capture Index::

(start table)
        [Value          [Description                        ]
      ! 1 .. 9          Xth captured expression.
      ! CAPTURE_BEFORE  Everything prior to matched string
      ! CAPTURE_ARENA   Entire matched string.
      ! CAPTURE_AFTER   Everything after to matched string
      ! CAPTURE_LAST    Last parenthesized pattern match.
(end table)

    Macro Returns:
        The 're_result()' primitive returns the length of the capture,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        re_search, search_fwd, search_back, search_string, search_list
 */
void
do_re_result(void)              /* ([int capture], [string &value], [int &offset]) */
{
    const int capture = get_xinteger(1, -100);
    int ret = -1;

    if (x_reglast) {
        const struct regprog *prog = &x_reglast->prog;
        const char *end, *str = NULL;

        if (capture > 0) {
            if (NULL == (str = search_capture(prog, capture - 1, &end))) {
                errorf("re_result: No such capture '%d'.", capture);
            } else {
                ret = (end - str);
            }
        } else {
            switch (capture) {
            case CAPTURE_BEFORE:
                ret = (prog->start  - (str = prog->buf));
                break;
            case CAPTURE_AFTER:
                ret = (prog->bufend - (str = prog->end));
                break;
            case CAPTURE_ARENA:
                ret = (prog->end    - (str = prog->start));
                break;
            case CAPTURE_LAST:
                if (prog->groupno > 0 &&
                        NULL != (str = search_capture(prog, prog->groupno - 1, &end))) {
                    ret = (end - str);
                }
                break;
            default:
                errorf("re_result: Unknown capture '%d'.", capture);
                break;
            }
        }
	argv_assign_nstr(2, (str ? str : ""), (str ? ret : 0));

    } else {
        errorf("re_result: No active captures.");
    }

    acc_assign_int((accint_t) ret);
}


/*  Function:           do_translate
 *      translate primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: translate - Buffer search and replace.

        int
        translate(string pattern, string replacement,
                [int global], [int re], [int case],
                    [int block], [int forward])

    Macro Description:
        The 'translate()' primitive perform string translations
        within the current buffer. The translate starting at the
        current cursor position, continuing until the end of the
        buffer or optionally terminated by the end user.

        If either the 'pattern' or 'replacement' is not specified,
        then the user is prompted for the change.

        Unless 'global' is specified, during translate operations the
        user to informed on the first match and prompted for the
        desired action.

>           Change [Yes|No|Global|One|Entire|Abort,Top|Center]?

        the actions being

            Yes     - Replace current match with the pattern and continue
                        searching.

            No      - Do not replace current match with the pattern and
                        continue searching.

            Global  - Replace all matches from cursor to end of file.

            Entire  - Replace all matches in the file.

            Abort   - Stop the translation.

            Top     - Moves cursor and the associated text for better
                        visibility to top of page and re-prompts.

            Center  - Centers the cursor and the associated text for
                        better visibility and re-prompts.


        Note!:
        The 'translate' primitive is provided primary for BRIEF
        compatibility. As the values of 're' and 'case' can reference
        the current global state resulting in possible inconsistent
        results between usage, it is advised that the state-less
        <re_translate> primitive is used by new macros.

    Macro Parameters:
        pattern - String containing the pattern to translate.

        replacement - String containing the replacement expression
            for each matched pattern. It may contain references to
            the pattern's capture groups by 'special variables' using
            one of three forms GRIEF, AWK or Perl, see below.

        global - Optional integer flag controls user prompts. If
            specified as non-zero every occurence of the pattern is
            replaced, otherwise only the first occurance is
            translated. If 'global' is omitted the user is prompted
            on each match.

        re - Optional integer value. If 're' is specified and is
            zero, then 'pattern' is treated as a literal string not
            as a regular expression; behaviour being the same as
            <index>. Otherwise the string shall be treated as a
            regular expression using the current global syntax, see
            <re_syntax> and <search_back> for additional information.

        case - Optional integer value specifying whether case folding
            should occur. If 'case' is specified and is zero, then
            the search is done with case insensitive. If omitted the
            current global setting is referenced.

        block - Optional integer flag. If 'block' is specified and is
            non-zero, the search operations are limited to the
            current marked region.

        forward - Optional integer flag specifying the search
            direction. If 'forward' is specified as non-zero or
            omitted, the translation operation moves forward.
            Otherwise the translation moves backwards until the top
            of buffer.

    Macro Returns:
        The 'translate()' primitive returns the number of
        translations which occurred, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        search_fwd, search_back, re_search
 */
void
do_translate(void)              /* (string pattern, string replacement, [int global],
                                        [int re], [int case], [int block], [int forward]) */
{
    const int global = get_xinteger(/*global*/3, GLOBAL_PROMPT);
    const int flags = search_flags(/*re*/4, /*case*/5, /*block*/6);
    const int dir = get_xinteger(/*foward*/7, TRUE);

    translate_buf(dir, global, flags, 1, 2);
}


/*  Function:           re_translate
 *      re_translate primitive.
 *
 *      Function similar to translate() primitive but allows the
 *      syntax to be specified at the time of the translate and
 *      is a bit more compact.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_translate - Search and replace.

        int
        re_translate([int flags], string pattern,
                [string replacement], [declare object])

    Macro Description:
        The 're_translate()' primitive is a generalised search and
        replace interface, combining the functionality of the more
        specialised primitives but presenting consistent and
        stateless search capabilities.

    Macro Parameters:
        flags - Optional integer flags, one or more of the 'SF_XXX'
            flags OR'ed together control the options to be applied
            during the search and replace.

        pattern - String containing the pattern to translate.

        replacement - String containing the replacement expression
            for each matched pattern. It may contain references to
            the pattern's capture groups by 'special variables' using
            one of three forms GRIEF, AWK or Perl, see below.

        object - Optional object to be searched. The search depends
            on the type of 'object'. If 'object' is a string, then a
            string search is performed; translating a string provides
            similar functionality to awk's sub() and gsub()
            functions. If 'object' is a list then a list search is
            done. If an integer the buffer associated with the stated
            buffer number is searched, otherwise if omitted a search
            on the current buffer is performed.

    Special Variables::

        o GRIEF variables -
            GRIEF style references.

        This style take the form of of the form "\d", where 'd' is a
        numeric group number started at an index of 0; note escapes
        need to be preceded by an additional backslash.

(start table)
            [Variable   [Value                                  ]
          ! \0, \1 ...  Xth + 1 captured expression.
(end table)

        o AWK Variable -
            AWK style references are selected using 'SF_AWK'.

        This style take the form of "\d", where 'd' is a numeric
        group number indexed from 1; note escapes need to be preceded
        by an additional backslash.

(start table)
            [Variable   [Value                                  ]
          ! \0          Matched string.
          ! \1, \2 ...  Xth captured expression.
          ! &           Match string, same as \0.
(end table)

        o PERL Variables -
            PERL style references are selected using 'SF_PERLVARS';
            is it also implied under 'SF_PERL', 'SF_RUBY' and
            'SF_TRE' unless 'SF_AWK' is stated.

        This style is super-set of all three forms. Capture
        references take the form "$x" when 'x' is a numeric group
        number indexed from 1 or one of the special variables $`, $&,
        and $'. In addition the AWK "\x" syntax is supported, yet
        their use should be avoided due to the need to quote the
        escapes.

(start table)
            [Variable   [Value                                  ]
          ! \1, \2 ...  Old style, Xth captured expression.
          ! $1, $2 ...  Xth captured expression.
          ! ${xx}       Xth captured expression.
          ! $+          Last parenthesized pattern match.
          ! $`          Everything prior to the matched string.
          ! $&          Entire matched string.
          ! $'          Everything after the matched string.
(end table)

    Macro Returns:
        The 're_translate()' primitive returns the number of
        translations which occurred, otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        re_search, translate
 */
void
do_re_translate(void)           /* ([int flags], string pattern, [string replacement], [list|string|int|NULL]) */
{
    const int flags = get_reflags(1);

    if (isa_undef(4) || isa_integer(4)) {       /* NULL or bufnum */
        BUFFER_t *bp, *saved_bp = NULL;

        if (NULL != (bp = buf_argument(4))) {
            const int dir = ((flags < 0 || 0 == (SF_BACKWARDS & flags)) ? TRUE : FALSE);
            const int global = (flags < 0 || (SF_PROMPT & flags) ? GLOBAL_PROMPT :
                                    ((SF_GLOBAL & flags) ? GLOBAL_ALL : GLOBAL_ONCE));

            if (bp != curbp) {
                saved_bp = curbp;
                curbp = bp;
                set_hooked();
            }
            translate_buf(dir, global, flags, -2, 3);
            if (saved_bp) {
                curbp = saved_bp;
                set_hooked();
            }
        }

    } else if (isa_string(4)) {                 /* string */
        translate_string(flags, get_str(4), get_strlen(4), 2, 3);

#if (TODO)
    } else if (isa_list(4)) {                   /* list */
        translate_list(flags, get_list(4), 2, 3);
#endif

    } else {
        acc_assign_int(-1);
    }
}


/*  Function:           re_comp
 *      re_comp primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_comp - Compile a regular expression.

        list
        re_comp([int flags], string pattern, [string &error])

    Macro Description:
        The 're_comp()' primitive is reserved for future development.

        The re_comp primitive converts a regular expression string
        (RE) into an internal form suitable for pattern matching.

    Macro Parameters:
        flags - Optional integer flags, one or more of the 'SF_XXX'
            flags OR'ed together control the options to be applied
            during the search and replace.

        pattern - String containing the pattern to translate.

        error - Optional string reference, on an error shall be
            populated with a description of the error.

        parms - Optional string containing configuration parameters.

    Macro Returns:
        The 're_comp()' primitive returns list containing the
        compiled expression, otherwise a null list.

    Warning!:
        DONOT modify the result otherwise you shall encounter
        undefined results during search operations.

    Macro Portability:
        n/a

    Macro See Also:
        re_search, re_translate, re_delete
 */
void
do_re_comp(void)                /* list ([int flags], string pattern, [string &error], [string parms]) */
{
    const int flags = get_reflags(1);
    const char *pattern = get_str(2);
    struct re_state rs = {0};

    search_options(&rs.regopts, TRUE, flags);

    //  parameters/
    //      delcost=1,
    //      inscost=1,
    //      maxcost=0x7fffffff,
    //      subcost=1,
    //      maxdel=0x7fffffff,
    //      maxerr=3,
    //      maxins=0x7fffffff,
    //      maxsub=0x7fffffff
    //
    if (! search_comp(&rs, pattern, get_strlen(2))) {
        acc_assign_null();
        return;
    }

#if (TODO)
//  Cache expression and return handle, as a list.
#endif
}


/*  Function:           re_delete
 *      re_delete primitive.
 *
 *  Parameters:
 *      nothing
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: re_delete - Delete a compiled expression.

        int
        re_delete(list handle)

    Macro Description:
        The 're_delete()' primitive

    Macro Parameters:
        handle -

    Macro Returns:
        The 're_delete()' primitive returns 1 on success, otherwise 0.

    Macro Portability:
        n/a

    Macro See Also:
        re_comp
 */
void
do_re_delete(void)              /* int (int handle) */
{
    /*TODO*/
}


/*
 *  Search flags management
 */

static int
get_reflags(int argi)
{
    const char *sopts;
    int flags = -1;

    if (isa_integer(argi)) {
        flags = get_xinteger(argi, -1);
        if (SF_AWK & flags) {
            if (0 == (flags & (SF_UNIX|SF_EXTENDED|SF_PERL|SF_RUBY))) {
                flags |= SF_UNIX;               /* implied by gsub() ??? */
            }
        }

    } else if (NULL != (sopts = get_xstr(argi))) {
        int m;

        flags = 0;
        while (0 != (m = *sopts++)) {
            switch (m) {
            case 'm': /* Treat string as multiple lines. */
                break;
            case 's': /* Single line string, as such '.' matches any-character including new-line. */
                flags |= SF_LINE;
                break;
            case 'i': /* Ignore case. */
                flags |= SF_IGNORE_CASE;
                break;
            case 'p': /* Prompt. */
                flags |= SF_PROMPT;
                break;
            case 'g': /* Global. */
                flags |= SF_GLOBAL;
                break;
            case 'b': /* Block. */
                flags |= SF_BLOCK;
                break;
            case 'c': /* Captures. */
                flags |= SF_CAPTURES;
                break;
            case 'x': /* White/space and comments. */
                break;

            case 'B': /* BRIEF. */
                flags |= SF_BRIEF;
                break;
            case 'U': /* Unix. */
                flags |= SF_UNIX;
                break;
            case 'X': /* Extended. */
                flags |= SF_EXTENDED;
                break;
            case 'P': /* Perl. */
                flags |= SF_PERL;
                break;
            case 'R': /* Ruby. */
                flags |= SF_RUBY;
                break;
            }
        }
    }
    return flags;
}


static int
search_flags(int rei, int casei, int blocki)
{
    int flags = 0;

    switch (x_regopts_mode) {
    case RE_RUBY:
        flags |= SF_RUBY;
        break;
    case RE_PERL:
        flags |= SF_PERL;
        break;
    case RE_EXTENDED:
        flags |= SF_EXTENDED;
        break;
    case RE_UNIX:
        flags |= SF_UNIX;
        break;
    case RE_BRIEF:
    default:
        break;
    }

    switch (x_regopts_capture) {
    case RE_PERLVARS:
        flags |= SF_PERLVARS;
        break;
    case RE_AWKVARS:
        flags |= SF_AWK;
        break;
    default:
        break;
    }

    if (rei >= 1 && isa_integer(rei)) {         /* regexp option */
        const int reflags = get_xinteger(rei, TRUE);

        /*  BRIEF style flags:
         *
         *      1   Match patterns from left to right, with minimal closure.
         *      2   Match patterns in same direction as search direction, with minimal closure.
         *      3   Match patterns from right to left, with minimal closure.
         *      0   Disable regular expressions.
         *     -1   Match patterns from left to right, with maximum closure.
         *     -2   Match patterns in same direction as search direction, with maximum closure.
         *     -3   Match patterns from right to left, with maximum closure.
         */
        switch (reflags) {
        case 0:
            flags |= SF_NOT_REGEXP;
            break;
        case -1:
        case -2:
        case -3:
            flags |= SF_MAXIMAL;
            break;
        }
    }

    if (casei >= 1 && isa_integer(casei)) {     /* case-sensitive option */
        if (0 == get_xinteger(casei, TRUE)) {
            flags |= SF_IGNORE_CASE;
        }
    } else if (0 == x_regopts_case_sense) {
        flags |= SF_IGNORE_CASE;                /* otherwise global/default */
    }

    if (blocki >= 1 && isa_integer(blocki)) {   /* block-mode option */
        if (get_xinteger(blocki, FALSE) > 0) {
            flags |= SF_BLOCK;
        }
    }

    SRCH_TRACE(("search_flags: regexp:%d, unix:%d, icase:%d, block:%d\n",
        !(flags & SF_NOT_REGEXP), (flags & SF_UNIX), (flags & SF_IGNORE_CASE), (flags & SF_BLOCK)))

    return flags;
}


/*  Function:           search_start
 *      Initialise the search environment.
 *
 *  Parameters:
 *      rs - Status buffer to be populated.
 *      dir - Direction of search (TRUE = forward, FALSE = backwards).
 *      flags - Search flags.
 *      pati - Pattern argument index (If > 0 absolute, otherwise -index and prompting).
 *      repli - Replacement argument index (0 = undefined).
 *
 *  Returns:
 *      TRUE on success otherwise FALSE on error.
 */
static int
search_start(struct re_state *rs, int dir, int flags, int pati, int repi)
{
    struct regopts *regopts = &rs->regopts;
    ANCHOR_t a = {0};

    /*
     *  Load user specifications, load/compile the pattern.
     */
    assert(rs);
    search_options(regopts, dir, flags);
    if (! search_arguments(rs, pati, repi)) {
        search_end(rs);
        return FALSE;
    }

    /*
     *  If block is TRUE, then we only search within marked block.
     */
    if (NULL == curbp) {
        /*
         *  undefined
         */
        rs->search_line = 0;
        rs->search_offset = 0;
        rs->start_line = 0;
        rs->start_col = -1;
        rs->end_line = 0;
        rs->end_col = -1;
        SRCH_TRACE(("search_start:no buffer\n"))

    } else if ((flags >= 0 && (SF_BLOCK & flags)) && anchor_get(NULL, NULL, &a)) {
        /*
         *  marked area
         */
        if (regopts->fwd_search) {
            rs->search_line = a.start_line;
            rs->search_offset = line_offset(a.start_line, a.start_col, LOFFSET_NORMAL);
        } else {
            rs->search_line = a.end_line;
            rs->search_offset = line_offset(a.end_line, a.end_col, LOFFSET_NORMAL);
        }

        rs->start_line = a.start_line;
        rs->start_col = a.start_col;
        rs->end_line = a.end_line;
        rs->end_col = a.end_col;
        SRCH_TRACE(("search_start:block (%d->%d/%d)\n", rs->search_line, rs->end_line, rs->search_offset))

    } else {
        /*
         *  end-of-buffer/top-of-buffer
         */
        rs->search_line = *cur_line;
        rs->search_offset = line_current_offset(LOFFSET_NORMAL);

        if (regopts->fwd_search) {
            rs->start_line = *cur_line;
            rs->start_col = -1;
            rs->end_line = curbp->b_numlines;
            rs->end_col = -1;
        } else {
            rs->start_line = 1;
            rs->start_col = -1;
            rs->end_line = *cur_line;
            rs->end_col = -1;
        }
        SRCH_TRACE(("search_start:buffer (%d->%d/%d)\n", rs->search_line, rs->end_line, rs->search_offset))
    }

    /*
     *  cook input for backwards searchs
     */
    if (! regopts->fwd_search) {
        if (rs->end_match) {
            rs->search_offset = -1;
            ++rs->search_line;
        }
    }

    SRCH_TRACE(("search_start:flags=0x%04x, mode=%d, case=%d, regexp=%d, forward=%d, line/dot=%d/%d -> %d/%d\n", \
        flags, regopts->mode, regopts->case_sense, regopts->regexp_chars, regopts->fwd_search, \
            rs->search_line, rs->search_offset, rs->end_line, (int)rs->end_col))

    return TRUE;
}


/*  Function:           search_options
 *      Setup the search option, inheriting the global configuration.
 *
 *  Parameters:
 *      regopts - Search options object.
 *      fwd_search - *TRUE* if forward search, otherwise *FALSE*.
 *      flags - Search flags, otherwise -1.
 *
 *  Flags:
 *      SF_BACKWARDS        Search in backwards direction.
 *      SF_IGNORE_CASE      Ignore/fold case.
 *      SF_BLOCK            Restrict search to current block.
 *      SF_LINE             Line mode.
 *
 *      SF_LENGTH           Return length of match.
 *      SF_MAXIMAL          BRIEF maximal search mode.
 *      SF_CAPTURES         Capture elements are retained.
 *
 *      SF_GLOBAL           Global translate (*).
 *      SF_PROMPT           Prompt for translate changes.
 *      SF_AWK              awk(1) style capture references.
 *      SF_PERLVARS         perl(1) style capture references.
 *
 *      SF_BRIEF/SF_CRISP   BRIEF/CRiSPEdit expressions.
 *      SF_NOT_REGEXP       Treat as plain test, not as a regular expression.
 *      SF_UNIX             Unix regular expressions.
 *      SF_EXTENDED         Extended.
 *      SF_PERL             PERL syntax.
 *
 *      (*) PROMPT is given priority over GLOBAL.
 *
 *  TODO:
 *      SF_MINIMAL          Non-BRIEF minimal search mode.
 *      SF_STICKY           Do not restore the cursor on completion.
 *      SF_COLUMN           Buffer column search mode, obeying start/end column.
 *      SF_LIST             Search buffer returning a list of matched line numbers.
 *      SF_LIST2            Search buffer returning a list of matched line numbers/columns/length.
 *
 *  Returns:
 *      nothing.
 */
void
search_options(struct regopts *regopts, const int fwd_search, const int flags)
{
    assert(regopts);
    assert(TRUE == fwd_search || FALSE == fwd_search);

    memset(regopts, 0, sizeof(struct regopts));

    regopts->fwd_search = fwd_search;           /* default direction */

    if (flags >= 0) {
        int mode = RE_BRIEF;

        regopts->flags = flags;
        if (SF_RUBY & flags) {
            mode = RE_RUBY;
#if defined(HAVE_LIBTRE)
        } else if (SF_TRE & flags) {
            mode = RE_TRE;
#endif
        } else if (SF_PERL & flags) {
            mode = RE_PERL;
        } else if (SF_EXTENDED & flags) {
            mode = RE_EXTENDED;
        } else if (SF_UNIX & flags) {
            mode = RE_UNIX;
        }
        regopts->mode = mode;

        regopts->case_sense = ((SF_IGNORE_CASE & flags) ? FALSE : TRUE);

        if (SF_NOT_REGEXP & flags) {            /* string match */
            regopts->regexp_chars = 0;
        } else {
            if (RE_BRIEF != mode || (SF_MAXIMAL & flags)) {
                regopts->regexp_chars = -2;     /* Unix/Perl */
            } else {
                regopts->regexp_chars = 1;      /* BRIEF */
            }
        }

        if (SF_BACKWARDS & flags) {
            regopts->fwd_search = FALSE;        /* clear, default TRUE */
        }

        regopts->prompt =                       /* re_translate */
            ((SF_PROMPT & flags) ? TRUE : FALSE);

        regopts->line =                         /* line mode */
            ((SF_LINE & flags) ? TRUE : FALSE);

    } else {
        regopts->flags = 0;
        regopts->mode = x_regopts_mode;
        regopts->case_sense = x_regopts_case_sense;
        regopts->regexp_chars = 1;              /* BRIEF */
    }
}


void
search_defoptions(struct regopts *regopts)
{
    *regopts = defoptions;
}


/*  Function:           search_arguments
 *      Load/compile a search expression and optional replacement arguments.
 *
 *  Parameters:
 *      rs - State buffer.
 *
 *      pati - Pattern argument index. Shall be either an positive
 *          absolute. index value or a negative type. If negative with
 *          if not available the pattern shall be prompted.
 *
 *      repi - Optional replacement argument index, otherwise 0.
 *
 *  Returns:
 *      TRUE or FALSE.
 */
static int
search_arguments(struct re_state *rs, int pati, int repi)
{
    struct regopts *regopts = &rs->regopts;
    const int abspati = (pati >= 1 ? pati : pati * -1);
    char patbuf[MAX_CMDLINE];
    const char *pattern = NULL;
    int load = FALSE;

    assert(abspati >= 0);

    memset(&rs->prog, 0, sizeof(rs->prog));     /* reset program */
    rs->replacement = NULL;

    /*
     *  Search pattern
     */
    if (isa_integer(abspati)) {                 /* load from image (re_comp) */
        load = TRUE;
    } else {                                    /* otherwise string */
        if (pati >= 1) {
            pattern = get_xstr(pati);           /* non prompted */
        } else {                                /* prompted */
            if (NULL == (pattern = get_xarg(abspati,
                    (repi > 0 ? "Pattern: " :
                        (regopts->fwd_search ? "Search fwd: " : "Search back: ")), patbuf, sizeof(patbuf)))) {
                return FALSE;                   /* abort */
            }

            if (0 == *pattern) {                /* pattern required */
                if (repi > 0) {
                    infof("No translate pattern specified.");
                } else {
                    infof("No pattern specified.");
                }
                return FALSE;
            }
        }
    }

    /*
     *  Replacement (if required)
     *
     *      Note, retrieve prior to compiling as ereply() via completion callbacks may
     *      reenter the search subsystem causing issues with the regular-expression
     *      status - SHOULD NO LONGER BE THE CASE.
     */
    if (repi >= 1) {
        const char *replacement = get_xstr(repi);

        if (NULL == replacement) {
            if (pati >= 1) {
                return FALSE;                   /* no prompting permitted */
            }

            if (FALSE == regopts->prompt) {     /* 10/05/11, prompt foreach translation */
                if (NULL == (replacement = replacement_prompt(rs))) {
                    return FALSE;
                }
            }
        } else {
            regopts->prompt = FALSE;
        }
        rs->replacement = replacement;          /* resulting replacement text */
    } else {
        regopts->prompt = FALSE;
    }

    /*
     *  Compile expression (if required)
     */
    if (load) {
        if (-1 == search_load(rs, get_integer(abspati))) {
            return FALSE;
        }
    } else {
        if (! search_comp(rs, pattern, strlen(pattern))) {
            return FALSE;
        }
    }

    /*
     *  Summaries special cases
     */
    rs->beg_match = ('^' == *pattern || (RE_BRIEF == regopts->mode && '<' == *pattern)) ? 1 : 0;
    rs->end_match = ('$' == *pattern || (RE_BRIEF == regopts->mode && '>' == *pattern)) ? 1 : 0;

    SRCH_TRACE(("\tpattern='%s' (%d/%d)\n", pattern, rs->beg_match, rs->end_match))
    if (rs->replacement) {
        SRCH_TRACE(("\treplacment='%s'\n", rs->replacement))
    }
    return TRUE;
}


static int
search_comp(struct re_state *rs, const char *pattern, int patlen)
{
    struct regopts *regopts = &rs->regopts;
    struct regprog *prog = &rs->prog;

    if (NULL == pattern) {
        return FALSE;
    }

    prog->setpos  = NULL;
    prog->start   = NULL;
    prog->end	  = NULL;
    prog->exec    = NULL;
    prog->capture = NULL;
    prog->destroy = NULL;
    prog->groupno = -1;

#if defined(HAVE_LIBTRE)
    if (RE_TRE == regopts->mode) {
        /*
         *  RE_TRE
         */
        regex_t *regexp = &prog->rx.tre.regex;
        int r, flags = REG_EXTENDED;

        if (! regopts->case_sense) {
            flags |= REG_ICASE;
        }

        if (1 == regopts->regexp_chars) {
            flags |= REG_UNGREEDY;
        }

        SRCH_TRACE(("search: TRE\n"))
        if (REG_OK != (r = tre_regcomp(regexp, pattern, flags))) {
            char err[128] = {0};

            tre_regerror(r, regexp, err, sizeof(err));
            ewprintf("regexp: %s", err);
            return FALSE;
        }

        prog->engine  = REGPROG_TRE;
        prog->hasgrps = TRUE;                   /* TODO */
        prog->exec    = tre_exec;
        prog->capture = tre_capture;
        prog->destroy = tre_destroy;

    } else
#endif
    if (regopts->mode >= RE_EXTENDED) {
        /*
         *  RE_EXTENDED, RE_PERL, RE_RUBY
         */
        struct re_onig *onig = &prog->rx.onig;
        OnigSyntaxType* onigsyntax =
                (RE_EXTENDED == regopts->mode ? ONIG_SYNTAX_POSIX_EXTENDED :
                    (RE_PERL == regopts->mode ? ONIG_SYNTAX_PERL : ONIG_SYNTAX_RUBY));
        OnigErrorInfo einfo;
        int r, onigoptions = 0;

        if (! regopts->case_sense) {
            ONIG_OPTION_ON(onigoptions, ONIG_OPTION_IGNORECASE);
        }

        if (-2 == regopts->regexp_chars) {
            ONIG_OPTION_ON(onigoptions, ONIG_OPTION_FIND_LONGEST);
        }

     // ONIG_OPTION_FIND_NOT_EMPTY

        SRCH_TRACE(("search: Oniguruma-%s, options:0x%x\n",
            (RE_EXTENDED == regopts->mode ? "posix-extended" :
                (RE_PERL == regopts->mode ? "perl" : "ruby")), onigoptions))

                                                /* TODO - character encoding */
        if (ONIG_NORMAL != (r = onig_new(&onig->regex, (void *)pattern, (void *)(pattern + patlen),
                                    onigoptions, ONIG_ENCODING_ASCII, onigsyntax, &einfo))) {
            unsigned char err[ONIG_MAX_ERROR_MESSAGE_LEN] = {0};

            onig_error_code_to_str(err, r, &einfo);
            ewprintf("regexp: %s", err);
            return FALSE;
        }

        onig->region  = onig_region_new();
        prog->engine  = REGPROG_ONIG;
        prog->hasgrps =                         /* capture support required */
            (onig_number_of_captures(onig->regex) > 0 ? TRUE : FALSE);
        prog->exec    = on_exec;
        prog->capture = on_capture;
        prog->destroy = on_destroy;

    } else {
        /*
         *  RE_BRIEF/RE_CRISP, RE_UNIX
         */
        SRCH_TRACE(("search: Standard-%s\n",
            (RE_BRIEF == regopts->mode ? "brief" : "unix")))

        if (NULL == regexp_comp2(&prog->rx.regexp, regopts, pattern)) {
            return FALSE;
        }

        prog->engine  = REGPROG_REGEXP;
        prog->hasgrps = TRUE;                   /* TODO */
        prog->exec    = re_exec;
        prog->capture = re_capture;
        prog->destroy = re_destroy;
    }

    return TRUE;
}


static int
search_load(struct re_state *rs, int pati)
{
    __CUNUSED(rs)
    __CUNUSED(pati)
    /*TODO*/
    return -1;
}


static __CINLINE int
search_exec(struct regprog *prog, const char *buf, int buflen, int offset)
{
    if (prog->exec) {
	return (prog->exec)(prog, buf, buflen, offset);
    }
    return -1;
}


static const char *
search_capture(const struct regprog *prog, int group, const char **end)
{
    if (prog->capture) {
	return (prog->capture)(prog, group, end);
    }
    return NULL;
}


static __CINLINE void
search_end(struct re_state *rs)
{
    struct regprog *prog = &rs->prog;

    if (REGPROG_NONE != prog->engine) {
        regrpl_free(&rs->rpl);
        if (prog->destroy) {
            (prog->destroy)(prog);
        }
        if (prog->working) {
            assert(prog->hasgrps);
            chk_free(prog->working);
            prog->working = NULL;
        }
        prog->engine = REGPROG_NONE;
    }
}


static __CINLINE void
search_free(struct re_state *rs)
{
    search_end(rs);
    chk_free((void *)rs);
}


/*
 *  Internal regexp binding
 */

static int
re_exec(struct regprog *prog, const char *buf, int buflen, int offset)
{
    REGEXP *regexp = &prog->rx.regexp;

    assert(REGPROG_REGEXP == prog->engine);
    if (0 != regexp_exec(regexp, buf, buflen, offset)) {
        const int groupno = regexp->groupno + 1;
        int group;

        prog->buf     = buf;
        prog->bufend  = buf + buflen;
        prog->setpos  = regexp->setpos;
        prog->start   = regexp->start;
        prog->end     = regexp->end;
        prog->groupno = regexp->groupno;

#if defined(SRCH_ENABLED)
        SRCH_TRACE(("exec: Standard-%s (at:%d, captures:%d)\n",
            (RE_BRIEF == regexp->options.mode ? "brief" : "unix"), prog->start - buf, groupno))
        SRCH_TRACE(("  <%.*s>\n", prog->end - prog->start, prog->start))
        for (group = 0; group < groupno; ++group) {
            SRCH_TRACE(("  [%d] %2d - %2d <%.*s>\n", group, regexp->startp[group] - buf, regexp->endp[group] - buf,
                (regexp->endp[group] - regexp->startp[group]), regexp->startp[group]))
        }
#endif
        return 1;
    }
    return 0;
}


static const char *
re_capture(const struct regprog *prog, int group, const char **end)
{
    const REGEXP *regexp = &prog->rx.regexp;
    const char *start;

    assert(REGPROG_REGEXP == prog->engine);
    if (group < 0 || group > regexp->groupno ||
            NULL == (start = regexp->startp[group])) {
        return NULL;
    }
    if (end) *end = regexp->endp[group];
    return start;
}


static void
re_destroy(struct regprog *prog)
{
    REGEXP *regexp = &prog->rx.regexp;

    assert(REGPROG_REGEXP == prog->engine);
    regexp_free(regexp);
}


/*
 *  Oniguruma (regular expression library) binding.
 */

static int
on_exec(struct regprog *prog, const char *buf, int buflen, int offset)
{
    const char *bufend = buf + buflen;
    struct re_onig *onig = &prog->rx.onig;
    OnigRegion *region = onig->region;
    int r;

    assert(REGPROG_ONIG == prog->engine);
    if ((r = onig_search(onig->regex, (void *)buf, (void *)bufend,
                (void *)(buf + offset), (void *)bufend, region, ONIG_OPTION_NONE)) >= 0) {
        const int groupno = region->num_regs;

        prog->buf     = buf;
        prog->bufend  = bufend;
        prog->setpos  = NULL;                   /* not supported */
        prog->start   = buf + region->beg[0];
        prog->end     = buf + region->end[0];
        prog->groupno = groupno;

#if defined(SRCH_ENABLED)
        {   int rn;
            SRCH_TRACE(("exec: Oniguruma (at:%d, captures:%d)\n", r, groupno - 1))
            SRCH_TRACE(("  <%.*s>\n", prog->end - prog->start, prog->start))
            for (rn = 1; rn < groupno; ++rn) {
                SRCH_TRACE(("  [%d] %2d - %2d <%.*s>\n", rn, region->beg[rn], region->end[rn],
                    (region->end[rn] - region->beg[rn]), buf + region->beg[rn]))
            }
        }
#endif
        return 1;

    } else if (ONIG_MISMATCH != r) {
        unsigned char err[ONIG_MAX_ERROR_MESSAGE_LEN] = {0};

        onig_error_code_to_str(err, r);
        ewprintf("regexp: %s", err);
    }
    return 0;
}


static const char *
on_capture(const struct regprog *prog, int group, const char **end)
{
    const struct re_onig *onig = &prog->rx.onig;
    const OnigRegion *region = onig->region;
    const int groupno = region->num_regs;

    ++group;                                    /* 0... == 1 ... */

    assert(REGPROG_ONIG == prog->engine);
    if (group >= 1 && group < groupno) {
        const char *buf = prog->buf;
        int beg;

        if ((beg = region->beg[group]) >= 0) {
            if (end) *end = buf + region->end[group];
            return buf + beg;
        }
    }
    return NULL;
}


static void
on_destroy(struct regprog *prog)
{
    struct re_onig *onig = &prog->rx.onig;

    assert(REGPROG_ONIG == prog->engine);
    if (onig->regex) {
        if (onig->region) {
            onig_region_free(onig->region, 1);
            onig->region = NULL;
        }
        onig_free(onig->regex);
        onig->regex = NULL;
    }
}


#if defined(HAVE_LIBTRE)
/*
 *  TRE binding.
 */

static int
tre_exec(struct regprog *prog, const char *buf, int buflen, int offset)
{
    regex_t *regexp = &prog->rx.tre.regex;
    regmatch_t *regions = prog->rx.tre.regions;
    int r;

    buf += offset;
    buflen -= offset;

    assert(REGPROG_TRE == prog->engine);
    if (REG_OK == (r = tre_regnexec(regexp, buf, buflen, TRE_REGIONS, regions, (offset ? REG_NOTBOL : 0)))) {
        int groupno = 0;

        while (groupno < TRE_REGIONS && regions[groupno].rm_so != -1) {
            ++groupno;
        }

        prog->buf     = buf;
        prog->bufend  = buf + buflen;
        prog->setpos  = NULL;                   /* not supported */
        prog->start   = buf + regions[0].rm_so;
        prog->end     = buf + regions[0].rm_eo;
        prog->groupno = groupno;

#if defined(SRCH_ENABLED)
        {   int rn;
            SRCH_TRACE(("exec: TRE (at:%d, captures:%d)\n", prog->start - buf, groupno))
            SRCH_TRACE(("  <%.*s>\n", prog->end - prog->start, prog->start))
            for (rn = 0; rn < groupno; ++rn) {
                SRCH_TRACE(("  [%d] %2d - %2d <%.*s>\n", rn, regions[rn].rm_so, regions[rn].rm_eo,
                    (regions[rn].rm_eo - regions[rn].rm_so), buf + regions[rn].rm_so))
            }
        }
#endif
        return 1;

    } else if (REG_NOMATCH != r) {
        char err[128] = {0};

        tre_regerror(r, regexp, err, sizeof(err));
        ewprintf("regexp: %s", err);
    }
    return 0;
}


static const char *
tre_capture(const struct regprog *prog, int group, const char **end)
{
//  const regex_t *regexp = &prog->rx.tre.regex;
    const regmatch_t *regions = prog->rx.tre.regions;
    regoff_t so, eo;

    ++group;                                    /* 0... == 1 ... */

    assert(REGPROG_TRE == prog->engine);
    if (group < 1 || group > prog->groupno ||
            (so = regions[group].rm_so) < 0 || (eo = regions[group].rm_eo) < 0) {
        return NULL;
    }
    if (end) *end = prog->buf + eo;
    return prog->buf + so;
}


static void
tre_destroy(struct regprog *prog)
{
    regex_t *regexp = &prog->rx.tre.regex;

    assert(REGPROG_TRE == prog->engine);
    tre_regfree(regexp);
}

#endif  /*HAVE_LIBTRE*/


/*  Function:           search_string
 *      Work horse for string searches.
 *
 *  Parameters:
 *      rs - State buffer.
 *      flags - Search flags.
 *      stri - Argument index of the string to be searched.
 *      pati - Argument index of the search pattern.
 *
 *  Returns:
 *      Index of matched string, otherwise 0.
 */
static int
search_string(struct re_state *rs, int flags, int pati, int stri)
{
    const char *buf = get_xstr(stri);
    const int buflen = get_strlen(stri);
    int ret = 0;

    if (NULL == buf ||
            FALSE == search_start(rs, TRUE, flags, pati, 0)) {
        return -1;
    }

    if (buflen > 0) {
        struct regprog *prog = &rs->prog;

        if ((SF_CAPTURES & flags) && prog->hasgrps) {
            assert(NULL == prog->working);
            if (NULL != (prog->working = chk_snalloc(buf, buflen))) {
                buf = prog->working;            /* capture working buffer */
            }
        }

        if (search_exec(prog, buf, buflen, 0)) {
            if (prog->setpos) {
                rs->search_result2 = (prog->end - prog->start);
                prog->start = prog->setpos;     /* explicit position */
            }
            rs->search_result = (prog->end - prog->start);
            ret = (prog->start - buf) + 1;
        }
    }
    return ret;
}


/*  Function:           search_list
 *      Work horse for list searches.
 *
 *  Parameters:
 *      rs - State buffer.
 *      flags - Search flags.
 *      pati - Argument index of the search pattern.
 *      listi - Argument index of the list to be searched.
 *      starti - Starting atom index.
 *
 *  Returns:
 *      Index of matched string, otherwise -1.
 */
static int
search_list(struct re_state *rs, int flags, int pati, int listi, int starti)
{
    const LIST *lp = get_xlist(listi);
    int atomno = get_xinteger(starti, 0);

    if (NULL == lp ||
            FALSE == search_start(rs, TRUE, flags, pati, 0)) {
        return -2;
    }

    if (NULL != (lp = atom_nth(lp, atomno))) {
        const LIST *nextlp;

        for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
            const char *str;

            if (NULL != (str = atom_xstr(lp))) {
               if (search_exec(&rs->prog, str, strlen(str), 0)) {
                    return atomno;              /* 0 ... x */
                }
            }
            ++atomno;
        }
    }
    return -1;
}


static int
search_buf(struct re_state *rs, int dir, int flags, int pati)
{
    const int printmsg = (SF_QUIET & flags ? FALSE : TRUE);
    int result = -1;

    if (search_start(rs, dir, flags, pati, 0)) {
        if (printmsg) {
            infof("Searching...");
        }

        if ((result = buffer_search(rs, 2)) > 0) {
            if (printmsg) {
                infof("Search completed.");
            }
        } else {
            if (printmsg) {
                infof("Pattern not found.");
            }
        }
    }
    return result;
}


/*  Function:           buffer_search
 *      Search the current buffer for the next match.
 *
 *  Parameters:
 *      rs - State buffer.
 *      cursor - Cursor action on match, 0=none, 1=update and 2=record/update.
 *
 *  Returns:
 *      TRUE if a matched, otherwise FALSE.
 */
static int
buffer_search(struct re_state *rs, int cursor)
{
    struct regprog *prog = &rs->prog;
    const int fwd_search = rs->regopts.fwd_search;
    LINENO search_line, end_line, offset;
    const LINECHAR *line_text = NULL;
    int line_length = 0;

    search_line = rs->search_line;
    offset = rs->search_offset;

    if (fwd_search) {
        if ((end_line = rs->end_line) > curbp->b_numlines) {
            end_line = curbp->b_numlines;
        }
    } else {
        if ((end_line = rs->start_line) < 1) {
            end_line = 1;
        }
        if (search_line > curbp->b_numlines) {
            search_line = curbp->b_numlines;
        }
    }

    if ((fwd_search && search_line <= end_line) ||
            (!fwd_search && search_line >= end_line)) {

        const LINE_t *clp;

        clp = vm_lock_line(search_line);
        if (clp) {
            if (offset > 0) {
                if (ltext(clp)) {               /* clip to length */
                    if ((line_length = llength(clp)) < offset) {
                        offset = line_length;
                    }
                }
            }

            do {
                if (NULL == (line_text = ltext(clp))) {
                    offset = 0;
                    line_length = 0;
                    line_text = (LINECHAR *)"";

                } else {
                    line_length = llength(clp);

                    if (offset < 0) {           /* backwards starting offset */
                        offset = line_length;
                                                /* backwards limit */
                    } else if (! fwd_search && line_length > offset) {
                        line_length = offset;
                    }
                }

            /* SRCH_TRACE2(("\t[%4d/%4d] '%.*s'\n", search_line, offset, line_length, line_text)) */

                if (search_exec(prog, (const char *)line_text, line_length, offset)) {
                    goto success;
                }

                vm_unlock(search_line);
                if (fwd_search) {
                    offset = 0;
                    if (++search_line > end_line) {
                        break;
                    }
                    clp = lforw(clp);           /* TODO - vm_line_forward(search_line) */

                } else {
                    offset = -1;
                    if (--search_line < end_line) {
                        break;
                    }
                    clp = lback(clp);           /* TODO - vm_line_back(search_line) */
                }
            } while (clp);

            vm_unlock(search_line);
        }
    }

    rs->search_line = search_line;
    rs->search_offset = -1;
    SRCH_TRACE(("==> NOT-FOUND cursor=%d, line/dot=%d/%d \n", \
        cursor, rs->search_line, rs->search_offset))
    acc_assign_int(0);
    return FALSE;

success:;
    assert(prog->start >= (const char *)line_text);
    assert(prog->end   <= (const char *)(line_text + line_length));

    if (prog->setpos) {
        rs->search_result2 = prog->end - prog->start;
        prog->start = prog->setpos;             /* explicit position */
    }

    rs->search_result = prog->end - prog->start;
    offset = prog->start - (const char *)line_text;

    if (cursor) {                               /* update cursor */
        if (cursor > 1) {                       /* record cursor movement? */
            u_dot();
        }
        win_modify(WFMOVE);
        *cur_line = search_line;
        *cur_col = line_column(search_line, offset);
        win_modify(WFMOVE);
    }

    rs->search_line = search_line;
    rs->search_offset = offset;

    SRCH_TRACE(("==> FOUND cursor=%d, line/dot=%d/%d, line/col=%d/%d\n", cursor, \
        rs->search_line, rs->search_offset, *cur_line, *cur_col))

    vm_unlock(search_line);

    return TRUE;
}


/*  Function:           translate_string
 *      Translate a string and return value to accumulator; similar to awks sub and gsub.
 *
 *  Parameters:
 *      flags - Search flags.
 *      sstr - Source string.
 *      slen - Length of the source string.
 *      pati - Pattern index.
 *      repi - Replace index.
 *
 *  Returns:
 *      nothing
 */
static void
translate_string(int flags, const char *sstr, int slen, int pati, int repi)
{
    struct re_state rs = {0};
    struct regprog *prog = &rs.prog;
    struct regrpl *rpl = NULL;

    const char *rstr, *replacement = NULL;
    char *dstr = NULL;
    int dsiz, dlen = 0;
    int rlen;

    if (FALSE == search_start(&rs, TRUE, flags, pati, repi)) {
        acc_assign_str("", 1);
        goto end_of_function;
    }

    dsiz = slen + 16;
    dstr = chk_alloc(dsiz);

    while (1) {

        /* next match */
        if (slen <= 0 || 0 == search_exec(prog, sstr, slen, 0)) {
            break;
        }

        /* copy leading */
        if (prog->start > sstr) {
            rlen = prog->start - sstr;
            if (rlen > (dsiz - dlen)) {
                dsiz += rlen + 1;
                dstr = (char *) chk_realloc(dstr, dsiz);
            }
            memcpy(dstr + dlen, sstr, rlen);
            dlen += rlen;
        }

        /* apply replacement */
        if (NULL == (replacement = rs.replacement) &&
                NULL == (replacement = replacement_prompt(&rs))) {
            break;
        }

        if (NULL == rpl) {
            struct regopts *regopts = &rs.regopts;

            rpl = &rs.rpl;
            if (0 != regrpl_comp(rpl, replacement, regopts->mode, regopts->flags)) {
                acc_assign_str("", 1);
                goto end_of_function;
            }
            SRCH_TRACE(("regrpl_comp(%s)\n", replacement))
#if defined(SRCH_ENABLED)
            regrpl_print(rpl);
#endif
        }

        if (regrpl_exec(prog, rpl) < 0) {
            acc_assign_str("", 1);
            goto end_of_function;
        }

        rstr = rpl->buf;
        rlen = rpl->len;

        if (rlen > (dsiz - dlen)) {
            dsiz += rlen + 1;
            dstr = (char *) chk_realloc(dstr, dsiz);
        }
        memcpy(dstr + dlen, rstr, rlen);
        dlen += rlen;

        /* position after matched area */
        sstr = prog->end;
        slen = prog->bufend - sstr;
        if (flags < 0 || 0 == (SF_GLOBAL & flags)) {
            break;
        }
    }

    /* copy trailing */
    if (slen >= (dsiz - dlen)) {
        dsiz += slen + 1;
        dstr = (char *) chk_realloc(dstr, dsiz);
    }
    memcpy(dstr + dlen, sstr, slen);
    dlen += slen;
    dstr[dlen] = '\0';
    acc_assign_str(dstr, dlen + 1);

end_of_function:
    search_end(&rs);
    chk_free(dstr);
}


/*  Function:           translate_buf
 *      translate buffer work horse.
 *
 *  Parameters:
 *      dir - Direction.
 *      global - Global flag (1=yes,0=no,-1=prompt).
 *      flags - Search flags, othewise -1 if undefined.
 *      pati - Pattern argument index.
 *      repi - Replacement argument index.
 *
 *  Returns:
 *      nothing
 */
static void
translate_buf(int dir, int global, int flags, int pati, int repi)
{
    struct re_state rs = {0};
    const struct regopts *regopts = &rs.regopts;
    struct regrpl *rpl = NULL;

    const char *replacement;
    int printmsg = (SF_QUIET & flags ? FALSE : TRUE);
    int found = FALSE;
    int count = 0;
    int abrt = FALSE;

    if (rdonly() ||
            FALSE == search_start(&rs, dir, flags, pati, repi)) {
        return;
    }

    position_save();
    if (printmsg) {
        infof("Searching...");
    }
    u_dot();                                    /* undo point */

    while (rs.search_line > 0) {
        /*
         *  Next ...
         */
        if (! buffer_search(&rs, xlatprompt(global) ? 1 : 0)) {
            break;
        }
        found = TRUE;

        /*
         *  Figure out whether we need to keep prompting,
         *  or whether it is a global translate or whether we do it only once
         */
        SRCH_TRACE(("\tmatch (line:%d, dot:%d)\n", rs.search_line, rs.search_offset))
        if (xlatprompt(global)) {
            const int ch = trans_response(rs.search_result, count);

            switch (ch) {
            case 'A':           /* Abort */
                abrt = TRUE;
                goto end_of_function;

            case 'U':           /* Undo */
                rs.search_line = *cur_line;
                rs.search_offset = line_current_offset(LOFFSET_NORMAL);
                --count;
                continue;

            case 'E':           /* Entire */
                rs.search_line = 1;
                rs.search_offset = 1;
                continue;

            case 'G':           /* Global */
                global = GLOBAL_ALL;
                break;

            case 'O':           /* One */
                global = GLOBAL_ALL;
                break;

            case 'N':           /* No */
                rs.search_offset += (dir ? 1 : -1);
                if (regopts->fwd_search) {      /* 24/09/11 */
                    if (rs.end_match) {
                        rs.search_offset = 0;
                        ++rs.search_line;
                    }
                }
                continue;

            default:            /* Yes */
                assert('Y' == ch);
                break;
            }
        }

        if (xlatall(global)) {
            percentage(PERCENTAGE_LINES,
                (accuint_t) rs.search_line, (accuint_t) curbp->b_numlines, "Global", "translate");
        }

        if (NULL == (replacement = rs.replacement) &&
                NULL == (replacement = replacement_prompt(&rs))) {
            break;
        }

        if (NULL == rpl) {
            rpl = &rs.rpl;
            if (0 != regrpl_comp(rpl, replacement, regopts->mode, regopts->flags)) {
                break;
            }
            SRCH_TRACE(("regrpl_comp(%s)\n", replacement))
#if defined(SRCH_ENABLED)
            regrpl_print(rpl);
#endif
        }

        if (replace_buffer(&rs, xlatprompt(global)) < 0) {
            printmsg = FALSE;
            found = FALSE;
            break;
        }
                                                /* next line */
        if (regopts->line || rs.beg_match || rs.end_match) {
            if (regopts->fwd_search) {
                rs.search_offset = 0;
                ++rs.search_line;
            } else {
                rs.search_offset = -1;
                --rs.search_line;
            }
        }

        ++count;
        if (xlatonce(global)) {
            break;
        }
    }

end_of_function:
    search_end(&rs);
    if (abrt) {
        *cur_line = rs.search_line;
        *cur_col = line_column(rs.search_line, rs.search_offset);
    }
    position_restore(abrt ? 0 : 1);             /* abrt -- discard restore position */
    win_modify(WFHARD);

    if (printmsg) {
        if (found) {
            infof("Translation complete; %d occurrence%s changed.", count, count == 1 ? "" : "s");
        } else {
            infof("Pattern not found.");
        }
    }
    acc_assign_int((accint_t) count);           /* translation count */
}


static const char *
replacement_prompt(struct re_state *rs)
{
    if (ABORT == ereply("Replacement: ", rs->replbuf, sizeof(rs->replbuf))) {
        return NULL;
    }
    return rs->replbuf;
}


/*  Function:           replace_buffer
 *      Buffer text replacement.
 *
 *      Adjusts the current line/col for the next search.
 *
 *  Parameters:
 *      rs - Status buffer.
 *      interactive - *true* replacement is interactive,
 *          prompting the user.
 *
 *  Returns:
 *      Returns 0 if OK, -1 on error.
 */
static int
replace_buffer(struct re_state *rs, int interactive)
{
    const struct regopts *regopts = &rs->regopts;
    struct regrpl *rpl = &rs->rpl;

    int lines_inserted, rlen, edot = 0;
    const char *rstr;

    if (regrpl_exec(&rs->prog, rpl) < 0) {
        return -1;
    }
    rstr = rpl->buf;
    rlen = rpl->len;

    /* mark interactive translates as undo'able */
    if (interactive) {
        u_soft_start();
    }

    *cur_line = rs->search_line;
    *cur_col  = line_current_column(rs->search_offset);
    lines_inserted =
        lreplacedot(rstr, rlen, rs->search_result, rs->search_offset, &edot);

    /* position line cursor
     *  On a forward translate move paste inserted text backward,
     *  back up one character prior to the insert text.
     */
    if (regopts->fwd_search) {
        rs->end_line += lines_inserted;
        rs->search_line = *cur_line;
        rs->search_offset = edot + (0 == rs->search_result);
	
    } else if (--rs->search_offset < 0) {
        LINE_t *lp;

        if (--rs->search_line) {
            lp = vm_lock_line(rs->search_line);
            rs->search_offset = llength(lp);
            vm_unlock(rs->search_offset);
        }
    }
    return 0;
}


/*  Function:           trans_response
 *      Prompt the user for the next replacement action.
 *
 *  Parameters:
 *      size - Size of current hilited region.
 *      count - Current replacement count.
 *
 *  Returns:
 *      User action.
 */
static int
trans_response(int size, int count)
{
    const char *prompt =
            (count ? "\001Change [^Yes|^No|^Global|^One|^Undo|^Entire|^Abort,^Top|^Center]? " :
                "\001Change [^Yes|^No|^Global|^One|^Entire|^Abort,^Top|^Center]? ");
    char buffer[4];
    int ch;

    trans_hilite(size);
    while (1) {
        if (ABORT == ereply1(prompt, buffer, sizeof(buffer)) || !*buffer) {
            ch = 'A';
        } else {
            ch = buffer[0];
        }

        switch (ch) {
        case 'T': case 't':     /* Top - move current line to near top of window */
        case ALT_T:
        case CTRL_T:
            curwp->w_left_offset = 0;
            if (window_top_line(curwp, curwp->w_line)) {
                vtupdate();
            }
            break;

        case 'C': case 'c':     /* Center */
        case ALT_C:
        case CTRL_C:
            curwp->w_left_offset = 0;
            if (window_center_line(curwp, curwp->w_line)) {
                vtupdate();
            }
            break;

        case 'U': case 'u':     /* Undo */
        case ALT_U:
        case CTRL_U:
            if (count) {
                trans_unhilite(size);
                do_undo(1);
                vtupdate();
                return 'U';
            }
            break;

        case 'Y': case 'y':     /* Yes */
        case 'N': case 'n':     /* No */
        case 'G': case 'g':     /* Global */
        case 'O': case 'o':     /* One */
        case 'E': case 'e':     /* Entire */
        case 'A': case 'a':     /* Abort (alternative ESC) */
            trans_unhilite(size);
            return toupper(ch);
        }
    }
}


static void
trans_hilite(int size)
{
    const uint32_t buffer_flags = curbp->b_flag1;

  //hilite_create(curbp, HILITE_TRANSLATE, 0, *cur_line, *cur_col, *cur_line, *cur_col + (size - 1));
    BFSET(curbp, BF_NO_UNDO);
    anchor_drop(MK_NORMAL);
    if (size > 1) {
        move_next_char(size - 1, TRUE);
    }
    win_modify(WFEDIT);
    vtupdate();
    curbp->b_flag1 = buffer_flags;
}


static void
trans_unhilite(int size)
{
    const uint32_t buffer_flags = curbp->b_flag1;

  //hilite_destroy(curbp, HILITE_TRANSLATE);
    BFSET(curbp, BF_NO_UNDO);
    if (size > 1) {
        move_prev_char(size - 1);
    }
    anchor_raise();
    curbp->b_flag1 = buffer_flags;
}


#if defined(SRCH_ENABLED)
/*  Function:           search_trace
 *      Search specific diagnostics.
 *
 *  Parameters:
 *      fmt - Format specification (sprintf style).
 *      ... - Additional arguments.
 *
 *  Returns:
 *      Address of root mount-point.
 */
static void
search_trace(const char *fmt, ...)
{
    va_list ap;

//  #if defined(DB_REGEXP)
//      if (0 == (trace_flags() & DB_REGEXP)) {
//          return;
//      }
//  #endif
    va_start(ap, fmt);
    trace_logv(fmt, ap);
    va_end(ap);
}
#endif  /*SRCH_ENABLED*/
/*end*/
