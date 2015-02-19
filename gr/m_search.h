#ifndef GR_M_SEARCH_H_INCLUDED
#define GR_M_SEARCH_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_search_h,"$Id: m_search.h,v 1.15 2014/10/27 23:27:55 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_search.h,v 1.15 2014/10/27 23:27:55 ayoung Exp $
 * Search primitives.
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

#include <edsym.h>

/*--export--defines--*/
/*
 *  re_escape expression modes.
 */
#define RE_BRIEF                0
#define RE_UNIX                 1
#define RE_EXTENDED             2
#define RE_PERL                 3
#define RE_RUBY                 4
/*--end--*/
#define RE_TRE                  5

/*
 *  re_escape capture modes.
 */
/*--export--defines--*/
#define RE_PERLVARS             2
#define RE_AWKVARS              1
/*--end--*/

/*--export--defines--*/
/*
 *  Search and translate flags
 */
#define SF_BACKWARDS            0x00001         /* Search in backwards direction. */
#define SF_IGNORE_CASE          0x00002         /* Ignore/fold case. */
#define SF_ICASE                SF_IGNORE_CASE
#define SF_BLOCK                0x00004         /* Restrict search to current block. */
#define SF_LINE                 0x00008         /* Line mode */

#define SF_LENGTH               0x00010         /* Return length of match. */
#define SF_MAXIMAL              0x00020         /* Maximal/greedy search mode (Non-BRIEF default). */
#define SF_MINIMAL              0x40000         /* Minimal/non-greedy search mode (BRIEF default). */
#define SF_CAPTURES             0x00040         /* Capture elements are retained. */
#define SF_QUIET                0x00080         /* Don't display progress messages. */

#define SF_GLOBAL               0x00100         /* Global translate. */
#define SF_PROMPT               0x00200         /* Prompt for translate changes. */
#define SF_AWK                  0x00400         /* awk(1) style capture references. */
#define SF_PERLVARS             0x00800         /* perl(1) style capture references. */

#define SF_BRIEF                0x00000         /* BRIEF expressions. */
#define SF_CRISP                SF_BRIEF
#define SF_NOT_REGEXP           0x01000         /* Treat as plain test, not as a regular expression. */
#define SF_UNIX                 0x02000         /* Unix regular expressions. */
#define SF_EXTENDED             0x04000         /* POSIX extended syntax. */
#define SF_PERL                 0x08000         /* PERL syntax. */
#define SF_RUBY                 0x10000         /* Ruby syntax. */
#define SF_TRE                  0x20000         /* TRE syntax. */
/*--end--*/
#define SF_TREFUSSY             0x80000         /* TRE fuzzy */

/*--export--defines--*/
/*
 *  re_result, special captures
 */
#define CAPTURE_BEFORE          -110
#define CAPTURE_AFTER           -111
#define CAPTURE_ARENA           -112
#define CAPTURE_LAST            -113
/*--end--*/

__CBEGIN_DECLS

extern void                 do_quote_regexp(void);

extern void                 do_search_case(void);
extern void                 do_re_syntax(void);

extern void                 do_search_buf(int dir);
extern void                 do_search_list(void);
extern void                 do_search_string(void);
extern void                 do_translate(void);

extern void                 do_re_comp(void);
extern void                 do_re_search(void);
extern void                 do_re_delete(void);
extern void                 do_re_result(void);
extern void                 do_re_translate(void);

__CEND_DECLS

#endif /*GR_M_SEARCH_H_INCLUDED*/
