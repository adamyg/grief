#include <edidentifier.h>
__CIDENT_RCSID(gr_prntf_c,"$Id: prntf.c,v 1.13 2017/01/19 17:09:30 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: prntf.c,v 1.13 2017/01/19 17:09:30 cvsuser Exp $
 * Print formatter.
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

#include "prntf.h"

#include "builtin.h"
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "lisp.h"
#include "main.h"
#include "symbol.h"

#define PRINTF_SIZE     512                     /* Base buffer size, in bytes */
#define PRINTF_INCR     (-2)                    /* Multipler */

typedef struct {
#define F_ZEROPAD       0x0001                  /* padding char; ' ' or '0' */
#define F_LEFTJUST      0x0002                  /* '-' flag specified */
#define F_PLUS          0x0004                  /* '+' flag specified */
#define F_BLANK         0x0008                  /* ' ' flag specified */
#define F_PREFIX        0x0010                  /* octal '0' or Hex '0x'/'0X' prefix */
#define F_LONG          0x0020
#define F_LONG_LONG     0x0040
#define F_CAPS          0x0080
#define F_USIGN         0x0100
#define F_NOPRECISION   0x0200

    char        tmpbuf[512], *tbp;              /* ANSI limit of 509 characters */
#define TRESET(_x)      (_x)->tbp = (_x)->tmpbuf
#define TSTORE(_x, _c) *((_x)->tbp)++ = (char)_c
#define TLENGTH(_x)     ((_x)->tbp - (_x)->tmpbuf)
#define TBUFFER(_x)     ((_x)->tmpbuf)

    unsigned    flags;
    int         width;
    int         precision;

    char *      obp;

#define ORESET(_x)      (_x)->obp = buf_ptr
#define OLENGTH(_x)     ((_x)->obp - buf_ptr)
#define OCHECK(_x,_n)   out_check(_x, _n)
#define OPUTC(_x,_c)    *((_x)->obp)++ = (char)(_c)
#define OPUTS(_x,_s,_n) { \
                            (void)memcpy((_x)->obp, (const void *)_s, _n); \
                            (_x)->obp += _n; \
                        }
} io_t;

static int              get_value(const char **fmt, int *arg);
static void             prtl(io_t *io, const LIST *lp, unsigned level);
static void             outs(io_t *io, const char *s);
static void             outb(io_t *io, accuint_t ul, const char *p);
static void             outl(io_t *io, int radix, accint_t lval);
static void             outf(io_t *io, const char *format, accfloat_t dval);
static void             outr(io_t *io, const char *s, int len);
static void             out_check(io_t *io, int n);
static void             uitoa(accuint_t value, char *buffer, int base);


/*
 *  Buffer used for printf()s. We expand the buffer as needed so that we don't place
 *  any restriction on the size of the formatted output.
 */
static char *           buf_ptr;

static int              buf_size;


/*  Function:           print_formatted
 *      Implement the printf() style processing, but also to support the Grief style extras.
 *
 *  Parameters:
 *      offset -            Format offset within the argument list.
 *
 *      len -               Address of buffer populated with resulting length.
 *
 *  Returns:
 *      Address of internal formatted text buffer, reused on each call.
 *
 */
const char *
print_formatted(int offset, int *len)
{
    const char *fmt;
    int arg = offset + 2;
    io_t io, *iop = &io;
    accint_t lval;
    const char *sval;
    char type;

    if (NULL == buf_ptr) {                      /* prime buffer */
        buf_size = PRINTF_SIZE;
        buf_ptr = (char *) chk_alloc(buf_size);
    }

#if (TODO_VSPRINTF)
    if (isa_list(offset + 1)) {                 /* vsprintf() style */
        const char LIST *lp = get_list(offset + 1));
        LIST *nextlp;

        for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
            /*
             *  .. process atom ..
             */
        }
    }
