#include <edidentifier.h>
__CIDENT_RCSID(cr_insert_c,"$Id: llinsert.c,v 1.17 2024/04/17 15:57:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llinsert.c,v 1.17 2024/04/17 15:57:12 cvsuser Exp $
 * Linked list management module
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

#include "llheaders.h"

List_p
ll_insert(Head_p hp, void * atom, register int pos)
{
    List_p lp, lp1;

    LL_HMAGIC(hp);
    assert(pos >= 0);
    LIST_INCR(ll_insert);

    if ((lp = ll_alloc(atom)) != (List_p)NULL) {
        lp->ll_owner = hp;

        if (pos == 0) {
            TAILQ_INSERT_HEAD(&hp->lh_queue, lp, ll_node);
        
        } else if (pos >= hp->lh_count) {
            TAILQ_INSERT_TAIL(&hp->lh_queue, lp, ll_node);
        
        } else {
            for (lp1 = TAILQ_FIRST(&hp->lh_queue); 
                    --pos > 0 && lp1; lp1 = TAILQ_NEXT(lp1, ll_node))
                /*continue*/;

            assert(pos == 0);
            assert(lp1 != NULL);

            TAILQ_INSERT_AFTER(&hp->lh_queue, lp1, lp, ll_node);
        }

        ++hp->lh_count;
    }  
    return (lp);
}

/*end*/
