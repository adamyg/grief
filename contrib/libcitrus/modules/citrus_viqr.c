/* $NetBSD: citrus_viqr.c,v 1.6 2013/05/28 16:57:56 joerg Exp $ */

/*-
 * Copyright (c)2006 Citrus Project,
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
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: citrus_viqr.c,v 1.6 2013/05/28 16:57:56 joerg Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"
#include <sys/queue.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <wchar.h>
#include <limits.h>

#include "citrus_namespace.h"
#include "citrus_types.h"
#include "citrus_bcs.h"
#include "citrus_module.h"
#include "citrus_ctype.h"
#include "citrus_stdenc.h"
#include "citrus_viqr.h"

#define ESCAPE	'\\'

/*
 * this table generated from RFC 1456.
 */
static const char *mnemonic_rfc1456[0x100] = {
  NULL , NULL , "A(?", NULL , NULL , "A(~", "A^~", NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , "Y?" , NULL , NULL , NULL ,
  NULL , "Y~" , NULL , NULL , NULL , NULL , "Y." , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL ,
  "A." , "A('", "A(`", "A(.", "A^'", "A^`", "A^?", "A^.",
  "E~" , "E." , "E^'", "E^`", "E^?", "E^~", "E^.", "O^'",
  "O^`", "O^?", "O^~", "O^.", "O+.", "O+'", "O+`", "O+?",
  "I." , "O?" , "O." , "I?" , "U?" , "U~" , "U." , "Y`" ,
  "O~" , "a('", "a(`", "a(.", "a^'", "a^`", "a^?", "a^.",
  "e~" , "e." , "e^'", "e^`", "e^?", "e^~", "e^.", "o^'",
  "o^`", "o^?", "o^~", "O+~", "O+" , "o^.", "o+`", "o+?",
  "i." , "U+.", "U+'", "U+`", "U+?", "o+" , "o+'", "U+" ,
  "A`" , "A'" , "A^" , "A~" , "A?" , "A(" , "a(?", "a(~",
  "E`" , "E'" , "E^" , "E?" , "I`" , "I'" , "I~" , "y`" ,
  "DD" , "u+'", "O`" , "O'" , "O^" , "a." , "y?" , "u+`",
  "u+?", "U`" , "U'" , "y~" , "y." , "Y'" , "o+~", "u+" ,
  "a`" , "a'" , "a^" , "a~" , "a?" , "a(" , "u+~", "a^~",
  "e`" , "e'" , "e^" , "e?" , "i`" , "i'" , "i~" , "i?" ,
  "dd" , "u+.", "o`" , "o'" , "o^" , "o~" , "o?" , "o." ,
  "u." , "u`" , "u'" , "u~" , "u?" , "y'" , "o+.", "U+~",
};

typedef struct {
	const char *name;
	WCHAR_T value;
} mnemonic_def_t;

static const mnemonic_def_t mnemonic_ext[] = {
/* add extra mnemonic here (should be sorted by WCHAR_T order). */
    {0} /*dummy, non-standard zero-size structure*/
};
//static const size_t mnemonic_ext_size =
//      sizeof(mnemonic_ext) / sizeof(mnemonic_def_t);
static const size_t mnemonic_ext_size = 0;

static const char *
mnemonic_ext_find(WCHAR_T wc, const mnemonic_def_t *head, size_t n)
{
	const mnemonic_def_t *mid;

	_DIAGASSERT(head != NULL);

	for (; n > 0; n >>= 1) {
		mid = head + (n >> 1);
		if (mid->value == wc) {
			return mid->name;
		} else if (mid->value < wc) {
			head = mid + 1;
			--n;
		}
	}
	return NULL;
}

struct mnemonic_t;
typedef TAILQ_HEAD(mnemonic_list_t, mnemonic_t) mnemonic_list_t;
typedef struct mnemonic_t {
	TAILQ_ENTRY(mnemonic_t) entry;
	int ascii;
	struct mnemonic_t *parent;
	mnemonic_list_t child;
	WCHAR_T value;
} mnemonic_t;

static mnemonic_t *
mnemonic_list_find(mnemonic_list_t *ml, int ch)
{
	mnemonic_t *m;

	_DIAGASSERT(ml != NULL);

	TAILQ_FOREACH(m, ml, entry) {
		if (m->ascii == ch)
			return m;
	}

	return NULL;
}

