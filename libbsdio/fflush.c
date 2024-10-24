/*	$OpenBSD: fflush.c,v 1.5 2005/08/08 08:05:36 espie Exp $ */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

#include <bstdio.h>
#include <errno.h>
#include "local.h"

/* Flush a single file, or (if fp is NULL) all files.  */
int
bfflush(BFILE *fp)
{
	int r;

	if (fp == NULL)
		return (_fwalk(__sflush_locked));
	FLOCKFILE(fp);
	if ((fp->_flags & (__BSWR | __BSRW)) == 0) {
		errno = EBADF;
		r = EOF;
	} else
		r = __sflush(fp);
	FUNLOCKFILE(fp);
	return (r);
}

int
__sflush(BFILE *fp)
{
	unsigned char *p;
	int n, t;

	t = fp->_flags;
	if ((t & __BSWR) == 0)
		return (0);

	if ((p = fp->_bf._base) == NULL)
		return (0);

	n = fp->_p - p;		/* write this much */

	/*
	 * Set these immediately to avoid problems with longjmp and to allow
	 * exchange buffering (via setvbuf) in user write function.
	 */
	fp->_p = p;
	fp->_w = t & (__BSLBF|__BSNBF) ? 0 : fp->_bf._size;

	for (; n > 0; n -= t, p += t) {
		t = (*fp->_write)(fp->_cookie, (char *)p, n);
		if (t <= 0) {
			fp->_flags |= __BSERR;
			return (EOF);
		}
	}
	return (0);
}

int
__sflush_locked(BFILE *fp)
{
	int r;

	FLOCKFILE(fp);
	r = __sflush(fp);
	FUNLOCKFILE(fp);
	return (r);
}
