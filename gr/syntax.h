#ifndef GR_SYNTAX_H_INCLUDED
#define GR_SYNTAX_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_syntax_h,"$Id: syntax.h,v 1.35 2022/07/10 13:12:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: syntax.h,v 1.35 2022/07/10 13:12:24 cvsuser Exp $
 * Syntax hiliting constructs.
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

#include <limits.h>
#include <tailqueue.h>
#include <libtrie.h>
#include <edsym.h>

__CBEGIN_DECLS

/*--export--defines--*/
/*
 *  Syntax rules/types
 */
#define SYNT_COMMENT                1
#define SYNT_PREPROCESSOR           2

#define SYNT_QUOTE                  10
#define SYNT_CHARACTER              11
#define SYNT_STRING                 12
#define SYNT_LITERAL                13

#define SYNT_HTML                   20
#define SYNT_BRACKET                21

#define SYNT_OPERATOR               30
#define SYNT_DELIMITER              31
#define SYNT_WORD                   40
#define SYNT_NUMERIC                41
#define SYNT_KEYWORD                42

#define SYNT_FORTRAN                100
#define SYNT_CSTYLE                 101
#define SYNT_LINECONT               102
#define SYNT_LINEJOIN               103
/*--end--*/

/*--export--defines--*/
/*
 *  Syntax flags
 *
 *      Flag                        Description
 *  ----------------------------------------------------------------------------
 *      SYNF_CASEINSENSITIVE        Case insensitive language tokens.
 *      SYNF_FORTRAN                FORTRAN style language.
 *      SYNF_STRING_ONELINE         String definitions don't continue over line breaks.
 *      SYNF_LITERAL_NOQUOTES       Literal strings don't translate quoted characters.
 *      SYNF_STRING_MATCHED         String open/close must be matched; otherwise ignored.
 *
 *      SYNF_COMMENTS_LEADINGWS     xxx
 *      SYNF_COMMENTS_TRAILINGWS    xxx
 *      SYNF_COMMENTS_QUOTE         xxx
 *      SYNF_COMMENTS_CSTYLE        C-style comments.
 *
 *      SYNF_PREPROCESSOR_WS        xxx
 *      SYNF_LINECONT_WS            xxx
 *      SYNF_MANDOC                 xxx
 *
 *      SYNF_HILITE_WS              Hilite white-space.
 *      SYNF_HILITE_LINECONT        Hilite line continuations.
 *      SYNF_HILITE_PREPROCESSOR    Hilite preprocessor directives.
 *
 *      SYNF_SPELL_WORD             Enable word spell check.
 *      SYNF_SPELL_COMMENT          Enable comment spell check.
 *
 */
#define SYNF_CASEINSENSITIVE        0x0001
#define SYNF_FORTRAN                0x0002
#define SYNF_STRING_ONELINE         0x0004
#define SYNF_LITERAL_NOQUOTES       0x0008
#define SYNF_STRING_MATCHED         0x4000

#define SYNF_COMMENTS_LEADINGWS     0x0010
#define SYNF_COMMENTS_TRAILINGWS    0x0020
#define SYNF_COMMENTS_QUOTE         0x0040
#define SYNF_COMMENTS_CSTYLE        0x0080

#define SYNF_PREPROCESSOR_WS        0x0100
#define SYNF_LINECONT_WS            0x0200
#define SYNF_MANDOC                 0x0400

#define SYNF_HILITE_WS              0x1000
#define SYNF_HILITE_LINECONT        0x2000
#define SYNF_HILITE_PREPROCESSOR    0x0400

#define SYNF_SPELL_WORD             0x1000
#define SYNF_SPELL_COMMENT          0x2000
/*--end--*/

/*--export--defines--*/
/*
 *  Keywords, standard table usage
 *
 *      Attribute                   Description
 *  ----------------------------------------------------------------------------
 *      SYNK_PRIMARY(0)             Language reserved words.
 *      SYNK_FUNCTIONS              Standard definitions, functions etc.
 *      SYNK_EXTENSIONS             Extensions.
 *      SYNK_TYPE                   Types.
 *      SYNK_STORAGECLASS           Storage classes.
 *      SYNK_DEFINITION             Definitions.
 *      SYNK_CONDITIONAL            Conditional statements.
 *      SYNK_REPEAT                 Repeat statements.
 *      SYNK_EXCEPTION              Exception.
 *      SYNK_DEBUG                  Debug statements.
 *      SYNK_LABEL                  Label's.
 *      SYNK_STRUCTURE              Structure definitions.
 *      SYNK_TYPEDEF                Type definitions.
 *      SYNK_CONSTANTS              System constants.
 *      SYNK_OPERATOR               Operators.
 *      SYNK_BOOLEAN                Boolean constants.
 *
 *      SYNK_PREPROCESSOR           Preprocessor primitives.
 *      SYNK_PREPROCESSOR_INCLUDE   Preprocessor #include primitive.
 *      SYNK_PREPROCESSOR_DEFINE    Preprocessor #define primitive.
 *      SYNK_PREPROCESSOR_COND      Preprocessor conditional primitives.
 *
 *      SYNK_TODO                   Magic comment keywords, additional dictionary words.
 *      SYNK_MARKUP                 Comment markups (e.g. doxygen).
 */
