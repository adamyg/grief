#include <edidentifier.h>
__CIDENT_RCSID(gr_m_tokenize_c,"$Id: m_tokenize.c,v 1.32 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_tokenize.c,v 1.32 2025/02/07 03:03:21 cvsuser Exp $
 * String primitives.
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

#if defined(LOCAL_MAIN)
#define ED_LEVEL 3
#endif
#include <editor.h>
#include <limits.h>
#if defined(LOCAL_MAIN)
#include <unistd.h>
#endif
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_tokenize.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "symbol.h"
#include "word.h"

#define SPLIT_QUOTING_SINGLE    0x01
#define SPLIT_QUOTING_DOUBLE    0x02

struct split {
    const char *        sp_delims;              /* delimiters */
    unsigned            sp_quoting;
    unsigned            sp_numerics;
    unsigned            sp_empties;
    unsigned            sp_limit;
    unsigned            sp_strings;             /* string atom count */
    unsigned            sp_integers;            /* integer atom count */
    LIST *              sp_list;                /* reply */
};

struct tokenizer {
    const char *        tk_delims;              /* delimiters */
    unsigned            tk_flags;               /* processing flags */
    unsigned            tk_strings;             /* string atom count */
    unsigned            tk_integers;            /* integer atom count */
    unsigned            tk_length;              /* length (used) in bytes */
    unsigned            tk_size;                /* buffer size, in bytes */
    char                tk_whitespace[8];       /* white-space characters */
    char *              tk_buffer;              /* buffer address */
    char *              tk_cursor;              /* token cursor, within buffer */
    LIST *              tk_list;                /* reply */
};


/*
 *  tokenize buffer.
 */
#define tok_clear(_tok) \
    _tok->tk_length = 0, _tok->tk_cursor = _tok->tk_buffer

#define tok_push(_tok, _ch) { \
        if (++_tok->tk_length >= _tok->tk_size) { \
            _tok->tk_buffer = chk_realloc(_tok->tk_buffer, _tok->tk_size += 1024); \
            _tok->tk_cursor = _tok->tk_buffer + (_tok->tk_length - 1); \
        } \
        *_tok->tk_cursor++ = _ch; \
    }

#define tok_delete(_tok) \
    chk_free((_tok)->tk_buffer)

static int              split_buffer(struct split *sp, const char *str, size_t length);
static int              split_isquote(struct split *sp, char ch);
static int              split_isnumeric(char ch);

static int              tokenize_buffer(struct tokenizer *tokens, const char *buffer, size_t length);
static void             tok_initialise(struct tokenizer *tok, const char *delims, unsigned flags, const char *whitespace);
static int              tok_escape(struct tokenizer *tok, const char **cursor);
static char             tok_isquote(unsigned flags, const char ch);
static int              tok_isnumeric(const struct tokenizer *tok, const char *cursor);
static int              tok_iswhitespace(const struct tokenizer *tok, const char ch);
static const char *     tok_triml(const struct tokenizer *tok, const char *cursor);
static size_t           tok_trimr(const struct tokenizer *tok, const char *buffer, size_t length);


/*  Function:           do_split
 *      split primitive, function to split a string into tokens
 *      and assign to a list.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: split - Split a string into tokens.

        list
        split(string expr, string|integer delims,
            [int numeric = FALSE], [int noquoting = FALSE],
                [int empties = FALSE], [int limit = 0])

    Macro Description:
        The 'split()' primitive splits the string 'expr' into a list of
        strings and returns the resulting list.

        split provides simple field processing, compared to the
        <tokenize> primitive which offers greater functionality.

    Macro Parameters:
        expr - String to be parsed.

        delims - Specifies either a string containing the characters
            used to split the token, alternative an integer character
            value of a single character.

        numeric - If specified and is true(1), then all tokens which
            start with a digit are converted to a number, rather than a
            string. Alternative when given as two (2) then values are
            converted using strtol(), allowing support leading base
            specifications hexidecimal (0x), octal (0) and binary (0b).

        nonquoting - Upon the 'delim' parameter containing a double
            quote character then it is assumed that any entries inside
            double quotes should have any embedded characters preserved.
            When specified the 'noquote' optional allows explicit
            control enabling and disabling of this feature.

        empties - Upon the 'delim' character being given as a integer
            value empty split column are not returned within the list.
            When specified the 'empties' optional allows explicit
            control enabling (non-zero) and disabling (zero) to whether
            empties are returned.

        limit - Limit the split to 'limit' elements, returning the trailing
            content in the last element; note any special processing
            including quotes and numeric wont apply on the trailing.

    Macro Results:
        List of strings where each string is a 'token' from the string
        parameter.

    Macro Portability:
        The options 'empties' and 'limit' are Grief extensions.

    Macro See Also:
        tokenize, split, sscanf, index, substr.

    Macro Examples:

        Using '|' was a delimiter and empties being returned.

>           ''         ==> ''
>           'a'        ==> 'a'
>           '|b'       ==> '', 'b'
>           '|'        ==> '', ''
>           'a|b'      ==> 'a', 'b'
>           'a|b|'     ==> 'a', 'b', ''
>           'a|b|c'    ==> 'a', 'b', 'c'
>           'a||b'     ==> 'a', '', 'b'

        the same without empties

>           ''          ==>
>           'a'         ==> 'a'
>           '|b'        ==> 'b'
>           '|'         ==>
>           'a|b'       ==> 'a', 'b'
>           'a|b|'      ==> 'a', 'b'
>           'a||b'      ==> 'a', 'b'

 */
