/*	$NetBSD: http.c,v 1.37 2014/06/11 13:12:12 joerg Exp $	*/
/*-
 * Copyright (c) 2000-2004 Dag-Erling Co�dan Sm�rgrav
 * Copyright (c) 2003 Thomas Klausner <wiz@NetBSD.org>
 * Copyright (c) 2008, 2009 Joerg Sonnenberger <joerg@NetBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: http.c,v 1.83 2008/02/06 11:39:55 des Exp $
 * $FreeBSD: http.c,v 1.103 2013/04/13 00:49:10 svnexp Exp $
 */

/*
 * The following copyright applies to the base64 code:
 *
 *-
 * Copyright 1997 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define _BSD_SOURCE     /* implied orgin */

#if defined(__linux__) || defined(__MINT__) || defined(__FreeBSD_kernel__) || defined(__CYGWIN__)
/* Keep this down to Linux or MiNT, it can create surprises elsewhere. */
/*
   __FreeBSD_kernel__ is defined for GNU/kFreeBSD.
   See http://glibc-bsd.alioth.debian.org/porting/PORTING .
*/
#define _GNU_SOURCE
#endif

/* Needed for gmtime_r on Interix */
#if !defined(_REENTRANT)
#define _REENTRANT
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(__linux__)  /* _BSD_SOURCE has been deprecated, glibc >= 2.2 */
#if defined(_BSD_SOURCE)
#define _DEFAULT_SOURCE
#endif
#endif

#include <sys/types.h>
#include <sys/socket.h>

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(HAVE_OPENSSL) && (WITH_SSL)
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <openssl/md5.h>
#else
#include "md5.h"
#endif
#define MD5Init(c) MD5_Init(c)
#define MD5Update(c, data, len) MD5_Update(c, data, len)
#define MD5Final(md, c) MD5_Final(md, c)

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <netdb.h>

#include <arpa/inet.h>

#include "fetch.h"
#include "common.h"
#include "httperr.h"

#include <libtime.h>

/* Maximum number of redirects to follow */
#define MAX_REDIRECT 5

/* Symbolic names for reply codes we care about */
#define HTTP_OK			200
#define HTTP_PARTIAL		206
#define HTTP_MOVED_PERM		301
#define HTTP_MOVED_TEMP		302
#define HTTP_SEE_OTHER		303
#define HTTP_NOT_MODIFIED	304
#define HTTP_USE_PROXY		305
#define HTTP_TEMP_REDIRECT	307
#define HTTP_PERM_REDIRECT	308
#define HTTP_NEED_AUTH		401
#define HTTP_NEED_PROXY_AUTH	407
#define HTTP_BAD_RANGE		416
#define HTTP_PROTOCOL_ERROR	999

#define HTTP_REDIRECT(xyz) ((xyz) == HTTP_MOVED_PERM \
			    || (xyz) == HTTP_MOVED_TEMP \
			    || (xyz) == HTTP_TEMP_REDIRECT \
			    || (xyz) == HTTP_USE_PROXY \
			    || (xyz) == HTTP_SEE_OTHER)

#define HTTP_ERROR(xyz) ((xyz) > 400 && (xyz) < 599)

static int http_cmd(conn_t *, const char *, ...) LIBFETCH_PRINTFLIKE(2, 3);

/*****************************************************************************
 * I/O functions for decoding chunked streams
 */

struct httpio
{
	conn_t		*conn;		/* connection */
	int		 chunked;	/* chunked mode */
	int		 keep_alive;	/* keep-alive mode */
	char		*buf;		/* chunk buffer */
	size_t		 bufsize;	/* size of chunk buffer */
	ssize_t		 buflen;	/* amount of data currently in buffer */
	int		 bufpos;	/* current read offset in buffer */
	int		 eof;		/* end-of-file flag */
	int		 error;		/* error flag */
	size_t		 chunksize;	/* remaining size of current chunk */
	off_t		 contentlength;	/* remaining size of the content */
#ifndef NDEBUG
	size_t		 total;
#endif
};

/*
 * Get next chunk header
 */
static int
http_new_chunk(struct httpio *io)
{
	char *p;

	if (fetch_getln(io->conn) == -1)
		return (-1);

	if (io->conn->buflen < 2 || !isxdigit((unsigned char)*io->conn->buf))
		return (-1);

	for (p = io->conn->buf; *p && !isspace((unsigned char)*p); ++p) {
		if (*p == ';')
			break;
		if (!isxdigit((unsigned char)*p))
			return (-1);
		if (isdigit((unsigned char)*p)) {
			io->chunksize = io->chunksize * 16 +
			    *p - '0';
		} else {
			io->chunksize = io->chunksize * 16 +
			    10 + tolower((unsigned char)*p) - 'a';
		}
	}

#ifndef NDEBUG
	if (fetchDebug) {
		io->total += io->chunksize;
		if (io->chunksize == 0)
			trace_log("%s(): end of last chunk\n", __CFUNCTION__);
		else
			trace_log("%s(): new chunk: %lu (%lu)\n",
			    __CFUNCTION__, (unsigned long)io->chunksize,
			    (unsigned long)io->total);
	}
#endif

	return (io->chunksize);
}

/*
 * Grow the input buffer to at least len bytes
 */
static __CINLINE int
http_growbuf(struct httpio *io, size_t len)
{
	char *tmp;

	if (io->bufsize >= len)
		return (0);

	if ((tmp = realloc(io->buf, len)) == NULL)
		return (-1);
	io->buf = tmp;
	io->bufsize = len;
	return (0);
}

/*
 * Fill the input buffer, do chunk decoding on the fly
 */
static int
http_fillbuf(struct httpio *io, size_t len)
{
	if (io->error)
		return (-1);
	if (io->eof)
		return (0);

	if (io->contentlength >= 0 && (off_t)len > io->contentlength)
		len = io->contentlength;

	if (io->chunked == 0) {
		if (http_growbuf(io, len) == -1)
			return (-1);
		if ((io->buflen = fetch_read(io->conn, io->buf, len)) == -1) {
			io->error = 1;
			return (-1);
		}
		if (io->contentlength)
			io->contentlength -= io->buflen;
		io->bufpos = 0;
		return (io->buflen);
	}

	if (io->chunksize == 0) {
		switch (http_new_chunk(io)) {
		case -1:
			io->error = 1;
			return (-1);
		case 0:
			io->eof = 1;
			if (fetch_getln(io->conn) == -1)
				return (-1);
			return (0);
		}
	}

	if (len > io->chunksize)
		len = io->chunksize;
	if (http_growbuf(io, len) == -1)
		return (-1);
	if ((io->buflen = fetch_read(io->conn, io->buf, len)) == -1) {
		io->error = 1;
		return (-1);
	}
	io->chunksize -= io->buflen;
	if (io->contentlength >= 0)
		io->contentlength -= io->buflen;

	if (io->chunksize == 0) {
		char endl[2];
		ssize_t len2;

		len2 = fetch_read(io->conn, endl, 2);
		if (len2 == 1 && fetch_read(io->conn, endl + 1, 1) != 1)
			return (-1);
		if (len2 == -1 || endl[0] != '\r' || endl[1] != '\n')
			return (-1);
	}

	io->bufpos = 0;

	return (io->buflen);
}

/*
 * Read function
 */
static ssize_t
http_readfn(void *v, void *buf, size_t len)
{
	struct httpio *io = (struct httpio *)v;
	size_t l, pos;

	if (io->error)
		return (-1);
	if (io->eof)
		return (0);

	for (pos = 0; len > 0; pos += l, len -= l) {
		/* empty buffer */
		if (!io->buf || io->bufpos == io->buflen)
			if (http_fillbuf(io, len) < 1)
				break;
		l = io->buflen - io->bufpos;
		if (len < l)
			l = len;
		memcpy((char *)buf + pos, io->buf + io->bufpos, l);
		io->bufpos += l;
	}

	if (!pos && io->error) {
		if (io->error == EINTR)
			io->error = 0;
		return (-1);
	}
	return (pos);
}

/*
 * Write function
 */
static ssize_t
http_writefn(void *v, const void *buf, size_t len)
{
	struct httpio *io = (struct httpio *)v;

	return (fetch_write(io->conn, buf, len));
}

/*
 * Close function
 */
static void
http_closefn(void *v)
{
	struct httpio *io = (struct httpio *)v;

	if (io->keep_alive) {
		int val;

		val = 0;
		setsockopt(io->conn->sd, IPPROTO_TCP, TCP_NODELAY, &val,
			   sizeof(val));
			  fetch_cache_put(io->conn, fetch_close);
#ifdef TCP_NOPUSH
		val = 1;
		setsockopt(io->conn->sd, IPPROTO_TCP, TCP_NOPUSH, &val,
		    sizeof(val));
#endif
	} else {
		fetch_close(io->conn);
	}

	free(io->buf);
	free(io);
}

