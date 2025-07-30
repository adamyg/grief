#include <edidentifier.h>
__CIDENT_RCSID(gr_charsetutil_c,"$Id: charsetutil.c,v 1.18 2025/02/07 05:14:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* utility functions.
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
#include <ctype.h>
#include <assert.h>

#include <libstr.h>                             /* str_...()/sxprintf() */
#include "charsetnames.h"
#include "libchartable.h"

#define ALIAS(__x)          "^" __x,
#if defined(_WIN32) || defined(WIN32)
#define EXTALIAS(__x)       "^" __x,
#else
#define EXTALIAS(__x)       /*nothing*/
#endif

static const char * const   charsetascii[] = {  /* US-ASCII (IANA preferred name) */
        "US-ASCII",
            ALIAS("ASCII")
            ALIAS("ANSI_X3.4-1968")
            ALIAS("ANSI_X3.4-1986")
            ALIAS("646")
            ALIAS("ISO646")
            ALIAS("ISO_646.IRV")
            EXTALIAS("ASCII-7")
            EXTALIAS("ASCII-8")
            EXTALIAS("ISO646-US")
            EXTALIAS("iso-ir-6")
            EXTALIAS("csASCII")
            EXTALIAS("us")
            ALIAS("POSIX")
            ALIAS("C")
        NULL
        };

static const char * const   charsetstd[] = {    /* Other Standards */
        "ISO-8859-1",
            ALIAS("Latin-1")
            EXTALIAS("Western European")
        "ISO-8859-2",
            ALIAS("Latin-2")
            EXTALIAS("Eastern European")
            EXTALIAS("Croatian")
            EXTALIAS("Czech")
            EXTALIAS("Hrvatski")
            EXTALIAS("Hungarian")
            EXTALIAS("Polish")
            EXTALIAS("Romanian")
            EXTALIAS("Slovak")
            EXTALIAS("Slovene")
        "ISO-8859-3",
            ALIAS("Latin-3")
            EXTALIAS("South European")
        "ISO-8859-4",
            ALIAS("Latin-4")
            EXTALIAS("Northern European")
        "ISO-8859-5",
            EXTALIAS("Latin/Cyrillic")
            EXTALIAS("Cyrillic")
            EXTALIAS("Russian")
            EXTALIAS("Slavic")
            EXTALIAS("Belarusian")
            EXTALIAS("Bulgarian")
            EXTALIAS("Macedonian")
            EXTALIAS("Serbian")
            EXTALIAS("Ukrainian")
        "ISO-8859-6",
            EXTALIAS("Latin/Arabic")
            EXTALIAS("Arabic")
        "ISO-8859-7",
            EXTALIAS("Latin/Greek")
            EXTALIAS("Greek")
        "ISO-8859-8",
            EXTALIAS("Latin/Hebrew")
            EXTALIAS("Hebrew")
        "ISO-8859-9",
            ALIAS("Latin-5")
            EXTALIAS("Turish")
        "ISO-8859-10",
            ALIAS("Latin-6")
        "ISO-8859-11",
            ALIAS("TIS-620")
            ALIAS("TIS620.2533")
            EXTALIAS("Latin/Thai")
            EXTALIAS("Thai")
        "ISO-8859-13",
            ALIAS("Latin-7")
            EXTALIAS("Baltic Rim")
            EXTALIAS("Lithuanian")
        "ISO-8859-14",
            ALIAS("Latin-8")
            EXTALIAS("Celtic")
        "ISO-8859-15",
            ALIAS("Latin-9")
            EXTALIAS("Latin-0")
        "ISO-8859-16",
            ALIAS("Latin-10")
            EXTALIAS("South-Eastern European")

        "UTF-8",
            EXTALIAS("Unicode")
        "BOCU-1",
        "SCSU",

        "EBCDIC",

        "VSCII",
        "ISCII",
        "TSCII",

        "Shift JIS",
        "EUC-JP",
        "ISO-2022-JP",

        "KOI8-R",
        "KOI8-U",
        "KO17",

        "GBK",
        "GB2312",
        "GB18030",
        "HKSCS",
            "^Big5HKSCS",

        "EUC-TW",
        "Big5",

        "KS-X-1001",
        "EUC-KR",
        "ISO-2022-KR",

        "ANSEL",
        NULL
        };

static int                  compare(const char * const *names, const char *name, size_t namelen, char *buffer, size_t bufsiz);

static void                 strpush(char *buffer, const char *src, size_t blen, size_t slen);


/*  Functions:              charset_canonicalize
 *      Canonicalize against a set of "well-known"/MINE character-set aliases.
 *
 *      MINE assigned character sets
 *
 *          http://www.iana.org/assignments/character-sets
 *
 *  Parameters:
 *      name -                  Character-set name.
 *      buffer -                Working buffer address.
 *      bufsiz -                Buffer size, in bytes (>= 16).
 *
 *  Aliases:
 *      US-ASCII                ASCII, POSIX, C
 *      ISO-8859-X              8859-X
 *      WINDOWS-12xx            CP12xx, 12xx
 *      CPxxx                   CP-xxx, [4789]xx, IBMxxx
 *
 *  Returns:
 *      Normalised name, otherwise NULL.
 */
const char *
charset_canonicalize(const char *name, size_t namelen, char *buffer, size_t bufsiz)
{
    char t_name[CS_NAMELEN+1];
    const char *end = name + (namelen > 0 ? namelen : strlen(name));
    size_t t_namelen = 0;

    assert(bufsiz >= 16);
    while (name < end && *name && t_namelen < CS_NAMELEN) {
        if ('\\' == (t_name[t_namelen] = *name++)) {
            if (name < end && *name) {          /* consume escaped characters */
                t_name[t_namelen] = *name++;
            }
        }
        ++t_namelen;
    }
    t_name[t_namelen] = 0;
    name = t_name;
    namelen = t_namelen;

    if (0 == compare(charsetascii, name, namelen, buffer, bufsiz) &&
            0 == compare(charsetstd, name, namelen, buffer, bufsiz) &&
            0 == compare(charsetnames, name, namelen, buffer, bufsiz)) {

        if (namelen >= 6 && namelen <= 7 &&
                    0 == strncmp(name, "8859-", 5)) {
            strcpy(buffer, "ISO-");             /* ISO-8859-X */
            strpush(buffer+4, name, bufsiz-4, namelen);

        } else if (6 == namelen &&
                    0 == str_nicmp(name, "CP12", 4) && isdigit(name[4]) && isdigit(name[5])) {
            strcpy(buffer, "WINDOWS-");         /* WINDOWS-12xx */
            strpush(buffer+8, name, bufsiz-8, namelen);

        } else if (4 == namelen &&
                    0 == strncmp(name, "12", 2)     && isdigit(name[2]) && isdigit(name[3])) {
            strcpy(buffer, "WINDOWS-");         /* WINDOWS-12xx */
            strpush(buffer+8, name, bufsiz-8, namelen);

        } else if (6 == namelen &&
                    0 == str_nicmp(name, "CP-", 3)  && isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5])) {
            strcpy(buffer, "CP");               /* CPXXX */
            strpush(buffer+2, name+3, bufsiz-2, namelen-3);

        } else if (3 == namelen &&
                    strchr("4789", name[0])         && isdigit(name[1]) && isdigit(name[2])) {
            strcpy(buffer, "CP");               /* CPXXX */
            strpush(buffer+2, name, bufsiz-2, namelen);

        } else if (6 == namelen &&
                    0 == str_nicmp(name, "IBM", 3)  && isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5])) {
            strcpy(buffer, "CP");               /* CPXXX */
            strpush(buffer+2, name+3, bufsiz-2, namelen-3);

        } else {
            return NULL;

        }
    }
    return buffer;
}


