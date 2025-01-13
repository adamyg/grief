#include <edidentifier.h>
__CIDENT_RCSID(cr_spzap_c,"$Id: spzap.c,v 1.10 2025/01/13 15:19:44 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spzap.c,v 1.10 2025/01/13 15:19:44 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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


void
spzap(SPTREE *tree, SPDELETE deleter, void *udata)
{
    while (! spempty(tree)) {
        SPBLK *sp = sphead(tree);
        void *data = sp->data;

        spdeq(sp, tree);
        if (deleter) {
            (*deleter)(data, udata);
        }
        spfreeblk(sp);
    }
    spfree(tree);
}
