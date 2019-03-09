#include <edidentifier.h>
__CIDENT_RCSID(gr_strparse_c,"$Id: strparse.c,v 1.6 2017/01/29 04:33:31 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: strparse.c,v 1.6 2017/01/29 04:33:31 cvsuser Exp $
 * libstr - String to numeric (float/integer) parser.
 *
 *
 *
 * Copyright (c) 1998 - 2017, Adam Young.
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#if defined(HAVE_INF_H)
#include <inf.h>                                /* gcc/linux */
#endif
#include <ctype.h>
#include <errno.h>

#include <libstr.h>

#if defined(LOCAL_DEBUG)
#if defined(LOCAL_MAIN)
#define DDEBUG(__d) printf __d;
#else
#define DDEBUG(__d) trace_log __d;
#endif
#endif
#ifndef DDEBUG
#define DDEBUG(__d)
#endif

struct parse {
    int           (*p_get)(void *);
    int           (*p_unget)(void *, int ch);
    long *          p_long;
    double *        p_double;
    void *          p_parm;
    char *          p_cursor;
    char *          p_end;
    const unsigned char *p_data;
    char            p_buffer[128];
};


#if !defined(NAN)                               /* IEEE NaN/Infinity */
static unsigned     _INF = 0x7F800000UL;
static unsigned     _NAN = 0x7F800001UL;

#define INFINITY    *((float *)(&_INF))
#define NAN         *((float*)(&_NAN))
#endif

#if defined(HAVE_ISFINITE) || defined(isfinite)
#define ISINF(__f)  isfinite(__f)
#elif defined(HAVE__FINITE) || defined(_MSC_VER)
#define ISINF(__f)  _finite(__f)
#else
#define ISINF(__f)  0
#endif

#if defined(HAVE_ISNAN) || defined(isnan)
#define ISNAN(__f)  isnan(__f)
#elif defined(HAVE__ISNAN) || defined(_MSC_VER)
#define ISNAN(__f)  _isnan(__f)
#else
#define ISNAN(__f)  0
#endif

static void         chinit(struct parse *p);
static void         chclose(struct parse *p);
static int          chparse(struct parse *p);
static int          chmatch(struct parse *p, const char *s);
static int          chget(struct parse *p);
static int          chunget(struct parse *p, int ch);

static int          sget(void *parm);
static int          sunget(void *parm, int ch);


static int
chterm(int ch)
{
    return (ch <= 0 || isspace(ch));
}


static int
chparse(struct parse *p)
{
#define INTDIGIT    0x01
#define FRACTDIGIT  0x02
#define EXPNDIGIT   0x04
    int sign = 0, dots = 0, base = 10, digits = 0;
    const char *num;
    int ch, ret = NUMPARSE_ERROR;

    /*
     *  White-space, sign and leading zero.
     */
    chinit(p);
    ch = chget(p);
    while (isspace(ch)) {
        ch = chget(p);
    }
    num = p->p_buffer;
    if ('-' == ch || '+' == ch) {
        sign = ch;
        ch = chget(p);
    }

    /*
     *  0# prefixes
     *      If the value of base is 0, the expected form of the subject sequence is that of a
     *          decimal constant,
     *          octal constant,
     *          binary constant
     *       or hexadecimal constant.
     */
    if ('0' == ch) {
        if ((ch = chget(p)) > 0) {
            switch(ch) {
            case 'x': case 'X':
                base = 16;          // 0x
                num = p->p_cursor;
                ch = chget(p);
                break;

            case '0': case '1': case '3': case '4':
            case '5': case '6': case '7':
                base = 8;           // 0#
                num = p->p_cursor - 1;
                ch = chget(p);
                digits |= INTDIGIT;
                break;

            case 'b': case 'B':
                base = 2;           // 0b
                num = p->p_cursor;
                ch = chget(p);
                break;

            case '.':               // 0.
                digits |= INTDIGIT;
                dots = 1;
                break;

            default:
                if (isspace(ch)) {
                    digits |= INTDIGIT;
                    break;
                }
                ret = NUMPARSE_ERR_SUFFIX;
                goto error;
            }

        } else {
            digits |= INTDIGIT;
        }

    } else if ('.' == ch) {
        ch = chget(p);
        if (!sign) {
            if ('.' == ch) {
                ch = chget(p);
                if (chterm(ch)) {
                    DDEBUG((" ELLIPSIS\n"))
                    return NUMPARSE_ELLIPSIS;
                }
                goto error;

            } else if (chterm(ch)) {
                DDEBUG((" DOT\n"))
                return NUMPARSE_DOT;
            }
        }
        digits |= INTDIGIT;         // .
        dots = 1;
    }

    /*
     *  Leading digits (mantissa including the decimal point)
     *  and optional decimal point.
     */
    while (ch > 0) {
        if (10 == base) {
            if (isdigit(ch)) {
                digits |= (dots ? FRACTDIGIT : INTDIGIT);
            } else if ('.' != ch || ++dots > 2) {
                break;
            }

        } else if (16 == base) {
            if (isxdigit(ch)) {
                digits |= (dots ? FRACTDIGIT : INTDIGIT);
            } else if ('.' != ch || ++dots > 2) {
                break;
            }

        } else if ( 8 == base) {
            if (! isdigit(ch) || ch > '7') {
                break;
            }
            digits |= INTDIGIT;

        } else /*2 == base*/ {
            if ('0' != ch && '1' != ch) {
                break;
            }
            digits |= INTDIGIT;
        }

        ch = chget(p);
    }

    /* Exponent/trailing */
    switch( ch) {
    case 'e': case 'E':         // decimal float
        if (10 == base && digits) {
            ch = chget(p);
            if ('-' == ch || '+' == ch) {
                ch = chget(p);              // sign
            }
            while (isdigit(ch)) {
                digits |= EXPNDIGIT;
                ch = chget(p);
            }
            if (0 == (digits & EXPNDIGIT)) {
                ret = NUMPARSE_ERR_EXPONENT;
                goto error;
            }
        }
        break;

    case 'p': case 'P':         // hexadecimal float
        if (10 == base && digits) {
            const char p1 =
                (p->p_cursor > p->p_buffer ? p->p_cursor[-1] : 0);

            if ('e' == p1 || 'E' == p1) {   // EP
                ch = chget(p);
                if ('-' == ch || '+' == ch) {
                    ch = chget(p);          // sign
                }
                while (isxdigit(ch)) {
                    digits |= EXPNDIGIT;
                    ch = chget(p);
                }
                if (0 == (digits & EXPNDIGIT)) {
                    ret = NUMPARSE_ERR_EXPONENT;
                    goto error;
                }
            }
        }
        break;

    case 'n': case 'N':         // NaN
        if (10 == base && !dots && !digits) {
            if (2 == chmatch(p, "an")) {
                *p->p_double = NAN;
                DDEBUG((" nan(%d)", ISNAN(*p->p_double)))
                return NUMPARSE_FLOAT;
            }
        }
        break;

    case 'i': case 'I':         // Inf[inite]
        if (10 == base && !dots && !digits) {
            int m;

            if (2 == chmatch(p, "nf") &&
                    (0 == (m = chmatch(p, "inite")) || 5 == m)) {
                *p->p_double = INFINITY;
                DDEBUG((" infinite(%d)", !ISINF(*p->p_double)))
                return NUMPARSE_FLOAT;
            }
        }
        break;

//TODO
//  case '#':                   // 1.#INF, 1.#QNAN, 1.#SNAN
//      break;
    }

    /* Convert */
    DDEBUG((" base:%u, sign:%u, digits:%x, dots:%u, last:%d/%c", base, sign, digits, dots, ch, (ch > 0 ? ch : ' ')))
    if (isspace(ch)) {
        chunget(p, ch);
        ch = 0;
    }

    if (ch <= 0) {

        chclose(p);

        if ((dots <= 1 && (digits & (FRACTDIGIT|EXPNDIGIT))) || (1 == dots && (INTDIGIT == digits))) {
            /*
             *  A decimal ASCII floating-point number, optionally preceded by white space.
             *
             *      Must have form "-I.FE-X", where
             *
             *          I is the integer part of the mantissa,
             *          F is the fractional part of the mantissa prefixed with a dot,
             *      and X is the exponent.
             *
             *  Either of the signs may be "+", "-", or omitted. Either I or F may be
             *  omitted, or both. The decimal point is not necessary unless F is present.
             */
            double dret;

            errno = 0; dret = strtod(p->p_buffer, NULL);
            if (ERANGE == errno) {
                DDEBUG((" OVER/UNDERFLOW\n"))
                return (dret == HUGE_VAL ? NUMPARSE_ERR_OVERFLOW : NUMPARSE_ERR_UNDERFLOW);
            }
            *p->p_double = dret;
            DDEBUG((" FLOAT(%g)\n", dret))
            return NUMPARSE_FLOAT;

        } else if (0 == dots && INTDIGIT == digits) {
            /*
             *  An integer number, either decimal, hex, octal or binary.
             */
            char *ep;
            long lret;

            errno = 0; lret = strtol(num, &ep, base);
            if (ERANGE == errno) {
                DDEBUG((" OVER/UNDERFLOW\n"))
                return (lret == LONG_MIN ? NUMPARSE_ERR_OVERFLOW : NUMPARSE_ERR_UNDERFLOW);
            }
            if (!*ep) {
                *p->p_long = lret;
                DDEBUG((" INTEGER(%d)\n", lret))
                return NUMPARSE_INTEGER;
            }
        }
    }

error:;
    chunget(p, ch);
    DDEBUG((" ERROR\n"))
    return ret;
}


static void
chinit(struct parse *p)
{
    *p->p_double = 0;
    *p->p_long   = 0;
    p->p_cursor  = p->p_buffer;
    p->p_end     = p->p_buffer + sizeof(p->p_buffer);
}


static void
chclose(struct parse *p)
{
    *p->p_cursor = 0;
}


static int
chmatch(struct parse *p, const char *s)
{
    int mch, ch, count = 0;

    DDEBUG((" match(%s) [", s))
    if (s && *s) {
        while ((mch = *s++) != 0 && (ch = chget(p)) > 0) {
            if (tolower(ch) != tolower(mch)) {
                break;
            }
            ++count;
        }
    }
    DDEBUG(("]=%d, ", count))
    return count;
}


static int
chget(struct parse *p)
{
    int ch;

    if (p->p_cursor >= p->p_end ||              /* buffer overflow or EOF */
            (ch = (*p->p_get)(p->p_parm)) <= 0) {
        DDEBUG(("~"))
        return -1;
    }
    *p->p_cursor++ = ch;
    DDEBUG(("%c", ch))
    return ch;
}


static int
chunget(struct parse *p, int ch)
{
    DDEBUG((" unget(%d/%c)", ch, (ch > 0 ? ch : ' ')))
    if (ch > 0) {
        assert(p->p_cursor > p->p_buffer);
        --p->p_cursor;
        assert(ch == *p->p_cursor);
        return (*p->p_unget)(p->p_parm, ch);
    }
    return -1;
}


/*  str_numerror ---
 *      The str_numerror() function shall map the error number in ret to an error message string
 *      and shall return a pointer to it.
 *
 *      The string pointed to shall not be modified by the application, but may be overwritten by
 *      a subsequent call to str_numerror().
 *
 *  Parameters:
 *      ret - Return code to be decoded.
 *
 *  Returns:
 *      Constant buffer containing the error message.
 */
const char *
str_numerror(int ret)
{
    const struct errors {
        int ret;
        const char *err;
    } errors[] = {
        { NUMPARSE_ERR_UNDERFLOW, "numeric underflow" },
        { NUMPARSE_ERR_OVERFLOW,  "numeric overflow" },
        { NUMPARSE_ERR_EXPONENT,  "invalid exponent" },
        { NUMPARSE_ERR_SUFFIX,    "invalid suffix on numeric constant" }
        };

    if (ret <= 0) {
        const struct errors *e = errors;
        unsigned i;

        for (i = 0; i < (sizeof(errors)/sizeof(errors[0])); ++i, ++e) {
            if (ret == e->ret) {
                return e->err;
            }
        }
        return "syntax error";
    }
    return "success";
}


/*  str_numparse ---
 *      The str_numparse() function shall convert the initial portion of the string
 *      pointed to by 'str' either a type long and double.
 *
 *      First, they decompose the input string into three parts:
 *
 *          o An initial, possibly empty, sequence of white-space characters (see isspace()).
 *
 *          o A subject sequence interpreted as an integer represented or floating
 *              point in some radix determined by the value of base.
 *
 *          o A final string of one or more unrecognised characters, including the
 *              terminating null byte of the input string.
 *
 *  Parameters:
 *      str - Address of the string buffer.
 *      dp - Double/floating point numeric result.
 *      lp - Long/integer numeric result.
 *      len - Populate with the number of characters processed from the source buffer 'str'.
 *
 *  Returns:
 *      Upon successful completion, these functions shall return the type of converted
 *      value, if any.
 *
 *          o NUMPARSE_INTEGER(1)  -       integer constant.
 *          o NUMPARSE_FLOAT(2) -          float constant.
 *          o NUMPARSE_DOT(3) -            single dot (.).
 *          o NUMPARSE_ELLIPSIS(4) -       double dot (..).
 *
 *      If no conversion could be performed or outside the range of representable values,
 *      a non-positive error code shall be returned.
 *
 *          o NUMPARSE_ERROR(0) -          syntax error.
 *          o NUMPARSE_ERR_UNDERFLOW(-4) - numeric underflow.
 *          o NUMPARSE_ERR_OVERFLOW(-3) -  numeric overflow.
 *          o NUMPARSE_ERR_EXPONENT(-2) -  invalid exponent.
 *          o NUMPARSE_ERR_SUFFIX(-1) -    invalid suffix on numeric constant.
 */
int
str_numparse(const char *str, double *dp, long *lp, int *len)
{
    struct parse p = {0};
    int ret;

    p.p_double = dp;
    p.p_long   = lp;
    p.p_get    = sget;
    p.p_unget  = sunget;
    p.p_parm   = (void *)&p;
    p.p_data   = (unsigned char *)str;

    if ((ret = chparse(&p)) > NUMPARSE_ERROR) {
        if (len) {
            *len = (p.p_cursor - p.p_buffer);
        }
        if (ret == NUMPARSE_FLOAT) {
            *lp = (long) *dp;
        }
    }
    return ret;
}


static int
sget(void *parm)
{
    struct parse *p = (struct parse *)parm;
    if (*p->p_data) return (int)*p->p_data++;
    return 0;
}


static int
sunget(void *parm, int ch)
{
    struct parse *p = (struct parse *)parm;
    --p->p_data;
    assert(ch == *p->p_data);
    return 0;
}


int
str_numparsex(int (*get)(void *), int (*unget)(void *, int ch), void *parm, double *dp, long *lp, int *len)
{
    struct parse p = {0};
    int ret;

    p.p_double = dp;
    p.p_long   = lp;
    p.p_get    = get;
    p.p_unget  = unget;
    p.p_parm   = parm;

    if ((ret = chparse(&p)) > NUMPARSE_ERROR) {
        if (len) {
            *len = (p.p_cursor - p.p_buffer);
        }
        if (ret == NUMPARSE_FLOAT) {
            *lp = (long) *dp;
        }
    }
    return ret;
}


#if defined(LOCAL_MAIN)
static const char *
Double(double d)
{
    static char buffer[32];
    char *p;

    (void) sprintf(buffer, "%.20g", d);
    DDEBUG((" (%s) ", buffer))
    for (p = buffer; *p; ++p) {
        if ('.' == *p) {
            for (++p; isdigit(*p); ++p) {
                if ('0' == *p) {
                    char *n = p;
                    for (++p; '0' == *p; ++p);
                    while (*p) {
                        *n++ = *p++;
                    }
                    *n = 0;
                    break;
                }
            }
            break;
        }
    }
    return buffer;
}


void
main(void)
{
#define DOUBLE(__x)         #__x, TDOUBLE,   __x, 0
#define DOUBLE2(__x,__v)    #__x, TDOUBLE,   __v, 0
#define INTEGER(__x)        #__x, TINTEGER,  0, __x
#define NUMBERIC(__x,__v)   __x,  TINTEGER,  0, __v
#define DOT(__x)            __x,  TDOT,      0, 0
#define ELLIPSIS(__x)       __x,  TELLIPSIS, 0, 0
#define ERROR(__x)          __x,  TERROR,    0, 0

    static struct testvalue {
        const char *text;
        enum {TDOUBLE, TINTEGER, TERROR, TDOT, TELLIPSIS} type;
        double d;
        long i;

    } values[] = {
        { INTEGER(0) },
        { INTEGER(+1234) },
        { INTEGER(-1234) },
        { INTEGER(0x1234) },
        { INTEGER(+0x1234) },
        { INTEGER(-0x1234) },
        { INTEGER(01234) },
        { INTEGER(+01234) },
        { INTEGER(-01234) },
        { NUMBERIC("0b010101", 21) },
        { NUMBERIC("+0b010101", 21) },
        { NUMBERIC("-0b010101", -21) },

        { DOUBLE(.0) },
        { DOUBLE(.0e1) },
        { DOUBLE(.1234) },
        { DOUBLE(1.) },
        { DOUBLE(1.234) },
        { DOUBLE(-1.234) },
        { DOUBLE(1.234e123) },
        { DOUBLE(-1.234e123) },
        { DOUBLE(1.234e+123) },
        { DOUBLE(1.234e-123) },
        { DOUBLE(1234e+123) },
        { DOUBLE(1234e-123) },
        { DOUBLE(1234.e123) },

        { DOUBLE(1.7976931348623157e+308) },    // Largest representable number without losing precision.
        { DOUBLE(2.2250738585072014e-308) },    // Smallest number without losing precision.

        { DOUBLE2(NaN, -1 /*NAN*/) },
        { DOUBLE2(Inf, -1 /*INFINITY*/) },
        { DOUBLE2(Infinite, -1 /*INFINITY*/) },
        { DOT(".") },
        { ELLIPSIS("..") },

     // { DOUBLE(-0xc.90fep-2) },               // C11
     // { DOUBLE(-0XC.90FEP-2) },
     
        { ERROR("") },
        { ERROR("-") },
        { ERROR("+") },
        { ERROR("0x") },
        { ERROR("0b") },
        { ERROR("1x") },
        { ERROR("a") },
        { ERROR(".e1") },
        { ERROR("...") },
        { ERROR("0y") },                        // suffix
        { ERROR("123.e") },                     // exponent
        { ERROR("9.999e+999") },                // overflow
        { ERROR("9.999e-999") },                // underflow
        };

    double d;
    long l;
    int v;

    for (v = 0; v < (sizeof(values)/sizeof(values[0])); ++v) {
        const struct testvalue *val = values + v;
        const int r = str_numparse(val->text, &d, &l, NULL);

        printf(" %-24.24s : %d [%s] (%s, %ld/0x%lx/0%lo)\n",
            val->text, r, str_numerror(r), Double(d), l, l, l);
    }
}
#endif  /*LOCAL_MAIN*/
/*end*/
