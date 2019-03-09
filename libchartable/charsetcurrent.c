#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetcurrent_c,"$Id: charsetcurrent.c,v 1.8 2018/10/01 22:10:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* libcharset current.
 *
 *
 * Copyright (c) 2010 - 2018, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include "config.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#if defined(_CYGWIN__) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(OS2)
#define  INCL_DOS
#include <os2.h>
#endif

#include "libchartable.h"

#if (0)
#define __DEBUG(x)          printf x;
#else
#define __DEBUG(x)
#endif


const char *
charset_terminal_encoding(void)
{
    return charset_current("", "");
}


const char *
charset_text_encoding(void)
{
    return charset_current("TEXTLANG", "");
}


const char *
charset_current(const char *env, const char *def)
{
    static char x_locale[80], x_alias[80];      /* local working buffers */
    const char *codeset = NULL;

#if defined(HAVE_LANGINFO_CODESET)
    if (NULL == env) {
        codeset = nl_langinfo(CODESET);
#if defined(__CYGWIN__)                         /* cygwin default, ignore */
        if (codeset && 0 == strcmp(codeset, "US-ASCII")) {
            codeset = NULL;
        }
#endif
    }
#endif

    if (NULL == codeset || 0 == codeset[0]) {
        /*
         *  [language[_territory]].[codeset][@modifier]]
         */
        const char *locale = NULL;
                                                /* old-style, query environment */
        if (NULL == env || NULL == (locale = getenv(env)) || 0 == locale[0]) {
            if (NULL == (locale = getenv("LC_ALL")) || 0 == locale[0]) {
                if (NULL == (locale = getenv("LC_CTYPE")) || 0 == locale[0]) {
                    locale = getenv("LANG");
                }
            }
        }

        if (locale && locale[0]) {
            if (NULL != (codeset =
                    charset_map_locale(locale, x_locale, sizeof(x_locale)))) {
                return codeset;
            }
        }

        codeset = locale;
    }

#if defined(_CYGWIN__) || defined(WIN32)
    if (NULL == codeset || 0 == codeset[0]) {
        unsigned acp = (unsigned)GetACP();      /* active code page */

        if (acp > 0) {
            sprintf(x_locale, "CP%u", GetACP());
            codeset = x_locale;
        }
    }
#endif

#if defined(OS2)
    if (NULL == codeset || 0 == codeset[0]) {
        ULONG cp[3] = {0}, cplen = 0;           /* UNTESTED */

        if (DosQueryCp(sizeof(cp), cp, &cplen)) {
            sprintf(x_locale, "CP%u", (unsigned) cp[0]);
            codeset = x_locale;
        }
    }
#endif

    if (codeset && codeset[0]) {
        const char *t_codeset;

        if (NULL != (t_codeset = charset_canonicalize(codeset, -1, x_alias, sizeof(x_alias))) ||
                NULL != (t_codeset = charset_alias_lookup(codeset, -1))) {
            return t_codeset;
        }
        return codeset;
    }
    return def;
}

