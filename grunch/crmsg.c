#include <edidentifier.h>
__CIDENT_RCSID(gr_crmsg_c,"$Id: crmsg.c,v 1.11 2020/04/23 12:35:50 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: crmsg.c,v 1.11 2020/04/23 12:35:50 cvsuser Exp $
 * Compiler messages.
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

#include "grunch.h"                             /* local definitions */

static unsigned         x_onceonly;             /* Error/warning one-shots */
static unsigned         x_warnings;             /* Number of warnings */
static unsigned         x_errors;               /* Number of fatal errors */

static char             x_funcprint[256];       /* Last function name printed in warning or error */

static unsigned char    x_msgflags[ (RC_MAX >> 3)+1 ];


/*  Function:           yymsginit
 *      Runtime initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
yymsginit(void)
{
    x_onceonly = 0;
    x_warnings = 0;
    x_errors = 0;
    x_funcprint[0] = 0;
    memset(x_msgflags, 0, sizeof(x_msgflags));
}


/*  Function:           yyerrors
 *       Retrieve the currnt error count.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Error count.
 */
unsigned
yyerrors(void)
{
    return x_errors;
}


/*  Function:           yywarnings
 *      Retrieve the currnt warning count.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Warning count
 */
unsigned
yywarnings(void)
{
    return x_warnings;
}


/*  Function:           yyerror
 *      Parser (bison) error reported.
 *
 *  Parameters:
 *      str -               Error description.
 *
 *  Returns:
 *      Warning count
 */
void
yyerror(const char *str)
{
    crerror_line(RC_SYNTAX_ERROR, x_lineno, str);
}


void
yyerrorf(const char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    crerrorv_line(RC_SYNTAX_ERROR, x_lineno, str, ap);
    va_end(ap);
}


void
yywarning(const char *str)
{
    crwarn_line(RC_SYNTAX_ERROR, x_lineno, str);
}


void
yywarningf(const char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    crwarnv_line(RC_SYNTAX_ERROR, x_lineno, str, ap);
    va_end(ap);
}



/*  Function:           xprintf
 *      Debug output.
 *
 *  Parameters:
 *      fmt -               Format specifiation (sprintf style)
 *      ...
 *
 *  Returns:
 *      nothing
 */
void
xprintf(const char *fmt, ...)
{
    static int br;                              /* break flag */
    va_list ap;

    if (xf_debug) {
        if (fmt && fmt[0]) {
            int level = x_decl_level + x_struct_level + x_block_level;

            if (br && level > 0) {
                fprintf(x_errfp, "%*s", level*4, "");
            }
            va_start(ap, fmt);
            vfprintf(x_errfp, fmt, ap);
            va_end(ap);
            br = (fmt[strlen(fmt)-1] == '\n' ? 1 : 0);
        } else {
            fflush(x_errfp);
        }
    }
}


int
cronceonly(cronceonly_t once)
{
    const unsigned t_once = (1 << (unsigned)once);

    if (x_onceonly & t_once)
        return 0;
    x_onceonly |= t_once;
    return 1;
}


void
crmessage(int msgno, int enable)
{
    if (msgno > 0 && msgno < RC_MAX) {
        unsigned char mask = 1 << ((unsigned char)msgno & 7);
        unsigned idx = (unsigned)msgno >> 3;

        if (enable) {
            x_msgflags[idx] &= ~mask;
        } else {
            x_msgflags[idx] |= mask;            /* disable */
        }
    }
}


/* Function:            reportignore
 *      Determine whether the given message has been disabled.
 */
static int
reportignore(int msgno)
{
    if (msgno > 0 && msgno < RC_MAX) {
        unsigned char mask = 1 << ((unsigned char)msgno & 7);
        unsigned idx = (unsigned)msgno >> 3;

        return (x_msgflags[idx] & mask);
    }
    return 0;
}


/* Function:            functionname
 *      Print the current function name if its changed since the last warning/error message.
 */
static void
functionname(void)
{
    if (x_funcname) {
        if (x_funcname[0]) {
            if (strncmp(x_funcname, x_funcprint, sizeof(x_funcprint)-1) != 0) {
                fprintf(x_errfp, "%s: In function %s():\n", x_filename, x_funcname);
                strncpy(x_funcprint, (const char *)x_funcname, sizeof(x_funcprint)-1);
                x_funcprint[sizeof(x_funcprint)-1] = 0;
            }
        }
    } else {
        if (x_funcprint[0] != 0x01) {
            fprintf(x_errfp, "%s: At top level:\n", x_filename);
            x_funcprint[0] = 0x01;
            x_funcprint[1] = 0;
        }
    }
}


void
crerror_line(int __CUNUSEDARGUMENT(msgno), int lineno, const char *str)
{
    char t_msgbuf[1024], *p = t_msgbuf;
    const char *t_msgend = t_msgbuf + (sizeof(t_msgbuf)-2);
    char c;

    while ((c = *str++) != 0 && p < t_msgend) {
        if ((c == 'O' || c == 'K') && str[0] == '_') {
            /*
             *  Replace parser tokens O_xxx and K_xxx with their true names.
             *
             *      XXX - may need to skip quoted text, for example 'O_DEFINITION'.
             */
            const char *symbol, *start = str-1;

            ++str;
            while ((c = *++str) == '_' || isalpha((unsigned char)c)) {
                continue;                       /* find word */
            }

            if (NULL != (symbol = yysymbol(start, str - start))) {
                if (symbol[1]) {                /* >=2 characters */
                    while (*symbol && p < t_msgend) {
                        *p++ = *symbol++;
                    }
                } else {                        /* single character (operator, quote etc) */
                    *p++ = '\'';
                    *p++ = *symbol;
                    *p++ = '\'';
                }
            } else {
                while (start < str && p < t_msgend) {
                    *p++ = *start++;
                }
            }
        } else {
            *p++ = c;
        }
    }
    *p = 0;

    if ('\r' == t_msgbuf[0]) {                  /* special, line join */
        fprintf(x_errfp, "%s\n", t_msgbuf+1);

    } else {
        const char syntax_error[] = "syntax error";

        functionname();

        if (0 == strncmp(t_msgbuf, syntax_error, sizeof(syntax_error)-1)) {
            fprintf(x_errfp, "%s(%d): %s\n", x_filename, lineno, t_msgbuf);

        } else {
            fprintf(x_errfp, "%s(%d): error: %s\n", x_filename, lineno, t_msgbuf);

        }
    }
    ++x_errors;
}


void
crerrorv_line(int msgno, int lineno, const char * fmt, va_list ap)
{
    char buf[1024];

    vsprintf(buf, fmt, ap);
    crerror_line(msgno, lineno, buf);
}


void
crerrorx_line(int msgno, int lineno, const char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    crerrorv_line(msgno, lineno, fmt, ap);
    va_end(ap);
}


void
crerror(int msgno, const char *str)
{
    crerror_line(msgno, x_lineno, str);
}


void
crerrorv(int msgno, const char * fmt, va_list ap)
{
    char buf[1024];

    vsprintf(buf, fmt, ap);
    crerror(msgno, buf);
}


void
crerrorx(int msgno, const char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    crerrorv(msgno, fmt, ap);
    va_end(ap);
}


void
crwarn_line(int __CUNUSEDARGUMENT(msgno), int lineno, const char *str)
{
    if (xf_warnings) {
        if ('\r' == str[0]) {                   /*special, line join*/
            fprintf(x_errfp, "%s\n", str + 1);
        } else {
            functionname();
            fprintf(x_errfp, "%s(%d): %s\n", x_filename, lineno, str);
        }
        ++x_warnings;
    }
}


void
crwarnv_line(int msgno, int lineno, const char *fmt, va_list ap)
{
    char buf[1024];

    vsprintf(buf, fmt, ap);
    crwarn_line(msgno, lineno, buf);
}


void
crwarnx_line(int msgno, int lineno, const char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    crwarnv_line(msgno, lineno, fmt, ap);
    va_end(ap);
}


void
crwarn(int msgno, const char * str)
{
    crwarn_line(msgno, x_lineno, str);
}


void
crwarnv(int msgno, const char *fmt, va_list ap)
{
    char buf[1024];

    vsprintf(buf, fmt, ap);
    crwarn(msgno, buf);
}


void
crwarnx(int msgno, const char * fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    crwarnv(msgno, fmt, ap);
    va_end(ap);
}

/*end*/
