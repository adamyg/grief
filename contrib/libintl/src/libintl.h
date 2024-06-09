/*	$NetBSD: libintl.h,v 1.8 2015/06/08 15:04:20 christos Exp $	*/

/*-
 * Copyright (c) 2000 Citrus Project,
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
 */

#ifndef __LIBINTL_H_DEFINED__
#define __LIBINTL_H_DEFINED__

#include <sys/cdefs.h>
#ifndef __format_arg
#define __format_arg(__cnt)	/**/
#endif
#ifndef __packed
#define __packed /**/
#endif

#if defined(LIBINTL_STATIC)
#   define LIBINTL_LINKAGE
#   define LIBINTL_ENTRY
#elif defined(WIN32) || defined(_WIN32)
#   if defined(__LIBINTL_BUILD)
#       define LIBINTL_LINKAGE __declspec(dllexport)
#   else
#       define LIBINTL_LINKAGE __declspec(dllimport)
#   endif
#   define LIBINTL_ENTRY __cdecl
#else
#   define LIBINTL_LINKAGE
#   define LIBINTL_ENTRY
#endif

#ifndef _LIBGETTEXT_H
/*
 * Avoid defining these if the GNU gettext compatibility header includes
 * us, since it re-defines those unconditionally and creates inline functions
 * for some of them. This is horrible.
 */
#define pgettext_expr(msgctxt, msgid) pgettext((msgctxt), (msgid))
#define dpgettext_expr(domainname, msgctxt, msgid) \
    dpgettext((domainname), (msgctxt), (msgid))
#define dcpgettext_expr(domainname, msgctxt, msgid, category) \
    dcpgettext((domainname), (msgctxt), (msgid), (category))
#define npgettext_expr(msgctxt, msgid1, msgid2, n) \
    npgettext((msgctxt), (msgid1), (msgid2), (n))
#define dnpgettext_expr(domainname, msgctxt, msgid1, n) \
    dnpgettext((domainname), (msgctxt), (msgid1), (msgid2), (n))
#define dcnpgettext_expr(domainname, msgctxt, msgid1, msgid2, n, category) \
    dcnpgettext((domainname), (msgctxt), (msgid1), (msgid2), (n), (category))
#endif

__BEGIN_DECLS
LIBINTL_LINKAGE char * LIBINTL_ENTRY gettext(const char *) __format_arg(1);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dgettext(const char *, const char *) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dcgettext(const char *, const char *, int) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY ngettext(const char *, const char *, unsigned long int)
			                    __format_arg(1) __format_arg(2);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dngettext(const char *, const char *, const char *, unsigned long int)
			                    __format_arg(2) __format_arg(3);
LIBINTL_LINKAGE char * LIBINTL_ENTRY dcngettext(const char *, const char *, const char *, unsigned long int, int) 
			                    __format_arg(2) __format_arg(3);

LIBINTL_LINKAGE const char * LIBINTL_ENTRY pgettext(const char *, const char *) __format_arg(2);
LIBINTL_LINKAGE const char * LIBINTL_ENTRY dpgettext(const char *, const char *, const char *)
		      __format_arg(3);
LIBINTL_LINKAGE const char * LIBINTL_ENTRY dcpgettext(const char *, const char *, const char *, int)
		       __format_arg(3);
LIBINTL_LINKAGE const char * LIBINTL_ENTRY npgettext(const char *, const char *, const char *,
		      unsigned long int) __format_arg(2) __format_arg(3);
LIBINTL_LINKAGE const char * LIBINTL_ENTRY dnpgettext(const char *, const char *, const char *,
		       const char *, unsigned long int) __format_arg(3)
		       __format_arg(4);
LIBINTL_LINKAGE const char * LIBINTL_ENTRY dcnpgettext(const char *, const char *, const char *,
			const char *, unsigned long int, int) __format_arg(3)
			__format_arg(4);

LIBINTL_LINKAGE char * LIBINTL_ENTRY textdomain(const char *);
LIBINTL_LINKAGE char * LIBINTL_ENTRY bindtextdomain(const char *, const char *);
LIBINTL_LINKAGE char * LIBINTL_ENTRY bind_textdomain_codeset(const char *, const char *);

__END_DECLS

#endif /*__LIBINTL_H_DEFINED__*/
