#include <edidentifier.h>
__CIDENT_RCSID(gr_syntax_c,"$Id: syntax.c,v 1.59 2021/06/10 06:13:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntax.c,v 1.59 2021/06/10 06:13:02 cvsuser Exp $
 * Syntax pre-processor.
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
#include "debug.h"
#include "display.h"
#include "echo.h"                               /* errorf() */
#include "eval.h"                               /* get_str() */
#include "lisp.h"
#include "main.h"
#include "map.h"
#include "spell.h"                              /* spell_check/nextword() */
#include "symbol.h"                             /* argv_assign() */
#include "syntax.h"
#include "tags.h"                               /* tags_check() */

static SyntaxTable_t *          syntax_new(const char *name);
static void                     syntax_delete(SyntaxTable_t *st);
static void                     syntax_default(SyntaxTable_t *st);
static void                     syntax_clear(SyntaxTable_t *st);
static SyntaxWords_t *          syntax_words(SyntaxTable_t *st, int table, int length, int create);

static const LINECHAR *         leading_write(SyntaxTable_t *st, const LINECHAR *start, const LINECHAR *end);

static int                      keyword_table(const char *tablename);
static void                     keyword_push(SyntaxTable_t *st, int table, int length, int total, const char *keywords, int sep);

static int                      style_lookup(const char *name);
static int                      style_getc(int n);
static const char *             style_gets(int n);
static int                      style_once(SyntaxTable_t *st, SyntaxChar_t style, const char *description);
static const char *             style_charset(const char *fmt, unsigned char *tab);

static int                      parse_lines(SyntaxTable_t *st, LINENO lineno, LINENO num);
static lineflags_t              parse_line(SyntaxTable_t *st, LINE_t *l);

static const struct flag {
    const char *    f_name;                     /* name/label */
    int             f_length;
    int             f_enum;

} stylenames[] = {
#define STYLENAME(x,n)  { x, (sizeof(x)-1), n }
    STYLENAME("comment",        SYNT_COMMENT),
    STYLENAME("preprocessor",   SYNT_PREPROCESSOR),
    STYLENAME("quote",          SYNT_QUOTE),
    STYLENAME("character",      SYNT_CHARACTER),
    STYLENAME("string",         SYNT_STRING),
    STYLENAME("literal",        SYNT_LITERAL),
    STYLENAME("html",           SYNT_HTML),
    STYLENAME("bracket",        SYNT_BRACKET),
    STYLENAME("operator",       SYNT_OPERATOR),
    STYLENAME("delimiter",      SYNT_DELIMITER),
    STYLENAME("word",           SYNT_WORD),
    STYLENAME("numeric",        SYNT_NUMERIC),
    STYLENAME("keyword",        SYNT_KEYWORD),
    STYLENAME("fortran",        SYNT_FORTRAN),
    STYLENAME("cstyle",         SYNT_CSTYLE),
    STYLENAME("linecont",       SYNT_LINECONT),
    STYLENAME("linecont",       SYNT_LINEJOIN)
#undef  STYLENAME
    };

#if defined(__cplusplus)
extern "C" {                                    /* MCHAR??? */
#endif
    static int is_ascii(int ch) {
#if defined(HAVE___ISASCII)
    return __isascii(ch);
#elif defined(HAVE_ISASCII)
    return isascii(ch);
#else
    return (ch > 0 && ch <= 0x7f);
#endif
    }
    static int is_blank(int ch) {
#if defined(HAVE___ISBLANK)
        return __isblank((unsigned char)ch));
#elif defined(HAVE_ISBLANK)
        return isblank((unsigned char)ch);
#else
        return (' ' == ch || '\t' == ch || '\f' == ch);
#endif
    }
    static int is_alnum(int ch)     { return isalnum((unsigned char)ch); }
    static int is_alpha(int ch)     { return isalpha((unsigned char)ch); }
    static int is_cntrl(int ch)     { return iscntrl((unsigned char)ch); }
    static int is_csym(int ch)      { return ('_' == ch || isalnum((unsigned char)ch)); }
    static int is_digit(int ch)     { return isdigit((unsigned char)ch); }
    static int is_graph(int ch)     { return isgraph((unsigned char)ch); }
    static int is_lower(int ch)     { return islower((unsigned char)ch); }
    static int is_print(int ch)     { return isprint((unsigned char)ch); }
    static int is_punct(int ch)     { return ispunct((unsigned char)ch); }
    static int is_space(int ch)     { return isspace((unsigned char)ch); }
    static int is_upper(int ch)     { return isupper((unsigned char)ch); }
    static int is_word(int ch)      { return ('_' == ch || '-' == ch || isalnum((unsigned char)ch)); }
    static int is_xdigit(int ch)    { return isxdigit((unsigned char)ch); }

    static const struct {                       /* Character classes */
        const char *name;
        unsigned    namelen;
        int       (*isa)(int);
    } character_classes[] = {
        { "ascii",  5,  is_ascii },             /* ASCII character. */
        { "alnum",  5,  is_alnum  },            /* An alphanumeric (letter or digit). */
        { "alpha",  5,  is_alpha  },            /* A letter. */
        { "blank",  5,  is_blank  },            /* A space or tab character. */
        { "cntrl",  5,  is_cntrl  },            /* A control character. */
        { "csym",   4,  is_csym   },            /* A language symbol. */
        { "digit",  5,  is_digit  },            /* A decimal digit. */
        { "graph",  5,  is_graph  },            /* A character with a visible representation. */
        { "lower",  5,  is_lower  },            /* A lower-case letter. */
        { "print",  5,  is_print  },            /* An alphanumeric (same as alnum). */
        { "punct",  5,  is_punct  },            /* A punctuation character. */
        { "space",  5,  is_space  },            /* A character producing white space in displayed text. */
        { "upper",  5,  is_upper  },            /* An upper-case letter. */
        { "word",   4,  is_word   },            /* A "word" character (alphanumeric plus "_"). */
        { "xdigit", 6,  is_xdigit }             /* A hexadecimal digit. */
        };
#if defined(__cplusplus)
};
#endif

static const struct {
    int             synk;
    const char *    name;
    unsigned        namelen;
    int             attr;
} attr_synk_map[] = {
#define SYNKNAME(x)     x, (sizeof(x)-1)

        { SYNK_PRIMARY,                     SYNKNAME("primary"),            ATTR_KEYWORD },
        { SYNK_FUNCTION,                    SYNKNAME("function"),           ATTR_KEYWORD_FUNCTION },
        { SYNK_EXTENSION,                   SYNKNAME("extension"),          ATTR_KEYWORD_EXTENSION },
        { SYNK_TYPE,                        SYNKNAME("type"),               ATTR_KEYWORD_TYPE },
        { SYNK_STORAGECLASS,                SYNKNAME("storageclass"),       ATTR_KEYWORD_STORAGECLASS },
        { SYNK_DEFINITION,                  SYNKNAME("definition"),         ATTR_KEYWORD_DEFINTION },
        { SYNK_CONDITIONAL,                 SYNKNAME("conditional"),        ATTR_KEYWORD_CONDITIONAL },
        { SYNK_REPEAT,                      SYNKNAME("repeat"),             ATTR_KEYWORD_REPEAT },
        { SYNK_EXCEPTION,                   SYNKNAME("exception"),          ATTR_KEYWORD_EXCEPTION },
        { SYNK_DEBUG,                       SYNKNAME("debug"),              ATTR_KEYWORD_DEBUG },
        { SYNK_LABEL,                       SYNKNAME("label"),              ATTR_KEYWORD_LABEL },
        { SYNK_STRUCTURE,                   SYNKNAME("structure"),          ATTR_KEYWORD_STRUCTURE },
        { SYNK_TYPEDEF,                     SYNKNAME("typedef"),            ATTR_KEYWORD_TYPEDEF },

        { SYNK_CONSTANT,                    SYNKNAME("constant"),           ATTR_CONSTANT },
        { SYNK_OPERATOR,                    SYNKNAME("operator"),           ATTR_OPERATOR },
        { SYNK_BOOLEAN,                     SYNKNAME("boolean"),            ATTR_BOOLEAN },

        { SYNK_PREPROCESSOR,                SYNKNAME("preprocessor"),       ATTR_PREPROCESSOR_KEYWORD },
        { SYNK_PREPROCESSOR_INCLUDE,        SYNKNAME("ppinclude"),          ATTR_PREPROCESSOR_INCLUDE },
        { SYNK_PREPROCESSOR_DEFINE,         SYNKNAME("ppdefine"),           ATTR_PREPROCESSOR_DEFINE },
        { SYNK_PREPROCESSOR_CONDITIONAL,    SYNKNAME("ppconditional"),      ATTR_PREPROCESSOR_CONDITIONAL },

        { SYNK_TODO,                        SYNKNAME("todo"),               ATTR_TODO },
        { SYNK_MARKUP,                      SYNKNAME("markup"),             ATTR_COMMENT_STANDOUT }
#undef SYNKNAME
    };

static TAILQ_HEAD(SyntaxTail, SyntaxTable)      /* table queue */
                                x_syntax_tables;

static SyntaxTable_t *          x_current_table;
static SyntaxTable_t *          x_default_table;

static int                      x_syntaxident;


