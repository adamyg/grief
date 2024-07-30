/*	$OpenBSD: wc.c,v 1.11 2005/10/19 21:49:02 espie Exp $	*/

/*
 * Copyright (c) 1980, 1987, 1991, 1993
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
 * 
 *  static char copyright[] =
 *  "@(#) Copyright (c) 1980, 1987, 1991, 1993\n\
 *       The Regents of the University of California.  All rights reserved.\n";
 * 
 */

#ifdef   HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef  _BSD_SOURCE
#define  _BSD_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fmt_scaled.h"
#include "err.h"

#ifndef MAXBSIZE
#define MAXBSIZE		1024
#endif

int64_t	tlinect, twordct, tcharct;
int	doline, doword, dochar, humanchar;
int 	rval;

const char *__progname		= "grwc";

void	print_counts(int64_t, int64_t, int64_t, char *);
void	cnt(char *);
void    format_and_print(long long v);


int
main(int argc, char *argv[])
{
	const char *progname;
	int ch;

	setlocale(LC_ALL, "");

	__progname = (NULL != (progname = strrchr(argv[0], '/')))
		|| (NULL != (progname = strrchr(argv[0], '\\'))) ? progname + 1 : argv[0];

	while ((ch = getopt(argc, argv, "lwchm")) != -1)
		switch((char)ch) {
		case 'l':
			doline = 1;
			break;
		case 'w':
			doword = 1;
			break;
		case 'c':
		case 'm':
			dochar = 1;
			break;
		case 'h':
			humanchar = 1;
			break;
		case '?':
		default:
			(void)fprintf(stderr,
			    "usage: %s [-c | -m] [-hlw] [file ...]\n",
			    __progname);
			exit(1);
		}
	argv += optind;
	argc -= optind;

	/*
	 * wc is unusual in that its flags are on by default, so,
	 * if you don't get any arguments, you have to turn them
	 * all on.
	 */
	if (!doline && !doword && !dochar)
		doline = doword = dochar = 1;

	if (!*argv) {
		cnt((char *)NULL);
	} else {
		int dototal = (argc > 1);

		do {
			cnt(*argv);
		} while(*++argv);

		if (dototal)
			print_counts(tlinect, twordct, tcharct, "total");
	}

	return rval;
}

void
cnt(char *file)
{
	u_char *C;
	short gotsp;
	int len;
	int64_t linect, wordct, charct;
	struct stat sbuf;
	int fd;
	u_char buf[MAXBSIZE];

	linect = wordct = charct = 0;
	if (file) {
		if ((fd = open(file, O_RDONLY, 0)) < 0) {
			warn("%s", file);
			rval = 1;
			return;
		}
	} else  {
		fd = STDIN_FILENO;
	}

	if (!doword) {
		/*
		 * Line counting is split out because it's a lot
		 * faster to get lines than to get words, since
		 * the word count requires some logic.
		 */
		if (doline) {
			while ((len = read(fd, buf, MAXBSIZE)) > 0) {
				charct += len;
				for (C = buf; len--; ++C)
					if (*C == '\n')
						++linect;
			}
			if (len == -1) {
				warn("%s", file);
				rval = 1;
			}
		}
		/*
		 * If all we need is the number of characters and
		 * it's a directory or a regular or linked file, just
		 * stat the puppy.  We avoid testing for it not being
		 * a special device in case someone adds a new type
		 * of inode.
		 */
		else if (dochar) {
			mode_t ifmt;

			if (fstat(fd, &sbuf)) {
				warn("%s", file);
				rval = 1;
			} else {
				ifmt = sbuf.st_mode & S_IFMT;
				if (ifmt == S_IFREG || ifmt == S_IFLNK
				    || ifmt == S_IFDIR) {
					charct = sbuf.st_size;
				} else {
					while ((len = read(fd, buf, MAXBSIZE)) > 0)
						charct += len;
					if (len == -1) {
						warn("%s", file);
						rval = 1;
					}
				}
			}
		}
	} else {
		/* Do it the hard way... */
		gotsp = 1;
		while ((len = read(fd, buf, MAXBSIZE)) > 0) {
			/*
			 * This loses in the presence of multi-byte characters.
			 * To do it right would require a function to return a
			 * character while knowing how many bytes it consumed.
			 */
			charct += len;
			for (C = buf; len--; ++C) {
				if (isspace (*C)) {
					gotsp = 1;
					if (*C == '\n')
						++linect;
				} else {
					/*
					 * This line implements the POSIX
					 * spec, i.e. a word is a "maximal
					 * string of characters delimited by
					 * whitespace."  Notice nothing was
					 * said about a character being
					 * printing or non-printing.
					 */
					if (gotsp) {
						gotsp = 0;
						++wordct;
					}
				}
			}
		}
		if (len == -1) {
			warn("%s", file);
			rval = 1;
		}
	}

	print_counts(linect, wordct, charct, file ? file : "");

	/*
	 * Don't bother checking doline, doword, or dochar -- speeds
	 * up the common case
	 */
	tlinect += linect;
	twordct += wordct;
	tcharct += charct;

	if (close(fd) != 0) {
		warn("%s", file);
		rval = 1;
	}
}

void 
format_and_print(long long v)
{
	if (humanchar) {
		char result[FMT_SCALED_STRSIZE];

		(void)fmt_scaled(v, result);
		(void)printf("%7s", result);
	} else {
		(void)printf(" %7lld", v);
	}
}

void
print_counts(int64_t lines, int64_t words, int64_t chars, char *name)
{
	if (doline)
		format_and_print((long long)lines);
	if (doword)
		format_and_print((long long)words);
	if (dochar)
		format_and_print((long long)chars);

	(void)printf(" %s\n", name);
}

/*end*/
