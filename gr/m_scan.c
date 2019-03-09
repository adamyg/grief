#include <edidentifier.h>
__CIDENT_RCSID(gr_m_scan_c,"$Id: m_scan.c,v 1.24 2018/10/11 22:37:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_scan.c,v 1.24 2018/10/11 22:37:23 cvsuser Exp $
 * scanf implementation.
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

#include <editor.h>
#include <stdarg.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_scan.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "main.h"
#include "symbol.h"

typedef struct iostream {
    int               (*s_refill)(struct iostream *);
    unsigned            s_total;
    unsigned            s_count;
    unsigned char  *    s_cursor;
    unsigned            s_ungetb[4];    /* min of 3 required */
    unsigned            s_ungetc;
} IOSTREAM_t;

/* flags */
#define F_CONSUME       0x0001                  /* consume matched characters */
#define F_WANTWHITE     0x0002                  /* dont consume leading white-space */
#define F_UNSIGNED      0x0004                  /* unsigned numeric value */
#define F_PREFIX        0x0008
#define F_HEX           0x0010
#define F_BIN           0x0020
#define F_LONG          0x0040
#define F_LONGLONG      0x0080
#define F_SQUOTES       0x0100
#define F_DQUOTES       0x0200
#define F_QESCAPE       0x0400

/* flags during cooking */
#define F_HAVEDIGITS    0x1000                  /* seen digits */
#define F_HAVEZERO      0x2000                  /* seen zero */
#define F_HAVESIGN      0x4000                  /* seen sign */
#define F_MAYBEPREFIX   0x8000                  /* 0x and 0b prefix is (still) legal */

/* datatype classes */
enum {
    D_INTEGER = 1,
    D_STRING,
    D_CHARACTER,
    D_FLOAT,
    D_CHARSET
};


static const unsigned char *setcook(const unsigned char *fmt, unsigned char *tab);


/* initialise an input stream */
static void
ioinput(IOSTREAM_t *io, int (*refill)(IOSTREAM_t *io), const char *buffer, unsigned count)
{
    io->s_total  = 0;
    io->s_refill = refill;
    io->s_cursor = (unsigned char *)buffer;
    io->s_count  = count;
    io->s_ungetc = 0;
}


/* refill the input buffer */
static int
iorefill(IOSTREAM_t *io)
{
    if (io->s_refill) {
        return (io->s_refill)(io);
    }
    return 0;
}


static int
ioget(IOSTREAM_t *io)
{
    if (io->s_ungetc) {
        ++io->s_total;
        return io->s_ungetb[ --io->s_ungetc ];
    }
    if (io->s_count == 0 && iorefill(io) <= 0) {
        return -1;
    }
    --io->s_count, ++io->s_total;
    return *io->s_cursor++;
}


static void
iounget(IOSTREAM_t *io, int c)
{
    assert(io->s_ungetc < (sizeof(io->s_ungetb)/sizeof(io->s_ungetb[0])));
    if (c > 0) {
        io->s_ungetb[ io->s_ungetc++ ] = c;
        --io->s_total;
    }
}


static int
iototal(IOSTREAM_t *io)
{
    return io->s_total;
}


static int
scan_formatted(IOSTREAM_t *io, int offset)
{
    unsigned char cache[512+1], *p;             /* cache/ptr for numeric conversions (512 = ANSI limit) */
    const unsigned char *fmt;
    int arg = 0;

    fmt = (unsigned char *)get_str(offset);     /* format specification */
    arg = offset + 1;                           /* first argument */

    trace_log("scanf(%s)\n", fmt);
    while (*fmt) {
        unsigned width, flags, conv;
        int base, c = 0, f;

        /* literal or start-of-spec */
        if ('%' != (f = *fmt++)) {              /* MCHAR??? */
            if (isspace(f)) {
                for (;;) {                      /* consume optional whitespace */
                    if ((c = ioget(io)) < 0 || !isspace(c)) {
                        iounget(io, c);
                        break;
                    }
                }
                continue;
            }
            goto literal;
        }
        base = width = flags = conv = 0;

        /*
         *  Flags/width/class
         */
more:;  f = *fmt++;
        switch(f) {
        case '%':                               /* literal character */
literal:;   if ((c = ioget(io)) < 0) {
                goto ioerror;
            }
            if (c != f) {
                goto bad_match;
            }
            continue;
                                                /* width */
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            width = (width * 10) + (f - '0');
            goto more;

        case '*':                               /* * -  consume */
            flags |= F_CONSUME;
            goto more;

        case 'l':                               /* l -  long */
            if ('l' == *fmt) {                  /* ll - long long */
                flags |= F_LONGLONG;
                ++fmt;
            } else {
                flags |= F_LONG;
            }
            goto more;

        case 'L':                               /* L -  long long */
            flags |= F_LONGLONG;
            goto more;

        case 'h':                               /* h -  short */
            if ('h' == *fmt) {
                ++fmt;                          /* hh - char */
            }
            goto more;

        case 'q':                               /* q -  quad */
            goto more;

        case 'j':                               /* j -  intmax_t (C99) */
        case 't':                               /* t -  ptrdiff_t (C99) */
        case 'z':                               /* z -  size_t (C99) */
            goto more;

        /*
         *  Conversions.
         */
        case 'd':                               /* decimal */
     /* case 'D':                               -* old style decimal */
            conv = D_INTEGER;
            base = 10;
            break;

        case 'i':                               /* integer/c99 */
            conv = D_INTEGER;
            flags = F_PREFIX;                   /* allow 0 (oct), 0b (binary) and 0x (hex) */
            base = 10;                          /* .. default base (dec) */
            break;

        case 'o':                               /* oct */
            conv = D_INTEGER;
            flags |= F_UNSIGNED;
            base = 8;
            break;

        case 'u':                               /* unsigned integer */
            conv = D_INTEGER;
            flags |= F_UNSIGNED;
            base = 10;
            break;

        case 'x':                               /* hex */
        case 'X':
            conv = D_INTEGER;
            flags |= F_UNSIGNED|F_HEX|F_PREFIX;
            base = 16;
            break;

        case 'e': case 'f': case 'g':           /* float */
        case 'E': case 'F': case 'G':
            conv = D_FLOAT;
            break;

        case 'a':                               /* float (C99) */
        case 'A':
            conv = D_FLOAT;
            break;

        case 'S':                               /* long string */
            flags |= F_LONG;
        case 's':                               /* string */
            conv = D_STRING;
            break;

        case 'C':                               /* long character */
            flags |= F_LONG;
        case 'c':                               /* character */
            flags |= F_WANTWHITE;
            conv = D_CHARACTER;
            break;

        case '[':                               /* character-set */
            conv = D_CHARSET;
            flags |= F_WANTWHITE;
            assert(sizeof(cache) >= 256);
            if (NULL == (fmt = setcook(fmt, cache))) {
                goto bad_format;
            }
            break;

        case 'n':                               /* current input count */
            if (!isa_undef(arg)) {
                sym_assign_int(get_symbol(arg), iototal(io));
            }
            ++arg;
            continue;

        case 'b':                               /* binary (extension) */
            conv = D_INTEGER;
            flags |= F_UNSIGNED|F_BIN|F_PREFIX;
            base = 2;
            break;

        case 'v':                               /* cvs (extension) */
            flags |= F_DQUOTES;
            conv = D_STRING;
            break;

        case 'V':                               /* cvs-microsoft (extension) */
            flags |= F_DQUOTES|F_QESCAPE;
            conv = D_STRING;
            break;

        case 'Q':                               /* quoted string (extension) */
            flags |= F_DQUOTES|F_SQUOTES;
            conv = D_STRING;
            break;

        case 'p':                               /* pointer */
        default:                                /* unknown */
            errorf("scanf: bad conversion '%%%c'", c);
            goto bad_format;

        case '\0':                              /* bad */
            errorf("scanf: unexpected end-of-format, missing conversion");
            goto bad_format;
        }

        if ((c = ioget(io)) < 0) {              /* parse input */
            goto ioerror;
        }

        if (0 == (F_WANTWHITE & flags)) {         /* consume leading white space */
            while (isspace(c)) {
                if ((c = ioget(io)) < 0) {
                    goto ioerror;
                }
            }
        }

        /*
         *  do the conversion.
         */
        switch (conv) {
        case D_INTEGER:
            /*
             *  Integer, which shall be either binary, octal, decimal or
             *  hexadecimal.
             */
            p = cache;                          /* input buffer */

            if (0 == width || width > sizeof(cache)-1) {
                width = sizeof(cache)-1;        /* limit to 512 */
            }

            /* first character maybe a sign */
            if (base != 2 && base != 8)
                if ('-' == c || '+' == c) {
                    *p++ = (unsigned char)c;
                    if (--width == 0 || (c = ioget(io)) < 0) {
                        goto endnumber;
                    }
                    flags |= F_HAVESIGN;
                }

            /* others */
            for (;;) {
                switch (c) {
                /* 0 */
                case '0':
                    if ((F_PREFIX & flags) &&   /* prefix and first digit */
                                0 == (flags & (F_HAVEZERO|F_HAVEDIGITS))) {
                        flags &= ~F_PREFIX;
                        flags |= F_MAYBEPREFIX;
                        base = 8;               /* oct until unless 0b or 0x */
                    }
                    flags |= F_HAVEZERO;
                    break;

                /* binary, if second character of '0b' */
                case 'b': case 'B':
                    if (F_MAYBEPREFIX & flags) {
                        if (0 == ((F_HEX|F_HAVEDIGITS) & flags)) {
                            c = '0';
                            flags &= ~F_MAYBEPREFIX;
                            base = 2;           /* 0b, change base */
                            break;
                        }
                    }
                    /*FALLTHRU*/

                /* a-f, if hex */
                case 'A': case 'C': case 'D': case 'E': case 'F':
                case 'a': case 'c': case 'd': case 'e': case 'f':
                    if (base < 16) {
                        iounget(io, c);
                        goto endnumber;
                    }
                    flags |= F_HAVEDIGITS;
                    break;

                /* hex, if second character of '0x' */
                case 'x': case 'X':
                    if (F_MAYBEPREFIX & flags)
                        if (0 == ((F_BIN|F_HAVEDIGITS) & flags)) {
                            c = '0';
                            flags &= ~F_MAYBEPREFIX;
                            base = 16;          /* 0x, change base */
                            break;
                        }
                    iounget(io, c);
                    goto endnumber;

                /* 1, for all base types */
                case '1':
                    flags |= F_HAVEDIGITS;
                    break;

                /* 2-7, if oct, dec or hex */
                case '2': case '3': case '4': case '5': case '6': case '7':
                    if (base < 8) {
                        iounget(io, c);
                        goto endnumber;
                    }
                    flags |= F_HAVEDIGITS;
                    break;

                /* 8-9, if dec or hex */
                case '8': case '9':
                    if (base < 10) {
                        iounget(io, c);
                        goto endnumber;
                    }
                    flags |= F_HAVEDIGITS;
                    break;

                /* unknown */
                default:
                    iounget(io, c);
                    goto endnumber;
                }

                /* cache and get next (if any) */
                *p++ = (unsigned char)c;
                if (--width == 0 || (c = ioget(io)) < 0) {
                    break;
                }
            }

            /* If neither zero or digits abort */
endnumber:; if (0 == (flags & (F_HAVEZERO|F_HAVEDIGITS))) {
                while (p > cache) {
                    iounget(io, *--p);          /* unstack bad characters */
                }
                goto bad_match;
            }

            *p = 0;

            trace_log("\tinteger (base=%u,width=%u,flags=%x) '%s'\n",
                base, width, flags, cache);

            /* Process the results */
            if (0 == (F_CONSUME & flags)) {
                accint_t res;

                if (F_UNSIGNED & flags) {
                    res = accstrtou((const char *)cache, (char **)NULL, base);
                } else {
                    res = accstrtoi((const char *)cache, (char **)NULL, base);
                }
                if (!isa_undef(arg)) {
                    sym_assign_int(get_symbol(arg), res);
                }
                ++arg;
            }
            break;

        case D_STRING: {
            /*
             *  String, up to first white-space character (space, tab or newline).
             *
             *  The input string stops at white space or at the maximum field
             *  width, whichever occurs first.
             *
             *  The resulting length maybe zero or more charcaters.
             */
                unsigned char quote = 0;
                unsigned acc = 0;

                trace_log("\tstring (width=%u,flags=%x)\n", width, flags);

                if (0 == width) {
                    width = (unsigned) ~0;      /* no width */
                }

                p = acc_expand(acc + 0x80);     /* use accumulator as local (128 byte chunks) */

                /* if first non-space is quote, treat as quoted (ie. "xxxx xxx xx") string */
                if ((F_DQUOTES|F_SQUOTES) & flags) {
                    if (('"' == c && (F_DQUOTES & flags)) ||
                            ('\'' == c && (F_SQUOTES & flags))) {
                        quote = (unsigned char)c;
                        if ((c = ioget(io)) < 0) {
                            c = quote, quote = 0;
                        }
                    }
                }

                /* until space or end-quote */
                for (;;) {
                    if (quote) {                /* quoted text */
                        if (c == quote) {
                            if (F_QESCAPE & flags) {
                                if ((c = ioget(io)) < 0 || c != quote) {
                                    iounget(io, c);
                                    break;
                                }
                            } else {
                                break;          /* .. end of quoted text, consume */
                            }
                        } else if ('\\' == c) { /* .. quote next character */
                            if (0 == (F_QESCAPE & flags)) {
                                if ((c = ioget(io)) < 0) {
                                    break;
                                }
                            }
                        }
                    } else if (isspace(c)) {
                        iounget(io, c);         /* .. break on whitespace */
                        break;
                    }

                    *p++ = (unsigned char)c;    /* .. push character */

                    if (0 == (++acc & 0x7f)) {  /* .. resize for next character */
                        p = ((unsigned char *)acc_expand(acc + 0x80)) + acc;
                    }

                    if (--width == 0 || (c = ioget(io)) < 0) {
                        break;                  /* .. width || EOF */
                    }
                }

                *p = 0;                         /* .. NUL terminate */

                if (0 == (F_CONSUME & flags)) {
                    if (!isa_undef(arg)) {
                        p = acc_expand(acc);
                        if (isa_integer(arg)) {
                            sym_assign_int(get_symbol(arg), (int)strlen((const char *)p));
                        } else if (isa_string(arg)) {
                            sym_assign_str(get_symbol(arg), (const char *)p);
                        }
                    }
                }
                acc_assign_str("", 0);
                ++arg;
            }
            break;

        case D_CHARSET:
            /*
             *  Character set, matches a nonempty sequence of characters from the specified set
             *  of accepted characters.
             *
             *  The input string stops at non-matching characters or at the maximum field width,
             *  whichever occurs first.
             *
             *  The resulting length must be one or more characters.
             */
            trace_log("\tcharset (width=%u,flags=%x)\n", width, flags);

            if (0 == width) {
                width = (unsigned)~0;           /* no width */
            }

            if (F_CONSUME & flags) {
                unsigned acc = 0;

                for (;;) {
                    if (0 == cache[(unsigned char) c]) {
                        iounget(io, c);         /* .. break on non-match */
                        break;
                    }
                    if (--width == 0 || (c = ioget(io)) < 0) {
                        break;                  /* .. width || EOF */
                    }
                    ++acc;                      /* .. accumulated count */
                }
                if (0 == acc) {
                    goto bad_match;             /* must match one or more */
                }

            } else {
                unsigned acc = 0;

                p = acc_expand(acc + 0x80);     /* use accumulator as local (128 byte chunks) */
                for (;;) {
                    if (0 == cache[(unsigned char) c]) {
                        iounget(io, c);         /* .. break on non-match */
                        break;
                    }

                    *p++ = (unsigned char)c;    /* .. push character */

                    if (0 == (++acc & 0x7f)) {  /* .. resize for next character */
                        p = ((unsigned char *)acc_expand(acc + 0x80)) + acc;
                    }

                    if (--width == 0 || (c = ioget(io)) < 0) {
                        break;                  /* .. width || EOF */
                    }
                }

                if (0 == acc) {
                    goto bad_match;             /* must match one or more */
                }
                *p = 0;                         /* .. NUL terminate */

                if (!isa_undef(arg)) {
                    p = acc_expand(acc);
                    if (isa_integer(arg)) {
                        sym_assign_int(get_symbol(arg), (int)strlen((const char *)p));
                    } else  if (isa_string(arg)) {
                        sym_assign_str(get_symbol(arg), (const char *)p);
                    }
                }
                acc_assign_str("", 0);
                ++arg;
            }
            break;

        case D_CHARACTER:
            /*
             *  Character, matches a sequence of width count characters (default of 1).
             */
            trace_log("\tcharacter (width=%u,flags=%u)\n", width, flags);

            if (0 == width) {
                width = 1;                      /* default, one character */
            }

            if (F_CONSUME & flags) {
                /*
                 *  ignore, one or more characters
                 */
                for (;;) {
                    if (--width == 0 || (c = ioget(io)) < 0) {
                        break;                  /* .. width || EOF */
                    }
                }
                if (width) {
                    goto bad_match;             /* abs width count */
                }

            } else if (1 == width) {
                /*
                 *  single character read
                 */
                if (!isa_undef(arg)) {
                    if (isa_integer(arg)) {
                        sym_assign_int(get_symbol(arg), c);
                    } else if (isa_string(arg)) {
                        unsigned char s[2];
                        s[0] = (unsigned char)c;
                        s[1] = '0';
                        sym_assign_str(get_symbol(arg), (const char *)s);
                    }
                }
                ++arg;

            } else {
                /*
                 *  general, multiple character read
                 */
                int acc = 0;

                p = acc_expand(acc + 0x80);     /* use accumulator as local (128 byte chunks) */
                for (;;) {
                    *p++ = (unsigned char)c;    /* .. push character */

                    if ((++acc & 0x7f) == 0) {  /* .. resize for next character */
                        p = ((unsigned char *)acc_expand(acc + 0x80)) + acc;
                    }

                    if (--width == 0 || (c = ioget(io)) < 0) {
                        break;                  /* .. width || EOF */
                    }
                }
                if (width) {
                    goto bad_match;             /* abs width count */
                }
                *p = 0;                         /* .. NUL terminate */

                if (!isa_undef(arg)) {
                    p = acc_expand(acc);
                    if (isa_integer(arg)) {
                        sym_assign_int(get_symbol(arg), (int)strlen((const char *)p));
                    } else if (isa_string(arg)) {
                        sym_assign_str(get_symbol(arg), (const char *)p);
                    }
                }
                acc_assign_str("", 0);
                ++arg;
            }
            break;

        case D_FLOAT:
            trace_log("\tfloat\n");
            break;
        }
    }
    system_errno(0);
    return (arg - offset) - 1;

bad_format:;
    system_errno(EINVAL);
    return -1;

ioerror:;
    system_errno(EIO);
    return -2;

bad_match:;
    system_errno(0);
    return 0;
}


/*
 *  character class interface adapters, resolve calling convention issues.
 */

static int
is_ascii(int c)
{
#if defined(HAVE___ISASCII)
    return __isascii((unsigned char)c);
#elif defined(HAVE_ISASCII)
    return isascii((unsigned char)c);
#else
    return (c >= 0 && c <= 0x7f);
#endif
}

static int is_alnum(int ch)     { return isalnum((unsigned char)ch);  }
static int is_alpha(int ch)     { return isalpha((unsigned char)ch);  }

static int is_blank(int c)      
{
#if defined(HAVE___ISBLANK)
    return __isblank((unsigned char)c));
#elif defined(HAVE_ISBLANK)
    return isblank((unsigned char)c);
#else
    return (' ' == c || '\t' == c);
#endif
}

