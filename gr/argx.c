#include <edidentifier.h>
__CIDENT_RCSID(gr_argx_c,"$Id: argx.c,v 1.7 2014/10/22 02:32:52 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: argx.c,v 1.7 2014/10/22 02:32:52 ayoung Exp $
 * Argument vector suppport.
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

#include <editor.h>
#include <assert.h>

#include "arg.h"


int
argx_create(const char **argv, char **argx, int *args)
{
    *argx = NULL;
    *args = 0;

    if (argv) {
        int argc, t_args = 0;

        for (argc = 0; argv[argc]; ++argc) {    /* storage requirement */
            t_args = (int)strlen(argv[argc]) + 1;
        }

        if (t_args) {
            char *t_argx;
                                                /* allocate, round?? */
            if (NULL == (t_argx = (char *)malloc(t_args))) {
                return ENOMEM;
            }
            *argx = t_argx;
            *args = t_args;
            for (argc = 0; argv[argc]; ++argc) {
                const char *val = argv[argc];
                const int len = (int)strlen(val) + 1;

                (void) memcpy(t_argx, val, len);
                t_argx += len;
                t_args -= len;
            }
            assert(0 == t_args);
        }
    }
    return 0;
}


int
argx_create_sep(const char **argv, int sep, char **argx, int *args)
{
    return -1;
}


int
argx_destroy(char **argx, int *args)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    if (argx && args > 0) {
        free((void *)argx);
    }
    return 0;
}


int
argx_add(char **argx, int *args, const char *str)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    return -1;
}


int
argx_add_sep(char **argx, int *args, const char *str, int sep)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    return -1;
}


int
argx_append(char **argx, int *args, const char *buf, int buflen)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    return -1;
}


int
argx_insert(char **argx, int *args, const char *cursor, const char *str)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    return -1;
}


int
argx_replace(char **argx, int *args, const char *cursor, const char *str)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    return 0;
}


int
argx_remove(char **argx, int *args, const char *cursor)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));

    if (argx && args > 0) {
        assert(NULL == cursor || ((char *)cursor >= *argx && (char *)cursor < (*argx) + *args));

        if (cursor) {
            const int len = (int)strlen(cursor) + 1;
            int t_args = *args;

            if ((t_args - len) <= 0) {
                free((void *) *argx);
                *argx = NULL;
                *args = 0;

            } else {
                char *t_argx = *argx;
                                                // XXX
                (void) memmove((char *)cursor, cursor + len, t_argx + t_args - cursor);
                if (NULL == (t_argx = (char *)realloc(t_argx, t_args))) {
                    return ENOMEM;
                }
                *argx = t_argx;
                *args = t_args;
            }
        }
    }
}


const char *
argx_next(const char *argx, int args, const char *cursor)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));

    if (argx && args > 0) {
        assert(NULL == cursor || (cursor >= argx && cursor < argx + args));

        if (cursor) {                           /* previous location */
            if (cursor >= argx) {
                const char *end = argx + args;

                while (cursor < end && *cursor) {
                    ++cursor;
                }
                if (++cursor < end) {
                    return cursor;
                }
            }
        } else {                                /* otherwise first element */
            return argx;
        }
    }
    return NULL;
}


int
argx_count(const char *argx, int args)
{
    int count = 0;

    assert((argx && args > 0) || (NULL == argx && 0 == args));
    if (argx && --args >= 0) {
        const char *cursor = argx + args;

        while (cursor >= argx) {
            if (0 == *cursor) ++count;
            --cursor, --args;
        }
        assert(-1 == args);
    }
    return count;
}


int
argx_dump(const char *argx, int args)
{
    int idx = 0;

    if (args > 0) {
        const char *cursor = argx, *end = cursor + args;

        while (cursor < end) {
            const size_t len = strlen(cursor) + 1;

            printf("%4u] <%s> (%d)\n", idx, cursor, len);
            cursor += len + 1;
            ++idx;
        }
    }
    return idx;
}


int
argx_extract(const char *argx, const int args, char **argv, int argc)
{
    return -1;
}


char *
argx_stringify(char *argx, int args, int sep)
{
    assert((argx && args > 0) || (NULL == argx && 0 == args));
    assert(sep);

    if (argx && --args >= 0 && sep) {
        char *cursor = argx + --args;           /* dont replace terminating NUL */

        while (cursor >= argx) {
            if (0 == *cursor) *cursor = sep;
            --cursor, --args;
        }
        assert(-1 == args);
    }
    return argx;
}


#if defined(LOCAL_MAIN)
int
main(void)
{
    return 0;
}
#endif  /*LOCAL_MAIN*/