void
do_split(void)                  /* (string buffer, string|integer delims, [, int numeric = FALSE]
                                        [, int noquoting = FALSE] [, int empties = FALSE] [, int limit = 0]) */
{
    const char *buffer = get_str(1);            /* buffer */
    size_t length = (buffer ? get_strlen(1):0); /* and its associated length */
    const char *delims = get_xstr(2);           /* string|integer */
    int numerics = get_xinteger(3, FALSE);
    int noquoting = get_xinteger(4, FALSE);
    int empties = get_xinteger(5, FALSE);
    int limit = get_xinteger(6, 0);
    unsigned quoting = 0;
    char t_delims[2];

    /*
     *  delimiter as string or character/
     *      if specified as a character then empty columns shall be filtered
     *      unless 'empties' is explicitly stated.
     */
    if (NULL == delims) {                       /* allow (string|character) */
        const int xdelim = get_xcharacter(2);

        t_delims[0] = (char)(xdelim > 0 ? xdelim : ' ');
        t_delims[1] = 0;
        if (get_xinteger(4, -1) == -1) {
            empties = 0;                        /* filter empties */
        }
        delims = t_delims;
    }

    /*
     *  quoting when double
     */
    if (0 == noquoting) {
        const char *end;

        for (end = delims; *end; ++end)
            if ('\'' == *end) {                 /* single quotes */
                quoting |= SPLIT_QUOTING_SINGLE;

            } else if ('"' == *end) {           /* double quotes */
                quoting |= SPLIT_QUOTING_DOUBLE;
            }
    }

    /*
     *  split
     */
    if (0 == length || NULL == delims || 0 == delims[0]) {
        acc_assign_null();

    } else {
        struct split sp = {0};

        sp.sp_delims   = delims;
        sp.sp_numerics = numerics;
        sp.sp_quoting  = quoting;
        sp.sp_empties  = empties;
        sp.sp_limit    = (limit <= 0 ? 0 : (unsigned)limit);

        ED_TRACE(("\tsplit(%d, %d/%d/%d/%d, '%s', '%s')\n", length, numerics, quoting, empties, limit, delims, buffer))

        if (0 == split_buffer(&sp, buffer, length)) {
            acc_assign_null();

        } else {
            const int llength = (sp.sp_integers * sizeof_atoms[F_INT]) +
                    (sp.sp_strings * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];

            if (NULL != (sp.sp_list = lst_alloc(llength, sp.sp_integers + sp.sp_strings))) {
                split_buffer(&sp, buffer, llength);
                acc_donate_list(sp.sp_list, llength);

            } else {
                acc_assign_null();
            }
        }
    }
}


static int
split_buffer(struct split *sp, const char *str, size_t length)
{
    const char *delims = sp->sp_delims;
    const unsigned numerics = sp->sp_numerics;
    const unsigned empties = sp->sp_empties;
    unsigned limit = sp->sp_limit;
    LIST *lp = sp->sp_list;
    unsigned atoms = 0;
    const char *start, *end;
    int delim;

    __CUNUSED(length)

    for (end = str, delim = 0;;) {
        /* Skip blanks before next token */
        if (! empties) {
            while (*end && NULL != strchr(delims, *end)) {
                if (split_isquote(sp, *end)) {
                    break;
                }
                ++end;
            }

            if ('\0' == *end) {
                break;
            }
        }

        /* Skip over this token. */
        start = end;
        if (limit && 0 == --limit) {
            while (*end)
                ++end;                          /* last token; implied string */
            goto asstring;
        }

        if (split_isquote(sp, *end)) {
            char endquote = *end;               /* closing character */

            ++start, ++end;
            while (*end) {
                if (endquote == *end) {
                    ++delim;                    /* .. delim found */
                    break;
                }
                ++end;
            }
        } else {
            while (*end) {
                if (NULL != strchr(delims, *end)) {
                    ++delim;                    /* .. delim found */
                    break;
                }
                ++end;
            }
        }

        /* Place token */
        if (numerics && split_isnumeric(*start)) {
            /*
             *  numeric/
             *      allow hex (0x###), dec, oct (0###) and binary (0b###) (numerics >= 2)
             */
            accint_t ret = 0;

            if (lp || numerics >= SPLIT_NUMERIC_STRICT) {
                if (numerics >= SPLIT_NUMERIC_STRTOL) {
                    char *endp = NULL;
                    int base = 0;

                    if ('0' == start[0] && 'b' == start[1]) {
                        start += 2;             /* 0bxxxx - binary */
                        base = 2;
                    }
                    ret = accstrtoi(start, &endp, base);
                    if (numerics >= SPLIT_NUMERIC_STRICT) {
                        if (NULL == endp || (*endp && NULL == strchr(delims, *endp))) {
                            goto asstring;      /* strict */
                        }
                    }
                } else {
                    ret = accstrtoi(start, NULL, 10);
                }
            }

            if (lp) {
                ED_TRACE(("\t\ttoken(INT,%d,%s) = %ld\n", atoms, start, ret))
                lp = atom_push_int(lp, ret);
            } else {
                ++sp->sp_integers;
            }

        } else {
            /*
             *  string
             */
asstring:;  if (lp) {
                ED_TRACE(("\t\ttoken(STR,%d,%d) = %s\n", atoms, (int)(end - start), start))
                lp = atom_push_nstr(lp, start, end - start);
            } else {
                ++sp->sp_strings;
            }
        }
        ++atoms;

        /* Check to see if we've finished. */
        if (delim) {
            ++end, delim = 0;                   /* skip delimiter */
        } else if ('\0' == *end) {
            break;                              /* ... end of string */
        }
    }

    if (lp) {
        atom_push_halt(lp);
    }
    return atoms;
}


static int
split_isquote(struct split *sp, char ch)
{
    const unsigned quoting = sp->sp_quoting;

    if (quoting) {
        if ((quoting & SPLIT_QUOTING_SINGLE) && '\'' == ch) {
            return SPLIT_QUOTING_SINGLE;

        } else if ((quoting & SPLIT_QUOTING_DOUBLE) && '"' == ch) {
            return SPLIT_QUOTING_DOUBLE;

        }
    }
    return 0;
}


static int
split_isnumeric(char ch)
{
    if (isdigit(ch) || '+' == ch || '-' == ch) {
        return TRUE;
    }
    return FALSE;
}


