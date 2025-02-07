#include <edidentifier.h>
__CIDENT_RCSID(gr_syntaxln_c, "$Id: syntaxln.c,v 1.4 2025/02/07 03:03:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntaxln.c,v 1.4 2025/02/07 03:03:22 cvsuser Exp $
 * Syntax support.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"
#include "color.h"                              /* attribute_value() */
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "hilite.h"
#include "echo.h"                               /* errorf() */
#include "eval.h"                               /* get_str() */
#include "lisp.h"
#include "main.h"
#include "map.h"
#include "spell.h"                              /* spell_check/nextword() */
#include "symbol.h"                             /* argv_assign() */
#include "syntax.h"
#include "tags.h"                               /* tags_check() */
#include "tty.h"                                /* ttrows() */


struct match_element {
    LINENO line;                    /* Line */
    LINENO col;                     /* Column */
    LINENO offset;                  /* Offset within line */
    LINENO line2;                   /* Secondary line */
    LINENO col2;                    /* Secondary column */
    LINENO offset2;                 /* Secondary Offset within line */
    int matched;                    /* Matched status */
};


struct match_tag {
    const LINECHAR *data;           /* Value */
    uint16_t length;                /* length */
};


struct match_stack {
    struct element {
        const LINECHAR *cursor;
        struct match_tag tag;
    } *elements;                    /* Element stack */
    size_t capacity;                /* Capacity, in elements */
    size_t top;                     /* Stack top */
};


struct match_state {
    SyntaxTable_t *st;              /* Current syntaxtable */
    struct match_element start;     /* Start position */
    struct match_element end;       /* Completion position */
    const LINECHAR *text;           /* Current line start */
    const LINECHAR *eol;            /* and end-of-line */
    const LINECHAR *filter;         /* First line filter */
    LINENO lineno;                  /* Current line number */
    struct match_stack stack;       /* Line stack */
    struct match_tag tag;           /* Active tag, level=0 */
    unsigned char open, close;      /* Characters being matched */
    unsigned char tagtype;          /* Matching tags logic: NUL,X=XML or H=HTML */
    unsigned backwards : 1;         /* backwards, otherwise forward */
    unsigned hilites : 2;           /* Hilite results */
    int nesting;                    /* Nesting level */
    int void_nesting;               /* Void nesting level */
    int lines;                      /* Numbers of lines to scan */
};


typedef void (*Parser_t)(const LINECHAR *cursor, void *udata);

static __CINLINE int        is_white(int32_t c);

static int                  find_paired(SyntaxTable_t *st);
static int                  find_nonwhite(struct match_state *ms, int32_t *ch);
static int                  find_elements(struct match_state *ms, int32_t matching, SyntaxChar_t flags);
static void                 find_elements_pairs(const LINECHAR *cursor, void *udata);
static void                 find_elements_tagged(const LINECHAR *cursor, void *udata);
static int                  find_point(struct match_state *ms);

static void                 tag_get(struct match_state *ms, struct match_tag *tag, const LINECHAR *cursor);
static int                  tag_empty(const struct match_tag *tag);
static int                  tag_equal(struct match_state *ms, const struct match_tag *open, const struct match_tag *close);
static int                  tag_isvoid(struct match_state *ms, const struct match_tag *tag);

static int                  parse_lines(SyntaxTable_t *st, LINENO lineno, LINENO num);
static lineflags_t          parse_line(SyntaxTable_t *st, const LINE_t *lp, const LINECHAR *text, const LINECHAR *end, Parser_t parser, void *udata);


void
do_syntax_find()                /* int ([int mode = 0],  [int &line], [int &col], ... */
{
    SyntaxTable_t *st = syntax_current();
    const int mode = get_xinteger(1, 0);        /* find mode */
    int ret = -1;                               /* nothing to match */
        /*
        // -2 = Unknown character.
        // -1 = Nothing to match.
        //  0 = Matched.
        //  1 = Unmatched.
        */

    if (NULL == curbp || NULL == st) {
        ret = -2;                               /* no syntax definition */

    } else if (0 == mode) {
        ret = find_paired(st);
    }

    acc_assign_int(ret);
}


static __CINLINE int
is_white(int32_t c)
{
    return (' ' == c || '\t' == c);
}


static int
find_paired(SyntaxTable_t *st)
{
    const char *ch = get_xstr(6);               /* element */
    int32_t match = (ch ? *((unsigned char *)ch) : 0);
    struct match_state ms = {0};
    int ret = 1;

    /* int (mode = 0, [int &line], [int &col], [int hilites = 1], [int lines = 75], [string element = ""]) */

    ms.st = st;
    ms.hilites = get_xinteger(4, 1);            /* auto hilite results */
    ms.lines = get_xinteger(5, 75);             /* scan lines */
    ms.start.line = *cur_line;
    ms.start.col = *cur_col;
    ms.start.offset = line_offset(ms.start.line, ms.start.col, LOFFSET_NORMAL);

    if (match) {
        ms.nesting = 1;
    } else {
        match = curbp->b_vchar[0];              /* note: resolved by line_offset() */
    }

    if (0 == find_elements(&ms, match, (SyntaxChar_t)-1L)) {
        if (ms.end.line) {                      /* matched, return */
            if (! isa_undef(2)) {
                sym_assign_int(get_symbol(2), (accint_t) ms.end.line);
            }
            if (! isa_undef(3)) {
                sym_assign_int(get_symbol(3), (accint_t) ms.end.col);
            }
            ret = 0;
        }
    } else {
        ret = -2;
    }

    chk_free((void *)ms.stack.elements);

    return ret;
}


/*
 *  syntax_parse ---
 *      Parse the current buffer, updating the syntax status of the requested line.
 */
int
syntax_parse(int all)
{
    SyntaxTable_t *st;
    LINENO min_line, max_line;

    if (NULL == (st = syntax_select())) {
        return 0;
    }

    if (st != curbp->b_syntax) {
        curbp->b_syntax = st;
        all = 1;                                /* new assignment, reset */
    }

    if (all) {                                  /* all of the buffer */
        min_line = 1;
        max_line = curbp->b_numlines;
    } else {                                    /* otherwise modified */
        if ((min_line = curbp->b_syntax_min) <= 0) {
            return 0;
        }
        max_line = curbp->b_syntax_max;
    }

    curbp->b_syntax_min = curbp->b_syntax_max = 0;
    return parse_lines(st, min_line, max_line - min_line);
}


/*
 *  syntax_virtual_cursor ---
 *      Visual cursor update event.
 */
void
syntax_virtual_cursor()
{
    SyntaxTable_t *st;

    if (! BFTST(curbp, BF_SYNTAX_MATCH))
        return;                                 /* disabled? */

    if (NULL != (st = curbp->b_syntax) && st->st_active) {
        int32_t match = (int32_t) curbp->b_vchar[0];
        struct match_state ms = {0};

        ms.st = st;
        ms.lines = ttrows() * 2;                /* ??? */

        ms.start.line = curbp->b_vline;
        ms.start.col = curbp->b_vcol;

        if (is_white(match)) {
            if (find_nonwhite(&ms, &match)) {
                ms.hilites = 2;                 /* characters */
                find_elements(&ms, match, SYNC_BRACKET_OPEN | SYNC_BRACKET_CLOSE);
            }

        } else {
            ms.hilites = 1;                     /* elements */
            ms.start.offset = curbp->b_voffset;
            find_elements(&ms, match, SYNC_BRACKET_OPEN | SYNC_BRACKET_CLOSE);
        }

        chk_free((void *)ms.stack.elements);
    }
}


static LINENO
find_nonwhite(struct match_state *ms, int32_t *match)
{
    LINE_t *lp = NULL;

    if (NULL != (lp = vm_lock_line2(ms->start.line))) {
        const LINECHAR *cp, *text = ltext(lp), *end = text + llength(lp);
        int pos = 1;

        for (cp = text; cp < end;) {
            int32_t t_ch = 0;
            int t_length, t_width =
                    character_decode(pos, cp, end, &t_length, &t_ch, NULL);

            if (is_white(t_ch)) {
                pos += t_width;
                cp += t_length;
                continue;
            }

            if (0 == t_ch || pos < ms->start.col)
                break;

            ms->start.col = pos;
            ms->start.offset = (LINENO)(cp - text);
            *match = t_ch;
            return 1;
        }
    }
    return 0;
}


/*
 *  find_matching ---
 *      Parse the specified line block, matching brackets.
 */
static struct element *
push(struct match_stack *stack, const LINECHAR *cursor)
{
    struct element *elm = stack->elements;

    if (stack->top == stack->capacity) {        /* extend? */
        const size_t capacity = (stack->capacity + 64);

        if (NULL == (elm = chk_realloc((void *)elm, capacity * sizeof(struct element))))
            return NULL;

        stack->capacity = capacity;
        stack->elements = elm;
    }

    assert(stack->top < stack->capacity);
    elm += stack->top++;
    elm->cursor = cursor;
    memset(&elm->tag, 0, sizeof(elm->tag));
    return elm;
}


static const LINECHAR *
unstack_pairs(struct match_state *ms)
{
    size_t e;

    if (ms->backwards) {
        for (e = ms->stack.top; e;) {
            const LINECHAR *cursor = ms->stack.elements[--e].cursor;
            if (ms->open == ms->close) {
                if (ms->nesting)
                    return cursor;
                ms->nesting = 1;

            } else {
                if (*cursor == ms->close) {
                    ++ms->nesting;
                } else {
                    //assert(*cursor == ms->open && ms->nesting);
                    if (--ms->nesting <= 0)
                        return cursor;
                }
            }
        }

    } else {
        const size_t top = ms->stack.top;
        for (e = 0; e != top;) {
            const LINECHAR *cursor = ms->stack.elements[e++].cursor;
            if (ms->open == ms->close) {
                if (ms->nesting)
                    return cursor;
                ms->nesting = 1;

            } else {
                if (*cursor == ms->open) {
                    ++ms->nesting;
                } else {
                    //assert(*cursor == ms->close && ms->nesting);
                    if (--ms->nesting <= 0)
                        return cursor;
                }
            }
        }
    }

    ms->stack.top = 0;
    return NULL;
}


static const LINECHAR *
unstack_tagged(struct match_state *ms)
{
    size_t e;

    if (ms->backwards) {
        for (e = ms->stack.top; e;) {
            const LINECHAR *cursor = ms->stack.elements[--e].cursor;
            if (*cursor == ms->close) {
                ++ms->nesting;
            } else {
                //assert(*cursor == ms->open && ms->nesting);
                if (--ms->nesting <= 0)
                    return cursor;
            }
        }

    } else {
        const size_t top = ms->stack.top;
        for (e = 0; e != top;) {
            const struct element *elm = ms->stack.elements + e++;

            if (elm->cursor[0] == ms->open) {
                if (elm->tag.length && elm->tag.data[0] == '/') {
                    if (0 == --ms->nesting) {   /* </xxxx */
                        ms->end.line2 = ms->lineno;
                        ms->end.offset2 = (LINENO)(elm->cursor - ms->text);
                        ms->end.matched = tag_equal(ms, &ms->tag, &elm->tag);
                    }

                } else {
                    if (ms->void_nesting) {     /* void, ignore */
                        continue;
                    } else if (tag_isvoid(ms, &elm->tag)) {
                        ms->void_nesting = 1;
                    }

                    if (0 == ms->nesting++) {   /* <xxxx */
                        ms->start.line2 = ms->lineno;
                        ms->start.offset2 = (LINENO)((elm->cursor - ms->text) + elm->tag.length);
                        ms->tag = elm->tag;
                    }
                }
                /*scan continues until closing*/

            } else {
                if (1 == elm->tag.length &&
                        ('/' == elm->tag.data[0] || '?' == elm->tag.data[0])) {
                    if (--ms->nesting <= 0) {   /* /> or ?> */
                        ms->end.line2 = ms->lineno;
                        ms->end.offset2 = (LINENO)((elm->cursor - ms->text) - 1);
                        ms->end.matched = 2;
                        return elm->cursor;
                    }

                } else if (ms->void_nesting) {  /* void nesting level */
                    if (0 == --ms->void_nesting)
                        --ms->nesting;
                }

                if (ms->nesting <= 0) {         /* closing > */
                    return elm->cursor;

                } else if (1 == ms->nesting) {  /* > */
                    if ((ms->tag.data + ms->tag.length) == elm->cursor) {
                        ms->start.offset2 = (LINENO)(elm->cursor - ms->text);
                    }
                }
            }
        }
    }

    ms->stack.top = 0;
    return NULL;
}


static void
element_swap(struct match_element *elm)
{
#define ELMSWAP(__a,__b)    { LINENO tmp=__a; __a=__b; __b=tmp; }
    ELMSWAP(elm->line, elm->line2);
    ELMSWAP(elm->col, elm->col2);
    ELMSWAP(elm->offset, elm->offset2);
}


static void
element_order(struct match_element *elm)
{
    if (elm->line2) {
        if (0 == elm->col2)
            elm->col2 = line_column(elm->line2, elm->offset2);
        if (elm->line > elm->line2) {
            element_swap(elm);
        } else if (elm->line == elm->line2 && elm->col > elm->col2) {
            element_swap(elm);
        }
    }
}


static int
find_elements(struct match_state *ms, int32_t match, SyntaxChar_t mask)
{
    SyntaxTable_t *st = ms->st;
    LINENO lineno = ms->start.line, eline = 0;
    Parser_t callback = find_elements_pairs;
    LINE_t *lp = NULL;

    ms->open = 0;

    if (match < 0xff) {
        const SyntaxChar_t charbits = st->syntax_charmap[match] & mask;
        unsigned b;

        if (SYNC_BRACKET_OPEN & charbits) {
            for (b = 0; b < (sizeof(st->bracket_chars)/2); ++b) {
                if (st->bracket_chars[b].open == match) {
                    eline = curbp->b_numlines;
                    ms->open = st->bracket_chars[b].open;
                    ms->close = st->bracket_chars[b].close;
                    ms->backwards = 0;
                    break;
                }
            }

        } else if (SYNC_BRACKET_CLOSE & charbits) {
            for (b = 0; b < (sizeof(st->bracket_chars)/2); ++b) {
                if (st->bracket_chars[b].close == match) {
                    eline = 1;
                    ms->open = st->bracket_chars[b].open;
                    ms->close = st->bracket_chars[b].close;
                    ms->backwards = 1;
                    break;
                }
            }

        } else if ((SYNC_STRING|SYNC_LITERAL) & charbits) {
            const int state = find_point(ms);
            switch (state) {
            case L_IN_STRING:
            case L_IN_LITERAL:
            case L_IN_CHARACTER:
                ms->backwards = 1;
                /*FALLTHRU*/
            case 0:
                ms->open = ms->close = (unsigned char)match;
                break;
            }
        }
    }

    if (0 == ms->open)                          /* non-open/close */
        return -1;

    ms->end.line = 0;
    ms->end.offset = -1;
    if ((SYNF_HTMLTAG|SYNF_XMLTAG) & st->st_flags) {
        ms->tagtype = (SYNF_XMLTAG & st->st_flags ? 'X' : 'H');
        callback = find_elements_tagged;
    }

    if (NULL != (lp = vm_lock_line2(lineno))) {
        if (NULL != (ms->text = ltext(lp))) {
            if ((LINENO)llength(lp) > ms->start.offset) {
                ms->filter = ms->text + ms->start.offset;
            }
        }
    }

    while (lp) {                                /* parse current line */
        if (NULL != (ms->text = ltext(lp))) {
            ms->eol = ms->text + llength(lp);
            ms->lineno = lineno;
            parse_line(st, lp, ms->text, ms->eol, callback, ms);
            if (ms->stack.top) {                /* unstack */
                const LINECHAR *cursor = (ms->tagtype ? unstack_tagged(ms) : unstack_pairs(ms));
                if (cursor) {
                    ms->end.offset = (LINENO)(cursor - ms->text);
                    ms->end.line = lineno;
                    ms->end.col = line_column(lineno, ms->end.offset);
                    vm_unlock(lineno);
                    break;
                }
            }
        }

        vm_unlock(lineno);
        if (ms->nesting <= 0)
            break;                              /* no match, inside comment etc */

        ms->filter = NULL;
        if (0 == ms->lines)
            break;
        --ms->lines;

        if (ms->backwards) {                    /* new line */
            if (lineno-- <= eline || NULL == (lp = lback(lp))) {
                break;
            }
        } else {
            if (lineno++ >= eline || NULL == (lp = lforw(lp))) {
                break;                          /* EOF */
            }
        }
    }

    if (ms->hilites) {                          /* optional, hilite results */
        HILITE_t *open;

        hilite_destroy(curbp, HILITE_SYNTAX_MATCH);

        if (1 == ms->hilites && ms->start.line2) {
            element_order(&ms->start);
            open = hilite_create(curbp, HILITE_SYNTAX_MATCH, -1, ms->start.line, ms->start.col, ms->start.line2, ms->start.col2);
        } else {
            open = hilite_create(curbp, HILITE_SYNTAX_MATCH, -1, ms->start.line, ms->start.col, ms->start.line, ms->start.col);
        }

        if (open && ms->end.line) {
            HILITE_t *close;

            if (ms->end.line2 && ms->end.matched) {
                element_order(&ms->end);
                if (2 == ms->hilites) {
                    close = hilite_create(curbp, HILITE_SYNTAX_MATCH, -1, ms->end.line2, ms->end.col2, ms->end.line2, ms->end.col2);
                } else {
                    close = hilite_create(curbp, HILITE_SYNTAX_MATCH, -1, ms->end.line, ms->end.col, ms->end.line2, ms->end.col2);
                }

            } else {
                close = hilite_create(curbp, HILITE_SYNTAX_MATCH, -1, ms->end.line, ms->end.col, ms->end.line, ms->end.col);
            }

            if (close) {
                close->h_attr = ATTR_HILITE;
                open->h_attr = ATTR_HILITE;
            }
        }
    }

    return 0;
}


static void
find_elements_pairs(const LINECHAR *cursor, void *udata)
{
    struct match_state *ms = (struct match_state *)udata;
    const unsigned char ch = *cursor;

    if (ms->filter) {                           /* line filter */
        if (ms->backwards) {
            if (cursor > ms->filter)
                return;
        } else {
            if (cursor < ms->filter)
                return;
        }
    }

    if (ch == ms->open || ch == ms->close) {    /* '{' or '}' */
        push(&ms->stack, cursor);
    }
}


static void
find_elements_tagged(const LINECHAR *cursor, void *udata)
{
    struct match_state *ms = (struct match_state *)udata;
    const unsigned char ch = *cursor;

    if (ms->filter) {                           /* line filter */
        if (ms->backwards) {
            if (cursor > ms->filter)
                return;
        } else {
            if (cursor < ms->filter)
                return;
        }
    }

    if (ch == ms->open) {
        struct element *elm = push(&ms->stack, cursor);
        if (elm) {
            tag_get(ms, &elm->tag, cursor + 1); /* '<[/] ...' */
        }

    } else if (ch == ms->close) {
        struct element *elm = push(&ms->stack, cursor);
        if (elm && cursor != ms->text) {
            const unsigned char chback = cursor[-1];
            if ('/' == chback ||                /* '/>' */
                    ('?' == chback && 1 == ms->lineno)) { /* '?>' */
                elm->tag.data = cursor - 1;
                elm->tag.length = 1;
            }
        }
    }
}


static int
find_point(struct match_state *ms)
{
    SyntaxTable_t *st = ms->st;
    LINENO lineno = ms->start.line;
    LINE_t *lp;

    if (NULL != (lp = vm_lock_line2(lineno))) {
        if (NULL != (ms->text = ltext(lp))) {
            return parse_line(st, lp, ms->text, ms->text + ms->start.offset, NULL, NULL);
        }
    }
    return -1;
}


#if defined(TODO)
static int
xml_start(int32_t ch)
{
    // https://www.w3.org/TR/xml/#NT-NameStartChar
    if (isalpha(ch) || '_' == ch || ':' == ch)
        return 1;
    if (ch >= 0xC0) {
        if ((ch >= 0xC0 && ch <= 0xD6) ||
            (ch >= 0xD8 && ch <= 0xF6) ||
            (ch >= 0xF8 && ch <= 0x2FF) ||
            (ch >= 0x370 && ch <= 0x37D) ||
            (ch >= 0x37F && ch <= 0x1FFF) ||
            (ch >= 0x200C && ch <= 0x200D) ||
            (ch >= 0x2070 && ch <= 0x218F) ||
            (ch >= 0x2C00 && ch <= 0x2FEF) ||
            (ch >= 0x3001 && ch <= 0xD7FF) ||
            (ch >= 0xF900 && ch <= 0xFDCF) ||
            (ch >= 0xFDF0 && ch <= 0xFFFD))
        return 1;
    }
    return 0;
}


static int
xml_next(unsigned ch)
{
    // https://www.w3.org/TR/xml/#NT-NameStartChar
    if (xml_start(ch))
        return 1;
    if (isdigit(ch) || '-' == ch || '.' == ch || 0xB7 == ch ||
            ((ch >= 0x300 && ch <= 0x36F) ||  (ch >= 0x203F && ch <= 0x2040)) {
        return 1;
    }
    return 0;
}
#endif //TODO


static void
tag_get(struct match_state *ms, struct match_tag *tag, const LINECHAR *cursor)
{
    const LINECHAR ch0 = *cursor;

    tag->length = 0;
    tag->data = NULL;

    switch (ms->tagtype) {
    case 'X':   // XML
        if (isalpha(ch0) || '_' == ch0 || ':' == ch0 ||
                '/' == ch0 /*closure*/ || ('?' == ch0 /*XMLDecl*/ && 1 == ms->lineno)) {
            const LINECHAR *end = ms->eol;

            // letters, digits, hyphens, underscores, and periods.
            tag->length = 1;
            tag->data = cursor++;
            while (cursor < end) {
                const LINECHAR ch = *cursor++;
                if (!(isalnum(ch) || '_' == ch || ':' == ch || '-' == ch || '.' == ch))
                    break;
                ++tag->length;
            }
        }
        break;

    case 'H':   // HTML
        if (isalpha(ch0) ||
                '/' == ch0 /*closure*/ || ('!' == ch0 /*DOCTYPE*/ && 1 == ms->lineno)) {
            const LINECHAR *end = ms->eol;

            // alphanumerics.
            tag->length = 1;
            tag->data = cursor++;
            while (cursor < end) {
                const LINECHAR ch = *cursor++;
                if (!isalnum(ch))
                    break;
                ++tag->length;
            }
        }
        break;
    }
}


static int
tag_empty(const struct match_tag *tag)
{
    return (0 == tag->length);
}


static unsigned char
caseinsensitive(unsigned char c)
{
    return (isupper(c) ? c - 'A' + 'a' : c);
}


static int
tag_equal(struct match_state *ms, const struct match_tag *open, const struct match_tag *close)
{
    const LINECHAR *d1 = open->data, *d2 = close->data;
    uint16_t l1 = open->length, l2 = close->length;

    if (l2 && *d2 == '/') {
        --l2, ++d2;
    }

    switch (ms->tagtype) {
    case 'X':   // XML - case sensitive.
        while (l1 && l2) {
            if (*d1++ != *d2++)
                return 0;
            l1--, l2--;
        }
        break;

    case 'H':   // HTML - case insensitive.
        while (l1 && l2) {
            if (caseinsensitive(*d1++) != caseinsensitive(*d2++))
                return 0;
            l1--, l2--;
        }
        break;
    }
    return (0 == (l1 | l2));
}


static int
tag_isvoid(struct match_state *ms, const struct match_tag *tag)
{
    if (tag->length) {
        SyntaxTable_t *st = ms->st;

        if (st->void_tags &&
                trie_nsearch(st->void_tags, (const char *)tag->data, tag->length)) {
            return 1;
        }
    }
    return 0;
}


/*
 *  parse_lines ---
 *      Parse the specified line block, determining the arena in which hilite is dirty due to content change.
 */
static int
parse_lines(SyntaxTable_t *st, LINENO lineno, LINENO num)
{
    LINE_t *lp;

    if (lineno > 1) {
        --lineno, ++num;
    }

    ED_TRACE(("syntax::parse_lines(flags:0x%04x, lineno:%d, num:%d, numline:%d)\n", \
        st->st_flags, lineno, num, (int)curbp->b_numlines))

    lp = vm_lock_line2(lineno);
    while (lp) {                                /* parse current line */
        const LINECHAR *text = ltext(lp), *end = text + llength(lp);
        lineflags_t state = parse_line(st, lp, text, end, NULL, NULL);

        lp->l_uflags &= ~L_HAS_EOL_COMMENT;

        switch (state) {
        case L_IN_COMMENT:
        case L_IN_COMMENT2:
            break;
        case L_IN_STRING:
        case L_IN_LITERAL:
        case L_IN_CHARACTER:
            if (st->st_flags & SYNF_STRING_ONELINE)
                state = 0;                      /* fuse string at EOL */
            break;
        case L_HAS_EOL_COMMENT:
            lp->l_uflags |= L_HAS_EOL_COMMENT;
            state = 0;
            break;
        }

        /* retrieve next */
        vm_unlock(lineno);
        if (NULL == (lp = lforw(lp))) {         /* NEWLINE */
            break;                              /* EOF */
        }
        ++lineno;                               /* new line */

        /* check range or status */
        if (num > 0) {                          /* required rescan min */
            --num;

        } else if (state == (L_SYNTAX_MASK & lp->l_uflags)) {
            break;                              /* state in-sync, no need to continue */
        }

        /* update status */
        lp->l_uflags &= ~L_SYNTAX_MASK;
        lp->l_uflags |= state;
    }
    return lineno;
}


static lineflags_t
parse_line(SyntaxTable_t *st, const LINE_t *line, const LINECHAR *begin, const LINECHAR *end, Parser_t callback, void *udata)
{
    const uint32_t flags = st->st_flags;
    const unsigned char schar = st->string_char, ichar = st->char_char,
            cchar = st->literal_char, quote = st->quote_char;
    const lineflags_t lsyntax = (L_SYNTAX_MASK & line->l_uflags);
    const SyntaxChar_t *charmap = st->syntax_charmap;
    const LINECHAR *cursor;

    if (NULL == (cursor = begin) || end <= begin)
        return lsyntax;

    ED_TRACE2(("\t<%.*s>\n", end - cursor, cursor))

    /* fixed style comments (aka FORTRAN, plus also COBOL) */
    if (SYNF_FORTRAN & flags) {
        if (cursor + st->comment_fixed_pos < end) {
            const unsigned char ch = cursor[st->comment_fixed_pos];
            if (st->comment_fixed_char[ch]) {
                 return L_HAS_EOL_COMMENT;
            }
        }
    }

    /* terminate current comment or string */
    if (lsyntax) {
        const lineflags_t lsyntaxin = (lsyntax & L_SYNTAX_IN);

        switch (lsyntaxin) {
        case L_IN_COMMENT:                      /* comment, type-1 */
            if (NULL == (cursor = syntax_comment_end(st, 0, cursor, end))) {
                return L_IN_COMMENT;
            }
            break;
        case L_IN_COMMENT2:                     /* comment, type-2 */
            if (NULL == (cursor = syntax_comment_end(st, 1, cursor, end))) {
                return L_IN_COMMENT2;
            }
            break;
        case L_IN_STRING:                       /* string */
            if (NULL == (cursor = syntax_string_end(st, cursor, end, quote, schar))) {
                return L_IN_STRING;
            }
            break;
        case L_IN_LITERAL:                      /* literals, optional quoting */
            if (NULL == (cursor = syntax_string_end(st, cursor, end,
                            (char)(flags & SYNF_LITERAL_NOQUOTES ? 0 : quote), ichar))) {
                return L_IN_LITERAL;
            }
            break;
        case L_IN_CHARACTER:
            if (NULL == (cursor = syntax_string_end(st, cursor, end, quote, cchar))) {
                return L_IN_CHARACTER;
            }
            /*FALLTHRU*/
        case L_IN_CONTINUE:
        case L_IN_PREPROC:
            if (begin < end) {
                const unsigned char lchar = st->linecont_char;

                if (0 != lchar) {
                    if (lchar == end[-1]) {
                        return lsyntax;         /* last character is continuation */

                    } else if (SYNF_LINECONT_WS & flags) {
                        const LINECHAR *back = end; /* scan backward consuming white-space */

                        while (back > begin) {  /* MCHAR??? */
                            const unsigned char ch = (unsigned char) *--back;
                            if (lchar == ch) {
                                return lsyntax; /* trailing continuation */
                            }
                            if (' ' == ch || '\t' == ch) {
                                continue;
                            }
                            break;
                        }
                    }
                }
            }
            break;
        default:
            assert(0 == lsyntaxin);
            break;
        }
    }

    /* outside string/comment */
    while (cursor < end) {                      /* MCHAR??? */

//TODO  if ((t_cursor = mchar_decode_safe(iconv, cursor, end, &t_ch)) > cursor)
        const unsigned char ch = (unsigned char) *cursor;

        if (0 == ch) {                          /* NULs, ignore */
            ++cursor;
            continue;
        }

        if (ch == quote) {                      /* quotes */
            if ((cursor + 1) < end) {
                cursor += 2;
                continue;
            }
            break;
        }

        if (ch == schar || ch == cchar) {       /* string/character */
            const LINECHAR *ocursor = cursor;

            if (callback) {
                callback(cursor, udata);
                cursor = syntax_string_end(st, ocursor + 1, end, quote, ch);
                if (cursor) callback(cursor - 1, udata);
            } else {
                cursor = syntax_string_end(st, ocursor + 1, end, quote, ch);
            }

            if (NULL == cursor) {
                if (0 == (st->st_flags & SYNF_STRING_MATCHED)) {
                    return (ch == schar ? L_IN_STRING : L_IN_CHARACTER);
                }
                cursor = ocursor + 1;           /* normal */
            }
            continue;
        }

        if (ch == ichar) {                      /* literals, optional quoting */
            const LINECHAR *ocursor = cursor;

            if (callback) {
                callback(cursor, udata);
                cursor = syntax_string_end(st, ocursor + 1, end, (char)(SYNF_LITERAL_NOQUOTES & flags ? 0 : quote), ch);
                if (cursor) callback(cursor - 1, udata);
            } else {
                cursor = syntax_string_end(st, ocursor + 1, end, (char)(SYNF_LITERAL_NOQUOTES & flags ? 0 : quote), ch);
            }

            if (NULL == cursor) {
                return L_IN_LITERAL;
            }
            continue;
        }

        if (charmap[ch] & SYNC_COMMENT) {       /* comments */
            const LINECHAR *t_end = NULL;

            switch (syntax_comment(st, cursor, begin, end, &t_end)) {
            case COMMENT_NORMAL:
                cursor = t_end;
                continue; //next character
            case COMMENT_EOL:
                return L_HAS_EOL_COMMENT;
            case COMMENT_OPEN:
                return L_IN_COMMENT;
            case COMMENT_OPEN2:
                return L_IN_COMMENT2;
            case COMMENT_NONE:
            default:
                break;
            }
        }

        if (callback) callback(cursor, udata);
        ++cursor;
    }

    if (begin < end) {
        unsigned char lchar;

        if (0 != (lchar = st->linecont_char)) {
            int iscont = FALSE;

            if (lchar == end[-1]) {
                iscont = TRUE;                  /* last character is continuation */

            } else if (SYNF_LINECONT_WS & flags) {
                cursor = end;                   /* scan backward consuming white-space */
                while (cursor > begin) {        /* MCHAR??? */
                    const unsigned char ch = (unsigned char) *--cursor;

                    if (ch == lchar) {
                        iscont = TRUE;          /* trailing continuation */
                        break;
                    }
                    if (' ' == ch || '\t' == ch) {
                        continue;
                    }
                    break;
                }
            }

            if (iscont) {
                unsigned char pchar;

                if (0 != (pchar = st->preprocessor_char)) {
                    cursor = begin;
                    while (cursor < end) {      /* MCHAR??? */
                        const unsigned char ch = (unsigned char) *cursor++;

                        if (0 == charmap[ch]) {
                            continue;           /* normal character */
                        }
                        if (ch == pchar) {
                            return L_IN_PREPROC;
                        }
                        break;
                    }
                }
                return L_IN_CONTINUE;           /* continuation */
            }
        }
    }
    return 0;
}

/*end*/
