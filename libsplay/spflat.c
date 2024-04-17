#include <edidentifier.h>
__CIDENT_RCSID(cr_spflat_c,"$Id: spflat.c,v 1.18 2024/04/17 15:57:14 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spflat.c,v 1.18 2024/04/17 15:57:14 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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
#include <assert.h>

typedef struct {
    int             index;
    int             count;
    SPBLK **        array;
} Flatten_t;


static void         flattennode(SPBLK *node, void *arg);


/*  Function:           spflatten
 *      The spflatten function takes a splay tree and builds an array of pointers to
 *      the SPBLKs, i.e. it flattens the tree so that the calling function can perform
 *      a simple iterative function over each node in the tree rather than using the
 *      convoluted callback mechanism of spwalk() and spapply(). The user is returned a
 *      chk_alloc()'ed array pointer, with the last element being NULL. The user must
 *      free this array.
 *
 *  Parameters:
 *      tree - splay tree reference.
 *
 *  Returns:
 *      Address of elements.
 */
SPBLK **
spflatten(SPTREE *tree)
{
    SPBLK **array;
    int count;

    count = spsize(tree);
    if ((array = chk_alloc((count + 1) * sizeof(SPBLK *))) != NULL) {
        Flatten_t f;

        f.index = 0;
        f.count = count;
        f.array = array;
        spwalk(tree, flattennode, &f);
        assert(f.index == count);
        array[f.index] = NULL;
    }
    return array;
}


static void
flattennode(SPBLK *node, void *arg)
{
    Flatten_t *f = (Flatten_t *)arg;

    assert(f->index < f->count);
    f->array[f->index++] = node;
}
