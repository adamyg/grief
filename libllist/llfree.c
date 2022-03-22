#include <edidentifier.h>
__CIDENT_RCSID(cr_free_c,"$Id: llfree.c,v 1.11 2022/03/21 15:13:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llfree.c,v 1.11 2022/03/21 15:13:03 cvsuser Exp $
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

void
ll_free(Head_p hp)
{
    LL_HMAGIC(hp);
    LIST_INCR(ll_free);
    LIST_DECR(hd_alloced);

    assert(hp->lh_count == 0);
    hp->lh_magic = ~LL_HEAD;
    chk_free(hp);
}
