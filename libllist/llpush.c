#include <edidentifier.h>
__CIDENT_RCSID(cr_push_c,"$Id: llpush.c,v 1.16 2024/04/17 15:57:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llpush.c,v 1.16 2024/04/17 15:57:12 cvsuser Exp $
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

void *
ll_push(Head_p hp, void * atom)
{
    LIST_INCR(ll_push);
    ll_insert(hp, atom, 0);
    return atom;
}


int
ll_pop(Head_p hp)
{
    List_p lp;

    LIST_INCR(ll_pop);
    if ((lp = ll_first(hp)) != NULL)
        ll_delete(lp);
    return ll_first(hp) ? 1 : 0;                /* TRUE if more */
}

/*end*/
