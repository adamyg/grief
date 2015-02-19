#include <edidentifier.h>
__CIDENT_RCSID(gr_arg_c,"$Id: arg.c,v 1.19 2014/10/22 02:32:52 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: arg.c,v 1.19 2014/10/22 02:32:52 ayoung Exp $
 * Command line argument processing functionality.
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

#include "config.h"

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include <chkalloc.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "arg.h"

#define BADCH               (int) '?'
#define EMSG                ""

#define E_EOL               -1
#define E_UNKNOWN           -2
#define E_AMBIGUOUS         -3
#define E_ARGREQUIRED       -4
#define E_ARGUNEXPECTED     -5

#define OPTDELIMIT          ','


static int                  getoption(struct argparms *, int nargc, const char *const *nargv,
                                    const char *ostr, const struct argoption *lopt, int long_only);

static const struct argoption * getlongoption(struct argparms *p,
                                    int nargc, const char *const *nargv, const struct argoption *lopt);

static int                  print_option(const struct argoption *option, char *buf);

static void                 outerr(struct argparms *p, const char *msg, ... );


/*  Synopsis:
        int arg_init(struct argparms *p, int nargc, char *const *nargv, const char *ostr)

        int arg_initl(struct argparms *p, int nargc, char *const *nargv, const char *ostr,
                const struct argoption *lopt, int longonly)

        int arg_getopt(struct argparms *p)

        void arg_close(struct argparms *p)

    Purpose:
        The arg_getopt function returns the next option letter in
        'argv' that matches a letter in 'ostr'. It supports all the
        rules of the command syntax standard, see below. This
        function is the functional replacement of the standard UNIX
        getopt() function.

        The arg_getoptl functions extends the command syntax
        including support for long options.

        arg_init initialises the command parser status and must be
        called prior to calling either arg_getopt ior arg_getoptl.

        ostr - must contain the option letters the command using
            arg_getopt() or arg_getoptl() will recognize; if a letter
            is followed by a colon, the option is expected to have an
            argument, or group of arguments, which may be separated
            from it by white space. 'argval' is set to point to the
            start of the option argument on return.

        argind - represents the 'argv' index of the next argument to be
            processed. 'argind' is external and is initialised to 1
            before the first call. When all options have been
            processed (that is, up to the first non-option argument),
            EOF is returned. The special option "--" (two hyphens)
            may be used to delimit the end of the options; when it is
            encountered, EOF is returned and "--"' is skipped. This
            is useful in delimiting non-option arguments that begin
            with `-' (hyphen).

        The additional parameters to arg_initl() enable long argument
        handling.

        lopt - defines the argument names

        longonly - specifies whether long options are only matched when
            preceded with "--" or or long and short arguments can be
            mixed.

        arglidx - represents the 'lopt' index of the matched long
            argument, initialised to -1 upon each call.

    Parameters:
        nargc -         Number of command line arguments.

        nargv -         NUL terinated vector of arguments.

        ostr -          Argument syntax specification.

        lopt -          Specifies the pointer to the long argument list

                          struct argoption {
                              const char *name; // Option name
                              int has_arg;      // 0=none, 1=required, 2=optional
                              int *flag;        // Value buffer
                              int val;          // value return on detection
                          };

                        The 'name' fields contains the address of the
                        NUL terminated string command label.
                        'has_arg' specifies whether an associated
                        argument isn't excepted (0), is required (1),
                        or is optional (2).

                        The following manifest constants are defined
                        for use within the 'has_arg' field:

                          arg_none              No argument expected.
                          arg_required          An argument must be supplied.
                          arg_optional          An optional argument.

                        'flag' is the optional address of the buffer
                        assigned with the value of argument. 'val'
                        states the value to be returned upon the
                        given option being encountered.

        longonly -      When specified as non-zero, long arguments are only
                            matched when preceded with "--".

    Command Syntax Standard:
        The following command syntax should be obeyed. The arg_getopt
        function supports Rules 3-10 below. Rules 10b-10c are only
        active during long argument processing. The enforcement of
        the other rules must be done by the command itself.

            1.   Command names (name above) must be between two and
                 nine characters long.

            2.   Command names must include only lower-case letters
                 and digits.

            3.   Option names (option above) must be one character
                 long.

            4.   All options must be preceded by "-".

            5.   Options with no arguments may be grouped after a
                 single "-".

            6.   The first option-argument (argval above) following an
                 option must be preceded by a tab or space character.

            7.   Option-arguments cannot be optional.

            8.   Groups of option-arguments following an option must
                 either be separated by commas or separated by tab or
                 space character and quoted (-o xxx, z, yy or
                   -o "xxx z yy").

            9.   All options must precede operands (cmdarg above) on the
                 command line.

            10. "--" may be used to indicate the end of the options.

            10b. "--" may precede a long option.

            10c. If a command element has the form "-f", where f is a
                 valid short option, don't consider it an abbreviated
                 form of a long option that starts with f. Otherwise
                 there would be no way to give the -f short option.

                 On the other hand, if there's a long option "fubar"
                 and the command element is "-fu", do consider that
                 an abbreviation of the long option, just like "--fu",
                 and not "-f" with arg "u".

            11.  The order of the options relative to one another
                 should not matter.

            12.  The relative order of the operands (cmdarg above)
                 may affect their significance in ways determined by
                 the command with which they appear.

            13.  "-" preceded and followed by a space character should
                 only be used to mean standard input.

    Returns:  int -

        The arg_getopt functions return the character matched,
        otherwise EOF at the end of input stream, or '?' upon a parer
        error.

        The parser prints an error message on the standard error and
        returns a "?" (question mark) when it encounters an option
        letter not included in 'ostr' or 'lopt' or no argument is
        located after an option that expects one. Error messages may
        be disabled by setting 'argerr' to 0. The value of the
        character that caused the error is in 'argopt'.

        In addition arg_getoptl() generates messages when ambiguous
        arguments between the short and long selections are presented.

*/
void
arg_init(struct argparms *p, int nargc, const char *const *nargv, const char *ostr)
{
    arg_initl(p, nargc, nargv, ostr, NULL, 0);
}


void
arg_initl(struct argparms *p, int nargc, const char *const *nargv,
    const char *ostr, const struct argoption *lopt, int longonly)
{
    p->err = 1;                                 /* if error message should be printed */
    p->ind = 1;                                 /* index into parent argv vector */
    p->lidx = -1;                               /* long index */
    p->val = NULL;                              /* argument associated with option */

    p->_prog = NULL;
    p->_errout = NULL;
    p->_place = EMSG;
    p->_nargc = nargc;
    p->_nargv = nargv;

    if ((const char *)-1 == ostr) {
        /*
         *  Automaticly build ostr from longoptions return values ....
         */
        ostr = (const char *) NULL;
        if (lopt) {
            char *nstr;
            int i, cnt;

            for (i = cnt = 0; lopt[i].name; ++i) {
                ++cnt;                          /* count long argument list */
            }

            if (cnt && NULL != (nstr = chk_alloc((size_t)((cnt * 2) + 1)))) {
                for (i = cnt = 0; lopt[i].name; ++i) {
                    const int val = lopt[i].val;

                    if (val > 0 && val <= 0xff && isprint(val)) {
                        nstr[cnt++] = (char)val;
                        if (lopt[i].has_arg == arg_optional) {
                            nstr[cnt++] = ';';  /* optional argument */
                        } else if (lopt[i].has_arg) {
                            nstr[cnt++] = ':';  /* requires an argument */
                        }
                    }
                }

                if (0 == cnt) {
                    chk_free(nstr);
                } else {
                    nstr[cnt] = '\0';
                    ostr = nstr;
                }
            }
        }
    } else if (ostr) {
        ostr = chk_salloc(ostr);
    }
    p->_ostr = ostr;
    p->_lopt = lopt;
    p->_longonly = longonly;
}


int
arg_getopt(struct argparms *p)
{
    return getoption(p, p->_nargc, p->_nargv, p->_ostr, p->_lopt, p->_longonly);
}


void
arg_close(struct argparms *p)
{
    p->_nargc = 0;
    p->_nargv = NULL;
    p->_place = NULL;
    if (p->_ostr) {
        chk_free((void *) p->_ostr);
        p->_ostr = NULL;
    }
    p->_lopt = NULL;
}


int
arg_print(int indent, const struct argoption *p)
{
#define ARG_PRINT_WIDTH     30
#define ARG_PRINT_MARGIN    2

    char opttxt[ 132 ];
    int width, i;

    /* calc width */
    width = 0;
    for (i = 0; p[i].name; ++i) {
        const int len = print_option(p + i, opttxt);
        if (len > width) {
            width = len;
        }
    }

    /* 2 pass - print */
    width += ARG_PRINT_MARGIN;
    if (width > ARG_PRINT_WIDTH) {
        width = ARG_PRINT_WIDTH;
    }

    for (i = 0; p[i].name; ++i) {
        int len = print_option(p + i, opttxt);

        if (len >= (ARG_PRINT_WIDTH - ARG_PRINT_MARGIN)) {
            fprintf(stderr, "%*s%s\n", indent, "", opttxt);
            fprintf(stderr, "%*s", indent + width, "");
        } else {
            fprintf(stderr, "%*s%s%*s", indent, "", opttxt, width - len, "");
        }

        /* description */
        if (p[i].d1) {
            const char *nl, *s = p[i].d1;
            int slen = (int)strlen(s);

            while (*s) {
                if (NULL != (nl = strchr(s, '\n'))) {
                    len = (nl - s) + 1;
                } else {
                    len = slen;
                }
                if (s != p[i].d1) {
                    fprintf(stderr, "%*s", indent + width, "");
                }
                fprintf(stderr, "%.*s", len, s);
                slen -= len;
                s += len;
            }
            fprintf(stderr, ".");
        }
        fprintf(stderr, "\n");
    }

    return width;
}


static int
print_option(const struct argoption *p, char *opttxt)
{
    int len = 0;

    if (p->val > 0 && p->val < 0x7f && isprint(p->val)) {
        len += sprintf(opttxt + len, "-%c", p->val);
    }

    if (p->name && p->name[0]) {
        len += sprintf(opttxt + len, "%s--%s", (len ? ", " : ""), p->name);
    }

    if (p->has_arg) {
        len += sprintf(opttxt + len,
                (p->has_arg == arg_optional ? " [%s]" : " %s"),
                (p->d2 ? p->d2 : "value"));
    }

    opttxt[len] = '\0';
    return len;
}


/*
 *  getoption ---
 *      Return the next option long or shor.
 *
 *  Returns:
 *      On success the associated option value otherwise ':' is an option was
 *      required yet was omitter or '?' on an unknown/ambiguous option.
 */
static int
getoption(struct argparms *p, int nargc, const char *const *nargv,
    const char *ostr, const struct argoption *lopt, int longonly)
{
    register const char *oli = NULL;            /* option letter list index */

    if (NULL == p->_prog) {
        if (*nargv) {
            if (NULL == (p->_prog = strrchr(*nargv, '/')) &&
                    NULL == (p->_prog = strrchr(*nargv, '\\'))) {
                p->_prog = *nargv;
            } else {
                ++p->_prog;
            }
        } else {
            p->_prog = "";
        }
    }

    p->lidx = -1;
    p->ret = 0;
    p->val = NULL;

    /*
     *  Update scanning pointer to start of next argument,
     *  being possible long-argument
     */
    if (NULL == p->_place || !*p->_place) {
        if (p->ind >= nargc || *(p->_place = nargv[p->ind]) != '-') {
            p->_place = EMSG;
            return (p->ret = EOF);
        }

        if (p->_place[1] && *++p->_place == '-') {
            if (! *++p->_place) {               /* found "--" , meaning premature end of options */
                ++p->ind;
                p->_place = EMSG;
                return (p->ret = EOF);
            }
        }

        /*  Reference GCC 'getopt':
         *      If ARGV-element has the form "-f", where f is a valid short option, don't
         *      consider it an abbreviated form of a long option that starts with f. Otherwise
         *      there would be no way to give the -f short option.
         *
         *      On the other hand, if there's a long option "fubar" and the ARGV-element is
         *      "-fu", do consider that an abbreviation of the long option, just like
         *      "--fu", and not "-f" with arg "u".
         *
         *      This distinction seems to be the most useful approach.
         *
         *  Upon no long-argumen match
         *      If this is not 'longonly', or the option starts with '--' and is not a valid
         *      short option, then it's an error. otherwise interpret it as a short option.
         */
        if (lopt) {
            if ('-' == nargv[p->ind][1] ||
                   (! longonly &&
                      (nargv[p->ind][2] || NULL == strchr(ostr ? ostr : "", nargv[p->ind][1])))) {

                const struct argoption *arg =   /* parse long arguments */
                        getlongoption(p, nargc, nargv, lopt);

                if (arg) {
                    if (0 == p->ret) {
                        if (arg->flag) {
                            *arg->flag = arg->val;
                            return 0;
                        }
                        return (p->ret = arg->val);
                    }
                    return (E_ARGREQUIRED == p->ret ? ':' : BADCH);
                }

                if ((! longonly || '-' == nargv[p->ind][1]) &&
                            NULL == strchr(ostr ? ostr : "", *p->_place)) {
                    if ('-' == nargv[p->ind][1]) {
                        outerr(p, "unrecognised option `-%s'", p->_place);
                    } else {
                        outerr(p, "unrecognised option `%c%s'", nargv[p->ind][0], p->_place);
                    }
                    p->_place = EMSG;
                    p->ret = E_UNKNOWN;
                    ++p->ind;
                    return BADCH;
                }
            }
        }
    }

    /*
     *  short argument
     */
    if (':' == (p->opt = (int) *p->_place++) ||
            (ostr && NULL == (oli = strchr(ostr, p->opt)))) {
        /*
         *  If the user didn't specify '-' as an option,
         *      assume it means EOF(-1) as POSIX specifies.
         */
        if ('-' == p->opt) {
            return (p->ret = EOF);              /* option letter unknown or ':' */
        }
        if (! *p->_place) {
            ++p->ind;
        }
        outerr(p, "illegal option -- %c", p->opt);
        p->ret = E_UNKNOWN;
        return BADCH;
    }

    if (oli[1] != ':' && oli[1] != ';') {       /* no argument */
        if (! *p->_place) {
            ++p->ind;
        }
        p->val = NULL;

    } else {                                    /* need an argument */
        if (*p->_place) {                       /* no white space */
            p->val = p->_place;

        } else if (nargc <= ++p->ind || 0 == strcmp(nargv[p->ind], "--")) {
            if (oli[1] == ';') {
                p->val = NULL;                  /* optional */

            } else {
                outerr(p, "option requires an argument -- %c", p->opt);
                p->ret = E_ARGREQUIRED;
                p->_place = EMSG;
                return BADCH;
            }
        } else {
            p->val = (const char *)(nargv[p->ind]);
        }

        p->_place = EMSG;
        ++p->ind;
    }
    return (p->ret = p->opt);                   /* dump back option letter */
}


/*
 *  getlongoption ---
 *      Match against the long options
 */
static const struct argoption *
getlongoption(struct argparms *p, int nargc, const char *const *nargv, const struct argoption *lopt)
{
    const struct argoption *match = NULL;
    const char *eq = NULL, *place = p->_place;
    int matches = 0, i;
    size_t len;

    if (NULL != (eq = strchr(place, '='))) {
        len = eq - place;                       /* with argument */
        ++eq;
    } else {
        len = strlen(place);
    }

    for (i = 0; lopt[i].name; ++i) {
        const struct argoption *arg = lopt + i;

        if (strncmp(place, arg->name, len)) {
            continue;                           /* no match */
        }

        if (len == strlen(arg->name)) {
            matches = 1;
            match = arg;
            p->lidx = i;
            break;
        }

        if (0 == matches++) {
            match = arg;
            p->lidx = i;
        }
        match = arg;
    }

    if (0 == matches) {
        p->lidx = -1;
        return NULL;                            /* no match */
    }

    p->ret = 0;
    p->val = NULL;
    p->opt = *place;                            /* first character of match */
    ++p->ind;

    if (matches > 1) {
        outerr(p, "option `--%s' is ambiguous", place);
        p->ret = E_AMBIGUOUS;

    } else if (match->has_arg == arg_none && eq) {
        outerr(p, "option `--%s' doesn't allow an argument", match->name);
        p->ret = E_ARGUNEXPECTED;

    } else if (match->has_arg == arg_required || match->has_arg == arg_optional) {
        if (eq) {
            p->val = (const char *)eq;          /* argument */

        } else if (match->has_arg == arg_required) {
            /*
             *  Warn regarding possible "missing arguments", for example
             *
             *      -f --nextarg
             */
            if (p->ind >= nargc || 0 == strcmp(nargv[p->ind], "--")) {
                outerr(p, "option `--%s' requires an argument", match->name);
                p->ret = E_ARGREQUIRED;

            } else {
                p->val = (const char *)(nargv[p->ind++]);
            }
        }
    }
    p->_place = NULL;
    return match;
}


/*
 *  outerr ---
 *      Error output implementation
 */
static void
outerr(struct argparms *p, const char *msg, ...)
{
    if (p->err) {
        va_list ap;

        if (p->_errout) {
            char buffer[512];
            int len = 0;

            if (p->_prog && p->_prog[0]) {
                len = sxprintf(buffer, sizeof(buffer), "%s: ", p->_prog);
            }
            va_start(ap, msg);
            vsxprintf(buffer + len, (int)(sizeof(buffer) - len), msg, ap);
            (p->_errout)(p, buffer);

        } else {
            if (p->_prog && p->_prog[0]) {
                fprintf(stderr, "%s: ", p->_prog);
            }
            va_start(ap, msg);
            vfprintf(stderr, msg, ap);
            fputc('\n', stderr);
        }
        va_end(ap);
    }
}


/*  Function:           arg_split
 *
 *      The 'arg_split' function splits 'cmd' into list of argument pointers. The list of
 *      pointers are the start address of each argument encountered within the argument
 *      buffer 'cmd', terminated with a NULL entry. Each argument is terminated with a '\0'
 *      by the replacement of the separators within 'cmd' with '\0' characters.
 *
 *      As it parses the command line, 'arg_split' looks for command separators, white
 *      space (spaces and tabs). Normally, these special characters cannot be passed to a
 *      command as part of an argument. However, you can include any of the special
 *      characters in an argument by enclosing the entire argument in double quotes ["]. The
 *      ["] themselves will not be passed to the command as part of the argument. Within a
 *      double quoted string the ["] character and [\] character can be represented as [\"]
 *      and [\\].
 *
 *  Parameters:
 *      cmd -               Specifies the argument buffer.
 *      argv -              NULL terminated list of argument pointers,
 *      cnt -               Maximum argument count, excluding NULL teminator.
 *
 *  Returns:
 *      int - argument count.
 */
int
arg_split(char *cmd, const char **argv, int cnt)
{
    char *start, *end;
    int argc;

    if (cmd == NULL || argv == NULL || cnt <= 0) {
        return -1;
    }

    argc = 0;
    for (;;) {
        /* Skip over blanks */
        while (*cmd == ' '|| *cmd == '\t' || *cmd == '\n') {
            ++cmd;                              /* white-space */
        }

        if (! *cmd)
            break;

        if ('\"' == *cmd || '\'' == *cmd) {     /* quoted argument */
            char quote = *cmd++;

            start = end = cmd;
            for (;;) {
                char ch = *cmd;

                if (0 == ch || '\n' == ch || ch == quote)
                    break;

                if ('\\' == ch) {
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }
                *end++ = *cmd++;
            }

        } else {
            start = end = cmd;
            for (;;) {
                char ch = *cmd;

                if (0 == ch || ch == '\n' || ' ' == ch || '\t' == ch)
                    break;

                if ('\\' == ch)  {
                    if (cmd[1] == '\"' || cmd[1] == '\'' || cmd[1] == '\\') {
                        ++cmd;
                    }
                }
                *end++ = *cmd++;
            }
        }

        /* Reallocate argument list index */
        if (cnt > 0) {
            argv[ argc++ ] = start;
            if (*cmd == '\0')
                break;
            *end = '\0';
            ++cmd;
            --cnt;
        }
    }
    argv[argc] = NULL;
    return argc;
}


/*  Function:           arg_subopt
 *
 *      The 'arg_subopt' function parses suboptions in a flag argument that was initially
 *      parsed by getopt(3C). The suboptions are separated by commas and may consist of
 *      either a single token or a token-value pair separated by an equal sign. Since commas
 *      delimit suboptions in the option string, they are not allowed to be part of the
 *      suboption or the value of a suboption; if present in the option input string, they
 *      are changed to null characters. White spaces within tokens or token-value pairs must
 *      be protected from the shell by quotes.
 *
 *      The syntax described above is used in the following example by the mount(1M),
 *      utility, which allows the user to specify mount parameters with the -o option as
 *      follows:
 *
 *             mount -o rw,hard,bg,wsize=1024 speed:/usr /usr
 *
 *      In this example there are four suboptions: rw, hard, bg, and wsize, the last of
 *      which has an associated value of 1024.
 *
 *      The 'arg_subopt' function takes the address of a pointer to the option string, a
 *      vector of possible tokens, and the address of a value string pointer. It returns the
 *      index of the token that matched the suboption in the input string, or -1 if there
 *      was no match. If the option string pointed to buy 'optionp' contains only one
 *      subobtion, getsubopt() updates 'optionp' to point to the null character at the end
 *      of the string; otherwise it isolates the suboption by replacing the comma separator
 *      with a null character, and updates 'optionp' to point to the start of the next
 *      suboption. If the suboption has an associated value, getsubopt() updates 'valuep' to
 *      point to the value's first character. Otherwise it sets 'valuep' to 'NULL'.
 *
 *      The token vector is organized as a series of pointers to null strings. The end of
 *      the token vector is identified by a null pointer.
 *
 *      When getsubopt() returns, a non-null value for 'valuep' indicates that the suboption
 *      that was processed included a value. The calling program may use this information to
 *      determine if the presence or absence of a value for this subobtion is an error.
 *
 *      When getsubopt() fails to match the suboption with the tokens in the 'tokens' array,
 *      the calling program should decide if this is an error, or if the unrecognised option
 *      should be passed to another program.
 *
 *  Returns:
 *      The 'arg_subopt' function returns -1 when the token it is scanning is not in the
 *      token vector. The variable addressed by 'valuep' contains a pointer to the first
 *      character of the token that was not recognized, rather than a pointer to a value for
 *      that token.
 *
 *      The variable addressed by 'optionp' points to the next option to be parsed, or a
 *      null character if there are no more options.
 */
int
arg_subopt(char **optionp, const char * const *tokens, char **valuep)
{
    register int cnt;
    register char *p;
    char *subval;

    subval = *valuep = NULL;

    if (! optionp || ! *optionp) {
        return(-1);
    }

    /* skip leading white-space, commas */
    for (p = *optionp; *p && (*p == OPTDELIMIT || *p == ' ' || *p == '\t'); ++p) {
        /*continue*/;
    }

    if (! *p) {
        *optionp = p;
        return(-1);
    }

    /* save the start of the token, and skip the rest of the token. */
    for (subval = p; *++p && *p != OPTDELIMIT && *p != '=' && *p != ' ' && *p != '\t';) {
        /*continue*/;
    }

    if (*p) {
        /*
         *  If there's an equals sign, set the value pointer, and skip over the value part
         *  of the token.  Terminate the token.
         */
        if (*p == '=') {
            *p = '\0';
            for (*valuep = ++p; *p && *p != OPTDELIMIT && *p != ' ' && *p != '\t'; ++p)
                /*continue*/;
            if (*p) {
                *p++ = '\0';
            }
        } else {
            *p++ = '\0';
        }

        /* Skip any whitespace or OPTDELIMITOR after this token. */
        for (; *p && (*p == OPTDELIMIT || *p == ' ' || *p == '\t'); ++p) {
            /*continue*/;
        }
    }

    /* set optionp for next round */
    *optionp = p;

    /* match */
    for (cnt = 0; *tokens; ++tokens, ++cnt)
        if (! strcmp(subval, *tokens)) {
            return (cnt);
        }
    *valuep = subval;
    return (-1);
}
/*end*/
