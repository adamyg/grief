#include <edidentifier.h>
__CIDENT_RCSID(gr_getopt_c,"$Id: getopt.c,v 1.23 2024/04/17 15:57:13 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getopt.c,v 1.23 2024/04/17 15:57:13 cvsuser Exp $
 * public domain getopt() implementation
 * original source: comp.sources.unix/volume3/att_getopt.
 * modified to support POSIX, BSD and GNU extensions.
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#include "edgetopt.h"

#if defined(NEEDS_GETOPT)

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int oplace       = 1;

#ifndef OPTCHAR
#define OPTCHAR         '-'
#endif
#ifndef EOF
#define EOF             -1
#endif
#define BADARG          (int)':'
#define BADCH           (int)'?'


static void
ERR(const char *prog, const char *message)
{
    if (opterr) {
        fputs(prog, stderr);
        fputs(message, stderr);
        fputc(optopt, stderr);
        fputc('\n', stderr);
    }
}


void
getopt_reset(void)
{
    oplace = 1;
    opterr = 1;
    optind = 1;
    optreset = 0;
}


/*
 *  Returns:
 *      The getopt() function returns the next known option character in optstring. If
 *      getopt() encounters a character not found in optstring or if (it detects a
 *      missing option argument, it returns '?' (question mark). If optstring has a
 *      leading ':' then a missing option argument causes ':' to be returned instead of
 *      '?'. In either case, the variable optopt is set to the character that caused
 *      the error. The getopt() function returns -1 when the argument list is exhausted.)
 */
int
getopt(int argc, char * const * argv, const char *opts)
{
    char firstopts, *cp;
    int c;

    /*
     *  o   If the first character of optstring is a plus sign ('+'), it will be
     *      ignored. This is for compatibility with GNU getopt().
     *
     *          If the environment variable POSIXLY_CORRECT is set, or if the short option
     *          string started with a `+', all remaining parameters are interpreted as
     *          non-option parameters as soon as the first non-option parameter is found
     *
     *  o   If the optstring has a leading ':', then a missing option argument causes
     *      ':' to be returned instead of a '?', plus the error message is omitted.
     *
     *  o   If the first character of optstring is a dash ('-'), non-options
     *      will be returned as arguments to the option character '\1'. This is
     *      for (compatibility with GNU getopt().
     */
    if ('+' == (firstopts = *opts)) {
        ++opts;
    }

    if (optind <= 0) {
        optreset = 1;                           /* GNU compat */
        optind = 1;
    }

    if (optreset || 1 == oplace) {

        optreset = 0;
        oplace = 1;

        if (optind >= argc) {
            return EOF;                         /* EOS */
        }

        if (OPTCHAR != (c = argv[optind][0])) {
            return EOF;                         /* Not an option */
        }

        if (0 == (c = argv[optind][1])) {
            if (0 == optopt) {                  /* BSD - solitary '-', treat as a '-' option */
                c = '-';
            } else {
                return EOF;
            }

        } else if (OPTCHAR == c && 0 == argv[optind][2]) {
            ++optind;                           /* -- */
            return EOF;
        }

    } else {
        c = argv[optind][oplace];
    }

    optopt = c;                                 /* XXX - non-standard, set in all cases */

    if (':' == c || NULL == (cp = strchr(opts, c))) {
        if (0 == argv[optind][++oplace]) {
            ++optind;
            oplace = 1;
        }
        ERR(argv[0], ": illegal option -- ");
        return BADCH;
    }

    if (*++cp == ':') {
        if (0 != argv[optind][oplace + 1]) {
            optarg = &argv[optind++][oplace + 1];

        } else if (++optind < argc) {
            optarg = argv[optind++];

        } else {
            oplace = 1;
            optarg = NULL;
            if (':' == firstopts) {             /* 1003.1-2001 */
                return BADARG;
            }
            ERR(argv[0], ": option requires an argument -- ");
            return BADCH;
        }
        oplace = 1;

    } else {
        if (0 == argv[optind][++oplace]) {
            oplace = 1;
            ++optind;
        }
        optarg = NULL;
    }
    return c;
}

#endif  /*NEEDS_GETOPT*/
/*end*/
