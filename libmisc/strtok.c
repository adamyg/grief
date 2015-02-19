#include <edidentifier.h>
__CIDENT_RCSID(gr_strtok_c,"$Id: strtok.c,v 1.6 2015/02/19 00:17:14 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strtok.c,v 1.6 2015/02/19 00:17:14 ayoung Exp $
 * libstr - String token.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <libstr.h>

/*
 *  str_tok ---
 *      Alternative strtok version, which differs from the regular ANSI strtok in that an empty
 *      token can be returned, and consecutive delimiters are not ignored like ANSI does.
 *
 *  Example:
 *
 *      Parse String = ",,mike,gleason,-West Interactive,402-573-1000"
 *      Delimiters = ",-"
 *
 *      o ANSI strtok:
 *          1=[mike] length=4
 *          2=[gleason] length=7
 *          3=[West Interactive] length=16
 *          4=[402] length=3
 *          5=[573] length=3
 *          6=[1000] length=4
 *
 *      o str_tok:
 *          1=[] length=0
 *          2=[] length=0
 *          3=[mike] length=4
 *          4=[gleason] length=7
 *          5=[] length=0
 *          6=[West Interactive] length=16
 *          7=[402] length=3
 *          8=[573] length=3
 *          9=[1000] length=4
 *
 */
char *
str_tok(char *buf, const char *delims)
{
    static char *p = NULL;
    char *start, *end;

    if (buf != NULL) {
        p = buf;
    } else {
        if (p == NULL)
            return (NULL);                      /* No more tokens. */
    }

    for (start = p, end = p; ; end++) {
        if (*end == '\0') {
            p = NULL;                           /* last token. */
            break;
        }
        if (strchr(delims, (int) *end) != NULL) {
            *end++ = '\0';
            p = end;
            break;
        }
    }

    return (start);
}


/*
 *  str_ntok ---
 *      Bounds-safe version of strtok, where you also pass a pointer to the token to write
 *      into, and its size.
 *
 *  Example:
 *      Using the example above, with a char token[8], you get the following. Notice that
 *      the token is not overrun, and is always nul-terminated:
 *
 *      1=[] length=0
 *      2=[] length=0
 *      3=[mike] length=4
 *      4=[gleason] length=7
 *      5=[] length=0
 *      6=[West In] length=7
 *      7=[402] length=3
 *      8=[573] length=3
 *      9=[1000] length=4
 */
int
str_ntok(char *result, size_t retlen, char *buf, const char *delims)
{
    static char *p = NULL;
    char *end, *lim, *dst;
    int len;

    dst = result;
    lim = dst + retlen - 1;                     /* NUL. */

    if (buf != NULL) {
        p = buf;
    } else {
        if (p == NULL) {
            *dst = '\0';
            return (-1);                        /* No more tokens. */
        }
    }

    for (end = p; ; end++) {
        if (*end == '\0') {
            p = NULL;                           /* last token. */
            break;
        }
        if (strchr(delims, (int) *end) != NULL) {
            ++end;
            p = end;
            break;
        }
        if (dst < lim)                          /* don't overrun token size. */
            *dst++ = *end;
    }
    *dst = '\0';
    len = (int) (dst - result);                 /* return length of token. */

#if (STRN_ZERO_PAD == 1)
    /* Pad with zeros. */
    for (++dst; dst <= lim; )
        *dst++ = 0;
#endif  /* STRN_ZERO_PAD */

    return (len);
}


#ifdef LOCAL_MAIN
#include <stdio.h>

void
main(int argc, char **argv)
{
    char buf[256];
    int i;
    char *t;
    char token[8];
    int tokenLen;

    if (argc < 3) {
        fprintf(stderr, "Usage: test \"buffer,with,delims\" <delimiters>\n");
        exit(1);
    }
    strcpy(buf, argv[1]);
    i = 1;
    t = strtok(buf, argv[2]);
    if (t == NULL)
        exit(0);
    do {
        printf("strtok %d=[%s] length=%d\n", i, t, (int) strlen(t));
        t = strtok(NULL, argv[2]);
        ++i;
    } while (t != NULL);

    printf("------------------------------------------------\n");
    strcpy(buf, argv[1]);
    i = 1;
    t = str_tok(buf, argv[2]);
    if (t == NULL)
        exit(0);
    do {
        printf("Strtok %d=[%s] length=%d\n", i, t, (int) strlen(t));
        t = Strtok(NULL, argv[2]);
        ++i;
    } while (t != NULL);

    printf("------------------------------------------------\n");
    strcpy(buf, argv[1]);
    i = 1;
    tokenLen = str_ntok(token, sizeof(token), buf, argv[2]);
    if (tokenLen < 0)
        exit(0);
    do {
        printf("Strntok %d=[%s] length=%d\n", i, token, tokenLen);
        tokenLen = str_ntok(token, sizeof(token), NULL, argv[2]);
        ++i;
    } while (tokenLen >= 0);
    exit(0);
}
#endif
/*end*/