static int
compare(const char * const *names, const char *name, size_t namelen, char *buffer, size_t bufsiz)
{
    const char *codeset, *first = names[0];
    unsigned idx;

    for (idx = 0; NULL != (codeset = names[idx]); ++idx) {
        if (('^' != *codeset && 0 == charset_compare(first = codeset, name, namelen)) ||
                ('^' == *codeset && 0 == charset_compare(codeset + 1, name, namelen))) {
            strxcpy(buffer, first, bufsiz);
            return 1;                           /* match, return base-name */
        }
    }
    return 0;
}


static void
strpush(char *buffer, const char *src, size_t blen, size_t slen)
{
    if (blen > 1) {
        while (*src && blen-- > 1 && slen-- > 0) {
            *buffer++ = *src++;
        }
        *buffer++ = 0;
    }
}


/*  Function:               charset_compare
 *      Compare two character-set names.
 *
 *      Following the Unicode Technical report #22 rules.
 *
 *          1. Ignore all characters except a-z, A-Z and 0-9.
 *          2. Compress leading zeros, for example 008 -> 08.
 *          3. Case insensitive comparsion of all characters.
 *
 *      For examples "UTF8", "UTF-8" and "utf-8" are all equivalent.
 *
 *  Parameters:
 *      primary -               Primary character set name.
 *      name -                  Name against which to comare.
 *      namelen -               Name length in bytes, -1 length is derived.
 *
 *  Returns:
 *      0 if matched, otherwise a lexigraphical based -1 or 1.
 */
int
charset_compare(const char *primary, const char *name, size_t namelen)
{
    int ret = 1;

    assert(primary);
    assert(name && namelen);

    for (;;) {
        int pch = 0, nch = 0;

        /* ignore non alpha-numeric and leading zeros */
        while (0 != (pch = primary[0]) &&
                (!isalnum(pch) || ('0' == pch && '0' == primary[1]))) {
            ++primary;
        }

        while (namelen > 0 && 0 != (nch = name[0]) &&
                (!isalnum(nch) || ('0' == nch && '0' == name[1]))) {
            --namelen;
            ++name;
        }

        /* case-insensitive name match */
        if (0 != (pch = primary[0]) &&
                (namelen > 0 && 0 != (nch = name[0]))) {
            if (pch == nch ||
                    (pch = tolower(pch)) == (nch = tolower(nch))) {
                ++primary;
                ++name; --namelen;
                continue;                       /* character match */
            }
        } else {
            if (namelen > 0) {
                nch = name[0];
            } else if (0 == pch) {
                ret = 0;                        /* EOS on both */
                break;
            }
        }
        ret = (pch > nch ? 1 : -1);
        break;
    }
    return ret;
}

/*end*/