static mnemonic_t *
mnemonic_create(mnemonic_t *parent, int ascii, WCHAR_T value)
{
	mnemonic_t *m;

	_DIAGASSERT(parent != NULL);

	m = malloc(sizeof(*m));
	if (m != NULL) {
		m->parent = parent;
		m->ascii = ascii;
		m->value = value;
		TAILQ_INIT(&m->child);
	}

	return m;
}

static int
mnemonic_append_child(mnemonic_t *m, const char *s,
	WCHAR_T value, WCHAR_T invalid)
{
	int ch;
	mnemonic_t *m0;

	_DIAGASSERT(m != NULL);
	_DIAGASSERT(s != NULL);

	ch = (unsigned char)*s++;
	if (ch == '\0')
		return EINVAL;
	m0 = mnemonic_list_find(&m->child, ch);
	if (m0 == NULL) {
		m0 = mnemonic_create(m, ch, (WCHAR_T)ch);
		if (m0 == NULL)
			return ENOMEM;
		TAILQ_INSERT_TAIL(&m->child, m0, entry);
	}
	m = m0;
	for (m0 = NULL; (ch = (unsigned char)*s) != '\0'; ++s) {
		m0 = mnemonic_list_find(&m->child, ch);
		if (m0 == NULL) {
			m0 = mnemonic_create(m, ch, invalid);
			if (m0 == NULL)
				return ENOMEM;
			TAILQ_INSERT_TAIL(&m->child, m0, entry);
		}
		m = m0;
	}
	if (m0 == NULL)
		return EINVAL;
	m0->value = value;

	return 0;
}

static void
mnemonic_destroy(mnemonic_t *m)
{
	mnemonic_t *m0, *tm0;

	_DIAGASSERT(m != NULL);

	TAILQ_FOREACH_SAFE(m0, &m->child, entry, tm0)
		mnemonic_destroy(m0);
	free(m);
}

typedef struct {
	size_t mb_cur_max;
	WCHAR_T invalid;
	mnemonic_t *mroot;
} _VIQREncodingInfo;

typedef struct {
	int chlen;
	char ch[MB_LEN_MAX];
} _VIQRState;

typedef struct {
	_VIQREncodingInfo	ei;
	struct {
		/* for future multi-locale facility */
		_VIQRState	s_mblen;
		_VIQRState	s_mbrlen;
		_VIQRState	s_mbrtowc;
		_VIQRState	s_mbtowc;
		_VIQRState	s_mbsrtowcs;
		_VIQRState	s_mbsnrtowcs;
		_VIQRState	s_wcrtomb;
		_VIQRState	s_wcsrtombs;
		_VIQRState	s_wcsnrtombs;
		_VIQRState	s_wctomb;
	} states;
} _VIQRCTypeInfo;

#define _CEI_TO_EI(_cei_)		(&(_cei_)->ei)
#define _CEI_TO_STATE(_cei_, _func_)	(_cei_)->states.s_##_func_

#define _FUNCNAME(m)			_citrus_VIQR_##m
#define _ENCODING_INFO			_VIQREncodingInfo
#define _CTYPE_INFO			_VIQRCTypeInfo
#define _ENCODING_STATE			_VIQRState
#define _ENCODING_MB_CUR_MAX(_ei_)	(_ei_)->mb_cur_max
#define _ENCODING_IS_STATE_DEPENDENT		1
#define _STATE_NEEDS_EXPLICIT_INIT(_ps_)	0

static __inline void
/*ARGSUSED*/
_citrus_VIQR_init_state(_VIQREncodingInfo * __restrict ei,
	_VIQRState * __restrict psenc)
{
	/* ei may be unused */
	_DIAGASSERT(psenc != NULL);

	psenc->chlen = 0;
}

static __inline void
/*ARGSUSED*/
_citrus_VIQR_pack_state(_VIQREncodingInfo * __restrict ei,
	void *__restrict pspriv, const _VIQRState * __restrict psenc)
{
	/* ei may be unused */
	_DIAGASSERT(pspriv != NULL);
	_DIAGASSERT(psenc != NULL);

	memcpy(pspriv, (const void *)psenc, sizeof(*psenc));
}

static __inline void
/*ARGSUSED*/
_citrus_VIQR_unpack_state(_VIQREncodingInfo * __restrict ei,
	_VIQRState * __restrict psenc, const void * __restrict pspriv)
{
	/* ei may be unused */
	_DIAGASSERT(psenc != NULL);
	_DIAGASSERT(pspriv != NULL);

	memcpy((void *)psenc, pspriv, sizeof(*psenc));
}

