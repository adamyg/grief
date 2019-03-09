#include <edidentifier.h>
__CIDENT_RCSID(cr_spinit_c,"$Id: spinit.c,v 1.15 2018/10/01 22:14:55 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spinit.c,v 1.15 2018/10/01 22:14:55 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
 *
 * The basic splay tree algorithms were originally presented in:
 * Self Adjusting Binary Trees,
 *   by D. D. Sleator and R. E. Tarjan,
 *   Proc. ACM SIGACT Symposium on Theory
 *   of Computing (Boston, Apr 1983) 235-245.
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#if defined(SPLAY_GLOBAL_STATS) && (SPLAY_GLOBAL_STATS)
SPSTATS                 __spstats;
#endif
static vmpool_t         hd_splays;


/*  
 *  spinit() -- initialize an empty splay tree
 */
SPTREE *
spinit(void)
{
    return spinit2(NULL);
}


/*  
 *  spinit2() -- initialize an empty splay tree
 */
SPTREE *
spinit2(SPCOMPARE compare)
{
    static SPTREE null_sptree = {0};
    SPTREE *tree = NULL;

    if ((tree = vm_alloc(&hd_splays, sizeof(SPTREE))) != NULL) {
        *tree = null_sptree;
        SP_INCR(spinit);
        SP_INCR(trees_alloced);
        SPLAY_INIT(&tree->sp_root);
        tree->sp_compare = compare;
    }
    return tree;
}


/*  
 *  Free the memory associated with the root of a tree.
 */
void
spfree(SPTREE *tree)
{
    SP_DECR(trees_alloced);
    SP_INCR(spfree);
    vm_free(&hd_splays, tree);
}

