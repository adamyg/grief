#include <edidentifier.h>
__CIDENT_RCSID(cr_spwalk_c,"$Id: spwalk.c,v 1.15 2022/03/21 15:17:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spwalk.c,v 1.15 2022/03/21 15:17:20 cvsuser Exp $
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

static void             spwalker(SPBLK *node, void (*func)(SPBLK *, void *), void *arg);


/*  Function:           spwalk
 *      The spwalk function in that each node of the splay tree is visits each node 
 *      within the splay tree and a user callback function is called.
 *  
 *  Parameters:
 *      tree - splay tree reference.
 *      func - callback function
 *      arg - user argument forwarded to callback.
 *
 *  Returns:
 *      nothing
 */
void
spwalk(SPTREE *tree, void (*func)(SPBLK *, void *), void *arg)
{
    _sproot *root = &tree->sp_root;

    spwalker(SPLAY_ROOT(root), func, arg);
}


static void
spwalker(SPBLK *node, void (*func)(SPBLK *, void *), void *arg)
{
    if (node) {
        spwalker(SPLAY_LEFT(node, _sp_node), func, arg);
        (*func)(node, arg);
        spwalker(SPLAY_RIGHT(node, _sp_node), func, arg);
    }
}