static int is_cntrl(int ch)     { return iscntrl((unsigned char)ch);  } 
static int is_csym(int ch)      { return ('_' == ch || isalnum((unsigned char)ch)); }
static int is_digit(int ch)     { return isdigit((unsigned char)ch);  } 
static int is_graph(int ch)     { return isgraph((unsigned char)ch);  } 
static int is_lower(int ch)     { return islower((unsigned char)ch);  } 
static int is_print(int ch)     { return isprint((unsigned char)ch);  } 
static int is_punct(int ch)     { return ispunct((unsigned char)ch);  } 
static int is_space(int ch)     { return isspace((unsigned char)ch);  } 
static int is_upper(int ch)     { return isupper((unsigned char)ch);  } 
static int is_word(int ch)      { return ('_' == ch || '-' == ch || isalnum((unsigned char)ch)); }
static int is_xdigit(int ch)    { return isxdigit((unsigned char)ch); }


/*
 *  character class implementation.
 */

static const unsigned char *
setcook(const unsigned char *fmt, unsigned char *tab)
{
    static const struct {                       /* character classes */
        const char         *name;
        int                 len;
        int               (*isa)(int);
    } character_classes[] = {
        { "ascii",  5,  is_ascii },             /* ASCII character. */
        { "alnum",  5,  is_alnum },             /* An alphanumeric (letter or digit). */
        { "alpha",  5,  is_alpha },             /* A letter. */
        { "blank",  5,  is_blank },             /* A space or tab character. */
        { "cntrl",  5,  is_cntrl },             /* A control character. */
        { "csym",   4,  is_csym  },             /* A language symbol. */
        { "digit",  5,  is_digit },             /* A decimal digit. */
        { "graph",  5,  is_graph },             /* A character with a visible representation. */
        { "lower",  5,  is_lower },             /* A lower-case letter. */
        { "print",  5,  is_print },             /* An alphanumeric (same as alnum). */
        { "punct",  5,  is_punct },             /* A punctuation character. */
        { "space",  5,  is_space },             /* A character producing white space in displayed text. */
        { "upper",  5,  is_upper },             /* An upper-case letter. */
        { "word",   4,  is_word  },             /* A "word" character (alphanumeric plus "_"). */
        { "xdigit", 6,  is_xdigit }             /* A hexadecimal digit. */
        };

    const unsigned char *start = fmt;
    unsigned char v = 0;
    int lvalue, range, c;

    c = *fmt++;

    /* leading special, treat as NOT operator */
    if (c == '^') {                             /* [^.. */
        v = 1, c = *fmt++;
    }

    /* initialise table (assumed tab >= 256) */
    (void) memset(tab, v, 256);
    v = !v;                                     /* flip value */

    /* '-' and ']' at start are treated normally */
    if (c == ']') {                             /* [] or [^] */
        tab[']'] = v, c = *fmt++;

    } else if (c == '-') {                      /* [- or [^- */
        tab['-'] = v, c = *fmt++;
    }

    /* consume until closing ']' or EOS */
    range = lvalue = 0;

    while ('\0' != c) {
        if (']' == c)                           /* end */
            return fmt;

/*      if (c == '[' && fmt[0] == '.') {        // collating symbols (eg [.<.])
//          c = fmt[1];
//          if (0 == c || '.' != fmt[2] || ']' != fmt[3]) {
//              errorf("regexp: Unmatched character-symbol '.]'");
//              return FALSE;
//          }
//          fmt += 4;
//      }
**/

        if (range) {                            /* right-side */
            /*
             *  ANSI, the effect of a scanset such as [a-zA-Z0-9] is implementation
             *  defined.
             *
             *  (bsd) V7 Unix scanf treats `a-z' as `the letters a through z',
             *  but treats `a-a' as `the letter a, the character -, and the letter a'.
             *
             *  To avoid confusion, error if the range is not numerically greater than
             *  the left side character.
             */
            if (c == '[') {
                errorf("scanf: r-value cannot be character-class/sequence '%%[%.*s'",
                    (int)(fmt - start) + 1, start);
                return NULL;
            }

            if (c <= lvalue) {
                errorf("scanf: invalid collating order '%%[%.*s'", (int)(fmt - start)+1, start);
                return NULL;
            }

            while (lvalue <= c) {               /* .. mark range */
                tab[lvalue++] = v;
            }
            --range;

            /*
             *  (bsd) V7 Unix scanf also treats formats such as [a-c-e]
             *  as the letters a through e'.
             */
            lvalue = c;

        } else if ('[' == c && *fmt == ':') {
             /*
              * Extension, 'character-classes'
              */
            const unsigned char *cc = ++fmt;    /* start of character-class */

            while (0 != (c = *fmt++)) {
                if (':' == c && ']' == *fmt) {  /* look for closing :] */
                    const int cclen = (fmt - cc) - 1;
                    int i;
                                                /* match class and execute */
                    for (i = (sizeof(character_classes)/sizeof(character_classes[0]))-1; i >= 0; --i)
                        if (cclen == character_classes[i].len &&
                                0 == strncmp((const char *)cc, character_classes[i].name, cclen)) {
                            for (c = 1; c < 256; ++c) {
                                if ((*character_classes[i].isa)(c)) {
                                    tab[c] = v;
                                }
                            }
                            break;
                        }
                    if (i < 0) {
                        errorf("scanf: unknown character-class '%.*s'", cclen, cc);
                        return NULL;
                    }
                    ++fmt;                      /* consume ']' */
                    break;
                }
            }
            if (0 == c) {
                errorf("scanf: unmatched ':]' within '%%[%.*s'", (int)(fmt - start)+1, start);
                return NULL;
            }
            lvalue = 0;                         /* cannot be l-value */

        } else if ('-' == c) {
            /*
             *  ANSI, the `-' is not considered to define a range if the character
             *  following is a closiing bracket.
             */
            if (*fmt == ']') {
                tab['-'] = v;
                return ++fmt;                   /* .. closing */
            }

            if (lvalue == 0) {                  /* missing right-side */
                errorf("scanf: unmatched range '%%[%.*s'", (int)(fmt - start)+1, start);
                return NULL;
            }
            ++range;

        } else {
             /*
              * clear character
              */
            tab[c] = v;
            lvalue = c;
        }
        c = *fmt++;                             /* next */
    }

    errorf("scanf: unexpected end-of-format '%%[%.*s'", (int)(fmt - start)+1, start);
    return NULL;
}


