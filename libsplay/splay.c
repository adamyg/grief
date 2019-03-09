#include <edidentifier.h>
__CIDENT_RCSID(cr_splay_c,"$Id: splay.c,v 1.13 2018/10/01 22:14:55 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: splay.c,v 1.13 2018/10/01 22:14:55 cvsuser Exp $
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


/*  Function:           splay
 *      Explicitly reorganize the tree. The tree is reorganized so that x is the root
 *      of the splay tree representing tree; 'q' must be in the tree, otherwise infinite
 *      looping will result; the left branch of the right subtree and the right branch
 *      of the left subtree are shortened in the process.
 *
 *  Parameters:
 *      q - element.
 *      tree - splay tree reference.
 *
 *  Returns:
 *      nothing
 */
void
splay(SPBLK * q, SPTREE * tree)
{
    _sproot *root = &tree->sp_root;

    SP_INCR(splay);
    SPI_INCR(tree->splays);
    libsplay_SPLAY(root, q);
}

