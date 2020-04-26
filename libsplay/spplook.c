#include <edidentifier.h>
__CIDENT_RCSID(cr_spplook_c,"$Id: spplook.c,v 1.18 2020/04/20 23:08:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spplook.c,v 1.18 2020/04/20 23:08:07 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
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
#include <assert.h>

typedef struct {
    const void *    key;
    int             keylen;
    int             ambiguous;
    SPBLK *         firstambiguous;
} Ambiguous_t;

static int          cmp_ambiguous(struct _sproot *root, struct _spblk *a, struct _spblk *b);


/*
 *  splookup() --
 *      Like splookup(), given key, find a node in a tree. Splays the found
 *      node to the root, but tell us if we found an ambiguity.
 */
SPBLK *
sp_partial_lookup(
    const void *key, SPTREE * tree, int *ambiguous, SPBLK ** first_sp)
{
    _sproot *root = &tree->sp_root;
    SPCOMPARE ocompare;
    Ambiguous_t ambig;
    SPBLK elm, *x;

    SP_INCR(splookup);
    SPI_INCR(tree->lookups);

    ambig.key = key;
    ambig.keylen = strlen(key);
    ambig.ambiguous = FALSE;
    ambig.firstambiguous = NULL;

    elm.key = (void *)key;
    elm.data = (void *)&ambig;

    ocompare = tree->sp_compare;
    tree->sp_compare = cmp_ambiguous;           /* local compare */
    x = SPLAY_FIND(libsplay, root, &elm);
    tree->sp_compare = ocompare;

    *ambiguous = ambig.ambiguous;
    if (first_sp)
        *first_sp = ambig.firstambiguous;
    return (x);
}


static int
cmp_ambiguous(
    struct _sproot *root, struct _spblk *a, struct _spblk *b)
{
    Ambiguous_t *ambig = (Ambiguous_t *)a->data;
    const char *ak = a->key, *bk = b->key;
    struct _spblk *broot = b;
    int ret;

    __CUNUSED(root)
    assert(ak == ambig->key);

    /*
     *  Compare keys
     */
    if ((ret = ak[0] - bk[0]) == 0) {           /* local optimisation */
        ret = strcmp(ak, bk);                   /* otherwise string compare */
    }

    /*
     *  Ambiguous tests
     */     
    if (! ambig->ambiguous) {
        int ambiguous = FALSE;

        if (ret) {                              /* no match, test current */
            if (strncmp(ak, bk, ambig->keylen) == 0) {
                ambiguous = TRUE;
            }

        } else {                                /* match, test children */
            if ((b = SPLAY_LEFT(broot, _sp_node)) != NULL &&
                        strncmp(ak, b->key, ambig->keylen) == 0) {
                ambiguous = TRUE;

            } else if ((b = SPLAY_RIGHT(broot, _sp_node)) != NULL &&
                        strncmp(ak, b->key, ambig->keylen) == 0) {
                ambiguous = TRUE;
            }
        }

        if (ambiguous) {
            ambig->ambiguous = TRUE;
            ambig->firstambiguous = (void *)b;
        }
    }
    return ret;
}

/*end*/