#include <edidentifier.h>
__CIDENT_RCSID(gr_regexp_c,"$Id: regexp.c,v 1.46 2020/06/03 16:17:32 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regexp.c,v 1.46 2020/06/03 16:17:32 cvsuser Exp $
 * Regular expression engine.
 *
 *  The orgin of this regular expression implementation has been lost with time,
 *  yet is similar in design to simple regexp engines of the time; examples include
 *  Ozan S. Yigit and Henry Spencer.
 *
 *  As with them this implementation uses a nondeterministic automata rather than the
 *  deterministic ones found in more complex implementations, which makes it simpler,
 *  smaller, and faster at compiling regular expressions, but possiblity slower at
 *  executing them; a number of optimisations are employed for simple cases.
 *
 *  An number of GRIEF expressions and enhancements including Knuth-Morris-Pratt
 *  string searching algorithm are included to reduce worst case execution.
 *
 *  For detail discussion see the following series of articles written by Russ Cox
 *  "Implementing Regular Expressions" (http://swtch.com/~rsc/regexp/regexp4.html).
 *
 *      o Regular Expression Matching Can Be Simple And Fast
 *          http://swtch.com/~rsc/regexp/regexp1.html
 *
 *      o Regular Expression Matching: the Virtual Machine Approach
 *          http://swtch.com/~rsc/regexp/regexp2.htm
 *
 *      o Regular Expression Matching in the Wild
 *          http://swtch.com/~rsc/regexp/regexp3.html
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#define ED_ASSERT
#include <editor.h>
#include <edassert.h>

#include "regexp.h"
#include "m_search.h"

#include "debug.h"
#include "echo.h"
#include "word.h"

#define MAGIC_BRIEF         "%@+*?[|{}\\$>"
#define MAGIC_UNIX          "+*.[|{}\\$>"

#define EWERROR(x)          {ewprintf(x); return -1;}

#define REGEXP_SIZE         512                 /* Initial allocation */
#define REGEXP_UNIT         128                 /* Round allocation size to 128 bytes */
#define REGEXP_ROUND(v)     (((v) + REGEXP_UNIT) &~ (REGEXP_UNIT - 1))

#define BITMAP_SIZE         (256 / 8)

enum _rxcodes {
    RE_FINISH       = 1,                        /* End of compiled expression */
    RE_ZERO_OR_MORE = 2,                        /* ..@ */
    RE_ONE_OR_MORE  = 3,                        /* ..+ */
    RE_STAR         = 4,                        /* * (BRIEF) */
    RE_QUESTION     = 5,                        /* ? (BRIEF) or . (Unix) */
    RE_CLASS        = 6,                        /* [xyz] or [~xyz] */
    RE_STRING       = 7,                        /* xyz... */
    RE_END          = 8,                        /* End of branch list */
    RE_OR           = 9,                        /* '|' alternatives */
    RE_SETPOS       = 10,                       /* \c */
    RE_BOL          = 11,                       /* < or ^ or % */
    RE_EOL          = 12,                       /* > or $ */
    RE_LOOP         = 13,                       /* Used as end of block terminator for @ and +. */
    RE_WORD_BEGIN   = 14,                       /* Used for \< */
    RE_WORD_END     = 15,                       /* Used for \> */
    RE_NEWLINE      = 16,                       /* \n */
        /*17 .. 19*/
    RE_OPEN         = 20,                       /* '{' OPEN0 .. OPEN9 */
    RE_CLOSE        = 30                        /* '}' CLOSE0 .. CLOSE9 */
    };

static const uint8_t x_bittab[] = {
    1, 2, 4, 8, 16, 32, 64, 128
    };

static const struct regopts defoptions = {
    0,
    TRUE,                                       /* Case sensitive search. */
    TRUE,                                       /* Regular expression characters. */
    RE_BRIEF,                                   /* BRIEF. */
    TRUE,                                       /* Forward search. */
    };

static const char *re_opcodes[] = {
    /* 00 */ "<0>",
    /* 01 */ "FINISH",
    /* 02 */ "ZERO-OR-MORE",
    /* 03 */ "ONE-OR-MORE",
    /* 04 */ "STAR",
    /* 05 */ "QUESTION",
    /* 06 */ "CLASS",
    /* 07 */ "STRING",
    /* 08 */ "END",
    /* 09 */ "OR",
    /* 10 */ "SETPOS",
    /* 11 */ "BOL",
    /* 12 */ "EOL",
    /* 13 */ "LOOP",
    /* 14 */ "WORD-BEGIN",
    /* 15 */ "WORD-END",
    /* 16 */ "NEWLINE",
    /* 17 */ "<17>",
    /* 18 */ "<18>",
    /* 19 */ "<19>",
    /* 20 */ "OPEN-0",
    /* 21 */ "OPEN-1",
    /* 22 */ "OPEN-2",
    /* 23 */ "OPEN-3",
    /* 24 */ "OPEN-4",
    /* 25 */ "OPEN-5",
    /* 26 */ "OPEN-6",
    /* 27 */ "OPEN-7",
    /* 28 */ "OPEN-8",
    /* 29 */ "OPEN-9",
    /* 30 */ "CLOSE-0",
    /* 31 */ "CLOSE-1",
    /* 32 */ "CLOSE-2",
    /* 33 */ "CLOSE-3",
    /* 34 */ "CLOSE-4",
    /* 35 */ "CLOSE-5",
    /* 36 */ "CLOSE-6",
    /* 37 */ "CLOSE-7",
    /* 38 */ "CLOSE-8",
    /* 39 */ "CLOSE-9"
    };

typedef uint8_t REGEXPATOM;                     /* Program unit type. */

typedef struct {
    struct regopts          regopts;
    const char *            pattern;            /* current pattern */
    int                     modifier;           /* *true* if modifiers are expected */
    int                     orbranch;
    REGEXPATOM *            atoms;
    size_t                  opsiz;
    size_t                  opidx;
    size_t                  opend;
    int                     stack[10];
    int                     level;
    int                     group;              /* current capture group */
} recomp_t;

typedef struct {
    struct regopts          regopts;
    REGEXP *                prog;
    const char *            start;
} rematch_t;

typedef int loopstate_t;

static int                  re_comp(recomp_t *rx);
static const char *         re_atom(recomp_t *rx, const char *pattern);
static const REGEXPATOM *   re_nextblock(const REGEXPATOM *re);
static int                  re_escape(const char **patternp, int *result);
static int                  re_expand(recomp_t *rx, size_t len);
static void                 re_shiftup(recomp_t *rx, size_t idx);
static int                  re_match(const rematch_t *match, const REGEXPATOM *re, const char *p, int size, loopstate_t *loopstate);
static void                 re_print(const REGEXPATOM *re);
static const char *         re_printbm(const REGEXPATOM *bm);
#if defined(DEBUG_REGEXP)
static void                 re_printbuf(const char *start, const char *end);
#endif

static void                 kmp_table(int wordlen, const char *pat, char *word, int16_t *table);
static const char *         kmp_search(const char *text, int textlen, const char *word, int wordlen, const int16_t *table);
static void                 kmp_itable(int wordlen, const char *pat, char *word, int16_t *table);
static const char *         kmp_isearch(const char *text, int textlen, const char *word, int wordlen, const int16_t *table);

static __CINLINE int        strcasematch(const char *s1, const char *s2, int len);


/*  Character Classes:
 *      Within a bracket expression, the name of a character class enclosed in
 *      [: and :] stands for the list of all characters (not all collating elements!)
 *      belonging to that class.
 *
 *  References:
 *      http://www.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap09.html
 *      http://www.greenend.org.uk/rjk/2002/06/regexp.html
 *
 */

#if defined(__cplusplus)
extern "C" {
#endif

    /*
     *  character class interface adapters, resolve calling convention issues.
     */
    static int is_ascii(int c);
    static int is_alnum(int c);
    static int is_alpha(int c); 
    static int is_blank(int c); 
    static int is_cntrl(int c); 
    static int is_csym(int ch);
    static int is_digit(int c); 
    static int is_graph(int c); 
    static int is_lower(int c); 
    static int is_print(int c); 
    static int is_punct(int c); 
    static int is_space(int c); 
    static int is_upper(int c); 
    static int is_word(int c);
    static int is_xdigit(int c);

    static const struct {
        const char         *name;
        unsigned            len;
        int               (*isa)(int);
    } character_classes[] = {
        { "ascii",  5, is_ascii  }, /* ASCII character. */
        { "alnum",  5, is_alnum  }, /* An alphanumeric (letter or digit). */
        { "alpha",  5, is_alpha  }, /* A letter. */
        { "blank",  5, is_blank  }, /* A space or tab character. */
        { "cntrl",  5, is_cntrl  }, /* A control character. */
        { "csym",   4, is_csym   }, /* A language symbol. */
        { "digit",  5, is_digit  }, /* A decimal digit. */
        { "graph",  5, is_graph  }, /* A character with a visible representation. */
        { "lower",  5, is_lower  }, /* A lower-case letter. */
        { "print",  5, is_print  }, /* An alphanumeric (same as alnum). */
        { "punct",  5, is_punct  }, /* A punctuation character. */
        { "space",  5, is_space  }, /* A character producing white space in displayed text. */
        { "upper",  5, is_upper  }, /* An upper-case letter. */
        { "word",   4, is_word   }, /* A "word" character (alphanumeric plus "_"). */
        { "xdigit", 6, is_xdigit }  /* A hexadecimal digit. */
        };

#if defined(__cplusplus)
}
#endif


static __CINLINE void
PUT16(REGEXPATOM *re, const uint16_t val)
{
    re[0] = (uint8_t) (val >> 8);
    re[1] = (uint8_t) (val);
}


static __CINLINE uint16_t
GET16(const REGEXPATOM *re)
{
    return (((uint16_t)re[0] << 8) | re[1]);
}


/*
 *  regexp_comp ---
 *      regular expression compilation.
 */
REGEXP *
regexp_comp(const struct regopts *regopts, const char *pattern)
{
    static REGEXP regexp;
    REGEXP *ret;

    assert(NULL == regexp._dmem);               /* confirm regexp_free() */
    if (NULL == (ret = regexp_comp2(&regexp, regopts, pattern))) {
	regexp_free(&regexp);
	return NULL;
    }
    return ret;
}


/*
 *  regexp_comp2 ---
 *      regular expression compilation.
 */
REGEXP *
regexp_comp2(REGEXP *regexp, const struct regopts *options, const char *pattern)
{
    register const REGEXPATOM *re;
    recomp_t rx = {0};

    rx.regopts  = (options ? *options : defoptions);
    rx.atoms    = regexp->_dmem;
    rx.opsiz    = regexp->_dsiz;
    rx.pattern  = pattern;

    if (FALSE == re_expand(&rx, REGEXP_SIZE)) {
        goto error;
    }

    if (re_comp(&rx) < 0) {
        goto error;
    }

    if (rx.level) {
        ewprintf("regexp: Missing close brace %s", pattern);
        goto error;
    }

    rx.atoms[rx.opidx] = RE_FINISH;

    if (DB_REGEXP & x_dflags) {
        trace_log("Pass 1:\n");
        re_print(rx.atoms);
    }

    /*
     *  Following piece of code walks down the regular expression and sets up
     *  the link fields needed by the '|'-OR pattern matching code.
     */
    for (re = rx.atoms; RE_FINISH != *re;) {
        register const REGEXPATOM *re1, *re2;

        if (RE_ZERO_OR_MORE == *re || RE_ONE_OR_MORE == *re) {
            re += 3;
            continue;
        }

        if (RE_OR != *re) {
            re += (RE_END == *re ? 3 : LGET16((const LIST *)re));
            continue;
        }

        re1 = re + 3;
        re1 = re_nextblock(re1);

        if (RE_END != *re1) {
            ewprintf("regexp: Inconsistency \"%s\".", pattern);
            goto error;
        }

        LPUT16((LIST *)re, (uint16_t)(re1 - re + 3));

        if (NULL == (re2 = re_nextblock(re1))) {
            ewprintf("regexp: Inconsistency \"%s\".", pattern);
            goto error;
        }

        while (1) {
            if (RE_OR != *re2) {
                if (NULL == (re2 = re_nextblock(re2))) {
                    ewprintf("regexp: Inconsistency \"%s\".", pattern);
                    goto error;
                }
                break;
            }
            re2 += 3;
            while (RE_END != *re2 && RE_FINISH != *re2)
                if (NULL == (re2 = re_nextblock(re2))) {
                    ewprintf("regexp: Inconsistency \"%s\".", pattern);
                    goto error;
                }
            if (RE_END == *re2) {
                re2 += 3;
            }
        }

        LPUT16((LIST *)re1, (uint16_t)(re2 - re1));
        re += 3;
    }

    if (DB_REGEXP & x_dflags) {
        trace_log("Pass 2:\n");
        re_print(rx.atoms);
    }

    regexp->magic   = REGEXP_MAGIC;
    regexp->options = rx.regopts;
    regexp->program = (void *)rx.atoms;
    regexp->_dmem   = rx.atoms;
    regexp->_dsiz   = rx.opsiz;
    return regexp;

error:;
    regexp->magic   = REGEXP_MAGIC;
    memset(&regexp->options, 0, sizeof(regexp->options));
    regexp->program = NULL;
    regexp->_dmem   = rx.atoms;
    regexp->_dsiz   = rx.opsiz;
    return NULL;
}


static int
re_comp(recomp_t *rx)
{
    const size_t lastend_on_entry = rx->opend;
    const char *pattern = rx->pattern;
    int ch, allow_or = FALSE;

#define MAX_OR              32

    rx->orbranch = FALSE;
    while (0 != (ch = *pattern)) {

        if (0 == rx->regopts.regexp_chars) {
            goto DEFAULT;
        }

        re_expand(rx, 16);
        switch (ch) {
        case '*':
            if (!rx->regopts.mode) {
                goto DEFAULT;                   /* STAR */
            }
            /*FALLTHRU*/

        case '@':
            if ('@' == ch && rx->regopts.mode) {
                goto DEFAULT;
            }
            /*FALLTHRU*/

        case '+':
            if (FALSE == rx->modifier) {
                goto DEFAULT;
            }
            re_shiftup(rx, rx->opend);
            rx->atoms[rx->opend] = ('+' == ch ? RE_ONE_OR_MORE : RE_ZERO_OR_MORE);
            rx->atoms[rx->opidx] = RE_LOOP;
            LPUT16((LIST *)(rx->atoms + rx->opidx), 3);
            rx->opidx += 3;
            LPUT16((LIST *)(rx->atoms + rx->opend), (uint16_t)(rx->opidx - rx->opend));
            rx->modifier = FALSE;
            ++pattern;
            break;

        case '|':
            if (FALSE == allow_or) {
                EWERROR("regexp: Null expression before |");
            }
            re_shiftup(rx, rx->opend);
            rx->atoms[rx->opend] = RE_OR;
            rx->atoms[rx->opidx] = RE_END;
            LPUT16((LIST *)(rx->atoms + rx->opidx), 0);
            rx->opidx += 3;
            rx->orbranch = TRUE;
            allow_or = FALSE;
            ++pattern;
            continue;

        case '{':
            if (rx->regopts.mode) {
                goto DEFAULT;
            }
open_bracket:
            if (rx->level >= RE_NSUBEXP) {
                EWERROR("regexp: Too many '{'");
            }
            rx->opend = rx->opidx;
            rx->atoms[rx->opidx] = (REGEXPATOM)(RE_OPEN + rx->group);
            rx->stack[rx->level++] = rx->group;
            if (rx->group < (RE_NSUBEXP-1)) {
                ++rx->group;
            }
            LPUT16((LIST *)(rx->atoms + rx->opidx), 3);
            rx->opidx += 3;
            rx->pattern = ++pattern;
            if (re_comp(rx) < 0) {
                --rx->level;
                return -1;
            }
            pattern = rx->pattern;
            allow_or = TRUE;
            --rx->level;
            break;

        case '}':
            if (rx->regopts.mode) {
                goto DEFAULT;
            }
close_bracket:
            rx->atoms[rx->opidx] = (REGEXPATOM)(RE_CLOSE + rx->stack[rx->level - 1]);
            LPUT16((LIST *)(rx->atoms + rx->opidx), 3);
            rx->opidx += 3;
            rx->modifier = TRUE;
            rx->opend    = lastend_on_entry;
            rx->pattern  = ++pattern;
            return 0;

        case '\\':
            if (rx->regopts.mode) {
                switch (pattern[1]) {
                case '(':
                    ++pattern;
                    goto open_bracket;
                case ')':
                    ++pattern;
                    goto close_bracket;
                }
            }
            /*FALLTHRU*/

DEFAULT:
        default:
            rx->opend = rx->opidx;
            rx->modifier = TRUE;
            allow_or = TRUE;
            if (NULL == (pattern = re_atom(rx, pattern))) {
                return -1;
            }
            LPUT16((LIST *)(rx->atoms + rx->opend), (uint16_t)(rx->opidx - rx->opend));
            break;
        }
        rx->orbranch = FALSE;
    }
    rx->pattern = pattern;
    return 0;
}


/*
 *  re_atom ---
 *      Encode the current expression element; excluding branches(|) and capture groups.
 *
 *  Unix Elements:
 *      <               Beginning of line
 *      >               End of line.
 *      .               Any character.
 *
 *  BRIEF Elements:
 *      <, %, ^         Beginning of line
 *      >, $            End of line.
 *      ?               Any character.
 *
 *  Common Elements:
 *      *               None or more characters.
 *      [...]           Character class [:<class>:] or range [x-y].
 *      \xxx            Escape sequence (see re_escape) plus any mode specific elements.
 *      ...             Standard character.
 *
 *  Returns:
 *      Resulting pattern cursor address, otherwise NULL on error.
 */
static const char *
re_atom(register recomp_t *rx, const char *pattern)
{
    const char *MAGIC = (rx->regopts.mode ? MAGIC_UNIX : MAGIC_BRIEF);
    int patinc = 1, opsize = 3;                 /* pattern increment, operator size (in bytes) */
    int len = 0, ch;

    if (0 == rx->regopts.regexp_chars) {
        goto DEFAULT;
    }

    re_expand(rx, 16);
    switch (*pattern) {
    case '<':
    case '%':
        if (rx->regopts.mode) {
            goto DEFAULT;
        }
        /*FALLTHRU*/

    case '^':
        rx->atoms[rx->opidx] = RE_BOL;
        break;

    case '>':
        if (rx->regopts.mode) {
            goto DEFAULT;
        }
        /*FALLTHRU*/

    case '$':
        rx->atoms[rx->opidx] = RE_EOL;
        break;

    case '.':
        if (!rx->regopts.mode) {
            goto DEFAULT;
        }
        rx->atoms[rx->opidx] = RE_QUESTION;
        break;

    case '?':
        if (rx->regopts.mode) {
            goto DEFAULT;
        }
        rx->atoms[rx->opidx] = RE_QUESTION;
        break;

    case '*':
        rx->atoms[rx->opidx] = RE_STAR;
        break;

    case '[': {
            const char *start = pattern;
            uint8_t bitmap[BITMAP_SIZE] = {0};
            char range, lvalue;
            int neg = 0;
            int i;

#define SET(__x)            bitmap[(uint8_t)(__x >> 3)] |= x_bittab[(uint8_t)(__x & 7)]
#define ISSET(__bm, __x)    (__bm[(uint8_t)__x >> 3] & x_bittab[(uint8_t)__x & 7])

            rx->atoms[rx->opidx] = RE_CLASS;
            ++pattern;

            if ('^' == *pattern || '~' == *pattern) {
                ++pattern;
                neg = 1;                        /* negation */
            }

            while (']' == *pattern || '-' == *pattern) {
                SET(*pattern);                  /* if leading is ']' or '-' treat as normal character */
                ++pattern;
            }

            range = lvalue = 0;

            while (*pattern && *pattern != ']') {
                /*
                 *  process character and ranges
                 */
                char c = *pattern++;            /* next character */

                if ('[' == c && '.' == pattern[0]) {
                    /*
                     *  collating symbols (eg [.<.])
                     */
                    if (0 == (c = pattern[1]) || '.' != pattern[2] || ']' != pattern[3]) {
                        ewprintf("regexp: unmatched character-sequence '.]' '%%[%.*s'",
                            (int)(pattern - start) + 1, start);
                        return NULL;
                    }
                    pattern += 4;               /* consume X.] */
                }

                if (range) {                    /* right-side */
                    /*
                     *  To avoid confusion, error if the range is not numerically greater than
                     *  the left side character.
                     */
                    if ('[' == c) {
                        ewprintf("regexp: [] r-value cannot be a character-class '%%[%.*s'",
                            (int)(pattern - start) + 1, start);
                        return NULL;
                    }

                    if (c <= lvalue) {
                        ewprintf("regexp: Invalid [] collating order '%%[%.*s'",
                            (int)(pattern - start) + 1, start);
                        return NULL;
                    }

                    for (; lvalue <= c; ++lvalue) {
                        SET(lvalue);            /* .. mark range */
                    }
                    --range;

                    lvalue = c;                 /* allow [a-b-c] */

                } else if ('[' == c && ':' == *pattern) {
                    /*
                     *  character-classes
                     */                         /* start of character-class */
                    const char *cc = ++pattern;

                    while (0 != (c = *pattern++)) {
                        if (':' == c && ']' == *pattern) {
                            int i2;             /* look for closing :] */

                            for (i2 = (sizeof(character_classes)/sizeof(character_classes[0]))-1; i2 >= 0; --i2) {
                                if (0 == strncmp(cc, character_classes[i2].name, character_classes[i2].len)) {
                                    int v;      /* matching class */

                                                // TODO: cache character class sets.
                                    for (v = 1; v <= 0xff; ++v)  {
                                        if ((*character_classes[i2].isa)(v)) {
                                            SET(v);
                                        }
                                    }
                                    break;
                                }
                            }
                            if (-1 == i2) {
                                ewprintf("regexp: Unknown [] character-class '%.*s'",
                                    (int)((pattern - cc) - 1), cc);
                                return NULL;
                            }
                            ++pattern;          /* consume ']' */
                            break;
                        }
                    }
                    if (0 == c) {
                        ewprintf("regexp: Unmatched ':]' within '%%[%.*s'",
                            (int)(pattern - start) + 1, start);
                        return NULL;
                    }
                    lvalue = 0;                 /* cannot be l-value */

                } else if ('-' == c) {
                    /*
                     *  The `-' is not considered to define a range if the character
                     *  following is a closiing bracket.
                     */
                    if (*pattern == ']') {
                        SET('-');
                        break;                  /* .. closing */
                    }
                    if (0 == lvalue) {          /* missing l-value */
                        ewprintf("regexp: Unmatched [] range '%%[%.*s'",
                            (int)(pattern - start) + 1, start);
                        return NULL;
                    }
                    ++range;

                } else if ('\\' == c) {         /* quote, grief extension */
#if (TODO)  //character classes short-hand
                    switch (*pattern) {
                    case 'd': //character class for digits.
                    case 'D': //character class for non-digits.
                    case 's': //character class for whitespace.
                    case 'w': //character class for word characters.
                    case 'W': //character class for non-word characters.
                    }
#endif //TODO

                    if (FALSE == re_escape(&pattern, &ch)) {
                        ++pattern;
                    }
                    lvalue = (char)ch;
                    SET(lvalue);

                } else {                        /* clear character */
                    lvalue = (char)c;
                    SET(lvalue);
                }
            }

            if (*pattern++ != ']') {
                ewprintf("regexp: Unmatched []");
                return NULL;
            }

#if !defined(DO_DYNAMIC_CASESENSE)
            if (! rx->regopts.case_sense) {     /* not case sensitive */
                unsigned a, A;
                for (a = 'a', A = 'A'; a <= 'z'; ++a, ++A) {
                    if (ISSET(bitmap, a)) {
                        SET(A);
                    } else if (ISSET(bitmap, A)) {
                        SET(a);
                    }
                }
            }
#endif /*!DO_DYNAMIC_CASESENSE*/

            re_expand(rx, BITMAP_SIZE + 3);
            for (i = 0; i < BITMAP_SIZE; ++i) { /* store, reverse if negation */
                rx->atoms[rx->opidx + 3 + i] = (neg ? ~bitmap[i] : bitmap[i]);
            }
            rx->opidx += 3 + i;
        }
        return pattern;

    case ']':
        ewprintf("regexp: Unmatched []");
        return NULL;

    case '\\':
        if ('c' == *++pattern) {
            rx->atoms[rx->opidx] = RE_SETPOS;
            break;
        }

        if (rx->regopts.mode) {
            if ('<' == *pattern) {
                rx->atoms[rx->opidx] = RE_WORD_BEGIN;
                break;
            }

            if ('>' == *pattern) {
                rx->atoms[rx->opidx] = RE_WORD_END;
                break;
            }
        }

#if (TODO)  //character classes short-hand
        switch (*pattern) {
        case 'd': //character class for digits.
        case 'D': //character class for non-digits.
        case 's': //character class for whitespace.
        case 'w': //character class for word characters.
        case 'W': //character class for non-word characters.
        }
#endif //TODO

        if (FALSE == re_escape(&pattern, &ch)) {
            len = strcspn(pattern + 1, MAGIC) + 1;
            goto NORMAL;
        }

        --pattern;
        re_expand(rx, len = 1);
        rx->atoms[rx->opidx] = RE_STRING;
        PUT16(rx->atoms + (rx->opidx + 3), (uint16_t)len);
        rx->atoms[rx->opidx + 5] = (REGEXPATOM)ch; /* string */
        opsize = len + 5;
        patinc = len;
        break;

DEFAULT:
    default:
        if (rx->regopts.regexp_chars) {
            len = strcspn(pattern, MAGIC);
        } else {
            len = (int)strlen(pattern);
        }

NORMAL: if (rx->orbranch && len > 1) {
            len = 1;

        } else if (len <= 0) {
            len = (int)strlen(pattern);

        } else if (len > 1 && rx->regopts.regexp_chars) {
            /*
             *  If followed by a repeat or modifier we need to include/not-include
             *  the last character because we need to obey the differing precedence
             *  between BRIEF/CRISP regexps and Unix regexps.
             */
            const char m = pattern[len];

            if ('@' == m || '*' == m || '+' == m || '|' == m) {
               --len;
            }
        }

        if (len <= 0) {                         /* stop endless matches */
            ewprintf("regexp: Empty expression encountered");
            return NULL;
        }

        {   REGEXPATOM *satom;

            re_expand(rx, len * 3);             /* string(int8) + table(int16) + pad */
            satom  = rx->atoms + rx->opidx;
            *satom = RE_STRING;
            PUT16(satom + 3, (uint16_t)len);
            if (len >= 4 && len <= 0x1fff) {
                uintptr_t table = (uintptr_t)((satom += 5) + len);

                opsize = len + ((len + 2) * sizeof(int16_t)) + 5;
                if (0x01 & table) ++table;      /* word align */
                if (rx->regopts.case_sense) {
                    kmp_table(len, pattern, (char *)satom, (int16_t *)table);
                } else {
                    kmp_itable(len, pattern, (char *)satom, (int16_t *)table);
                }
            } else {
                memcpy(satom + 5, pattern, len);
                opsize = len + 5;
            }
            patinc = len;
        }
        break;
    }

    pattern += patinc;
    if (0 == *pattern || '}' == *pattern) {
        LPUT16((LIST *)(rx->atoms + rx->opidx), 0);
    } else {
        LPUT16((LIST *)(rx->atoms + rx->opidx), (uint16_t)opsize);
    }
    rx->opidx += opsize;
    return pattern;
}


static const REGEXPATOM *
re_nextblock(const REGEXPATOM *re)
{
    if (*re >= (RE_OPEN + 0) && *re <= (RE_OPEN + 9)) {
        const REGEXPATOM XCLOSE = *re + 10;

        while (XCLOSE != *re) {
            const unsigned len = (RE_OR == *re || RE_END == *re) ? 3 : LGET16((const LIST *)re);
            if (0 == len) {
                return NULL;
            }
            re += len;
        }
    }
    if (RE_OR == *re || RE_END == *re) {
        return re + 3;
    }
    return re + LGET16((const LIST *)re);
}


/*  re_escape ---
 *      ESC sequence parser, returning the decoded character value.
 *
 *  Supported Escapes:
 *
 *      o fixed
 *
 *          '\e'            ESC
 *          '\f'            Formfeed
 *          '\n'            Newline
 *          '\r'            Return
 *          '\t'            Tab
 *          '\v'            Vertical tab
 *
 *      o Numeric
 *
 *          '\x##'          Hexidecimal
 *          '\X####'
 *          '\x{# ...}'
 *          '\0##'          Octal
 *          '\o{# ...}'
 *          '\###           Decimal
 *          '\d###
 *
 *  Returns:
 *      TRUE on success and the associated character value otherwise FALSE.
 */
static int
re_escape(const char **patternp, int *result)
{
    static const char hexchrs[] = "0123456789abcdef";
    static const char octchrs[] = "012345678";

    const char *pattern = *patternp;
    int escaped = TRUE;
    int ch = *pattern++;

    switch (ch) {
                    // fixed controls
    case 'e': ch = 0x1b; break;                 // ESC
    case 'f': ch = '\f'; break;                 // Formfeed
    case 'n': ch = '\n'; break;                 // Newline
    case 'r': ch = '\r'; break;                 // Return
    case 't': ch = '\t'; break;                 // Tab
    case 'v': ch = '\v'; break;                 // vertical tab

    case 'x':
    case 'X': {     // hex
            int limit = ('X' == ch ? 4 : 2);
            int accum, mxd;

            if ('{' == *pattern) {              // extended hex
                limit = 9;                      // 32bit
                ++pattern;
            }
            for (accum = mxd = 0; mxd++ < limit;) {
                const char *p = strchr(hexchrs, tolower(*pattern));

                if (NULL == p) {
                    if (9 == limit) {
                        if ('}' == ch) {
                            ++pattern;          // terminator
                        } else {
                            EWERROR("regexp: Unterminated hexidecimal constant");
                        }
                    }
                    break;
                }
                ++pattern;
                accum = (accum * 16) + (p - hexchrs);
            }
            ch = accum;
        }
        break;

    case '0': {     // octal
            int accum, mxd;

            for (accum = mxd = 0; mxd++ < 3;) {
                const char *p = strchr(octchrs, *pattern);

                if (NULL == p) break;
                ++pattern;
                accum = (accum * 8) + (p - octchrs);
            }
            ch = accum;
        }
        break;

    case 'o':       // extended octal
        if ('{' == *pattern) {
            int accum, mxd;

            for (accum = mxd = 0; mxd++ < 10;) {
                const char *p = strchr(octchrs, *pattern);

                if (NULL == p) {
                    if ('}' == ch) {
                        ++pattern;              // terminator
                    } else {
                        EWERROR("regexp: Unterminated octal constant");
                    }
                    break;
                }
                ++pattern;
                accum = (accum * 8) + (p - octchrs);
            }
            ch = accum;
        } else {
            escaped = FALSE;
        }
        break;

    case 'd':       // decimal
        ch = *pattern++;
        /*FALLTHRU*/

    default:
        if (! isdigit(ch)) {
            escaped = FALSE;
        } else {
            int accum, mxd = 0;

            accum = ch - '0';
            while (mxd++ < 2 && isdigit(*pattern)) {
                accum = (accum * 10) + (*pattern - '0');
                ++pattern;
            }
            ch = accum;
        }
        break;
    }

    if (escaped) *patternp = pattern;
    *result = (0xff & ch);
    return escaped;
}


/*  re_expand --
 *      Verify the available parser storage against the additional
 *      requirement of 'len' bytes, expanding the underlying
 *      accumulator is required.
 *
 *  Returns:
 *      TRUE on success, otherwise FALSE on error.
 */
static int
re_expand(register recomp_t *rx, size_t len)
{
    const size_t needed = rx->opidx + len + 16, /*op,length*/
            opsiz = REGEXP_ROUND(needed);

    assert(len > 0);
    assert(rx->opsiz >= rx->opidx);
    if (opsiz >= rx->opsiz || NULL == rx->atoms) {
        REGEXPATOM *natoms = (REGEXPATOM *)chk_realloc(rx->atoms, opsiz);

        assert(NULL != natoms);
        if (NULL == natoms) {
            return FALSE;
        }
        rx->opsiz = opsiz;
        rx->atoms = natoms;
    }
    return TRUE;
}

static int is_ascii(int c)
{
#if defined(HAVE___ISASCII)
    return  __isascii((unsigned char)c);
#elif defined(HAVE_ISASCII)
    return isascii((unsigned char)c);
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
static int is_csym(int ch)      { return ('_' == ch || isalnum((unsigned char)ch)); }
static int is_digit(int c)      { return isdigit((unsigned char)c); }
static int is_graph(int c)      { return isgraph((unsigned char)c); }
static int is_lower(int c)      { return islower((unsigned char)c); }
static int is_print(int c)      { return isprint((unsigned char)c); }
static int is_punct(int c)      { return ispunct((unsigned char)c); }
static int is_space(int c)      { return isspace((unsigned char)c); }
static int is_upper(int c)      { return isupper((unsigned char)c); }
static int is_word(int c)       { return ('_' == c || isalnum((unsigned char)c)); }
static int is_xdigit(int c)     { return isxdigit((unsigned char)c); }


static void
re_shiftup(recomp_t *rx, size_t idx)
{
    REGEXPATOM *atoms = rx->atoms;
    register REGEXPATOM *dst = &atoms[rx->opidx - 1] + 3;
    register REGEXPATOM *src = dst - 3;
    register REGEXPATOM *ptr = &atoms[idx];

    while (src >= ptr) {
        *dst-- = *src--;
    }
    rx->opidx += 3;
}


/*
 *  regexp_exec ---
 *      Regular expression execution.
 *
 *  Parameters:
 *      prog -              Compiled search expressionn.
 *      buf -               Buffer address.
 *      buflen -            Length of search buffer, in bytes.
 *      offset -            Search offset within buffer.
 *
 *  Returns:
 *      *true* on success, otherwise *false* on error/mismatch.
 */
int
regexp_exec(REGEXP *prog, const char *buf, int buflen, int offset)
{
    const REGEXPATOM *re = (const REGEXPATOM *)prog->program;
    register const char *p;
    register int loops;
    rematch_t match;
    int reincr = 1;

    assert(re);
    assert(buf);
    assert(buflen >= 0);
    assert(offset >= 0 && offset <= buflen);

    match.prog = prog;
    match.start = buf;
    match.regopts = prog->options;

    if (RE_BOL == *re) {
        if (match.regopts.fwd_search && offset) {
            return FALSE;                       /* not start of line */
        }
        if (! match.regopts.fwd_search) {
            offset = 0;                         /* ignore */
        }
    }
    p = buf + offset;
    buflen -= offset;

    prog->setpos = NULL;                        /* \\c marker */
    prog->start = prog->end = NULL;
    prog->groupno = -1;

    if (match.regopts.fwd_search) {
        loops = buflen + 1;

        if (buflen && RE_STRING == *re) {       /* leading string/character */
            const size_t slength = GET16(re + 3);
            const char *sdata = (const char *)(re + 5);

            if (slength > 0) {
                if (buflen < (int)slength) {
                    return 0;                   /* no match */
                }

                if (slength >= 4 && slength <= 0x1fff) {
                    uintptr_t table = (uintptr_t)(re + slength + 5);
                    int leading;

                    if (0x01 & table) ++table;  /* word align */
                    if (match.regopts.case_sense) {
                        if (NULL == (sdata = kmp_search(p, buflen, sdata, slength, (const int16_t *)table))) {
                            return 0;           /* no match */
                        }
                    } else {
                        if (NULL == (sdata = kmp_isearch(p, buflen, sdata, slength, (const int16_t *)table))) {
                            return 0;           /* no match */
                        }
                    }

                    if (RE_FINISH == *(re + LGET16((const LIST *)re))) {
                        prog->start = sdata;    /* simple string, done */
                        prog->end = sdata + slength;
                        return 1;
                    }

                    leading = (sdata - p);      /* leading text to skip */
                    buflen -= leading;
                    loops -= leading;
                    p = sdata;

                 } else {       /* 1, 2 or 3 */
                    int orig_loops;

                    loops -= (slength - 1);     /* reduce by string length */
                    orig_loops = loops;
                    if (match.regopts.case_sense) {
                        const char sch0 = sdata[0];

                        while (--loops > 0) {
                            if (sch0 == *p++ &&
                                    (1 == slength || 0 == memcmp(p - 1, sdata, slength))) {
match:;                         if (RE_FINISH ==*(re + LGET16((const LIST *)re))) {
                                                /* simple string match, done */
                                    prog->start = p - 1;
                                    prog->end = p + slength - 1;
                                    return 1;
                                }
                                --p;
                                break;
                            }
                        }
                    } else {
                        while (--loops > 0) {
                            if (strcasematch(p++, sdata, slength)) {
                                goto match;
                            }
                        }
                    }
                    ++loops;
                    buflen -= orig_loops - loops;
                }
            }
        }
    } else {
        loops  = (p - buf) + 1;
        reincr = -1;
    }

    if (RE_BOL == *re && loops > 1) {           /* only a single loop required */
        loops = 1;
    }

    while (loops-- > 0) {
        if ((prog->length = re_match(&match, re, p, buflen, NULL)) >= 0) {
            prog->start = p;
            prog->end = p + prog->length;
            return 1;
        }
        p += reincr; buflen -= reincr;
    }
    return 0;
}


/*
 *  regexp_free ---
 *      Release the compiled expression.
 *
 *  Returns:
 *      nothing
 */
int
regexp_free(REGEXP *prog)
{
    if (prog) {
        assert(REGEXP_MAGIC == prog->magic);
        if (prog->_dmem) {
            assert(NULL == prog->program || prog->program == prog->_dmem);
            assert(prog->_dsiz);
            chk_free((void *)prog->_dmem);
            prog->_dmem = NULL;
        }
        prog->program = NULL;
        prog->_dsiz = 0;
        prog->magic = 0;
    }
    return 0;
}


/*
 *  is_reword ---
 *      Support function, determine whether a character is a 'word' type character.
 */
static __CINLINE int
is_reword(int ch)
{
    return ('_' == ch || isalnum((unsigned char)ch));
}


/*
 *  re_match ---
 *      Recursive routine to perform pattern matching.
 *
 *  Returns the length of the matched text or -1 if no match was found.
 */
static int
re_match(const rematch_t *match, register const REGEXPATOM *re, register const char *p, int size, loopstate_t *loopstate)
{
    REGEXP *prog = match->prog;
    const REGEXPATOM *re_start = re;
    const char *p_start = p;
    int i, ret;

again:;
    for (;; re += LGET16((const LIST *)re)) {
        switch (*re) {
        case RE_STRING: {
                const size_t slength = GET16(re + 3);
                const char *sdata = (const char *)(re + 5);

                if (size >= (int)slength) {
                    if (match->regopts.case_sense) {
                        if (*sdata == *p &&
                                (1 == slength || 0 == memcmp(p, sdata, slength))) {
                            size -= slength;
                            p += slength;
                            break;
                        }
                    } else {
                        if (strcasematch(p, sdata, slength)) {
                            size -= slength;
                            p += slength;
                            break;
                        }
                    }
                }
            }
            return -1;

        case RE_CLASS:
            if (size > 0) {
                const uint8_t *bitmap = re + 3;
                unsigned char uch = *((unsigned char *)p);

                if (ISSET(bitmap, uch)) {
                    --size;
                    ++p;
                    break;
                }

#if defined(DO_DYNAMIC_CASESENSE)
                if (! match->regopts.case_sense) {
                    if (isalpha(ch)) {          /* not case sensitive */
                        const unsigned char uc = toupper((unsigned char)uch);
                        const unsigned char lc = tolower((unsigned char)uch);
                                                /* TODO: buffer encoding specific */

                        if (ISSET(bitmap, uc) || ISSET(bitmap, lc)) {
                            --size;
                            ++p;
                            break;
                        }
                    }
                }
#endif /*DO_DYNAMIC_CASESENSE*/
            }
            return -1;

        case RE_FINISH:
            return (p - p_start);

        case RE_LOOP:
            if (loopstate && --*loopstate > 0) {
                re = re_start;
                goto again;
            }
            break;

        case RE_END:
            break;

        case RE_BOL:
            if (p != match->start) {
                return -1;
            }
            break;

        case RE_EOL:
            if (size) {                         /* ! eos */
                if (*p != '\n') {               /* and !new-line */
                    return -1;
                }
                --size;
                ++p;
            }
            break;

        case RE_QUESTION:
            if (size <= 0) {
                return -1;
            }
            --size;
            ++p;
            break;

        case RE_OR:
            do {
                if ((ret = re_match(match, re + 3, p, size, NULL)) >= 0) {
                    return (p - p_start) + ret;
                }
                re += LGET16((const LIST *)re);
            } while (RE_OR == *re);
            if ((ret = re_match(match, re, p, size, NULL)) >= 0) {
                return (p - p_start) + ret;
            }
            return -1;

        case RE_STAR: {                         /* minimal */
                if (match->regopts.regexp_chars > 0) {
                    for (i = size; i >= 0; --i) {
                        if ((ret = re_match(match, re + 3, p + i, size - i, NULL)) >= 0) {
                            return (p - p_start) + i + ret;
                        }
                    }
                } else {                        /* max */
                    const char *eptr;

                    for (i = size, eptr = p; i >= 0; eptr++, --i) {
                        if ((ret = re_match(match, re + 3, eptr, i, NULL)) >= 0) {
                            return (p - p_start) + i + ret;
                        }
                    }
                }
            }
            return -1;

        case RE_ONE_OR_MORE:
        case RE_ZERO_OR_MORE: {
                REGEXP saved_regexp = *prog;
                const REGEXPATOM *eptr = re + LGET16((const LIST *)re);
                const REGEXPATOM *re_next = re + 3;
                int matched_len = -1;
                loopstate_t ls = 0;

                i = (RE_ZERO_OR_MORE == *re ? 1 : 2);
                for (; i < size + 2; ++i) {
                    ls = i - 1;
                    if ((ret = re_match(match, 1 == i ? eptr : re_next, p, size, &ls)) < 0) {
                        if (ls)
                            break;
                        if (i >= size + 3)
                            break;
                    } else {
                        matched_len = ret;
                        saved_regexp = *prog;
                        if (matched_len >= size)
                            break;
                        if (match->regopts.regexp_chars > 0)
                            break;              /* minimal -- should only '*' be minimal??? */
                    }
                }

                *prog = saved_regexp;
                if (RE_ZERO_OR_MORE == *re && i == 1 && matched_len < 0)
                    break;
                if (matched_len < 0)
                    return -1;
#if defined(DEBUG_REGEXP)
                re_printbuf(p, latest_rx->end);
#endif
                return (p - p_start) + matched_len;
            }

        case RE_OPEN + 0:
        case RE_OPEN + 1:
        case RE_OPEN + 2:
        case RE_OPEN + 3:
        case RE_OPEN + 4:
        case RE_OPEN + 5:
        case RE_OPEN + 6:
        case RE_OPEN + 7:
        case RE_OPEN + 8:
        case RE_OPEN + 9:
            i = *re - RE_OPEN;
            prog->startp[i] = p;
            break;

        case RE_CLOSE + 0:
        case RE_CLOSE + 1:
        case RE_CLOSE + 2:
        case RE_CLOSE + 3:
        case RE_CLOSE + 4:
        case RE_CLOSE + 5:
        case RE_CLOSE + 6:
        case RE_CLOSE + 7:
        case RE_CLOSE + 8:
        case RE_CLOSE + 9:
            i = *re - RE_CLOSE;
            prog->endp[i] = p;
            if (i > prog->groupno) {
                prog->groupno = i;
            }
            break;

        case RE_SETPOS:
            prog->setpos = p;
            break;

        case RE_WORD_BEGIN:
            if (p == match->start)
                break;
            if (is_reword(*p) && !is_reword(p[-1]))
                break;
            return -1;

        case RE_WORD_END:
            if (0 == size)
                break;
            if (is_reword(*p))
                return -1;
            break;
        }
    }
}


static void
re_print(const REGEXPATOM *re)
{
    char buf[1024], buf1[1024], buf2[1024];
    const REGEXPATOM *start = re;
    const char *p;
    int i;

    trace_log("\n");
    while (1) {
        p = buf1;
        switch (*re) {
        case RE_FINISH:
        case RE_ZERO_OR_MORE:
        case RE_ONE_OR_MORE:
        case RE_STAR:
        case RE_QUESTION:
        case RE_BOL:
        case RE_EOL:
        case RE_END:
        case RE_OR:
        case RE_LOOP:
        case RE_SETPOS:
            p = re_opcodes[*re];
            break;

        case RE_CLASS:
            sprintf(buf1, "CLASS %s", re_printbm(re + 3));
            break;

        case RE_STRING: {
                const size_t slength = GET16(re + 3);
                const char *sdata = (const char *)(re + 5);

                sprintf(buf1, "STRING len=%lu str='%.*s'", (unsigned long)slength, slength, sdata);
            }
            break;

        case RE_OPEN + 0:
        case RE_OPEN + 1:
        case RE_OPEN + 2:
        case RE_OPEN + 3:
        case RE_OPEN + 4:
        case RE_OPEN + 5:
        case RE_OPEN + 6:
        case RE_OPEN + 7:
        case RE_OPEN + 8:
        case RE_OPEN + 9:
            sprintf(buf2, "OPEN-%d", *re - RE_OPEN);
            p = buf2;
            break;

        case RE_CLOSE + 0:
        case RE_CLOSE + 1:
        case RE_CLOSE + 2:
        case RE_CLOSE + 3:
        case RE_CLOSE + 4:
        case RE_CLOSE + 5:
        case RE_CLOSE + 6:
        case RE_CLOSE + 7:
        case RE_CLOSE + 8:
        case RE_CLOSE + 9:
            sprintf(buf2, "CLOSE-%d", *re - RE_CLOSE);
            p = buf2;
            break;

        default:
            sprintf(buf1, "** DONT KNOW = 0x%02x", *re);
            break;
        }

        if (RE_FINISH == *re) {
            sprintf(buf, "[%04lx] --> ---/--- %s\n",
                (long) (re - start), p);
        } else {
            sprintf(buf, "[%04lx] --> %03ld/%03ld %s\n",
                (long)  (re - start), (long) (LGET16((const LIST *)re)),
                (long) ((re - start) + LGET16((const LIST *)re)), p);
        }
        trace_log("\t%s", buf);
        if (RE_FINISH == *re) {
            break;
        }

        if (RE_OR == *re || RE_END == *re ||
                RE_ZERO_OR_MORE == *re || RE_ONE_OR_MORE == *re) {
            i = 3;
        } else {
            i = LGET16((const LIST *)re);
        }
        if (i == 0) i = 3;
        re += i;
    }
}


static const char *
re_printbm(const REGEXPATOM *bitmap)
{
    static char buf[(256*2)+1];
    char *cp = buf;
    int run = 0, isnot = 0;
    int ch;

    *cp++ = '[';
    if (ISSET(bitmap, 0)) {
        *cp++ = '~';
        isnot = 1;
    }

    for (ch = 0; ch < 256; ++ch) {
        int set = ISSET(bitmap, ch);

        if (isnot) set = !set;
        if (set) {
            if (0 == run++) {                   /* opening set */
                switch (ch) {
                case 0x1b:              // ESC
                    *cp++ = '\\', *cp++ = 'e';
                    break;
                case '\f':              // formfeed
                    *cp++ = '\\', *cp++ = 'f';
                    break;
                case '\n':              // newline
                    *cp++ = '\\', *cp++ = 'n';
                    break;
                case '\r': ;            // return
                    *cp++ = '\\', *cp++ = 'r';
                    break;
                case '\t':              // tab
                    *cp++ = '\\', *cp++ = 't';
                    break;
                case '\v':              // vertical tab
                    *cp++ = '\\', *cp++ = 'v';
                    break;
                default:
                    if (ch <= 0x1f) {   // non-printable controls
                        *cp++ = '\\', *cp++ = 'c'; *cp++ = (char)('a' + ch);
                    } else {
                        *cp++ = (char) ch;
                    }
                    break;
                }
            }

        } else {
            if (run > 1) {                      /* close set */
                *cp++ = '-';
                *cp++ = (char) (ch - 1);
            }
            run = 0;
        }
    }

    *cp++ = ']';
    *cp = '\0';
    assert(cp < buf + sizeof(buf));
    return buf;
}


/*
 *  Support function, substr find using KMP.
 *
 *      Knuth-Morris-Pratt string searching algorithm (or KMP algorithm) searches for occurrences
 *      of a "word" W within a main "text string" S by employing the observation that when a
 *      mismatch occurs, the word itself embodies sufficient information to determine where the
 *      next match could begin, thus bypassing re-examination of previously matched characters.
 *
 *      The algorithm was conceived in 1974 by Donald Knuth and Vaughan Pratt, and independently
 *      by James H. Morris. The three published it jointly in 1977.
 *
 *  Returns TRUE if the string match, otherwise FALSE.
 */
static void
kmp_table(int wordlen, const char *pat, char *word, int16_t *table)
{
    const unsigned char *W = (const unsigned char *)word;
    register int16_t *T = table;
    int i;

    memmove((void *)W, pat, wordlen);
    T[0] = -1;
    for (i = 0; i < wordlen; ++i) {
        T[i+1] = T[i] + 1;
        while (T[i+1] > 0 && W[i] != W[T[i+1]-1]) {
            T[i+1] = T[T[i+1]-1] + 1;
        }
    }
}


static const char *
kmp_search(const char *text, int textlen, const char *word, int wordlen, const int16_t *table)
{
    const void *result = NULL;
    const unsigned char *S = (const unsigned char *)text,
            *W = (const unsigned char *)word;
    const int16_t *T = table;
    int i, j;

    for (i = j = 0; i < textlen;) {
        if (j < 0 || S[i] == W[j]) {
            ++i, ++j;
            if (j >= wordlen) {
                result = text + i - j;
                break;
            }
        }
        else j = T[j];
    }
    return result;
}


/*
 *  Support function, substr find using KMP, with case-folding.
 *  Returns TRUE if the string match, otherwise FALSE.
 */
static void
kmp_itable(int wordlen, const char *pat, char *word, int16_t *table)
{
    unsigned char *W = (unsigned char *)word;
    register int16_t *T = table;
    int i;

    for (i = 0; i < wordlen; ++i) {
        W[i] = (unsigned char)toupper(*((unsigned char *)pat++));
    }
    T[0] = -1;
    for (i = 0; i < wordlen; ++i) {
        T[i+1] = T[i] + 1;
        while (T[i+1] > 0 && W[i] != W[T[i+1]-1]) {
            T[i+1] = T[T[i+1]-1] + 1;
        }
    }
}


static const char *
kmp_isearch(const char *text, int textlen, const char *word, int wordlen, const int16_t *table)
{
    const void *result = NULL;
    const unsigned char *S = (const unsigned char *)text,
            *W = (const unsigned char *)word;
    const int16_t *T = table;
    int i, j;

    for (i = j = 0; i < textlen;) {
        if (j < 0 || toupper(S[i]) == W[j]) {
            ++i, ++j;
            if (j >= wordlen) {
                result = text + i - j;
                break;
            }
        }
        else j = T[j];
    }
    return result;
}


/*
 *  Support function, compares two strings with case insensitivity.
 *  Returns TRUE if the string match, otherwise FALSE.
 */
static __CINLINE int
strcasematch(const char *s1, const char *s2, int len)
{
    register const unsigned char *_s1 = (const unsigned char *)s1,
            *_s2 = (const unsigned char *)s2;

    do {
                                                /* TODO: buffer encoding specific */
        if ((*_s1 != *_s2) && (tolower(*_s1) != tolower(*_s2))) {
            return FALSE;
        }
        if ('\0' == *_s1++) {
            break;
        }
        ++_s2;
    } while (--len != 0);

    return TRUE;
}


#if defined(DEBUG_REGEXP)
static void
re_printbuf(const char *start, const char *end)
{
    const char *cp;
    int i, j;

    if (0 == (DB_REGEXP & x_dflags)) {
        return;
    }

    trace_log("Matched: '");
    for (cp = rx->start; *cp; ++cp) {
        if (*cp == '\n') {
            trace_log("\\n");
        } else {
            trace_log("%c", *cp);
        }
    }
    trace_log("'\n\t");
    for (i = start - rx->start; i > 0; --i) {
        trace_log(" ");
    }
    trace_log("^");
    if (j = end - start) {
        for (i = j - 2; i > 0; --i) {
            trace_log("-");
        }
        trace_log("^");
    }
    trace_log("\n");
}
#endif /*DEBUG_REGEXP*/
