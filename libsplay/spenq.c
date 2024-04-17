#include <edidentifier.h>
__CIDENT_RCSID(cr_spenq_c,"$Id: spenq.c,v 1.16 2024/04/17 15:57:14 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spenq.c,v 1.16 2024/04/17 15:57:14 cvsuser Exp $
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

/*  Function:           spenq
 *      Enqueue a new entry to the tree
 *  
 *  Parameters:
 *      q - Element.
 *      tree - splay tree reference.
 *
 *  Returns:
 *      nothing
 */
void
spenq(SPBLK *q, SPTREE *tree)
{
    _sproot *root = &tree->sp_root;

    SP_INCR(spenq);
    SPI_INCR(tree->enqs);

    if (SPLAY_INSERT(libsplay, root, q)) {
        fprintf(stderr, "\r\nDuplicate entry in splay tree -- aborting\r\n");
        fflush(stderr);
        abort();
    }

    /*  Add a bit of randomness to the way the splay tree is built to
     *  avoid catastrophic performance when inserting alphabetically
     *  into a tree.
     */
    ++tree->sp_count;
    if ((tree->sp_count & 0x7) == 0) {
        libsplay_SPLAY(root, q);
    }
}

