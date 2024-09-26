#include <edidentifier.h>
__CIDENT_RCSID(gr_m_getopt_c,"$Id: m_getopt.c,v 1.26 2024/07/04 11:09:30 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_getopt.c,v 1.26 2024/07/04 11:09:30 cvsuser Exp $
 * Command line/argument option processing.
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

#ifndef ED_LEVEL
#define ED_LEVEL 1
#endif
#include <editor.h>
#include <chkalloc.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_getopt.h"                           /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* mac_... */
#include "arg.h"                                /* ... */
#include "debug.h"                              /* trace_...() */
#include "echo.h"
#include "eval.h"                               /* get_...() */
#include "lisp.h"                               /* atom_...() */
#include "symbol.h"                             /* argv_...() */

enum {
    OTYPE_MODIFIER_PLUS     = 0x1000,
    OTYPE_NONE              = 0,
    OTYPE_ANYTHING,
    OTYPE_INTEGER,
    OTYPE_NUMERIC,
    OTYPE_CHARACTER,
    OTYPE_STRING,
    OTYPE_FLOAT,
    OTYPE_DOUBLE,
    OTYPE_BOOLEAN
};

#define GETOPT_ARGC         64                  /* max argument number */

static void                 getopterr(struct argparms *p, const char *msg);
static struct argoption *   longdecode(const LIST *options, int *optcount);
static int                  optiondecode(const char *spec, int *value, struct argoption *p);
static int                  booleanvalue(const char *value);
static int                  optioncheck(int type, const char *value);
static int                  is_white(int ch);

static int                  x_getopt_argc;
static const char *         x_getopt_argv[GETOPT_ARGC+2];
static struct argparms      x_getopt_parms;
static struct argoption *   x_getopt_longoptions;
static char *               x_getopt_args;

static struct argoption *   x_getsubopt_options;
static char *               x_getsubopt_buffer;
static char *               x_getsubopt_cursor;
static int                  x_getsubopt_delimiter;
static const char *         x_getsubopt_quotes;


/*  Function:           do_getopt
 *      getopt primitives.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: getopt - Get options

        int
        getopt(string &value, [[string shortopts], list longopts,
            string|list args, [string caller]])

    Macro Description:
        The 'getopt()' primitive is a command-line parser similar to the
        system library function of the same name. The 'getopt()'
        function provides a superset of the functionality of getopt,
        accepting options in two forms, words (long-options) and
        characters (short-options).

        The 'getopt()' primitive is designed to be executed within a loop,
        with the first invocation suppling the available options
        'shortopts' and/or 'longopts', the arguments to be parsed 'args'
        and the application/macro name 'caller'. Subsequent calls within
        the same execution loop only then request the next available
        option without additional arguments; the general form of getopt
        usage is as follows.

>           string value;
>           int ch;
>
>           if ((ch = getopt(value, shortopts, longopts, args)) > 0) {
>               do {
>               } while ((ch = getopt(value)) > 0);

      Short Options::

        The short option string 'shortopts' may contain the following
        elements

            o individual characters
            o characters followed by a colon to indicate an option
                argument is to follow.

        For example, an option string "x" recognizes an option `-x',
        and an option string "x:" recognizes an option and argument
        `-x argument'.

      Long Options::

        The long option list 'longopts' may contain a list of strings
        each defining a long option, of the form:

>           "[tag][,[#]value][[:=][scinfd]]"

        tag - Option tag or name.

        value - Index value returned upon an option match. The value is
            either a character or a numeric denoted by a leading '#'.

            If omitted the index shall be taken as the next within
            the sequence either following the previous index or using
            the opening index of zero(0).

       operator - Defines whether the value is optional (=) or
            required (:). If omitted, it is assumed no value is
            required.

       type - Optional value type against which the value shall be
            validated, stated as either a single character or it
            equivalent full name.

                a[nything]     - Anything.
                s[tring]       - String.
                c[haracter]    - Character value.
                i[integer][+]  - Decimal integer value plus optional positive only modifier.
                n[numeric][+]  - Numeric (oct, dec and hex).
                f[loat]        - Floating point including integer.
                d[ouble]       - Double precision floating point including integer.
                b[oolean]      - Boolean, 0/1, y[e]s/n[o], on/off, true/false;
                                    result being a string containing '0' or '1'.

    Macro Returns:
        Option value or index, starting at zero, otherwise one of the
        following negative error codes. If the case of error codes -2
        or less, value shall contain an error condition descripting
        the condition.

            -1 -    End of the options (EOF).
            -2 -    Unknown option.
            -3 -    Ambiguous option.
            -4 -    Argument required.
            -5 -    Unexpected argument.

        Plus the following argument syntax error conditions.

            -10 -   Invalid value, for example "expected a numeric value".
            -99 -   Invalid option specification.

    Macro Portability:
        A Grief extension.

    Macro Example:

(code)
        list longoptions = {
                "help,h",       // note duplicates result in first match
                "verbose,v",
                "verbose2,#2",
                "integer"       // extended format
                };
        int ch;

        if ((ch = getopt(value, "hv", longoptions, get_parm(1))) > 0) {
            do {
                switch (ch) {
                case 'h':       // -h (short) or --help (long) option
                    break;

                case 'v':       // -v (short) or --verbose (long)
                    break;

                case 2:         // --verbose2
                    break;

                case 3:         // --integer=<value>
                    i = atoi(value);
                    break;

                case '?':       // error or unknown option
                case ':':       // or argument expected
                    if (length(value)) {
                        error("myfunction: %s", value);
                    }
                    break;

                default:
                    break;
                }
            } while ((ch = getopt(value)) > 0);
        }
(end)

    Macro See Also:
        arg_list, split_arguments
 */
void
do_getopt(void)                 /* (string &value, [[string shortoptions], list longoptions, string|list args,
                                        [string caller], [int maxargc], [int flags]]) */
{
    const char *shortoptions = get_xstr(2);
    const LIST *longoptions = get_xlist(3);
    int ret = -1;

    /*
     *  initialise options
     */
    if (shortoptions || longoptions) {
        int longcount = 0;

        getopt_shutdown();

        /*
         *  long options
         */
        if (longoptions) {
            if (NULL == (x_getopt_longoptions = longdecode(longoptions, &longcount))) {
                ED_TRACE(("getopt() : -99\n"))
                acc_assign_int(-99);
                return;
            }
        }

        /*
         *  options either short or long are available
         */
        if (shortoptions || longcount) {
            const LIST *largs = get_xlist(4);
            const char *caller = get_xstr(5);
         /* const int maxargv = get_xinteger(6, GETOPT_ARGC); - TODO */
         /* const int flags = get_xinteger(7, 0); - TODO */

            assert(0 == x_getopt_argc);

            if (NULL == caller && mac_sd) {     /* caller/program, argv[0] */
                const struct mac_stack *stk1 = mac_sp;
                caller = (stk1->caller ? stk1->caller : stk1->name);
            }
            x_getopt_argv[x_getopt_argc++] = (caller ? caller : "");

            if (largs) {                        /* decode arguments, ARG[1...x] */
                const LIST *nextlp, *lp;
                int slen = 0;

                for (lp = largs; (nextlp = atom_next(lp)) != lp; lp = nextlp) {
                    const char *svalue;

                    if (NULL != (svalue = atom_xstr(lp))) {
                        if (x_getopt_argc < GETOPT_ARGC) {
                            x_getopt_argv[x_getopt_argc++] = svalue;
                            slen += (int)(strlen(svalue) + 3);
                        }
                    }
                }
                x_getopt_argv[x_getopt_argc] = NULL;
                if (slen) {                     /* working storage */
                    x_getopt_args = chk_alloc(slen);
                }

            } else {
                const char *sargs = get_xstr(4);

                if (sargs && sargs[0] &&
                        NULL != (x_getopt_args = chk_salloc(sargs))) {
                    x_getopt_argc = arg_split(x_getopt_args, x_getopt_argv + 1, GETOPT_ARGC) + 1;
                } else {
                    /*
                     *  TODO - allow NULL which decodes arguments (aka arg_list())
                     *    if (isa_list(4)) {
                     *    } else if (isa_null(4)) {
                     *    }
                     */
                    x_getopt_argv[x_getopt_argc] = NULL;
                }
            }

            /* initialise */
            ED_TRACE2(("\tshort(%s), long(%d), ARGC=%d\n", (shortoptions ? shortoptions : ""), longcount, x_getopt_argc))
            for (longcount = 0; longcount < x_getopt_argc; ++longcount) {
                ED_TRACE2(("\t\tARGV[%d]=%s\n", longcount, x_getopt_argv[longcount]))
            }

            arg_initl(&x_getopt_parms, x_getopt_argc, x_getopt_argv, shortoptions, x_getopt_longoptions, FALSE);
            arg_errout(&x_getopt_parms, getopterr);
        }
    }

    /*
     *  next option/
     *      returns either the associated option value otheriwise ':' is an option was
     *      required yet was omitter or '?' on an unknown/ambiguous option.
     */
    if (x_getopt_argc) {
        const char *value;

        ret = arg_getopt(&x_getopt_parms);
        if (ret >= 0) {                         /* return option value or error */
            value = x_getopt_parms.val;
            if (x_getopt_parms.lidx >= 0) {
                const struct argoption *lopt = &x_getopt_longoptions[x_getopt_parms.lidx];
                const int type = lopt->udata;

                if (OTYPE_BOOLEAN == type) {
                    switch (booleanvalue(value)) {
                    case TRUE:  value = "1"; break;
                    case FALSE: value = "0"; break;
                    default:
                        ret = -10;
                    }
                } else {
                    if (! optioncheck(type, value)) {
                        ret = -10;
                    }
                }

                if (-10 == ret) {
                    char buffer[256];           /* MAGIC */

                    if (value) {
                        sxprintf(buffer, sizeof(buffer),
                            "option `--%s \"%s\"' invalid format", lopt->name, value);
                    } else {
                        sxprintf(buffer, sizeof(buffer),
                            "option `--%s' invalid format", lopt->name);
                    }
                    getopterr(&x_getopt_parms, buffer);
                }
            }

        } else {                                /* EOF return remaining arguments */
            if (x_getopt_args) {
                char *cursor = x_getopt_args;
                int count = 0, ind;

                x_getopt_args[0] = 0;

                for (ind = x_getopt_parms.ind; x_getopt_argv[ind]; ++ind) {
                    const char *s, *arg = x_getopt_argv[ind];
                    int len = (int)strlen(arg);

                    for (s = arg; *s; ++s) {
                        if (is_white(*s) || '"' == *s || '\\' == *s) {
                            break;
                        }
                    }

                    if (count++) {
                        *cursor++ = ' ';
                    }

                    if (*s) {                   /* quote white-space, plus single/double quotes */
                        *cursor++ = '"';
                        while (*arg) {
                            if ('\\' == *arg || '"' == *arg) {
                                *cursor++ = '\\';
                            }
                            *cursor++ = *arg++;
                        }
                        *cursor++ = '"';
                        *cursor = '\0';

                    } else {
                        strcpy(cursor, arg);
                        cursor += len;
                    }
                }
                value = x_getopt_args;

            } else  {
                value = "";
            }
        }

        ED_TRACE(("getopt() : %d (%s)\n", ret, value?value:"(null)"))
        argv_assign_str(1, value);
        acc_assign_int(ret);

        if (ret < 0) {
            getopt_shutdown();
        } else if ('?' == ret || ':' == ret) {  /* release error message storage */
            chk_free((void *)x_getopt_parms.val);
        }

    } else {
        ED_TRACE(("getopt() : -1\n"))
        acc_assign_int(-1);
    }
}


/*  Function:           getopt_shutdown
 *      Release all getopt temporary resources.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
getopt_shutdown(void)
{
    ED_TRACE2(("\tgetopt_shutdown\n"))
    chk_free(x_getopt_longoptions), x_getopt_longoptions = NULL;
    chk_free(x_getopt_args), x_getopt_args = NULL;
    memset((void *)x_getopt_argv, 0, sizeof(x_getopt_argv));
    x_getopt_argc = 0;
}


/*  Function:           getopterr
 *      Option error/diagnostic support.
 *
 *  Parameters:
 *      p - Options object.
 *
 *  Returns:
 *      nothing
 */
static void
getopterr(struct argparms *p, const char *msg)
{
    p->val = chk_salloc(msg);
}


/*  Function:           do_getsubopt
 *      getsubopt primitive; retrieve the next sub-option,
 *      follow normal getsubopt usage.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: getsubopt - Parse suboption arguments from a string.

        int
        getsubopt(string value, [list options],
                [string|list args], [string delim], [string quotes])

    Macro Description:

        The 'getsubopt()' primitive shall parse suboption arguments in a
        flag argument. Such options often result from the use of
        <getopt>.

    Macro Parameters:
        options - Option declaration list of one more strings of the
            following form "tag[, index][[:=][type]]", see below for more
            details.

        value - Tag value, otherwise set to an empty string.

        args - Argument buffer to be processed. Note, on success the
            buffer shall be modified with the leading matched option and
            trailing value removed.

        delim - Optional delimiter, if omitted defaults to a comma(,).

    Macro Return:
        Option index, otherwise one of the following negative error
        codes. If the case of error codes -2 or less, value shall
        contain an error condition descripting the condition,

            -1 -    End of the options (EOF).
            -2 -    Unknown option.
            -4 -    Argument required.
            -5 -    Unexpected argument.
            -6 -    Invalid value, for example "expected a numeric value".
            -10 -   Invalid option specification.

    Macro Portability:
        A Grief extension.

    Macro Example:

(code)
        list suboptions = {
                "help,h",
                "verbose,v",
                "verbose2,#2",
                "integer:i"
                };

        if ((ch = getsubopt(value, suboptions, get_parm(1))) >= 0) {
            do {
                switch (ch) {
                case 'h':
                    break;
                case 'v':
                    break;
                case 2:
                    break;
                case 3:
                    break;
                default:
                    break;
                }
            } while ((ch = getsubopt(value)) >= 0);
        }
        if (ch < 0) {
            error(value);
        }
(end)

 */
void
do_getsubopt(void)              /* (string value, [list options], [string|list args],
                                        [string delim], [string quotes]) */
{
    const LIST *options = get_xlist(2);
    const char *value = NULL;
    char errbuf[128];
    int ret = -1;                               /* return, "end of options" */

    /*
     *  new options
     */
    if (options) {
        const char *sargs = get_xstr(3);
        const int delimiter = get_xcharacter(4);
        const char *quotes = get_xstr(5);
        int count = 0;

        getsubopt_shutdown();                   /* reset */

        if (NULL == (x_getsubopt_options = longdecode(options, &count))) {
            ED_TRACE(("getsubopt() : -10\n"))
            acc_assign_int(-10);
            return;
        }

        if (sargs && sargs[0]) {
            x_getsubopt_cursor = x_getsubopt_buffer = chk_salloc(sargs);
        }

        x_getsubopt_delimiter = (delimiter > 0 ? delimiter : ',');
        x_getsubopt_quotes = chk_salloc(quotes ? quotes : "'\"");
    }

    /*
     *  next option
     */
    if (x_getsubopt_options && x_getsubopt_cursor) {
        const char *tag = NULL;
        char *cursor;

        /*
         *  skip leading white-space, commas
         */
        for (cursor = x_getsubopt_cursor; *cursor &&
                (*cursor == x_getsubopt_delimiter || is_white(*cursor)); ++cursor) {
            /*continue*/;
        }

        /*
         *  token start?
         */
        if (*cursor) {
            /*
             *  save the start of the token, and skip the rest of the token.
             */
            for (tag = cursor; *++cursor &&
                    *cursor != x_getsubopt_delimiter && *cursor != '=' && !is_white(*cursor);) {
                /*continue*/;
            }

            if (*cursor) {
                /*
                 *  If there's an equals sign, set the value pointer, and skip over the
                 *  value part of the token. Terminate the token.
                 */
                if ('=' == *cursor) {
                    char quoted = 0;

                    *cursor = '\0';
                    for (value = ++cursor; *cursor &&
                              ((!quoted && *cursor != x_getsubopt_delimiter && !is_white(*cursor))
                            || ( quoted && *cursor != quoted)); ++cursor) {
                        /*
                         *  opening character denotes start of quoted value ?
                         */
                        if (value == cursor && strchr(x_getsubopt_quotes, *cursor)) {
                            quoted = *cursor;
                            ++value;            /* remove opening quote */
                        }
                    }
                    if (*cursor) {
                        *cursor++ = '\0';       /* terminate current value */
                    }

                } else {
                    *cursor++ = '\0';
                }

                /*
                 *  Skip any whitespace/delimiter after this token.
                 */
                for (; *cursor &&
                        (*cursor == x_getsubopt_delimiter || is_white(*cursor)); ++cursor) {
                    /*continue*/;
                }

                /*
                 *  match and return result
                 */
                x_getsubopt_cursor = cursor;

                ret = -2;                       /* error, "unknown option" */
                if (x_getsubopt_options) {
                    const struct argoption *option;

                    for (option = x_getsubopt_options; option->name; ++option) {
                        if (0 == strcmp(option->name, tag)) {
                            /*
                             *  absolute match
                             */
                            ret = option->val;
                            if (NULL == value) {
                                if (arg_required == option->has_arg) {
                                    sxprintf(errbuf, sizeof(errbuf),  "option '%s' requires an argument", tag);
                                    value = errbuf;
                                    ret = -4;   /* error, "argument required" */
                                }
                            } else {
                                if (arg_none == option->has_arg) {
                                    sxprintf(errbuf, sizeof(errbuf), "option '%s' doesn't allow an an argument", tag);
                                    value = errbuf;
                                    ret = -5;   /* error, "unexpected argument" */
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    argv_assign_str(1, value ? value : "");
    ED_TRACE(("getsubopt() : %d (%s)\n", ret, (value ? value : "")))
    if (ret < 0) {
        getsubopt_shutdown();
    }
    acc_assign_int(ret);
}


void
getsubopt_shutdown(void)
{
    ED_TRACE2(("\tgetsubopt_shutdown\n"))
    x_getsubopt_cursor = NULL;
    chk_free((void *) x_getsubopt_options), x_getsubopt_options = NULL;
    chk_free((void *) x_getsubopt_buffer); x_getsubopt_buffer = NULL;
    chk_free((void *) x_getsubopt_quotes); x_getsubopt_quotes = NULL;
    x_getsubopt_delimiter = ',';
}


/*  Function:           do_split_arguments
 *      Argument split.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: split_arguments - Argument split

        list
        split_arguments(string arguments)

    Macro Description:
        The 'split_arguments()' primitive splits 'argument' into list of
        words. As it parses the command line, 'split_arguments' looks
        for command separators, white space (spaces and tabs). Normally,
        these special characters cannot be passed to a command as part
        of an argument. However, special characters in an argument by
        enclosing the entire argument in double quotes ["]. The ["]
        themselves will not be passed to the command as part of the
        argument. Within a double quoted string the ["] character and
        [\] character can be represented as [\"] and [\\].

    Macro Parameters:
        argument - String containing the argument buffer.

    Macro Return:
        The 'split_argument' returns a list of words encountered within
        the argument buffer.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        getsubopt
 */
void
do_split_arguments(void)        /* list (string arguments) */
{
    const char *arguments = get_xstr(1);
    int maxargc = get_xinteger(2, GETOPT_ARGC);
    LIST *newlp = NULL;
    int llen = 0;

    if (maxargc < 1) {
        maxargc = 1;
    }

    if (arguments && arguments[0]) {
        char *args = NULL;
        const char **argv;

        if (NULL != (args = chk_salloc(arguments))) {
            if (NULL != (argv = chk_calloc(sizeof(char *) * (maxargc + 1), 1))) {
                int argc;

                if ((argc = arg_split(args, argv, maxargc)) > 0) {
                    llen = (argc * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
                    if (NULL != (newlp = lst_alloc(llen, argc))) {
                        LIST *lp = newlp;
                        int idx;

                        for (idx = 0; idx < argc; ++idx) {
                            lp = atom_push_str(lp, argv[idx]);
                        }
                        atom_push_halt(lp);
                    }
                }
                chk_free((void *)argv);
            }
            chk_free(args);
        }
    }

    if (newlp && llen) {
        acc_donate_list(newlp, llen);
    } else {
        acc_assign_null();
    }
}


/*  Function:           longdecode
 *      Decode a long option specification.
 *
 *  Parameters:
 *      options - Option definition.
 *      optcount - Returning option count.
 *
 *  Returns:
 *      Address of argoption vector, otherwise NULL.
 */
static struct argoption *
longdecode(const LIST *options, int *optcount)
{
    struct argoption *argoptions = NULL;
    const LIST *nextlp, *lp;
    int count = 0;

    if (options) {
        for (lp = options; (nextlp = atom_next(lp)) != lp; lp = nextlp) {
            const char *spec;

            if (NULL != (spec = atom_xstr(lp))) {
                if (optiondecode(spec, NULL, NULL)) {
                    ewprintf("getopt: invalid specification (%s)", spec);
                    return NULL;
                }
                ++count;
            }
            lp = nextlp;
        }
    }

    if (count) {
        int ret, value = 0;

        if (NULL == (argoptions =
                chk_calloc(count + 1, sizeof(struct argoption)))) {
            return NULL;
        }
        count = 0;
        for (lp = options; (nextlp = atom_next(lp)) != lp; lp = nextlp) {
            const char *spec;

            if (NULL != (spec = atom_xstr(lp))) {
                ret = optiondecode(spec, &value, argoptions + count);
                assert(0 == ret);
                ++count;
            }
        }
        argoptions[count].name = NULL;
    }

    if (optcount) {
        *optcount = count;
    }
    return argoptions;
}


/*  Function:           optiondecode
 *      Decode an option specification
 *
 *  Parameters:
 *      spec - Option specification.
 *      cursor - Index cursor.
 *      p - Option to populated.
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
static int
optiondecode(const char *spec, int *cursor, struct argoption *p)
{
    const char *tag = NULL, *ch;
    int has_arg = arg_none, value = 0, type = OTYPE_NONE;

    ED_TRACE(("\toptiondecode(%s)", spec))

    /* value */
    if (NULL != (ch = strchr(spec, ','))) {

        if (NULL == tag) {
            tag = chk_snalloc(spec, ch - spec);
        }

        if ('#' == *++ch) {
            ++ch;
            if (! isdigit(*ch)) {               /* quoted character */
                value = *ch++;

            } else {                            /* #value */
                char *endp = NULL;

                errno = 0;
                value = (int) strtol(ch, &endp, 0);
                if (EINVAL == errno || endp == ch || value <= 0 || value > 0xffff) {
                    goto error;                 /* invalid value */
                }
                ch = endp;
            }
        } else {
            value = *ch++;
        }

        if ('?' == value || ':' == value) {
            goto error;                         /* reserved values */
        }
        spec = ch;

    } else if (cursor) {
        value = *cursor;
    }

    /* option parameter and type */
    if (NULL != (ch = strchr(spec, ':')) ||
            NULL != (ch = strchr(spec, '='))) {

        if (NULL == tag) {
            tag = chk_snalloc(spec, ch - spec);
        }

        has_arg = (':' == *ch ? arg_required : arg_optional);

        if (*++ch) {
            type = -1;
            switch (tolower((unsigned char)*ch)) {
            case 'a':       /* a[nything] */
                if (0 == ch[1] || 0 == str_icmp(ch, "anything")) {
                    type = OTYPE_ANYTHING;
                }
                break;
            case 'i':       /* i[integer][+]    decimal integer value plus optional positive only modifier. */
                if (0 == ch[1] || 0 == str_icmp(ch, "integer")) {
                    type = OTYPE_INTEGER;

                } else if (0 == str_icmp(ch, "i+") || 0 == str_icmp(ch, "integer+")) {
                    type = OTYPE_INTEGER | OTYPE_MODIFIER_PLUS;
                }
                break;
            case 'n':       /* n[numeric][+]    numeric (oct, dec and hex). */
                if (0 == ch[1] || 0 == str_icmp(ch, "numeric")) {
                    type = OTYPE_NUMERIC;

                } else if (0 == str_icmp(ch, "n+") || 0 == str_icmp(ch, "numeric+")) {
                    type = OTYPE_NUMERIC | OTYPE_MODIFIER_PLUS;
                }
                break;
            case 'c':       /* c[haracter]      character value. */
                if (0 == ch[1] || 0 == str_icmp(ch, "character")) {
                    type = OTYPE_CHARACTER;
                }
                break;
            case 's':       /* s[tring]         string. */
                if (0 == ch[1] || 0 == str_icmp(ch, "string")) {
                    type = OTYPE_STRING;
                }
                break;
            case 'f':       /* f[loat]          floating point including integer. */
                if (0 == ch[1] || 0 == str_icmp(ch, "float")) {
                    type = OTYPE_FLOAT;
                }
                break;
            case 'd':       /* d[ouble]         double precision floating point including integer. */
                if (0 == ch[1] || 0 == str_icmp(ch, "double")) {
                    type = OTYPE_DOUBLE;
                }
                break;
            case 'b':       /* b[oolean]        boolean, y[es]/n[o], no/off, true/false. */
                if (0 == ch[1] || 0 == str_icmp(ch, "boolean")) {
                    type = OTYPE_BOOLEAN;
                }
                break;
            }

            if (type < 0) {
                goto error;
            }
        }
    }

    /* populate option buffer */
    if (NULL == tag) {
        tag = chk_salloc(spec);
    }

    if (NULL == tag || 0 == *tag || '-' == *tag || ' ' == *tag) {
        goto error;                             /* empty/invalid tag */
    }

    if (cursor && p) {
        ED_TRACE(("(tag:%s, value:%d/'%c', has_arg:%d, type:%d) : 0\n", tag, value, value, has_arg, type))
        p->name  = tag;
        p->has_arg = has_arg;
        p->val   = value;
        p->udata = type;
        *cursor  = ++value;
        return 0;
    }
    ED_TRACE((": 0\n"))
    chk_free((char *)tag);
    return 0;

error:;
    ED_TRACE((": -1\n"))
    chk_free((char *)tag);
    return -1;
}


static int
booleanvalue(const char *value)
{
    if (value) {
        switch (*value) {
        case '1':               /* 1 */
            if (value[1] == '\0') {
                return TRUE;
            }
            break;
        case '0':               /* 0 */
            if (value[1] == '\0') {
                return FALSE;
            }
            break;
        case 'y': case 'Y':     /* y[es] */
            if (*value ||
                    ((value[1] == 'e' || value[1] == 'E') &&
                     (value[2] == 's' || value[2] == 'S') &&
                      value[3] == '\0')) {
                return TRUE;
            }
            break;
        case 'n': case 'N':     /* n[o] */
            if (*value ||
                    ((value[1] == 'o' || value[1] == 'O') &&
                      value[2]== '\0')) {
                return FALSE;
            }
            break;
        case 'o': case 'O':     /* on/off */
            if (0 == str_icmp(value, "on")) {
                return TRUE;
            }
            if (0 == str_icmp(value, "off")) {
                return FALSE;
            }
            break;
        case 't': case 'T':     /* true */
            if (0 == str_icmp(value, "true")) {
                return TRUE;
            }
            break;
        case 'f': case 'F':     /* false */
            if (0 == str_icmp(value, "false")) {
                return FALSE;
            }
            break;
        }
    }
    return -1;
}


static int
optioncheck(int type, const char *value)
{
    int modifierplus = 0;

    if (OTYPE_MODIFIER_PLUS & type) {
        type &= ~OTYPE_MODIFIER_PLUS;
        ++modifierplus;
    }

    assert(type >= 0);
    assert(type <= OTYPE_BOOLEAN);

    switch (type) {
    case OTYPE_NONE:            /* none */
        if (value && *value) {
            return FALSE;
        }
        break;

    case OTYPE_ANYTHING:        /* anything */
        break;

    case OTYPE_INTEGER: {       /* decimal integer */
            char *endp = NULL;
            int ivalue;

            if (NULL == value) {
                return FALSE;
            }
            errno = 0;
            ivalue = (int) strtol(value, &endp, 10);
            if (errno || *endp) {
                return FALSE;
            } else if (ivalue < 0 && modifierplus) {
                return FALSE;
            }
        }
        break;

    case OTYPE_NUMERIC: {       /* numeric (oct, dec and hex) */
            char *endp = NULL;
            int ivalue;

            if (NULL == value) {
                return FALSE;
            }
            errno = 0;
            ivalue = (int) strtol(value, &endp, 0);
            if (errno || *endp) {
                return FALSE;
            } else if (ivalue < 0 && modifierplus) {
                return FALSE;
            }
        }
        break;

    case OTYPE_CHARACTER:       /* character value */
    case OTYPE_STRING:          /* string */
        if (NULL == value) {
            return FALSE;
        }
        break;

    case OTYPE_FLOAT:           /* floating point */
    case OTYPE_DOUBLE: {        /* double precision floating */
            char *endp = NULL;
            double dvalue;

            if (NULL == value) {
                return FALSE;
            }
            errno = 0;
            dvalue = strtod(value, &endp);
            if (errno || *endp) {
                return FALSE;
            } else if (dvalue < 0 && modifierplus) {
                return FALSE;
            }
        }
        break;

    case OTYPE_BOOLEAN:         /* boolean */
        switch (booleanvalue(value)) {
        case TRUE:
        case FALSE:
            break;
        default:
            return FALSE;
        }
        break;

    default:                    /* others/quiet compiler */
        break;
    }
    return TRUE;
}


/*  Function:           is_white
 *      Determine whether a white-space character (ie. space or tab).
 *
 *  Parameters:
 *      c - Character value.
 *
 *  Returns:
 *      *true* if a white-space character, otherwise *false*.
 */
static int
is_white(int ch)
{
    return (' ' == ch || '\t' == ch);
}

/*end*/

