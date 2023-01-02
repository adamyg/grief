#include <edidentifier.h>
__CIDENT_RCSID(gr_vtype_c,"$Id: vtype.c,v 1.11 2022/12/03 16:33:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vtype.c,v 1.11 2022/12/03 16:33:06 cvsuser Exp $
 * Self-organising integer vector data structure.
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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

#if defined(MAIN)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#undef NDEBUG           /* enforce asserts */
#else
#include <editor.h>
#endif
#include <assert.h>
#include "vtype.h"

#if defined(MAIN)
#define _D(x)           printf x;
#else
#define _D(x)
#endif

#ifndef VTBINS          /* 0^26 elements */
#define VTBINS          22
#endif

#ifndef VTINIT          /* first and second bins hold 32 then 64,128,256,... etc */
#define VTINIT          5
#endif
#define VTSIZE(i)       ((i) ? 1 << ((i) + (VTINIT - 1)) : (1 << VTINIT))

#if defined(MAIN)
void *  chk_alloc(size_t size) { return malloc(size); }
void    chk_free(void *p) { free(p); }
#endif


int
vt_init(vtable_t *va, unsigned membsize)
{
    if (va == NULL || membsize == 0) {
        errno = EINVAL;
        return -1;
    }
    memset(va, 0, sizeof *va);
    va->size = membsize;
    return 0;
}



int
vt_deinit(vtable_t *va)
{
    if (vt_release(va, 0) == -1)
        return -1;
    return 0;
}



vtable_t *
vt_new(unsigned membsize)
{
    vtable_t *va;

    if ((va = chk_alloc(sizeof *va)) == NULL)
        return NULL;
    memset(va, 0, sizeof *va);
    if (vt_init(va, membsize) == -1) {
        chk_free(va);
        return NULL;
    }
    return va;
}


int
vt_del(void *va)
{
    int ret = 0;

    if (va) {
        ret += vt_release(va, 0);
        chk_free(va);
    }

    if (ret)
        return -1;
    return 0;
}


int
vt_release(vtable_t *va, unsigned int from)
{
    unsigned int r, i;
    int ret = 0;

    if (va == NULL)
        return 0;

    r = (1 << VTINIT);
    for (i = 0; i < VTBINS; i++) {
        if (from <= r)
            break;
        r *= 2;
    }
    if (from != 0) i++;
    for ( ; i < VTBINS; i++) {
        if (va->bins[i]) {
            chk_free(va->bins[i]);
            va->bins[i] = 0;
        }
    }

    if (ret)
        return -1;
    return 0;
}


void *
vt_get(vtable_t *va, unsigned int idx)
{
    unsigned int r, i, n;

    if (va == NULL) {
        errno = EINVAL;
        return NULL;
    }

    r = (1 << VTINIT);
    for (i = 0; i < VTBINS; i++) {
        if (r > idx) {
            break;
        }
        r *= 2;
    }
    if (i == VTBINS) {
        errno = ERANGE;
        return NULL;
    }

    n = VTSIZE(i);                              /* n is nmemb in bin i */
    if (va->bins[i] == 0) {
        size_t s = n * va->size;
        char *mem = chk_alloc(s);

        _D(("vt_get: new bin %2d (%6d,%5d) = %p\n", i, s, n, mem));

        if (mem == NULL)
            return NULL;

        va->bins[i] = mem;
        memset(mem, 0, s);                      /* zero region */
    }

    return (char *)va->bins[i] + (i ? idx - n : idx) * va->size;
}


//  static int
//  vt_index(vtable_t *va, void *elem)
//  {
//      int i;
//  
//      for (i = 0; i < VTBINS; i++) {
//          if (va->bins[i]) {
//              unsigned n = VTSIZE(i);
//              char * start = va->bins[i];
//              char * end = start + n * va->size;
//  
//              if ((char *)elem >= start && (char *)elem < end)
//                  return (i ? n : 0) + (((char *)elem - start) / va->size);
//          }
//      }
//      errno = EFAULT;
//      return -1;
//  }


void
vt_iterate(void *va, vtable_iterator_t *iter)
{
    if (va && iter)
        iter->i1 = iter->i2 = 0;
}


void *
vt_next(void *va0, vtable_iterator_t *iter)
{
    vtable_t *va = va0;
    unsigned int n;

    if (va == NULL || iter == NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* n is nmemb in iter->i1 */
    n = iter->i1 == 0 ? (1 << VTINIT) :
                        1 << (iter->i1 + (VTINIT - 1));

    if (iter->i2 == n) {
        iter->i2 = 0;
        iter->i1++;
    }

    while (va->bins[iter->i1] == 0) {
        iter->i1++;
        if (iter->i1 == VTBINS) {
            return NULL;
        }
    }

    return (char *)va->bins[iter->i1] + iter->i2++ * va->size;
}


#if defined(MAIN)
int
main(void)
{
    vtable_t vt;
    int i;

#define ELEMENTS        1000000

    vt_init(&vt, sizeof(unsigned long));
    for (i = 0; i < ELEMENTS; ++i) {
        int *v = vt_get(&vt, i);
        *v = i;
    }

    for (i = 0; i < ELEMENTS; ++i) {
        int *v = vt_get(&vt, i);
        assert( *v == i );
    }

    return 0;
}
#endif

/*end*/
