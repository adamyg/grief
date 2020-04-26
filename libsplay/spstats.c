#include <edidentifier.h>
__CIDENT_RCSID(cr_spstats_c,"$Id: spstats.c,v 1.14 2020/04/20 23:08:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spstats.c,v 1.14 2020/04/20 23:08:07 cvsuser Exp $
 * libsplay version 2.0 - SPLAY tree implementation.
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

/*  Function:           spstats
 *      Return run-time stat on the tree usage.
 *  
 *  Parameters:
 *      tree - splay tree reference.
 *      buf - working buffer (> 80 characters in size).
 *
 *  Returns:
 *      Stat's information description.
 */
const char *
spstats(const SPTREE *tree, char *buf, unsigned length)
{
    __CUNUSED(buf)
    __CUNUSED(buf)
    __CUNUSED(length)

    if (tree == NULL) {
        return "NULL tree";
    }

#if defined(INTERNAL_STATS) && (INTERNAL_STATS)
    llen   = tree->lookups ? (100 * tree->lkpcmps) / tree->lookups : 0;
    elen   = tree->enqs    ? (100 * tree->enqcmps) / tree->enqs : 0;
    sloops = tree->splays  ? (100 * tree->splayloops) / tree->splays : 0;

    (void) sprintf(buf,
        "Lookups(%ld %ld.%02ld) Insertions(%ld %ld.%02ld) Splays(%ld %ld.%02ld)",
        tree->lookups, llen / 100, llen % 100,
        tree->enqs, elen / 100, elen % 100,
        tree->splays, sloops / 100, sloops % 100);

    return buf;
#else
    return "Not enabled";
#endif
}

