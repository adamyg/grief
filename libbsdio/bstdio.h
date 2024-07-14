#ifndef GR_BSTDIO_H_INCLUDED
#define GR_BSTDIO_H_INCLUDED
/* $OpenBSD: stdio.h,v 1.35 2006/01/13 18:10:09 miod Exp $	*/
/* $NetBSD: stdio.h,v 1.18 1996/04/25 18:29:21 jtc Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 *
 *	@(#)stdio.h	5.17 (Berkeley) 6/3/91
 */

#include <edcdefs.h>
#include <sys/types.h>

#include <edtypes.h>
#include <stdio.h>
#include <stdarg.h>

#include <stddef.h>
#if !defined(__va_list)
#define __va_list va_list
#endif

#if defined(__BSTDIO_INTERNAL) || defined(_BSD_SOURCE) 
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif

#ifndef __POSIX_VISIBLE
#define __POSIX_VISIBLE 1
#endif
#endif

#if defined(__BSTDIO_INLINE) || defined(__BSTDIO_INTERNAL)
#if !defined(__BSTDIO_INLINE)
#define __BSTDIO_INLINE
#endif
#if !defined(__BSTDIO_INTERNAL)
#define __BSTDIO_INTERNAL		/* see __Bsputc() */
#endif
#endif

/*
 * NB: to fit things in six character monocase externals, the stdio
 *  code uses the prefix `__s' for stdio objects, typically followed
 *  by a three-character attempt at a mnemonic.
 */
typedef int64_t bfpos_t;		/* fpos_t */

/* stdio buffers */
#if defined(__BSTDIO_INTERNAL)

__BEGIN_DECLS
struct __Bsbuf {
	unsigned char *_base;
	int _size;
};

__END_DECLS

#endif  /*__BSTDIO_INTERNAL*/


/*
 * stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__BSLBF|__BSWR)) == (__BSLBF|__BSWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__BSRD, _w is 0
 *	if _flags&__BSWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 *
 * NOTE: if you change this structure, you also need to update the
 * std() initializer in findfp.c.
 */
#if defined(__BSTDIO_INTERNAL)

__BEGIN_DECLS
typedef struct __BsFILE {
	unsigned char *_p;		/* current position in (some) buffer */
	int	_r;			/* read space left for getc() */
	int	_w;			/* write space left for putc() */
	short	_flags;			/* flags, below; this FILE is free if 0 */
	short	_file;			/* fileno, if Unix descriptor, else -1 */
	struct	__Bsbuf _bf;		/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;		/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;		/* cookie passed to io functions */
	int	(*_close)(void *);
	int	(*_read)(void *, char *, int);
	bfpos_t	(*_seek)(void *, bfpos_t, int);
	int	(*_write)(void *, const char *, int);

	/* extension data, to avoid further ABI breakage */
	struct	__Bsbuf _ext;

	/* data for long sequences of ungetc() */
	unsigned char *_up;		/* saved _p when _p is doing ungetc data */
	int	_ur;			/* saved _r when _r is counting ungetc data */

	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];		/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];		/* guarantee a getc() buffer */

	/* separate buffer for fgetln() when line crosses buffer boundary */
	struct	__Bsbuf _lb;		/* buffer for fgetln() */

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;		/* stat.st_blksize (may be != _bf._size) */
	bfpos_t	_offset;		/* current lseek offset */
} BFILE;

__END_DECLS

#define __BSLBF		0x0001		/* line buffered */
#define __BSNBF		0x0002		/* unbuffered */
#define __BSRD		0x0004		/* OK to read */
#define __BSWR		0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define __BSRW		0x0010		/* open for reading & writing */
#define __BSEOF		0x0020		/* found EOF */
#define __BSERR		0x0040		/* found error */
#define __BSMBF		0x0080		/* _buf is from malloc */
#define __BSAPP		0x0100		/* fdopen()ed in append mode */
#define __BSSTR		0x0200		/* this is an sprintf/snprintf string */
#define __BSOPT		0x0400		/* do fseek() optimisation */
#define __BSNPT		0x0800		/* do not do fseek() optimisation */
#define __BSOFF		0x1000		/* set iff _offset is in fact correct */
#define __BSMOD		0x2000		/* true => fgetln modified _p text */
#define __BSALC		0x4000		/* allocate string space dynamically */
#define __BSIGN		0x8000		/* ignore this file in _fwalk */

#else

typedef void * BFILE;

#endif  /*__BSTDIO_INTERNAL*/


/*
 * The following three definitions are for ANSI C, which took them
 * from System V, which brilliantly took internal interface macros and
 * made them official arguments to setvbuf(), without renaming them.
 * Hence, these ugly _IOxxx names are *supposed* to appear in user code.
 *
 * Although numbered as their counterparts above, the implementation
 * does not rely on this.
 */

#if !defined(_IOFBF)
#define _IOFBF		0		/* setvbuf should set fully buffered */
#define _IOLBF		1		/* setvbuf should set line buffered */
#define _IONBF		2		/* setvbuf should set unbuffered */
#endif

#if !defined(BUFSIZ)
#define BUFSIZ		1024		/* size of buffer used by setbuf */
#endif

