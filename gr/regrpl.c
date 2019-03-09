#include <edidentifier.h>
__CIDENT_RCSID(gr_regrpl_c,"$Id: regrpl.c,v 1.15 2018/10/01 20:59:48 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regrpl.c,v 1.15 2018/10/01 20:59:48 cvsuser Exp $
 * Regular expression replacement.
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#if defined(LOCAL_MAIN)
#define HAVE_CONFIG_H
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>

#define __CINLINE       inline
#define chk_alloc       malloc
#define chk_realloc     realloc
#define chk_free        free
#define trace_log       printf
#define errorf          printf

#else
#define ED_ASSERT
#include <editor.h>
#include <edassert.h>
#include "debug.h"
#include "echo.h"
#include "word.h"
#endif

#include "m_search.h"
#include "regprog.h"
#include "regrpl.h"

#if (1)
#define SRCH_ENABLED
#define SRCH_TRACE(x)   search_trace x;
#else
#define SRCH_TRACE(x)
#endif

enum _rplcodes {
    RPL_END             = 0x00,                 /* End of compiled expression. */
    RPL_SIMPLE          = 0x01,                 /* Simple text replacement. */
    RPL_STRING          = 0x02,                 /* String text. */
    RPL_CAPTURE         = 0x03,                 /* Capture. */
    RPL_CAPTUREMAX      = 0x04,                 /* Highest capture. */
    RPL_HINTS           = 0x05,                 /* Requirement hints. */
    RPL_MATCH           = 0x06,                 /* Matched region. */
    RPL_BEFORE          = 0x07,                 /* Everything prior to matched string. */
    RPL_AFTER           = 0x08,                 /* Everything after to matched string. */
};

enum _rphcodes {
    RPH_MATCH           = 0x01,
    RPH_BEFORE          = 0x02,
    RPH_AFTER           = 0x04,
    RPH_CAPTURE         = 0x08,
    RPH_AWK             = 0x10,
    RPH_UNIX            = 0x20,
    RPH_PERL            = 0x40
};

#define REGRPL_DEFAULT  512                     /* Default allocation size. */
#define REGRPL_UNIT     128                     /* Round allocation size to 128 bytes */
#define REGRPL_ROUND(v) (((v) + (REGRPL_UNIT * 2)) &~ (REGRPL_UNIT - 1))

static REGRPL *         rpl_comp(regrpl_t *rpl, const char *replace, int mode, int flags);
static REGRPL *         rpl_text(regrpl_t *rpl, REGRPL *rp, uint8_t token, const char *start, const char *end, uint16_t escaped, uint16_t *storage);

static __CINLINE REGRPL *
PUT16(REGRPL *rp, const uint16_t val)
{
    *rp++ = (uint8_t) (val >> 8);
    *rp++ = (uint8_t) (val);
    return rp;
}

static __CINLINE const REGRPL *
GET16(const REGRPL *rp, uint16_t *value)
{
    register uint16_t val;
    val  = ((uint16_t)*rp++) << 8;
    val |= *rp++;
    *value = val;
    return rp;
}


/*  Function:           regrpl_comp
 *      Compile a replace expression.
 *
 *  Parameters:
 *      rpl -               Replacment object.
 *      replace -           Replace pattern.
 *      mode -              Expression mode.
 *      flags -             Flags/modifiers.
 *
 *  AWK Variables:
 *      \0                  Matched string.
 *      \1, \2 ...          Xth captured expression.
 *      &                   Match string, same as \0.
 *
 *  Perl Variables:
 *      $1, $2 ...          Xth captured expression.
 *      $+                  Last parenthesized pattern match.
 *      $`                  Everything prior to matched string.
 *      $&                  Entire matched string.
 *      $'                  Everything after to matched string.
 */
int
regrpl_comp(regrpl_t *rpl, const char *replace, int mode, int flags)
{
    if (NULL == rpl ||
            (NULL == (rpl->prog = rpl_comp(rpl, replace, mode, flags)))) {
        return -1;
    }
    return 0;
}


static __CINLINE REGRPL *
rpl_alloc(regrpl_t *rpl, size_t psiz)
{
    void *pmem = rpl->_pmem;

    if (NULL == pmem || psiz >= rpl->_psiz) {
        const size_t npsiz = REGRPL_ROUND(psiz);

        if (NULL == (pmem = chk_realloc(pmem, npsiz))) {
            return NULL;
        }
        rpl->_pmem = pmem;
        rpl->_psiz = npsiz;
    }
    return pmem;
}


static __CINLINE REGRPL *
rpl_more(regrpl_t *rpl, REGRPL *rp, size_t len)
{
    REGRPL *pmem = rpl->_pmem;
    const size_t plen = (rp - pmem),            /* current length */
            psiz = plen + (len + 32);           /* required length, plus headroom */

    if (psiz >= rpl->_psiz || NULL == pmem) {
        const size_t npsiz = REGRPL_ROUND(psiz);

        if (NULL == (pmem = (REGRPL *)chk_realloc(pmem, npsiz))) {
            return NULL;
        }
        rpl->_pmem = pmem;
        rpl->_psiz = npsiz;
    }
    return pmem + plen;
}


static REGRPL *
rpl_comp(regrpl_t *rpl, const char *replacement, int mode, int flags)
{
    const int perl = (RE_PERL == mode || RE_RUBY == mode || RE_TRE == mode || (SF_PERLVARS & flags));
    const int unx  = (RE_UNIX == mode || RE_EXTENDED == mode);
    const int awk  = ((SF_AWK & flags) && !perl);

    REGRPL *rpret, *rp;
    uint16_t storage = 0;
    uint8_t hints = 0;

    const char *start;
    uint16_t escaped = 0;
    int ch;

    if (NULL == (start = replacement)) {
	return NULL;
    }

#define REP_CLOSEFRAME(__end) \
        if (__end > start) { \
            rp = rpl_text(rpl, rp, RPL_STRING, start, __end, escaped, &storage); \
            escaped = 0; \
        }

    rpret = rpl_alloc(rpl, REGRPL_DEFAULT);
    rp = rpret + 4;                             /* HINT(1+2+1) */

    if (perl) hints |= RPH_PERL; 
    if (unx)  hints |= RPH_UNIX; 
    if (awk)  hints |= RPH_AWK;

    while (0 != (ch = *replacement)) {
        if ('\\' == ch) {
            const char *end = replacement;
            int nch;

            if (0 != (nch = *++replacement)) {
                switch (nch) {
                case '0':                       
                    if (perl) {
                        ++replacement;
                        break;
                    } else if (awk) {           /* \0 */
                        REP_CLOSEFRAME(end)
                        *rp++ = RPL_MATCH;
                        hints |= RPH_MATCH;
                        start = ++replacement;
                        break;
                    }
                    /*FALLTHRU*/                /* \0...\9 */

                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                case '9':
                    REP_CLOSEFRAME(end)
                    if (perl || awk) {          /* \1...\9 */
                        *rp++ = RPL_CAPTURE;
                        *rp++ = (REGRPL)(nch - '1');
                    } else {                    /* \0...\9 */
                        *rp++ = RPL_CAPTURE;
                        *rp++ = (REGRPL)(nch - '0');
                    }
                    hints |= RPH_CAPTURE;
                    start = ++replacement;
                    break;
                default:
                    ++escaped;
                    ++replacement;
                    break;
                }
            }
            continue;

        } else if ('&' == ch) {                 /* & */
            if (awk || unx) {
                REP_CLOSEFRAME(replacement)
                *rp++ = RPL_MATCH;
                hints |= RPH_MATCH;
                start = ++replacement;
                continue;
            }

        } else if ('$' == ch && perl) {
            const char *end = replacement;
            int nch;

            if (0 != (nch = *++replacement)) {
                switch (nch) {                  /* $1 ... $9 */
                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8':
                case '9':
                    REP_CLOSEFRAME(end)
                    *rp++ = RPL_CAPTURE;
                    *rp++ = (REGRPL)(nch - '1');
                    hints |= RPH_CAPTURE;
                    start = ++replacement;
                    break;
                case '{':                       /* ${<captureno>} */
                    REP_CLOSEFRAME(end)
                    start = ++replacement;
                    while (0 != (nch = *replacement)) {
                        if (! isdigit((unsigned char)nch)) {
                            if ('}' == nch) {
                                ++replacement;
                            }
                            break;
                        }
                        ++replacement;
                    }
                    *rp++ = RPL_CAPTURE;
                    *rp++ = (REGRPL)atoi(start) - 1;
                    hints |= RPH_CAPTURE;
                    start = replacement;
                    break;
                case '+':                       /* $+ */
                    REP_CLOSEFRAME(end)
                    *rp++ = RPL_CAPTUREMAX;
                    start = ++replacement;
                    hints |= RPH_CAPTURE;
                    break;
                case '`':                       /* $` */
                    REP_CLOSEFRAME(end)
                    *rp++ = RPL_BEFORE;
                    hints |= RPH_BEFORE;
                    start = ++replacement;
                    break;
                case '&':                       /* $& */
                    REP_CLOSEFRAME(end)
                    *rp++ = RPL_MATCH;
                    start = ++replacement;
                    hints |= RPH_MATCH;
                    break;
                case '\'':                      /* $' */
                    REP_CLOSEFRAME(end)
                    *rp++ = RPL_AFTER;
                    hints |= RPH_AFTER;
                    start = ++replacement;
                    break;
                default:
                    break;
                }
            }
            continue;
        }
        ++replacement;
    }

#undef REP_CLOSEFRAME

    rpret = rpl->_pmem;
    if (replacement > start) {                  /* close frame */
        if (rp == (rpret + 4)) {                /* optimisation, simple string */
             rp = rpl_text(rpl, rpret, RPL_SIMPLE, start, replacement, escaped, &storage);
            *rp = RPL_END;
            return rpret;
        }
        rp = rpl_text(rpl, rp, RPL_STRING, start, replacement, escaped, &storage);

    } else if (rp == (rpret + 4)) {             /* empty */
        *rpret = RPL_END;
        return rpret;
    }

    *rp = RPL_END;                              /* terminator */
    assert(rp < (rpret + rpl->_psiz));

     rp = rpret;
    *rp = RPL_HINTS;
     rp = PUT16(++rp, storage);
    *rp = hints;
    return rpret;
}


static REGRPL *
rpl_text(regrpl_t *rpl, REGRPL *rp, uint8_t token,
    const char *start, const char *end, uint16_t escaped, uint16_t *storage)
{
    const size_t len = (end - start) - escaped;

    assert(end > start);
    assert(len > 0 && len < 0x7fff);

    *storage += (uint16_t)len;

     rp = rpl_more(rpl, rp, len);
    *rp = token;
     rp = PUT16(++rp, ((uint16_t)len));

    if (escaped) {
        char ch;

        while (start < end) {
            if ('\\' == (ch = *start++)) {
                if (start < end) {
                    ch = *start++;
                }
                --escaped;
            }
            *rp++ = ch;
        }
        assert(0 == escaped);
    } else {
        memcpy(rp, start, len);
        rp += len;
    }
    return rp;
}


static __CINLINE const char *
rpl_capture(const struct regprog *prog, int group, const char **end)
{
    return (prog->capture)(prog, group, end);
}


/*  Function:           regrpl_exec
 *      Execute a replace expression.
 *
 *  Returns:
 *      A non-negative value on success, with 1 denoting the resources should be released
 *      and 0 denoting either static or pre-eallocated resources; otherwise -1 on error.
 */
int
regrpl_exec(const struct regprog *prog, regrpl_t *rpl)
{
    register const REGRPL *rp = rpl->prog;
    uint16_t siz = 0, /*hints = 0,*/ len = 0;
    register char *dst = NULL, *end;

    if (rp) {
        if (RPL_SIMPLE == *rp) {
            rp = GET16((const REGRPL *)++rp, &len);
            rpl->buf = (const char *)rp;
            rpl->len = len;
            return 0;
        }

        dst = rpl->_dmem;
        end = (dst ? dst + rpl->_dsiz : NULL);

        while (RPL_END != *rp) {
            const char *str = NULL;

            switch (*rp++) {
            case RPL_HINTS:
                rp = GET16((const REGRPL *)rp, &len);
                /*hints = *rp++;*/ ++rp;
                siz += len;
                break;

            case RPL_SIMPLE:
            case RPL_STRING:
                rp = GET16((const REGRPL *)rp, &len);
                str = (const char *)rp;
                rp += len;
                break;

            case RPL_BEFORE:
                len = (uint16_t)(prog->start  - (str = prog->buf));
                break;

            case RPL_MATCH:
                len = (uint16_t)(prog->end    - (str = prog->start));
                break;

            case RPL_AFTER:
                len = (uint16_t)(prog->bufend - (str = prog->end));
                break;

            case RPL_CAPTURE: {
                    const uint8_t capture = *rp++;
                    const char *cend;

                    if (NULL == (str = rpl_capture(prog, capture, &cend))) {
                        errorf("No such capture '%d' in pattern.", capture + 1);
                    } else {
                        len = (uint16_t)(cend - str);
                    }
                }
                break;

            case RPL_CAPTUREMAX: {
                    const uint8_t capture = (uint8_t)prog->groupno;
                    const char *cend;

                    if (NULL == (str = rpl_capture(prog, capture, &cend))) {
                        errorf("No such capture '%d' in pattern.", capture + 1);
                    } else {
                        len = (uint16_t)(cend - str);
                    }
                }
                break;
            }

            if (str && len) {
                if ((dst + len) >= end) {
                    const size_t dlen = (dst - (char *)rpl->_dmem);
                    char *mem;

                    siz += (uint16_t)(rpl->_dsiz + dlen + (len | 0x1f));
                    if (NULL == (mem = (char *)chk_realloc(rpl->_dmem, siz + 0x20))) {
                        goto error;
                    }
                    rpl->_dmem = mem;
                    rpl->_dsiz = siz;
                    dst = mem + dlen;
                    end = mem + siz;
                    siz = 0;
                }
                trace_log("+<%.*s>\n", len, str);
                (void) memcpy(dst, str, len);
                dst += len;
            }
        }
    }

    if (NULL == dst) {
        rpl->buf = "";
        rpl->len = 0;
        return 0;
    }

    rpl->buf = rpl->_dmem;
    rpl->len = (dst - rpl->buf);
    *dst = '\0';
    return 1;

error:;
    rpl->buf = "";
    rpl->len = 0;
    return -1;
}


void
regrpl_reset(regrpl_t *rpl)
{
    if (rpl) {
        rpl->prog = NULL;
    }
}


void
regrpl_free(regrpl_t *rpl)
{
    if (rpl) {
        chk_free(rpl->_dmem);
        chk_free(rpl->prog);
        memset(rpl, 0, sizeof(*rpl));
    }
}


void
regrpl_print(const regrpl_t *rpl)
{
    register const REGRPL *rp = rpl->prog;

    if (rp) {
        while (RPL_END != *rp) {
            uint16_t len;

            switch (*rp++) {
            case RPL_SIMPLE:
                rp  = (REGRPL *)GET16((const REGRPL *)rp, &len);
                trace_log("\tSIMPLE:  <%u/%.*s>\n", len, len, (const char *)rp);
                rp += len;
                break;

            case RPL_STRING:
                rp  = (REGRPL *)GET16((const REGRPL *)rp, &len);
                trace_log("\tSTRING:  <%u/%.*s>\n", len, len, (const char *)rp);
                rp += len;
                break;

            case RPL_CAPTURE:
                trace_log("\tCAPTURE: <%u>\n", *rp++);
                break;

            case RPL_CAPTUREMAX:
                trace_log("\tCAPTURE: <+>\n");
                break;

            case RPL_BEFORE:
                trace_log("\tBEFORE:  <*>\n");
                break;

            case RPL_MATCH:
                trace_log("\tMATCH:   <*>\n");
                break;

            case RPL_AFTER:
                trace_log("\tAFTER:   <*>\n");
                break;

            case RPL_HINTS: {
                    uint8_t hints;

                    rp = (REGRPL *)GET16((const REGRPL *)rp, &len);
                    hints = *rp++;

                    trace_log("\tHINTS:   <len:%u,hint:0x%2x%s%s%s>\n", len, hints,
                        (hints & RPH_AWK ? " awk" : ""), (hints & RPH_UNIX ? " unx" : ""), (hints & RPH_PERL ? " perl" : ""));
                }
                break;
            }
        }
        trace_log("\tEND:\n");
    } else {
        trace_log("\tNUL:\n");
    }
}


#if defined(LOCAL_MAIN)
static void
test(const char *replace, int mode, int flags)
{
    regrpl_t t_rpl = {0}, *rpl = &t_rpl;

    trace_log("  - replace:<%s>\n", replace);
    regrpl_comp(rpl, replace, mode, flags);
    regrpl_print(rpl);
    regrpl_free(rpl);
}


int
main(int argc, char *argv[])
{
    const char *arg;

    if (2 != argc) {
        fprintf(stderr, "Usage: %s [all|brief|awk|unx|perl1-3]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (0 == strcmp(arg = argv[1], "all")) arg = "";

    if (!*arg || 0 == strcmp(arg, "brief")) {
        printf("[brief]\n");
        test("", 0, 0);
        test("hello", 0, 0);
        test("hello\\1world", 0, 0);
        test("\\1", 0, 0);
        test("hello\\1world\\2", 0, 0);
        test("\\1hello\\2world\\3", 0, 0);
    }

    if (!*arg || 0 == strcmp(arg, "awk")) {
        printf("[awk]\n");
        test("&", 0, SF_AWK);
        test("he&llo", 0, SF_AWK);
        test("&he&llo&", 0, SF_AWK);
        test("\\0he\\0llo\\0", 0, SF_AWK);
        test("\\0he\\0l\\1l\\9o\\0", 0, SF_AWK);
    }

    if (!*arg || 0 == strcmp(arg, "unx")) {
        printf("[unx]\n");
        test("&", RE_UNIX, 0);
        test("\\&", RE_UNIX, 0);
        test("he&llo", RE_UNIX, 0);
        test("&he&llo&", RE_UNIX, 0);
        test("\\0he\\0llo\\0", RE_UNIX, 0);
        test("\\0he\\0l\\1l\\9o\\0", RE_UNIX, 0);
    }

    if (!*arg || 0 == strcmp(arg, "perl1")) {
        printf("[perl1]\n");
        test("hello", RE_PERL, 0);
        test("hello$1world", RE_PERL, 0);
        test("$1", RE_PERL, 0);
        test("hello$1world$2", RE_PERL, 0);
        test("$1hello$2world$3", RE_PERL, 0);
    }

    if (!*arg || 0 == strcmp(arg, "perl2")) {
        printf("[perl2]\n");
        test("hello$world", RE_PERL, 0);
        test("$", RE_PERL, 0);
        test("hello$world$", RE_PERL, 0);
        test("$hello$world$", RE_PERL, 0);
    }

    if (!*arg || 0 == strcmp(arg, "perl3")) {
        printf("[perl3]\n");
        test("hello${1}world", RE_PERL, 0);
        test("${1}", RE_PERL, 0);
        test("hello${1}world${2}", RE_PERL, 0);
        test("${1}hello${2}world${3}", RE_PERL, 0);
        test("$1hello$`$&$'world$3", RE_PERL, 0);
    }

    exit(EXIT_SUCCESS);
}
#endif  /*LOCAL_MAIN*/
