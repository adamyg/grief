#include <edidentifier.h>
__CIDENT_RCSID(cr_clear_c,"$Id: llclear.c,v 1.11 2015/02/19 00:17:06 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llclear.c,v 1.11 2015/02/19 00:17:06 ayoung Exp $
 * Linked list management module
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

#include "llheaders.h"

void
ll_clear(Head_p hp)
{
    List_p lp = ll_first(hp);

    LL_HMAGIC(hp);
    LIST_INCR(ll_clear);

    while (lp) {
        ll_unhook(lp);

        if (lp->ll_elem) {
            chk_free((void *) lp->ll_elem);
            lp->ll_elem = NULL;
        }

        lp->ll_magic = ~LL_LIST;
        vm_free(&ll_freelist, lp);
        LIST_DECR(lp_alloced);
        lp = ll_first(hp);
    }
}


void
ll_clear2(Head_p hp, ListNodeFree_t freenode)
{
    List_p lp = ll_first(hp);

    LL_HMAGIC(hp);
    LIST_INCR(ll_clear);

    while (lp) {
        ll_unhook(lp);

        if (lp->ll_elem) {
            if (freenode) (*freenode)((void *) lp->ll_elem);
            lp->ll_elem = NULL;
        }

        lp->ll_magic = ~LL_LIST;
        vm_free(&ll_freelist, lp);
        LIST_DECR(lp_alloced);
        lp = ll_first(hp);
    }
}

