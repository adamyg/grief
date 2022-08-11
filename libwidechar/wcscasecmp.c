/*-
 * Copyright (c)1999 Citrus Project,
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

#include "widechar.h"
#include <wctype.h>

int
Wcscasecmp(const WChar_t *s1, const WChar_t *s2)
{
	WChar_t l1, l2;

	while ((l1 = towlower((wint_t)(*s1++))) == (l2 = towlower((wint_t)(*s2++)))) { //FIXME
		if (l1 == 0)
			return (0);
	}
	return ((int)l1 - (int)l2);
}

int
Wcsncasecmp(const WChar_t *s1, const WChar_t *s2, size_t n)
{
	WChar_t l1, l2;

	if (n == 0)
		return (0);
	do {
		if (((l1 = towlower((wint_t)(*s1++)))) != (l2 = towlower((wint_t)(*s2++)))) { //FIXME
			return ((int)l1 - (int)l2);
		}
		if (l1 == 0)
			break;
	} while (--n != 0);
	return (0);
}
