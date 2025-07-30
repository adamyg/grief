#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetlocale_c,"$Id: charsetlocale.c,v 1.17 2025/02/07 05:14:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* libcharset locale map.
 *
 *
 * Copyright (c) 2010 - 2025, Adam Young.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libchartable.h"
#include "charsetlocales.h"

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif

static const char ISO8859_1[]   = "iso8859-1";
static const char ISO8859_2[]   = "iso8859-2";
static const char ISO8859_3[]   = "iso8859-3";
static const char ISO8859_4[]   = "iso8859-4";
static const char ISO8859_5[]   = "iso8859-5";
static const char ISO8859_6[]   = "iso8859-6";
static const char ISO8859_7[]   = "iso8859-7";
static const char ISO8859_8[]   = "iso8859-8";
static const char ISO8859_9[]   = "iso8859-9";
static const char ISO8859_10[]  = "iso8859-10";
static const char ISO8859_11[]  = "iso8859-11";
static const char ISO8859_12[]  = "iso8859-12";
static const char ISO8859_13[]  = "iso8859-13";
static const char ISO8859_14[]  = "iso8859-14";
static const char ISO8859_15[]  = "iso8859-15";
static const char ISO8859_16[]  = "iso8859-16";

static const struct {
    const char *            name;
    const char *            encoding;
} terrories[] = {
    { "AT", ISO8859_15 },
    { "BE", ISO8859_15 },
    { "DE", ISO8859_15 },
    { "ES", ISO8859_15 },
    { "FI", ISO8859_15 },
    { "FR", ISO8859_15 },
    { "GR", ISO8859_7  },
    { "IE", ISO8859_15 },
    { "IT", ISO8859_15 },
    { "LU", ISO8859_15 },
    { "NL", ISO8859_15 },
    { "PT", ISO8859_15 },
    { "ar", ISO8859_6  },
    { "bs", ISO8859_2  },
    { "cs", ISO8859_2  },
    { "cy", ISO8859_14 },
    { "fa", ISO8859_8  },
    { "hr", ISO8859_2  },
    { "hu", ISO8859_2  },
    { "iw", ISO8859_8  },
    { "lt", ISO8859_13 },
    { "lv", ISO8859_13 },
    { "mi", ISO8859_13 },
    { "mk", ISO8859_5  },
    { "mt", ISO8859_3  },
    { "pl", ISO8859_2  },
    { "pl", ISO8859_2  },
    { "ro", ISO8859_2  },
    { "ru", ISO8859_5  },
    { "sk", ISO8859_2  },
    { "sl", ISO8859_2  },
    { "sr", ISO8859_2  },
    { "th", ISO8859_11 },
    { "tr", ISO8859_9  },
    { NULL, NULL       }
    };

static const struct locale_alias *      alias_lookup(const char *locale);
static const struct charset_altname *   altname_lookup(const char *locale);
static const struct charset_altname *   charset_lookup(const char *locale);


/*  Function:               charset_map_locale
 *      Map the specified locale to a normalised character-set.
 *
 *  Parameters:
 *      locale -                Locale speicification.
 *
 *      buffer -                Buffer address, used as working storage.
 *
 *      bufsiz -                Buffer size in bytes (>= 16 required).
 *
 *  Locale Format:
 *
 *          [language[_territory]].[codeset][@modifier]]
 *
 *      where language is an ISO 639 language code, territory is an ISO 3166 country code,
 *      and codeset is a character set or encoding identifier like ISO-8859-1 or UTF-8.
 *
 *      For a list of locales, see locale(1) or www.icu.org
 *
 *  Examples:
 *      en_US.utf8@euro
 *
 *  Returns:
 *      Character-set, otherwise NULL if unknown.
 */
const char *
charset_map_locale(const char *locale, char *buffer, size_t bufsiz)
{
    const char *codeset = NULL;

    if (locale && locale[0]) {
        const struct locale_alias *alias;
        char *t_locale = NULL;

        /* locale lookup */
        if (NULL != (alias = alias_lookup(locale))) {
            codeset = alias->charset;

        } else if (NULL != (t_locale = strdup(locale))) {
            if (NULL != (codeset = strrchr(t_locale, '.'))) {
                const char *t_codeset;
                char *modifier;

                if (NULL != (modifier = strrchr(++codeset, '@'))) {
                    *modifier++ = 0;            /* remove modifier */
                }

                if (NULL != (t_codeset = charset_canonicalize(codeset, -1, buffer, bufsiz)) ||
                        NULL != (t_codeset = charset_alias_lookup(codeset, -1))) {
                    codeset = t_codeset;

                } else {
                    const struct charset_altname *altname;

                    if (NULL != (altname = charset_lookup(codeset)) ||
                            NULL != (altname = altname_lookup(codeset))) {
                        codeset = altname->charset;
                    }
                }
            }
            free(t_locale);
        }

        /* country codes, fall-back logic */
        if (NULL == codeset) {
            char terrory[3] = {0};
            const char *name;

            if (2 == strlen(locale)) {
                memcpy(terrory, locale, 2);
            } else if (NULL != (name = strchr(locale, '_')) && *++name) {
                memcpy(terrory, name, 2);
            }

            if (terrory[0]) {
                unsigned i;

                for (i = 0; NULL != (name = terrories[i].name); ++i) {
                    if (0 == strcmp(name, terrory)) {
                        codeset = terrories[i].encoding;
                        break;
                    }
                }
            }
        }
    }
    return codeset;
}


static int
alias_compare(void const *key, void const *val)
{
    const struct locale_alias *alias = val;
    return strcmp(key, alias->locale);
}


static const struct locale_alias *
alias_lookup(const char *locale)
{
    return (struct locale_alias *)bsearch((void *)locale, locale_alias_table,
                sizeof(locale_alias_table)/sizeof(locale_alias_table[0]),
                    sizeof(locale_alias_table[0]), alias_compare);
}


static int
altname_compare(void const *key, void const *val)
{
    const struct charset_altname *rec = val;
    return strcmp(key, rec->altname);
}


static const struct charset_altname *
altname_lookup(const char *altname)
{
    return (struct charset_altname *)bsearch((void *)altname, charset_altname_table,
                sizeof(charset_altname_table)/sizeof(charset_altname_table[0]),
                    sizeof(charset_altname_table[0]), altname_compare);
}


static const struct charset_altname *
charset_lookup(const char *charset)
{
    const struct charset_altname *cursor = charset_altname_table,
            *end = (cursor + sizeof(charset_altname_table)/sizeof(charset_altname_table[0]));

    while (cursor < end) {
        if (0 == charset_compare(charset, cursor->charset, strlen(cursor->charset))) {
            return cursor;
        }
        ++cursor;
    }
    return NULL;
}
