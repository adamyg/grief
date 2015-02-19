#ifndef GR_LOCAL_H_INCLUDED
#define GR_LOCAL_H_INCLUDED
/*	$OpenBSD: local.h,v 1.12 2005/10/10 17:37:44 espie Exp $	*/

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

#include "wcio.h"
#include "fileext.h"

#if !defined(DEFFILEMODE)			/* 0666 */
#if defined(S_IRUSR) && defined(S_IWUSR) && \
	    defined(S_IRGRP) && defined(S_IWGRP) && \
	    defined(S_IROTH) && defined(S_IWOTH)
#define DEFFILEMODE	(S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)
#else
#define DEFFILEMODE	(0666)
#endif
#endif	/*DEFFILEMODE*/

#ifndef _DIAGASSERT				/* BSD assert */
#define _DIAGASSERT(_a)
#endif

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

#define __sflush	__Bsflush
#define __sflush_locked	__Bsflush_locked
#define __sfp		__Bsfp
#define __srefill	__Bsrefill
#define __sread		__Bsread
#define __swrite	__Bswrite
#define __sseek		__Bsseek
#define __sclose	__Bsclose
#define __sinit		__Bsinit
#define _cleanup	__Bcleanup
#define __smakebuf	__Bsmakebuf
#define __swhatbuf	__Bswhatbuf
#define _fwalk		__Bfwalk
#define __swsetup	__Bswsetup
#define __sflags	__Bsflags
#define __vfprintf	__Bvfprintf
#define __sdidinit	__Bsdidinit

int	__sflush(BFILE *);
int	__sflush_locked(BFILE *);
BFILE * __sfp(void);
int	__srefill(BFILE *);
int	__sread(void *, char *, int);
int	__swrite(void *, const char *, int);
bfpos_t	__sseek(void *, bfpos_t, int);
int	__sclose(void *);
void	__sinit(void);
void	_cleanup(void);
void	__smakebuf(BFILE *);
int	__swhatbuf(BFILE *, size_t *, int *);
int	_fwalk(int (*)(BFILE *));
int	__swsetup(BFILE *);
int	__sflags(const char *, int *);
int	__vfprintf(BFILE *, const char *, __va_list);

extern void __atexit_register_cleanup(void (*)(void));
extern int __sdidinit;

/*
 * Return true if the given BFILE cannot be written now.
 */
#define	cantwrite(fp) \
	((((fp)->_flags & __BSWR) == 0 || (fp)->_bf._base == NULL) && \
	 __swsetup(fp))

/*
 * Test whether the given stdio file has an active ungetc buffer;
 * release such a buffer, without restoring ordinary unread data.
 */
#define	HASUB(fp) (_UB(fp)._base != NULL)
#define	FREEUB(fp) { \
	if (_UB(fp)._base != (fp)->_ubuf) \
		free(_UB(fp)._base); \
	_UB(fp)._base = NULL; \
}

/*
 * test for an fgetln() buffer.
 */
#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(fp) { \
	free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

#define FLOCKFILE(fp)		do { if (__Bisthreaded) bflockfile(fp); } while (0)
#define FUNLOCKFILE(fp)		do { if (__Bisthreaded) bfunlockfile(fp); } while (0)

#endif /*GR_LOCAL_H_INCLUDED*/