/*
 * Wrap a file descriptor up
 */
static fetchIO *
http_funopen(conn_t *conn, int chunked, int keep_alive, off_t clength)
{
	struct httpio *io;
	fetchIO *f;

	if ((io = calloc(1, sizeof(*io))) == NULL) {
		fetch_syserr();
		return (NULL);
	}
	io->conn = conn;
	io->chunked = chunked;
	io->contentlength = clength;
	io->keep_alive = keep_alive;
	f = fetchIO_unopen(io, http_readfn, http_writefn, http_closefn);
	if (f == NULL) {
		fetch_syserr();
		free(io);
		return (NULL);
	}
	return (f);
}


/*****************************************************************************
 * Helper functions for talking to the server and parsing its replies
 */

/* Header types */
typedef enum {
	hdr_syserror = -2,
	hdr_error = -1,
	hdr_end = 0,
	hdr_unknown = 1,
	hdr_connection,
	hdr_content_length,
	hdr_content_range,
	hdr_last_modified,
	hdr_location,
	hdr_transfer_encoding,
	hdr_www_authenticate,
	hdr_proxy_authenticate,
} hdr_t;

/* Names of interesting headers */
static struct {
	hdr_t		 num;
	const char	*name;
} hdr_names[] = {
	{ hdr_connection,		"Connection" },
	{ hdr_content_length,		"Content-Length" },
	{ hdr_content_range,		"Content-Range" },
	{ hdr_last_modified,		"Last-Modified" },
	{ hdr_location,			"Location" },
	{ hdr_transfer_encoding,	"Transfer-Encoding" },
	{ hdr_www_authenticate,		"WWW-Authenticate" },
	{ hdr_proxy_authenticate,	"Proxy-Authenticate" },
	{ hdr_unknown,			NULL },
};

/*
 * Send a formatted line; optionally echo to terminal
 */
LIBFETCH_PRINTFLIKE(2, 3)
static int
http_cmd(conn_t *conn, const char *fmt, ...)
{
	va_list ap;
	size_t len;
	char *msg;
	int r;

	va_start(ap, fmt);
	len = vasprintf(&msg, fmt, ap);
	va_end(ap);

	if (msg == NULL) {
		errno = ENOMEM;
		fetch_syserr();
		return (-1);
	}

	ED_TRACE(("cmd: <%.*s> (%u)\n", (int)(len - 2), msg, (unsigned)len))
	r = fetch_write(conn, msg, len);
	free(msg);

	if (r == -1) {
		fetch_syserr();
		return (-1);
	}

	return (0);
}

/*
 * Get and parse status line
 */
static int
http_get_reply(conn_t *conn)
{
	char *p;

	if (fetch_getln(conn) == -1)
		return (-1);
	/*
	 * A valid status line looks like "HTTP/m.n xyz reason" where m
	 * and n are the major and minor protocol version numbers and xyz
	 * is the reply code.
	 * Unfortunately, there are servers out there (NCSA 1.5.1, to name
	 * just one) that do not send a version number, so we can't rely
	 * on finding one, but if we do, insist on it being 1.0 or 1.1.
	 * We don't care about the reason phrase.
	 */
	ED_TRACE(("rly: <%s>\n", conn->buf))
	if (strncmp(conn->buf, "HTTP", 4) != 0)
		return (HTTP_PROTOCOL_ERROR);
	p = conn->buf + 4;
	if (*p == '/') {
		if (p[1] != '1' || p[2] != '.' || (p[3] != '0' && p[3] != '1'))
			return (HTTP_PROTOCOL_ERROR);
		p += 4;
	}
	if (*p != ' ' ||
	    !isdigit((unsigned char)p[1]) ||
	    !isdigit((unsigned char)p[2]) ||
	    !isdigit((unsigned char)p[3]))
		return (HTTP_PROTOCOL_ERROR);

	conn->err = (p[1] - '0') * 100 + (p[2] - '0') * 10 + (p[3] - '0');
	return (conn->err);
}

/*
 * Check a header; if the type matches the given string, return a pointer
 * to the beginning of the value.
 */
static const char *
http_match(const char *str, const char *hdr)
{
	while (*str && *hdr &&
	    tolower((unsigned char)*str++) == tolower((unsigned char)*hdr++))
		/* nothing */;
	if (*str || *hdr != ':')
		return (NULL);
	while (*hdr && isspace((unsigned char)*++hdr))
		/* nothing */;
	return (hdr);
}

/*
 * Get the next header and return the appropriate symbolic code.  We
 * need to read one line ahead for checking for a continuation line
 * belonging to the current header (continuation lines start with
 * white space).
 *
 * We get called with a fresh line already in the conn buffer, either
 * from the previous http_next_header() invocation, or, the first
 * time, from a fetch_getln() performed by our caller.
 *
 * This stops when we encounter an empty line (we dont read beyond the header
 * area).
 *
 * Note that the "headerbuf" is just a place to return the result. Its
 * contents are not used for the next call. This means that no cleanup
 * is needed when ie doing another connection, just call the cleanup when
 * fully done to deallocate memory.
 */

/* Limit the max number of continuation lines to some reasonable value */
#define HTTP_MAX_CONT_LINES 10

/* Place into which to build a header from one or several lines */
typedef struct {
	char	*buf;		/* buffer */
	size_t	 bufsize;	/* buffer size */
	size_t	 buflen;	/* length of buffer contents */
} http_headerbuf_t;

static void
init_http_headerbuf(http_headerbuf_t *buf)
{
	buf->buf = NULL;
	buf->bufsize = 0;
	buf->buflen = 0;
}

static void
clean_http_headerbuf(http_headerbuf_t *buf)
{
	if (buf->buf)
		free(buf->buf);
	init_http_headerbuf(buf);
}

/* Remove whitespace at the end of the buffer */
static void
http_conn_trimright(conn_t *conn)
{
	while (conn->buflen && isspace((unsigned char)conn->buf[conn->buflen - 1]))
		conn->buflen--;
	conn->buf[conn->buflen] = '\0';
}

/*
 * Get the next header and return the appropriate symbolic code.
 */
static hdr_t
http_next_header(conn_t *conn, http_headerbuf_t *hbuf, const char **p)
{
	unsigned int i, len;

	/*
	 * Have to do the stripping here because of the first line. So
	 * it's done twice for the subsequent lines. No big deal
	 */
	http_conn_trimright(conn);
	if (conn->buflen == 0)
		return (hdr_end);

	/* Copy the line to the headerbuf */
	if (hbuf->bufsize < conn->buflen + 1) {
		if ((hbuf->buf = realloc(hbuf->buf, conn->buflen + 1)) == NULL)
			return (hdr_syserror);
		hbuf->bufsize = conn->buflen + 1;
	}
	strcpy(hbuf->buf, conn->buf);
	hbuf->buflen = conn->buflen;

	/*
	 * Fetch possible continuation lines. Stop at 1st non-continuation
	 * and leave it in the conn buffer
	 */
	for (i = 0; i < HTTP_MAX_CONT_LINES; i++) {
		if (fetch_getln(conn) == -1)
			return (hdr_syserror);

		/*
		 * Note: we carry on the idea from the previous version
		 * that a pure whitespace line is equivalent to an empty
		 * one (so it's not continuation and will be handled when
		 * we are called next)
		 */
		http_conn_trimright(conn);
		ED_TRACE(("hdr: <%s>\n", conn->buf))
		if (conn->buf[0] != ' ' && conn->buf[0] != "\t"[0])
			break;

		/* Got a continuation line. Concatenate to previous */
		len = hbuf->buflen + conn->buflen;
		if (hbuf->bufsize < len + 1) {
			len *= 2;
			if ((hbuf->buf = realloc(hbuf->buf, len + 1)) == NULL)
				return (hdr_syserror);
			hbuf->bufsize = len + 1;
		}
		strcpy(hbuf->buf + hbuf->buflen, conn->buf);
		hbuf->buflen += conn->buflen;
	}

	/*
	 * We could check for malformed headers but we don't really care.
	 * A valid header starts with a token immediately followed by a
	 * colon; a token is any sequence of non-control, non-whitespace
	 * characters except "()<>@,;:\\\"{}".
	 */
	for (i = 0; hdr_names[i].num != hdr_unknown; i++)
		if ((*p = http_match(hdr_names[i].name, hbuf->buf)) != NULL)
			return (hdr_names[i].num);

	return (hdr_unknown);
}

/**************************
 * [Proxy-]Authenticate header parsing
 */

