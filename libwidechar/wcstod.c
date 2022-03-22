/*-
 * Copyright (c) 2002 Tim J. Robbins
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

#include <stdlib.h>
#include <ctype.h>

double
Wcstod(const WChar_t *nptr, WChar_t **endptr)
{
	const WChar_t *norg = nptr;
	char *buf, *end = NULL;
	double val;
	int len;

	while (*nptr < 0xff && isspace(*nptr)) {
		++nptr;
	}

	/*
	 * Convert the supplied wide char string to multibyte.
	 * Note: could be optimised to limit converted text.
	 */
	if ((len = Wcstoutf8(nptr, NULL, 0)) <= 0 ||
			NULL == (buf = (char *)malloc(len + 1/*+nul*/))) {
		if (endptr != NULL)
			*endptr = (WChar_t *)nptr;
		return 0.0;
	}

	(void) Wcstoutf8(nptr, buf, len + 1);

	/*
	 * conversion
	 */
	val = strtod(buf, &end);

	/*
	 * Convert position, numeric are never multi-byte, hence 1:1
	 */
	if (endptr != NULL) {
		if (end == buf) {
			*endptr = (WChar_t *)norg;
		} else {
			*endptr = (WChar_t *)(nptr + (end - buf));
		}
	}
	free(buf);

	return val;
}
