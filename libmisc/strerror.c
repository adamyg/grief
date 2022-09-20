#include <edidentifier.h>
__CIDENT_RCSID(gr_strerror_c,"$Id: strerror.c,v 1.11 2022/09/20 15:19:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strerror.c,v 1.11 2022/09/20 15:19:12 cvsuser Exp $
 * libstr - str_error utility functions.
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

#include <config.h>
#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#if defined(_WIN32) || defined(WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0601                  /* Window 7 */
#endif
#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif
#undef   u_char
#include <winsock2.h>
#include <windows.h>
#endif
#include <stdarg.h>

#include <libstr.h>
#include <unistd.h>

#if defined(_WIN32) || defined(WIN32)
static struct win32wsa {
    int value;
    const char *desc;
} x_win32wsa[] = {
#include "win32wsa.h"
    };
#endif  /*WIN32*/


#if defined(_WIN32) || defined(WIN32)
static int
win32wsacmp(const void *a, const void *b)
{
    const int aval = ((struct win32wsa *)a)->value;
    const int bval = ((struct win32wsa *)b)->value;

    if (aval < bval) {
        return -1;
    } else if (aval == bval) {
        return 0;
    } return 1;
}
#endif


const char *
str_error(const int xerrno)
{
    const char *ret;

    if (0 == xerrno) {
        return "Success";
    }

#if defined(_WIN32) || defined(WIN32)
    if (xerrno >= 80 && xerrno < 12000) {
        static volatile unsigned sorted  = 0;
        const struct win32wsa *wsa;
        struct win32wsa key;

        key.value = xerrno;
        if (0 == sorted) {
            if (1 == ++sorted) {
                qsort(x_win32wsa, sizeof(x_win32wsa)/sizeof(x_win32wsa[0]), sizeof(x_win32wsa[0]), win32wsacmp);
            }
        }
        if (NULL != (wsa = bsearch(&key, x_win32wsa,
                sizeof(x_win32wsa)/sizeof(x_win32wsa[0]), sizeof(struct win32wsa), win32wsacmp))) {
            return wsa->desc;
        }
    }
#endif

#if defined(HAVE_STRERROR)
#if defined(_WIN32) || defined(WIN32)
    if (xerrno < 0 || NULL == (ret = w32_strerror(xerrno))) {
        ret = "Unknown error";
    }
#else
    if (xerrno < 0 || NULL == (ret = strerror(xerrno))) {
        ret = "Unknown error";
    }
#endif
#else   /*old school*/
    ret = (xerrno < 0 || xerrno >= sys_nerr) ?
                "Unknown error" : sys_errlist[xerrno];
#endif
    return ret;
}
/*end*/