/*
 * Read doublequote-delimited string into output buffer obuf (allocated
 * by caller, whose responsibility it is to ensure that it's big enough)
 * cp points to the first char after the initial '"'
 * Handles \ quoting
 * Returns pointer to the first char after the terminating double quote, or
 * NULL for error.
 */
static const char *
http_parse_headerstring(const char *cp, char *obuf)
{
	for (;;) {
		switch (*cp) {
		case 0: /* Unterminated string */
			*obuf = 0;
			return (NULL);
		case '"': /* Ending quote */
			*obuf = 0;
			return (++cp);
		case '\\':
			if (*++cp == 0) {
				*obuf = 0;
				return (NULL);
			}
			/* FALLTHROUGH */
		default:
			*obuf++ = *cp++;
		}
	}
}

/* Http auth challenge schemes */
typedef enum {HTTPAS_UNKNOWN, HTTPAS_BASIC,HTTPAS_DIGEST} http_auth_schemes_t;

/* Data holder for a Basic or Digest challenge. */
typedef struct {
	http_auth_schemes_t scheme;
	char	*realm;
	char	*qop;
	char	*nonce;
	char	*opaque;
	char	*algo;
	int	 stale;
	int	 nc; /* Nonce count */
} http_auth_challenge_t;

static void
init_http_auth_challenge(http_auth_challenge_t *b)
{
	b->scheme = HTTPAS_UNKNOWN;
	b->realm = b->qop = b->nonce = b->opaque = b->algo = NULL;
	b->stale = b->nc = 0;
}

static void
clean_http_auth_challenge(http_auth_challenge_t *b)
{
	if (b->realm)
		free(b->realm);
	if (b->qop)
		free(b->qop);
	if (b->nonce)
		free(b->nonce);
	if (b->opaque)
		free(b->opaque);
	if (b->algo)
		free(b->algo);
	init_http_auth_challenge(b);
}

/* Data holder for an array of challenges offered in an http response. */
#define MAX_CHALLENGES 10
typedef struct {
	http_auth_challenge_t *challenges[MAX_CHALLENGES];
	int	count; /* Number of parsed challenges in the array */
	int	valid; /* We did parse an authenticate header */
} http_auth_challenges_t;

static void
init_http_auth_challenges(http_auth_challenges_t *cs)
{
	int i;
	for (i = 0; i < MAX_CHALLENGES; i++)
		cs->challenges[i] = NULL;
	cs->count = cs->valid = 0;
}

static void
clean_http_auth_challenges(http_auth_challenges_t *cs)
{
	int i;
	/* We rely on non-zero pointers being allocated, not on the count */
	for (i = 0; i < MAX_CHALLENGES; i++) {
		if (cs->challenges[i] != NULL) {
			clean_http_auth_challenge(cs->challenges[i]);
			free(cs->challenges[i]);
		}
	}
	init_http_auth_challenges(cs);
}

/*
 * Enumeration for lexical elements. Separators will be returned as their own
 * ascii value
 */
typedef enum {HTTPHL_WORD=256, HTTPHL_STRING=257, HTTPHL_END=258,
	      HTTPHL_ERROR = 259} http_header_lex_t;

/*
 * Determine what kind of token comes next and return possible value
 * in buf, which is supposed to have been allocated big enough by
 * caller. Advance input pointer and return element type.
 */
static int
http_header_lex(const char **cpp, char *buf)
{
	size_t l;
	/* Eat initial whitespace */
	*cpp += strspn(*cpp, " \t");
	if (**cpp == 0)
		return (HTTPHL_END);

	/* Separator ? */
	if (**cpp == ',' || **cpp == '=')
		return (*((*cpp)++));

	/* String ? */
	if (**cpp == '"') {
		*cpp = http_parse_headerstring(++*cpp, buf);
		if (*cpp == NULL)
			return (HTTPHL_ERROR);
		return (HTTPHL_STRING);
	}

	/* Read other token, until separator or whitespace */
	l = strcspn(*cpp, " \t,=");
	memcpy(buf, *cpp, l);
	buf[l] = 0;
	*cpp += l;
	return (HTTPHL_WORD);
}

/*
 * Read challenges from http xxx-authenticate header and accumulate them
 * in the challenges list structure.
 *
 * Headers with multiple challenges are specified by rfc2617, but
 * servers (ie: squid) often send them in separate headers instead,
 * which in turn is forbidden by the http spec (multiple headers with
 * the same name are only allowed for pure comma-separated lists, see
 * rfc2616 sec 4.2).
 *
 * We support both approaches anyway
 */
static int
http_parse_authenticate(const char *cp, http_auth_challenges_t *cs)
{
	int ret = -1;
	http_header_lex_t lex;
	char *key = malloc(strlen(cp) + 1);
	char *value = malloc(strlen(cp) + 1);
	char *buf = malloc(strlen(cp) + 1);

	if (key == NULL || value == NULL || buf == NULL) {
		fetch_syserr();
		goto out;
	}

	/* In any case we've seen the header and we set the valid bit */
	cs->valid = 1;

	/* Need word first */
	lex = http_header_lex(&cp, key);
	if (lex != HTTPHL_WORD)
		goto out;

	/* Loop on challenges */
	for (; cs->count < MAX_CHALLENGES; cs->count++) {
		cs->challenges[cs->count] =
			malloc(sizeof(http_auth_challenge_t));
		if (cs->challenges[cs->count] == NULL) {
			fetch_syserr();
			goto out;
		}
		init_http_auth_challenge(cs->challenges[cs->count]);
		if (!strcasecmp(key, "basic")) {
			cs->challenges[cs->count]->scheme = HTTPAS_BASIC;
		} else if (!strcasecmp(key, "digest")) {
			cs->challenges[cs->count]->scheme = HTTPAS_DIGEST;
		} else {
			cs->challenges[cs->count]->scheme = HTTPAS_UNKNOWN;
			/*
			 * Continue parsing as basic or digest may
			 * follow, and the syntax is the same for
			 * all. We'll just ignore this one when
			 * looking at the list
			 */
		}

		/* Loop on attributes */
		for (;;) {
			/* Key */
			lex = http_header_lex(&cp, key);
			if (lex != HTTPHL_WORD)
				goto out;

			/* Equal sign */
			lex = http_header_lex(&cp, buf);
			if (lex != '=')
				goto out;

			/* Value */
			lex = http_header_lex(&cp, value);
			if (lex != HTTPHL_WORD && lex != HTTPHL_STRING)
				goto out;

			if (!strcasecmp(key, "realm"))
				cs->challenges[cs->count]->realm =
					strdup(value);
			else if (!strcasecmp(key, "qop"))
				cs->challenges[cs->count]->qop =
					strdup(value);
			else if (!strcasecmp(key, "nonce"))
				cs->challenges[cs->count]->nonce =
					strdup(value);
			else if (!strcasecmp(key, "opaque"))
				cs->challenges[cs->count]->opaque =
					strdup(value);
			else if (!strcasecmp(key, "algorithm"))
				cs->challenges[cs->count]->algo =
					strdup(value);
			else if (!strcasecmp(key, "stale"))
				cs->challenges[cs->count]->stale =
					strcasecmp(value, "no");
			/* Else ignore unknown attributes */

			/* Comma or Next challenge or End */
			lex = http_header_lex(&cp, key);
			/*
			 * If we get a word here, this is the beginning of the
			 * next challenge. Break the attributes loop
			 */
			if (lex == HTTPHL_WORD)
				break;

			if (lex == HTTPHL_END) {
				/* End while looking for ',' is normal exit */
				cs->count++;
				ret = 0;
				goto out;
			}
			/* Anything else is an error */
			if (lex != ',')
				goto out;

		} /* End attributes loop */
	} /* End challenge loop */

	/*
	 * Challenges max count exceeded. This really can't happen
	 * with normal data, something's fishy -> error
	 */

out:
	if (key)
		free(key);
	if (value)
		free(value);
	if (buf)
		free(buf);
	return (ret);
}


/*
 * Parse a last-modified header
 */
static int
http_parse_mtime(const char *p, time_t *mtime)
{
//	char locale[64], *r;
	struct tm tm;

//	strncpy(locale, setlocale(LC_TIME, NULL), sizeof(locale));
//	setlocale(LC_TIME, "C");
//	r = strptime(p, "%a, %d %b %Y %H:%M:%S GMT", &tm);
//	/* XXX should add support for date-2 and date-3 */
//	setlocale(LC_TIME, locale);
//	if (r == NULL)
//		return (-1);
//	*mtime = timegm(&tm);
//	return (0);

	*mtime = timehttp(p, &tm);
	ED_TRACE(("last modified: [%04d-%02d-%02d %02d:%02d:%02d]\n",
		  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec))
	return 0;
}


