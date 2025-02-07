#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_qsort_c,"$Id: bsd_qsort.c,v 1.9 2025/02/07 02:48:37 cvsuser Exp $")

/*- -*- indent-width: 4; tabs: 8; -*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <edtypes.h>
#if defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__)
#include <sys/utypes.h>
#else
#include <sys/types.h>
#endif
#include <libmisc.h>

#include <stdlib.h>
#if defined(I_AM_QSORT_S)
#include <errno.h>
#endif

#if defined(I_AM_QSORT_R)
#define cmp_t sortcmpr_t
#elif defined(I_AM_QSORT_R_COMPAT)
#define cmp_t osortcmpr_t
#elif defined(I_AM_QSORT_S)
#define cmp_t sortcmpr_t
#else
#define cmp_t sortcmp_t
#endif
static __CINLINE char	*med3(char *, char *, char *, cmp_t, void *);

#undef MIN
#define	MIN(a, b)	((a) < (b) ? a : b)


/*
 * qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */

static __CINLINE void
swapfunc(char *a, char *b, size_t es)
{
	char t;

	do {
		t = *a;
		*a++ = *b;
		*b++ = t;
	} while (--es > 0);
}

#define	vecswap(a, b, n) \
	if ((n) > 0) swapfunc(a, b, n)

#if defined(I_AM_QSORT_R)                       // sortcmpr_t=(const void *a, const void *b, void *thunk)
#define	CMP(t, x, y) (cmp((x), (y), (t)))
#elif defined(I_AM_QSORT_R_COMPAT)              // osortcmpr_t=(void *thunk, const void *a, const void *b)
#define	CMP(t, x, y) (cmp((t), (x), (y)))
#elif defined(I_AM_QSORT_S)                     // sortcmpr_t=(const void *a, const void *b, void *thunk)
#define	CMP(t, x, y) (cmp((x), (y), (t)))
#else                                           // sortcmp_t=(const void *a, const void *b)
#define	CMP(t, x, y) (cmp((x), (y)))
#endif

static __CINLINE char *
med3(char *a, char *b, char *c, cmp_t cmp, void *thunk)
{
	return CMP(thunk, a, b) < 0 ?
	       (CMP(thunk, b, c) < 0 ? b : (CMP(thunk, a, c) < 0 ? c : a ))
	      :(CMP(thunk, b, c) > 0 ? b : (CMP(thunk, a, c) < 0 ? a : c ));
}

/*
 * The actual qsort() implementation is static to avoid preemptible calls when
 * recursing. Also give them different names for improved debugging.
 */
#if defined(I_AM_QSORT_R)
#define local_qsort local_qsort_r
#elif defined(I_AM_QSORT_R_COMPAT)
#define local_qsort local_qsort_r_compat
#elif defined(I_AM_QSORT_S)
#define local_qsort local_qsort_s
#endif
static void
local_qsort(void *a, size_t n, size_t es, cmp_t cmp, void *thunk)
{
	char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
	size_t d1, d2;
	int cmp_result;
	int swap_cnt;

	/* if there are less than 2 elements, then sorting is not needed */
//	if (__predict_false(n < 2))
	if (n < 2)
		return;
loop:
	swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm;
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swapfunc(pl, pl - es, es);
		return;
	}
	pm = (char *)a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *)a + (n - 1) * es;
		if (n > 40) {
			size_t d = (n / 8) * es;

			pl = med3(pl, pl + d, pl + 2 * d, cmp, thunk);
			pm = med3(pm - d, pm, pm + d, cmp, thunk);
			pn = med3(pn - 2 * d, pn - d, pn, cmp, thunk);
		}
		pm = med3(pl, pm, pn, cmp, thunk);
	}
	swapfunc(a, pm, es);
	pa = pb = (char *)a + es;

	pc = pd = (char *)a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (cmp_result = CMP(thunk, pb, a)) <= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swapfunc(pa, pb, es);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (cmp_result = CMP(thunk, pc, a)) >= 0) {
			if (cmp_result == 0) {
				swap_cnt = 1;
				swapfunc(pc, pd, es);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc)
			break;
		swapfunc(pb, pc, es);
		swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	if (swap_cnt == 0) {  /* Switch to insertion sort */
		for (pm = (char *)a + es; pm < (char *)a + n * es; pm += es)
			for (pl = pm;
			     pl > (char *)a && CMP(thunk, pl - es, pl) > 0;
			     pl -= es)
				swapfunc(pl, pl - es, es);
		return;
	}

	pn = (char *)a + n * es;
	d1 = MIN(pa - (char *)a, pb - pa);
	vecswap(a, pb - d1, d1);
	/*
	 * Cast es to preserve signedness of right-hand side of MIN()
	 * expression, to avoid sign ambiguity in the implied comparison.  es
	 * is safely within [0, SSIZE_MAX].
	 */
	d1 = MIN(pd - pc, pn - pd - (ssize_t)es);
	vecswap(pb, pn - d1, d1);

	d1 = pb - pa;
	d2 = pd - pc;
	if (d1 <= d2) {
		/* Recurse on left partition, then iterate on right partition */
		if (d1 > es) {
			local_qsort(a, d1 / es, es, cmp, thunk);
		}
		if (d2 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(pn - d2, d2 / es, es, cmp); */
			a = pn - d2;
			n = d2 / es;
			goto loop;
		}
	} else {
		/* Recurse on right partition, then iterate on left partition */
		if (d2 > es) {
			local_qsort(pn - d2, d2 / es, es, cmp, thunk);
		}
		if (d1 > es) {
			/* Iterate rather than recurse to save stack space */
			/* qsort(a, d1 / es, es, cmp); */
			n = d1 / es;
			goto loop;
		}
	}
}

#if defined(I_AM_QSORT_R)
void
bsd_qsort_r(void *a, size_t n, size_t es, void *thunk, cmp_t cmp)
{
	local_qsort_r(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_R_COMPAT)
void
bsd_qsort_r_compat(void *a, size_t n, size_t es, cmp_t cmp, void *thunk)
{
	local_qsort_r_compat(a, n, es, cmp, thunk);
}
#elif defined(I_AM_QSORT_S)
int /*errno_t*/
bsd_qsort_s(void *a, size_t /*rsize_t*/ n, size_t /*rsize_t*/ es, cmp_t cmp, void *thunk)
{
#if !defined(RSIZE_MAX)
#define RSIZE_MAX SIZE_MAX
#endif
	if (n > RSIZE_MAX) {
		return (EINVAL);
	} else if (es > RSIZE_MAX) {
		return (EINVAL);
	} else if (n != 0) {
		if (a == NULL) {
			return (EINVAL);
		} else if (cmp == NULL) {
			return (EINVAL);
		} else if (es <= 0) {
			return (EINVAL);
		}
	}
	local_qsort_s(a, n, es, cmp, thunk);
	return (0);
}
#else
void
bsd_qsort(void *a, size_t n, size_t es, cmp_t cmp)
{
	local_qsort(a, n, es, cmp, NULL);
}
#endif
