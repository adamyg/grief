#include <edidentifier.h>
__CIDENT_RCSID(cr_sptest_c,"$Id: sptest.c,v 1.11 2022/12/04 15:23:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sptest.c,v 1.11 2022/12/04 15:23:27 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
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

#include "spheaders.h"

static SPTREE *         tree;
static char             lbuf[256];

static void
New(char *str)
{
    SPBLK *q = chk_alloc(sizeof(SPBLK) + 32);
    char *s = (char *) (q + 1);

    strcpy(s, str);
    q->key = s;

    spenq(q, tree);
}


int
main(int argc, char **argv)
{
    int carg;
    SPBLK *q;
    int lookup = FALSE;

    tree = spinit();

    for (carg = 0; ++carg < argc;) {
        char *argp = argv[carg];

        if (*argp == '-') {
            switch (*++argp) {
            case 'l':
                lookup = TRUE;
                break;
            }

        } else if (argp[0] >= '0' && argp[0] <= '9') {
            int i = atoi(argp);
        
            while (i >= 0) {
                char buf[20];
                sprintf(buf, "%05d", i--);
                New(buf);
            }
        } else {
            New(argp);
        }
    }

    for (;;) {
        ptree(tree);
        printf(lookup ? "Lookup ? " : "Delete ? ");
        fflush(stdout);
        fgets(lbuf, sizeof lbuf - 1, stdin);
        
        if (lbuf[strlen(lbuf) - 1] == '\n')
            lbuf[strlen(lbuf) - 1] = NULL;
        
        if (lbuf[0] == NULL)
            exit(1);
        q = splookup(lbuf, tree);
        if (q == NULL) {
            printf("Entry not found.\n");
            continue;
        }

        if (!lookup) {
            printf("Before deletion\n");
            ptree(tree);
            fflush(stdout);
            spdeq(q, tree);
        }
    }
}

