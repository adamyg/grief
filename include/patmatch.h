#ifndef GR_PATMATCH_H_INCLUDED
#define GR_PATMATCH_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_patmatch_h,"$Id: patmatch.h,v 1.6 2018/10/04 01:28:00 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: patmatch.h,v 1.6 2018/10/04 01:28:00 cvsuser Exp $
 * Simple pattern matching.
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edsym.h>

__CBEGIN_DECLS

enum {
/*--export--enum--*/
/*
 *  file_match flags
 */
    MATCH_TRAILINGWHITE = 0x01,
        /* If set, ignores trailing whitespace **/

    MATCH_NOCASE        = 0x02,
        /*  If set, ignore cases during character comparisons */

    MATCH_AUTOCASE      = 0x04,
        /*  If set, sets NOCASE based on target operating system */

    MATCH_NOESCAPE      = 0x08,
        /*  If not set, a backslash character (\) in pattern followed by
         *  any other character will match that second character in string.
         *  In particular, "\\" will match a backslash in string.
         *
         *  If set, a backslash character will be treated as an ordinary
         *  character.
        **/

    MATCH_PATHNAME      = 0x10,
        /*  If set, a slash (/) character in string will be explicitly
         *  matched by a slash in pattern; it will not be matched by either
         *  the asterisk (*) or question-mark (?) special characters, nor by
         *  a bracket ([]) expression
         *
         *  If not set, the slash character is treated as an ordinary
         *  character.
        **/

    MATCH_PERIODA       = 0x20,         /* asterisk (*) */
    MATCH_PERIODQ       = 0x40,         /* question mark (?) */
    MATCH_PERIODB       = 0x80,         /* bracket ([]) */
    MATCH_PERIOD        = 0xE0,         /* MATCH_PERIODA|MATCH_PERIODQ|MATCH_PERIODB */
        /*  If set, a leading period in string will match a period in
         *  pattern; where the location of "leading" is indicated by the
         *  value of MATCH_PATHNAME as follows:
         *
         *     o is set, a period is "leading" if it is the first character
         *       in string or if it immediately follows a slash.
         *
         *     o is not set, a period is "leading" only if it is the first
         *       character of string.
         *
         *  If not set, no special restrictions are placed on matching a
         *  period.
         *
         *  The three MATCH_PERIODA, MATCH_PERIODQ and MATCH_PERIODB allow
         *  selective control whether the asterisk (*) or question mark (?)
         *  special characters, nor by a bracket ([]) expression have affect.
         *
        **/
/*--end--*/
};

typedef void (*matcherrfn_t)(const char *fmt, ...);

extern int              patmatch(const char *file, const char *str, int flags);
extern int              patmatchx(const char *file, const char *str, int flags, matcherrfn_t errfn);

__CEND_DECLS

#endif /*GR_PATMATCH_H_INCLUDED*/
