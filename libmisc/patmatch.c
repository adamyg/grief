#include <edidentifier.h>
__CIDENT_RCSID(gr_patmatch_c,"$Id: patmatch.c,v 1.15 2022/12/03 16:33:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: patmatch.c,v 1.15 2022/12/03 16:33:05 cvsuser Exp $
 * Basic pattern (not regexp, fnmatch style) matching support.
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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
#include <edtypes.h>
#include <edheaders.h>
#include <edfileio.h>
#include <patmatch.h>

#ifndef TRUE
#define TRUE 	1
#define FALSE	0
#endif

static int              match(const unsigned char *pattern, const unsigned char *name, int flags, matcherrfn_t errfn);
static void             errout(const char *fmt, ...);
static int              white(const unsigned char *p);
static int              range(const unsigned char **p, const unsigned char **n, int flags, matcherrfn_t errfn);


/*  Function:           patmatch
 *      Simple pattern matching engine.
 *
 *   Parameters:
 *      pattern - Pattern.
 *      file - File name buffer.
 *      flags - Control files.
 *
 *  Returns:
 *      true if matches otherwise false if not.
 */
int
patmatch(const char *pattern, const char *name, int flags)
{
    return match((const unsigned char *)pattern, (const unsigned char *)name, flags, errout);
}


/*  Function:           patmatch
 *      Simple pattern matching engine.
 *
 *   Parameters:
 *      pattern - Pattern.
 *      file - File name buffer.
 *      flags - Control files.
 *
 *  Returns:
 *      true if matches otherwise false if not.
 */
int
patmatchx(const char *pattern, const char *name, int flags, matcherrfn_t errfn)
{
    return match((const unsigned char *)pattern, (const unsigned char *)name, flags, errfn);
}


static int
match(const unsigned char *pattern, const unsigned char *name, int flags, matcherrfn_t errfn)
{
#define IGNORECASE(c) \
            ((flags & MATCH_NOCASE) && isupper(c) ? tolower(c) : c)

    const unsigned char *p = (const unsigned char *)pattern;
    const unsigned char *n = (const unsigned char *)name;
    int ch;

    while (0 != (ch = *p++)) {
        ch = IGNORECASE(ch);
        switch (ch) {
        case '?':           /* Match next character */
            if (FILEIO_ISSEP(*n) && (flags & MATCH_PATHNAME)) {
                return FALSE;
            }

            if (*n == '.' && (flags & MATCH_PERIODQ) &&
                    (n == name || ((flags & MATCH_PATHNAME) && FILEIO_ISSEP(n[-1])))) {
                return FALSE;
            }

            if (*n == '\0') {
                return FALSE;
            }
            break;

        case '*':           /* Match next [n] characters */
            if (*n == '.' && (flags & MATCH_PERIODA) &&
                    (n == name || ((flags & MATCH_PATHNAME) && FILEIO_ISSEP(n[-1])))) {
                return FALSE;
            }
                                                /* handle '**' and '*?' expressions */
            for (ch = *p++; ch == '?' || ch == '*'; ch = *p++, ++n) {
                if (ch == '?' && *n == '\0') {
                    return FALSE;
                }
            }

            if (ch == '\0') {
                return TRUE;
            }
            --p;

            {
                int c1 = IGNORECASE(ch);         /* next character to match */

                while (*n != '\0') {            /* recurse into next pattern(s) */
                    if ((c1 == '[' || IGNORECASE(*n) == c1) &&
                            match(p, n, flags, errfn) == TRUE) {
                        return TRUE;
                    }
                    ++n;
                }
            }
            return FALSE;

        case '[':           /* Match one of the given set */
            if (FILEIO_ISSEP(*n) && (flags & MATCH_PATHNAME)) {
                return FALSE;
            }

            if (*n == '.' && (flags & MATCH_PERIODB) &&
                    (n == name || ((flags & MATCH_PATHNAME) && FILEIO_ISSEP(n[-1])))) {
                return FALSE;
            }

            if (! range(&p, &n, flags, errfn)) {
                return FALSE;
            }
            break;

        case '\\':          /* Quote */
            if (0 == (flags & MATCH_NOESCAPE)) {
                if ((ch = *p++) == '\0') {      /* \... (unterminated) loses. */
                    return FALSE;
                }
            }
            /*FALLTHRU*/

        default:            /* other characters */
            if (ch != IGNORECASE(*n)) {
                return FALSE;
            }
            break;
        }
        n++;
    }

    if (0 == (flags & MATCH_TRAILINGWHITE)) {
        return (*n == '\0' ? TRUE : FALSE);     /* EOL? */
    }

    return white(n);
}


static void
errout(const char *fmt, ...)
{
    __CUNUSED(fmt)
}


static int
white(const unsigned char *p)
{
    while (*p) {
        if (' ' != *p && '\t' != *p /*&& '\r' != *p && '\n' != *p*/) {
            return FALSE;                       /* nop */
        }
    }
    return TRUE;
}


/*  Character Classes:
 *      Within a bracket expression, the name of a character class enclosed in [: and
 *      :] stands for the list of all characters (not all collating elements!)
 *      belonging to that class.
 *
 *  References:
 *      http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap09.html
 *      http://www.greenend.org.uk/rjk/2002/06/regexp.html
 *
 */

static int
is_ascii(int c)
{
#if defined(HAVE___ISASCII)
    return __isascii(c);
#elif defined(HAVE_ISASCII)
    return isascii(c);
#else
    return (c > 0 && c <= 0x7f);
#endif
}

static int is_alnum(int c)      { return isalnum((unsigned char)c); }
static int is_alpha(int c)      { return isalpha((unsigned char)c); }
static int is_blank(int c)
{
#if defined(HAVE___ISBLANK)
    return __isblank((unsigned char)c);
#elif defined(HAVE_ISBLANK)
    return isblank((unsigned char)c);
#else
    return (' ' == c || '\t' == c);
#endif
}
static int is_cntrl(int c)      { return iscntrl((unsigned char)c); }
static int is_csym(int c)       { return (c == '_' || isalnum((unsigned char)c)); }
static int is_digit(int c)      { return isdigit((unsigned char)c); }
static int is_graph(int c)      { return isgraph((unsigned char)c); }
static int is_lower(int c)      { return islower((unsigned char)c); }
static int is_print(int c)      { return isprint((unsigned char)c); }
static int is_punct(int c)      { return ispunct((unsigned char)c); }
static int is_space(int c)      { return isspace((unsigned char)c); }
static int is_upper(int c)      { return isupper((unsigned char)c); }
static int is_word(int c)       { return ('_' == c || '-' == c || isalnum((unsigned char)c)); }
static int is_xdigit(int c)     { return isxdigit((unsigned char)c); }

static const struct {
    const char         *name;
    unsigned            len;
    int               (*isa)(int);
} character_classes[] = {
    { "ascii",      5,  is_ascii    },          /* ASCII character. */
    { "alnum",      5,  is_alnum    },          /* An alphanumeric (letter or digit). */
    { "alpha",      5,  is_alpha    },          /* A letter. */
    { "blank",      5,  is_blank    },          /* A space or tab character. */
    { "cntrl",      5,  is_cntrl    },          /* A control character. */
    { "csym",       4,  is_csym     },          /* A language symbol. */
    { "digit",      5,  is_digit    },          /* A decimal digit. */
    { "graph",      5,  is_graph    },          /* A character with a visible representation. */
    { "lower",      5,  is_lower    },          /* A lower-case letter. */
    { "print",      5,  is_print    },          /* An alphanumeric (same as alnum). */
    { "punct",      5,  is_punct    },          /* A punctuation character. */
    { "space",      5,  is_space    },          /* A character producing white space in displayed text. */
    { "upper",      5,  is_upper    },          /* An upper-case letter. */
    { "word",       4,  is_word     },          /* A "word" character (alphanumeric plus "_"). */
    { "xdigit",     6,  is_xdigit   }           /* A hexadecimal digit. */
    };


/*  Function:           range
 *      Character set range match.
 *
 *   Description:
 *      '[' introduces a pattern bracket expression, that will matches a single
 *      collating element contained in the non-empty set of collating elements. The
 *      following rules apply:
 *
 *      o   A bracket expression is either a matching list expression or a non-matching
 *          list expression. It consists of one or more expressions.
 *
 *      o   A matching list expression specifies a list that matches any one of the
 *          expressions represented in the list. The first character in the list must
 *          not be the circumflex (^). For example, [abc] is a pattern that matches any
 *          of the characters 'a', 'b' or 'c'.
 *
 *      o   A non-matching list expression begins with a circumflex (^), and specifies
 *          a list that matches any character or collating element except for the
 *          expressions represented in the list after the leading circumflex. The
 *          circumflex will have this special meaning only when it occurs first in the
 *          list, immediately following the left-bracket.
 *
 *      o   A range expression represents the set of collating elements that fall
 *          between two elements in the current collation sequence, inclusively. It is
 *          expressed as the starting point and the ending point separated by a hyphen
 *          (-). For example, [a-z] is a pattern that matches any of the characters a
 *          to z inclusive.
 *
 *      o   A bracket expression followed by '+' means one or more times.
 */
static int
range(const unsigned char **pp, const unsigned char **pn, int flags, matcherrfn_t errfn)
{
    const unsigned char *p = *pp;
    const unsigned char *n = *pn;
    const unsigned char *start = p;             /* start of set */
    unsigned negative, cnt = 0;
    int value, ch;

    if (*n == '\0') {
        return FALSE;                           /* [... (unterminated) loses. */
    }

    /* Match a character within the given set. */
more:;
    negative = (*p == '!' || *p == '^');        /* not operator */
    if (negative) ++p;

    ch = *p++;
    if (ch == '\\' && !(flags & MATCH_NOESCAPE)) {
        ch = (int)*p++;
    }

    value = IGNORECASE(*n);                      /* test value */

    while (']' != ch) {                         /* end-of-set */
        int c1;

        if (ch == '\0') {                       /* [... (unterminated) loses. */
            errfn("match: Unmatched [] '%%[%s'", start);
            goto return_nomatch;
        }

        /*
         *  test current character 'c1' and load look-ahead into 'ch'.
         */
        c1 = IGNORECASE(ch);

        ch = *p++;
        if (ch == '\\' && !(flags & MATCH_NOESCAPE)) {
            ch = *p++;
        }
        ch = IGNORECASE(ch);


        /*
         *  ranges, character-classes and absolute
         */
        if ('-' == ch && ']' != *p) {           /* range (eg a-z) */
            /*
             *  range,
             *      To avoid confusion, error if the range is not numerically greater than
             *      the left side character.
             */
            int c2 = *p++;

            if (c2 == '\\' && !(flags & MATCH_NOESCAPE)) {
                c2 = *p++;
            }

            if (c2 == '[') {                    /* character-class, loses. */
                errfn("match: [] r-value cannot be a character-class '%%[%.*s'", (int)(p - start) + 1, start);
                goto return_nomatch;
            }

            if (c2 == '\0') {                   /* [... unterminated, loses. */
                errfn("match: Unmatched [] '%%[%.*s'", (int)(p - start)+1, start);
                goto return_nomatch;
            }

            c2 = IGNORECASE(c2);
            ch = *p++;                          /* load next */

            if (c1 > c2) {                      /* invalid order, loses. */
                errfn("match: Invalid [] collating order '%%[%.*s'", (int)(p - start)+1, start);
                goto return_nomatch;
            }
                                                /* range test */
            if ((unsigned)value >= (unsigned)c1 && (unsigned)value <= (unsigned)c2) {
                goto matched;
            }

            ch = *p++;                          /* load next */

        } else if ('[' == c1 && ':' == ch) {
            /*
             *  character-classes
             */
            const unsigned char *cc = p;        /* start of character-class */
            int i;

            while (':' != (ch = *p++) && ']' != *p)
                if (0 == ch) {
                    errfn("match: Unmatched ':]' within '%%[%.*s'", (int)(p - start) + 1, start);
                    goto return_nomatch;
                }

            ++p;                                /* consume ']' */
            ch = *p++;                          /* load next */

            for (i = (sizeof(character_classes)/sizeof(character_classes[0]))-1; i >= 0; --i)
                if (0 == strncmp((const char *)cc, character_classes[i].name, character_classes[i].len)) {
                    if ((*character_classes[i].isa)((unsigned char)value)) {
                        goto matched;           /* class matched */
                    }
                    break;
                }
            if (-1 == i) {
                errfn("match: Unknown [] character-class '%.*s'",
                    (int)((p - cc) - 1), cc);
                goto return_nomatch;
            }

        } else {
            /*
             *  absolute match
             */
            if (value == c1) {                  /* abs test */
                goto matched;
            }
        }
    }

    /* No match */
    if (! negative) {       /* Match expected, at least one previous match required */
        if (0 == cnt)
            goto return_nomatch;
        if (*p == '+')
            ++p;                                /* remove modifier */
        --n;                                    /* push terminator */

    } else {                /* No match expected, loop if required otherwise success */
        if (*p == '+') {
            ++cnt;
            if (n[1]) {                         /* additional characters to scan */
                n++, p = start;
                goto more;
            }
            ++p;                                /* remove modifier */
        }
    }
    goto return_match;

    /* Matched .. skip the rest of the [...] expression */
matched:
    while (ch != ']') {
        if (ch == '[' && *p == ':') {
            ++p;                                /* consume ':' */
            do {
                ch = *p++;
                if (ch == ':' && *p == ']') {
                    ++p;                        /* consume ']' */
                    break;
                }
            } while (ch);
        }
        if (ch == '\0') {                       /* [... (unterminated) loses. */
            goto return_nomatch;
        }
        ch = *p++;
    }

    if (negative) {         /* No match excepted, if [!x]+ previous match required */
        if (*p == '+') {
            if (0 == cnt)
                goto return_nomatch;
            ++p;                                /* remove modifier */
        }
        n--;                                    /* push terminator */

    } else {                /* Match expected, loop if required otherwise success */
        if (*p == '+') {
            ++cnt;
            if (n[1]) {                         /* additional characters to scan */
                ++n, p = start;
                goto more;
            }
            ++p;                                /* remove modifier */
        }
    }

return_match:
    *pn = n; *pp = p;
    return TRUE;

return_nomatch:
    *pn = n; *pp = p;
    return FALSE;
}
#undef  __TOLOWER
/*end*/
