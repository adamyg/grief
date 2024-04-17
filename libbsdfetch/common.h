/*	$NetBSD: common.h,v 1.23 2014/01/08 20:25:34 joerg Exp $	*/
/*-
 * Copyright (c) 2013 - 2024 Adam Young
 * Copyright (c) 1998-2004 Dag-Erling Coïdan Smørgrav
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
 *    derived from this software without specific prior written permission
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
 * $FreeBSD: common.h,v 1.30 2007/12/18 11:03:07 des Exp $
 */

#ifndef _COMMON_H_INCLUDED
#define _COMMON_H_INCLUDED

#define FTP_DEFAULT_PORT	21
#define HTTP_DEFAULT_PORT	80
#define FTP_DEFAULT_PROXY_PORT	21
#define HTTP_DEFAULT_PROXY_PORT	3128

#if defined(HAVE_OPENSSL) && (WITH_SSL)
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#if defined(__GNUC__) && __GNUC__ >= 3
#define LIBFETCH_PRINTFLIKE(fmtarg, firstvararg)	\
	__attribute__((__format__ (__printf__, fmtarg, firstvararg)))
#else
#define LIBFETCH_PRINTFLIKE(fmtarg, firstvararg)
#endif

#if !defined(__sun) && !defined(__hpux) && !defined(__INTERIX) && \
    !defined(__digital__) && !defined(__linux) && !defined(__MINT__) && \
    !defined(__sgi) && !defined(__minix) && \
    !defined(__CYGWIN__) && !defined(_WIN32) && !defined(WIN32)
#define HAVE_SA_LEN
#endif

#if !defined(HAVE_TIMEGM)
#include <libtime.h>
#define timegm(_tm)	xtimegm(_tm)
#endif

#define ED_LEVEL        1
#include <eddebug.h>
#if defined(_WIN32) || defined(WIN32)
#include <sys/socket.h>
#include <unistd.h>
#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf	_snprintf
#endif
#if !defined(strcasecmp)
#define strcasecmp	stricmp
#define strncasecmp	strnicmp
#endif
#if !defined(asprintf) && !defined(__CYGWIN__)
#include "../libbsdio/bstdio.h"
#define asprintf	basprintf
#define vasprintf	bvasprintf
#endif
#define gettimeofday	w32_gettimeofday
#define getuid		w32_getuid
#endif
#if !defined(PATH_MAX)
#define PATH_MAX	1024
#endif

/* Connection */
typedef struct fetchconn conn_t;

struct fetchconn {
	int		 sd;		/* socket descriptor */
	char		*buf;		/* buffer */
	size_t		 bufsize;	/* buffer size */
	size_t		 buflen;	/* length of buffer contents */
	char		*next_buf;	/* pending buffer, e.g. after getln */
	size_t		 next_len;	/* size of pending buffer */
	int		 err;		/* last protocol reply code */
#if defined(HAVE_OPENSSL) && (WITH_SSL)
	SSL		*ssl;		/* SSL handle */
	SSL_CTX		*ssl_ctx;	/* SSL context */
	X509		*ssl_cert;	/* server certificate */
#  if OPENSSL_VERSION_NUMBER < 0x00909000L
	SSL_METHOD *ssl_meth;		/* SSL method */
#  else
	const SSL_METHOD *ssl_meth;	/* SSL method */
#  endif
#endif

	char		*ftp_home;
					/*extension*/
	int		(*ftp_infoparser)(conn_t *, void *);
	void		*ftp_infodata;
#define FEAT_FEAT		(1 << 0)
#define FEAT_LIST		(1 << 1)
#define FEAT_MDTM		(1 << 2)
#define FEAT_MLST		(1 << 3)
#define FEAT_REST_STREAM	(1 << 4)
#define FEAT_SIZE		(1 << 5)
#define FEAT_TVFS		(1 << 6)
	unsigned	ftp_features;

	struct url	*cache_url;
	int		 cache_af;
	int		(*cache_close)(conn_t *);
	conn_t		*next_cached;
};

/* Structure used for error message lists */
struct fetcherr {
	const int	 num;
	const int	 cat;
	const char	*string;
};

void		 fetch_seterr(struct fetcherr *, int);
void		 fetch_syserr(void);
void		 fetch_info(const char *, ...)  LIBFETCH_PRINTFLIKE(1, 2);
int		 fetch_default_port(const char *);
int		 fetch_default_proxy_port(const char *);
int		 fetch_bind(int, int, const char *);
conn_t		*fetch_cache_get(const struct url *, int);
void		 fetch_cache_put(conn_t *, int (*)(conn_t *));
conn_t		*fetch_connect(struct url *, int, int);
conn_t		*fetch_reopen(int);
int		 fetch_ssl(conn_t *, const struct url *, int);
ssize_t		 fetch_read(conn_t *, char *, size_t);
int		 fetch_getln(conn_t *);
ssize_t		 fetch_write(conn_t *, const void *, size_t);
int		 fetch_close(conn_t *);
int		 fetch_add_entry(struct url_list *, struct url *, const char *, int);
int		 fetch_add_entry2(struct url_list *ue, struct url *base, 
			const char *name, const struct url_stat *sb, int pre_quoted);
int		 fetch_netrc_auth(struct url *url);
int		 fetch_no_proxy_match(const char *);
int		 fetch_urlpath_safe(char);

					/*extension*/
int		 fetch_mlsx_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_list_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_eplf_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_unix_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_mnet_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_win32_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);
int		 fetch_dos_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf);

int		 fetch_socketread(int sd, void *buf, size_t len);
int		 fetch_socketclose(int sd);

const char *	 fetchDebugTime(time_t timestamp, char *buffer, unsigned buflen);
const char *	 fetchDebugMode(unsigned mode, char *buffer, unsigned buflen);
const char *	 fetchDebugSize(off_t size, char *buffer, unsigned buflen);

#define ftp_seterr(n)	 fetch_seterr(ftp_errlist, n)
#define http_seterr(n)	 fetch_seterr(http_errlist, n)
#define netdb_seterr(n)	 fetch_seterr(netdb_errlist, n)
#define url_seterr(n)	 fetch_seterr(url_errlist, n)

fetchIO		*fetchIO_unopen(void *, ssize_t (*)(void *, void *, size_t),
    ssize_t (*)(void *, const void *, size_t), void (*)(void *));

/*
 * I don't really like exporting http_request() and ftp_request(),
 * but the HTTP and FTP code occasionally needs to cross-call
 * eachother, and this saves me from adding a lot of special-case code
 * to handle those cases.
 *
 * Note that _*_request() free purl, which is way ugly but saves us a
 * whole lot of trouble.
 */
fetchIO		*http_request(struct url *, const char *,
		     struct url_stat *, struct url *, const char *);
fetchIO		*ftp_request(struct url *, const char *, const char *,
		     struct url_stat *, struct url *, const char *);


/*
 * Check whether a particular flag is set
 */
#define CHECK_FLAG(x)	(flags && strchr(flags, (x)))

#endif
