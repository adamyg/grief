#include <edidentifier.h>
__CIDENT_RCSID(gr_crasc_c,"$Id: crasc.c,v 1.18 2020/04/23 12:35:50 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crasc.c,v 1.18 2020/04/23 12:35:50 cvsuser Exp $
 * ASCII/LISP backend code generator.
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
            fputs("\\a", x_afp);
            col++;
            break;
        case '\b':
            fputs("\\b", x_afp);
            col++;
            break;
        case '\t':
            fputs("\\t", x_afp);
            col++;
            break;
        case '\v':
            fputs("\\v", x_afp);
            col++;
            break;
        case '\r':
            fputs("\\r", x_afp);
            col++;
            break;
        case '\n':
            fputs("\\n", x_afp);
            col++;
            break;
        case '\f':
            fputs("\\f", x_afp);
            col++;
            break;
        default:
            fputc(*str, x_afp);
            break;
        }
        ++str; ++col;
    }
    need_space = TRUE;
    fputc('"', x_afp);
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
gena_sym(const char *str)
{
    consec_opens = 0;
    printstr(str);
    need_space = TRUE;
}


void
gena_reg(const char *name, int index)
{
    char buf[32];

    printstr(name);
    sprintf(buf, "#%" ACCINT_FMT, index);       // <symbol>#<index>
    printstr(buf);
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
    printstr("NULL");
    need_space = TRUE;
}


static void
printstr(const char *str)
{
    if (*str != '\n') {
        if (col > IDENT_LINELENGTH) {
            printnl();
        } else if (need_space) {
            fputc(' ', x_afp);
        }
    }
    need_space = FALSE;

    while (*str) {
        if ('\n' == *str) {
            printnl(); ++str;
            continue;
        }
        fputc(*str, x_afp);
        ++str; ++col;
    }

    if (xf_flush) {
        fflush(x_afp);
    }
}


static void
printnl(void)
{
    int i;

    fputc('\n', x_afp);
    col = list_level * IDENT_LEVEL;
    for (i = col; i-- > 0;) {
        fputc(' ', x_afp);
    }
}

/*end*/