#endif  /*XXX_VSPRINTF*/

    if (NULL == (fmt = get_xstr(offset + 1))) { /* 05/04/10 */
        ewprintf("msg: missing format specification");
        return "<bad-format>";
    }

    ORESET(iop);
    while (*fmt) {
        /* New specification ? */
        if (*fmt != '%') {
literal:;   OCHECK(iop, 1);
            OPUTC(iop, *fmt++);
            continue;
        }

        if (*++fmt == '%') {
            goto literal;
        }

        io.flags = 0;
        io.width = io.precision = -1;

        /* flags */
        while (*fmt) {
            if (*fmt == '0') {                  /* zero pad */
                io.flags |= F_ZEROPAD;
            } else if (*fmt == '-') {           /* left justification */
                io.flags |= F_LEFTJUST;
            } else if (*fmt == '+') {           /* plus sign */
                io.flags |= F_PLUS;
            } else if (*fmt == ' ') {           /* padded */
                io.flags |= F_BLANK;
            } else if (*fmt == '#') {           /* alternative form */
                io.flags |= F_PREFIX;
            } else {
                break;
            }
            ++fmt;
        }

        /* width/precision */
        if (isdigit(*fmt) || *fmt == '*') {
            if ((io.width = get_value(&fmt, &arg)) < 0) {
                /*
                 *  Negative field widths are not supported; if you attempt
                 *  to specify a negative field width, it is interpreted as
                 *  a minus (`-') flag followed by a positive field width.
                 */
                io.flags |= F_LEFTJUST;
                io.width = -io.width;
            }
        }

        if (*fmt == '.') {
            ++fmt;

            if (*fmt != '*' && !isdigit(*fmt)) {
                io.flags |= F_NOPRECISION;      /* precision missing */
            } else {
                if ((io.precision = get_value(&fmt, &arg)) < 0) {
                   io.precision = -io.precision;
                }
            }
        }

        /* Prefix */
        if (*fmt == 'l' || *fmt == 'L') {       /* long */
            ++fmt;
            io.flags |= F_LONG;
            if ('l' == *fmt) {
                io.flags |= F_LONG_LONG;        /* long long */
                ++fmt;
            }
        } else if (*fmt == 'h') {               /* short */
            ++fmt;
        }

        /* Type */
        type = *fmt++;
        if ('t' == type) {                      /* object type */
            /*
             *  natural type of the operand
             */
            if (isa_integer(arg)) {
                type = 'd';
            } else if (isa_float(arg)) {
                io.flags = F_LONG;
                type = 'g';                     /* ACCFLOAT_FMT */
            } else {
                type = 's';                     /* default/unknown */
            }
        }

        switch (type) {
        case 's':               /* string */
            sval = "<bad-string>";              /* undefined */

            if (isa_string(arg)) {
                if (NULL == (sval = get_str(arg))) {
                    sval = "<NULL>";
                }
            } else if (isa_list(arg)) {         /* 04/04/10 */
                prtl(iop, get_list(arg), 0);
                sval = NULL;
            } else if (isa_integer(arg)) {
                outl(iop, 10, get_accint(arg));
                sval = NULL;
            } else if (isa_float(arg)) {
                outl(iop, 10, (accint_t)get_accfloat(arg));
                sval = NULL;
            } else if (isa_null(arg)) {         /* 04/04/10 */
                sval = "<NULL>";
            }
            if (sval) outs(iop, sval);
            ++arg;
            break;

        case 'c':  {            /* character */
                int cvalue = get_xcharacter(arg++);

                OCHECK(iop, 1);
                OPUTC(iop, (cvalue > 0 ? cvalue : ' '));
            }
            break;

        case 'X':               /* hexadecimal */
            io.flags |= F_CAPS;
            /*FALLTHRU*/
        case 'x':
            outl(iop, 16, get_xaccint(arg++, 0));
            break;

        case 'd':               /* decimal */
        case 'i':
        case 'u':
        case 'j':               /* c99, intmax_t */
            outl(iop, 10, get_xaccint(arg++, 0));
            break;

        case 'o':               /* octal */
            outl(iop, 8, get_xaccint(arg++, 0));
            break;

        case 'b':               /* binary */
            outl(iop, 2, get_xaccint(arg++, 0));
            break;

        case 'B':               /* binary special */
            /*
             *  Print integer expression as a decoded binary, using the bit values as
             *  described within the secondary string parameter of the form;
             *
             *      <base><arg>
             *
             *  Where <base> is the output base expressed as a control character, e.g.
             *  \10 gives octal; \20 gives hex. Each arg is a sequence of characters,
             *  the first of which gives the bit number to be inspected (origin 1), and
             *  the next characters (up to a control character, i.e. a character <= 32),
             *  give the name of the register. Thus:
             *
             *  Example:
             *      "val=%B", 3, "\10\2BITTWO\1BITONE"
             *
             *  Would produce output:
             *      val=3<BITTWO,BITONE>
             */
            lval = get_xaccint(arg++, 0);
            if (NULL == (sval = get_xstr(arg++))) {
                sval = "<bad-string>";
            }
            outb(iop, (accuint_t) lval, sval);
            break;

        case 'f': case 'F':     /* float, all forms */
        case 'e': case 'E':
        case 'g': case 'G': {
                char format[2];

                format[0] = type;
                format[1] = 0;
                outf(iop, format, get_xaccfloat(arg++, 0.0));
            }
            break;

        case 0:                 /* NUL */
            goto end_format;

        case 'n':               /* character count */
            /*
             *  Number of characters successfully written so far to the
             *  stream or buffer; this value is stored in the integer whose
             *  "name" is given as the argument.
             */
            if (NULL != (sval = get_xstr(arg++))) {
                SYMBOL *sp;

                if (NULL == (sp = sym_lookup(sval))) {
                    ewprintf("msg: undefined symbol: %s", sval);

                } else if (F_INT == sp->s_type) {
                    sym_assign_int(sp, (accint_t) OLENGTH(iop));
                }
            } else {
                ewprintf("msg: argv[%d] expected to be a symbol name", arg-1);
            }
            break;

        case 'p':
            /*
             *  Percentage style message. If a second hasn't passed since the
             *  last message then ignore this entry.
             */
            if (! second_passed()) {
                return NULL;
            }
            break;

        default:
            OCHECK(iop, 1);
            OPUTC(iop, fmt[-1]);
            break;
        }
    }

