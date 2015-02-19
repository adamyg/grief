#include <edidentifier.h>
__CIDENT_RCSID(gr_crasc_c,"$Id: crasc.c,v 1.15 2014/10/22 02:33:28 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crasc.c,v 1.15 2014/10/22 02:33:28 ayoung Exp $
 * ASCII back end code generator.
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

#include "grunch.h"         /*local definitions*/

static int                  need_space, list_level, consec_opens, col;

static void                 printstr(const char *);
static void                 printnl(void);


void
init_ascii(void)
{
    col = 0;
    consec_opens = 0;
    need_space = FALSE;
    list_level = 0;
}


void
gena_list(void)
{
    if (0 == consec_opens++ /*|| 2 == list_level*/) {
        printstr("\n(");
    } else {
        printstr("(");
    }
    ++list_level;
}


void
gena_end_list(void)
{
    consec_opens = 0;
    need_space = FALSE;
    if (0 == --list_level) {
        printstr("\n)\n");
    } else {
        printstr(")");
        need_space = TRUE;
    }
}


void
gena_int(accint_t ival)
{
    char buf[32];

    consec_opens = 0;
    sprintf(buf, "%" ACCINT_FMT, ival);
    printstr(buf);
    need_space = TRUE;
}


void
gena_float(accfloat_t fval)
{
    char buf[64];

    consec_opens = 0;
    sprintf(buf, "%" ACCFLOAT_FMT, fval);
    printstr(buf);
    need_space = TRUE;
}


void
gena_string(const char *str)
{
    consec_opens = 0;
    printstr("\"");
    while (*str) {
        switch (*str) {
        case '\a':
            fputs("\\a", x_ofp);
            col++;
            break;
        case '\b':
            fputs("\\b", x_ofp);
            col++;
            break;
        case '\t':
            fputs("\\t", x_ofp);
            col++;
            break;
        case '\v':
            fputs("\\v", x_ofp);
            col++;
            break;
        case '\r':
            fputs("\\r", x_ofp);
            col++;
            break;
        case '\n':
            fputs("\\n", x_ofp);
            col++;
            break;
        case '\f':
            fputs("\\f", x_ofp);
            col++;
            break;
        default:
            fputc(*str, x_ofp);
            break;
        }
        ++str; ++col;
    }
    need_space = TRUE;
    fputc('"', x_ofp);
    ++col;
}


void
gena_id(const char *str)
{
    if (builtin_index(str) < 0) {
        consec_opens = 0;
    }
    printstr(str);
    need_space = TRUE;
}


void
gena_token(int val)
{
    gena_id(yymap(val));
}


void
gena_lit(const char *str)
{
    consec_opens = 0;
    printstr(str);
    need_space = TRUE;
}


void
gena_macro(void)
{
}


void
gena_finish(void)
{
}


void
gena_null(void)
{
    printstr(" NULL");
}


static void
printstr(const char *str)
{
    if (*str != '\n') {
        if (col > IDENT_LINELENGTH) {
            printnl();
        } else if (need_space) {
            fputc(' ', x_ofp);
        }
    }
    need_space = FALSE;

    while (*str) {
        if ('\n' == *str) {
            printnl(); ++str;
            continue;
        }
        fputc(*str, x_ofp);
        ++str; ++col;
    }

    if (xf_flush) {
        fflush(x_ofp);
    }
}


static void
printnl(void)
{
    int i;

    fputc('\n', x_ofp);
    col = list_level * IDENT_LEVEL;
    for (i = col; i-- > 0;) {
        fputc(' ', x_ofp);
    }
}
/*end*/