/*<<GRIEF>>
    Macro: create_syntax - Syntax table creation.

        int
        create_syntax(string table)

    Macro Description:
        The 'create_syntax' primitive creates a new syntax table with the
        name specified by 'table'. If the table already exists, the
        existing table shall be reinitialised.

    Macro Parameters:
        table - Unique syntaxtable name.

    Macro Returns:
        The 'create_syntax' primitive returns the syntax identifier
        associated with the syntax table, otherwise -1 if the syntax
        could not be created.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax
 */
void
do_create_syntax(void)          /* int (string table) */
{
    const char *name = get_str(1);
    SyntaxTable_t *st;

    if (NULL != (st = syntax_lookup(name, FALSE))) {
        syntax_clear(st);                       /* reinitialize */

    } else if (NULL == (st = syntax_new(name))) {
        acc_assign_int(-1);
        return;
    }

    TAILQ_INSERT_HEAD(&x_syntax_tables, st, st_node);
    syntax_default(st);
    syntax_hilite_init(st);
    x_current_table = st;
    acc_assign_int(st->st_ident);               /* table identifier */
}


/*<<GRIEF>>
    Macro: attach_syntax - Attach a syntax to a buffer.

        int
        attach_syntax(int|string syntable)

    Macro Description:
        The 'attach_syntax()' primitive associates the current buffer
        with the syntax table specified by the name 'table'.

        Until another syntax table is associated with the buffer, the
        syntax table 'syntable' shall be used in all operations that
        require a syntax; these include parenthesis matching and
        spell-checks.

    Macro Parameters:
        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Macro Returns:
        The 'attach_syntax()' primitive returns the syntax identifier
        associated with the attached syntax table, otherwise -1 if table
        did not exist.

        On invalid table reference errors the following diagnostics
        message(s) shall be echoed on the command prompt.

>           syntax: table 'xxx' undefined.

>           syntax: no current table.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax
 */
void
do_attach_syntax(void)          /* int (int|string table) */
{
    SyntaxTable_t *st;

    if (isa_undef(1)) {                         /* NULL */
        st = x_default_table;

    } else {
        const char *name = get_xstr(1);
                                                /* "" or "DEFAULT" */
        if (name && (0 == *name || 0 == strcmp(name, "DEFAULT"))) {
            st = x_default_table;

        } else if (NULL == (st = syntax_argument(1, TRUE))) {
            acc_assign_int(-1);
            return;
        }
    }

    if (xf_synhilite) {
        BFSET(curbp, BF_SYNTAX);
    }

    curbp->b_syntax = st;
    syntax_parse(TRUE);
    acc_assign_int(st->st_ident);
}


/*<<GRIEF>>
    Macro: detach_syntax - Detach a syntax from a buffer.

        void
        detach_syntax()

    Macro Description:
        The 'detach_syntax()' primitive removes the associated syntax
        definition from the current buffer.

    Macro Parameters:
        none

    Macro Returns:
        The 'detach_syntax()' primitive returns the syntax identifier
        which was associated with the buffer, otherwise -1 if no
        syntax was associated.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, attach_syntax, detach_syntax, inq_syntax
 */
void
do_detach_syntax(void)          /* void () */
{
    SyntaxTable_t *st = curbp->b_syntax;

    BFCLR(curbp, BF_SYNTAX);
    curbp->b_syntax = NULL;
    acc_assign_int(st ? st->st_ident : -1);
}


/*<<GRIEF>>
    Macro: set_syntax_flags - Set syntax flags.

        int
        set_syntax_flags(int flags, [int|string syntable])

    Macro Description:
        The 'set_syntax_flags()' primitive sets the active flags for
        the specified syntax table.

    Macro Parameters:
        flags - Integer syntax flags, one or more of the
            following flags OR'ed together control the attributes of
            the reference syntax table.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Flags::

(start table)
        [Flag                       [Description                        ]
      ! SYNF_CASEINSENSITIVE        Case insensitive language tokens.
      ! SYNF_FORTRAN                FORTRAN style language.
      ! SYNF_STRING_ONELINE         String definitions don't continue 
                                    over line breaks.
      ! SYNF_LITERAL_NOQUOTES       xxx
      ! SYNF_COMMENTS_LEADINGWS     xxx
      ! SYNF_COMMENTS_TRAILINGWS    xxx
      ! SYNF_COMMENTS_QUOTE         xxx
      ! SYNF_COMMENTS_CSTYLE        C-style comments.
      ! SYNF_PREPROCESSOR_WS        xxx
      ! SYNF_LINECONT_WS            xxx
      ! SYNF_HILITE_WS              Hilite white-space.
      ! SYNF_HILITE_LINECONT        Hilite line continuations.
      ! SYNF_HILITE_PREPROCESSOR    Hilite preprocessor directives.
      ! SYNF_SPELL_WORD             Enable word spell check.
      ! SYNF_SPELL_COMMENT          Enable comment spell check.
(end table)

    Macro Returns:
        The 'set_syntax_flags()' primitive returns the value of the
        resulting flags, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, set_syntax_flags, inq_syntax
 */
void
do_set_syntax_flags(void)       /* int ([int flags], [int|string syntable]) */
{
    const int nflags = get_xinteger(1, 0);
    SyntaxTable_t *st = syntax_argument(2, TRUE);

    if (NULL == st) {
        acc_assign_int(-1);
        return;
    }
    st->st_flags |= (unsigned)nflags;
    acc_assign_int((accint_t) st->st_flags);
}


/*<<GRIEF>>
    Macro: inq_syntax - Retrieve the syntax identifier.

        int
        inq_syntax([int &flags], [int|string syntable])

    Macro Description:
        The 'inq_syntax()' primitive retrieves the syntax identifier
        associated with the specified syntax.

    Macro Parameters:
        flags - Option integer reference, to be populated with the
            active flags of the referenced syntax-table.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Macro Returns:
        The 'inq_syntax()' primitive returns the syntax-table
        identifier, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax, set_syntax_flags, inq_syntax
 */
void
inq_syntax(void)                /* ([int &flags], [int|string syntable]) */
{
    SyntaxTable_t *st = syntax_argument(2, TRUE);

    if (NULL == st) {
        acc_assign_int((accint_t)-1);
        return;
    }
    argv_assign_int(1, (accint_t)st->st_flags);
    acc_assign_int((accint_t)st->st_ident);
}


/*<<GRIEF>>
    Macro: define_keywords - Add keywords to a syntax dictionary.

        void
        define_keywords([int|string] keywords,
                string words|list words, [int length],
                [int flags], [int|string syntable])

    Macro Description:
        The 'define_keywords()' primitive adds a set of keywords to the
        specified dictionary, which shall be color syntax highlighted
        in the associated color with the table specified by 'table'.

    Macro Description:
        keywords - Keyword table identifier, see table below.

        words - List of words, otherwise a string that is the
            concatenation of keywords each being of absolute 'length'
            characters, optionally comma separated (see length).

        length - Length of the keywords. If positive the keyword string
            is assumed to contain non delimited words of all of same
            length. Otherwise if given as a negative value, the
            'keyword' string is assumed to contain comma separated
            values of variable length words.

        flags - Optional control flags.

            ICASE   - Ignore case.
            CASE    - Case sensitive.
            ABBR    - Allow abbreviations 
                        (delimiter xxx[cccxx]) or (xxxx-xxxxx).
            PATTERN - Pattern match.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Keyword Tables::

(start table)
        [Constant                       [Name           [Attribute                  ]
      ! SYNK_PRIMARY                    primary         ATTR_KEYWORD
      ! SYNK_FUNCTION                   function        ATTR_KEYWORD_FUNCTION
      ! SYNK_EXTENSION                  extension       ATTR_KEYWORD_EXTENSION
      ! SYNK_TYPE                       type            ATTR_KEYWORD_TYPE
      ! SYNK_STORAGECLASS               storageclass    ATTR_KEYWORD_STORAGECLASS
      ! SYNK_DEFINITION                 definition      ATTR_KEYWORD_DEFINTION
      ! SYNK_CONDITIONAL                conditional     ATTR_KEYWORD_CONDITIONAL
      ! SYNK_REPEAT                     repeat          ATTR_KEYWORD_REPEAT
      ! SYNK_EXCEPTION                  exception       ATTR_KEYWORD_EXCEPTION
      ! SYNK_DEBUG                      debug           ATTR_KEYWORD_DEBUG
      ! SYNK_LABEL                      label           ATTR_KEYWORD_LABE
      ! SYNK_STRUCTURE                  structure       ATTR_KEYWORD_STRUCTURE
      ! SYNK_TYPEDEF                    typedef         ATTR_KEYWORD_TYPEDEF

      ! SYNK_CONSTANT                   constant        ATTR_CONSTANT
      ! SYNK_OPERATOR                   operator        ATTR_OPERATOR
      ! SYNK_BOOLEAN                    boolean         ATTR_BOOLEAN

      ! SYNK_PREPROCESSOR               preprocessor    ATTR_PREPROCESSOR_KEYWORD
      ! SYNK_PREPROCESSOR_INCLUDE       ppinclude       ATTR_PREPROCESSOR_INCLUDE
      ! SYNK_PREPROCESSOR_DEFINE        ppdefine        ATTR_PREPROCESSOR_DEFINE
      ! SYNK_PREPROCESSOR_CONDITIONAL   ppconditional   ATTR_PREPROCESSOR_CONDITIONAL

      ! SYNK_TODO                       todo            ATTR_TODO
      ! SYNK_MARKUP                     markup          ATTR_COMMENT_STANDOUT
(end table)

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax
 */