end_format:;
    if (len) *len = OLENGTH(iop);
    OPUTC(iop, '\0');
    return buf_ptr;
}


static int
get_value(const char **fmt, int *arg)
{
    int number;

    if (**fmt == '*') {                         /* indirect */
        if ((number = get_xinteger(*arg, -1)) < 0) {
            number = 0;
        }
        (*arg)++; (*fmt)++;

    } else {                                    /* direct */
        int sign = 1;

        number = 0;
        if (**fmt == '-') {
            (*fmt)++;
            sign = -1;
        }

        if (isdigit(**fmt)) {
            do {
                number = (number * 10) + *(*fmt)++ - '0';
            } while (isdigit(**fmt));
            number *= sign;
        }
    }

    return number;
}


static void
out_check(io_t *io, int n)
{
    int offset = io->obp - buf_ptr;

    if (offset + n >= (buf_size - 4) /*spare assumed*/) {
        int obuf_size = buf_size;

        do {
            if (PRINTF_INCR < 0) {
                buf_size *= -PRINTF_INCR;
            } else {
                buf_size += PRINTF_INCR;
            }
        } while (obuf_size+n > buf_size);

        buf_ptr = (char *) chk_realloc(buf_ptr, buf_size);
        assert(buf_ptr);
        io->obp = buf_ptr + offset;
    }
}


/*  Function:           prtl
 *      List printer.
 *
 *  Parameters:
 *      io -                io stream.
 *      lp -                LIST pointer.
 *      level -             Current depth/level.
 *
 *  Returns:
 *      nothing
 */
static void
prtl(io_t *io, const LIST *lp, unsigned level)
{
    if (lp) {
        if (level) {
            outr(io, "{", 2);
        }

        if (level < 10) {
            /*
             *  export elements
             */
            const LIST *nextlp;
            unsigned count;

            for (count = 0; (nextlp = atom_next(lp)) != lp; lp = nextlp, ++count) {
                accint_t ival;
                accfloat_t fval;
                const char *sval;
                const LIST *lval;

                if (count) {
                    outr(io, ", ", 2);
                }

                if (atom_xint(lp, &ival)) {
                    outl(io, 10, ival);

                } else if (atom_xfloat(lp, &fval)) {
                    outf(io, ACCFLOAT_FMT, fval);

                } else if (NULL != (sval = atom_xstr(lp))) {
                    outr(io, sval, strlen(sval));

                } else if (NULL != (lval = atom_xlist(lp))) {
                    prtl(io, lval, level + 1);

                } else {
                    outr(io, "??", 2);
                }
            }
        } else {
            /*
             *  limit recursion to a level of 10.
             */
            outr(io, "...", 3);
        }

        if (level) {
            outr(io, "}", 2);
        }
    }
}


