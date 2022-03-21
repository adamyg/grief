#include <edidentifier.h>
__CIDENT_RCSID(cr_hook_c,"$Id: llhook.c,v 1.12 2022/03/21 15:13:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llhook.c,v 1.12 2022/03/21 15:13:03 cvsuser Exp $
 * Linked list management module
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

#include "llheaders.h"

List_p
ll_hook(Head_p hp, List_p lp)
{
    LIST_INCR(ll_hook);
    LL_HMAGIC(hp);
    LL_MAGIC(lp);

    lp->ll_owner = hp;
    TAILQ_INSERT_TAIL(&hp->lh_queue, lp, ll_node);
    ++hp->lh_count;

    return lp;
}

