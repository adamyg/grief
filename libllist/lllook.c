#include <edidentifier.h>
__CIDENT_RCSID(cr_look_c,"$Id: lllook.c,v 1.9 2015/02/19 00:17:08 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: lllook.c,v 1.9 2015/02/19 00:17:08 ayoung Exp $
 * Linked list management module.
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

List_p
ll_lookup(Head_p hp, void * str, int (*func)(void const *, void const *))
{
    List_p lp;

    LL_HMAGIC(hp);
    LIST_INCR(ll_lookup);

    for (lp = TAILQ_FIRST(&hp->lh_queue); lp; lp = TAILQ_NEXT(lp, ll_node)) {
        assert(lp->ll_owner == hp);
        assert(lp->ll_elem != NULL);

        if ((*func)(str, lp->ll_elem) == 0) {
            return lp;
        }
    }
    return (NULL);
}

