#ifndef GR_EDGETOPT_H_INCLUDED
#define GR_EDGETOPT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edgetopt_h,"$Id: edgetopt.h,v 1.19 2023/01/01 11:26:58 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edgetopt.h,v 1.19 2023/01/01 11:26:58 cvsuser Exp $
 * getopt() interface/implemenation.
 *
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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

#include <config.h>
#include <edtypes.h>

#if defined(HAVE_GETOPT_H) && \
            !defined(_WIN32) && !defined(WIN32) && !defined(NEEDS_GETOPT)
#include <getopt.h>

#elif defined(HAVE_UNISTD_H) && \
            !defined(_WIN32) && !defined(WIN32) && !defined(NEEDS_GETOPT)
#include <unistd.h>

#elif defined(__MINGW32__)

__CBEGIN_DECLS
extern int              optind;
extern int              opterr;
extern char *           optarg;
extern int              optopt;
extern int              optreset;
__CEND_DECLS


#elif defined(NEEDS_GETOPT) || defined(_WIN32) || defined(WIN32)
#if !defined(NEEDS_GETOPT)
#define NEEDS_GETOPT    /*getopt.c requirement*/
#endif

__CBEGIN_DECLS
extern int              getopt(int argc, char *const *argv, const char *opts);

extern int              optind;
extern int              opterr;
extern char *           optarg;
extern int              optopt;
extern int              optreset;
__CEND_DECLS

#endif /*NEEDS_GETOPT*/

__CBEGIN_DECLS

/*
 *  name:               Argument name.
 *
 *  has_arg:            Value specification.
 *
 *      no_argument         (or 0) if the option does not take an argument,
 *      required_argument   (or 1) if the option requires an argument,
 *      optional_argument   (or 2) if the option takes an optional argument.
 *
 *  flag & val:         Flag address and value.
 *
 *      If the field `flag' is not NULL, it points to a variable that is set to the value given
 *      in the field `val' when the option is found, but left unchanged if the option is not found.
 *
 */

struct bsd_option {
    const char *        name;
    int                 has_arg;
#ifndef no_argument
#define no_argument             0
#define required_argument       1
#define optional_argument       2
#endif
    int *               flag;
    int                 val;
};

extern void             bsd_optreset(void);
extern int              bsd_getopt(int nargc, char * const *nargv, const char *options);
extern int              bsd_getopt_long(int nargc, char * const *nargv, const char *options,
                                const struct bsd_option *long_options, int *idx);
extern int              bsd_getopt_long_only(int nargc, char * const *nargv, const char *options,
                                const struct bsd_option *long_options, int *idx);
__CEND_DECLS

#endif /*GR_EDGETOPT_H_INCLUDED*/