/*  Function:           do_tokenize
 *      split tokenize, function to tokenize a string into list of word.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: tokenize - Tokenize a string into token elements.

        list
        tokenize(string expr, string delims, int flags, [string whitespace = "\t\n\r"])

    Macro Description:
        The 'tokenize()' primitive tokenizes the string 'expr' into a
        list of strings and returns the list in list context.

        tokenize() provides greater field processing then the simpler
        <split> primitive and should be used by new macros.

    Macro Parameters:

        expr - String to be tokenize.

        delims - is a string consisting of one or more characters which
            indicate the delimiter characters.

        flags - is an integer containing a set of flags which indicate
            how the input string is to be tokenized. Flags consist of
            one or more the 'tokenize flags' detailed below OR'ed
            together.

        whitespace - Optional set of characters to be treated as
            whitespace.

    Tokenize flags:

      General::

        o *TOK_COLLAPSE_MULTIPLE* -
            Splits the string 'expr' into a list of strings and returns
            the list in list context. Collapses occurrences of the repeated
            delimiter characters treating them as single delimiter, in other
            words empty elements with the delimited text shall not be returned.


      Numeric field conversion::

        o *TOK_NUMERIC* -
            Fields which begin with a digit are converted into their
            decimal numeric value and returned as integer element rather
            than a string.

        o *TOK_NUMERIC_STRTOL* -
            Numeric fields are converted using <strtol> allowing
            support leading base specifications hexadecimal (0x),
            octal (0) and binary (0b).

        o *TOK_NUMERIC_STRICT* -
            Strict conversion of numeric fields where by any invalid
            values, for example trailing non-numeric characters, result
            in the field being returned as a string element and not a
            integer element.

      Parsing options::

        o *TOK_WHITESPACE* -
            Allow leading and trailing whitespace around quoted and
            numeric element.

        o *TOK_BACKSLASHES* -
            Allow backslashes to escape the meaning of any delimiter
            characters and both single and double.

        o *TOK_ESCAPE* -
            Enable backslash escape sequence processing.

        o *TOK_ESCAPEALL* -
            Control the behaviour of *TOK_ESCAPE* to escape all
            characters preceded with a backslashes, otherwise by default
            unknown escape sequences are ignored.

     Quote options::

        o *TOK_DOUBLE_QUOTES* -
            Enables double quote support where all characters enclosed
            within a pair of matching quotes are treated as a single
            element including any embedded delimiters.

        o *TOK_DOUBLE_QUOTES* -
            Same as *TOK_DOUBLE_QUOTES* but enables single quote support.

        o *TOK_QUOTE_STRINGS* -
            When single or double quoted support is enabled allow the
            element is be enclosed within a extra pair of quotes, for
            example

>               ""hello world""

        o *TOK_PRESERVE_QUOTES* -
            When an element is enclosed in quotes and the quote character
            is specified in 'delims' then the returned element shall also
            be enclosed within the encountered quotes.

     Field Processing Options::

        o *TOK_TRIM_LEADING* -
            Remove any leading whitespace from non-quoted string
            elements. Whitespace is defined as any space, tab or newline
            character unless they exist within the set of specified
            delimiters.

        o *TOK_TRIM_TRAILING* -
            Remove any trailing whitespace from string elements.

        o *TOK_TRIM* -
            Remove any leading and trailing whitespace characters.

        o *TOK_TRIM_QUOTED* -
            Apply trim logic to quoted strings.

    Macro Return:
        The 'tokensize()' primitive returns a list of the words
        and/or numeric values as encountered within the string 'str'.

    Macro Portability:
        Many of the features are Grief specific; CRiSPEdit has a
        similar primitive yet as the two were developed independently
        features differ.

    Macro See Also:
        tokenize, split, sscanf, index, substr.

 */
void
do_tokenize(void)               /* (string str, string delims, int flags = 0, [string whitespace = "\t\n\r"]) */
{
    const char def_whitespace[] = {' ', '\t', '\n', '\r', 0};

    const char *buffer = get_str(1);            /* string */
    size_t length = (buffer ? get_strlen(1):0); /* and its associated length */
    const char *delims = get_xstr(2);           /* delimiters (string|integer) */
    int flags = get_xinteger(3, 0);             /* flags */
    const char *whitespace = get_xstr(4);       /* optional whitespace character override */
    struct tokenizer tok;
    char t_delims[2];

    /*
     *  parse arguments
     */
    if (NULL == delims) {                       /* allow (string|character) */
        const int xdelim = get_xcharacter(2);

        if (xdelim > 0) {
            t_delims[0] = (char)xdelim;
            t_delims[1] = 0;
            delims = t_delims;
        }
    }

    if (delims && (TOK_PRESERVE_QUOTES & flags)) {
        if (strchr(delims, '"')) {
            flags |= TOK_DOUBLE_QUOTES;         /* enable double quotes */
        }
        if (strchr(delims, '\'')) {
            flags |= TOK_SINGLE_QUOTES;         /* enable single quotes */
        }
        if (0 == (flags & (TOK_SINGLE_QUOTES|TOK_DOUBLE_QUOTES))) {
            flags &= ~TOK_PRESERVE_QUOTES;      /* not needed */
        }
    }

    /*
     *  tokensize/
     *
     *      XXX - optimise list usage, preallocate and extend verses two passes. Need to
     *      profile and determine best approach.  Preallocation mat also result in
     *      higher memory usage.
     */
    tok_initialise(&tok, delims, flags, (whitespace ? whitespace : def_whitespace));

    ED_TRACE(("\ttokensize(%d, 0x%x, '%s', '%s')\n", length, flags, delims, buffer))

    if (0 == length || NULL == delims || 0 == delims[0]) {
        acc_assign_null();

    } else if (0 == tokenize_buffer(&tok, buffer, length)) {
        acc_assign_null();

    } else {
        const int llength = (tok.tk_integers * sizeof_atoms[F_INT]) +
                (tok.tk_strings * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];

        if (NULL != (tok.tk_list = lst_alloc(llength, tok.tk_integers + tok.tk_strings))) {
            tokenize_buffer(&tok, buffer, llength);
            acc_donate_list(tok.tk_list, llength);

        } else {
            acc_assign_null();
        }
    }

    tok_delete(&tok);
}


