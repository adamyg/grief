#include <edidentifier.h>
__CIDENT_RCSID(cr_spblk_c,"$Id: spblk.c,v 1.19 2024/11/26 16:51:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spblk.c,v 1.19 2024/11/26 16:51:29 cvsuser Exp $
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

static vmpool_t         hd_blks;


/*
 *  spblk ---
 *      Allocate a SPBLK structure, with optional user storage using chk_alloc().
 *      Returned storage should be destroyed using spfreeblk() whereas the user storage, if any, using chk_free().
 */
SPBLK *
spblk(size_t size)
{
    static SPBLK null_blk;
    SPBLK *sp;

    if (NULL != (sp = vm_alloc(&hd_blks, sizeof(*sp)))) {
        *sp = null_blk;
        if (size > 0) {
            if (NULL == (sp->data = chk_alloc(size))) {
                vm_free(&hd_blks, sp);
                return NULL;
            }
            memset(sp->data, 0, size);
        } else {
            sp->data = NULL;
        }
        SP_INCR(spblk);
        SP_INCR(spblk_alloced);
    }
    return sp;
}


/*
 *  spfreeblk ---
 *      Free memory for an SPBLK structure.
 */
void
spfreeblk(SPBLK *sp)
{
    sp->key = sp->data = (void *)-1;            /* force memory errors on ref */
    SP_INCR(spfreeblk);
    SP_DECR(spblk_alloced);
    vm_free(&hd_blks, sp);
}

/*end*/
