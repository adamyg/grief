#include <edidentifier.h>
__CIDENT_RCSID(cr_alloc_c,"$Id: llalloc.c,v 1.12 2022/03/21 15:13:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llalloc.c,v 1.12 2022/03/21 15:13:03 cvsuser Exp $
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

vmpool_t                ll_freelist = {0};

#if defined(LLIST_STATS) && (LLIST_STATS)
struct list_stats       __list_stats = {0};
#endif


List_p
ll_alloc(void *atom)
{
    List_p lp;

    LIST_INCR(ll_alloc);
  
    if ((lp = vm_alloc(&ll_freelist, sizeof(List))) != NULL) {
        LIST_INCR(lp_alloced);

        (void)memset(lp, 0, sizeof(List));
        lp->ll_magic = LL_LIST;
        lp->ll_elem = atom;
    }
    return (lp);
}