static int
tokenize_buffer(struct tokenizer *tok, const char *buffer, size_t buflen)
{
    const char *delims = tok->tk_delims;
    const unsigned flags = tok->tk_flags;
    LIST *lp = tok->tk_list;
    const char *cursor = buffer;
    unsigned atoms = 0;

    __CUNUSED(buflen)

    // Break argument list.
    //
    while (*cursor) {
        const char *start = cursor, *leading = cursor;
        int quoted_string = FALSE;
        size_t length;
        char quoting, ch;

        //  Consume leading whitespace (if required).
        //
        ED_TRACE(("\to cursor (%.16s...)\n", cursor))

        if (TOK_WHITESPACE & flags) {
            leading = tok_triml(tok, leading);
            if (! *leading) {                   /* termination */
                if ((TOK_TRIM & flags) && (TOK_COLLAPSE_MULTIPLE & flags)) {
                    break;                      /*  EOS, empty and not required */
                }
            }
        }

        //  Quoted strings.
        //
        if (0 != (quoting = tok_isquote(flags, *leading))) {
            ED_TRACE(("\t >> matching quoted '%c'\n", quoting))

            cursor = ++leading;

            tok_clear(tok);                     /* clear accumulator */

            if (TOK_PRESERVE_QUOTES & flags) {
                tok_push(tok, quoting);         /* opening quote character */
            }

            if (quoting == *leading && (TOK_QUOTE_STRINGS & flags)) {
                /*  Quoted strings
                //      ""text"",
                //
                //  but only if the construct doesn't match
                //      ""[white-space],
                */
                ++leading;                      /* consume quote */

                while (tok_iswhitespace(tok, *leading)) {
                    ++leading;                  /* next non-whitespace character */
                }

                if (*leading && NULL == strchr(delims, *leading)) {
                    /*
                    //  next character is NOT a delimiter, hence include
                    */
                    quoted_string = TRUE;       /* denotes nested quotes */
                    tok_push(tok, quoting);
                    ++cursor;
                }
            }

            while (0 != (ch = *cursor)) {
                /*
                //  Scan until end of string
                */
                ED_TRACE2(("\t -> character '%c' (%d/0x%x)\n", (isprint(ch) ? ch : ' '), ch, ch))
                ++cursor;                       /* consume character */

                if (quoting == ch) {
                    if (quoted_string && quoting == *cursor) {
                        /*
                        //  End of quoted string ""text""
                        */
                        ED_TRACE(("\t -> quoted_string complete\n"))
                        quoted_string = FALSE;
                        tok_push(tok, ch);
                        ++cursor;
                    }

                    if (! quoted_string) {      /* closing quote? */
                        /*
                        //  Throw away whitespace until EOS or delimiter, for example
                        //
                        //      "..."[white-space]{EOS|delimiter]
                        //      ""...""[white-space]{EOS|delimiter]
                        //
                        //  missing unmatched quotes result in the lexer back-tracking and
                        //  ignoring quotes all together.
                        */
                        if (TOK_PRESERVE_QUOTES & flags) {
                            tok_push(tok, quoting);
                        }

                        if (TOK_WHITESPACE & flags) {
                            cursor = tok_triml(tok, cursor);
                        }

                        if (0 == (ch = *cursor)) {
                            ED_TRACE(("\t -> terminating character EOS\n"))
                            start = 0;          /* eos */

                        } else  {
                            ED_TRACE(("\t -> terminating character '%c' (%c/0x%x)\n", (isprint(ch) ? ch : ' '), ch, ch))
                            if (strchr(delims, ch)) {
                                start = 0;      /* end condition met (stop back-tracking) */
                            }
                        }
                        break;
                    }

                } else if ('\\' == ch) {
                    ch = (char)tok_escape(tok, &cursor);
                }

                tok_push(tok, ch);              /* append */
            }
        }

        //  Unquoted strings (or back-tracked quoted string, start != 0)
        //
        if (start) {
            ED_TRACE(("\t >> matching unquoted\n"))

            tok_clear(tok);                     /* clear accumulator */
            cursor = start;                     /* start of string (maybe trimmed) */

            while (0 != (ch = *cursor) && NULL == strchr(delims, ch)) {
                /*
                //  Scan until delimiter or end-of-string.
                */
                ED_TRACE2(("\t -> character '%c'\n", ch))
                ++cursor;                       /* consume character */
                if ('\\' == ch) {
                    ch = (char)tok_escape(tok, &cursor);
                }
                tok_push(tok, ch);              /* append */
            }
        }

        //  Push result.
        //
        start = tok->tk_buffer;
        length = tok->tk_length;

        if (! quoting || (TOK_TRIM_QUOTED & flags)) {
            if (TOK_TRIM_LEADING & flags) {
                while (length && tok_iswhitespace(tok, *start)) {
                    ++start, --length;          /* left trim result */
                }
            }

            if (TOK_TRIM_TRAILING & flags) {
                if (length) {                   /* right trim result */
                    length = tok_trimr(tok, start, length);
                }
            }
        }

        if (length || 0 == (TOK_COLLAPSE_MULTIPLE & flags)) {

            tok_push(tok, 0);                   /* NUL terminate */

            if (((TOK_NUMERIC|TOK_NUMERIC_STRTOL|TOK_NUMERIC_STRICT) & flags) &&
                        tok_isnumeric(tok, start)) {
                /*
                //  Numeric
                //      allow hex (0x###), dec, oct (0###) and binary (0b###) (numerics >= 2)
                */
                accint_t ret = 0;               /* result */

                if (lp || (TOK_NUMERIC_STRICT & flags)) {
                    char *endp = NULL;
                    int base = 0;

                    if (TOK_NUMERIC_STRTOL & flags) {
                        if ('0' == start[0] && 'b' == start[1]) {
                            start += 2;         /* 0bxxxx - binary */
                            base = 2;
                        }
                    } else {
                        base = 10;              /* fixed base */
                    }

                    ret = accstrtoi(start, &endp, base);
                    if (TOK_NUMERIC_STRICT & flags) {
                        if ((TOK_WHITESPACE & flags) && endp) {
                            endp = (char *)tok_triml(tok, endp);
                        }
                        if (! endp || *endp) {
                            if (lp) {
                                const char t_ch = (char)(endp ? *endp : ' ');
                                __CUNUSED(t_ch)
                                ED_TRACE(("\t -> bad numeric '%c' (%d/0x%x)\n", (isprint(t_ch) ? t_ch : ' '), t_ch, t_ch))
                            }
                            goto asstring;      /* strict */
                        }
                    }
                }

                if (lp) {
                    ED_TRACE(("\t\ttoken(INT,%d,%s) = %ld\n", atoms, start, ret))
                    lp = atom_push_int(lp, ret);
                } else {
                    ++tok->tk_integers;
                }

            } else {
                /*
                //  String
                */
asstring:;      if (lp) {
                    ED_TRACE(("\t\ttoken(STR,%d,%d) = %s\n", atoms, length, start))
                    lp = atom_push_nstr(lp, start, length);

                } else {
                    ++tok->tk_strings;
                }
            }
            ++atoms;
        }

        //  Next or end.
        //
        if (0 == *cursor) {
            break;                              /* end of buffer */
        }

        if ((TOK_PRESERVE_QUOTES & flags) &&
                tok_isquote(flags, *cursor)) {
            /*
            //  not 100% unsure of symantics/
            //      if TOK_PRESERVE_QUOTES then dont consume the delimiter if a quote character
            //
            //  allows the following constructs
            //      "hello""world"
            */
            continue;
        }

        ++cursor;                               /* consume delimiter */
    }

    if (lp) {
        atom_push_halt(lp);
    }

    ED_TRACE(("\t==> %u (%u/%d)\n", atoms, tok->tk_strings, tok->tk_integers))
    assert(atoms == (tok->tk_strings + tok->tk_integers));
    return atoms;
}