/*  Function:           do_sscanf
 *      sscanf primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 *  
 *<<GRIEF>>
    Macro: sscanf - Read formatted data from string.

        int 
        sscanf(string str, string format, ...)

    Description:
        The 'sscanf()' primitive reads data from the string buffer 'str'.
        Data input are stored in the locations specified by argument
        according to the format string 'format'.

        The additional arguments should be objects of the type specified
        by their corresponding 'format' specifier within the format
        string.

    Parameters:
        str - Source buffer.

        format - String that contains a format string, see below.

        ... - Additional arguments; depending on the format string, the
            function may expect a sequence of additional arguments, each
            containing a reference to a variable where the
            interpretation of the extracted characters is stored with
            the appropriate type. There should be at least as many of
            these arguments as the number of values stored by the format
            specifiers. Additional arguments are ignored by the function.

    Return Value:
        The 'sscanf()' primitive returns the number of input fields
        that were successfully converted. An EOF (-1) is returned if
        an error is encountered.

    Macro Portability: 
        A Grief extension.

    Format Specification:
        The 'format' may be composed of one or more whitespace
        characters, non whitespace characters, and format specifications.

        The format string is read from left to right. Characters that
        are not part of the format specifications must match characters
        in the input stream. These characters are read from the input
        stream but are discarded and not stored. If a character in the
        input stream conflicts with the format string, scanf terminates.
        Any conflicting characters remain in the input stream.

        o whitespace characters - 
            blank (' '), tab ('\t'), or newline ('\n'), cause scanf to
            skip whitespace characters in the input stream. A single
            whitespace character in the format string matches 0 or more
            whitespace characters in the input stream.

        o non whitespace characters - 
            with the exception of the percent sign ('%'), cause scanf to
            read but not store a matching character from the input
            stream. The scanf function terminates if the next character
            in the input stream does not match the specified
            non-whitespace character.

        o format specifications - 
            begin with a percent sign ('%') and cause scanf to read and
            convert characters from the input stream to the specified
            type values. The converted value is stored to an argument
            from the parameter list. Characters following a percent sign
            that are not recognized as a format specification are treated
            as ordinary characters. For example, %% matches a single
            percent sign in the input stream.

    Format Specification:

        The first format specification encountered in the format string
        references the first argument after 'format'. The scanf function
        converts input characters and stores the value using the format
        specification. The second format specification accesses the
        second argument after 'format', and so on. If there are more
        arguments than format specifications, the extra arguments are
        ignored. Results are unpredictable if there are not enough
        arguments for the format specifications.

        Values in the input stream are called input fields and are
        delimited by whitespace characters. When converting input fields, 
        scanf ends a conversion for an argument when a whitespace
        character or another unrecognized character is encountered.

        Format specifications have the following format:

>           % [*] [width] type

        Each field in the format specification can be a single character
        or a number which specifies a particular format option.

        The type field is where a single character specifies whether
        input characters are interpreted as a character, string, or
        number. This field can be any one of the characters in the
        following table.

(start table "Type Specifiers","scanf types",format=nd)
            [Character  [Argument Type  [Input Format                                    ]

        !   d           int &           Signed decimal number.

        !   i           int &           Signed decimal, hexadecimal, or octal integer.

        !   u           int &           Unsigned decimal number.

        !   o           int &           Unsigned octal number.

        !   x           int &           Unsigned hexadecimal number.

        !   e           float &         Floating-point number.

        !   f           float &         Floating-point number.

        !   g           float &         Floating-point number.

        !   c           int &           A single character.

        !   s           string &        A string of characters terminated by whitespace.

        !   \[          string &        A string not to be delimited by space characters.

(end table)

        An asterisk ('*') as the first character of a format
        specification causes the input field to be scanned but not
        stored. The asterisk suppresses assignment of the format
        specification.

        The width field is a non-negative number that specifies the
        maximum number of characters read from the input stream. No more
        than width characters are read and converted for the
        corresponding argument. However, fewer than width characters may
        be read if a whitespace or other unrecognized character is
        encountered first.

    Character Set:

        '[' indicates a string not to be delimited by space characters.
        
        The conversion specification includes all subsequent characters
        in the format string up to and including the matching right
        square bracket (]).

        The characters between the square brackets comprise the scanset, 
        unless the character after the left square bracket is a
        circumflex (^), in which case the scanset contains all
        characters that do not appear in the scanlist between the
        circumflex and the right square bracket.

        If the conversion specification begins with "[]" or "[^]", the
        right square bracket is included in the scanlist and the next
        right square bracket is the matching right square bracket that
        ends the conversion specification; otherwise the first right
        square bracket is the one that ends the conversion specification.

        If a hyphen character (-) is in the scanlist and is not the
        first character, nor the second where the first character is a
        circumflex (^), nor the last character, it indicates a range of
        characters to be matched. To include a hyphen, make it the last
        character before the final close bracket. For instance, 
        `[^]0-9-]' means the set `everything except close bracket, zero
        through nine, and hyphen'.

     *Character classes*

        Within a bracket expression, the name of a character class
        enclosed in [: and :] stands for the list of all characters (not
        all collating elements!) belonging to that class.

(start table "Type Specifiers",Types,format=nd)

        [Identifier     [Description                                        ]

    !   alpha           A letter.
    !   upper           An upper-case letter.
    !   lower           A lower-case letter.
    !   blank           A space or tab character.
    !   digit           A decimal digit.
    !   xdigit          A hexadecimal digit.
    !   alnum           An alphanumeric (letter or digit).
    !   print           An alphanumeric (same as alnum).
    !   blank           A space or tab character.
    !   space           A character producing white space in displayed text.
    !   punct           A punctuation character.
    !   graph           A character with a visible representation.
    !   cntrl           A control character.
    !   word            A "word" character (alphanumeric plus "_").

(end table)

 */
void
do_sscanf(void)                 /* (string buf, string format, ...) */
{
    const char *input = get_str(1);             /* input string */
    IOSTREAM_t io;
    int ret;

    ioinput(&io, NULL, input, (int)strlen(input));
    ret = scan_formatted(&io, 2);
    acc_assign_int((accint_t) ret);
}
/*end*/
