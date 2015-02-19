/*	$OpenBSD: fopen.c,v 1.5 2005/08/08 08:05:36 espie Exp $ */
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

#define __USE_BSD

#include <sys/types.h>
#include <sys/stat.h>
#include <bstdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "local.h"

BFILE *
bfopen(const char *file, const char *mode)
{
	BFILE *fp;
	int f;
	int flags, oflags;

	if ((flags = __sflags(mode, &oflags)) == 0)
		return (NULL);
	if ((fp = __sfp()) == NULL)
		return (NULL);
	if ((f = open(file, oflags, DEFFILEMODE)) < 0) {
		fp->_flags = 0;			/* release */
		return (NULL);
	}
	fp->_file   = f;
	fp->_flags  = flags;
	fp->_cookie = fp;
	fp->_read   = __sread;
	fp->_write  = __swrite;
	fp->_seek   = __sseek;
	fp->_close  = __sclose;

	/*
	 * When opening in append mode, even though we use O_APPEND,
	 * we need to seek to the end so that ftell() gets the right
	 * answer.  If the user then alters the seek pointer, or
	 * the file extends, this will fail, but there is not much
	 * we can do about this.  (We could set __BSAPP and check in
	 * fseek and ftell.)
	 */
	if (oflags & O_APPEND) {
		__sseek((void *)fp, (bfpos_t)0, (int)SEEK_END);
	}
	return (fp);
}
