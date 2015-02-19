#ifndef GR_DIFF_ENGINE_H_INCLUDED
#define GR_DIFF_ENGINE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_diff_engine_h,"$Id: diff_engine.h,v 1.6 2014/10/22 02:32:57 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- 
 * diff - compute a shortest edit script (SES) given two sequences.
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

typedef const void *(*idx_fn)(const void *s, int idx, void *context);
typedef int (*cmp_fn)(const void *a, const void *b, void *context);

typedef enum {
	DIFF_MATCH = 1,
	DIFF_DELETE,
	DIFF_INSERT
} diff_op;

struct diff_edit {
	short   op;
	int     off;        /* off into s1 if MATCH or DELETE but s2 if INSERT */
	int     len;
};

extern int                  diff_engine(const void *a, int aoff, int n, const void *b, int boff, int m,
	                             idx_fn idx, cmp_fn cmp, void *context, int dmax, struct varray *ses, int *sn, struct varray *buf);

#endif /*GR_DIFF_ENGINE_H_INCLUDED*/
