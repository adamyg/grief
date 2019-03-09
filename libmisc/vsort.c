#include <edidentifier.h>
__CIDENT_RCSID(gr_vsort_c,"$Id: vsort.c,v 1.7 2017/01/29 04:33:32 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*-
 * Copyright (c) 1998 - 2017, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 *
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Qsort routine based on J. L. Bentley and M. D. McIlroy,
 * "Engineering a sort function",
 *
 * Software--Practice and Experience 23 (1993) 1249-1265.
 * We have modified their original by adding a check for already-sorted input,
 * which seems to be a win per discussions on pgsql-hackers around 2006-03-21.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "varray.h"

#define Min(a, b)   ((a) < (b) ? (a) : (b))

typedef struct {
    struct varray * va;
    int             size;
    varraycmp_t     cmp;
    int             sz4;
    int             sz1;
} vqcxt_t;

static void         vqsort(vqcxt_t *cxt, int a, int n);


void
varray_sort(struct varray *va, varraycmp_t cmp)
{
    vqcxt_t cxt;

    if ((cxt.va = va) == NULL)
        return;

    if ((cxt.cmp = cmp) == NULL)
        return;

    if ((cxt.size = va->size) == 0)
        return;

    cxt.sz4 = cxt.size >> 2;
    cxt.sz1 = cxt.size & 0x03;
    assert(cxt.sz1 <= 3);

    vqsort(&cxt, 0, (int)va->upper);
}


static int
cmp(vqcxt_t *cxt, int a, int b)
{
    struct varray *va = cxt->va;
    const char *av, *bv;

    assert(a != b);
    assert(a >= 0 && a < (int)va->upper);
    assert(b >= 0 && b < (int)va->upper);
    av = varray_get(va, a);
    bv = varray_get(va, b);
    assert(av && bv);

    return cxt->cmp(va, av, bv);
}


static void
swap(vqcxt_t *cxt, int a, int b)
{
    struct varray *va = cxt->va;
    void *av, *bv;
    int sz;

    if (a == b) 
        return;

    /*  We break the swap arena into units (8*dwords and 8*bytes).
     *
     *  On the first time through the loop we get the
     *  "leftover base unit (dwords/bytes)" (len % 8).
     *
     *  On every other iteration, we perform 8 SWAP's so we
     *  handle all units.
     */
    assert(a >= 0 && a < (int)va->upper);
    assert(b >= 0 && b < (int)va->upper);
    av = varray_get(va, a);
    bv = varray_get(va, b);
    assert(av && bv);

#define SWAPINIT(type,a,b)  type swapt, *swap1 = (type *)(a), *swap2 = (type *)(b)
#define SWAPDATA()          swapt = *swap1, *swap1++ = *swap2, *swap2++ = swapt

    /*
     *  dwords
     */
    if ((sz = cxt->sz4) != 0) {
        SWAPINIT(unsigned long, av, bv);
        unsigned loops4 = (sz + 8 - 1) >> 3;

        switch (sz & (8 - 1)) {
        case 0:
            do {
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
        av = (void *)swap1, bv = (void *)swap2;
    }

    /*
     *  bytes
     */
    if ((sz = cxt->sz1) != 0) {
        SWAPINIT(unsigned char, av, bv);
        unsigned loops1 = (sz + 4 - 1) >> 2;

        switch (sz & (4 - 1)) {
        case 0:
            do {
                SWAPDATA();
        case 3: SWAPDATA();
        case 2: SWAPDATA();
        case 1: SWAPDATA();
            } while (--loops1);
        }
    }
}


static void
vecswap(vqcxt_t *cxt, int a, int b, int n)
{
    while (n-- > 0)
        swap(cxt, a++, b++);
}


static int
med3(vqcxt_t *cxt, int a, int b, int c)
{
    return cmp(cxt, a, b) < 0 ?
	    (cmp(cxt, b, c) < 0 ? b : (cmp(cxt, a, c) < 0 ? c : a))
            : (cmp(cxt, b, c) > 0 ? b : (cmp(cxt, a, c) < 0 ? a : c));
}


static void
vqsort(vqcxt_t *cxt, int a, int n)
{
#define es  1                                   /*element size*/
    int     pa, pb, pc, pd, pl, pm, pn;
    int     d, r, presorted;

loop:   
    if (n < 7)
    {
        for (pm = a + es; pm < a + n * es; pm += es)
            for (pl = pm; pl > a && cmp(cxt, pl - es, pl) > 0; pl -= es)
                swap(cxt, pl, pl - es);
        return;
    }

    presorted = 1;
    for (pm = a + es; pm < a + n * es; pm += es)
    {
        if (cmp(cxt, pm - es, pm) > 0)
        {
            presorted = 0;
            break;
        }
    }
    if (presorted)
        return;

    pm = a + (n / 2) * es;
    if (n > 7)
    {
        pl = a;
        pn = a + (n - 1) * es;
        if (n > 40)
        {
            d = (n / 8) * es;
            pl = med3(cxt, pl, pl + d, pl + 2 * d);
            pm = med3(cxt, pm - d, pm, pm + d);
            pn = med3(cxt, pn - 2 * d, pn - d, pn);
        }
        pm = med3(cxt, pl, pm, pn);
    }

    swap(cxt, a, pm);
    pa = pb = a + es;
    pc = pd = a + (n - 1) * es;

    for (;;)
    {
        while (pb <= pc && (r = cmp(cxt, pb, a)) <= 0)
        {
            if (r == 0)
            {
                swap(cxt, pa, pb);
                pa += es;
            }
            pb += es;
        }

        while (pb <= pc && (r = cmp(cxt, pc, a)) >= 0)
        {
            if (r == 0)
            {
                swap(cxt, pc, pd);
                pd -= es;
            }
            pc -= es;
        }

        if (pb > pc)
            break;
        swap(cxt, pb, pc);
        pb += es;
        pc -= es;
    }
    pn = a + n * es;
    r = Min(pa - a, pb - pa);
    vecswap(cxt, a, pb - r, r);
    r = Min(pd - pc, pn - pd - es);
    vecswap(cxt, pb, pn - r, r);
    if ((r = pb - pa) > es)
        vqsort(cxt, a, r / es);
    if ((r = pd - pc) > es)
    {
        /* Iterate rather than recurse to save stack space */
        a = pn - r;
        n = r / es;
        goto loop;
    }
}
/*end*/
