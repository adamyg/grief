#ifndef GR_LIBMISC_H_INCLUDED
#define GR_LIBMISC_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libmisc_h,"$Id: libmisc.h,v 1.21 2023/01/01 11:26:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libmisc.h,v 1.21 2023/01/01 11:26:59 cvsuser Exp $
 * libmisc - Miscellaneous library functions.
 *
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

#include <edtypes.h>

#ifndef FALSE
#define FALSE               0
#define TRUE                1
#endif
#define ABORT               2

__CBEGIN_DECLS

extern void                 memswap(void *a, void *b, size_t size);

extern uint32_t             crc32_EDB88320(const void *data, size_t size, uint32_t crc32);

typedef int (* sortcmpr_t)(void *thunk, const void *a, const void *b);
typedef int (* sortcmp_t)(const void *a, const void *b);

extern void                 bsd_qsort_r(void *base, size_t nmemb, size_t size, void *thunk, sortcmpr_t cmp);
extern void                 bsd_qsort(void *base, size_t nmemb, size_t size, sortcmp_t cmp);

extern int                  bsd_mergesort_r(void *base, size_t nmemb, size_t size, void *thunk, sortcmpr_t cmp);
extern int                  bsd_mergesort(void *base, size_t nmemb, size_t size, sortcmp_t cmp);

extern int                  bsd_heapsort_r(void *vbase, size_t nmemb, size_t size, void *thunk, sortcmpr_t cmp);
extern int                  bsd_heapsort(void *vbase, size_t nmemb, size_t size, sortcmp_t cmp);

extern int                  bsd_radixsort(const uchar_t **a, int n, const uchar_t *tab, uint_t endch);
extern int                  bsd_sradixsort(const uchar_t **a, int n, const uchar_t *tab, uint_t endch);

__CEND_DECLS

#endif /*GR_LIBMISC_H_INCLUDED*/
