/*	$OpenBSD: fseek.c,v 1.7 2005/08/08 08:05:36 espie Exp $ */
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bstdio.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"

#define	POS_ERR	(-(bfpos_t)1)

/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */
int
bfseeko(BFILE *fp, off_t offset, int whence)
{
	bfpos_t (*seekfn)(void *, bfpos_t, int);
	bfpos_t target, curoff;
	size_t n;
	struct stat st;
	int havepos;

	/* make sure stdio is set up */
	if (!__sdidinit)
		__sinit();

	/*
	 * Have to be able to seek.
	 */
	if ((seekfn = fp->_seek) == NULL) {
		errno = ESPIPE;		/* historic practice */
		return (EOF);
	}

	/*
	 * Change any SEEK_CUR to SEEK_SET, and check `whence' argument.
	 * After this, whence is either SEEK_SET or SEEK_END.
	 */
	FLOCKFILE(fp);
	switch (whence) {

	case SEEK_CUR:
		/*
		 * In order to seek relative to the current stream offset,
		 * we have to first find the current stream offset a la
		 * ftell (see ftell for details).
		 */
		__sflush(fp);	/* may adjust seek offset on append stream */
		if (fp->_flags & __BSOFF)
			curoff = fp->_offset;
		else {
			curoff = (*seekfn)(fp->_cookie, (bfpos_t)0, SEEK_CUR);
			if (curoff == (bfpos_t)-1) {
				FUNLOCKFILE(fp);
				return (EOF);
			}
		}
		if (fp->_flags & __BSRD) {
			curoff -= fp->_r;
			if (HASUB(fp))
				curoff -= fp->_ur;
		} else if (fp->_flags & __BSWR && fp->_p != NULL)
			curoff += fp->_p - fp->_bf._base;

		offset += curoff;
		whence = SEEK_SET;
		havepos = 1;
		break;

	case SEEK_SET:
	case SEEK_END:
		curoff = 0;		/* XXX just to keep gcc quiet */
		havepos = 0;
		break;

	default:
		FUNLOCKFILE(fp);
		errno = EINVAL;
		return (EOF);
	}

	/*
	 * Can only optimise if:
	 *	reading (and not reading-and-writing);
	 *	not unbuffered; and
	 *	this is a `regular' Unix file (and hence seekfn==__sseek).
	 * We must check __NBF first, because it is possible to have __NBF
	 * and __BSOPT both set.
	 */
	if (fp->_bf._base == NULL)
		__smakebuf(fp);
	if (fp->_flags & (__BSWR | __BSRW | __BSNBF | __BSNPT))
		goto dumb;
	if ((fp->_flags & __BSOPT) == 0) {
		if (seekfn != __sseek ||
		    fp->_file < 0 || fstat(fp->_file, &st) ||
		    (st.st_mode & S_IFMT) != S_IFREG) {
			fp->_flags |= __BSNPT;
			goto dumb;
		}
#if defined(WIN32)
		fp->_blksize = 1024;		/* 1024 or 4096 */
#else
		fp->_blksize = st.st_blksize;
#endif
		fp->_flags |= __BSOPT;
	}

	/*
	 * We are reading; we can try to optimise.
	 * Figure out where we are going and where we are now.
	 */
	if (whence == SEEK_SET)
		target = offset;
	else {
		if (fstat(fp->_file, &st))
			goto dumb;
		target = st.st_size + offset;
	}

	if (!havepos) {
		if (fp->_flags & __BSOFF)
			curoff = fp->_offset;
		else {
			curoff = (*seekfn)(fp->_cookie, (bfpos_t)0, SEEK_CUR);
			if (curoff == POS_ERR)
				goto dumb;
		}
		curoff -= fp->_r;
		if (HASUB(fp))
			curoff -= fp->_ur;
	}

	/*
	 * Compute the number of bytes in the input buffer (pretending
	 * that any ungetc() input has been discarded).  Adjust current
	 * offset backwards by this count so that it represents the
	 * file offset for the first byte in the current input buffer.
	 */
	if (HASUB(fp)) {
		curoff += fp->_r;	/* kill off ungetc */
		n = fp->_up - fp->_bf._base;
		curoff -= n;
		n += fp->_ur;
	} else {
		n = fp->_p - fp->_bf._base;
		curoff -= n;
		n += fp->_r;
	}

	/*
	 * If the target offset is within the current buffer,
	 * simply adjust the pointers, clear EOF, undo ungetc(),
	 * and return.  (If the buffer was modified, we have to
	 * skip this; see fgetln.c.)
	 */
	if ((fp->_flags & __BSMOD) == 0 &&
	    target >= curoff && target < (bfpos_t)(curoff + n)) {
		int o = target - curoff;

		fp->_p = fp->_bf._base + o;
		fp->_r = n - o;
		if (HASUB(fp))
			FREEUB(fp);
		fp->_flags &= ~__BSEOF;
		FUNLOCKFILE(fp);
		return (0);
	}

	/*
	 * The place we want to get to is not within the current buffer,
	 * but we can still be kind to the kernel copyout mechanism.
	 * By aligning the file offset to a block boundary, we can let
	 * the kernel use the VM hardware to map pages instead of
	 * copying bytes laboriously.  Using a block boundary also
	 * ensures that we only read one block, rather than two.
	 */
	curoff = target & ~(fp->_blksize - 1);
	if ((*seekfn)(fp->_cookie, curoff, SEEK_SET) == POS_ERR)
		goto dumb;
	fp->_r = 0;
	fp->_p = fp->_bf._base;
	if (HASUB(fp))
		FREEUB(fp);
	fp->_flags &= ~__BSEOF;
	n = target - curoff;
	if (n) {
		if (__srefill(fp) || (size_t)fp->_r < n)
			goto dumb;
		fp->_p += n;
		fp->_r -= n;
	}
	FUNLOCKFILE(fp);
	return (0);

	/*
	 * We get here if we cannot optimise the seek ... just
	 * do it.  Allow the seek function to change fp->_bf._base.
	 */
dumb:
	if (__sflush(fp) ||
	    (*seekfn)(fp->_cookie, (bfpos_t)offset, whence) == POS_ERR) {
		FUNLOCKFILE(fp);
		return (EOF);
	}
	/* success: clear EOF indicator and discard ungetc() data */
	if (HASUB(fp))
		FREEUB(fp);
	fp->_p = fp->_bf._base;
	fp->_r = 0;
	/* fp->_w = 0; */	/* unnecessary (I think...) */
	fp->_flags &= ~__BSEOF;
	FUNLOCKFILE(fp);
	return (0);
}

/*
 * fseek()'s offset is a long and sizeof(off_t) != sizeof(long) on all arches
 */
int
bfseek(BFILE *fp, long offset, int whence)
{
	off_t off = offset;

	return(bfseeko(fp, off, whence));
}