static void
outs(io_t *io, const char *s)
{
    int slen;

    /*
     *  The precision specifies the maximum number of characters to be
     *  printed. Characters in excess of precision are not printed.
     */
    slen = strlen(s);                           /* retrieve string length */
    if (io->precision > 0) {
        if (io->precision < slen) {
            slen = io->precision;               /* trim */
        }
    }
    outr(io, s, slen);
}


static void
outb(io_t *io, accuint_t ul, const char *p)
{
    char numbuf[64], *nbp;                      /* uitoa() buffer */
    int tmp, n;

    TRESET(io);

    if (io->flags & F_PREFIX) {
        TSTORE(io, 'B');
    }

    if ((n = *p++) == 0) {                      /* first character is base */
        return;
    }

    uitoa(ul, numbuf, n);                       /* convert long to string */

    if (io->precision > 0 || (io->flags & F_ZEROPAD)) {
        int padding;

        /*
         *  The precision specifies the minimum number of digits to be printed.
         *  If the number of digits in the argument is less than precision, the
         *  output value is padded on the left with zeros. The value is not
         *  truncated when the number of digits exceeds precision.
         *
         *  Padding specified by the precision overrides the the padding
         *  specified by the field width.
         *
         *    width:            minimum number of characters.
         *    precision:        minimum number of digits.
         */
        if (io->precision > 0) {
            padding = io->precision - strlen(numbuf);
        } else if (io->width > 0) {
            padding = io->width - strlen(numbuf);
        } else {
            padding = -1;
        }

        while (padding-- > 0) {
            TSTORE(io, '0');
        }
    }

    nbp = numbuf;                               /* output lval */
    while (*nbp != '\0') {
        TSTORE(io, *nbp);
        nbp++;
    }

    if (ul > 0) {
        for (tmp = 0; *p;) {
            n = *p++;
            if (ul & (1 << (n - 1))) {
                TSTORE(io,  tmp ? ',' : '<');
                for (; (n = *p) > ' '; ++p) {
                    TSTORE(io,  n);
                }
                tmp = 1;
            } else {
                for (; *p > ' '; ++p) {
                    continue;
                }
            }
        }

        if (tmp) {
            TSTORE(io, '>');
        }
    }
    outr(io, TBUFFER(io), TLENGTH(io));
}


static void
outl(io_t *io, int radix, accint_t lval)
{
    char numbuf[64], *nbp;                      /* uitoa() buffer */

    TRESET(io);

    if (radix < 2 || radix > 36) {
        radix = 10;
    }

    /*
     *  Output prefix...
     */
    if (radix == 10) {
        if ((io->flags & F_USIGN) == 0 && lval < 0) {
            TSTORE(io, '-');
            lval = -lval;

        /*
         *  If ' ' and '+' flags both appear, the ' ' flag will be ignored.
         */
        } else if (io->flags & F_PLUS) {
            TSTORE(io, '+');

        } else if (io->flags & F_BLANK) {
            TSTORE(io, ' ');
        }
    }

    if (io->flags & F_PREFIX) {
        switch (radix) {
        case 2:
            TSTORE(io, 'B');
        case 8:
            TSTORE(io, '0');
            break;
        case 16:
            TSTORE(io, '0');
            TSTORE(io, (io->flags & F_CAPS ? (char) 'X' : (char) 'x'));
            break;
        }
    }

    /*
     *  Output lval
     */
    uitoa((accuint_t) lval, numbuf, radix);     /* convert long to string */
    if (io->flags & F_CAPS) {
        str_upper(numbuf);
    }

    /*
     *  Dump result
     */
    if (io->flags & F_LEFTJUST) {               /* '-', ignore '0' (ANSI) */
        io->flags &= ~F_ZEROPAD;
    }

    if (io->precision > 0 || (io->flags & F_ZEROPAD)) {
        int padding;

        /*
         *  The precision specifies the minimum number of digits to be printed.
         *  If the number of digits in the argument is less than precision, the
         *  output value is padded on the left with zeros. The value is not
         *  truncated when the number of digits exceeds precision.
         *
         *  Padding specified by the precision overrides the the padding
         *  specified by the field width.
         *
         *      width:          minimum number of characters.
         *      precision:      minimum number of digits.
         */
        if (io->precision > 0) {
            padding = io->precision - (int)strlen(numbuf);
        } else if (io->width > 0) {
            padding = io->width - (int)strlen(numbuf);
        } else {
            padding = -1;
        }

        while (padding-- > 0) {
            TSTORE(io, '0');
        }
    }

    nbp = numbuf;                               /* output lval */
    while (*nbp != '\0') {
        TSTORE(io, *nbp);
        nbp++;
    }

    outr(io, TBUFFER(io), TLENGTH(io));
}