void
do_define_keywords(void)        /* ([int|string] keywords, string words|list words, [int length],
                                        [int flags], [int|string syntable]) */
{
    const char *tablename = get_xstr(1);
    const LIST *keywordslp = get_xlist(2);
    const char *keywords = (NULL == keywordslp ? get_str(2) : NULL);
    int length = get_xinteger(3, 0);
//  const int flags = get_xinteger(4, 0);       /*TODO*/
    SyntaxTable_t *st = syntax_argument(5, TRUE);
    int kwlen, table = -1, sep = 0;

    if (NULL == st) return;

    /* table [1] */
    if (tablename) {
        if ((table = keyword_table(tablename)) < 0 || table >= KEYWORD_TABLES) {
            errorf("syntax: table name (%s) unknown.", tablename);
            return;
        }

    } else {
        if ((table = get_xinteger(1, -1)) < 0 || table >= KEYWORD_TABLES) {
            errorf("syntax: table number (%d) illegal.", table);
            return;
        }
    }

    /* keyword data [2] */
    if (keywordslp) {
        const LIST *nextlp;
        const char *word;
                                                /* foreach(word) */
        for (;(nextlp = atom_next(keywordslp)) != keywordslp; keywordslp = nextlp) {
            if (NULL != (word = atom_xstr(keywordslp))) {
                int wordlen = (int)strlen(word);

                word = str_trim(word, &wordlen);
                if (wordlen) {
                    keyword_push(st, table, (int)wordlen, (int)wordlen, word, 0);
                }
            }
        }

    } else {
        if (NULL == keywords || 0 == (kwlen = (int)strlen(keywords))) {
            errorf("syntax: keyword list (%d) empty.", table);
            return;
        }

        if (length) {                           /* length [3] */
            if (length < 0) {
                length = -length;
                sep = 1;
            }
            if (length < 2 || length > KEYWORD_LEN) {
                errorf("syntax: keyword length (%d) not supported.", length);
                return;
            }
        }

        if (0 == length) {                      /* dynamic list, comma separated */
            const char *cursor = keywords;
            int total = 0;

            for (;;) {
                const char *comma = strchr(cursor, ',');
                int newlength = (comma ? (comma - cursor) : (int)strlen(cursor));

                if (newlength != length) {      /* length change */
                    if (length && total) {
                        keyword_push(st, table, length, total, keywords, ',');
                    }
                    length = newlength;
                    keywords = cursor;
                    total = 0;
                }

                if (0 == length && NULL == comma) {
                    break;                      /* nothing more */
                }

                total += length;
                cursor += length;               /* move cursor */
                if (*cursor) {
                    assert(',' == *cursor);
                    ++cursor;
                }
            }

        } else {                                /* fixed style */
            int total = kwlen;

            if ((kwlen + sep) % (length + sep)) {
                errorf("syntax: keyword list (%d,%d) length incorrect.", length, table);
                return;
            }

            if (sep) {
                int i;

                total = length;
                sep = keywords[length];         /* dynamic separator */
                for (i = length; i < kwlen; i += length + 1) {
                    if (keywords[i] != sep) {
                        errorf("syntax: keyword list (%d,%d) missing separator '%c'.", length, table, sep);
                        return;
                    }
                    total += length;
                }
            }

            if (total) {
                keyword_push(st, table, length, total, keywords, sep);
            }
        }
    }

    st->keywords_sorted = -1;
}


/*  Function:           keyword_push
 *      Push the specified keywords into the given syntax table.
 *
 *  Parameters:
 *      st - Syntax instance.
 *      table - Table index.
 *      length - Work length.
 *      total  - Additional storage length needed.
 *      keywords - Keyword list.
 *      sep - Optional separator.
 *
 *  Returns:
 *      nothing
 */
static void
keyword_push(SyntaxTable_t *st, int table, int length, int total, const char *keywords, int sep)
{
    SyntaxWords_t *words = syntax_words(st, table, length, TRUE);
    const char *okwlist;
    char *kwlist;

#define WORDS_UNIT          128                 /* round alloc sizes to 128 bytes */
#define WORDS_ROUND(v)      (((v) + WORDS_UNIT) &~ (WORDS_UNIT-1))

    trace_ilog("keyword_push(table:%d,length:%d,total:%d,sep:%d,keywords:\"%.*s\")\n",
        table, length, total, sep, total + (sep ? total/length : 0), keywords);

    assert(words->w_table   == (unsigned)table);
    assert(words->w_length  == (unsigned)length);
    assert((total % length) == 0);

    if (NULL == (okwlist = words->w_list)) {    /* new table */
        unsigned needed = total + 1;

        needed = WORDS_ROUND(needed);
        if (NULL != (kwlist = chk_alloc(needed))) {
            assert(0 == words->w_storage);
            words->w_storage = needed;
            words->w_used = 0;
            words->w_list = kwlist;
        }

    } else {                                    /* existing table */
        unsigned okwlen = words->w_used,
            needed = okwlen + total + 1;

        assert(okwlist[okwlen] == '\0');
        if (needed >= words->w_storage) {       /* reallocate buffer if required */
            needed = WORDS_ROUND(needed);
            if (NULL != (kwlist = chk_realloc((char *)okwlist, needed))) {
                words->w_storage = needed;
                words->w_list = kwlist;
                kwlist += okwlen;
            }
        } else {
            kwlist = (char *)okwlist + okwlen;
        }
    }

#undef  WORDS_ROUND
#undef  WORDS_UNIT

    if (kwlist) {                               /* append data */
        if (sep) {
            const int kwlen = total + (total/length);
            int i, k;

            for (i = k = 0; k < kwlen; k += length + 1) {
                assert(0 == keywords[length] || sep == keywords[length]);
                memcpy(kwlist + i, (const char *)keywords+k, length);
                trace_ilog("\t%.*s\n", length, keywords+k);
                i += length;
            }
            assert(total == i);
        } else {
            memcpy(kwlist, (const char *)keywords, total);
        }

        assert(total && 0 == (total % length));
        kwlist[total] = '\0';
        words->w_used += total;
        assert(words->w_used < words->w_storage);
        assert(0 == (words->w_used % length));
    }

    acc_assign_int(words->w_used/length);
}


/*<<GRIEF>>
    Macro: syntax_token - Define a syntax token.

        void
        syntax_token(int type,
            [<type1> param1], [<type2> param2], [int|string syntable])

    Macro Description:
        The 'syntax_token()' primitive adds and/or modifies a syntax
        tokeniser element of the table specified by the first
        parameter 'table'. The actual type and number of parameters
        vary according to the second parameter 'type'.

    Macro Parameters:
        type - Table attribute.

        param1 - First parameter.

        param2 - Optional second parameter.

        syntable - Optional syntax-table name or identifier, if
            omitted the current syntax table shall be referenced.

    Table Attributes:

        The following *SYNT* table attribute are available;

        SYNT_COMMENT -          <COMMENT>, <open-string> [, <close>-string>]

             Comment syntax definition, defining either a block comment
             or an end-of-line comment. Block comments are specified as
             token pair, being an <open> and non new-line <close> strings,
             with end-of-line comments being a single <open> token.

        SYNT_CSTYLE -           <CSTYLE>, <character>|<character-set>

        SYNT_PREPROCESSOR -     <PRE-PROCESSOR>, <character-set>

        SYNT_STRING -           <STRING>, <character-set>

        SYNT_LITERAL -          <LITERAL>, <character-set>

        SYNT_LINECONT -         <LINECONT>, <character>

        SYNT_LINEJOIN -         <LINEJOIN>, <character>

        SYNT_QUOTE -            <QUOTE>, <character-set>

        SYNT_CHARACTER -        <CHARACTER>, <character-set>

        SYNT_BRACKET -          <BRACKET>, <open> [, <close>]

        SYNT_HTML -             <HTML>, <open>, <close>

        SYNT_WORD -             <WORD>, <character-set>

                Defines the character-set which represent a single word.

        SYNT_KEYWORD -          <KEYWORD>, <character-set>

                Defines the character-set which represent a single
                keyword.

        SYNT_NUMERIC -          <NUMERIC>, <primary-set> [, <secondary-set>]

        SYNT_OPERATOR -         <OPERATOR>, <character>

        SYNT_DELIMITER -        <DELIMITER>, <character-set>

        SYNT_FORTRAN -          <FORTRAN>, <character-set>, <[left-margin], code [,comment-margin]>

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        create_syntax
 */
