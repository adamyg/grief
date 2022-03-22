#include <edidentifier.h>
__CIDENT_RCSID(cr_sptree_c,"$Id: sptree.c,v 1.14 2022/03/21 15:17:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sptree.c,v 1.14 2022/03/21 15:17:20 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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


static void
spprinter(const SPBLK *node)
{
    if (node) {
        spprinter(SPLAY_LEFT(node, _sp_node));
        printf("%s\n", (const char *)node->key);
        spprinter(SPLAY_RIGHT(node, _sp_node));
    }
}


void
spprint(const SPTREE *tree)
{
    if (NULL == tree) {
        printf("NULL TREE\n");

    } else {
        char buffer[128];

        spprinter(SPLAY_ROOT(&tree->sp_root));
        printf("Statistics: %s\n", spstats(tree, buffer, sizeof(buffer)));
    }
}

