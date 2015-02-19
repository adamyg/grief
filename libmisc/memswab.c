#include <edidentifier.h>
__CIDENT_RCSID(gr_memswab_c,"$Id: memswab.c,v 1.6 2015/02/19 00:17:12 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: memswab.c,v 1.6 2015/02/19 00:17:12 ayoung Exp $
 * Memory block swap
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

#include <edtypes.h>
#include <libmisc.h>

void
memswap(void *a, void *b, size_t size)
{
#define SWAPINIT(type,a,b)      type tmp, *atmp = (type *)(a), *btmp = (type *)(b)
#define SWAPDATA()              tmp = *atmp, *atmp++ = *btmp, *btmp++ = tmp

    if (size > 4) {             /* aligned dwords */
#if defined(HAVE_UINTPTR_T)
        if (0 == (((uintptr_t)a|(uintptr_t)b) & 3)) {
#else
        if (0 == (((uint32_t)a|(uint32_t)b) & 3)) {
#endif
            SWAPINIT(uint32_t, a, b);
            const int partial = size & (8 - 1);
            unsigned loops4 = (size + 8 - 1) >> 3;

            size -= partial << 3;
            switch (partial) {
            case 0:
                do {
                    size -= 8 << 3;
                    SWAPDATA();
            case 7: SWAPDATA();
            case 6: SWAPDATA();
            case 5: SWAPDATA();
            case 4: SWAPDATA();
            case 3: SWAPDATA();
            case 2: SWAPDATA();
            case 1: SWAPDATA();
                } while (--loops4);
            }
            a = (void *)atmp, b = (void *)btmp;
        }
    }

    if (size) {                 /* trailing bytes */
        SWAPINIT(uint8_t, a, b);
        const int partial = size & (4 - 1);
        unsigned loops1 = (size + 4 - 1) >> 2;

        switch (partial) {
        case 0:
            do {
                SWAPDATA();
        case 3: SWAPDATA();
        case 2: SWAPDATA();
        case 1: SWAPDATA();
            } while (--loops1);
        }
}

#undef  SWAPINIT
#undef  SWAPDATA
}
/*end*/