void
do_syntax_token(void)           /* int (int type|string type, [arg1], [arg2], [int|string syntable]) */
{
#define ARG1    2
#define ARG2    3

    const char *stylename = get_xstr(1);
    const int style = (stylename ? style_lookup(stylename) : get_xinteger(1, -1));
    SyntaxTable_t *st = syntax_argument(4, TRUE);
    SyntaxChar_t *charmap;
    const char *s1, *s2;
    unsigned char charset[256];
    int c, i;

    if (NULL == st) return;

    charmap = st->syntax_charmap;
    switch (style) {
    case SYNT_COMMENT:          /* <COMMENT>, <open> [, <close>] */
        if (NULL != (s1 = get_xstr(ARG1)) && *s1) {
            if (NULL != (s2 = get_xstr(ARG2)) && *s2 && '\n' != *s2) {
                                                /* Block comments */
                if ((i = st->comment_blk_num) >= COMMENT_MAX) {
                    errorf("syntax: too many block comment styles, max %d.", COMMENT_MAX);
                } else {
                    charmap[(unsigned char) *s1] |= SYNC_COMMENT;
                    st->comment_blk_val[i][0] = chk_salloc(s1);
                    st->comment_blk_len[i][0] = strlen(s1);

                    charmap[(unsigned char) *s2] |= SYNC_COMMENT;
                    st->comment_blk_val[i][1] = chk_salloc(s2);
                    st->comment_blk_len[i][1] = (int)strlen(s2);
                    ++st->comment_blk_num;
                }

            } else {                            /* EOL comments */
                if ((i = st->comment_eol_num) >= COMMENT_MAX) {
                    errorf("syntax: too many end-of-line comment styles, max %d.", COMMENT_MAX);
                } else {
                    charmap[(unsigned char) *s1] |= SYNC_COMMENT;
                    st->comment_eol_val[i] = chk_salloc(s1);
                    st->comment_eol_len[i] = (int)strlen(s1);
                    ++st->comment_eol_num;
                }
            }
            st->styles |= SYNC_COMMENT;
        }
        break;

    case SYNT_CSTYLE:           /* <CSTYLE>, <character>|<character-set> */
        if (NULL != (s1 = get_xstr(ARG1))) {    /* character-set */
            style_charset(s1, st->comment_cstyle_char);

                                                /* character */
        } else if ((c = style_getc(ARG1)) > 0 && c <= 0xff) {
            st->comment_cstyle_char[c] = 1;
        }
        break;

    case SYNT_PREPROCESSOR:     /* <PREPROCESSOR>, <character-set> */
        if ((c = style_getc(ARG1)) > 0)
            if (style_once(st, SYNC_PREPROCESSOR, "PREPROCESSOR")) {
                charmap[c] |= SYNC_PREPROCESSOR;
                st->preprocessor_char = (unsigned char) c;
            }
        break;

    case SYNT_STRING:           /* <STRING>, <character-set> */
        if ((c = style_getc(ARG1)) > 0)
            if (style_once(st, SYNC_STRING, "STRING")) {
                charmap[c] |= SYNC_STRING;
                st->string_char = (unsigned char) c;
            }
        break;

    case SYNT_LITERAL:          /* <LITERAL>, <character-set> */
        if ((c = style_getc(ARG1)) > 0)
            if (style_once(st, SYNC_LITERAL, "LITERAL")) {
                charmap[c] |= SYNC_LITERAL;
                st->literal_char = (unsigned char) c;
            }
        break;

    case SYNT_LINECONT:         /* <LINECONT>, <character-set> */
        if ((c = style_getc(ARG1)) > 0) {
            if (style_once(st, SYNC_LINECONT, "LINECONT")) {
                charmap[c] |= SYNC_LINECONT;
                st->linecont_char = (unsigned char) c;
            }
        }
        break;

#if defined(SYNT_LINEJOIN)
    case SYNT_LINEJOIN:         /* <LINEJOIN>, <character> */
        if ((c = style_getc(ARG1)) > 0) {
            if (style_once(st, SYNC_LINEJOIN, "LINEJOIN")) {
                charmap[c] |= SYNC_LINEJOIN;
            }
        }
        break;
#endif

    case SYNT_QUOTE:            /* <QUOTE>, <character-set> */
        if ((c = style_getc(ARG1)) > 0)
            if (style_once(st, SYNC_QUOTE, "QUOTE")) {
                charmap[c] |= SYNC_QUOTE;
                st->quote_char = (unsigned char) c;
            }
        break;

    case SYNT_CHARACTER:        /* <CHARACTER>, <character-set> */
        if ((c = style_getc(ARG1)) > 0)
            if (style_once(st, SYNC_CHARACTER, "CHARACTER")) {
                charmap[c] |= SYNC_CHARACTER;
                st->char_char = (unsigned char) c;
            }
        break;

    case SYNT_BRACKET:          /* <BRACKET>, <open> [, <close>] */
        if (NULL != (s1 = style_gets(ARG1)) && NULL != (s2 = style_gets(ARG2))) {
            i = (int)strlen(s1);
            if (i != (int)strlen(s2)) {
                errorf("syntax: bracket set does not match.");

            } else {
                while (i-- > 0) {
                    charmap[(unsigned char) s1[i]] |= SYNC_BRACKET_OPEN;
                    charmap[(unsigned char) s2[i]] |= SYNC_BRACKET_CLOSE;
                }
                st->styles |= (SYNC_BRACKET_OPEN | SYNC_BRACKET_CLOSE);
            }
        }
        break;

    case SYNT_HTML:             /* <HTML>, <open>, <close> */
        if (NULL != (s1 = style_gets(ARG1)) && NULL != (s2 = style_gets(ARG2))) {
            i = (int)strlen(s1);
            if (i != (int)strlen(s2)) {
                errorf("syntax: html set does not match.");

            } else {
                while (i-- > 0) {
                    charmap[(unsigned char) s1[i]] |= SYNC_HTML_OPEN;
                    charmap[(unsigned char) s2[i]] |= SYNC_HTML_CLOSE;
                }
                st->styles |= (SYNC_HTML_OPEN | SYNC_HTML_CLOSE);
            }
        }
        break;

    case SYNT_WORD:             /* <WORD>, <primary-set> [, <secondary-set>] */
        if (NULL != (s1 = style_gets(ARG1))) {
            if (style_once(st, SYNC_WORD, "WORD")) {
                /*leading*/
                style_charset(s1, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_WORD;
                    } else {
                        charmap[i] &= ~SYNC_WORD;
                    }
                }

                /*secondary*/
                if (NULL == (s2 = get_xstr(ARG2)) || 0 == *s2)
                    s2 = s1;                    /* optional, apply first set when omitted */
                style_charset(s2, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_WORD2;
                    } else {
                        charmap[i] &= ~SYNC_WORD2;
                    }
                }
            }
        }
        break;

    case SYNT_KEYWORD:          /* <KEYWORD>, <primary-set> [, <secondary-set>] */
        if (NULL != (s1 = style_gets(ARG1))) {
            if (style_once(st, SYNC_KEYWORD, "KEYWORD")) {
                /*leading*/
                style_charset(s1, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_KEYWORD;
                    } else {
                        charmap[i] &= ~SYNC_KEYWORD;
                    }
                }

                /*secondary*/
                if (NULL == (s2 = get_xstr(ARG2)) || 0 == *s2)
                    s2 = s1;                    /* optional, apply first set when omitted */
                style_charset(s2, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_KEYWORD2;
                    } else {
                        charmap[i] &= ~SYNC_KEYWORD2;
                    }
                }
            }
        }
        break;

    case SYNT_NUMERIC:          /* <NUMERIC>, <primary-set> [, <secondary-set>] */
        if (NULL != (s1 = style_gets(ARG1))) {
            if (style_once(st, SYNC_NUMERIC, "NUMERIC")) {
                /*leading*/
                style_charset(s1, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_NUMERIC;
                    } else {
                        charmap[i] &= ~SYNC_NUMERIC;
                    }
                }

                /*secondary*/
                if (NULL == (s2 = get_xstr(ARG2)) || 0 == *s2)
                    s2 = s1;                    /* optional, apply first set when omitted */
                style_charset(s2, charset);
                for (i = 0; i < 256; ++i) {
                    if (charset[i]) {
                        charmap[i] |= SYNC_NUMERIC2;
                    } else {
                        charmap[i] &= ~SYNC_NUMERIC2;
                    }
                }
            }
        }
        break;

    case SYNT_OPERATOR:         /* <OPERATOR>, <character> */
        if (NULL != (s1 = style_gets(ARG1))) {
            if (style_once(st, SYNC_OPERATOR, "OPERATOR")) {
                unsigned char *s = (unsigned char *)s1;

                for (i = 0; i < 256; ++i) {
                    charmap[i] &= ~SYNC_OPERATOR;
                }
                while (*s) {
                    charmap[*s] |= SYNC_OPERATOR;
                    ++s;
                }
            }
        }
        break;

    case SYNT_DELIMITER:        /* <DELIMITER>, <character-set> */
        if (NULL != (s1 = style_gets(ARG1))) {
            if (style_once(st, SYNC_DELIMITER, "DELIMITER")) {
                unsigned char *s = (unsigned char *)s1;

                for (i = 0; i < 256; ++i) {
                    charmap[i] &= ~SYNC_DELIMITER;
                }
                while (*s) {
                    charmap[*s] |= SYNC_DELIMITER;
                    ++s;
                }
            }
        }
        break;

    case SYNT_FORTRAN:          /* <FORTRAN>, <character-set>, <[left-margin], code [,comment-margin]> */
        if (NULL != (s1 = get_xstr(ARG1))) {
            style_charset(s1, st->comment_fixed_char);
            st->styles |= SYNT_FORTRAN;
        }

        if (NULL != (s2 = get_xstr(ARG2)) && *s2) {
            const char *s3 = NULL, *s4 = NULL;

            if (NULL != (s3 = strchr(s2, ',')) && 0 != *++s3) {
                if (NULL != (s4 = strchr(s3, ',')) && 0 != *++s4) {
                    /* left,code,comments */
                    st->comment_fixed_margin[FIXED_LMARGIN]  = atoi(s2);
                    st->comment_fixed_margin[FIXED_RCODE]    = atoi(s3);
                    st->comment_fixed_margin[FIXED_RCOMMENT] = atoi(s4);
                } else {
                    /* code,comments */
                    st->comment_fixed_margin[FIXED_RCODE]    = atoi(s2);
                    st->comment_fixed_margin[FIXED_RCOMMENT] = atoi(s3);
                }
            } else {
                /* code=comments */
                st->comment_fixed_margin[FIXED_RCOMMENT] =
                    st->comment_fixed_margin[FIXED_RCODE] = atoi(s2);
            }
        }
        break;

    default:
        if (stylename) {
            errorf("syntax: unknown syntax style '%s'", stylename);
        } else {
            errorf("syntax: unknown syntax style '%d'", style);
        }
        break;
    }

