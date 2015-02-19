#ifndef GR_M_TOKENIZE_H_INCLUDED
#define GR_M_TOKENIZE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_tokenize_h,"$Id: m_tokenize.h,v 1.9 2014/10/22 02:33:11 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_tokenize.h,v 1.9 2014/10/22 02:33:11 ayoung Exp $
 * STring tokenizer.
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

#include <edsym.h>

__CBEGIN_DECLS

enum {
/*--export--enum--*/
/*
 *  Split numeric forms
 */
    SPLIT_NONUMERIC         = 0,
    SPLIT_NUMERIC           = 1,
    SPLIT_NUMERIC_STRTOL    = 2,
    SPLIT_NUMERIC_STRICT    = 3
/*--end--*/
};


enum {
/*--export--enum--*/
/*
 *  Tokenize flags
 */
        /*  General
        //  
        //  o TOK_COLLAPSE_MULTIPLE
        //      Collapes occurrences of the repeated delimiter characters 
        //      treating them as single delimiter, in other words empty 
        //      elements with the delimited text shall not be returned.
        */
    TOK_COLLAPSE_MULTIPLE   = (1 << 1),

        /*  Numeric field conversion
        //
        //  o TOK_NUMERIC
        //      Fields which begin with a digit are converted into thier
        //      decimal numeric value and returned as integer element rather
        //      than a string.
        //
        //  o TOK_NUMERIC_STROL
        //      Numeric fields are converted within strtol() allowing support
        //      leading base specifications hexidecimal (0x), octal (0) and
        //      binary (0b).
        //
        //  o TOK_NUMERIC_STRICT
        //      Strict conversion of numeric fields where by any invalid
        //      values, for example trailing non-numeric characters, result
        //      in the the field being returned as a string elemment and not
        //      a integer element.
        */
    TOK_NUMERIC             = (1 << 3),
    TOK_NUMERIC_STRTOL      = (1 << 4),
    TOK_NUMERIC_STRICT      = (1 << 5),

        /*  Parsing options
        //
        //  o TOK_WHITESPACE
        //      Allow leading and trailng whitespace around quoted and
        //      numeric element.
        //
        //  o TOK_BACKSLASHES
        //      Allow backslahes to escape the meaning of any delimiter
        //      characters and both single and double.
        //
        //  o TOK_ESCAPE
        //      Enable backslash escape sequence processing.
        //
        //  o TOK_ESCAPEALL
        //      Control the behaviour of TOK_ESCAPE to escape all characters
        //      preceeded with a backslashes, otherwise by default unknown
        //      escape sequences are ignored.
        */
    TOK_WHITESPACE          = (1 << 6),         /* permit leading/trailing whitespace */
    TOK_BACKSLASHES         = (1 << 7),         /* backslashes */
    TOK_ESCAPE              = (1 << 8),         /* escape sequence support */
    TOK_ESCAPEALL           = (1 << 9),         /* escape 'all' characters */

        /*  Quote options
        //
        //  o TOK_DOUBLE_QUOTES
        //      Enables double quote support where all characters enclosed
        //      within a pair of matching quotes are treated as a single
        //      element including any embedded delimiters.
        //
        //  o TOK_DOUBLE_QUOTES
        //      Same as TOK_DOUBLE_QUOTES but enables single quote support.
        //
        //  o TOK_QUOTE_STRINGS
        //      When single or double quoted support is enabled allow the element
        //      is be enclosed within a extra pair of quotes, for example
        //
        //          ""hello world"".
        //
        //  o TOK_PRESERVE_QUOTES
        //      When an element is enclosed in quotes and the quote character
        //      is specified in 'delims' then the returned element shall also be
        //      enclosed within the enountered quotes.
        */
    TOK_DOUBLE_QUOTES       = (1 << 10),        /* "xxxx" */
    TOK_SINGLE_QUOTES       = (1 << 11),        /* 'xxxxx' */
    TOK_QUOTE_STRINGS       = (1 << 12),        /* ""xxxxx"" */
    TOK_PRESERVE_QUOTES     = (1 << 13),

        /*  Result processing options
        //
        //  o TOK_TRIM_LEADING
        //      Remove any leading whitespace from non-quoted string elements. 
        //      Whitespace is defined as any space, tab or newline character unless
        //      they exist within the set of specified delimiters.
        //
        //  o TOK_TRIM_TRAILING
        //      Remove any trailing whitespace from string elements.
        //
        //  o TOK_TRIM
        //      Remove any leading and trailing whitespace characters.
        //
        //  O TOK_TRIM_QUOTED
        //      Apply trim logic to quoted strings.
        */
    TOK_TRIM_LEADING        = (1 << 14),
    TOK_TRIM_TRAILING       = (1 << 15),
    TOK_TRIM                = (TOK_TRIM_LEADING|TOK_TRIM_TRAILING),
    TOK_TRIM_QUOTED         = (1 << 16)
/*--end--*/
};

extern void                 do_split(void);
extern void                 do_tokenize(void);

__CEND_DECLS

#endif /*GR_M_TOKENIZE_H_INCLUDED*/
