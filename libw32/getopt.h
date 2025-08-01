#ifndef LIBW32_GETOPT_H_INCLUDED
#define LIBW32_GETOPT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_getopt_h,"$Id: getopt.h,v 1.15 2025/02/03 02:27:35 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win <getopt.h>
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#if defined(__MINGW32__)

#include_next <getopt.h>                        /* native getopt.h */

#else

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_VAR int          opterr;                 /* if error message should be printed */
LIBW32_VAR int          optind;                 /* index into parent argv vector */
LIBW32_VAR int          optopt;                 /* character checked for validity */
LIBW32_VAR int          optreset;               /* reset getopt */
LIBW32_VAR char         *optarg;

/*
 *  GNU like getopt_long() and BSD4.4 getsubopt()/optreset extensions.
 */
#define no_argument         0
#define required_argument   1
#define optional_argument   2

struct option {
        const char *name;                       /* name of long option */
        /*
         *  one of no_argument, required_argument, and optional_argument:
         *  whether option takes an argument
         */
        int has_arg;
        int *flag;                              /* if not NULL, set *flag to val when option found */
        int val;                                /* if flag not NULL, value to set *flag to; else return value */
};

LIBW32_API int          getopt(int nargc, char * const *nargv, const char *options);
LIBW32_API int          getopt_long(int argvc, char * const *argv, const char *options, const struct option *long_options, int *idx);
LIBW32_API int          getopt_long2(int argvc, char * const *argv, const char *options, const struct option *long_options, int *idx, char *buf, int buflen);

#if defined(LIBW32_LIBRARY)
extern void __w32_getopt_globals(void);
#endif

__END_DECLS

#endif  /*!__MINGW32__*/

#endif /*LIBW32_GETOPT_H_INCLUDED*/