#undef ARG1
#undef ARG2
}


static int
style_lookup(const char *name)
{
    int length = (int)strlen(name);

    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof(stylenames)/sizeof(stylenames[0])); ++i)
            if (length == stylenames[i].f_length &&
                    0 == str_nicmp(stylenames[i].f_name, name, length)) {
                return stylenames[i].f_enum;
            }
    }
    return -1;
}


static int
style_getc(int n)
{
    int ret;

    if ((ret = get_xcharacter(n)) <= 0 || ret >= 256) {
        errorf("syntax: expected character argument(%d)", n);
        ret = 0;
    }
    return ret;
}


static const char *
style_gets(int n)
{
    const char *ret;

    if (NULL == (ret = get_xstr(n))) {
        errorf("syntax: expected string argument(%d)", n);
    }
    return ret;
}


static int
style_once(SyntaxTable_t *st, SyntaxChar_t style, const char *description)
{
    if (st->styles & style) {
        errorf("syntax: %s, multiple SYNT_%s styles,", st->st_name, description);
        return 0;
    }
    st->styles |= style;
    return 1;
}


static const char *
style_charset(const char *_fmt, unsigned char *charset)
{
    const unsigned char *fmt = (unsigned char *)_fmt;
    const unsigned char *start = fmt;
    unsigned char c, v = 0;
    int lvalue, range;

    c = *fmt++;
    if ('^' == c) {                             /* ^, NOT operator */
        v = 1, c = *fmt++;
    }
    memset(charset, v, 256);
    v = (v ? 0 : 1);                            /* flip value */
    if ('-' == c) {                             /* - or ^- */
        charset['-'] = v, c = *fmt++;
    }

    range = lvalue = 0;
    while (c) {
        if (range) {                            /* right-side */
            /*
             *  Error if the range is not numerically greater than the left side character.
             */
            if (c == '[') {
                errorf("syntax: r-value cannot be character-class/sequence '%s'", start);
                return NULL;
            }

            if (c <= lvalue) {
                errorf("syntax: invalid collating order '%s'", start);
                return NULL;
            }

            while (lvalue <= c) {               /* .. mark range */
                charset[lvalue++] = v;
            }
            --range;

            /*
             *  Allow formats such as [a-c-e] as the letters 'a' through 'e'.
             */
            lvalue = c;

        } else if ('[' == c && *fmt == ':') {
            /*
             *  Character-classes.
             */
            const unsigned char *cc = ++fmt;    /* start of character-class */

            while (0 != (c = *fmt++))
                if (':' == c && ']' == *fmt) {  /* look for closing :] */
                    int i;
                                                /* matching class */
                    for (i = (sizeof(character_classes)/sizeof(character_classes[0]))-1; i >= 0; --i)
                        if (0 == strncmp((const char *)cc, character_classes[i].name, character_classes[i].namelen)) {
                            unsigned idx;

                            for (idx = 0; idx < 256; ++idx)
                                if ((*character_classes[i].isa)(idx)) {
                                    charset[idx] = v;
                                }
                            break;
                        }
                    if (-1 == i) {
                        errorf("syntax: unknown character-class '%.*s'", (int)((fmt - cc)-1), cc);
                        return NULL;
                    }
                    ++fmt;                      /* consume ']' */
                    break;
                }
            if (0 == c) {
                errorf("syntax: unmatched ':]' within '%s'", start);
                return NULL;
            }
            lvalue = 0;                         /* cannot be l-value */

        } else if ('-' == c) {
            /*
             *  `-' is not considered to define a range if the character following is a closing bracket.
             */
            if ('\0' == *fmt) {
                charset[(unsigned char) '-'] = v;
                return (const char *)(++fmt);   /* .. closing */
            }

            if (0 == lvalue) {                  /* missing right-side */
                errorf("syntax: unmatched range '%s", start);
                return NULL;
            }
            ++range;

        } else {
            /*
             *  Clear character.
             */
            charset[c] = v;
            lvalue = c;
        }
        c = *fmt++;                             /* next */
    }

    if (range) {
        errorf("syntax: unexpected end-of-format '%s'", start);
        return NULL;
    }
    return (const char *)fmt;
}


/*
 *  syntax_init ---
 *      Syntax processing run-time initialisation.
 */
void
syntax_init(void)
{
    SyntaxChar_t *charmap;

    TAILQ_INIT(&x_syntax_tables);
    x_current_table = NULL;

    if (NULL == (x_default_table = syntax_new("DEFAULT"))) {
        return;
    }
    charmap = x_default_table->syntax_charmap;
    charmap[(unsigned char) '['] = SYNC_BRACKET_OPEN;  charmap[(unsigned char) ']'] = SYNC_BRACKET_CLOSE;
    charmap[(unsigned char) '('] = SYNC_BRACKET_OPEN;  charmap[(unsigned char) ')'] = SYNC_BRACKET_CLOSE;
    charmap[(unsigned char) '{'] = SYNC_BRACKET_OPEN;  charmap[(unsigned char) '}'] = SYNC_BRACKET_CLOSE;
}


/*
 *  syntax_shutdown ---
 *      Syntax processing cleanup, releasing any run-time resources.
 */
void
syntax_shutdown(void)
{
    SyntaxTable_t *st;

    while (NULL != (st = TAILQ_FIRST(&x_syntax_tables))) {
        syntax_delete(st);
    }
    syntax_delete(x_default_table);
    x_default_table = NULL;
    x_current_table = NULL;
}


/*
 *  table2attr ---
 *      Map a table identifier to its associated screen attribute.
 */
static int
table2attr(int table)
{
    unsigned i;

    for (i = 0; i < (unsigned)(sizeof(attr_synk_map)/sizeof(attr_synk_map[0])); ++i) {
        if (table == attr_synk_map[i].synk) {
            return attr_synk_map[i].attr;
        }
    }
    return -1;
}


static SyntaxTable_t *
syntax_new(const char *name)
{
    SyntaxTable_t *st;
    size_t slen;

    assert(name && name[0]);

    slen = strlen(name);
    if (NULL == (st = (SyntaxTable_t *)
            chk_alloc(sizeof(SyntaxTable_t) + slen + 1))) {
        errorf("syntax: cannot allocation table space.");
        return NULL;
    }
    memset(st, 0, sizeof(SyntaxTable_t));
    strcpy((char *)(st + 1), name);
    st->st_name = (const char *)(st + 1);
    st->st_ident = x_syntaxident++;             /* unique identifier */
    return st;
}


SyntaxTable_t *
syntax_argument(int argi, int err)
{
    assert(argi > 0);
    if (isa_integer(argi)) {                    /* by identifier */
        const int ident = get_xinteger(argi, 0);
        SyntaxTable_t *st;

        for (st = TAILQ_FIRST(&x_syntax_tables); st; st = TAILQ_NEXT(st, st_node)) {
            if (ident == st->st_ident) {
                return st;
            }
        }
        if (err) errorf("syntax: table '%d' undefined.", ident);
        return NULL;
    }
    return syntax_lookup(get_xstr(argi), err);  /* name or NULL */
}


SyntaxTable_t *
syntax_lookup(const char *name, int err)
{
    SyntaxTable_t *st;

    if (NULL == name) {
        st = x_current_table;
    } else {                                    /* table names are case sensitive */
        for (st = TAILQ_FIRST(&x_syntax_tables); st; st = TAILQ_NEXT(st, st_node)) {
            if (0 == strcmp(st->st_name, name)) {
                break;
            }
        }
    }

    if (NULL == st && err) {
        if (NULL == name) {
            errorf("syntax: no current table.");
        } else {
            errorf("syntax: table '%s' undefined.", name);
        }
    }
    return st;
}


