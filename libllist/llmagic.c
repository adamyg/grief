#include <edidentifier.h>
__CIDENT_RCSID(cr_magic_c,"$Id: llmagic.c,v 1.11 2022/03/21 15:13:03 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: llmagic.c,v 1.11 2022/03/21 15:13:03 cvsuser Exp $
 * Linked list management module.
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


static void
chk_magic(List_p lp, short magic)
{
    assert(lp);
    assert(lp->ll_magic != ~magic);
    assert(lp->ll_magic == magic);
}


void
ll_hmagic(Head_p hp)
{
    chk_magic((List_p)hp, LL_HEAD);
}


void
ll_magic(List_p lp)
{
    chk_magic(lp, LL_LIST);
}


int
ll_hcheck(Head_p hp)
{
    List_p lp = (List_p)hp;
    
    if (lp && lp->ll_magic == LL_HEAD) {
        return 1;
    }
    return 0;
}


int
ll_check(List_p lp)
{
    if (lp && lp->ll_magic == LL_LIST) {
        return 1;
    }
    return 0;
}