static void
outf(io_t *io, const char *format, accfloat_t fval)
{
    char fmtbuf[64], *fp = fmtbuf;              /* sprint() format buffer */
    int len;

    /*
     *  Build format
     */
    *fp++ = '%';

    if (io->flags & F_ZEROPAD)  *fp++ = '0';
    if (io->flags & F_LEFTJUST) *fp++ = '-';
    if (io->flags & F_PLUS)     *fp++ = '+';
    if (io->flags & F_BLANK)    *fp++ = ' ';
    if (io->flags & F_PREFIX)   *fp++ = '#';

    if (io->width > 0) {
        fp += sprintf(fp, "%d", io->width);
        io->width = 0;
    }

    if (io->precision > 0) {
        fp += sprintf(fp, ".%d", io->precision);
    } else if (io->precision == 0 || (io->flags & F_NOPRECISION)) {
        fp += sprintf(fp, ".%d", 0);
    } else {
        fp += sprintf(fp, ".%d", 6);            /* .. default 6 digits */
    }

    if (F_LONG & io->flags) {
        *fp++ = 'l';
    }
    while (*format) {
        *fp++ = *format++;
    }
    *fp = '\0';

    /*
     *  Output fval
     */
    len = sprintf(TBUFFER(io), fmtbuf, fval);   /* convert float to string */

    /*
     *  Dump result
     */
    outr(io, TBUFFER(io), len);
}


static void
outr(io_t *io, const char *s, int len)
{
    int padding = 0;

    /* 
     *  The width argument is a nonnegative decimal integer controlling the minimum
     *  number of characters printed. If the number of characters in the output value is
     *  less than the specified width, blanks are added to the left or the right of the
     *  values depending on whether the flag (for left alignment) is specified until the
     *  minimum width is reached. If width is prefixed with 0, zeros are added until the
     *  minimum width is reached.
     *
     *  The width specification never causes a value to be truncated. If the number of
     *  characters in the output value is greater than the specified width, or if width
     *  is not given, all characters of the value are printed (subject to the precision
     *  specification).
     */
    if (io->width > 0) {
        padding = io->width - len;              /* width specifies output columns */
    }

    OCHECK(io, padding + len);
    if ((io->flags & F_LEFTJUST) == 0)
        while (padding-- > 0) {
            OPUTC(io, ' ');
        }

    OPUTS(io, s, len);
    if (io->flags & F_LEFTJUST)
        while (padding-- > 0) {
            OPUTC(io, ' ');
        }
}


static void
uitoa(accuint_t value, char *buffer, int base)
{
    static const char hexdigits[] = "0123456789abcdef";
    unsigned char val;
    char *s = buffer;

    do {
        val = (unsigned char)(value % (accuint_t)base);
        *s++ = hexdigits[val];
        value /= (accuint_t)base;               /* prepare for next calculation */
    } while (value > 0);
    *s = '\0';                                  /* NUL terminate the string */
    str_rev(buffer);
}
/*end*/