/*
 * Parse a content-length header
 */
static int
http_parse_length(const char *p, off_t *length)
{
	off_t len;

	for (len = 0; *p && isdigit((unsigned char)*p); ++p)
		len = len * 10 + (*p - '0');
	if (*p)
		return (-1);
	ED_TRACE(("content length: [%lld]\n", (long long)len))
	*length = len;
	return (0);
}

/*
 * Parse a content-range header
 */
static int
http_parse_range(const char *p, off_t *offset, off_t *length, off_t *size)
{
	off_t first, last, len;

	if (strncasecmp(p, "bytes ", 6) != 0)
		return (-1);
	p += 6;
	if (*p == '*') {
		first = last = -1;
		++p;
	} else {
		for (first = 0; *p && isdigit((unsigned char)*p); ++p)
			first = first * 10 + *p - '0';
		if (*p != '-')
			return (-1);
		for (last = 0, ++p; *p && isdigit((unsigned char)*p); ++p)
			last = last * 10 + *p - '0';
	}
	if (first > last || *p != '/')
		return (-1);
	for (len = 0, ++p; *p && isdigit((unsigned char)*p); ++p)
		len = len * 10 + *p - '0';
	if (*p || len < last - first + 1)
		return (-1);
	if (first == -1) {
		ED_TRACE(("content range: [*/%lld]\n", (long long)len))
		*length = 0;
	} else {
		ED_TRACE(( "content range: [%lld-%lld/%lld]\n",
		    (long long)first, (long long)last, (long long)len))
		*length = last - first + 1;
	}
	*offset = first;
	*size = len;
	return (0);
}


/*****************************************************************************
 * Helper functions for authorization
 */

/*
 * Base64 encoding
 */
