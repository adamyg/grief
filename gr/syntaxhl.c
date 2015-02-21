#include <edidentifier.h>
__CIDENT_RCSID(gr_syntaxhl_c,"$Id: syntaxhl.c,v 1.31 2015/02/21 22:47:27 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntaxhl.c,v 1.31 2015/02/21 22:47:27 ayoung Exp $
 * Basic syntax highlighting.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "buffer.h"
#include "color.h"
#include "debug.h"
#include "syntax.h"

static int                  hilite_select(SyntaxTable_t *st, void *object);
static int                  hilite_write(SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end);
static void                 hilite_destroy(SyntaxTable_t *st, void *object);

static const LINECHAR *     hilite_word(SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end);
static const LINECHAR *     hilite_keyword(SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end);
static const LINECHAR *     hilite_numeric(SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end);

static __CINLINE int        is_white(char c);


void
syntax_hilite_init(SyntaxTable_t *st)
{
    SyntaxDriver_t *hilite;

    if (NULL == (hilite = chk_calloc(sizeof(SyntaxDriver_t), 1))) {
        return;
    }
    hilite->sd_instance     = hilite;
    hilite->sd_select       = hilite_select;
    hilite->sd_write        = hilite_write;
    hilite->sd_destroy      = hilite_destroy;
    st->st_drivers[SYNI_BASIC] = hilite;
}


static int
hilite_select(SyntaxTable_t *st, void *object)
{
    __CUNUSED(st)
    __CUNUSED(object)
    return 1;
}


static int
trailing(uint32_t flags, const LINECHAR *cursor, const LINECHAR *end, unsigned char ch, unsigned sz)
{
    if (SYNF_MANDOC & flags) {
        cursor += sz;
        if ((cursor + 1) < end) {               /* X\bX, mandoc */
            if ('\b' == cursor[0] && ch == cursor[1]) {
                return sz + 2;
            }
        }
    }
    return sz;
}


static int
hilite_write(
    SyntaxTable_t *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end)
{
    const uint32_t flags = st->st_flags;
    const SyntaxChar_t *charmap = st->syntax_charmap;
    size_t length = (end - cursor) + offset;    /* true line length */
    const LINECHAR *begin = cursor - offset, *t_cursor;
    unsigned flength = 0;

    __CUNUSED(object)

    /* start of line */
    if (0 == offset) {
        /* fixed (aka FORTRAN) comments */
        if (SYNF_FORTRAN & flags) {
            /*
             *  TODO: SYNC_LINEJOIN
             */
            const unsigned lmargin = st->comment_fixed_margin[FIXED_LMARGIN];

            if (cursor + st->comment_fixed_pos < end) {
                unsigned char ch = cursor[st->comment_fixed_pos];

                if (st->comment_fixed_char[ch]) {
                    const unsigned rcomment = st->comment_fixed_margin[FIXED_RCOMMENT];
                    unsigned olength = 0;       /* overflow length */

                    if (rcomment > 0) {         /* .. greater then comment right margin */
                        if (length > rcomment) {
                            olength = length - rcomment;
                            end = begin + rcomment;
                        }
                    }
                    cursor = syntax_write(st, cursor, end, ATTR_COMMENT);
                    if (olength) {
                        syntax_write(st, cursor, end + olength, ATTR_WHITESPACE /*TODO, ATTR_RULER_COLUMN*/);
                    }
                    return (0);
                }
            }

            if (lmargin > 0 && lmargin < 80) {
                if (length <= lmargin) {        /* FORTRAN 1..5 column hiliting */
                    LINECHAR lbuffer[80];

                    memset(lbuffer, ' ', sizeof(lbuffer));
                    memcpy(lbuffer, (const void *)cursor, length);
                    syntax_write(st, lbuffer, lbuffer + lmargin, ATTR_WHITESPACE /*TODO, ATTR_FIXED*/);
                    return (0);
                }
                flength = lmargin;
                cursor = syntax_write(st, cursor, cursor + flength, ATTR_WHITESPACE /*TODO, ATTR_FIXED*/);
            }
        }

        /* preprocessor */
        if (SYNF_PREPROCESSOR_WS & flags) {
            t_cursor = cursor;
            while (t_cursor < end && is_white(*t_cursor)) {
                ++t_cursor;
            }
            if (t_cursor != cursor) {
                cursor = syntax_write(st, cursor, t_cursor, ATTR_CURRENT);
                if (cursor >= end) {
                    return (0);
                }
            }
        }

        if (cursor < end && *cursor == st->preprocessor_char) {
            if (SYNF_HILITE_PREPROCESSOR & flags) {
                syntax_write(st, cursor, end, ATTR_PREPROCESSOR);
                return (0);
            }

            t_cursor = cursor + 1;
            while (t_cursor < end && is_white(*t_cursor)) {
                ++t_cursor;
            }
            while (t_cursor < end && charmap[*((unsigned char *)t_cursor)] != 0) {
                ++t_cursor;
            }
            cursor = syntax_write(st, cursor, t_cursor, ATTR_PREPROCESSOR);
        }
    }

    /* eg. FORTRAN 6..72 column */
    if (SYNF_FORTRAN & flags) {
        const unsigned rcode = st->comment_fixed_margin[FIXED_RCODE];

        if (rcode > 0 && length > rcode) {
            end = cursor + (rcode - flength);
            flength = length - rcode;
        } else {
            flength = 0;
        }
    }

    /* process remaining characters */
    while (cursor < end) {
        unsigned char ch = (unsigned char) *cursor;
/*TODO  iconv->ic_decode(iconv, cursor, end, &t_ch, &wraw); */
        SyntaxChar_t syntax = charmap[ch];
        size_t sz = 1;

        if ('_' == ch && (SYNF_MANDOC & flags)) {
            if ((cursor + 2) < end && '\b' == cursor[1]) {
/*TODO		if (iconv->ic_decode(iconv, cursor, end, &t_ch, &wraw)) {*/
                ch = cursor[2];                 /* _\bX, mandoc */
                syntax = charmap[ch];
                sz = 3;
            }
        }

        if (0 == syntax) {                      /* unclassified */
            t_cursor = cursor + sz;
            while (t_cursor < end &&
                        (0 == charmap[*((unsigned char *)t_cursor)])) {
                ch = *t_cursor++;
                ++sz;
            }
            cursor = syntax_write(st, cursor, cursor + trailing(flags, cursor, end, ch, sz), ATTR_CURRENT);
            continue;
        }

        if (SYNC_COMMENT & syntax) {            /* comments */
            const LINECHAR *t_end;
            CommentStatus_t comment;

            comment = syntax_comment(st, cursor, begin, end, &t_end);
            if (comment) {
                if (COMMENT_EOL == comment) {
                    unsigned olength = 0;       /* overflow length */

                    if (flags & SYNF_FORTRAN) {
                        const int rcomment = st->comment_fixed_margin[FIXED_RCOMMENT];

                        if (flength > 0) {
                            end += flength;     /* ... append trimming length */
                        }

                        if (rcomment > 0) {     /* ... greater then margin 2 */
                            if (end > begin + rcomment) {
                                olength = (end - begin) - rcomment;
                                end = begin + rcomment;
                            }
                        }
                    }
                    cursor = syntax_write(st, cursor, end, ATTR_COMMENT);
                    if (olength) {
                        syntax_write(st, cursor, end + olength, ATTR_PREPROCESSOR /*TODO, ATTR_WHITESPACE|ATTR_FIXED*/);
                    }
                    return 0;
                }
                cursor = syntax_write(st, cursor, t_end, ATTR_COMMENT);
                continue;
            }
        }

        if (SYNC_WORD & syntax) {               /* words */
            if (0 == (syntax & SYNC_NUMERIC)) {
                cursor = hilite_word(st, cursor, end);
                continue;
            }
        }

        if (SYNC_KEYWORD & syntax) {            /* keywords */
            cursor = hilite_keyword(st, cursor, end);
            continue;
        }

        if (SYNC_DELIMITER & syntax) {          /* delimiters */
            cursor = syntax_write(st, cursor, cursor + trailing(flags, cursor, end, ch, sz), ATTR_DELIMITER);
            continue;
        }
                                                /* character/string */
        if ((SYNC_CHARACTER|SYNC_STRING) & syntax) {
            cursor = syntax_string_write(st, cursor, 1, end, st->quote_char, ch /**cursor*/);
            continue;
        }

        if (SYNC_LITERAL & syntax) {            /* character/string literals (non-escaped) */
            cursor = syntax_string_write(st, cursor, 1, end, 0, *cursor);
            continue;
        }

        if (SYNC_NUMERIC & syntax) {            /* numeric (plus special operator handling) */
            cursor = hilite_numeric(st, cursor, end);
            continue;
        }

        if (SYNC_OPERATOR & syntax) {           /* operator (must after be numeric) */
            cursor = syntax_write(st, cursor, cursor + trailing(flags, cursor, end, ch, sz), ATTR_OPERATOR);
            continue;
        }

        if (SYNC_HTML_OPEN & syntax) {          /* HTML */
            t_cursor = cursor;
            while (t_cursor < end) {
                if (charmap[*((unsigned char *)t_cursor)] & SYNC_HTML_CLOSE) {
                    ++t_cursor;
                    break;
                }
                ++t_cursor;
            }
            cursor = syntax_write(st, cursor, t_cursor, ATTR_LINK);
            continue;
        }

        if (SYNC_HTML_CLOSE & syntax) {         /* unmatched close */
            cursor = syntax_write(st, cursor, cursor + sz, ATTR_LINK);
            continue;
        }
                                                /* brackets */
        if ((SYNC_BRACKET_OPEN|SYNC_BRACKET_CLOSE) & syntax) {
            cursor = syntax_write(st, cursor, cursor + trailing(flags, cursor, end, ch, sz), ATTR_DELIMITER);
            continue;
        }
                                                /* hilite white-space after line-cont */
        if ((SYNC_LINECONT & syntax) && (SYNF_HILITE_LINECONT & flags)) {
            t_cursor = cursor + sz;
            if (t_cursor < end) {
                while (t_cursor < end && is_white(*t_cursor)) {
                    ++t_cursor;
                }
                if (t_cursor == end) {          /* hilite trailing white-space */
                    cursor = syntax_write(st, cursor, t_cursor, ATTR_WHITESPACE);
                    continue;
                }
            }
        }

        if (SYNC_QUOTE & syntax) {              /* quoted */
            t_cursor = cursor + 2;
            if (t_cursor < end) {
                cursor = syntax_write(st, cursor, t_cursor, ATTR_CURRENT);
                continue;
            }
        }
                                                /* undefined */
        cursor = syntax_write(st, cursor, cursor + trailing(flags, cursor, end, ch, sz), ATTR_CURRENT);
    }

    /* FORTRAN 72+ or 132+ column */
    if ((SYNF_FORTRAN & flags) && flength > 0) {
        syntax_write(st, cursor, end + flength, ATTR_WHITESPACE /*ATTR_RULER_COLUMN*/);
    }
    return (0);
}


static void
hilite_destroy(
    SyntaxTable_t *st, void *object)
{
    __CUNUSED(st)
    chk_free(object);
}


static const LINECHAR *
hilite_word(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end)
{
    const uint32_t flags = st->st_flags;
    const SyntaxChar_t *charmap = st->syntax_charmap;
    const LINECHAR *t_cursor = cursor;
    int length, colour;

    if (SYNF_MANDOC & flags) {
        const LINECHAR *end2 = end - 1,
                *end3 = end2 - 1;

        while (t_cursor < end) {
            const unsigned char ch = *((unsigned char *)t_cursor);

            if ('_' == ch) {
                if (t_cursor < end3 && '\b' == t_cursor[1]) {
                    if (charmap[(unsigned char)t_cursor[2]] & SYNC_WORD2) {
                        t_cursor += 3;          /* _\bX, mandoc */
                        continue;
                    }
                    break;
                }
            }

            if (charmap[ch] & SYNC_WORD2) {
                if (++t_cursor < end2){
                    if ('\b' == *t_cursor && ch == t_cursor[1]) {
                        t_cursor += 2;          /* X\bX, mandoc */
                    }
                }

            } else {
                break;
            }
        }

    } else {
        while (t_cursor < end &&                /* body */
                (charmap[*((unsigned char *)t_cursor)] & SYNC_WORD2)) {
            ++t_cursor;
        }
    }

    length = (int) (t_cursor - cursor);

    if ((colour = syntax_keyword(st, cursor, length)) < 0) {
        colour = ATTR_WORD;                     /* not keyword, default to WORD */
    }

    return syntax_write(st, cursor, t_cursor, colour);
}


static const LINECHAR *
hilite_keyword(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end)
{
    const SyntaxChar_t *charmap = st->syntax_charmap;
    const LINECHAR *t_cursor = cursor;
    int length, colour;

    while (t_cursor < end && (charmap[*((unsigned char *)t_cursor)] & SYNC_KEYWORD2)) {
        ++t_cursor;
    }
    length = (int) (t_cursor - cursor);
    if ((colour = syntax_keyword(st, cursor, length)) < 0) {
        colour = ATTR_WORD;                     /* not keyword, word */
    }
    return syntax_write(st, cursor, t_cursor, colour);
}


static const LINECHAR *
hilite_numeric(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end)
{
    const SyntaxChar_t *charmap = st->syntax_charmap;
    const LINECHAR *t_cursor = cursor;
    unsigned char ch;

    /*address leading -|+*/
    ch = (unsigned char)*t_cursor++;

    if (charmap[ch] & SYNC_OPERATOR){
        if (t_cursor < end) {
            unsigned char t_ch = *((unsigned char *)t_cursor);

            if (charmap[t_ch] & SYNC_OPERATOR) {
                /*
                 *  Operator (normally '+' or '-'), examples
                 *      --var
                 */
                do {
                    t_ch = (unsigned char)*++t_cursor;
                } while (t_cursor < end && (charmap[t_ch] & SYNC_OPERATOR));
                return syntax_write(st, cursor, t_cursor, ATTR_OPERATOR);

            } else if ((charmap[t_ch] & SYNC_NUMERIC) == 0) {
                /*
                 *  - var;
                 */
                return syntax_write(st, cursor, t_cursor, ATTR_OPERATOR);
            }
        } else {
            /*
             *  +\n
             */
            return syntax_write(st, cursor, end, ATTR_OPERATOR);
        }
    }

    /*numeric*/
    ch = *((unsigned char *)t_cursor);

    while (t_cursor < end && (charmap[ch] & SYNC_NUMERIC2)) {
        if ('-' == ch || '+' == ch) {
            ch = (unsigned char) t_cursor[-1];
            if ('e' != ch && 'E' != ch) {
                break;
            }
        }
        ch = (unsigned char)*++t_cursor;
    }
    return syntax_write(st, cursor, t_cursor, ATTR_NUMBER);
}


static __CINLINE int
is_white(char c)
{
    return (' ' == c || '\t' == c);
}
/*end*/