static int
_citrus_VIQR_mbrtowc_priv(_VIQREncodingInfo * __restrict ei,
	WCHAR_T * __restrict pwc, const char ** __restrict s, size_t n,
	_VIQRState * __restrict psenc, size_t * __restrict nresult)
{
	const char *s0;
	WCHAR_T wc;
	mnemonic_t *m, *m0;
	size_t i;
	int ch, escape;

	_DIAGASSERT(ei != NULL);
	/* pwc may be null */
	_DIAGASSERT(s != NULL);
	_DIAGASSERT(psenc != NULL);
	_DIAGASSERT(nresult != NULL);

	if (*s == NULL) {
		_citrus_VIQR_init_state(ei, psenc);
		*nresult = (size_t)_ENCODING_IS_STATE_DEPENDENT;
		return 0;
	}
	s0 = *s;

	i = 0;
	m = ei->mroot;
	for (escape = 0;;) {
		if (psenc->chlen == i) {
			if (n-- < 1) {
				*s = s0;
				*nresult = (size_t)-2;
				return 0;
			}
			psenc->ch[psenc->chlen++] = *s0++;
		}
		ch = (unsigned char)psenc->ch[i++];
		if (ch == ESCAPE) {
			if (m != ei->mroot)
				break;
			escape = 1;
			continue;
		}
		if (escape != 0)
			break;
		m0 = mnemonic_list_find(&m->child, ch);
		if (m0 == NULL)
			break;
		m = m0;
	}
	while (m != ei->mroot) {
		--i;
		if (m->value != ei->invalid)
			break;
		m = m->parent;
	}
	if (ch == ESCAPE && m != ei->mroot)
		++i;
	psenc->chlen -= i;
	memmove(&psenc->ch[0], &psenc->ch[i], psenc->chlen);
	wc = (m == ei->mroot) ? (WCHAR_T)ch : m->value;
	if (pwc != NULL)
		*pwc = wc;
	*nresult = (size_t)(wc == 0 ? 0 : s0 - *s);
	*s = s0;

	return 0;
}

static int
_citrus_VIQR_wcrtomb_priv(_VIQREncodingInfo * __restrict ei,
	char * __restrict s, size_t n, WCHAR_T wc,
	_VIQRState * __restrict psenc, size_t * __restrict nresult)
{
	mnemonic_t *m;
	int ch = 0;
	const char *p;

	_DIAGASSERT(ei != NULL);
	_DIAGASSERT(s != NULL);
	_DIAGASSERT(psenc != NULL);
	_DIAGASSERT(nresult != NULL);

	switch (psenc->chlen) {
	case 0: case 1:
		break;
	default:
		return EINVAL;
	}
	m = NULL;
	if ((uint32_t)wc <= 0xFF) {
		p = mnemonic_rfc1456[wc & 0xFF];
		if (p != NULL)
			goto mnemonic_found;
		if (n-- < 1)
			goto e2big;
		ch = (unsigned int)wc;
		m = ei->mroot;
		if (psenc->chlen > 0) {
			m = mnemonic_list_find(&m->child, psenc->ch[0]);
			if (m == NULL)
				return EINVAL;
			psenc->ch[0] = ESCAPE;
		}
		if (mnemonic_list_find(&m->child, ch) == NULL) {
			psenc->chlen = 0;
			m = NULL;
		}
		psenc->ch[psenc->chlen++] = ch;
	} else {
		p = mnemonic_ext_find(wc, &mnemonic_ext[0], mnemonic_ext_size);
		if (p == NULL) {
			*nresult = (size_t)-1;
			return EILSEQ;
		} else {
mnemonic_found:
			psenc->chlen = 0;
			while (*p != '\0') {
				if (n-- < 1)
					goto e2big;
				psenc->ch[psenc->chlen++] = *p++;
			}
		}
	}
	memcpy(s, psenc->ch, psenc->chlen);
	*nresult = psenc->chlen;
	if (m == ei->mroot) {
		psenc->ch[0] = ch;
		psenc->chlen = 1;
	} else {
		psenc->chlen = 0;
	}

	return 0;

e2big:
	*nresult = (size_t)-1;
	return E2BIG;
}