/* initialise the token context structure */
static void
tok_initialise(struct tokenizer *tok, const char *delims, unsigned flags, const char *whitespace)
{
    unsigned cursor, idx;

    /*
     *  base object
     */
    memset(tok, 0, sizeof(struct tokenizer));
    tok->tk_delims = delims;
    tok->tk_flags = flags;

    /*
     *  setup the whitespace character-set
     */
    for (cursor = idx = 0; whitespace[idx] && cursor < (sizeof(tok->tk_whitespace)-1); ++idx) {
        if (NULL == strchr(delims, whitespace[idx])) {
            tok->tk_whitespace[cursor++] = whitespace[idx];
        }
    }
    tok->tk_whitespace[cursor] = 0;
}


/* decode an escape sequence */
static int
tok_escape(struct tokenizer *tok, const char **cursor)
{
    const char *t_cursor = *cursor;
    register char nextch = *t_cursor;
    const unsigned flags = tok->tk_flags;
    char ch = '\\';

    if ((TOK_ESCAPE|TOK_ESCAPEALL) & flags) {
        ++t_cursor;

        switch (nextch) {
        case '0': case '1': case '2': case '4': case '5': case '6': case '7':
            {/* octal (\[0-7]{1,3} */
                unsigned i, byte = 0;

                byte = (byte << 3) + nextch - '0';
                for (i = 2; i <= 3; ++i) {
                    nextch = *t_cursor;
                    if (nextch < '0' || nextch > '7') {
                        break;
                    }
                    byte = (byte << 3) + nextch - '0';
                    ++t_cursor;
                }
                ch = (char)byte;
            }
            break;

        case 'x':
            {/*hex (\x[0-9A-F]{1,3} */
                unsigned i, byte = 0;

                for (i = 0; i < 2; ++i) {
                    nextch = *t_cursor;         /* next character */
                    if (isdigit(nextch)) {
                        byte = (byte << 4) + nextch - '0';
                    } else if (nextch >= 'A' && nextch <= 'F') {
                        byte = (byte << 4) + nextch - 'A' + 10;
                    } else if (nextch >= 'a' && nextch <= 'f') {
                        byte = (byte << 4) + nextch - 'a' + 10;
                    } else {
                        break;
                    }
                    ++t_cursor;
                }

                if (i > 1) {                    /* valid hexidecimal value */
                    ch = (char)byte;

                } else if (TOK_ESCAPEALL & flags) {
                    ch = 'x';

                } else {
                    --t_cursor;                 /* "\x" */
                }
            }
            break;

        /* c/c++ style plus well-known extensions */
        case 'a':   ch = '\a'; break;           /* alert */
        case 'b':   ch = '\b'; break;           /* backspace */
        case 'e':   ch = 0x1b; break;           /* ESC */
        case 'f':   ch = '\f'; break;           /* formfeed */
        case 'n':   ch = '\n'; break;           /* newline */
        case 'r':   ch = '\r'; break;           /* return */
        case 't':   ch = '\t'; break;           /* tab */
        case 'v':   ch = '\v'; break;           /* vertical tab */
        case '\'':  ch = '\''; break;           /* single quote */
        case '\"':  ch = '\"'; break;           /* double quote */
        case '\\':  ch = '\\'; break;           /* quote */
        case '?':   ch = '?';  break;           /* literal question mark */

        /* unknown */
        default:
            if (TOK_ESCAPEALL & flags) {
                ch = nextch;                    /* \[unknown|delimiter] */

            } else if ((TOK_BACKSLASHES & flags) && strchr(tok->tk_delims, nextch)) {
                ch = nextch;                    /* \[delimiter], plus quotes already handled about */

            } else {
                /*unknown escape, report both backslash and character */
                --t_cursor;
            }
            break;
        }
        ED_TRACE2(("\t\tESC %c ==> '%c' (%d/0x%x)\n", nextch, (isprint(ch) ? ch : ' '), ch, ch))

    } else if (TOK_BACKSLASHES & flags) {
        if ('\"' == nextch || '\\' == nextch || strchr(tok->tk_delims, nextch)) {
            ch = nextch;                        /* character '\"', '\\' or delimiter */
            ++t_cursor;
        }
        ED_TRACE2(("\t\tBCK %c ==> '%c' (%d/0x%x)\n", nextch, (isprint(ch) ? ch : ' '), ch, ch))

    }

    *cursor = t_cursor;
    return ch;
}


/* determine whether a active quote chatracter */
static char
tok_isquote(unsigned flags, const char ch)
{
    if ((TOK_SINGLE_QUOTES & flags) && '\'' == ch) {
        return ch;

    } else if ((TOK_DOUBLE_QUOTES & flags) && '"' == ch) {
        return ch;
    }
    return 0;
}


/* determine if the element could be a numeric */
static int
tok_isnumeric(const struct tokenizer *tok, const char *cursor)
{
    char ch = *cursor;

    if (TOK_WHITESPACE & tok->tk_flags) {
        while (tok_iswhitespace(tok, ch)) {
            ch = *cursor++;
        }
    }

    if (isdigit(ch) || '+' == ch || '-' == ch) {
        return TRUE;
    }
    return FALSE;
}


/* left trim whitespace */
static const char *
tok_triml(const struct tokenizer *tok, const char *cursor)
{
    char ch = *cursor;

    while (tok_iswhitespace(tok, ch)) {
        ch = *++cursor;
    }
    return cursor;
}


