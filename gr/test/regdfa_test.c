#include <edidentifier.h>
__CIDENT_RCSID(gr_regdfa_test_c,"$Id: regdfa_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regdfa_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $
 *
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "regdfa.h"

static const char * x_patterns[] = {
    "^#.*$",
    "[ \t;]#.*$",
    "([\\$%&@\\*]|\\$#)[A-Za-z_0-9]+",
    "\\$([_\\./,\"\\\\#\\*\\?\\]\\[;!@:\\$<>\\(\\)%=\\-~\\^\\|&`'\\+]|\\^[A-Z])",
    "[A-Za-z_][A-Za-z0-9_]*",
    "[0-9]+(\\.[0-9]+)?([Ee][\\-\\+]?[0-9]+)?",
    "0x[0-9A-Fa-f]+",
    "\"(\\\\\"|[^\"])*\"", 
    "\'(\\\\\'|[^\'])*\'", 
    "[\\(\\[{<>}\\]\\),\\.:;?]",
    "[-+!%&\\*/=<>|~^]",
    "[A-Za-z0-9]+",
    };


static char x_code[] = {
    "BEGIN {\n"
    "    my ($var) = $ENV{'PERLINC'};\n"
    "\n"
    "    if (defined($var) && -d $var) {            # import PERLINC\n"
    "        my ($quoted_var) = quotemeta($var);\n"
    "        push(@INC, $var)\n"
    "            if (! grep /^$quoted_var$/, @INC);\n"
    "\n"
    "    } elsif ($^O eq 'MSWin32') {               # ActivePerl (defaults)\n"
    "        if (! grep /\\/perl\\/lib/, @INC) {\n"
    "            push(@INC, '   c:\\'/p     erl/lib')   if (-d 'c:/perl/lib');\n"
    "            push(@INC, \"/p\\\"erl /lib\")    if (-d \"/perl/lib\");\n"
    "        }\n"
    "#comment\n"
    "\n"
    "use strict;\n"
    "use warnings 'all';\n"
    "\n"
    "my $o_output = 'crntypes.h';\n"
    "my $o_debug  = 0;\n"
    "my $o_value  = 0;\n"
    "\n"
    };


static void
test(void)
{
    struct regdfa *regdfa;
    const char *data = x_code, *dataend = x_code + sizeof(x_code);
    const char *nm;
    int sol, idx;

    printf("COMP:\n");
    if (NULL == (regdfa = regdfa_create(x_patterns, sizeof(x_patterns)/sizeof(x_patterns[0]), 0))) {
        return;
    }

    printf("MATCH:\n");
    nm = NULL;
    for (sol = 1; *data;) {
        const char *start = data, *end;

        if ((idx = regdfa_match(regdfa, data, dataend, sol, &start, &end)) >= 0) {
            const char *nl = strchr(start, '\n');
            int len = (end - data) + 1;

            if (nl) {
                int t_len = (nl - data) + 1;
                if (t_len <= len) {
                    len = t_len - 1;
                } else {
                    nl = 0;
                }
            }

            if (nm) {
                printf("0-%.*s", (unsigned)(data - nm), nm);
                nm = NULL;
            }
            printf("%d-%.*s", idx+1, len, start);

            data = start + len;
            if (nl) {
                printf("\n");
                ++data;
                sol = 1;
            } else {
                sol = 0;
            }

        } else {
            sol = (*data == '\n' ? 1 : 0);

            if (sol) {
                if (nm) {
                    printf("0-%.*s\n", (unsigned)(data - nm), nm);
                } else {
                    printf("0-\n");
                }
                nm = NULL;
            } else if (NULL == nm) {
                nm = data;
            }
            ++data;
        }
    }

    if (nm) {
        printf("0-%.*s\n", (unsigned)(nm - data), nm);
        nm = NULL;
    }

    regdfa_destroy(regdfa);
}


int
main(int argc, char **argv)
{
    test();
    return 0;
}


void *
chk_malloc(size_t size)
{
    assert(size);
    return malloc(size);
}


void *
chk_calloc(size_t size)
{
    assert(size);
    return calloc(size, 1);
}


size_t
chk_shrink(void *p, size_t size)
{
    assert(p);
    assert(size);
    return 0;
}


void *
chk_realloc(void *p, size_t size)
{
    assert(size);
    return realloc(p, size);
}


void
chk_free(void *p)
{
    assert(p);
    free(p);
}


void
ewprintf(const char *fmt, ...)
{
    va_list ap;

    fflush(stdout);
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    va_end(ap);
}


void
debug(const char *fmt, ...)
{
    va_list ap;

    fflush(stdout);
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}