static int
/* ARGSUSED */
_citrus_VIQR_put_state_reset(_VIQREncodingInfo * __restrict ei,
	char * __restrict s, size_t n, _VIQRState * __restrict psenc,
	size_t * __restrict nresult)
{
	/* ei may be unused */
	_DIAGASSERT(s != NULL);
	_DIAGASSERT(psenc != NULL);
	_DIAGASSERT(nresult != NULL);

	switch (psenc->chlen) {
	case 0: case 1:
		break;
	default:
		return EINVAL;
	}
	*nresult = 0;
	psenc->chlen = 0;

	return 0;
}

static __inline int
/*ARGSUSED*/
_citrus_VIQR_stdenc_wctocs(_VIQREncodingInfo * __restrict ei,
	_csid_t * __restrict csid, _index_t * __restrict idx, WCHAR_T wc)
{
	/* ei may be unused */
	_DIAGASSERT(csid != NULL);
	_DIAGASSERT(idx != NULL);

	*csid = 0;
	*idx = (_index_t)wc;

	return 0;
}

static __inline int
/*ARGSUSED*/
_citrus_VIQR_stdenc_cstowc(_VIQREncodingInfo * __restrict ei,
	WCHAR_T * __restrict pwc, _csid_t csid, _index_t idx)
{
	/* ei may be unused */
	_DIAGASSERT(pwc != NULL);

	if (csid != 0)
		return EILSEQ;
	*pwc = (WCHAR_T)idx;

	return 0;
}

static void
_citrus_VIQR_encoding_module_uninit(_VIQREncodingInfo *ei)
{
	_DIAGASSERT(ei != NULL);

	mnemonic_destroy(ei->mroot);
}

static int
/*ARGSUSED*/
_citrus_VIQR_encoding_module_init(_VIQREncodingInfo * __restrict ei,
	const void * __restrict var, size_t lenvar)
{
	int errnum;
	const char *s;
	size_t i, n;
	const mnemonic_def_t *p;

	_DIAGASSERT(ei != NULL);
	/* var may be unused */

	ei->mb_cur_max = 1;
	ei->invalid = (WCHAR_T)-1;
	ei->mroot = mnemonic_create(NULL, '\0', ei->invalid);
	if (ei->mroot == NULL)
		return ENOMEM;
	for (i = 0; i < sizeof(mnemonic_rfc1456) / sizeof(const char *); ++i) {
		s = mnemonic_rfc1456[i];
		if (s == NULL)
			continue;
		n = strlen(s);
		_DIAGASSERT(n <= MB_LEN_MAX);
		if (ei->mb_cur_max < n)
			ei->mb_cur_max = n;
		errnum = mnemonic_append_child(ei->mroot,
		    s, (WCHAR_T)i, ei->invalid);
		if (errnum != 0) {
			_citrus_VIQR_encoding_module_uninit(ei);
			return errnum;
		}
	}
	for (i = 0; i < mnemonic_ext_size; ++i) {
		p = &mnemonic_ext[i];
		_DIAGASSERT(p != NULL && p->name != NULL);
		n = strlen(p->name);
		_DIAGASSERT(n <= MB_LEN_MAX);
		if (ei->mb_cur_max < n)
			ei->mb_cur_max = n;
		errnum = mnemonic_append_child(ei->mroot,
		    p->name, p->value, ei->invalid);
		if (errnum != 0) {
			_citrus_VIQR_encoding_module_uninit(ei);
			return errnum;
		}
	}

	return 0;
}

static __inline int
/*ARGSUSED*/
_citrus_VIQR_stdenc_get_state_desc_generic(_VIQREncodingInfo * __restrict ei,
	_VIQRState * __restrict psenc, int * __restrict rstate)
{
	/* ei may be unused */
	_DIAGASSERT(psenc != NULL);
	_DIAGASSERT(rstate != NULL);

	*rstate = (psenc->chlen == 0)
	    ? _STDENC_SDGEN_INITIAL
	    : _STDENC_SDGEN_INCOMPLETE_CHAR;

	return 0;
}

/* ----------------------------------------------------------------------
 * public interface for ctype
 */

_CITRUS_CTYPE_DECLS(VIQR);
_CITRUS_CTYPE_DEF_OPS(VIQR);

#include "citrus_ctype_template.h"

/* ----------------------------------------------------------------------
 * public interface for stdenc
 */

_CITRUS_STDENC_DECLS(VIQR);
_CITRUS_STDENC_DEF_OPS(VIQR);

#include "citrus_stdenc_template.h"