/* right trim*/
static size_t
tok_trimr(const struct tokenizer *tok, const char *buffer, size_t length)
{
    const char *end = buffer + length;          /* end of string */

    while (length) {
        if (! tok_iswhitespace(tok, *--end)) {  /* not white-space */
            return length;
        }
        --length;                               /* remove character */
    }
    return 0;
}


/* is the specified character whitespace */
static int
tok_iswhitespace(const struct tokenizer *tok, const char ch)
{
    register const char *whitespace = tok->tk_whitespace;

    while (*whitespace) {
        if (ch == *whitespace++) {
            return 1;
        }
    }
    return 0;
}


#if defined(LOCAL_MAIN)
/*
 *  local test framework
 */
struct ATOM {
    int type;
    unsigned length;
    union {
        const char *s;
        double d;
        int i;
    } u;
};

static LIST *           l_return;
static unsigned         l_length;
static unsigned         l_atoms;
static struct ATOM      l_returns[100];

static const char *     arg_strings[8];
static int              arg_integers[8];

static int              x_debug;

static void             ret_print(int argi);
static int              ret_test_int(int argi, accint_t value);
static int              ret_test_str(int argi, const char *value);

LIST *
lst_alloc(size_t length, int atoms) {
    assert(length > 0);
    assert(atoms >= 0);
    assert(length < sizeof(l_returns));
    l_length = length;
    l_atoms = atoms;
    return (LIST *)l_returns;
}

void *
chk_alloc(size_t size) {
    return malloc(size);
}

void *
chk_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void
chk_free(void *ptr) {
    free(ptr);
}

static const char *
arg_string(int argi, const char *def) {
    const char *ret = def;

    if (argi > 0) {
        if (argi <= (sizeof(arg_strings)/sizeof(arg_strings[0]))) {
            if (arg_strings[argi-1]) {
                ret = arg_strings[argi-1];
            }
        }
    }
    return ret;
}

const char *
get_xstr(int argi) {
    const char *ret = arg_string(argi, NULL);
    ED_TRACE2(("\tget_xstr(%d) = %s\n", argi, (ret ? ret : "(null)")))
    return ret;
}

const char *
get_str(int argi) {
    const char *ret = arg_string(argi, "");
    ED_TRACE2(("\tget_str(%d) = %s\n", argi, ret))
    return ret;
}

int
get_strlen(int argi) {
    const char *str = arg_string(argi, "");
    int ret = strlen(str);
    ED_TRACE2(("\tget_strlen(%d) = %d\n", argi, ret))
    return ret;
}

static void
arg_clear() {
    memset(arg_strings, 0, sizeof(arg_strings));
    memset(arg_integers, 0, sizeof(arg_integers));
}

static int
arg_integer(int argi, int def) {
    int ret = def;

    if (argi > 0) {
        if (argi <= (sizeof(arg_integers)/sizeof(arg_integers[0]))) {
            if (arg_integers[argi-1] >= 0) {
                ret = arg_integers[argi-1];
            }
        }
    }
    return ret;
}

int
get_xinteger(int argi, int def) {
    int ret = arg_integer(argi, def);
    ED_TRACE2(("\tget_xinteger(%d) = %d\n", argi, ret))
    return ret;
}

int
get_xcharacter(int argi) {
    const char *str = arg_string(argi, NULL);
    int ret;

    if (str && str[0]) {
        ret = str[0];
    } else {
        ret = arg_integer(argi, 0);
    }
    ED_TRACE2(("\tget_xcharacter(%d) = %d\n", argi, ret))
    return ret;
}

