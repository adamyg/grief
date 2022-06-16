/*-
 * Copyright (c)2003 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  $NetBSD: iconv.c,v 1.18 2011/10/31 13:27:51 yamt Exp $
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: iconv.c,v 1.18 2011/10/31 13:27:51 yamt Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#include <errno.h>
#include <iconv.h>
#include <langinfo.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#if !defined(WIN32)
#include <util.h>
#endif

static void usage(void) __dead;
static int  scmp(const void *, const void *);
static void show_codesets(void);
static void do_conv(const char *, FILE *, const char *, const char *, int, int);
#if defined(WIN32)
static void help(void);
#endif

static void
usage(void)
{
	const char *prog = getprogname();
	(void)fprintf(stderr,
	    "Usage:\t%s [-cs] -f <from_code> -t <to_code> [file ...]\n"
	    "\t%s -f <from_code> [-cs] [-t <to_code>] [file ...]\n"
	    "\t%s -t <to_code> [-cs] [-f <from_code>] [file ...]\n"
	    "\t%s -l\n"
#if defined(WIN32)
	    "\t%s -h\n"
#endif
	    , prog, prog, prog, prog, prog);
	exit(1);
}


/*
 * qsort() helper function
 */
static int
scmp(const void *v1, const void *v2)
{
	const char * const *s1 = v1;
	const char * const *s2 = v2;

	return strcasecmp(*s1, *s2);
}


#if defined(WIN32)				/*DLL binding*/
static const char * local_nl_langinfo(int elm);
#define nl_langinfo(__a)        local_nl_langinfo(__a)

#define __ICONV_ERRNO()		errno = iconv_errno();
#else
#define __ICONV_ERRNO()
#endif

static void
show_codesets(void)
{
	char **list;
	size_t sz, i;

	if (__iconv_get_list(&list, &sz)) {
		__ICONV_ERRNO()
		err(EXIT_FAILURE, "__iconv_get_list()");
	}

	qsort(list, sz, sizeof(char *), scmp);

	for (i = 0; i < sz; i++)
		(void)printf("%s\n", list[i]);

	__iconv_free_list(list, sz);
}

#define INBUFSIZE		1024
#define OUTBUFSIZE		(INBUFSIZE * 2)
/*ARGSUSED*/
static void
do_conv(const char *fn, FILE *fp, const char *from, const char *to, int silent, int hide_invalid)
{
	char inbuf[INBUFSIZE], outbuf[OUTBUFSIZE], *out;
	const char *in;
	size_t inbytes, outbytes, ret, invalids;
	iconv_t cd;
	uint32_t flags = 0;

#ifndef __ICONV_F_HIDE_INVALID
#define __ICONV_F_HIDE_INVALID	0x0001	/*citrus_iconv.h*/
#endif

	if (hide_invalid)
		flags |= __ICONV_F_HIDE_INVALID;
	cd = iconv_open(to, from);
	if (cd == (iconv_t)-1) {
		__ICONV_ERRNO()
		err(EXIT_FAILURE, "iconv_open(%s, %s)", to, from);
	}

	invalids = 0;
	while ((inbytes = fread(inbuf, 1, INBUFSIZE, fp)) > 0) {
		in = inbuf;
		while (inbytes > 0) {
			size_t inval;

			out = outbuf;
			outbytes = OUTBUFSIZE;
			ret = __iconv(cd, &in, &inbytes, &out, &outbytes, flags, &inval);
			invalids += inval;
			if (outbytes < OUTBUFSIZE)
				(void)fwrite(outbuf, 1, OUTBUFSIZE - outbytes, stdout);
			if (ret == (size_t)-1) {
				__ICONV_ERRNO()
				if (errno != E2BIG) {
					/*
					 * XXX: iconv(3) is bad interface.
					 *   invalid character count is lost here.
					 *   instead, we just provide __iconv function.
					 */
					if (errno != EINVAL || in == inbuf)
						err(EXIT_FAILURE, "iconv()");

					/* incomplete input character */
					(void)memmove(inbuf, in, inbytes);
					ret = fread(inbuf + inbytes, 1, INBUFSIZE - inbytes, fp);
					if (ret == 0) {
						fflush(stdout);
						if (feof(fp))
							errx(EXIT_FAILURE,
							    "unexpected end of file; "
							    "the last character is "
							    "incomplete.");
						else
							err(EXIT_FAILURE, "fread()");
					}
					in = inbuf;
					inbytes += ret;
				}
			}
		}
	}
	/* reset the shift state of the output buffer */
	outbytes = OUTBUFSIZE;
	out = outbuf;
	ret = iconv(cd, NULL, NULL, &out, &outbytes);
	if (ret == (size_t)-1) {
		__ICONV_ERRNO()
		err(EXIT_FAILURE, "iconv()");
	}
	if (outbytes < OUTBUFSIZE)
		(void)fwrite(outbuf, 1, OUTBUFSIZE - outbytes, stdout);

	if (invalids > 0 && !silent)
		warnx("warning: invalid characters: %lu", (unsigned long)invalids);

	iconv_close(cd);
}