#if !defined(EOF)
#define EOF		(-1)
#endif

/*
 * FOPEN_MAX is a minimum maximum, and should be the number of descriptors
 * that the kernel can provide without allocation of a resource that can
 * fail without the process sleeping.  Do not use this for anything.
 */
#define BOPEN_MAX	20		/* must be <= OPEN_MAX <sys/syslimits.h> */

#define BFILENAME_MAX	1024		/* must be <= PATH_MAX <sys/syslimits.h> */

#ifndef SEEK_SET
#define SEEK_SET	0		/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR	1		/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END	2		/* set file offset to EOF plus offset */
#endif


/*
 * Functions defined in ANSI C standard.
 */
__BEGIN_DECLS
void	 bclearerr(BFILE *);
int	 bfclose(BFILE *);
int	 bfeof(BFILE *);
int	 bferror(BFILE *);
int	 bfflush(BFILE *);
int	 bfgetc(BFILE *);
int	 bfgetpos(BFILE *, bfpos_t *);
char	*bfgets(char *, int, BFILE *);
BFILE	*bfopen(const char *, const char *);
int	 bfprintf(BFILE *, const char *, ...)
		__ATTRIBUTE_FORMAT__ ((printf, 2, 3))
		__ATTRIBUTE_NONNULL__ ((2));
int	 bfputc(int, BFILE *);
int	 bfputs(const char *, BFILE *);
size_t	 bfread(void *, size_t, size_t, BFILE *);
BFILE	*bfreopen(const char *, const char *, BFILE *);
int	 bfscanf(BFILE *, const char *, ...)
		__ATTRIBUTE_FORMAT__ ((scanf, 2, 3))
		__ATTRIBUTE_NONNULL__ ((2));
int	 bfseek(BFILE *, long, int);
int	 bfseeko(BFILE *, off_t, int);
int	 bfsetpos(BFILE *, const bfpos_t *);
long	 bftell(BFILE *);
off_t	 bftello(BFILE *);
size_t	 bfwrite(const void *, size_t, size_t, BFILE *);
int	 bgetc(BFILE *);
int	 bgetchar(void);
int	 bgetdelim(char ** __CRESTRICT, size_t * __CRESTRICT, int, BFILE * __CRESTRICT);
int	 bgetline(char ** __CRESTRICT, size_t * __CRESTRICT, BFILE * __CRESTRICT);
char	*bgets(char *);

int	 bprintf(const char *, ...)
		__ATTRIBUTE_FORMAT__ ((printf, 1, 2))
		__ATTRIBUTE_NONNULL__ ((1));
int	 bputc(int, BFILE *);
void	 brewind(BFILE *);
void	 bsetbuf(BFILE *, char *);
int	 bsetvbuf(BFILE *, char *, int, size_t);
int	 bsprintf(char *, const char *, ...)
			__ATTRIBUTE_FORMAT__ ((printf, 2, 3))
			__ATTRIBUTE_NONNULL__ ((2));
int	 bvsprintf(char *, const char *, __va_list ap)
			__ATTRIBUTE_FORMAT__ ((printf, 2, 0))
			__ATTRIBUTE_NONNULL__ ((2));
int	 bsscanf(const char *, const char *, ...)
		__ATTRIBUTE_FORMAT__ ((scanf, 2, 3))
		__ATTRIBUTE_NONNULL__ ((2));
int	 bvsscanf(const char *, const char *, __va_list ap)
		__ATTRIBUTE_FORMAT__ ((scanf, 2, 0))
		__ATTRIBUTE_NONNULL__ ((2));
BFILE	*btmpfile(void);
int	 bungetc(int, BFILE *);
int	 bvfprintf(BFILE *, const char *, __va_list)
		__ATTRIBUTE_FORMAT__ ((printf, 2, 0))
		__ATTRIBUTE_NONNULL__ ((2));
int	 bsnprintf(char *, size_t, const char *, ...)
		__ATTRIBUTE_FORMAT__ ((printf, 3, 4))
		__ATTRIBUTE_NONNULL__ ((3));
int	 bvfscanf(BFILE *, const char *, __va_list)
		__ATTRIBUTE_FORMAT__ ((scanf, 2, 0))
		__ATTRIBUTE_NONNULL__ ((2));
int	 bvsnprintf(char *, size_t, const char *, __va_list)
		__ATTRIBUTE_FORMAT__ ((printf, 3, 0))
		__ATTRIBUTE_NONNULL__ ((3));

__END_DECLS


/*
 * Functions defined in POSIX 1003.1.
 */
#if __BSD_VISIBLE || __POSIX_VISIBLE || __XPG_VISIBLE

__BEGIN_DECLS
BFILE	*bfdopen(int, const char *);
int	 bfileno(BFILE *);

#if (__POSIX_VISIBLE >= 199506)
void	 bflockfile(BFILE *);
int	 bftrylockfile(BFILE *);
void	 bfunlockfile(BFILE *);

/*
 * These are normally used through macros as defined below, but POSIX
 * requires functions as well.
 */
