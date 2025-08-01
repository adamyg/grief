#include <edidentifier.h>
__CIDENT_RCSID(gr_getopt_common_c,"$Id: getopt_common.c,v 1.15 2025/01/13 16:06:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getopt_common.c,v 1.15 2025/01/13 16:06:38 cvsuser Exp $
 * common globals getopt/bsd_getopt.
 *
 *
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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <stdio.h>

#if defined(NEEDS_GETOPT)

int   opterr   = 1;                             /* if error message should be printed */
int   optind   = 1;                             /* index into parent argv vector */
int   optopt   = '?';                           /* character checked for validity */
char *optarg   = NULL;                          /* argument associated with option */
int   optreset = 0;                             /* reset getopt */

#endif /*NEEDS_GETOPT*/

extern void __getopt_common_dummy(void);
void
__getopt_common_dummy() 
{
}

/*end*/