enum {
    SYNK_PRIMARY,
    SYNK_FUNCTION,
    SYNK_EXTENSION,
    SYNK_TYPE,
    SYNK_STORAGECLASS,
    SYNK_DEFINITION,
    SYNK_CONDITIONAL,
    SYNK_REPEAT,
    SYNK_EXCEPTION,
    SYNK_DEBUG,
    SYNK_LABEL,
    SYNK_STRUCTURE,
    SYNK_TYPEDEF,
    SYNK_CONSTANT,
    SYNK_OPERATOR,
    SYNK_BOOLEAN,

    SYNK_PREPROCESSOR,
    SYNK_PREPROCESSOR_INCLUDE,
    SYNK_PREPROCESSOR_DEFINE,
    SYNK_PREPROCESSOR_CONDITIONAL,

    SYNK_TODO,
    SYNK_MARKUP,

    SYNK_MAX
};
/*--end--*/

/*--export--defines--*/
/*
 *  Keywords flags.
 *
 *      Flag                        Description
 *  ----------------------------------------------------------------------------
 *      SYNF_IGNORECASE             Ignore case.
 *      SYNK_NATCHCASE              Match case.
 *      SYNF_PATTERN                Pattern match (glob style).
 */
enum {
    SYNF_IGNORECASE         = 1,
    SYNK_NATCHCASE          = 2,
    SYNF_PATTERN            = 4
};
/*--end--*/

typedef uint32_t SyntaxChar_t;

typedef enum {
    SYNI_BASIC              = 0,
    SYNI_REGDFA,
    SYNI_COLUMN,
    SYNI_TYPE5,
    SYNI_MAX,
} SyntaxType_t;

typedef enum {
    SYNW_SPELL              =0x0001,
    SYNW_TODO               =0x0002,
    SYNW_TAGS               =0x0004,
    SYNW_COMMENT_SPELL      =0x0010,
    SYNW_COMMENT_TODO       =0x0020,
    SYNW_WORD_SPELL         =0x0040,
    SYNW_WORD_TODO          =0x0080
} SyntaxFlags_t;

typedef struct SyntaxDriver {
    void *              sd_instance;
    int                 sd_writex;
    int               (*sd_select)(struct SyntaxTable *st, void *object);
    int               (*sd_write)(struct SyntaxTable *st, void *object, const LINECHAR *cursor, unsigned offset, const LINECHAR *end);
    int               (*sd_cont)(struct SyntaxTable *st, void *object, int attr, const LINECHAR *cursor, unsigned offset, const LINECHAR *end);
    void              (*sd_destroy)(struct SyntaxTable *st, void *object);
} SyntaxDriver_t;

typedef struct {
    unsigned            l_count;                /* element count */
    unsigned            l_data_storage;         /* total allocated data storage, in bytes */
    unsigned            l_data_used;            /* used storage, in bytes */
    const char **       l_vector;               /* option vector; optional */
    const char *        l_data;                 /* compat word list */
} SyntaxWordList_t;

typedef struct SyntaxWords {
    MAGIC_t             w_magic;                /* structure magic */
    unsigned            w_table;                /* table identifier */
    vbyte_t             w_attr;                 /* associated attribute */
    unsigned            w_length;               /* word length */
    SyntaxWordList_t    w_words;                /* standard words */
} SyntaxWords_t;

typedef struct {
    MAGIC_t             kw_magic;               /* structure magic */
    unsigned            kw_total;               /* total slots */
    unsigned            kw_used;                /* allocated slots */
    SyntaxWords_t       kw_words[1];            /* slots [0] ... */
} SyntaxKeywords_t;

