#include <edidentifier.h>
__CIDENT_RCSID(cr_sputil_c,"$Id: sputil.c,v 1.13 2024/04/17 15:57:15 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sputil.c,v 1.13 2024/04/17 15:57:15 cvsuser Exp $
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

static int          compare(struct _sproot *root, struct _spblk *a, struct _spblk *b);

SPLAY_GENERATE(libsplay, _sproot, _spblk, _sp_node, compare)

static int
compare(struct _sproot *root, struct _spblk *a, struct _spblk *b)
{
    SPTREE * tree = (SPTREE *)root;
    const char *ak = a->key, *bk = b->key;
    int ret;

    if (tree->sp_compare)
        return (tree->sp_compare)(root, a, b);

    if ((ret = ak[0] - bk[0]) == 0)             /* local optimisation */
        ret = strcmp(ak, bk);                   /* otherwise string compare */

    return ret;
}