int	 bgetc_unlocked(BFILE *);
int	 bgetchar_unlocked(void);
int	 bputc_unlocked(int, BFILE *);
//	int	 bputchar_unlocked(int);
#endif /* __POSIX_VISIBLE >= 199506 */

__END_DECLS

#endif /* __BSD_VISIBLE || __POSIX_VISIBLE || __XPG_VISIBLE */

/*
 * Routines that are purely local.
 */
#if __BSD_VISIBLE
__BEGIN_DECLS
int	 basprintf(char **, const char *, ...)
		__ATTRIBUTE_FORMAT__ ((printf, 2, 3))
		__ATTRIBUTE_NONNULL__ ((2));
char	*bfgetln(BFILE *, size_t *);
int	 bfpurge(BFILE *);
int	 bgetw(BFILE *);
int	 bputw(int, BFILE *);
void	 bsetbuffer(BFILE *, char *, int);
int	 bsetlinebuf(BFILE *);
int	 bvasprintf(char **, const char *, __va_list)
			__ATTRIBUTE_FORMAT__ ((printf, 2, 0))
			__ATTRIBUTE_NONNULL__ ((2));
__END_DECLS

/*
 * Stdio function-access interface.
 */
__BEGIN_DECLS
BFILE	*bfunopen(const void *,
		int (*)(void *, char *, int),
		int (*)(void *, const char *, int),
		bfpos_t (*)(void *, bfpos_t, int),
		int (*)(void *));
__END_DECLS

#define bfropen(cookie, fn) bfunopen(cookie, fn, 0, 0, 0)
#define bfwopen(cookie, fn) bfunopen(cookie, 0, fn, 0, 0)
#endif /* __BSD_VISIBLE */

/*
 * Functions internal to the implementation.
 */
__BEGIN_DECLS

int	__Bsrget(BFILE *);
int	__Bswbuf(int, BFILE *);

__END_DECLS

/*
 * The __sfoo macros are here so that we can
 * define function versions in the C library.
 */
#if defined(__BSTDIO_INLINE)

#define __Bsgetc(p)		(--(p)->_r < 0 ? __Bsrget(p) : (int)(*(p)->_p++))
#if defined(__BSD_VISIBLE)
#if defined(__GNUC__)
static __inline int __Bsputc(int _c, BFILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__Bswbuf(_c, _p));
}
#else
#define __Bsputc(c, p) \
	(--(p)->_w < 0 ? \
		(p)->_w >= (p)->_lbfsize ? \
			(*(p)->_p = (c)), *(p)->_p != '\n' ? \
				(int)*(p)->_p++ : \
				__Bswbuf('\n', p) : \
			__Bswbuf((int)(c), p) : \
		(*(p)->_p = (c), (int)*(p)->_p++))
#endif
#endif /*__BSD_VISIBLE*/

#define __Bsfeof(p)		(((p)->_flags & __BSEOF) != 0)
#define __Bsferror(p)		(((p)->_flags & __BSERR) != 0)
#define __Bsclearerr(p)		((void)((p)->_flags &= ~(__BSERR|__BSEOF)))
#define __Bsfileno(p)		((p)->_file)

#if defined(__BSTDIO_INTERNAL)
#undef  __sfeof
#undef  __sferror
#undef  __sclearerr
#undef  __sfileno

#define __sfeof(p)		__Bsfeof(p)
#define __sferror(p)		__Bsferror(p)
#define __sclearerr(p)		__Bsclearerr(p)
#define __sfileno(p)		__Bsfileno(p)
#endif  /*__BSTDIO_INTERNAL*/

extern int __Bisthreaded;

#define bfeof(p)		(!__Bisthreaded ? __Bsfeof(p) : (bfeof)(p))
#define bferror(p)		(!__Bisthreaded ? __Bsferror(p) : (bferror)(p))
#define bclearerr(p)		(!__Bisthreaded ? __Bsclearerr(p) : (bclearerr)(p))

#if __POSIX_VISIBLE
#define bfileno(p)		(!__Bisthreaded ? __Bsfileno(p) : (bfileno)(p))
#endif

#define bgetc(fp)		(!__Bisthreaded ? __Bsgetc(fp) : (bgetc)(fp))

#if __BSD_VISIBLE
/*
 * The macro implementations of putc and putc_unlocked are not
 * fully POSIX compliant; they do not set errno on failure
 */
#define bputc(x, fp)		(!__Bisthreaded ? __Bsputc(x, fp) : (bputc)(x, fp))
#endif /* __BSD_VISIBLE */

#ifndef lint
#if (__POSIX_VISIBLE >= 199506)
#define bgetc_unlocked(fp)	__Bsgetc(fp)
/*
 * The macro implementations of putc and putc_unlocked are not
 * fully POSIX compliant; they do not set errno on failure
 */
#if __BSD_VISIBLE
#define bputc_unlocked(x, fp)	__Bsputc(x, fp)
#endif /* __BSD_VISIBLE */
#endif /* __POSIX_VISIBLE >= 199506 */
#endif /* lint */

#endif /* __BSTDIO_INLINE */

#endif /*GR_BSTDIO_H_INCLUDED*/