static SyntaxWords_t *
syntax_words(SyntaxTable_t *st, int table, int length, int create)
{
    SyntaxKeywords_t *keywords;
    SyntaxWords_t *words;
    unsigned idx;

    assert(table >= 0);
    assert(table < KEYWORD_TABLES);
    assert(length > 0);
    assert(length <= KEYWORD_LEN);

#define KEYWORDS_INIT       12
#define KEYWORDS_SIZE(_x)   (sizeof(SyntaxKeywords_t) + ((_x) * sizeof(SyntaxWords_t)))

    /*
     *  lookup keywords
     */
    if (NULL == (keywords = st->keywords_tables[table])) {
        if (create) {
            if (NULL != (keywords = chk_calloc(KEYWORDS_SIZE(KEYWORDS_INIT), 1))) {
                /*
                 *  create new keywords table and assign first slot/
                 *      initial (12) based on average usage.
                 */
                st->keywords_tables[table] = keywords;
                keywords->kw_used = 1;
                keywords->kw_total = KEYWORDS_INIT;
                idx = 0;
                goto newwords;
            }
        }
        return NULL;
    }

    /*
     *  lookup length
     */
    for (idx = 0; idx < keywords->kw_used; ++idx) {
        words = keywords->kw_words + idx;

        assert(words->w_table == (unsigned)table);
        if ((unsigned)length == words->w_length) {
            return words;
        }
    }

    /*
     *  expand
     */
    if (create) {
        unsigned total;

        if (keywords->kw_used >= (total = keywords->kw_total)) {
            SyntaxKeywords_t *nkeywords;
            unsigned ntotal = (total << 1);     /* double table */

            if (NULL != (nkeywords = chk_realloc(keywords, KEYWORDS_SIZE(ntotal)))) {
                trace_ilog("resized syntax_words(%d) %u => %u\n", table, total, ntotal);
                st->keywords_tables[table] = keywords = nkeywords;
                keywords->kw_total = total = ntotal;
            }
        }

        if (keywords->kw_used < total) {
            idx = keywords->kw_used++;          /* allocate next free slot */
            goto newwords;
        }
    }
    return NULL;

#undef  KEYWORDS_INIT
#undef  KEYWORDS_SIZE

newwords:;
    /*
     *  allocate slot
     */
    assert(idx < keywords->kw_used);
    assert(keywords->kw_used <= keywords->kw_total);

    trace_ilog("new syntax_words(%d, %d)[%d] sizing(%u of %u)\n",
        table, length, idx, keywords->kw_used, keywords->kw_total);

    words = keywords->kw_words + idx;
    words->w_table   = (unsigned) table;
    words->w_attr    = table2attr(table);       /* associated attribute */
    words->w_length  = (unsigned) length;
    words->w_used    = 0;
    words->w_storage = 0;
    words->w_list    = 0;
    return words;
}


static int
writex_flags(SyntaxTable_t *st, const BUFFER_t *bp, const int colour, int flags)
{
    int newflags = 0;

    __CUNUSED(st)
    switch (colour) {
    case ATTR_SPELL:
    case ATTR_TODO:
        break;
    default:
        switch (colour) {
        case ATTR_COMMENT:
            if (SYNF_SPELL_COMMENT & st->st_flags) newflags |= SYNW_SPELL;
            if (SYNW_COMMENT_SPELL & flags) newflags |= SYNW_SPELL;
            if (SYNW_COMMENT_TODO  & flags) newflags |= SYNW_TODO;
            break;
        case ATTR_WORD:
            if (SYNF_SPELL_WORD & st->st_flags) newflags |= SYNW_SPELL;
            if (SYNW_WORD_SPELL & flags) newflags |= SYNW_SPELL;
            if (SYNW_WORD_TODO  & flags) newflags |= SYNW_TODO;
            break;
        }
        newflags |= (flags & (SYNW_SPELL|SYNW_TODO));
        if (! BFTST(bp, BF_SPELL)) {
            newflags &= ~SYNW_SPELL;
        }
        break;
    }
    return newflags;
}


static int
writex_spell(SyntaxTable_t *st, const LINECHAR *word, int wordlen, int colour, int flags)
{
    __CUNUSED(colour)

    if (SYNW_TODO & flags) {
        int attr;                               /* <todo> keyword search, case insensitive */

        if ((attr = syntax_keywordx(st, word, wordlen, SYNK_TODO, SYNK_TODO, 1)) >= 0) {
            assert(ATTR_TODO == attr);
            return attr;
        }
    }

    if (SYNW_SPELL & flags) {                   /* spell check */
        if (spell_check((const char *)word, wordlen) > 0) {
            if (SYNW_TAGS & flags) {
                if (1 == tags_check(/*TODO, scope by filename*/ (const char *)word, wordlen)) {
                    return ATTR_TAG;
                }
            }
            return ATTR_SPELL;
        }
    }

    if (SYNW_TAGS & flags) {                     /* tabdb */
        if (tags_check(/*TODO, scope by filename*/ (const char *)word, wordlen) > 0) {
            return ATTR_TAG;
        }
    }

    return -1;
}


const LINECHAR *
syntax_write(SyntaxTable_t *st, const LINECHAR *start, const LINECHAR *end, int colour)
{
    return syntax_writex(st, start, end, colour, (st->st_active ? st->st_active->sd_writex : 0));
}


const LINECHAR *
syntax_writex(SyntaxTable_t *st, const LINECHAR *start, const LINECHAR *end, int colour, int flags)
{
    const int dotabs = (SYNF_HILITE_WS & st->st_flags) ? TRUE : FALSE;
    const int length = end - start;

    WINDOW_t *wp = curwp;
    BUFFER_t *bp = wp->w_bufp;

    if ((flags = writex_flags(st, bp, colour, flags)) > 0) {
        const LINECHAR *cursor = start, *word;
        const int currentline = (wp->w_disp_line == wp->w_line);
        const int edit = (WFEDIT & wp->w_status);
        int t_column = wp->w_disp_column, t_offset = 0, t_chars = 0;
        int wordlen = 0;

        while (NULL != (word =                  /* check each word */
                (const LINECHAR *)spell_nextword(curbp, (void *)start, length, &wordlen, &t_offset, &t_chars, &t_column))) {
            int t_colour;

            if ((t_colour = writex_spell(st, word, wordlen, colour, flags)) >= 0) {
                if (0 == currentline ||         /* non-active word */
                        0 == edit || !(wp->w_col >= t_column && wp->w_col <= (t_column + t_chars))) {

                    if (cursor < word) {        /* text prior */
                        vtwritehl((void *)cursor, word - cursor, colour, dotabs);
                    }
                                                /* word */
                    vtwritehl((void *)word, wordlen, t_colour, 0);
                    cursor = word + wordlen;
                }
            }
        }

        if (cursor < end) {                     /* remaining line */
            vtwritehl((void *)cursor, end - cursor, colour, dotabs);
        }

    } else {
        vtwritehl((void *)start, length, colour, dotabs);
    }
    return end;
}


static void
syntax_default(SyntaxTable_t *st)
{
    /*
     *  SYNF_COMMENTS_CSTYLE
     */
    style_charset("*", st->comment_cstyle_char);

    /*
     *  SYNF_FORTRAN
     */
    st->comment_fixed_pos = 0;                  /* first column */
    st->comment_fixed_margin[FIXED_LMARGIN]  = 6;
    st->comment_fixed_margin[FIXED_RCODE]    = 72;
    st->comment_fixed_margin[FIXED_RCOMMENT] = 132;
    style_charset("^0-9 \t\r\n", st->comment_fixed_char);
 // st->linejoin_char = '&';                    /*TODO*/

    /*
     *  SYNF_HILITE_LINECONT
     *      default specification (sh, make, c/c++)
     */
    st->linecont_char = '\\';
}


static void
syntax_clear(SyntaxTable_t *st)
{
    const char *name = st->st_name;
    unsigned i, i2;

    if (st != x_default_table) {
        TAILQ_REMOVE(&x_syntax_tables, st, st_node);
    }

    for (i = 0; i < (unsigned)(sizeof(st->st_drivers)/sizeof(st->st_drivers[0])); ++i) {
        SyntaxDriver_t *driver;

        if (NULL != (driver = st->st_drivers[i])) {
            (driver->sd_destroy)(st, driver->sd_instance);
        }
    }

    for (i = 0; i < st->comment_blk_num; ++i) {
        for (i2 = 0; i2 < COMMENT_MAX; ++i2) {
            chk_free((char *)st->comment_blk_val[i][i2]);
        }
    }

    for (i = 0; i < st->comment_eol_num; ++i) {
        chk_free((char *)st->comment_eol_val[i]);
    }

    for (i = 0; i < KEYWORD_TABLES; ++i) {
        SyntaxKeywords_t *keywords;

        if (NULL != (keywords = st->keywords_tables[i])) {
            for (i2 = 0; i2 < keywords->kw_used; ++i2) {
                chk_free((char *)keywords->kw_words[i2].w_list);
            }
            chk_free(keywords);
        }
    }

    memset(st, 0, sizeof(SyntaxTable_t));
    st->st_name = name;
}


static void
syntax_delete(SyntaxTable_t *st)
{
    if (st == x_current_table) {
        x_current_table = NULL;
    }
    syntax_clear(st);
    chk_free(st);
}


/*
 *  keyword_table ---
 *      Map a keyword table-name to its internal index.
 *
 *      Attribute-names and table-names are shared, hence we only need to map
 *      one-to-one between known attributes and tables identifiers.
 */
static int
keyword_table(const char *name)
{
    unsigned namelen, i;
    int attr;

    if (name && (namelen = strlen(name)) > 0) {
        /* local mapping */
        for (i = 0; i < (sizeof(attr_synk_map)/sizeof(attr_synk_map[0])); ++i) {
            if (namelen == attr_synk_map[i].namelen &&
                    0 == str_icmp(name, attr_synk_map[i].name)) {
                return attr_synk_map[i].synk;
            }
        }

        if ((attr = attribute_value(name)) >= 0) {
            for (i = 0; i < (sizeof(attr_synk_map)/sizeof(attr_synk_map[0])); ++i) {
                if (attr == attr_synk_map[i].attr) {
                    return attr_synk_map[i].synk;
                }
            }
        } else {
            /* TODO
             *  attribute_create(name, synk_ident++)
             */
        }
    }
    return -1;
}


static int          keyword_cmplen;
static int          keyword_same;

static int
keyword_stricmp(const void * k1, const void * k2)
{
    int ret = str_nicmp(k1, k2, keyword_cmplen);
    if (0 == ret) {
        ++keyword_same;
    }
    return ret;
}