static char *
http_base64(const char *src)
{
	static const char base64[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz"
	    "0123456789+/";
	char *str, *dst;
	size_t l;
	int t, r;

	l = strlen(src);
	if ((str = malloc(((l + 2) / 3) * 4 + 1)) == NULL)
		return (NULL);
	dst = str;
	r = 0;

	while (l >= 3) {
		t = (src[0] << 16) | (src[1] << 8) | src[2];
		dst[0] = base64[(t >> 18) & 0x3f];
		dst[1] = base64[(t >> 12) & 0x3f];
		dst[2] = base64[(t >> 6) & 0x3f];
		dst[3] = base64[(t >> 0) & 0x3f];
		src += 3; l -= 3;
		dst += 4; r += 4;
	}

	switch (l) {
	case 2:
		t = (src[0] << 16) | (src[1] << 8);
		dst[0] = base64[(t >> 18) & 0x3f];
		dst[1] = base64[(t >> 12) & 0x3f];
		dst[2] = base64[(t >> 6) & 0x3f];
		dst[3] = '=';
		dst += 4;
		r += 4;
		break;
	case 1:
		t = src[0] << 16;
		dst[0] = base64[(t >> 18) & 0x3f];
		dst[1] = base64[(t >> 12) & 0x3f];
		dst[2] = dst[3] = '=';
		dst += 4;
		r += 4;
		break;
	case 0:
		break;
	}

	*dst = 0;
	return (str);
}


/*
 * Extract authorization parameters from environment value.
 * The value is like scheme:realm:user:pass
 */
typedef struct {
	char	*scheme;
	char	*realm;
	char	*user;
	char	*password;
} http_auth_params_t;

static void
init_http_auth_params(http_auth_params_t *s)
{
	s->scheme = s->realm = s->user = s->password = 0;
}

static void
clean_http_auth_params(http_auth_params_t *s)
{
	if (s->scheme)
		free(s->scheme);
	if (s->realm)
		free(s->realm);
	if (s->user)
		free(s->user);
	if (s->password)
		free(s->password);
	init_http_auth_params(s);
}

static int
http_authfromenv(const char *p, http_auth_params_t *parms)
{
	int ret = -1;
	char *v, *ve;
	char *str = strdup(p);

	if (str == NULL) {
		fetch_syserr();
		return (-1);
	}
	v = str;

	if ((ve = strchr(v, ':')) == NULL)
		goto out;

	*ve = 0;
	if ((parms->scheme = strdup(v)) == NULL) {
		fetch_syserr();
		goto out;
	}
	v = ve + 1;

	if ((ve = strchr(v, ':')) == NULL)
		goto out;

	*ve = 0;
	if ((parms->realm = strdup(v)) == NULL) {
		fetch_syserr();
		goto out;
	}
	v = ve + 1;

	if ((ve = strchr(v, ':')) == NULL)
		goto out;

	*ve = 0;
	if ((parms->user = strdup(v)) == NULL) {
		fetch_syserr();
		goto out;
	}
	v = ve + 1;


	if ((parms->password = strdup(v)) == NULL) {
		fetch_syserr();
		goto out;
	}
	ret = 0;
out:
	if (ret == -1)
		clean_http_auth_params(parms);
	if (str)
		free(str);
	return (ret);
}


/*
 * Digest response: the code to compute the digest is taken from the
 * sample implementation in RFC2616
 */
#undef  IN
#undef  OUT
#define IN const
#define OUT

#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef unsigned char HASHHEX[HASHHEXLEN+1];

static const char *hexchars = "0123456789abcdef";
static void
CvtHex(IN HASH Bin, OUT HASHHEX Hex)
{
	unsigned short i;
	unsigned char j;

	for (i = 0; i < HASHLEN; i++) {
		j = (Bin[i] >> 4) & 0xf;
		Hex[i*2] = hexchars[j];
		j = Bin[i] & 0xf;
		Hex[i*2+1] = hexchars[j];
	};
	Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
static void
DigestCalcHA1(
	IN char * pszAlg,
	IN char * pszUserName,
	IN char * pszRealm,
	IN char * pszPassword,
	IN char * pszNonce,
	IN char * pszCNonce,
	OUT HASHHEX SessionKey
	)
{
	MD5_CTX Md5Ctx;
	HASH HA1;

	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, pszUserName, strlen(pszUserName));
	MD5Update(&Md5Ctx, ":", 1);
	MD5Update(&Md5Ctx, pszRealm, strlen(pszRealm));
	MD5Update(&Md5Ctx, ":", 1);
	MD5Update(&Md5Ctx, pszPassword, strlen(pszPassword));
	MD5Final(HA1, &Md5Ctx);
	if (strcasecmp(pszAlg, "md5-sess") == 0) {

		MD5Init(&Md5Ctx);
		MD5Update(&Md5Ctx, HA1, HASHLEN);
		MD5Update(&Md5Ctx, ":", 1);
		MD5Update(&Md5Ctx, pszNonce, strlen(pszNonce));
		MD5Update(&Md5Ctx, ":", 1);
		MD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
		MD5Final(HA1, &Md5Ctx);
	};
	CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
static void
DigestCalcResponse(
	IN HASHHEX HA1,           /* H(A1) */
	IN char * pszNonce,       /* nonce from server */
	IN char * pszNonceCount,  /* 8 hex digits */
	IN char * pszCNonce,      /* client nonce */
	IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
	IN char * pszMethod,      /* method from the request */
	IN char * pszDigestUri,   /* requested URL */
	IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
	OUT HASHHEX Response      /* request-digest or response-digest */
	)
{
/*	DEBUG(fprintf(stderr,
		      "Calc: HA1[%s] Nonce[%s] qop[%s] method[%s] URI[%s]\n",
		      HA1, pszNonce, pszQop, pszMethod, pszDigestUri));*/
	MD5_CTX Md5Ctx;
	HASH HA2;
	HASH RespHash;
	HASHHEX HA2Hex;

	// calculate H(A2)
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, pszMethod, strlen(pszMethod));
	MD5Update(&Md5Ctx, ":", 1);
	MD5Update(&Md5Ctx, pszDigestUri, strlen(pszDigestUri));
	if (strcasecmp(pszQop, "auth-int") == 0) {
		MD5Update(&Md5Ctx, ":", 1);
		MD5Update(&Md5Ctx, HEntity, HASHHEXLEN);
	};
	MD5Final(HA2, &Md5Ctx);
	CvtHex(HA2, HA2Hex);

	// calculate response
	MD5Init(&Md5Ctx);
	MD5Update(&Md5Ctx, HA1, HASHHEXLEN);
	MD5Update(&Md5Ctx, ":", 1);
	MD5Update(&Md5Ctx, pszNonce, strlen(pszNonce));
	MD5Update(&Md5Ctx, ":", 1);
	if (*pszQop) {
		MD5Update(&Md5Ctx, pszNonceCount, strlen(pszNonceCount));
		MD5Update(&Md5Ctx, ":", 1);
		MD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
		MD5Update(&Md5Ctx, ":", 1);
		MD5Update(&Md5Ctx, pszQop, strlen(pszQop));
		MD5Update(&Md5Ctx, ":", 1);
	};
	MD5Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
	MD5Final(RespHash, &Md5Ctx);
	CvtHex(RespHash, Response);
}

/*
 * Generate/Send a Digest authorization header
 * This looks like: [Proxy-]Authorization: credentials
 *
 *  credentials      = "Digest" digest-response
 *  digest-response  = 1#( username | realm | nonce | digest-uri
 *                      | response | [ algorithm ] | [cnonce] |
 *                      [opaque] | [message-qop] |
 *                          [nonce-count]  | [auth-param] )
 *  username         = "username" "=" username-value
 *  username-value   = quoted-string
 *  digest-uri       = "uri" "=" digest-uri-value
 *  digest-uri-value = request-uri   ; As specified by HTTP/1.1
 *  message-qop      = "qop" "=" qop-value
 *  cnonce           = "cnonce" "=" cnonce-value
 *  cnonce-value     = nonce-value
 *  nonce-count      = "nc" "=" nc-value
 *  nc-value         = 8LHEX
 *  response         = "response" "=" request-digest
 *  request-digest = <"> 32LHEX <">
 */
static int
http_digest_auth(conn_t *conn, const char *hdr, http_auth_challenge_t *c,
		 http_auth_params_t *parms, struct url *url)
{
	static unsigned char nulhash[HASHHEXLEN+1]= {0};
	int r;
	char noncecount[10];
	char cnonce[40];
	char *options = 0;
	HASHHEX HA1, digest;

	if (!c->realm || !c->nonce) {
		ED_TRACE(("realm/nonce not set in challenge\n"))
		return(-1);
	}
	if (!c->algo)
		c->algo = strdup("");

	if (asprintf(&options, "%s%s%s%s",
		     *c->algo? ",algorithm=" : "", c->algo,
		     c->opaque? ",opaque=" : "", c->opaque?c->opaque:"")== -1)
		return (-1);

	if (!c->qop) {
		c->qop = strdup("");
		*noncecount = 0;
		*cnonce = 0;
	} else {
		c->nc++;
		sprintf(noncecount, "%08x", c->nc);
		/* We don't try very hard with the cnonce ... */
		sprintf(cnonce, "%x%lx", getpid(), (unsigned long)time(0));
	}

	DigestCalcHA1(c->algo, parms->user, c->realm,
		    parms->password, c->nonce, cnonce, HA1);
	ED_TRACE(("HA1: [%s]\n", HA1))
	DigestCalcResponse(HA1, c->nonce, noncecount, cnonce, c->qop,
			"GET", url->doc, (const unsigned char *)nulhash /*""*/, digest);

	if (c->qop[0]) {
		r = http_cmd(conn, "%s: Digest username=\"%s\",realm=\"%s\","
			     "nonce=\"%s\",uri=\"%s\",response=\"%s\","
			     "qop=\"auth\", cnonce=\"%s\", nc=%s%s\r\n",
			     hdr, parms->user, c->realm,
			     c->nonce, url->doc, digest,
			     cnonce, noncecount, options);
	} else {
		r = http_cmd(conn, "%s: Digest username=\"%s\",realm=\"%s\","
			     "nonce=\"%s\",uri=\"%s\",response=\"%s\"%s\r\n",
			     hdr, parms->user, c->realm,
			     c->nonce, url->doc, digest, options);
	}
	if (options)
		free(options);
	return (r);
}

/*
 * Encode username and password
 */
static int
http_basic_auth(conn_t *conn, const char *hdr, const char *usr, const char *pwd)
{
	char *upw, *auth;
	int r;

	if (asprintf(&upw, "%s:%s", usr, pwd) == -1)
		return (-1);
	auth = http_base64(upw);
	free(upw);
	if (auth == NULL)
		return (-1);
	r = http_cmd(conn, "%s: Basic %s\r\n", hdr, auth);
	free(auth);
	return (r);
}

/*
 * Chose the challenge to answer and call the appropriate routine to
 * produce the header.
 */
static int
http_authorize(conn_t *conn, const char *hdr, http_auth_challenges_t *cs,
	       http_auth_params_t *parms, struct url *url)
{
//	http_auth_challenge_t *basic = NULL;
	http_auth_challenge_t *digest = NULL;
	int i;

	/* If user or pass are null we're not happy */
	if (!parms->user || !parms->password) {
		ED_TRACE(("NULL usr or pass\n"))
		return (-1);
	}

	/* Look for a Digest and a Basic challenge */
	for (i = 0; i < cs->count; i++) {
//		if (cs->challenges[i]->scheme == HTTPAS_BASIC)
//			basic = cs->challenges[i];
		if (cs->challenges[i]->scheme == HTTPAS_DIGEST)
			digest = cs->challenges[i];
	}

	/* Error if "Digest" was specified and there is no Digest challenge */
	if (!digest && (parms->scheme &&
			!strcasecmp(parms->scheme, "digest"))) {
		ED_TRACE(("Digest auth in env, not supported by peer\n"))
		return (-1);
	}
	/*
	 * If "basic" was specified in the environment, or there is no Digest
	 * challenge, do the basic thing. Don't need a challenge for this,
	 * so no need to check basic!=NULL
	 */
	if (!digest || (parms->scheme && !strcasecmp(parms->scheme,"basic")))
		return (http_basic_auth(conn,hdr,parms->user,parms->password));

	/* Else, prefer digest. We just checked that it's not NULL */
	return (http_digest_auth(conn, hdr, digest, parms, url));
}

/*****************************************************************************
 * Helper functions for connecting to a server or proxy
 */

/*
 * Connect to the correct HTTP server or proxy.
 */
static conn_t *
http_connect(struct url *URL, struct url *purl, const char *flags, int *cached)
{
	conn_t *conn;
	int af, verbose;
#ifdef TCP_NOPUSH
	int val;
#endif

	*cached = 1;

#ifdef INET6
	af = AF_UNSPEC;
#else
	af = AF_INET;
#endif

	verbose = CHECK_FLAG('v');
	if (CHECK_FLAG('4'))
		af = AF_INET;
#ifdef INET6
	else if (CHECK_FLAG('6'))
		af = AF_INET6;
#endif

	if (purl && strcasecmp(URL->scheme, SCHEME_HTTPS) != 0) {
		URL = purl;
	} else if (strcasecmp(URL->scheme, SCHEME_FTP) == 0) {
		/* can't talk http to an ftp server */
		/* XXX should set an error code */
		return (NULL);
	}

	if ((conn = fetch_cache_get(URL, af)) != NULL) {
		*cached = 1;
		return (conn);
	}

	if ((conn = fetch_connect(URL, af, verbose)) == NULL)
		/* fetch_connect() has already set an error code */
		return (NULL);
	if (strcasecmp(URL->scheme, SCHEME_HTTPS) == 0 &&
	    fetch_ssl(conn, URL, verbose) == -1) {
		fetch_close(conn);
		/* grrr */
#ifdef EAUTH
		errno = EAUTH;
#else
		errno = EPERM;
#endif
		fetch_syserr();
		return (NULL);
	}

#ifdef TCP_NOPUSH
	val = 1;
	setsockopt(conn->sd, IPPROTO_TCP, TCP_NOPUSH, &val, sizeof(val));
#endif

	return (conn);
}

static struct url *
http_get_proxy(struct url * url, const char *flags)
{
	struct url *purl;
	char *p;

	if (flags != NULL && strchr(flags, 'd') != NULL)
		return (NULL);
	if (fetch_no_proxy_match(url->host))
		return (NULL);
	if (((p = getenv("HTTP_PROXY")) || (p = getenv("http_proxy"))) &&
	    *p && (purl = fetchParseURL(p))) {
		if (!*purl->scheme)
			strcpy(purl->scheme, SCHEME_HTTP);
		if (!purl->port)
			purl->port = fetch_default_proxy_port(purl->scheme);
		if (strcasecmp(purl->scheme, SCHEME_HTTP) == 0)
			return (purl);
		fetchFreeURL(purl);
	}
	return (NULL);
}


typedef struct {
	char		 buffer[1024];
	char		*cursor;
	size_t		 size;
	size_t		 cached;
} getln_t;

static char *
http_getln(fetchIO *in, int *ret, getln_t *ln)
{
	char *cursor = ln->cursor;
	size_t size = ln->size, len = 0;
	int length;

	if (NULL == cursor) {
		size = sizeof(ln->buffer) * 2;
		if (NULL == (cursor = malloc(size + 2))) {
			return NULL;
		}

	} else if ((len = (int)ln->cached) > 0) {
		memcpy(cursor, (const char *)(ln->buffer + (sizeof(ln->buffer) - len)), len);
		ln->cached = 0;
	}

	while ((length = fetchIO_read(in, ln->buffer, sizeof(ln->buffer))) > 0) {
		char *buffer = ln->buffer;

		while (length-- > 0) {
			if ('\r' == *buffer) {
				if (length && '\n' == *buffer) {
					--length;
				}
				ln->cached = length;
				break;
			}
			if (++len >= size) {
				size *= 2;
				if (NULL == (cursor = realloc(cursor, size + 2))) {
					len = -1;
					break;
				}
			}
			cursor[len] = *buffer++;
		}
	}

	ln->cursor = cursor;
	ln->size = size;

	cursor[len] = 0;
	return ((*ret = len) > 0 ? cursor : NULL);
}

static void
http_print_html(FILE *out, fetchIO *in)
{
	getln_t ln = {0};
	char *line, *p, *q;
	int comment, tag;
	int len = 0;

	comment = tag = 0;
	while ((line = http_getln(in, &len, &ln)) != NULL) {
		while (len && isspace((unsigned char)line[len - 1]))
			--len;
		for (p = q = line; q < line + len; ++q) {
			if (comment && *q == '-') {
				if (q + 2 < line + len && strcmp(q, "-->") == 0) {
					tag = comment = 0;
					q += 2;
				}
			} else if (tag && !comment && *q == '>') {
				p = q + 1;
				tag = 0;
			} else if (!tag && *q == '<') {
				if (q > p)
					fwrite(p, q - p, 1, out);
				tag = 1;
				if (q + 3 < line + len && strcmp(q, "<!--") == 0) {
					comment = 1;
					q += 3;
				}
			}
		}
		if (!tag && q > p)
			fwrite(p, q - p, 1, out);
		fputc('\n', out);
	}
	free(ln.cursor);
}


static void
set_if_modified_since(conn_t *conn, time_t last_modified, int verbose)
{
	static const char weekdays[] = "SunMonTueWedThuFriSat";
	static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
#if defined(HAVE_GMTIME_R)
	struct tm tm = {0};
#endif
	struct tm *ltm;
	char buf[80];

#if defined(HAVE_GMTIME_R)
	ltm = gmtime_r(&last_modified, &tm);
#else
	ltm = gmtime(&last_modified);
#endif
	snprintf(buf, sizeof(buf), "%.3s, %02d %.3s %4ld %02d:%02d:%02d GMT",
	    weekdays + ltm->tm_wday * 3, ltm->tm_mday, months + ltm->tm_mon * 3,
	    (long)(ltm->tm_year + 1900), ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	if (verbose)
		fetch_info("If-Modified-Since: %s", buf);
	http_cmd(conn, "If-Modified-Since: %s\r\n", buf);
}


/*****************************************************************************
 * Core
 */

/*
 * Send a request and process the reply
 *
 * XXX This function is way too long, the do..while loop should be split
 * XXX off into a separate function.
 */
fetchIO *
http_request(struct url *URL, const char *op, struct url_stat *us,
    struct url *purl, const char *flags)
{
	conn_t *conn;
	struct url *url, *new;
	int chunked, direct, if_modified_since, noredirect;
	int keep_alive, verbose, cached;
	int e, i, n, val;
	off_t offset, clength, length, size;
	time_t mtime;
	const char *p;
	fetchIO *f;
	hdr_t h;
	char hbuf[URL_HOSTLEN + 7], *host;
	http_headerbuf_t headerbuf;
	http_auth_challenges_t server_challenges;
	http_auth_challenges_t proxy_challenges;

	init_http_headerbuf(&headerbuf);
	init_http_auth_challenges(&server_challenges);
	init_http_auth_challenges(&proxy_challenges);
	direct = CHECK_FLAG('d');
	noredirect = CHECK_FLAG('A');
	verbose = CHECK_FLAG('v');
	if_modified_since = CHECK_FLAG('i');
	keep_alive = 0;

	if (direct && purl) {
		fetchFreeURL(purl);
		purl = NULL;
	}

	/* try the provided URL first */
	url = URL;

	/* if the A flag is set, we only get one try */
	n = noredirect ? 1 : MAX_REDIRECT;
	i = 0;

	e = HTTP_PROTOCOL_ERROR;
	do {
		new = NULL;
		chunked = 0;
		offset = 0;
		clength = -1;
		length = -1;
		size = -1;
		mtime = 0;

		/* check port */
		if (!url->port)
			url->port = fetch_default_port(url->scheme);

		/* were we redirected to an FTP URL? */
		if (purl == NULL && strcmp(url->scheme, SCHEME_FTP) == 0) {
			if (strcmp(op, "GET") == 0)
				return (ftp_request(url, "RETR", NULL, us, purl, flags));
			else if (strcmp(op, "HEAD") == 0)
				return (ftp_request(url, "STAT", NULL, us, purl, flags));
		}

		/* connect to server or proxy */
		if ((conn = http_connect(url, purl, flags, &cached)) == NULL)
			goto ouch;

		host = url->host;
#ifdef INET6
		if (strchr(url->host, ':')) {
			snprintf(hbuf, sizeof(hbuf), "[%s]", url->host);
			host = hbuf;
		}
#endif
		if (url->port != fetch_default_port(url->scheme)) {
			if (host != hbuf) {
				strcpy(hbuf, host);
				host = hbuf;
			}
			snprintf(hbuf + strlen(hbuf),
			    sizeof(hbuf) - strlen(hbuf), ":%d", url->port);
		}

		/* send request */
		if (verbose)
			fetch_info("requesting %s://%s%s",
			    url->scheme, host, url->doc);
		if (purl) {
			http_cmd(conn, "%s %s://%s%s HTTP/1.1\r\n",
			    op, url->scheme, host, url->doc);
		} else {
			http_cmd(conn, "%s %s HTTP/1.1\r\n",
			    op, url->doc);
		}

		if (if_modified_since && url->last_modified > 0)
			set_if_modified_since(conn, url->last_modified, verbose);

		/* virtual host */
		http_cmd(conn, "Host: %s\r\n", host);

		/*
		 * Proxy authorization: we only send auth after we received
		 * a 407 error. We do not first try basic anyway (changed
		 * when support was added for digest-auth)
		 */
		if (purl && proxy_challenges.valid) {
			http_auth_params_t aparams;
			init_http_auth_params(&aparams);
			if (*purl->user || *purl->pwd) {
				aparams.user = /*purl->user ?*/
					strdup(purl->user) /*: strdup("")*/;
				aparams.password = /*purl->pwd ?*/
					strdup(purl->pwd) /*: strdup("")*/;
			} else if ((p = getenv("HTTP_PROXY_AUTH")) != NULL &&
				   *p != '\0') {
				if (http_authfromenv(p, &aparams) < 0) {
					http_seterr(HTTP_NEED_PROXY_AUTH);
					goto ouch;
				}
			}
			http_authorize(conn, "Proxy-Authorization",
				       &proxy_challenges, &aparams, url);
			clean_http_auth_params(&aparams);
		}

		/*
		 * Server authorization: we never send "a priori"
		 * Basic auth, which used to be done if user/pass were
		 * set in the url. This would be weird because we'd send the
		 * password in the clear even if Digest is finally to be
		 * used (it would have made more sense for the
		 * pre-digest version to do this when Basic was specified
		 * in the environment)
		 */
		if (server_challenges.valid) {
			http_auth_params_t aparams;
			init_http_auth_params(&aparams);
			if (*url->user || *url->pwd) {
				aparams.user = /*url->user ?*/
					strdup(url->user) /*: strdup("")*/;
				aparams.password = /*url->pwd ?*/
					strdup(url->pwd) /*: strdup("")*/;
			} else if ((p = getenv("HTTP_AUTH")) != NULL &&
				   *p != '\0') {
				if (http_authfromenv(p, &aparams) < 0) {
					http_seterr(HTTP_NEED_AUTH);
					goto ouch;
				}
			} else if (fetchAuthMethod &&
				   fetchAuthMethod(url) == 0) {
				aparams.user = /*url->user ?*/
					strdup(url->user) /*: strdup("")*/;
				aparams.password = /*url->pwd ?*/
					strdup(url->pwd) /*: strdup("")*/;
			} else {
				http_seterr(HTTP_NEED_AUTH);
				goto ouch;
			}
			http_authorize(conn, "Authorization",
				       &server_challenges, &aparams, url);
			clean_http_auth_params(&aparams);
		}

		/* other headers */
		if ((p = getenv("HTTP_REFERER")) != NULL && *p != '\0') {
			if (strcasecmp(p, "auto") == 0)
				http_cmd(conn, "Referer: %s://%s%s\r\n",
				    url->scheme, host, url->doc);
			else
				http_cmd(conn, "Referer: %s\r\n", p);
		}
		if ((p = getenv("HTTP_USER_AGENT")) != NULL && *p != '\0')
			http_cmd(conn, "User-Agent: %s\r\n", p);
		else
			http_cmd(conn, "User-Agent: %s\r\n", _LIBFETCH_VER);
		if (url->offset > 0)
			http_cmd(conn, "Range: bytes=%lld-\r\n", (long long)url->offset);
	    //	http_cmd(conn, "Connection: close\r\n");
		http_cmd(conn, "\r\n");

		/*
		 * Force the queued request to be dispatched.  Normally, one
		 * would do this with shutdown(2) but squid proxies can be
		 * configured to disallow such half-closed connections.  To
		 * be compatible with such configurations, fiddle with socket
		 * options to force the pending data to be written.
		 */
#ifdef TCP_NOPUSH
		val = 0;
		setsockopt(conn->sd, IPPROTO_TCP, TCP_NOPUSH, &val, sizeof(val));
#endif
		val = 1;
		setsockopt(conn->sd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val));

		/* get reply */
		switch (http_get_reply(conn)) {
		case HTTP_OK:
		case HTTP_PARTIAL:
		case HTTP_NOT_MODIFIED:
			/* fine */
			break;
		case HTTP_MOVED_PERM:
		case HTTP_MOVED_TEMP:
		case HTTP_SEE_OTHER:
		case HTTP_USE_PROXY:
			/*
			 * Not so fine, but we still have to read the
			 * headers to get the new location.
			 */
			break;
		case HTTP_NEED_AUTH:
			if (server_challenges.valid) {
				/*
				 * We already sent out authorization code,
				 * so there's nothing more we can do.
				 */
				http_seterr(conn->err);
				goto ouch;
			}
			/* try again, but send the password this time */
			if (verbose)
				fetch_info("server requires authorization");
			break;
		case HTTP_NEED_PROXY_AUTH:
			if (proxy_challenges.valid) {
				/*
				 * We already sent our proxy
				 * authorization code, so there's
				 * nothing more we can do. */
				http_seterr(conn->err);
				goto ouch;
			}
			/* try again, but send the password this time */
			if (verbose)
				fetch_info("proxy requires authorization");
			break;
		case HTTP_BAD_RANGE:
			/*
			 * This can happen if we ask for 0 bytes because
			 * we already have the whole file.  Consider this
			 * a success for now, and check sizes later.
			 */
			break;
		case HTTP_PROTOCOL_ERROR:
			/* fall through */
		case -1:
			--i;
			if (cached)
				continue;
			fetch_syserr();
			goto ouch;
		default:
			http_seterr(conn->err);
			if (!verbose)
				goto ouch;
			/* fall through so we can get the full error message */
		}

		/* get headers. http_next_header expects one line readahead */
		if (fetch_getln(conn) == -1) {
			fetch_syserr();
			goto ouch;
		}
		ED_TRACE(("hdr: <%s>\n", conn->buf))
		do {
			switch ((h = http_next_header(conn, &headerbuf, &p))) {
			case hdr_syserror:
				fetch_syserr();
				goto ouch;
			case hdr_error:
				http_seterr(HTTP_PROTOCOL_ERROR);
				goto ouch;
			case hdr_connection:
				/* XXX too weak? */
				keep_alive = (strcasecmp(p, "keep-alive") == 0);
				break;
			case hdr_content_length:
				http_parse_length(p, &clength);
				break;
			case hdr_content_range:
				http_parse_range(p, &offset, &length, &size);
				break;
			case hdr_last_modified:
				http_parse_mtime(p, &mtime);
				break;
			case hdr_location:
				if (!HTTP_REDIRECT(conn->err))
					break;
				/*
				 * if the A flag is set, we don't follow
				 * temporary redirects.
				 */
				if (noredirect &&
				    conn->err != HTTP_MOVED_PERM &&
				    conn->err != HTTP_PERM_REDIRECT &&
				    conn->err != HTTP_USE_PROXY) {
					n = 1;
					break;
				}
				if (new)
					free(new);
				if (verbose)
					fetch_info("%d redirect to %s", conn->err, p);
				if (*p == '/')
					/* absolute path */
					new = fetchMakeURL(url->scheme, url->host, url->port, p,
					    url->user, url->pwd);
				else
					new = fetchParseURL(p);
				if (new == NULL) {
					/* XXX should set an error code */
					ED_TRACE(("failed to parse new URL\n"))
					goto ouch;
				}

				/* Only copy credentials if the host matches */
				if (!strcmp(new->host, url->host) && !*new->user && !*new->pwd) {
					strcpy(new->user, url->user);
					strcpy(new->pwd, url->pwd);
				}
				new->offset = url->offset;
				new->length = url->length;
				break;
			case hdr_transfer_encoding:
				/* XXX weak test*/
				chunked = (strcasecmp(p, "chunked") == 0);
				break;
			case hdr_www_authenticate:
				if (conn->err != HTTP_NEED_AUTH)
					break;
				if (http_parse_authenticate(p, &server_challenges) == 0)
					++n;
				break;
			case hdr_proxy_authenticate:
				if (conn->err != HTTP_NEED_PROXY_AUTH)
					break;
				if (http_parse_authenticate(p, &proxy_challenges) == 0)
					++n;
				break;
			case hdr_end:
				/* fall through */
			case hdr_unknown:
				/* ignore */
				break;
			}
		} while (h > hdr_end);

		/* we need to provide authentication */
		if (conn->err == HTTP_NEED_AUTH ||
		    conn->err == HTTP_NEED_PROXY_AUTH) {
			e = conn->err;
			if ((conn->err == HTTP_NEED_AUTH &&
			     !server_challenges.valid) ||
			    (conn->err == HTTP_NEED_PROXY_AUTH &&
			     !proxy_challenges.valid)) {
				/* 401/7 but no www/proxy-authenticate ?? */
				ED_TRACE(("401/7 and no auth header\n"))
				goto ouch;
			}
			fetch_close(conn);
			conn = NULL;
			continue;
		}

		/* requested range not satisfiable */
		if (conn->err == HTTP_BAD_RANGE) {
			if (url->offset == size && url->length == 0) {
				/* asked for 0 bytes; fake it */
				offset = url->offset;
				clength = -1;
				conn->err = HTTP_OK;
				break;
			} else {
				http_seterr(conn->err);
				goto ouch;
			}
		}

		/* we have a hit or an error */
		if (conn->err == HTTP_OK
		    || conn->err == HTTP_NOT_MODIFIED
		    || conn->err == HTTP_PARTIAL
		    || HTTP_ERROR(conn->err))
			break;

		/* all other cases: we got a redirect */
		e = conn->err;
		clean_http_auth_challenges(&server_challenges);
		fetch_close(conn);
		conn = NULL;
		if (!new) {
			ED_TRACE(("redirect with no new location\n"))
			break;
		}
		if (url != URL)
			fetchFreeURL(url);
		url = new;
	} while (++i < n);

	/* we failed, or ran out of retries */
	if (conn == NULL) {
		http_seterr(e);
		goto ouch;
	}

	ED_TRACE(("offset %lld, length %lld, size %lld, clength %lld\n",
		  (long long)offset, (long long)length, (long long)size, (long long)clength))

	if (conn->err == HTTP_NOT_MODIFIED) {
		http_seterr(HTTP_NOT_MODIFIED);
		return (NULL);
	}

	/* check for inconsistencies */
	if (clength != -1 && length != -1 && clength != length) {
		http_seterr(HTTP_PROTOCOL_ERROR);
		goto ouch;
	}
	if (clength == -1)
		clength = length;
	if (clength != -1)
		length = offset + clength;
	if (length != -1 && size != -1 && length != size) {
		http_seterr(HTTP_PROTOCOL_ERROR);
		goto ouch;
	}
	if (size == -1)
		size = length;

	/* fill in stats */
	if (us) {
		us->size = size;
		us->atime = us->mtime = mtime;
	}

	/* too far? */
	if (URL->offset > 0 && offset > URL->offset) {
		http_seterr(HTTP_PROTOCOL_ERROR);
		goto ouch;
	}

	/* report back real offset and size */
	URL->offset = offset;
	URL->length = clength;

	if (clength == -1 && !chunked)
		keep_alive = 0;

	if (conn->err == HTTP_NOT_MODIFIED) {
		http_seterr(HTTP_NOT_MODIFIED);
		if (keep_alive) {
			fetch_cache_put(conn, fetch_close);
			conn = NULL;
		}
		goto ouch;
	}

	/* wrap it up in a fetchIO */
	if ((f = http_funopen(conn, chunked, keep_alive, clength)) == NULL) {
		fetch_syserr();
		goto ouch;
	}

	if (url != URL)
		fetchFreeURL(url);
	if (purl)
		fetchFreeURL(purl);

	if (HTTP_ERROR(conn->err)) {
		http_print_html(stderr, f);

		if (keep_alive) {
			char buf[512];
			do {
			} while (fetchIO_read(f, buf, sizeof(buf)) > 0);
		}

		fetchIO_close(f);
		f = NULL;
	}
	clean_http_headerbuf(&headerbuf);
	clean_http_auth_challenges(&server_challenges);
	clean_http_auth_challenges(&proxy_challenges);
	return (f);

ouch:
	if (url != URL)
		fetchFreeURL(url);
	if (purl)
		fetchFreeURL(purl);
	if (conn != NULL)
		fetch_close(conn);
	clean_http_headerbuf(&headerbuf);
	clean_http_auth_challenges(&server_challenges);
	clean_http_auth_challenges(&proxy_challenges);
	return (NULL);
}


/*****************************************************************************
 * Entry points
 */

/*
 * Retrieve and stat a file by HTTP
 */
fetchIO *
fetchXGetHTTP(struct url *URL, struct url_stat *us, const char *flags)
{
	return (http_request(URL, "GET", us, http_get_proxy(URL, flags), flags));
}

/*
 * Retrieve a file by HTTP
 */
fetchIO *
fetchGetHTTP(struct url *URL, const char *flags)
{
	return (fetchXGetHTTP(URL, NULL, flags));
}

/*
 * Store a file by HTTP
 */
fetchIO *
fetchPutHTTP(struct url *URL, const char *flags)
{
	fetch_info("fetchPutHTTP(): not implemented");
	return (NULL);
}

/*
 * Get an HTTP document's metadata
 */
int
fetchStatHTTP(struct url *URL, struct url_stat *us, const char *flags)
{
	fetchIO *f;

	f = http_request(URL, "HEAD", us, http_get_proxy(URL, flags), flags);
	if (f == NULL)
		return (-1);
	fetchIO_close(f);
	return (0);
}

enum http_states {
	ST_NONE,
	ST_LT,
	ST_LTA,
	ST_TAGA,
	ST_H,
	ST_R,
	ST_E,
	ST_F,
	ST_HREF,
	ST_HREFQ,
	ST_TAG,
	ST_TAGAX,
	ST_TAGAQ
};

struct index_parser {
	struct url_list *ue;
	struct url *url;
	enum http_states state;
};

static ssize_t
parse_index(struct index_parser *parser, const char *buf, size_t len)
{
	char *end_attr, p = *buf;

	switch (parser->state) {
	case ST_NONE:
		/* Plain text, not in markup */
		if (p == '<')
			parser->state = ST_LT;
		return 1;
	case ST_LT:
		/* In tag -- "<" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == 'a' || p == 'A')
			parser->state = ST_LTA;		/* "<a" */
		else if (!isspace((unsigned char)p))
			parser->state = ST_TAG;
		return 1;
	case ST_LTA:
		/* In tag -- "<a" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (isspace((unsigned char)p))
			parser->state = ST_TAGA;	/* "<a " */
		else
			parser->state = ST_TAG;
		return 1;
	case ST_TAG:
		/* In tag, but not "<a" -- disregard */
		if (p == '>')
			parser->state = ST_NONE;
		return 1;
	case ST_TAGA:
		/* In a-tag -- "<a " already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (p == 'h' || p == 'H')
			parser->state = ST_H;		/* "<a href=" */
		else if (!isspace((unsigned char)p))
			parser->state = ST_TAGAX;
		return 1;
	case ST_TAGAX:
		/* In unknown keyword in a-tag */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (isspace((unsigned char)p))
			parser->state = ST_TAGA;
		return 1;
	case ST_TAGAQ:
		/* In a-tag, unknown argument for keys. */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGA;
		return 1;
	case ST_H:
		/* In a-tag -- "<a h" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (p == 'r' || p == 'R')		/* "<a hr" */
			parser->state = ST_R;
		else if (isspace((unsigned char)p))
			parser->state = ST_TAGA;
		else
			parser->state = ST_TAGAX;
		return 1;
	case ST_R:
		/* In a-tag -- "<a hr" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (p == 'e' || p == 'E')
			parser->state = ST_E;		/* "<a hre" */
		else if (isspace((unsigned char)p))
			parser->state = ST_TAGA;
		else
			parser->state = ST_TAGAX;
		return 1;
	case ST_E:
		/* In a-tag -- "<a hre" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (p == 'f' || p == 'F')
			parser->state = ST_F;		/* "<a href" */
		else if (isspace((unsigned char)p))
			parser->state = ST_TAGA;
		else
			parser->state = ST_TAGAX;
		return 1;
	case ST_F:
		/* In a-tag -- "<a href" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_TAGAQ;
		else if (p == '=')
			parser->state = ST_HREF;	/* "<a href=" */
		else if (!isspace((unsigned char)p))
			parser->state = ST_TAGAX;
		return 1;
	case ST_HREF:
		/* In a-tag -- "<a href=" already found */
		if (p == '>')
			parser->state = ST_NONE;
		else if (p == '"')
			parser->state = ST_HREFQ;	/* "<a href=\"" */
		else if (!isspace((unsigned char)p))
			parser->state = ST_TAGA;
		return 1;
	case ST_HREFQ:
		/* In href of the a-tag */
		end_attr = memchr(buf, '"', len);
		if (end_attr == NULL)
			return 0;
		*end_attr = '\0';
		parser->state = ST_TAGA;
		if (*buf) {
			if ('?' != *buf) {		/* exclude implied index references (eg. ?C=M;O=A), FIX */
				if (fetch_add_entry(parser->ue, parser->url, buf, 1))
					return -1;
			}
		}
		return end_attr + 1 - buf;
	}
	/* NOTREACHED */
	abort();
	return 0;
}

struct http_index_cache {
	struct http_index_cache *next;
	struct url *location;
	struct url_list ue;
};

static struct http_index_cache *index_cache;

/*
 * List a directory
 */
int
fetchListHTTP(struct url_list *ue, struct url *url, const char *pattern, const char *flags)
{
	fetchIO *f;
	char buf[2 * PATH_MAX];
	size_t buf_len, sum_processed;
	ssize_t read_len, processed;
	struct index_parser state;
	struct http_index_cache *cache = NULL;
	int do_cache, ret;

	do_cache = CHECK_FLAG('c');

	if (do_cache) {
		for (cache = index_cache; cache != NULL; cache = cache->next) {
			if (strcmp(cache->location->scheme, url->scheme))
				continue;
			if (strcmp(cache->location->user, url->user))
				continue;
			if (strcmp(cache->location->pwd, url->pwd))
				continue;
			if (strcmp(cache->location->host, url->host))
				continue;
			if (cache->location->port != url->port)
				continue;
			if (strcmp(cache->location->doc, url->doc))
				continue;
			return fetchAppendURLList(ue, &cache->ue);
		}

		cache = malloc(sizeof(*cache));
		fetchInitURLList(&cache->ue);
		cache->location = fetchCopyURL(url);
	}

	f = fetchGetHTTP(url, flags);
	if (f == NULL) {
		if (do_cache) {
			fetchFreeURLList(&cache->ue);
			fetchFreeURL(cache->location);
			free(cache);
		}
		return -1;
	}

	state.url = url;
	state.state = ST_NONE;
	if (do_cache) {
		state.ue = &cache->ue;
	} else {
		state.ue = ue;
	}

	buf_len = 0;

	while ((read_len = fetchIO_read(f, buf + buf_len, sizeof(buf) - buf_len)) > 0) {
		buf_len += read_len;
		sum_processed = 0;
		do {
			processed = parse_index(&state, buf + sum_processed, buf_len);
			if (processed == -1)
				break;
			buf_len -= processed;
			sum_processed += processed;
		} while (processed != 0 && buf_len > 0);
		if (processed == -1) {
			read_len = -1;
			break;
		}
		memmove(buf, buf + sum_processed, buf_len);
	}

	fetchIO_close(f);

	ret = read_len < 0 ? -1 : 0;

	if (do_cache) {
		if (ret == 0) {
			cache->next = index_cache;
			index_cache = cache;
		}

		if (fetchAppendURLList(ue, &cache->ue))
			ret = -1;
	}

	return ret;
}
