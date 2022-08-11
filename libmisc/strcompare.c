#include <edidentifier.h>
__CIDENT_RCSID(gr_strcompare_c,"$Id: strcompare.c,v 1.15 2022/08/10 14:25:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strcompare.c,v 1.15 2022/08/10 14:25:21 cvsuser Exp $
 * libstr - String compare utility functions.
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <config.h>

#if defined(HAVE_STRVERSCMP) && !defined(_GNU_SOURCE)
#if defined(linux) || defined(__CYGWIN__) //FIXME
#define _GNU_SOURCE
#endif
#endif

#include <editor.h>
#include <edtypes.h>
#include <assert.h>

#if defined(HAVE_STRVERSCMP)
#include <string.h>                             /* strverscmp() */
#endif
#include <libstr.h>
#include <unistd.h>


/*  Function:           str_icmp
 *      Compares two strings with case insensitivity.
 *
 *  Description:
 *      The str_icmp routine compares two strings, one pointed to by 's1' and the other
 *      string pointed to by 's2', with case insensitivity.
 *
 *  Parameter(s):
 *      s1 - Address of the first string.
 *      s2 - Address of the second string.
 *
 *  Returns:
 *      The str_icmp routine returns a value less than, equal to, or greater than zero
 *      (0). A returned value of less than zero (0) indicates that the first string is
 *      lexicographically less than the second string. A returned value of zero (0)
 *      means that both strings are equal. A returned value of greater than zero means
 *      that the first string is lexicographically greater than the second string.
 */
int
str_icmp(const char *s1, const char *s2)
{
    if (s1 != s2) {
        register const unsigned char *_s1 = (const unsigned char *)s1,
                *_s2 = (const unsigned char *)s2;

        while ((*_s1 == *_s2) || (tolower(*_s1) == tolower(*_s2))) {
            if ('\0' == *_s1++) {
                return 0;                       /* match */
            }
            ++_s2;
        }
        return (tolower(*_s1) - tolower(*_s2));
    }
    return 0;                                   /* match */
}


/*  Function:           str_nicmp
 *      Compares two strings up to a limit with case insensitivity.
 *
 *  Description:
 *      The str_nicmp routine compares two string,s one pointed to by 's1' and the other
 *      string pointed to by 's2', with case insensitivity, up to the length specified by
 *      'len'.
 *
 *  Parameters:
 *      s1 - Address of the first string.
 *      s2 - Address of the second string.
 *      len - Number of characters to compare up to.
 *
 *  Returns:
 *      The str_nicmp routine returns a value less than, equal to, or greater than zero
 *      (0). A returned value of less than zero (0) indicates that the first string is
 *      lexicographically less than the second string. A returned value of zero (0) means
 *      that both strings are equal. A returned value of greater than zero means that the
 *      first string is lexicographically greater than the second string.
 */
int
str_nicmp(const char *s1, const char *s2, int len)
{
    if (s1 != s2 && len > 0) {
        register const unsigned char *_s1 = (const unsigned char *)s1,
                *_s2 = (const unsigned char *)s2;

        do {                                    /* diff */
            if ((*_s1 != *_s2) && (tolower(*_s1) != tolower(*_s2))) {
                return (tolower(*_s1) - tolower(*_s2));
            }
            if ('\0' == *_s1++) {
                break;
            }
            ++_s2;
        } while (--len != 0);
    }
    return 0;                                   /* match */
}


/*  Function:           str_verscmp
 *      strverscmp(3)/versionsort(3) style version comparsion function.
 *
 *      The strverscmp() function returns an integer less than, equal to, or greater than zero if
 *      s1 is found, respectively, to be earlier than, equal to, or later than s2.
 *
 *  Parameters:
 *      s1 - Address of the first string.
 *      s2 - Address of the second string.
 *
 *   Returns:
 *      The str_verscmp() function returns an integer less than, equal to, or greater than zero
 *      if s1 is found, respectively, to be earlier than, equal to, or later than s2.
 */
int
str_verscmp(const char *s1, const char *s2)
{
#if defined(HAVE_STRVERSCMP)
    return strverscmp(s1, s2);

#elif defined(HAVE___STRVERSCMP)
    return __strverscmp(s1, s2);

#else   /* local implementation */
    if (s1 != s2) {
        register const unsigned char *_s1 = (const unsigned char *)s1,
                *_s2 = (const unsigned char *)s2;

        for (; (*_s1 == *_s2) && *_s1; ++_s1, ++_s2) {
            ;
        }

        if ((*_s1 || *_s2) && (0 == *_s1 || 0 == *_s2)) {
            if ((*(_s1 - 1) == '0') && isdigit(*_s1 ? *_s1 : *_s2)) {
                return (*_s1 ? -1 : 1);
            }
        }
        return (int)(*_s1 - *_s2);
    }
    return 0;
#endif
}

/*end*/
