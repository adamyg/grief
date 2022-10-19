#include <edidentifier.h>
__CIDENT_RCSID(gr_edbt_main_c,"$Id: edbt_main.c,v 1.6 2022/09/20 15:19:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edbt_main.c,v 1.6 2022/09/20 15:19:11 cvsuser Exp $
 * backtrace test application
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <edstacktrace.h>

static int
function4(int a, int b, char *c)
{
    FILE *out = fopen("stackdump.out", "w");
    edbt_stackdump(out, 1);
    fclose(out);
    return 0;
}


static int
function3(int a, int b, char *c)
{
    char x[100];
    return function4(a, b, x);
}


static int
function2(int a, int b, char *c)
{
    char y[100];
    return function3(a, b, y);
}


static int
function1(int a, int b, char *c)
{
    char z[100];
    return function2(a, b, z);
}


int
main(int argc, const char *argv[])
{
    return function1(1, 2, NULL);
}

/*end*/