static int
keyword_strcmp(const void * k1, const void * k2)
{
    int ret = strncmp(k1, k2, keyword_cmplen);
    if (0 == ret) {
        ++keyword_same;
    }
    return ret;
}


static SyntaxTable_t *
syntax_select(void)
{
    register SyntaxTable_t *st;

    if (NULL == (st = curbp->b_syntax)) {
        /*
         *  Default selection, if required
         */
        if (NULL == (st = x_default_table)) {
            return NULL;
        }
    }

    if (-1 == st->keywords_sorted) {
        /*
         *  Sort the keywords/
         *      unless an redraw() is performed during keyword definitions,
         *      this logic shall generally be executed only once or twice
         *      an edit session.
         */
        int icase, table, length;
        unsigned kwlen;
        char *kwlist;

        st->keywords_sorted = 0;                /* count of keyword tables */
        icase = (st->st_flags & SYNF_CASEINSENSITIVE);

        trace_ilog("%s keywords:\n", st->st_name);
        for (table = 0; table < KEYWORD_TABLES; ++table) {
            st->keywords_total[table] = 0;

            for (length = 1; length <= KEYWORD_LEN; ++length) {
                SyntaxWords_t *words = syntax_words(st, table, length, FALSE);

                if (NULL == words) {
                    continue;
                }

                assert(words->w_table == (unsigned)table);
                assert(words->w_length == (unsigned)length);
                assert(words->w_list);
                assert(words->w_used);

                if (NULL != (kwlist = (char *)words->w_list) && (kwlen = words->w_used) > 0) {
                    unsigned i, n, count = kwlen / length;

                    keyword_cmplen = length;    /* sort list */
                    keyword_same = 0;

                    qsort(kwlist, count, length, (icase ? keyword_stricmp : keyword_strcmp));

                    if (keyword_same) {         /* duplicate keywords, compress */
                        char *cursor = kwlist;
                        unsigned remaining = kwlen - length;

                        keyword_same = 0;
                        for (i = 0; i < remaining;) {
                            if (0 == (icase ? keyword_stricmp : keyword_strcmp)(cursor, cursor + length)) {
                                /*
                                 *  remove duplicate word
                                 */
                                remaining -= length;
                                memmove(cursor + length, (const char *)(cursor + (length*2)), remaining - i);
                                --count;
                            } else {
                                cursor += length;
                                i += length;
                            }
                        }
                        words->w_used = kwlen = count * length;
                    }

                    words->w_count = count;
                    st->keywords_total[table] += count;
                    ++st->keywords_sorted;      /* increase count */

                    /* dump word list */
                    trace_ilog("\tkeywords (level=%d, count=%d, len=%d, dups=%d):\n",
                        table, count, kwlen, keyword_same);

                    for (i = n = 0; i < kwlen; i += length) {
                        if (0 == n) {
                            trace_log("\t\t");
                        }
                        trace_log("%*.*s", length, length, kwlist + i);
                        if ((n += length + 2) < 72) {
                            trace_log(", ");
                        } else {
                            trace_log("\n");
                            n = 0;
                        }
                    }

                    if (i && n != 0) {
                        trace_log("\n");
                    }

                } /*defined*/
            } /*length*/
        } /*tables*/
    } /* sorted */

    if (NULL == st->st_active) {
        /*
         *  Select the highest available engine
         *
         *     [type5]      (work-in-progress).
         *      column.
         *     [dfa2]
         *      dfa.
         *      basic.
         */
        SyntaxDriver_t *driver;
        unsigned idx;

        for (idx = (sizeof(st->st_drivers)/sizeof(st->st_drivers[0])); idx-- > 0;)
            if (NULL != (driver = st->st_drivers[idx])) {
                if ((driver->sd_select)(st, driver->sd_instance)) {
                    st->st_active = driver;
                    break;
                }
            }
    }
    return st;
}


/*
 *  syntax_keywordx ---
 *      Lookup a keyword within a specific range.
 */
int
syntax_keywordx(const SyntaxTable_t *st, const LINECHAR *token, int length, int start, int end, int icase)
{
    int (*compare)(const void *s1, const void *s2) =
                    ((icase || (st->st_flags & SYNF_CASEINSENSITIVE)) ? keyword_stricmp : keyword_strcmp);
    int table;

    if (length < 1 || length > KEYWORD_LEN) {
        return -1;
    }

    assert(start >= 0);
    assert(end < KEYWORD_TABLES);

    for (table = start; table <= end; ++table) {
        const SyntaxWords_t *words = syntax_words(((SyntaxTable_t *)st), table, length, FALSE);

        if (words) {
            const char *keywords;

            assert(words->w_table == (unsigned)table);
            assert(words->w_length == (unsigned)length);
            assert(words->w_list);
            assert(words->w_used);
                                                /* binary search sorted words list */
            if (NULL != (keywords = words->w_list)) {
                keyword_cmplen = length;
                if (bsearch(token, keywords, words->w_count, length, compare)) {
                    return words->w_attr;       /* associated attribute */
                }
            }
        }
    }
    return -1;
}


/*
 *  syntax_keyword ---
 *      Lookup a keyword levels.
 */
int
syntax_keyword(const SyntaxTable_t *st, const LINECHAR *token, int length)
{
    return syntax_keywordx(st, token, length, SYNK_PRIMARY, SYNK_BOOLEAN, 0);
}


/*
 *  syntax_preprocessor ---
 *      Lookup PREPROCESSOR keywords if defined otherwise level PRIMARY keywords.
 */
int
syntax_preprocessor(const SyntaxTable_t *st, const LINECHAR *token, int length)
{
    int attr;

    if ((attr = syntax_keywordx(st, token, length,
                    SYNK_PREPROCESSOR, SYNK_PREPROCESSOR_CONDITIONAL, 0)) < 0) {
        if (syntax_keywordx(st, token, length, SYNK_PRIMARY, SYNK_PRIMARY, 0) >= 0) {
            attr = ATTR_PREPROCESSOR_KEYWORD;   /* preprocessor/language keyword */
        } else {
            attr = -1;                          /* general processor */
        }
    }
    return attr;
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
            return (0);
        }
        max_line = curbp->b_syntax_max;
    }

    curbp->b_syntax_min = curbp->b_syntax_max = 0;
    return parse_lines(st, min_line, max_line - min_line);
}


/*
 *  syntax_highlight ---
 *      Write the given line, highlighting the syntax.
 */
void
syntax_highlight(const LINE_t *lp)
{
    SyntaxTable_t *st;

    if (NULL != (st = curbp->b_syntax) && st->st_active) {
        const LINECHAR *cursor, *t_cursor, *end;

        if (NULL != (cursor = ltext(lp))) {
            /*
             *  non-empty lines
             */
            struct SyntaxDriver *driver = st->st_active;

            end = cursor + llength(lp);
            st->st_lsyntax = (L_SYNTAX_MASK & lp->l_uflags);
            t_cursor = leading_write(st, cursor, end);
            if (t_cursor) {
                (driver->sd_write)(st, driver->sd_instance,
                        t_cursor, (unsigned)(t_cursor - cursor), end);
            }
        }
    }
}


/*
 *  leading_write ---
 *      Common handling for leading line conditions.
 */
static const LINECHAR *
leading_write(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end)
{
    const lineflags_t lsyntaxin = (L_SYNTAX_IN & st->st_lsyntax);

    switch (lsyntaxin) {
    case L_IN_COMMENT:
    case L_IN_COMMENT2:         /* line spanning comments */
        if (SYNF_COMMENTS_CSTYLE & st->st_flags) {
            /* skip leading white-space following the convention;
             *
             *   (*
             *    *  Comment block, similar to one this example
             *    *  has been enclosed within.
             *    *)
             */
            const LINECHAR *t_end = cursor;

            while (t_end < end && is_blank(*t_end)) {
                ++t_end;                        /* consume leading white-space */
            }
            if (t_end < end && st->comment_cstyle_char[(unsigned char)*t_end]) {
                cursor = syntax_write(st, cursor, t_end, ATTR_CURRENT);
            }
        }
        return syntax_comment_write(st, (L_IN_COMMENT == lsyntaxin ? 0 : 1), cursor, 0, end);

    case L_IN_STRING:           /* strings */
        return syntax_string_write(st, cursor, 0, end, st->quote_char, st->string_char);

    case L_IN_LITERAL:          /* literals */
        return syntax_string_write(st, cursor, 0, end, 0, st->literal_char);

    case L_IN_CHARACTER:        /* characters */
        return syntax_string_write(st, cursor, 0, end, st->quote_char, st->char_char);

    case L_IN_CONTINUE:
    case L_IN_PREPROC:          /* continuation */
        break;

    default:
        assert(0 == lsyntaxin);
        break;
    }
    return cursor;
}


const LINECHAR *
syntax_comment_write(
    SyntaxTable_t *st, unsigned type, const LINECHAR *cursor, int offset, const LINECHAR *end)
{
    struct SyntaxDriver *driver = st->st_active;
    const LINECHAR *t_cursor = cursor + offset;
    const LINECHAR *t_end;

    if (NULL != (t_end = syntax_comment_end(st, type, t_cursor, end))) {
        end = t_end;
    }

    if (driver->sd_cont) {
        (driver->sd_cont) (st, driver->sd_instance, ATTR_COMMENT, t_cursor, offset, end);
        return end;
    }

    return syntax_write(st, cursor, end, ATTR_COMMENT);
}