int
main(int argc, char **argv)
{
	int ch, i;
	int opt_l = 0, opt_s = 0, opt_c = 0;
	const char *opt_f = NULL, *opt_t = NULL;
	FILE *fp;

	setlocale(LC_ALL, "");
	setprogname(argv[0]);

#if defined(WIN32)
	while ((ch = getopt(argc, argv, "cslf:t:h")) != EOF) {
#else
	while ((ch = getopt(argc, argv, "cslf:t:")) != EOF) {
#endif
		switch (ch) {
		case 'c':
			opt_c = 1;
			break;
		case 's':
			opt_s = 1;
			break;
		case 'l': /* list */
			opt_l = 1;
			break;
		case 'f': /* from */
			opt_f = strdup(optarg);
			break;
		case 't': /* to */
			opt_t = strdup(optarg);
			break;
#if defined(WIN32)
		case 'h':
			help();
                        break;
#endif
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (opt_l) {
		if (argc > 0 || opt_s || opt_f != NULL || opt_t != NULL) {
			warnx("-l is not allowed with other flags.");
			usage();
		}
		show_codesets();
	} else {
		if (opt_f == NULL) {
			if (opt_t == NULL)
				usage();
			opt_f = nl_langinfo(CODESET);
		} else if (opt_t == NULL)
			opt_t = nl_langinfo(CODESET);

		if (argc == 0)
			do_conv("<stdin>", stdin, opt_f, opt_t, opt_s, opt_c);
		else {
			for (i = 0; i < argc; i++) {
				fp = fopen(argv[i], "r");
				if (fp == NULL)
					err(EXIT_FAILURE, "Cannot open `%s'", argv[i]);
				do_conv(argv[i], fp, opt_f, opt_t, opt_s, opt_c);
				(void)fclose(fp);
			}
		}
	}
	return EXIT_SUCCESS;
}


#if defined(WIN32)
static const char *
local_nl_langinfo(int elm) 
{
	if (CODESET == elm)
		return "UTF-8";		/*TODO*/
	return "";
}


static void
help(void)
{
	static const char *helptext[] = {
	    "",
	    "NAME",
	    "     iconv -- codeset conversion utility",
	    "",
	    "SYNOPSIS",
	    "     iconv [-cs] -f from_name -t to_name [file ...]",
	    "     iconv -f from_name [-cs] [-t to_name] [file ...]",
	    "     iconv -t to_name [-cs] [-f from_name] [file ...]",
	    "     iconv -l",
	    "     iconv -h",
	    "",
	    "DESCRIPTION",
	    "     The iconv utility converts the codeset of file (or from standard input if",
	    "     no file is specified) from codeset from_name to codeset to_name and out-",
	    "     puts the converted text on standard output.",
	    "",
	    "     The following options are available:",
	    "",
	    "     -c    Prevent output of any invalid characters.  By default, iconv out-",
	    "           puts an ``invalid character'' specified by the to_name codeset when",
	    "           it encounters a character which is valid in the from_name codeset",
	    "           but does not have a corresponding character in the to_name codeset.",
	    "",
	    "     -f    Specifies the source codeset name as from_name.",
	    "",
	    "     -l    Lists available codeset names.  Note that not all combinations of",
	    "           from_name and to_name are valid.",
	    "",
	    "     -s    Silent.  By default, iconv outputs the number of ``invalid",
	    "           characters'' to standard error if they exist.  This option prevents",
	    "           this behaviour.",
	    "",
	    "     -t    Specifies the destination codeset name as to_name.",
	    "",
	    "     -h    Command line usage/help.",
	    "",
	    "EXIT STATUS",
	    "     The iconv utility exits 0 on success, and >0 if an error occurs.",
	    "",
	    "SEE ALSO",
	    "     iconv(3)",
	    "",
	    "STANDARDS",
	    "     iconv conform to IEEE Std 1003.1-2001 (``POSIX.1'').",
	    "",
	    "HISTORY",
	    "     iconv first appeared in NetBSD 2.0.",
	    "",
	    NULL};
	const char *text;
	unsigned i;

	for (i = 0; NULL != (text = helptext[i]); ++i) {
		fprintf(stderr, "%s\n", text);
	}
	exit(1);
}    
#endif

