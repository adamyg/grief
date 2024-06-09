#include <edidentifier.h>
__CIDENT_RCSID(cr_unhook_c,"$Id: llunhook.c,v 1.16 2024/04/17 15:57:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llunhook.c,v 1.16 2024/04/17 15:57:12 cvsuser Exp $
 * Linked list management module.
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
ll_unhook(List_p lp)
{
    Head_p hp;

    LL_MAGIC(lp);
    LIST_INCR(ll_unhook);

    hp = lp->ll_owner;
    assert(hp);
    LL_HMAGIC(hp);
    assert(hp->lh_count > 0);
  
    TAILQ_REMOVE(&hp->lh_queue, lp, ll_node);
    lp->ll_owner = NULL;
    --hp->lh_count;

    return (lp);
}

/*end*/
