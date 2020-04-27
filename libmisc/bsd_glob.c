#include <edidentifier.h>
__CIDENT_RCSID(gr_bsd_glob_c,"$Id: bsd_glob.c,v 1.6 2020/04/11 21:36:46 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1989, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Guido van Rossum.
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

/*
 * glob(3) -- a superset of the one defined in POSIX 1003.2.
 *
 * The [!...] convention to negate a range is supported (SysV, Posix, ksh).
 *
 * Optional extra services, controlled by flags not defined by POSIX:
 *
 * GLOB_QUOTE:
 *      Escaping convention: \ inhibits any special meaning the following
 *      character might have (except \ at end of string is retained).
 * GLOB_MAGCHAR:
 *      Set in gl_flags if pattern contained a globbing character.
 * GLOB_NOMAGIC:
 *      Same as GLOB_NOCHECK, but it will only append pattern if it did
 *      not contain any magic characters.  [Used in csh style globbing]
 * GLOB_ALTDIRFUNC:
 *      Use alternately specified directory access functions.
 * GLOB_TILDE:
 *      expand ~user/foo to the /home/dir/of/user/foo
 * GLOB_BRACE:
 *      expand {1,2}{a,b} to 1a 1b 2a 2b
 * gl_matchc:
 *      Number of matches in the current invocation of glob.
 *
 * $OpenBSD: glob.c,v 1.36 2011/05/12 07:15:10 pyr Exp$
 */

#include <config.h>
#include <editor.h>
#include <edtypes.h>
#include <eddir.h>
#include <libstr.h>
#if defined(WIN32)
#include <win32_io.h>
#endif
#include "bsd_glob.h"

#ifdef HAVE_PWD_H
#include <pwd.h>
#else
#ifdef HAVE_PASSWD
__CBEGIN_DECLS
extern struct passwd *  getpwnam(const char *);
extern struct passwd *  getpwuid(uid_t);
__CEND_DECLS
#endif
#endif

#ifndef MAXPATHLEN
#ifdef PATH_MAX
#define MAXPATHLEN	PATH_MAX
#ifdef MACOS_TRADITIONAL
#define MAXPATHLEN	255
#else
#define MAXPATHLEN	1024
#endif
#endif
#endif

#include "charclass.h"

#define	DOLLAR		'$'
#define	DOT		'.'
#define	EOS		'\0'
#define	LBRACKET	'['
#define	NOT		'!'
#define	QUESTION	'?'
#define	QUOTE		'\\'
#define	RANGE		'-'
#define	RBRACKET	']'
#define	SEP		'/'
#ifdef DOSISH
#define SEP2		'\\'
#endif
#define	STAR		'*'
#define	TILDE		'~'
#define	UNDERSCORE	'_'
#define	LBRACE		'{'
#define	RBRACE		'}'
#define	SLASH		'/'
#define	COMMA		','

#ifndef DEBUG
#define	M_QUOTE		0x8000
#define	M_PROTECT	0x4000
#define	M_MASK		0xffff
#define	M_ASCII		0x00ff

typedef unsigned short Char;

#else
#define	M_QUOTE		0x80
#define	M_PROTECT	0x40
#define	M_MASK		0xff
#define	M_ASCII		0x7f

typedef char Char;
#endif

#define	CHAR(c)		((Char)((c)&M_ASCII))
#define	META(c)		((Char)((c)|M_QUOTE))
#define	M_ALL		META('*')
#define	M_END		META(']')
#define	M_NOT		META('!')
#define	M_ONE		META('?')
#define	M_RNG		META('-')
#define	M_SET		META('[')
#define	M_CLASS		META(':')
#define	ismeta(c)	(((c)&M_QUOTE) != 0)

#define	GLOB_LIMIT_MALLOC	65536
#define	GLOB_LIMIT_STAT		128
#define	GLOB_LIMIT_READDIR	16384

struct glob_lim {
	size_t		glim_malloc;
	size_t		glim_stat;
	size_t		glim_readdir;
};

static int		compare(const void *, const void *);
static int		g_Ctoc(const Char *, char *, u_int);
static int		g_lstat(Char *, struct stat *, glob_t *);
static void *	        g_opendir(Char *, glob_t *);
static struct glob_dirent *g_readdir(void *dirp, struct glob_dirent *entry);
static Char	*	g_strchr(const Char *, int);
static int		g_strncmp(const Char *, const char *, size_t);
static int		g_stat(Char *, struct stat *, glob_t *);
static int		glob0(const Char *, glob_t *, struct glob_lim *);
static int		glob1(Char *, Char *, glob_t *, struct glob_lim *);
static int		glob2(Char *, Char *, Char *, Char *, Char *, Char *, glob_t *, struct glob_lim *);
static int		glob3(Char *, Char *, Char *, Char *, Char *, Char *, Char *, glob_t *, struct glob_lim *);
static int		globextend(const Char *, glob_t *, struct glob_lim *, struct stat *);
static const Char *	globtilde(const Char *, Char *, size_t, glob_t *);
static int		globexp1(const Char *, glob_t *, struct glob_lim *);
static int		globexp2(const Char *, const Char *, glob_t *, struct glob_lim *);
static int		match(Char *, Char *, Char *);
#ifdef DEBUG
static void		qprintf(const char *, Char *);
#endif

int
bsd_glob(const char *pattern, int flags, int (*errfunc)(const char *, int), glob_t *pglob)
{
	const u_char *patnext;
	int c;
	Char *bufnext, *bufend, patbuf[MAXPATHLEN];
	struct glob_lim limit = { 0, 0, 0 };

	patnext = (u_char *) pattern;
	if (!(flags & GLOB_APPEND)) {
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		pglob->gl_statv = NULL;
		if (!(flags & GLOB_DOOFFS))
			pglob->gl_offs = 0;
	}
	pglob->gl_flags = flags & ~GLOB_MAGCHAR;
	pglob->gl_errfunc = errfunc;
	pglob->gl_matchc = 0;

	if (pglob->gl_offs < 0 || pglob->gl_pathc < 0 ||
			pglob->gl_offs >= INT_MAX || pglob->gl_pathc >= INT_MAX ||
			pglob->gl_pathc >= INT_MAX - pglob->gl_offs - 1) {
		return GLOB_NOSPACE;
	}

	bufnext = patbuf;
	bufend = bufnext + MAXPATHLEN - 1;
#ifdef DOSISH
	/* Treat patterns like "C:*" correctly. In this case, the '*' should match any 
	 * file in the current directory on the C: drive. However, the glob code does 
	 * not treat the colon specially, so it looks for files beginning "C:" in
	 * the current directory. To fix this, change the pattern to add an explicit 
	 * "./" at the start (just after the drive letter and colon 
	 * ie change to "C:./ *").
	 */
	if (pattern[1] == ':' && isalpha((unsigned char)pattern[0]) &&
			pattern[2] != SEP && pattern[2] != SEP2 && bufend - bufnext > 4) {
		*bufnext++ = pattern[0];
		*bufnext++ = ':';
		*bufnext++ = '.';
		*bufnext++ = SEP;
		patnext += 2;
	}
#endif
	if (flags & GLOB_NOESCAPE) {
		while (bufnext < bufend && (c = *patnext++) != EOS) {
			*bufnext++ = (Char)c;
		}
	} else {
		/* Protect the quoted characters. */
		while (bufnext < bufend && (c = *patnext++) != EOS)
			if (c == QUOTE) {
#ifdef DOSISH
				/* To avoid backslashitis on Win32, we only treat \ as a quoting 
				 * character if it precedes one of the metacharacters []-{}~\
				 */
				if ((c = *patnext++) != '[' && c != ']' &&
					c != '-' && c != '{' && c != '}' &&
					c != '~' && c != '\\') {
#else
				if ((c = *patnext++) == EOS) {
#endif
					c = QUOTE;
					--patnext;
				}
				*bufnext++ = (Char)(c | M_PROTECT);
			} else {
				*bufnext++ = (Char)c;
			}
	}
	*bufnext = EOS;

	if (flags & GLOB_BRACE)
		return globexp1(patbuf, pglob, &limit);
	else
		return glob0(patbuf, pglob, &limit);
}

/*
 * Expand recursively a glob {} pattern. When there is no more expansion
 * invoke the standard globbing routine to glob the rest of the magic
 * characters
 */
static int
globexp1(const Char *pattern, glob_t *pglob, struct glob_lim *limitp)
{
	const Char* ptr = pattern;

	/* Protect a single {}, for find(1), like csh */
	if (pattern[0] == LBRACE && pattern[1] == RBRACE && pattern[2] == EOS)
		return glob0(pattern, pglob, limitp);

	if ((ptr = (const Char *) g_strchr(ptr, LBRACE)) != NULL)
		return globexp2(ptr, pattern, pglob, limitp);

	return glob0(pattern, pglob, limitp);
}


/*
 * Recursive brace globbing helper. Tries to expand a single brace.
 * If it succeeds then it invokes globexp1 with the new pattern.
 * If it fails then it tries to glob the rest of the pattern and returns.
 */
static int
globexp2(const Char *ptr, const Char *pattern, glob_t *pglob,
    struct glob_lim *limitp)
{
	int     i, rv;
	Char   *lm, *ls;
	const Char *pe, *pm, *pl;
	Char    patbuf[MAXPATHLEN];

	/* copy part up to the brace */
	for (lm = patbuf, pm = pattern; pm != ptr; *lm++ = *pm++)
		;
	*lm = EOS;
	ls = lm;

	/* Find the balanced brace */
	for (i = 0, pe = ++ptr; *pe; pe++)
		if (*pe == LBRACKET) {
			/* Ignore everything between [] */
			for (pm = pe++; *pe != RBRACKET && *pe != EOS; pe++)
				;
			if (*pe == EOS) {
				/*
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pe = pm;
			}
		} else if (*pe == LBRACE)
			i++;
		else if (*pe == RBRACE) {
			if (i == 0)
				break;
			i--;
		}

	/* Non matching braces; just glob the pattern */
	if (i != 0 || *pe == EOS)
		return glob0(patbuf, pglob, limitp);

	for (i = 0, pl = pm = ptr; pm <= pe; pm++) {
		switch (*pm) {
		case LBRACKET:
			/* Ignore everything between [] */
			for (pl = pm++; *pm != RBRACKET && *pm != EOS; pm++)
				;
			if (*pm == EOS) {
				/*
				 * We could not find a matching RBRACKET.
				 * Ignore and just look for RBRACE
				 */
				pm = pl;
			}
			break;

		case LBRACE:
			i++;
			break;

		case RBRACE:
			if (i) {
				i--;
				break;
			}
			/* FALLTHROUGH */
		case COMMA:
			if (i && *pm == COMMA)
				break;
			else {
				/* Append the current string */
				for (lm = ls; (pl < pm); *lm++ = *pl++)
					;

				/*
				 * Append the rest of the pattern after the
				 * closing brace
				 */
				for (pl = pe + 1; (*lm++ = *pl++) != EOS; )
					;

				/* Expand the current pattern */
#ifdef DEBUG
				qprintf("globexp2:", patbuf);
#endif
				rv = globexp1(patbuf, pglob, limitp);
				if (rv && rv != GLOB_NOMATCH)
					return rv;

				/* move after the comma, to the next string */
				pl = pm + 1;
			}
			break;

		default:
			break;
		}
	}
	return 0;
}


/*
 * expand tilde from the passwd file.
 */
static const Char *
globtilde(const Char *pattern, Char *patbuf, size_t patbuf_len, glob_t *pglob)
{
#ifdef HAVE_PASSWD
	struct passwd *pwd;
#endif
	char *h;
	const Char *p;
	Char *b, *eb;

	if (*pattern != TILDE || !(pglob->gl_flags & GLOB_TILDE))
		return pattern;

	/* Copy up to the end of the string or / */
	eb = &patbuf[patbuf_len - 1];
	for (p = pattern + 1, h = (char *) patbuf;
			h < (char *)eb && *p && *p != SLASH; *h++ = (char)*p++)
		continue;
	*h = EOS;

	if (((char *) patbuf)[0] == EOS) {
		/*
		 * handle a plain ~ or ~/ by expanding $HOME
		 * first and then trying the password file
		 */
#if defined(DOSISH) || !defined(HAVE_ISSETGUID)
		const int tainted = 0;
#else
		const int tainted = issetugid();
#endif

		if (tainted != 0 || (h = getenv("HOME")) == NULL) {
#ifdef HAVE_PASSWD
			if ((pwd = getpwuid(getuid())) == NULL) {
				return pattern;
			} else {
				h = pwd->pw_dir;
			}
#else
			return pattern;
#endif
		}
	} else {
		/*
		 * Expand a ~user
		 */
#ifdef HAVE_PASSWD
		if ((pwd = getpwnam((char*) patbuf)) == NULL) {
			return pattern;
		} else {
			h = pwd->pw_dir;
		}
#else
		return pattern;
#endif
	}

	/* Copy the home directory */
	for (b = patbuf; b < eb && *h; *b++ = *h++)
		;

	/* Append the rest of the pattern */
	while (b < eb && (*b++ = *p++) != EOS)
		;
	*b = EOS;

	return patbuf;
}

static int
g_strncmp(const Char *s1, const char *s2, size_t n)
{
	int rv = 0;

	while (n--) {
		rv = *(Char *)s1 - *(const unsigned char *)s2++;
		if (rv)
			break;
		if (*s1++ == '\0')
			break;
	}
	return rv;
}

static int
g_charclass(const Char **patternp, Char **bufnextp)
{
	const Char *pattern = *patternp + 1;
	Char *bufnext = *bufnextp;
	const Char *colon;
	struct cclass *cc;
	size_t len;

	if ((colon = g_strchr(pattern, ':')) == NULL || colon[1] != ']')
		return 1;	/* not a character class */

	len = (size_t)(colon - pattern);
	for (cc = cclasses; cc->name != NULL; cc++) {
		if (!g_strncmp(pattern, cc->name, len) && cc->name[len] == '\0')
			break;
	}
	if (cc->name == NULL)
		return -1;	/* invalid character class */
	*bufnext++ = M_CLASS;
	*bufnext++ = (Char)(cc - &cclasses[0]);
	*bufnextp = bufnext;
	*patternp += len + 3;

	return 0;
}

/*
 * The main glob() routine: compiles the pattern (optionally processing
 * quotes), calls glob1() to do the real pattern matching, and finally
 * sorts the list (unless unsorted operation is requested).  Returns 0
 * if things went well, nonzero if errors occurred.  It is not an error
 * to find no matches.
 */
static int
glob0(const Char *pattern, glob_t *pglob, struct glob_lim *limitp)
{
	const Char *qpatnext;
	int c, err, oldpathc;
	Char *bufnext, patbuf[MAXPATHLEN];

	qpatnext = globtilde(pattern, patbuf, MAXPATHLEN, pglob);
	oldpathc = pglob->gl_pathc;
	bufnext = patbuf;

	/* We don't need to check for buffer overflow any more. */
	while ((c = *qpatnext++) != EOS) {
		switch (c) {
		case LBRACKET:
			c = *qpatnext;
			if (c == NOT)
				++qpatnext;
			if (*qpatnext == EOS ||
			    g_strchr(qpatnext+1, RBRACKET) == NULL) {
				*bufnext++ = LBRACKET;
				if (c == NOT)
					--qpatnext;
				break;
			}
			*bufnext++ = M_SET;
			if (c == NOT)
				*bufnext++ = M_NOT;
			c = *qpatnext++;
			do {
				if (c == LBRACKET && *qpatnext == ':') {
					do {
						err = g_charclass(&qpatnext,
						    &bufnext);
						if (err)
							break;
						c = *qpatnext++;
					} while (c == LBRACKET && *qpatnext == ':');
					if (err == -1 &&
					    !(pglob->gl_flags & GLOB_NOCHECK))
						return GLOB_NOMATCH;
					if (c == RBRACKET)
						break;
				}
				*bufnext++ = CHAR(c);
				if (*qpatnext == RANGE &&
				    (c = qpatnext[1]) != RBRACKET) {
					*bufnext++ = M_RNG;
					*bufnext++ = CHAR(c);
					qpatnext += 2;
				}
			} while ((c = *qpatnext++) != RBRACKET);
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_END;
			break;
		case QUESTION:
			pglob->gl_flags |= GLOB_MAGCHAR;
			*bufnext++ = M_ONE;
			break;
		case STAR:
			pglob->gl_flags |= GLOB_MAGCHAR;
			/* collapse adjacent stars to one,
			 * to avoid exponential behavior
			 */
			if (bufnext == patbuf || bufnext[-1] != M_ALL)
				*bufnext++ = M_ALL;
			break;
		default:
			*bufnext++ = CHAR(c);
			break;
		}
	}
	*bufnext = EOS;
#ifdef DEBUG
	qprintf("glob0:", patbuf);
#endif

	if ((err = glob1(patbuf, patbuf+MAXPATHLEN-1, pglob, limitp)) != 0)
		return(err);

	/*
	 * If there was no match we are going to append the pattern
	 * if GLOB_NOCHECK was specified or if GLOB_NOMAGIC was specified
	 * and the pattern did not contain any magic characters
	 * GLOB_NOMAGIC is there just for compatibility with csh.
	 */
	if (pglob->gl_pathc == oldpathc) {
		if ((pglob->gl_flags & GLOB_NOCHECK) ||
		    ((pglob->gl_flags & GLOB_NOMAGIC) &&
		    !(pglob->gl_flags & GLOB_MAGCHAR)))
			return(globextend(pattern, pglob, limitp, NULL));
		else
			return(GLOB_NOMATCH);
	}
	if (!(pglob->gl_flags & GLOB_NOSORT))
		qsort(pglob->gl_pathv + pglob->gl_offs + oldpathc,
		    pglob->gl_pathc - oldpathc, sizeof(char *), compare);
	return(0);
}

static int
compare(const void *p, const void *q)
{
	return(strcmp(*(char **)p, *(char **)q));
}

static int
glob1(Char *pattern, Char *pattern_last, glob_t *pglob, struct glob_lim *limitp)
{
	Char pathbuf[MAXPATHLEN];

	/* A null pathname is invalid -- POSIX 1003.1 sect. 2.4. */
	if (*pattern == EOS)
		return(0);
	return(glob2(pathbuf, pathbuf+MAXPATHLEN-1,
	    pathbuf, pathbuf+MAXPATHLEN-1,
	    pattern, pattern_last, pglob, limitp));
}

/*
 * The functions glob2 and glob3 are mutually recursive; there is one level
 * of recursion for each segment in the pattern that contains one or more
 * meta characters.
 */
static int
glob2(Char *pathbuf, Char *pathbuf_last, Char *pathend, Char *pathend_last,
    Char *pattern, Char *pattern_last, glob_t *pglob, struct glob_lim *limitp)
{
	struct stat sb;
	Char *p, *q;
	int anymeta;

	/*
	 * Loop over pattern segments until end of pattern or until
	 * segment with meta character found.
	 */
	for (anymeta = 0;;) {
		if (*pattern == EOS) {		/* End of pattern? */
			*pathend = EOS;
			if (g_lstat(pathbuf, &sb, pglob))
				return(0);

			if ((pglob->gl_flags & GLOB_LIMIT) &&
					limitp->glim_stat++ >= GLOB_LIMIT_STAT) {
				errno = 0;
				*pathend++ = SEP;
				*pathend = EOS;
				return(GLOB_NOSPACE);
			}

			if (((pglob->gl_flags & GLOB_MARK) &&
#ifdef DOSISH
				(SEP != pathend[-1] && SEP2 != pathend[-1])
#else
				pathend[-1] != SEP
#endif
			    ) && 
                            (S_ISDIR(sb.st_mode)
#if defined(S_ISLNK)
			        || (S_ISLNK(sb.st_mode) && 
                                        (g_stat(pathbuf, &sb, pglob) == 0) && S_ISDIR(sb.st_mode))
#endif
                            )) {
				if (pathend+1 > pathend_last)
					return (1);
				*pathend++ = SEP;
				*pathend = EOS;
			}
			++pglob->gl_matchc;
			return(globextend(pathbuf, pglob, limitp, &sb));
		}

		/* Find end of next segment, copy tentatively to pathend. */
		q = pathend;
		p = pattern;
#ifdef DOSISH
		while (*p != EOS && *p != SEP && *p != SEP2) {
#else
		while (*p != EOS && *p != SEP) {
#endif
			if (ismeta(*p))
				anymeta = 1;
			if (q+1 > pathend_last)
				return (1);
			*q++ = *p++;
		}

		if (!anymeta) {		/* No expansion, do next segment. */
			pathend = q;
			pattern = p;
#ifdef DOSISH
			while (*pattern == SEP || *pattern == SEP2) {
#else
			while (*pattern == SEP) {
#endif
				if (pathend+1 > pathend_last) {
					return (1);
				}
				*pathend++ = *pattern++;
			}
		} else
			/* Need expansion, recurse. */
			return(glob3(pathbuf, pathbuf_last, pathend,
			    pathend_last, pattern, p, pattern_last,
			    pglob, limitp));
	}
	/* NOTREACHED */
}

static int
glob3(Char *pathbuf, Char *pathbuf_last, Char *pathend, Char *pathend_last,
    Char *pattern, Char *restpattern, Char *restpattern_last, glob_t *pglob,
    struct glob_lim *limitp)
{
        struct glob_dirent dirententry, *dp;
	void *dirp;
	int err;
	char buf[MAXPATHLEN];

	/*
	 * The readdirfunc declaration can't be prototyped, because it is
	 * assigned, below, to two functions which are prototyped in glob.h
	 * and dirent.h as taking pointers to differently typed opaque
	 * structures.
	 */
        struct glob_dirent *(*readdirfunc)(void *, struct glob_dirent *);

	if (pathend > pathend_last)
		return (1);
	*pathend = EOS;
	errno = 0;

	if ((dirp = g_opendir(pathbuf, pglob)) == NULL) {
		/* TODO: don't call for ENOENT or ENOTDIR? */
		if (pglob->gl_errfunc) {
			if (g_Ctoc(pathbuf, buf, sizeof(buf)))
				return(GLOB_ABORTED);
			if (pglob->gl_errfunc(buf, errno) ||
			    pglob->gl_flags & GLOB_ERR)
				return(GLOB_ABORTED);
		}
		return(0);
	}

	err = 0;

	/* Search directory for matching names. */
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
	        readdirfunc = pglob->gl_readdir;
	else
	        readdirfunc = g_readdir;
	while ((dp = (*readdirfunc)(dirp, &dirententry))) {
		u_char *sc;
		Char *dc;

		if ((pglob->gl_flags & GLOB_LIMIT) &&
				limitp->glim_readdir++ >= GLOB_LIMIT_READDIR) {
			errno = 0;
			*pathend++ = SEP;
			*pathend = EOS;
			err = GLOB_NOSPACE;
			break;
		}

		/* Initial DOT must be matched literally. */
		if (dp->d_name[0] == DOT && *pattern != DOT)
			continue;
		dc = pathend;
		sc = (u_char *) dp->d_name;
		while (dc < pathend_last && (*dc++ = *sc++) != EOS)
			;
		if (dc >= pathend_last) {
			*dc = EOS;
			err = 1;
			break;
		}

		if (!match(pathend, pattern, restpattern)) {
			*pathend = EOS;
			continue;
		}
		err = glob2(pathbuf, pathbuf_last, --dc, pathend_last,
		                restpattern, restpattern_last, pglob, limitp);
		if (err)
			break;
	}

	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		(*pglob->gl_closedir)(dirp);
	else
		closedir((DIR *)dirp);
	return(err);
}


/*
 * Extend the gl_pathv member of a glob_t structure to accommodate a new item,
 * add the new item, and update gl_pathc.
 *
 * This assumes the BSD realloc, which only copies the block when its size
 * crosses a power-of-two boundary; for v7 realloc, this would cause quadratic
 * behavior.
 *
 * Return 0 if new item added, error code if memory couldn't be allocated.
 *
 * Invariant of the glob_t structure:
 *	Either gl_pathc is zero and gl_pathv is NULL; or gl_pathc > 0 and
 *	gl_pathv points to (gl_offs + gl_pathc + 1) items.
 */
static int
globextend(const Char *path, glob_t *pglob, struct glob_lim *limitp, struct stat *sb)
{
	char **pathv;
	ssize_t i;
	size_t newn, len;
	char *copy = NULL;
	const Char *p;
	struct stat **statv;

#if !defined(SIZE_MAX)
#define SIZE_MAX        INT_MAX
#endif
	newn = 2 + pglob->gl_pathc + pglob->gl_offs;
	if (pglob->gl_offs >= INT_MAX ||
	    pglob->gl_pathc >= INT_MAX ||
	    newn >= INT_MAX ||
	    SIZE_MAX / sizeof(*pathv) <= newn ||
	    SIZE_MAX / sizeof(*statv) <= newn) {
 nospace:
		for (i = pglob->gl_offs; i < (ssize_t)(newn - 2); i++) {
			if (pglob->gl_pathv && pglob->gl_pathv[i])
				free(pglob->gl_pathv[i]);
			if ((pglob->gl_flags & GLOB_KEEPSTAT) != 0 &&
			    pglob->gl_pathv && pglob->gl_pathv[i])
				free(pglob->gl_statv[i]);
		}
		if (pglob->gl_pathv) {
			free(pglob->gl_pathv);
			pglob->gl_pathv = NULL;
		}
		if (pglob->gl_statv) {
			free(pglob->gl_statv);
			pglob->gl_statv = NULL;
		}
		return(GLOB_NOSPACE);
	}

	pathv = realloc(pglob->gl_pathv, newn * sizeof(*pathv));
	if (pathv == NULL)
		goto nospace;
	if (pglob->gl_pathv == NULL && pglob->gl_offs > 0) {
		/* first time around -- clear initial gl_offs items */
		pathv += pglob->gl_offs;
		for (i = pglob->gl_offs; --i >= 0; )
			*--pathv = NULL;
	}
	pglob->gl_pathv = pathv;

	if ((pglob->gl_flags & GLOB_KEEPSTAT) != 0) {
		statv = realloc(pglob->gl_statv, newn * sizeof(*statv));
		if (statv == NULL)
			goto nospace;
		if (pglob->gl_statv == NULL && pglob->gl_offs > 0) {
			/* first time around -- clear initial gl_offs items */
			statv += pglob->gl_offs;
			for (i = pglob->gl_offs; --i >= 0; )
				*--statv = NULL;
		}
		pglob->gl_statv = statv;
		if (sb == NULL)
			statv[pglob->gl_offs + pglob->gl_pathc] = NULL;
		else {
			limitp->glim_malloc += sizeof(**statv);
			if ((pglob->gl_flags & GLOB_LIMIT) &&
			    limitp->glim_malloc >= GLOB_LIMIT_MALLOC) {
				errno = 0;
				return(GLOB_NOSPACE);
			}
			if ((statv[pglob->gl_offs + pglob->gl_pathc] = malloc(sizeof(**statv))) == NULL)
				goto copy_error;
			memcpy(statv[pglob->gl_offs + pglob->gl_pathc], sb, sizeof(*sb));
		}
		statv[pglob->gl_offs + pglob->gl_pathc + 1] = NULL;
	}

	for (p = path; *p++;)
		;
	len = (size_t)(p - path);
	limitp->glim_malloc += len;
	if ((copy = malloc(len)) != NULL) {
		if (g_Ctoc(path, copy, len)) {
			free(copy);
			return(GLOB_NOSPACE);
		}
		pathv[pglob->gl_offs + pglob->gl_pathc++] = copy;
	}
	pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

	if ((pglob->gl_flags & GLOB_LIMIT) &&
	    (newn * sizeof(*pathv)) + limitp->glim_malloc >
	    GLOB_LIMIT_MALLOC) {
		errno = 0;
		return(GLOB_NOSPACE);
	}
 copy_error:
	return(copy == NULL ? GLOB_NOSPACE : 0);
}


/*
 * pattern matching function for filenames.  Each occurrence of the *
 * pattern causes a recursion level.
 */
static int
match(Char *name, Char *pat, Char *patend)
{
	int ok, negate_range;
	Char c, k;

	while (pat < patend) {
		c = *pat++;
		switch (c & M_MASK) {
		case M_ALL:
			if (pat == patend)
				return(1);
			do {
			    if (match(name, pat, patend))
				    return(1);
			} while (*name++ != EOS);
			return(0);
		case M_ONE:
			if (*name++ == EOS)
				return(0);
			break;
		case M_SET:
			ok = 0;
			if ((k = *name++) == EOS)
				return(0);
			if ((negate_range = ((*pat & M_MASK) == M_NOT)) != EOS)
				++pat;
			while (((c = *pat++) & M_MASK) != M_END) {
				if ((c & M_MASK) == M_CLASS) {
					Char idx = *pat & M_MASK;
					if (idx < NCCLASSES &&
					    cclasses[idx].isctype(k))
						ok = 1;
					++pat;
				}
				if ((*pat & M_MASK) == M_RNG) {
					if (c <= k && k <= pat[1])
						ok = 1;
					pat += 2;
				} else if (c == k)
					ok = 1;
			}
			if (ok == negate_range)
				return(0);
			break;
		default:
			if (*name++ != c)
				return(0);
			break;
		}
	}
	return(*name == EOS);
}

/* Free allocated data belonging to a glob_t structure. */
void
bsd_globfree(glob_t *pglob)
{
	int i;
	char **pp;

	if (pglob->gl_pathv != NULL) {
		pp = pglob->gl_pathv + pglob->gl_offs;
		for (i = pglob->gl_pathc; i--; ++pp)
			if (*pp)
				free(*pp);
		free(pglob->gl_pathv);
		pglob->gl_pathv = NULL;
	}
	if (pglob->gl_statv != NULL) {
		for (i = 0; i < pglob->gl_pathc; i++) {
			if (pglob->gl_statv[i] != NULL)
				free(pglob->gl_statv[i]);
		}
		free(pglob->gl_statv);
		pglob->gl_statv = NULL;
	}
}

static void *
g_opendir(Char *str, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (!*str)
		strxcpy(buf, ".", sizeof buf);          /*strlcpy()*/
	else {
		if (g_Ctoc(str, buf, sizeof(buf)))
			return(NULL);
	}

	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return ((*pglob->gl_opendir)(buf));

	return (void *)(opendir(buf));
}

static struct glob_dirent *
g_readdir(void *dirp, struct glob_dirent *entry)
{
        struct dirent *dent;

        if (NULL == (dent = readdir((DIR *)dirp))) {
                return NULL;
        }
        entry->d_name = dent->d_name;
        entry->d_user = dent;
        return entry;
}

static int
g_lstat(Char *fn, struct stat *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (g_Ctoc(fn, buf, sizeof(buf)))
		return(-1);
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return((*pglob->gl_lstat)(buf, sb));
#if defined(WIN32)
	return(w32_lstat(buf, sb));
#else
#ifdef HAVE_LSTAT
	return(lstat(buf, sb));
#else
	return(stat(buf, sb));
#endif
#endif
}

static int
g_stat(Char *fn, struct stat *sb, glob_t *pglob)
{
	char buf[MAXPATHLEN];

	if (g_Ctoc(fn, buf, sizeof(buf)))
		return(-1);
	if (pglob->gl_flags & GLOB_ALTDIRFUNC)
		return((*pglob->gl_stat)(buf, sb));
#if defined(WIN32)
        return(w32_stat(buf, sb));
#else
	return(stat(buf, sb));
#endif
}

static Char *
g_strchr(const Char *str, int ch)
{
	do {
		if (*str == ch) {
			return ((Char *)str);
		}
	} while (*str++);
	return (NULL);
}

static int
g_Ctoc(const Char *str, char *buf, u_int len)
{
	while (len--) {
		if ((*buf++ = (char) *str++) == EOS) {
			return (0);
		}
	}
	return (1);
}

#ifdef DEBUG
static void
qprintf(const char *str, Char *s)
{
	Char *p;

	(void)printf("%s:\n", str);
	for (p = s; *p; p++)
		(void)printf("%c", CHAR(*p));
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", *p & M_PROTECT ? '"' : ' ');
	(void)printf("\n");
	for (p = s; *p; p++)
		(void)printf("%c", ismeta(*p) ? '_' : ' ');
	(void)printf("\n");
}
#endif
/*end*/