int
trace_log(const char *fmt, ...) {
    if (x_debug) {
        va_list ap;

        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
    return 0;
}

LIST *
atom_push_nstr(LIST *lp, const char *value, size_t length) {
    struct ATOM *atoms = (struct ATOM *)lp;
    char *str;

    atoms->type = F_LIT;
    atoms->length = length;
    atoms->u.s = str = chk_alloc(length+1);
    memcpy((char *)str, value, length);
    str[length+1] = 0;
    return (LIST *)(atoms + 1);
}

LIST *
atom_push_int(LIST *lp, accint_t value) {
    struct ATOM *atoms = (struct ATOM *)lp;

    atoms->type = F_INT;
    atoms->u.i = value;
    return (LIST *)(atoms + 1);
}

LIST *
atom_push_halt(LIST *lp) {
    struct ATOM *atoms = (struct ATOM *)lp;

    atoms->type = F_HALT;
    return (LIST *)(atoms + 1);
}

static int
ret_test_int(int argi, accint_t value) {
    if (argi < l_atoms) {
        const struct ATOM *atom = l_returns + argi;

        if (F_INT == atom->type && value == atom->u.i) {
            return 0;
        }
    }
    printf("ERROR argi(%d) expected <%d> but ", argi, (int)value);
    ret_print(argi);
    printf("\n");
    return 1;
}

static int
ret_test_str(int argi, const char *value) {
    if (argi < l_atoms) {
        const struct ATOM *atom = l_returns + argi;
        const size_t length = strlen(value);

        if (F_LIT == atom->type &&
                length == atom->length && 0 == strncmp(value, atom->u.s, length)) {
            return 0;
        }
    }
    printf("ERROR argi(%d) expected <%s> but ", argi, value);
    ret_print(argi);
    printf("\n");
    return 1;
}

static void
ret_print(int argi) {
    if (argi < l_atoms) {
        const struct ATOM *atom = l_returns + argi;

        switch(atom->type) {
        case F_LIT:
            printf("LIT <%s>", atom->u.s);
            return;
        case F_INT:
            printf("INT <%d>", (int)atom->u.i);
            return;
        case F_HALT:
            printf("HALT");
            return;
        }
    }
    printf("NULL");
}

static void
ret_clear(void) {
    struct ATOM *atoms = (struct ATOM *)l_returns;
    unsigned idx;

    for (idx = 0; idx < l_atoms; ++idx, ++atoms) {
        switch(atoms->type) {
        case F_LIT:
            chk_free((char *)atoms->u.s);
            break;
        }
        atoms->type = F_HALT;
    }
}

void
acc_assign_null(void) {
    l_return = 0;
}

void
acc_donate_list(LIST *lp, int length) {
    assert(length == l_length);
    assert(lp == (LIST *)l_returns);
    l_return = lp;
}

static void
split_test1(unsigned test, const char *text, const char *delim, int numeric, int noquoting, int empties, int limit) {
    trace_log("\n\n");
    trace_log("test 1.%u: split('%s','%s', numeric:%u, noquoting:%u, empties:%u, limit:%u)\n",
                test, text, delim, numeric, noquoting, empties, limit);
    arg_clear();
    arg_strings[0]  = text;
    arg_strings[1]  = delim;
    arg_integers[2] = numeric;
    arg_integers[3] = noquoting;
    arg_integers[4] = empties;
    arg_integers[5] = limit;
    ret_clear();
    do_split();
}

static void
split_test2(unsigned test, const char *text, char delim, int numeric, int noquoting, int empties, int limit) {
    trace_log("\n\n");
    trace_log("test 1.%u: split('%s','%c', numeric:%u, noquoting:%u, empties:%u, limit:%u)\n",
                test, text, delim, numeric, noquoting, empties, limit);
    arg_clear();
    arg_strings[0]  = text;
    arg_integers[1] = delim;
    arg_integers[2] = numeric;
    arg_integers[3] = noquoting;
    arg_integers[4] = empties;
    arg_integers[5] = limit;
    ret_clear();
    do_split();
}

static void
tokenize_test1(unsigned test, const char *text, const char *delim, int flags) {
    trace_log("\n\n");
    trace_log("test 2.%u: tokenize('%s','%s',%d)\n", test, text, delim, flags);
    arg_clear();
    arg_strings[0]  = text;
    arg_strings[1]  = delim;
    arg_integers[2] = flags;
    arg_integers[5] = 0;
    ret_clear();
    do_tokenize();
}

static void
tokenize_test2(unsigned test, const char *text, char delim, int flags) {
    trace_log("\n\n");
    trace_log("test (%d) tokenize('%s','%c',%d)\n", test, text, delim, flags);
    arg_clear();
    arg_strings[0]  = text;
    arg_integers[1] = delim;
    arg_integers[2] = flags;
    ret_clear();
    do_tokenize();
}

static void
ret_success(int ret) {
    if (0 == ret) {
        printf(" ==> SUCCESS\n");
    } else {
        printf(" ==> FAILURE\n");
    }
}

static void
test_split(void) {
    int rc;

    /* basic */
    split_test1(1, "hello world", " ", 0, 0, 0, 0);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    split_test2(2, "hello world", ' ', 0, 0, 0, 0);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    /* limits */
    split_test2(10, "hello", ' ', 0, 0, 0, 1);
        rc =  ret_test_str(0, "hello");
        ret_success(rc);

    split_test2(11, "hello", ' ', 0, 0, 0, 2);
        rc =  ret_test_str(0, "hello");
        ret_success(rc);

    split_test2(12, "hello world", ' ', 0, 0, 0, 2);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    split_test2(13, "hello world hello world ", ' ', 0, 0, 0, 2);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world hello world ");
        ret_success(rc);

    /* quotes */
    split_test1(20, "\" hello \",\" world \"", "\",", 0, 0, 0, 0);
        rc =  ret_test_str(0, " hello ");
        rc |= ret_test_str(1, " world ");
        ret_success(rc);
}

static void
test_tokenize(void) {
    int rc;

    /* basic */
    tokenize_test1(1, "hello world", " ", 0);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    tokenize_test2(2, "hello world", ' ', 0);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    /* numeric */
    tokenize_test1(10, "hello 1 world", " ", TOK_NUMERIC);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 1);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    tokenize_test1(11, "hello 0x1234 world ", " ", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 0x1234);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    tokenize_test1(12, "hello 0123 world ", " ", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 0123);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    tokenize_test1(13, "hello 0b0101 world ", " ", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 5);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /* quotes */
    tokenize_test1(20, "\" hello \",\" world \"", ",", TOK_DOUBLE_QUOTES);
        rc =  ret_test_str(0, " hello ");
        rc |= ret_test_str(1, " world ");
        ret_success(rc);

    tokenize_test1(21, "' hello ',' world '", ",", TOK_SINGLE_QUOTES);
        rc =  ret_test_str(0, " hello ");
        rc |= ret_test_str(1, " world ");
        ret_success(rc);

    tokenize_test1(22, "\" hello \",\" world \"", "\",", TOK_PRESERVE_QUOTES);
        rc =  ret_test_str(0, "\" hello \"");
        rc |= ret_test_str(1, "\" world \"");
        ret_success(rc);

    tokenize_test1(23, "\" hello \"\" world \"", "\"", TOK_PRESERVE_QUOTES);
        rc =  ret_test_str(0, "\" hello \"");
        rc |= ret_test_str(1, "\" world \"");
        ret_success(rc);

    /* collapse option */
    tokenize_test1(30, " hello ,, world ", ",", TOK_COLLAPSE_MULTIPLE);
        rc =  ret_test_str(0, " hello ");
        rc |= ret_test_str(1, " world ");
        ret_success(rc);

    tokenize_test1(31, " hello  , , world ", ",", TOK_COLLAPSE_MULTIPLE|TOK_TRIM);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_str(1, "world");
        ret_success(rc);

    /* numeric options */
    tokenize_test1(40, "hello,1,world", ",", TOK_NUMERIC);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 1);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /* decimal */
    tokenize_test1(41, "hello,1,world", ",", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 1);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /* hex-decimal */
    tokenize_test1(42, "hello,0x2,world", ",", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 2);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /* octal */
    tokenize_test1(43, "hello,003,world", ",", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 3);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /* binary */
    tokenize_test1(44, "hello,0b100,world", ",", TOK_NUMERIC_STRTOL);
        rc =  ret_test_str(0, "hello");
        rc |= ret_test_int(1, 4);
        rc |= ret_test_str(2, "world");
        ret_success(rc);

    /*strict*/
    tokenize_test1(72, "1234,5678a", ",", TOK_NUMERIC_STRICT);
        rc =  ret_test_int(0, 1234);
        rc |= ret_test_str(1, "5678a");

    /* escape options
     *
     *      51: "\" hello \"","\" world \""
     */
    tokenize_test1(51, "\"\\\" hello \\\"\",\"\\\" world \\\"\"", "\",", TOK_DOUBLE_QUOTES|TOK_BACKSLASHES);
        rc =  ret_test_str(0, "\" hello \"");
        rc |= ret_test_str(1, "\" world \"");
        ret_success(rc);

    tokenize_test1(52, "\\a," "\\b," "\\e," "\\f," "\\n," "\\r," "\\t," \
                            "\\\'," "\\\"," "\\\\," "\\?," "\\x," "\\X", ",", TOK_ESCAPE);
        rc =  ret_test_str(0,  "\a");
        rc |= ret_test_str(1,  "\b");
        rc |= ret_test_str(2,  "\x1b");
        rc |= ret_test_str(3,  "\f");
        rc |= ret_test_str(4,  "\n");
        rc |= ret_test_str(5,  "\r");
        rc |= ret_test_str(6,  "\t");
        rc |= ret_test_str(7,  "\'");
        rc |= ret_test_str(8,  "\"");
        rc |= ret_test_str(9,  "\\");
        rc |= ret_test_str(10, "\?");
        rc |= ret_test_str(11, "\\x");          /* bad hex */
        rc |= ret_test_str(12, "\\X");          /* unknown */
        ret_success(rc);

    tokenize_test1(53, "\\x ," "\\X ", ",", TOK_ESCAPEALL);
        rc =  ret_test_str(0, "x ");            /* bad hex, only report 'x' */
        rc |= ret_test_str(1, "X ");            /* unknown escape, only report 'X' */
        ret_success(rc);

    tokenize_test1(54, "\x20,\x20""a,\x20""1", ",", TOK_ESCAPE);
        rc =  ret_test_str(0, " ");
        rc |= ret_test_str(1, " a");
        rc |= ret_test_str(2, " 1");
        ret_success(rc);

    tokenize_test1(55, "\040,\040""a,\040""1", ",", TOK_ESCAPE);
        rc =  ret_test_str(0, " ");
        rc |= ret_test_str(1, " a");
        rc |= ret_test_str(2, " 1");
        ret_success(rc);

    /* quoted strings (ie "" aaa "") */
    tokenize_test1(61, "\"\" aaa \"\",\" bbbb \"", ",", TOK_DOUBLE_QUOTES|TOK_QUOTE_STRINGS);
        rc =  ret_test_str(0, "\" aaa \"");
        rc |= ret_test_str(1, " bbbb ");
        ret_success(rc);

    tokenize_test1(62, "'' aaa '',' bbbb '", ",", TOK_SINGLE_QUOTES|TOK_QUOTE_STRINGS);
        rc =  ret_test_str(0, "' aaa '");
        rc |= ret_test_str(1, " bbbb ");
        ret_success(rc);

    /*
     *  whitespace option
     *      permits leading/trailing whitespace around members
     */
    tokenize_test1(71, " \" aaaa \" , \" bbbb \" ", ",", TOK_DOUBLE_QUOTES|TOK_WHITESPACE);
        rc =  ret_test_str(0, " aaaa ");
        rc |= ret_test_str(1, " bbbb ");
        ret_success(rc);

    tokenize_test1(72, " 1234  , 5678a ", ",", TOK_NUMERIC|TOK_WHITESPACE);
        rc =  ret_test_int(0, 1234);
        rc |= ret_test_int(1, 5678);
        ret_success(rc);

    tokenize_test1(73, " 1234  , 5678a ", ",", TOK_NUMERIC_STRICT|TOK_WHITESPACE);
        rc =  ret_test_int(0, 1234);
        rc |= ret_test_str(1, " 5678a ");
        ret_success(rc);

    /* trim options */
    tokenize_test1(81, " aaaa , bbbb ", ",", TOK_TRIM_LEADING);
        rc =  ret_test_str(0, "aaaa ");
        rc |= ret_test_str(1, "bbbb ");
        ret_success(rc);

    tokenize_test1(82, " aaaa , bbbb ", ",", TOK_TRIM_TRAILING);
        rc =  ret_test_str(0, " aaaa");
        rc |= ret_test_str(1, " bbbb");
        ret_success(rc);

    tokenize_test1(83, " aaaa , bbbb ", ",", TOK_TRIM);
        rc =  ret_test_str(0, "aaaa");
        rc |= ret_test_str(1, "bbbb");
        ret_success(rc);

    tokenize_test1(84, "\" aaaa \",\" bbbb \"", ",", TOK_DOUBLE_QUOTES|TOK_TRIM);
        rc =  ret_test_str(0, " aaaa ");
        rc |= ret_test_str(1, " bbbb ");
        ret_success(rc);

    tokenize_test1(85, "\" aaaa \",\" bbbb \"", ",", TOK_DOUBLE_QUOTES|TOK_TRIM_QUOTED|TOK_TRIM_LEADING);
        rc =  ret_test_str(0, "aaaa ");
        rc |= ret_test_str(1, "bbbb ");
        ret_success(rc);

    tokenize_test1(86, "\" aaaa \",\" bbbb \"", ",", TOK_DOUBLE_QUOTES|TOK_TRIM_QUOTED|TOK_TRIM_TRAILING);
        rc =  ret_test_str(0, " aaaa");
        rc |= ret_test_str(1, " bbbb");
        ret_success(rc);

    tokenize_test1(87, "\" aaaa \",\" bbbb \"", ",", TOK_DOUBLE_QUOTES|TOK_TRIM_QUOTED|TOK_TRIM);
        rc =  ret_test_str(0, "aaaa");
        rc |= ret_test_str(1, "bbbb");
        ret_success(rc);
}


void
sys_abort(void)
{
    abort();
}


#undef  LOCAL_MAIN
#include "word.c"

static void
usage(const char *fmt, ...)
{
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        printf("\n");
    }
    printf("\ntokenisze [-d] [-v]\n");
    exit(3);
}


int
main(int argc, char *argv[]) {
    int c;

    while ((c = getopt(argc, argv, "dvh")) != EOF) {
        switch (c) {
        case 'v':
        case 'd':
            ++x_debug;
            break;
        default:
        case 'h':
            usage(NULL);
            break;
        }
    }
    if (0 != (argc -= optind)) {
        usage("unexpected arguments %s ...", argv[optind]);
    }
    test_split();

    test_tokenize();
    return 0;
}
#endif  /*LOCAL_MAIN*/

/*eof*/