const LINECHAR *
syntax_string_write(
    SyntaxTable_t *st, const LINECHAR *cursor, int offset, const LINECHAR *end, char quote, char ch)
{
    struct SyntaxDriver *driver = st->st_active;
    const LINECHAR *t_cursor = cursor + offset;
    const LINECHAR *t_end;

    if (NULL != (t_end = syntax_string_end(st, t_cursor, end, quote, ch))) {
        end = t_end;
    }

    if (driver->sd_cont) {
        (driver->sd_cont) (st, driver->sd_instance, ATTR_STRING, t_cursor, offset, end);
        return end;
    }

    return syntax_write(st, cursor, end, ATTR_STRING);
}


static int
str_ncmp(const char *s1, const char *s2, int len)
{
    return strncmp(s1, s2, len);
}


/*
 *  syntax_comment ---
 *      Determine whether the cursor is at the start of a comment.
 *
 *  Returns:
 *      The type is assigned the derived comment construction plus the
 *      address of the comment within the buffer unless COMMENT_NONE.
 */
CommentStatus_t
syntax_comment(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *begin, const LINECHAR *end, const LINECHAR **endp)
{
    const SyntaxChar_t *charmap = st->syntax_charmap;

    int (*compare)(const char *s1, const char *s2, int len) =
            ((st->st_flags & SYNF_CASEINSENSITIVE) ? str_nicmp : str_ncmp);

    const uint32_t tws = (st->st_flags & SYNF_COMMENTS_TRAILINGWS);
    const uint32_t lws = (st->st_flags & SYNF_COMMENTS_LEADINGWS);
    unsigned i;

    /* block comments */
    for (i = 0; i < st->comment_blk_num; ++i) {
        const char *comment = st->comment_blk_val[i][0];
        unsigned length = st->comment_blk_len[i][0];

        if (comment && *cursor == *comment && (cursor + length <= end))
            if (0 == (*compare)((const char *)cursor, comment, length)) {
                const LINECHAR *t_end
                        = syntax_comment_end(st, i, cursor + length, end);

                if (NULL == t_end) {
                    *endp = end;
                    return COMMENT_OPEN + i;    /* OPEN or OPEN2 */
                }
                *endp = t_end;
                return COMMENT_NORMAL;
            }
    }

    /* end-of-line comments */
    for (i = 0; i < st->comment_eol_num; ++i) {
        const char *comment = st->comment_eol_val[i];
        unsigned length = st->comment_eol_len[i];

        if (comment && *cursor == *comment && (cursor + length <= end))
            if (0 == (*compare)((const char *)cursor, comment, length)) {
                const LINECHAR *t_cursor = cursor + length;
                unsigned char t_ch = (unsigned char) *cursor;

                if (t_cursor < end) {           /* MCHAR??? */
                    if (tws) {
                        if (! is_blank(t_ch)) {
                            continue;           /* not trailing white-space */
                        }
                    } else if ((charmap[t_ch] & SYNC_WORD) &&
                                    (charmap[(unsigned char)*t_cursor] & SYNC_WORD2)) {
                        /*
                         *  Example:
                         *      dnl     autoconf comment.
                         *      dnlx    non an autoconf comment.
                         */
                        continue;               /* part of a larger word */
                    }
                }

                if (begin < cursor) {
                    if (lws) {
                        if (! is_blank(cursor[-1])) {
                            continue;           /* not leading white-space (e.g. perl) */
                        }
                    } else if ((charmap[t_ch] & SYNC_WORD) &&
                                    (charmap[(unsigned char)cursor[-1]] & SYNC_WORD2)) {
                        /*
                         *  example:
                         *      dnl     autoconf comment
                         *      xdnl    non an autoconf comment
                         */
                        continue;               /* part of a larger word */
                    }
                }
                *endp = end;
                return COMMENT_EOL;             /* EOL comment */
            }
    }
    *endp = NULL;
    return COMMENT_NONE;
}


const LINECHAR *
syntax_comment_end(
    SyntaxTable_t *st, unsigned type, const LINECHAR *cursor, const LINECHAR *end)
{
    unsigned int length;
    const char *comment;
    char quote, ch;

    assert(type < st->comment_blk_num);

    if (type >= st->comment_blk_num ||
            0 == (length = st->comment_blk_len[type][1]) ||
            NULL == (comment = st->comment_blk_val[type][1])) {
        return NULL;
    }

    quote = (char)((st->st_flags & SYNF_COMMENTS_QUOTE) ? st->quote_char : 0);
    ch = *comment;                              /* opening character */

    for (end -= (length - 1); cursor < end;) {
        if (quote && *cursor == quote && (cursor + 1) < end) {
            cursor += 2;
        } else {
            if (*cursor == ch) {                /* MCHAR??? */
                if (0 == strncmp(comment, (const char *)cursor, length)) {
                    return cursor + length;
                }
            }
            ++cursor;
        }
    }
    return NULL;
}


const LINECHAR *
syntax_string_end(
    SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end, const char quote, const char ch)
{
    __CUNUSED(st)

    while (cursor < end) {
        const char nch = *cursor++;

        if (quote && quote == nch && cursor < end) {
            ++cursor;

        } else if (ch == nch) {
            return cursor;
        }
    }
    return NULL;
}


/*
 *  parse_lines ---
 *      Parse the specified line block, determining the arena in which
 *      hilite is dirty due to content change.
 */
static int
parse_lines(SyntaxTable_t *st, LINENO lineno, LINENO num)
{
    register LINE_t *lp;

    if (lineno > 1) {
        --lineno, ++num;
    }

    ED_TRACE(("syntax::parse_lines(flags:0x%04x, lineno:%d, num:%d, numline:%d)\n", \
        st->st_flags, lineno, num, (int)curbp->b_numlines))

    lp = vm_lock_line2(lineno);
    while (lp) {                                /* parse current line */
        lineflags_t state = parse_line(st, lp);

        liflagclr(lp, LI_DIRTY);
        lp->l_uflags &= ~L_HAS_EOL_COMMENT;

        switch (state) {
        case L_IN_COMMENT:
        case L_IN_COMMENT2:
            break;

        case L_IN_STRING:
        case L_IN_LITERAL:
        case L_IN_CHARACTER:
            if (st->st_flags & SYNF_STRING_ONELINE) {
                state = 0;                      /* fuse string at EOL */
            }
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

        } else if (state == (lp->l_uflags & L_SYNTAX_MASK)) {
            break;                              /* state in-sync, no need to continue */
        }

        /* update status */
        lp->l_uflags &= ~L_SYNTAX_MASK;
        lp->l_uflags |= state;
    }
    return lineno;
}


static lineflags_t
parse_line(SyntaxTable_t *st, LINE_t *line)
{
    const uint32_t flags = st->st_flags;
    const lineflags_t lsyntax = (line->l_uflags & L_SYNTAX_MASK);
    const SyntaxChar_t *charmap = st->syntax_charmap;
    const LINECHAR *cursor, *begin, *end;
    unsigned char schar, ichar, cchar, quote;

    /* Retrieve line buffer and length */
    if (NULL == (cursor = begin = ltext(line))) {
        return lsyntax;
    }
    end = begin + llength(line);
    if (end == begin) {
        return lsyntax;
    }

    ED_TRACE2(("\t<%.*s>\n", end - cursor, cursor))

    /* fixed style comments (aka FORTRAN, plus also COBOL) */
    if (SYNF_FORTRAN & flags) {
        if (cursor + st->comment_fixed_pos < end) {
            unsigned char ch = cursor[st->comment_fixed_pos];

            if (st->comment_fixed_char[ch]) {
                return L_HAS_EOL_COMMENT;
            }
        }
    }

    /* cache string/comment delimiters */
    schar = st->string_char;
    cchar = st->char_char;
    ichar = st->literal_char;
    quote = st->quote_char;

    /* terminate current comment or string */
    if (lsyntax) {
        const lineflags_t lsyntaxin = (lsyntax & L_SYNTAX_IN);

        switch (lsyntaxin) {
        case L_IN_COMMENT:
            if (NULL == (cursor = syntax_comment_end(st, 0, cursor, end))) {
                return L_IN_COMMENT;
            }
            break;

        case L_IN_COMMENT2:
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
                unsigned char lchar;

                if (0 != (lchar = st->linecont_char)) {
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

    /* now we should be out of comments, strings, etc... */
    while (cursor < end) {                      /* MCHAR??? */
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
            cursor = syntax_string_end(st, ocursor + 1, end, quote, ch);
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
            cursor = syntax_string_end(st, ocursor + 1, end, (char)(SYNF_LITERAL_NOQUOTES & flags ? 0 : quote), ch);
            if (NULL == cursor) {
                return L_IN_LITERAL;
            }
            continue;
        }

        if (charmap[ch] & SYNC_COMMENT) {       /* comments */
            const LINECHAR *t_end;

            switch (syntax_comment(st, cursor, begin, end, &t_end)) {
            case COMMENT_NORMAL:
                cursor = t_end;
                break;

            case COMMENT_EOL:
                return L_HAS_EOL_COMMENT;

            case COMMENT_OPEN:
                return L_IN_COMMENT;

            case COMMENT_OPEN2:
                return L_IN_COMMENT2;

            case COMMENT_NONE:
            default:
                ++cursor;
            }
            continue;
        }
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