typedef struct SyntaxTable {
    /*
     *  Name status etc
     */
    MAGIC_t             st_magic;               /* struct magic */
    const char *        st_name;                /* name of this table */
    uint32_t            st_flags;               /* syntax processing flags */
    int32_t             st_ident;               /* syntax table unique identifier/handle */

    SyntaxDriver_t *    st_active;
    SyntaxDriver_t *    st_drivers[SYNI_MAX];

    TAILQ_ENTRY(SyntaxTable)                    /* table queue */
                        st_node;

    lineflags_t         st_lsyntax;             /* current line status */

    /*
     *  Block comment open/close, one is generally required with pascal
     *  being one of the few languages requiring two.
     */
#define COMMENT_MAX         2

    unsigned            comment_blk_num;        /* block comments */
    unsigned            comment_blk_len[COMMENT_MAX][2];
    const char *        comment_blk_val[COMMENT_MAX][2];

    unsigned            comment_eol_num;        /* end-of-line comments */
    unsigned            comment_eol_len[COMMENT_MAX];
    const char *        comment_eol_val[COMMENT_MAX];

    /*
     *  Fixed format column support, for example Fortran and Cobol.
     */
#define FIXED_LMARGIN       0
#define FIXED_RCODE         1
#define FIXED_RCOMMENT      2

    unsigned            comment_fixed_pos;
    unsigned            comment_fixed_margin[3];

    unsigned char       comment_fixed_char[256];
    unsigned char       comment_cstyle_char[256];

    /*
     *  Major lex events and character map
     */
#define SYNC_WORD           0x00000001
#define SYNC_WORD2          0x00000002
#define SYNC_KEYWORD        0x00000004
#define SYNC_KEYWORD2       0x00000008

#define SYNC_NUMERIC        0x00000010
#define SYNC_NUMERIC2       0x00000020

#define SYNC_DELIMITER      0x00000100
#define SYNC_QUOTE          0x00000200
#define SYNC_OPERATOR       0x00000400

#define SYNC_STRING         0x00001000
#define SYNC_LITERAL        0x00002000
#define SYNC_CHARACTER      0x00004000

#define SYNC_PREPROCESSOR   0x00010000
#define SYNC_COMMENT        0x00020000
#define SYNC_LINECONT       0x00040000
#define SYNC_LINEJOIN       0x00080000

#define SYNC_BRACKET_OPEN   0x00100000
#define SYNC_BRACKET_CLOSE  0x00200000
#define SYNC_HTML_OPEN      0x00400000
#define SYNC_HTML_CLOSE     0x00800000

    SyntaxChar_t        styles;

    unsigned char       string_char;
    unsigned char       char_char;
    unsigned char       literal_char;
    unsigned char       quote_char;
    unsigned char       preprocessor_char;
    unsigned char       linecont_char;

    SyntaxChar_t        syntax_charmap[256];

#define KEYWORD_LEN         64                  /* required for posix functions etc */
#define KEYWORD_TABLES      SYNK_MAX

    struct trie *       keyword_patterns;
    int                 keywords_sorted;
    int                 keywords_total[KEYWORD_TABLES];
    SyntaxKeywords_t *  keywords_tables[KEYWORD_TABLES];
} SyntaxTable_t;

typedef enum {
    COMMENT_NONE,
    COMMENT_NORMAL,
    COMMENT_EOL,
    COMMENT_OPEN,
    COMMENT_OPEN2
} CommentStatus_t;

extern void                 syntax_init(void);
extern void                 syntax_shutdown(void);

extern SyntaxTable_t *      syntax_argument(int argi, int err);
extern SyntaxTable_t *      syntax_lookup(const char *name, int err);
extern int                  syntax_keyword(const SyntaxTable_t *st, const LINECHAR *token, int length);
extern int                  syntax_keywordx(const SyntaxTable_t *st, const LINECHAR *token, int length, int start, int end, int icase);
extern int                  syntax_preprocessor(const SyntaxTable_t *st, const LINECHAR *token, int length);

extern const LINECHAR *     syntax_write(SyntaxTable_t *st, const LINECHAR *start, const LINECHAR *end, int colour);
extern const LINECHAR *     syntax_writex(SyntaxTable_t *st, const LINECHAR *start, const LINECHAR *end, int colour, int flags);
extern const LINECHAR *     syntax_comment_write(SyntaxTable_t *st, unsigned type, const LINECHAR *cursor, int offset, const LINECHAR *end);
extern const LINECHAR *     syntax_string_write(SyntaxTable_t *st, const LINECHAR *cursor, int offset, const LINECHAR *end, char quote, char ch);
extern const LINECHAR *     syntax_string_end(SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *end, const char quote, const char ch);
extern const LINECHAR *     syntax_comment_end(SyntaxTable_t *st, unsigned type, const LINECHAR *cursor, const LINECHAR *end);
extern CommentStatus_t      syntax_comment(SyntaxTable_t *st, const LINECHAR *cursor, const LINECHAR *begin, const LINECHAR *end, const LINECHAR **endp);

extern int                  syntax_parse(int all);
extern void                 syntax_highlight(const LINE_t *lp);

extern void                 syntax_hilite_init(SyntaxTable_t *st);

extern void                 do_attach_syntax(void);
extern void                 do_create_syntax(void);
extern void                 do_define_keywords(void);
extern void                 do_detach_syntax(void);
extern void                 do_set_syntax_flags(void);
extern void                 do_syntax_build(void);
extern void                 do_syntax_column_ruler(void);
extern void                 do_syntax_rule(void);
extern void                 do_syntax_token(void);
extern void                 inq_syntax(void);

__CEND_DECLS

#endif /*GR_SYNTAX_H_INCLUDED*/
