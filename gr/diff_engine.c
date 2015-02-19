#include <edidentifier.h>
__CIDENT_RCSID(gr_diff_engine_c,"$Id: diff_engine.c,v 1.4 2014/10/22 02:32:57 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* diff - compute a shortest edit script (SES) given two sequences 
 *
 *  This algorithm is basically Myers' solution to SES/LCS with the Hirschberg linear space 
 *  refinement as described in the following publication:
 *
 *      E. Myers, ``An O(ND) Difference Algorithm and Its Variations,''
 *      Algorithmica 1, 2 (1986), 251-266.
 *      http://www.cs.arizona.edu/people/gene/PAPERS/diff.ps
 *
 *  This is the same algorithm used by GNU diff(1).
 *
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
 */

#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include <errno.h>

#include "diff_engine.h"

/*  --------------------------------------------------------------------------------------  */

#ifndef VARRAY_INIT_SIZE
#define VARRAY_INIT_SIZE    5
#endif

typedef char * ptrdiff_t;
typedef void * ref_t;

#define BINSIZ(i)           ((i) ? 1 << ((i) + (VARRAY_INIT_SIZE-1)) : (1 << VARRAY_INIT_SIZE))

struct varray {
        size_t      size;               /* element size */
	ptrdiff_t   al;                 /* relative offset of this object to allocator of bins */
	ref_t       bins[16];           /* 0 to 2^20 elements */
};


static int          varray_init(struct varray *va, size_t membsize /*, struct allocator *al*/);
//  static int              varray_reinit(struct varray *va, struct allocator *al);
static int          varray_deinit(struct varray *va);
//  static struct varray *  varray_new(size_t membsize, struct allocator *al);
//  static int              varray_del(void *va);
static int          varray_release(struct varray *va, unsigned int from);
static void *       varray_get(struct varray *va, unsigned int idx);
//  static int              varray_index(struct varray *va, void *elem);
//  static void             varray_iterate(void *va, iter_t *iter);
//  static void *           varray_next(void *va, iter_t *iter);


static int
varray_init(struct varray *va, size_t membsize /*, struct allocator *al*/)
{
	if (va == NULL || membsize == 0) {
//		PMNO(errno = EINVAL);
		return -1;
	}

	memset(va, 0, sizeof *va);
	va->size = membsize;
//	if (al && al->tail) {                   /* al is a suba allocator */
//		va->al = (char *)va - (char *)al;
//	}
	return 0;
}


static int
varray_deinit(struct varray *va)
{
	if (varray_release(va, 0) == -1) {
//		AMSG("");
		return -1;
	}
	return 0;
}


static int
varray_release(struct varray *va, unsigned int from)
{
	unsigned int r, i;
	int ret = 0;

	if (va == NULL) {
		return 0;
	}

	r = (1 << VARRAY_INIT_SIZE);
	for (i = 0; i < 16; i++) {
		if (from <= r) {
			break;
		}
		r *= 2;
	}
	if (from != 0) i++;
	for ( ; i < 16; i++) {
		if (va->bins[i]) {
                        free (va->bins[i]);
			va->bins[i] = 0;
		}
	}

	if (ret) {
//		AMSG("");
		return -1;
	}

	return 0;
}


static void *
varray_get(struct varray *va, unsigned int idx)
{
	unsigned int r, i, n;

	if (va == NULL) {
//		PMNO(errno = EINVAL);
		return NULL;
	}

	r = (1 << VARRAY_INIT_SIZE);        /* First and second bins hold 32 then 64,128,256,... */
	for (i = 0; i < 16; i++) {
		if (r > idx) {
			break;
		}
		r *= 2;
	}
	if (i == 16) {
//		PMNO(errno = ERANGE);
		return NULL;
	}

	n = BINSIZ(i);                      /* n is nmemb in bin i */

        if (NULL == va->bins[i]) {
	        char *mem = calloc (n * va->size, 1);
		if (mem == NULL) {
//			AMSG("");
			return NULL;
		}
		va->bins[i] = mem;
	}

        return ((char *)va->bins[i] + (i ? idx - n : idx) * va->size);
}


/*  --------------------------------------------------------------------------------------  */

#define FV(k)       _v(ctx, (k), 0)
#define RV(k)       _v(ctx, (k), 1)

struct _ctx {
	idx_fn idx;
	cmp_fn cmp;
	void *context;
	struct varray *buf;
	struct varray *ses;
	int si;
	int dmax;
};

struct middle_snake {
	int x, y, u, v;
};

static void
_setv(struct _ctx *ctx, int k, int r, int val)
{
	int j;
	int *i;

        /* Pack -N to N into 0 to N * 2
         */
	j = k <= 0 ? -k * 4 + r : k * 4 + (r - 2);

	i = (int *)varray_get(ctx->buf, j);
	*i = val;
}


static int
_v(struct _ctx *ctx, int k, int r)
{
	int j;

	j = k <= 0 ? -k * 4 + r : k * 4 + (r - 2);

	return *((int *)varray_get(ctx->buf, j));
}


static int
_find_middle_snake(const void *a, int aoff, int n, const void *b, int boff, int m,
        struct _ctx *ctx, struct middle_snake *ms)
{
	int delta, odd, mid, d;

	delta = n - m;
	odd = delta & 1;
	mid = (n + m) / 2;
	mid += odd;

	_setv(ctx, 1, 0, 0);
	_setv(ctx, delta - 1, 1, n);

	for (d = 0; d <= mid; d++) {
		int k, x, y;

		if ((2 * d - 1) >= ctx->dmax) {
			return ctx->dmax;
		}

		for (k = d; k >= -d; k -= 2) {
			if (k == -d || (k != d && FV(k - 1) < FV(k + 1))) {
				x = FV(k + 1);
			} else {
				x = FV(k - 1) + 1;
			}
			y = x - k;

			ms->x = x;
			ms->y = y;
			if (ctx->cmp) {
				while (x < n && y < m &&
                                        ctx->cmp(ctx->idx(a, aoff + x, ctx->context), ctx->idx(b, boff + y, ctx->context), ctx->context) == 0) {
					x++; y++;
				}
			} else {
				const unsigned char *a0 = (const unsigned char *)a + aoff;
				const unsigned char *b0 = (const unsigned char *)b + boff;
				while (x < n && y < m && a0[x] == b0[y]) {
					x++; y++;
				}
			}
			_setv(ctx, k, 0, x);

			if (odd && k >= (delta - (d - 1)) && k <= (delta + (d - 1))) {
				if (x >= RV(k)) {
					ms->u = x;
					ms->v = y;
					return 2 * d - 1;
				}
			}
		}
		for (k = d; k >= -d; k -= 2) {
			int kr = (n - m) + k;

			if (k == d || (k != -d && RV(kr - 1) < RV(kr + 1))) {
				x = RV(kr - 1);
			} else {
				x = RV(kr + 1) - 1;
			}
			y = x - kr;

			ms->u = x;
			ms->v = y;
			if (ctx->cmp) {
			        while (x > 0 && y > 0 && 
                                                ctx->cmp(ctx->idx(a, aoff + (x - 1), ctx->context), ctx->idx(b, boff + (y - 1), ctx->context), ctx->context) == 0) {
					x--; y--;
				}
			} else {
				const unsigned char *a0 = (const unsigned char *)a + aoff;
				const unsigned char *b0 = (const unsigned char *)b + boff;
				while (x > 0 && y > 0 && a0[x - 1] == b0[y - 1]) {
					x--; y--;
				}
			}
			_setv(ctx, kr, 1, x);

			if (!odd && kr >= -d && kr <= d) {
				if (x <= FV(kr)) {
					ms->x = x;
					ms->y = y;
					return 2 * d;
				}
			}
		}
	}

	errno = EFAULT;

	return -1;
}


static void
_edit(struct _ctx *ctx, int op, int off, int len)
{
	struct diff_edit *e;

	if (len == 0 || ctx->ses == NULL) {
		return;
	}           /* Add an edit to the SES (or
                     * coalesce if the op is the same)
                     */
	e = varray_get(ctx->ses, ctx->si);
	if (e->op != op) {
		if (e->op) {
			ctx->si++;
			e = varray_get(ctx->ses, ctx->si);
		}
		e->op = op;
		e->off = off;
		e->len = len;
	} else {
		e->len += len;
	}
}


static int
_ses(const void *a, int aoff, int n, const void *b, int boff, int m, struct _ctx *ctx)
{
	struct middle_snake ms;
	int d;

	if (n == 0) {
		_edit(ctx, DIFF_INSERT, boff, m);
		d = m;
	} else if (m == 0) {
		_edit(ctx, DIFF_DELETE, aoff, n);
		d = n;
	} else {
                    /* Find the middle "snake" around which we
                     * recursively solve the sub-problems.
                     */
		d = _find_middle_snake(a, aoff, n, b, boff, m, ctx, &ms);
		if (d == -1) {
			return -1;
		} else if (d >= ctx->dmax) {
			return ctx->dmax;
		} else if (ctx->ses == NULL) {
			return d;
		} else if (d > 1) {
			if (_ses(a, aoff, ms.x, b, boff, ms.y, ctx) == -1) {
				return -1;
			}

			_edit(ctx, DIFF_MATCH, aoff + ms.x, ms.u - ms.x);

			aoff += ms.u;
			boff += ms.v;
			n -= ms.u;
			m -= ms.v;
			if (_ses(a, aoff, n, b, boff, m, ctx) == -1) {
				return -1;
			}
		} else {
			int x = ms.x;
			int u = ms.u;

                 /* There are only 4 base cases when the
                  * edit distance is 1.
                  *
                  * n > m   m > n
                  *
                  *   -       |
                  *    \       \    x != u
                  *     \       \
                  *
                  *   \       \
                  *    \       \    x == u
                  *     -       |
                  */

			if (m > n) {
				if (x == u) {
					_edit(ctx, DIFF_MATCH, aoff, n);
					_edit(ctx, DIFF_INSERT, boff + (m - 1), 1);
				} else {
					_edit(ctx, DIFF_INSERT, boff, 1);
					_edit(ctx, DIFF_MATCH, aoff, n);
				}
			} else {
				if (x == u) {
					_edit(ctx, DIFF_MATCH, aoff, m);
					_edit(ctx, DIFF_DELETE, aoff + (n - 1), 1);
				} else {
					_edit(ctx, DIFF_DELETE, aoff, 1);
					_edit(ctx, DIFF_MATCH, aoff + 1, m);
				}
			}
		}
	}

	return d;
}


int
diff_engine(const void *a, int aoff, int n, const void *b, int boff, int m, idx_fn idx, 
        cmp_fn cmp, void *context, int dmax, struct varray *ses, int *sn, struct varray *buf)
{
	struct _ctx ctx;
	int d, x, y;
	struct diff_edit *e = NULL;
	struct varray tmp;

	if (!idx != !cmp) {                     /* ensure both NULL or both non-NULL */
		errno = EINVAL;
		return -1;
	}

	ctx.idx = idx;
	ctx.cmp = cmp;
	ctx.context = context;
	if (buf) {
		ctx.buf = buf;
	} else {
		varray_init(&tmp, sizeof(int) /*, NULL*/);
		ctx.buf = &tmp;
	}
	ctx.ses = ses;
	ctx.si = 0;
	ctx.dmax = dmax ? dmax : INT_MAX;

	if (ses && sn) {
		if ((e = varray_get(ses, 0)) == NULL) {
//			AMSG("");
			if (NULL == buf) {
				varray_deinit(&tmp);
			}
			return -1;
		}
		e->op = 0;
	}

         /* The _ses function assumes the SES will begin or end with a delete
          * or insert. The following will insure this is true by eating any
          * beginning matches. This is also a quick to process sequences
          * that match entirely.
          */
	x = y = 0;
	if (cmp) {
		while (x < n && y < m && 
                            cmp(idx(a, aoff + x, context), idx(b, boff + y, context), context) == 0) {
			x++; y++;
		}
	} else {
		const unsigned char *a0 = (const unsigned char *)a + aoff;
		const unsigned char *b0 = (const unsigned char *)b + boff;
		while (x < n && y < m && a0[x] == b0[y]) {
			x++; y++;
		}
	}
	_edit(&ctx, DIFF_MATCH, aoff, x);

	if ((d = _ses(a, aoff + x, n - x, b, boff + y, m - y, &ctx)) == -1) {
//		AMSG("");
		if (buf == NULL) {
			varray_deinit(&tmp);
		}
		return -1;
	}
	if (ses && sn) {
		*sn = e->op ? ctx.si + 1 : 0;
	}

	if (buf == NULL) {
		varray_deinit(&tmp);
	}

	return d;
}

