/* $Id: libguess.c,v 1.4 2015/03/01 02:46:47 cvsuser Exp $
 *
 * libguess implementation for windows.
 *
 * Copyright (c) 2012-2015 Adam Young.
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
 * ==end==
 */

#include <libguess.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

LIBGUESS_LINKAGE const char * LIBGUESS_ENTRY 
libguess_determine_encoding(const char *inbuf, int buflen, const char *lang)
{
    const static struct encoding {
        const char *region;
        const char *lang;
        guess_impl_f guess;
    } encodings[] = {
        { "jp", GUESS_REGION_JP, guess_jp },
        { "tw", GUESS_REGION_TW, guess_tw },
        { "cn", GUESS_REGION_CN, guess_cn },
        { "kr", GUESS_REGION_KR, guess_kr },
        { "ru", GUESS_REGION_RU, guess_ru },
        { "ar", GUESS_REGION_AR, guess_ar },
        { "tr", GUESS_REGION_TR, guess_tr },
        { "gr", GUESS_REGION_GR, guess_gr },
        { "hw", GUESS_REGION_HW, guess_hw },
        { "pl", GUESS_REGION_PL, guess_pl },
        { "bl", GUESS_REGION_BL, guess_bl }
        };
    const struct encoding *e = encodings;
    unsigned i;

#if defined(HAVE_STRCASECMP) && !defined(WIN32)
#define COMPARE     strcasecmp
#else
#define COMPARE     _stricmp
#endif

    if (lang) {
        if (lang[1] && 0 == lang[2]) {          /* extension */
            const int c0 = tolower(*((unsigned char *)lang)), 
                    c1 = tolower(*((unsigned char *)lang + 1));

            for (i = 0; i < (sizeof(encodings)/sizeof(encodings[0])); ++i, ++e) {
                if (c0 == e->region[0] && c1 == e->region[1]) {
                    return (*e->guess)(inbuf, buflen);
                }
            }


        } else {
            const int c = tolower(*((unsigned char *)lang)); 
            
            for (i = 0; i < (sizeof(encodings)/sizeof(encodings[0])); ++i, ++e) {
                if (c == *e->lang && 0 == COMPARE(lang, e->lang)) {
                    return (*e->guess)(inbuf, buflen);
                }
            }
        }

    } else {                                    /* extension */
        for (i = 0; i < (sizeof(encodings)/sizeof(encodings[0])); ++i, ++e) {
            if (0 == COMPARE(lang, e->lang)) {
                const char *res = (*e->guess)(inbuf, buflen);
                if (res) res;
            }
        }
    }
    return NULL;
}
/*end*/


