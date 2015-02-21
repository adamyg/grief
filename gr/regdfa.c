#include <edidentifier.h>
__CIDENT_RCSID(gr_regdfa_c,"$Id: regdfa.c,v 1.28 2015/02/21 22:46:35 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: regdfa.c,v 1.28 2015/02/21 22:46:35 ayoung Exp $
 * DFA regular expression engine.
 * Streamlined engine for use by the syntax hiliting code.
 *
 *  Regular expression syntax:
 *
 *      ^           If this is the first character of the regular expression, it
 *                  matches the beginning of the line.
 *
 *      $           If this is the last character of the regular expression, it matches
 *                  the end of the line.
 *
 *      .           Match any single character except newline.
 *
 *      [...]       Matches any one character contained within the character sequence.
 *
 *      *           Match the preceding character or range of characters 0 or more times.
 *
 *      +           Match the preceding character or range of characters 1 or more times.
 *
 *      ?           Match the preceding character or range of characters 0 or 1 times.
 *
 *      |           This symbol is used to indicate where to separate two sub
 *                  regular expressions for a logical OR operation.
 *
 *      (..)        Group boundaries.
 *
 *      \\d         Same as [0-9].
 *
 *      \\x         Same as [a-fA-f0-9].
 *
 *      \\s         Same as [ \\t\\f].
 *
 *      \\w         Same as [a-zA-Z_0-9].
 *
 *      \\t         Tab.
 *
 *      \\n         Newline.
 *
 *      \\r         Return.
 *
 *      \\f         Formfeed.
 *
 *      \\a         Alarm (bell, beep, etc).
 *
 *      \\e         Escape (\027).
 *
 *      \\Q..\\E    A section enclosed in these symbols it taken literally. In side
 *                  these sections, meta characters and special symbols have no
 *                  meaning. If a \\E needs to appear in one of these sections, the
 *                  \\ must be escaped with \\.
 *
 *      \\c         Anchor start of the matched text to the proceeding token.
 *
 *      \\          This escapes the meaning of a special.
 *
 *  Character Sequences:
 *
 *      The conversion specification includes all subsequent characters in the format
 *      string up to and including the matching right square bracket (]).
 *
 *      The characters between the square brackets (the scanlist) comprise the scanset,
 *      unless the character after the left square bracket is a circumflex (^), in
 *      which case the scanset contains all characters that do not appear in the
 *      scanlist between the circumflex and the right square bracket.
 *
 *      If the conversion specification begins with "[]" or "[^]", the right square
 *      bracket is included in the scanlist and the next right square bracket is the
 *      matching right square bracket that ends the conversion specification; otherwise
 *      the first right square bracket is the one that ends the conversion specification.
 *
 *      If a hyphen character (-) is in the scanlist and is not the first character,
 *      nor the second where the first character is a circumflex (^), nor the last
 *      character, it indicates a range of characters to be matched. To include a
 *      hyphen, make it the last character before the final close bracket. For
 *      instance, `[^]0-9-]' means the set `everything except close bracket, zero
 *      through nine, and hyphen'.
 *
 *      Within a bracket expression, the name of a character class enclosed in [: and
 *      :] stands for the list of all characters (not all collating elements!)
 *      belonging to that class.
 *
 *          alnum -         An alphanumeric (letter or digit).
 *          alpha -         A letter.
 *          blank -         A space, tab or form-feed character.
 *          cntrl -         A control character.
 *          digit -         A decimal digit.
 *          graph -         A character with a visible representation.
 *          lower -         A lower-case letter.
 *          print -         An alphanumeric (same as alnum).
 *          punct -         A punctuation character.
 *          space -         A character producing white space in displayed text.
 *          upper -         An upper-case letter.
 *          word  -         "word" character (alphanumeric plus "_").
 *          xdigit -        A hexadecimal digit.
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

//#define ED_LEVEL 1

#include <editor.h>
#include <limits.h>                             /* CHAR_BITS */
#include <ctype.h>

#include <rbtree.h>
#include "echo.h"
#include "debug.h"
#include "regdfa.h"

#define LOOKUP_INDEX                            /* [in]direct table definitions */
#define LOOKUP_MAX          255                 /* 2^8 or 2^16 */

#if defined(ED_LEVEL) && (ED_LEVEL >= 2)
#define DFA_DEBUG(x)        trace_log x;
#define DFA_DEBUG2(x)       trace_log x;
#else
#define DFA_DEBUG(x)
#define DFA_DEBUG2(x)
#endif
#define DFA_CSET_DEBUG      1

/*
 *  Memory management
 */
typedef union {                                 /* system alignment */
    int                     x_int;
    long                    x_long;
#if defined(HAVE_LONG_LONG_INT)
#if (SIZEOF_LONG < 8)
    long long               x_long_long;
#endif
#endif
    float                   x_float;
    double                  x_double;
} alignment_t;

#define __HEAPALIGNMENT     sizeof(alignment_t)
#define __ALIGNDOWN(n)      ((unsigned int)(n) & ~(sizeof(alignment_t) - 1))
#define HEAPALIGN(n)        __ALIGNDOWN((unsigned int)(n)+sizeof(alignment_t)-1)

typedef struct dfaheapnode {
    struct dfaheapnode *    next;
} dfaheapnode_t;

typedef struct dfaheap {
#define HEAP_MAGIC          MKMAGIC('D','f','a','H')
    MAGIC_t                 h_magic;
    unsigned                h_size;
    dfaheapnode_t *         h_head;
    char *                  h_cursor;
    char *                  h_end;
} dfaheap_t;

/*
 *  Character set management
 *
 *      with the character-set the following values are reserved as such are
 *      assumed not to be contained within the text being matched.
 *
 *          CSET_ACCEPT     0
 *          CSET_ACCEPT2    1
 *          CSET_ANCHOR     2
 *          CSET_SOL        3
 *          CSET_EOL        4
 */
#define INT_BYTES           (sizeof(unsigned int))
#define INT_BITS            (INT_BYTES * CHAR_BIT)
#if (SIZEOF_INT == 4)
#define INT_SHIFT           5                   /* / 32 */
#elif (SIZEOF_INT == 8)
#define INT_SHIFT           6
#else
#error unknown SIZEOF_INT ...
#endif

#define CSET_MAX            (256)               /* assumes 2^x size */
#define CSET_SIZE           (CSET_MAX/INT_BITS)

typedef unsigned int charset_t[CSET_SIZE];

#define cset_zero(s)        memset((s), (unsigned char) 0, sizeof(charset_t))
#define cset_fill(s)        memset((s), (unsigned char) 0xff, sizeof(charset_t))
#define cset_copy(a,b)      memcpy((a), (b), sizeof(charset_t))

#define cset_set(s,c)       ((s)[(c)/INT_BITS] |=  (1 << (c)%INT_BITS))
#define cset_clr(s,c)       ((s)[(c)/INT_BITS] &= ~(1 << (c)%INT_BITS))
#define cset_tst(s,c)       ((s)[(c)/INT_BITS] &   (1 << (c)%INT_BITS))

#define CSET_ACCEPT         0
#define CSET_ACCEPT2        1
#define CSET_ANCHOR         2
#define CSET_SPECIAL_MAX    CSET_ANCHOR
#define CSET_SOL            3
#define CSET_EOL            4

static void                 cset_all(charset_t);

/*
 *  Variable length bitmap management
 */
struct bitmap {             /* bitmap object */
    unsigned    bits;                           /* total bits */
    unsigned    bytes;                          /* size, in bytes */
    unsigned    data[1];                        /* bit data */
};

typedef struct bitmap *bitmap_t;

static bitmap_t             bm_create(dfaheap_t *heap, unsigned bits);
static void                 bm_zero(bitmap_t bm);
static void                 bm_fill(bitmap_t bm);
static void                 bm_copy(bitmap_t dst, const bitmap_t src);
static int                  bm_compare(const bitmap_t a, const bitmap_t b);
static int                  bm_union(bitmap_t out, const bitmap_t a, const bitmap_t b);
static int                  bm_first(const bitmap_t bs);
static int                  bm_next(const bitmap_t bs, int previous);

#define bm_set(bs, i)       (bs->data[((i)/INT_BITS)] |=  (1 << (i & (INT_BITS - 1))))
#define bm_clr(bs, i)       (bs->data[((i)/INT_BITS)] &= ~(1 << (i & (INT_BITS - 1))))
#define bm_tst(bs, i)       (bs->data[((i)/INT_BITS)] &   (1 << (i & (INT_BITS - 1))))

/*
 *  DFA management
 */
#if (LOOKUP_MAX <= 0xff)
typedef uint8_t dfaindex_t;
#elif (LOOKUP_MAX <= 0xffff)
typedef uint16_t dfaindex_t;
#else
#error "LOOKUP_MAX range too large ..."
#endif

typedef RB_HEAD(dfatree, dfastate) dfatree_t;

typedef struct dfastate {
    RB_ENTRY(dfastate)      dfanode;            /* RB tree node */
    bitmap_t                bits;               /* NFA ... */
    struct dfastate *       next;               /* construction queue */
    int                     accept;             /* accept state */
    int                     anchor;
    dfaindex_t              index;              /* DFA index */
#if defined(LOOKUP_INDEX)
    dfaindex_t              lookup[CSET_MAX];   /* state lookup */
#else
    struct dfastate *       lookup[CSET_MAX];   /* state lookup */
#endif
} dfastate_t;

typedef enum {
    NODE_CHARSET,
    NODE_QUEST,
    NODE_OR,
    NODE_STAR,
    NODE_PLUS,
    NODE_CAT
} nodetype_t;

typedef struct renode {
    nodetype_t              type;
    charset_t               charset;
    struct renode *         left;
    struct renode *         right;
    int                     accept;
    int                     anchor;

    int                     nullable;
    bitmap_t                firstpos;
    bitmap_t                lastpos;
    bitmap_t                followpos;
} renode_t;

typedef struct {
    struct regdfa *         regex;              /* Regular expression being built */
    dfaheap_t *             heap;               /* Work heap */
    dfaheap_t *             temp;               /* Temporary heap */
    dfatree_t               rbtree;             /* Tree of unique DFA states */
    int                     nodenumber;         /* Tree node number */
    int                     nodecursor;         /* cursor used whilst flatting tree */
    renode_t **             nodetable;          /* Flatten node (depth-first representation) */

    const char *            cursor;             /* cursor into regex pattern */
    const char *            end;                /* end of pattern */
    unsigned                flags;              /* REGDFA_... flags */
    int                     quote;              /* Quote mode */
    int                     type;               /* token type 0=charset, -1=error, otherwise operator char */
    charset_t               charset;            /* current charset */
    int                     lower;              /* repeat lower */
    int                     upper;              /* repeat upper */
} recompile_t;

static dfaheap_t *          heap_create(unsigned size);
static void                 heap_destroy(dfaheap_t *mp);
static void *               heap_alloc(dfaheap_t *mp, unsigned size);

static int                  dfa_functions(recompile_t *re, renode_t *rx);
static dfastate_t *         dfa_new(recompile_t *re, const bitmap_t bm);
static dfastate_t *         dfa_find(recompile_t *re, const bitmap_t bits);
static int                  dfa_compare(const dfastate_t *a, const dfastate_t *b);

RB_PROTOTYPE(dfatree, dfastate, dfanode, dfa_compare);

static int                  charset_alias(recompile_t *re, int value, const char *alias, int length);
static void                 charset_assign(recompile_t *re, unsigned chr, int value);

static renode_t *           parse_3(recompile_t *re);
static renode_t *           parse_2(recompile_t *re);
static renode_t *           parse_1(recompile_t *re);
static renode_t *           parse_expression(recompile_t *re, const char *begin, const char *end);

static dfastate_t *         dfa_new(recompile_t *re, const bitmap_t bm);
static dfastate_t *         dfa_find(recompile_t *re, const bitmap_t bits);

#if defined(__cplusplus)
extern "C" {
#endif
#if !defined(HAVE_ISASCII) && !defined(HAVE___ISASCII)
    static int              isascii(int c);
#endif
#if !defined(HAVE_ISBLANK) && !defined(HAVE___ISBLANK)
    static int              isblank(int c);
#endif
    static int              is_word(int c);

    static const struct {                       /* character classes */
        const char *    name;
        int             len;
        int           (*isa)(int);              /* Test function */
    } charclasses[] = {
#if defined(HAVE___ISASCII)
        { "ascii",  5,  __isascii },            /* ASCII character. */
#else
        { "ascii",  5,  isascii   },
#endif
        { "alnum",  5,  isalnum   },            /* An alphanumeric (letter or digit). */
        { "alpha",  5,  isalpha   },            /* A letter. */
#if defined(HAVE___ISBLANK)
        { "blank",  5,  __isblank },            /* A space or tab character. */
#else
        { "blank",  5,  isblank   },
#endif
        { "cntrl",  5,  iscntrl   },            /* A control character. */
        { "digit",  5,  isdigit   },            /* A decimal digit. */
        { "graph",  5,  isgraph   },            /* A character with a visible representation. */
        { "lower",  5,  islower   },            /* A lower-case letter. */
        { "print",  5,  isprint   },            /* An alphanumeric (same as alnum). */
        { "punct",  5,  ispunct   },            /* A punctuation character. */
        { "space",  5,  isspace   },            /* A character producing white space in displayed text. */
        { "upper",  5,  isupper   },            /* An upper-case letter. */
        { "word",   4,  is_word   },            /* A "word" character (alphanumeric plus "_"). */
        { "xdigit", 6,  isxdigit  }             /* A hexadecimal digit. */
        };
#if defined(__cplusplus)
};
#endif


/*  Function:           heap_create
 *      Allocate a heap control structure in one of two forms based on the value of
 *      size. If specified, then size represents the size of the arena is reserves for
 *      all heap alocations. Otherwise if zero, memory shall be allocation using
 *      calloc() with the heap acting as a simple container.
 *
 *  Parameters:
 *      size -              Heap size.
 *
 *  Returns:
 *      Address of the heap control object.
 */
static dfaheap_t *
heap_create(unsigned size)
{
    dfaheap_t *heap;

    if (NULL != (heap =
            (dfaheap_t *)chk_calloc(sizeof(dfaheap_t) + size, 1))) {
        heap->h_magic = HEAP_MAGIC;
        heap->h_size = size;
        heap->h_head = NULL;
        if (size) {
            char *base = (char *)(heap + 1);

            heap->h_cursor = base;
            heap->h_end = base + size;
        }
    }
    DFA_DEBUG(("regdfa: heap_create(%u)\n", size))
    return heap;
}


/*  Function:           heap_shrink
 *      Reduce the heaps arena to total size of all allocated blocks, returning the
 *      unused storage to main heap.
 *
 *      Note this function has no affected if the heap was created with a specified
 *      size of 0, stating the use of calloc() base storage.
 *
 *  Parameters:
 *      heap -              Heap control object.
 *
 *  Returns:
 *      nothing
 */
static int
heap_shrink(dfaheap_t *heap)
{
    unsigned used, size;

    assert(heap);
    if (NULL != heap->h_cursor) {
        char *base = (char *)(heap + 1);

        assert(NULL == heap->h_head);
        size = heap->h_size;
        used = heap->h_cursor - base;
#if !defined(USING_PURIFY)
        chk_shrink((void *)heap, sizeof(dfaheap_t) + used);
#endif
        heap->h_size = used;
        heap->h_end = base + used;

    } else {
        assert(NULL == heap->h_end);
        used = size = heap->h_size;
    }

    DFA_DEBUG(("regdfa: heap_shrink(%u of %u)\n", used, size))
    return 0;
}


/*  Function:           heap_destroy
 *      Destory the heap and any asociated allocated storage.
 *
 *  Parameters:
 *      heap -              Heap control object.
 *
 *  Returns:
 *      nothing
 */
static void
heap_destroy(dfaheap_t *heap)
{
    if (heap) {
        if (NULL != heap->h_cursor) {
            assert(NULL == heap->h_head);
            DFA_DEBUG(("regdfa: heap_destroy(%u of %u)\n", (heap->h_cursor - (char *)(heap + 1)), heap->h_size))

        } else {                                /* release of chained allocation blocks */
            dfaheapnode_t *node, *head = heap->h_head;

            DFA_DEBUG(("regdfa: heap_destroy(%u)\n", heap->h_size))
            assert(NULL == heap->h_end);
            while (NULL != (node = head)) {
                head = node->next;
                chk_free(node);
            }
        }
        chk_free(heap);
    }
}


/*  Function:           heap_alloc
 *      Allocation a zero filled block of the heap.
 *
 *   Parameters:
 *      heap -              Heap control object.
 *      size -              Size of object, in bytes.
 *
 *  Returns:
 *      Address of the allocation block, otherwise NULL.
 */
static void *
heap_alloc(dfaheap_t *heap, unsigned size)
{
    unsigned alignedsize = HEAPALIGN(size);     /* force block alignment */
    char *block = NULL;

    assert(heap);
    assert(size);
    if (size) {
        assert(heap->h_magic == HEAP_MAGIC);
        if (NULL != heap->h_cursor) {
            assert(NULL == heap->h_head);
            if ((heap->h_cursor + alignedsize) >= heap->h_end) {
                block = NULL;
            } else {
                block = heap->h_cursor;
                heap->h_cursor += alignedsize;
            }
        } else {
            dfaheapnode_t *node;

            assert(NULL == heap->h_end);
            if (NULL != (node =
                    (dfaheapnode_t *)chk_calloc(sizeof(dfaheapnode_t) + size, 1))) {
                node->next = heap->h_head;      /* chain allocation blocks */
                heap->h_head = node;
                heap->h_size += size;           /* tally total size */
                block = (char *)(node + 1);
            }
        }
    }

    if (NULL == block) {
        ewprintf("regdfa: out of local memory");
    }
    return block;
}


/*  Function:           bm_create
 *      Build a new bitmap using the given heap 'heap' of the size 'bits'.
 *
 *  Parameters:
 *      heap -              Storage heap from which to allocate bitmap.
 *      bits -              Bit count.
 *
 *  Returns:
 *      Bit index, otherwise -1 if none.
 */
static bitmap_t
bm_create(dfaheap_t *heap, unsigned bits)
{
    unsigned bytes = INT_BYTES * ((bits / INT_BITS) + ((bits % INT_BITS) ? 1 : 0));
    bitmap_t bm;

 /* assert(bytes <= bits); */
    if (NULL == (bm =
            (bitmap_t)heap_alloc(heap, sizeof(struct bitmap) + bytes))) {
        return NULL;
    }
    DFA_DEBUG2(("bm_create(bits:%i, bytes:%u)=%p\n", bits, bytes, bm))
    bm->bits = bits;                            /* total bits, encoding within first element */
    bm->bytes = bytes;                          /* size of data arena, in bytes */
    bm_zero(bm);
    return bm;
}


static void
bm_zero(bitmap_t bm)
{
    assert(bm->bytes > 0);
    memset(bm->data, 0, bm->bytes);
}


static void
bm_fill(bitmap_t bm)
{
    const unsigned bytes = bm->bytes;           /* bytes */
    const unsigned last = bm->bits % INT_BITS;  /* bits within last word */

    assert(bytes > 0 && 0 == (bytes % INT_BYTES));
    memset(bm->data, 0xff, bytes);              /* set all bytes */
    if (last) {                                 /* partial set of last element */
        bm->data[ (bytes / INT_BYTES) - 1 ] = ((1 << last) - 1);
    }
}


static void
bm_copy(bitmap_t dst, const bitmap_t src)
{
    assert(src->bytes == dst->bytes);
    memcpy((void *)dst->data, (const char *)src->data, src->bytes);
}


static int
bm_compare(const bitmap_t a, const bitmap_t b)
{
    assert(a->bits == b->bits);
    assert(a->bytes == b->bytes);
    return memcmp(a->data, b->data, a->bytes);
}


/*  Function:           bm_next
 *      Return the next bit set within the specificed bitmap
 *
 *  Parameters:
 *      bm -                Bitmap to be tested.
 *      cursor -            Cursor of previous bit.
 *
 *  Returns:
 *      Bit index, otherwise -1 if none.
 */
static int
bm_next(const bitmap_t bm, int cursor)
{
    const int bits = (int) bm->bits;

    DFA_DEBUG2(("\tbm_next(%p, cursor:%u)", bm, cursor))

    ++cursor;
    while (cursor < bits) {
        const unsigned int word = cursor >> INT_SHIFT;
        const unsigned int data = bm->data[word];
        unsigned bit = cursor & (INT_BITS - 1);

        while (bit < INT_BITS) {
            if (data & (0x1 << bit)) {
                bit = ((word * INT_BITS) + bit);
                DFA_DEBUG2(("=%u\n", bit))
                assert(bit < bm->bits);
                return (int)(bit);
            }
            ++bit;
        }
        cursor = cursor - (cursor & (INT_BITS - 1)) + INT_BITS;
    }
    DFA_DEBUG2(("=-1\n"))
    return -1;
}


/*  Function:           bm_first
 *      Return the first bit set within the specificed bitmap
 *
 *  Parameters:
 *      bm -                Bitmap to be tested.
 *
 *  Returns:
 *      Bit index, otherwise -1 if none.
 */
static int
bm_first(const bitmap_t bm)
{
    return bm_next(bm, -1);
}


/*  Function:           bm_union
 *      Generate a union of two bitmaps
 *
 *  Parameters:
 *      dst -               Destination bitmap.
 *      a -                 First bitmap
 *      b -                 Second bitmap.
 *
 *  Returns:
 *      *true* if one or bits where bit, otherwise *false*.
 */
static int
bm_union(bitmap_t dst, const bitmap_t a, const bitmap_t b)
{
    unsigned int *ddata = dst->data;
    const unsigned int *adata = a->data;
    const unsigned int *bdata = b->data;
    unsigned word = (a->bytes / INT_BYTES);
    unsigned char set = 0;

    DFA_DEBUG2(("bm_union(%p, %p, %p)=%p\n", dst, a, b))

    assert(a->bits == b->bits);
    assert(dst->bits == a->bits);
    assert(dst->bytes > 0 && 0 == (dst->bytes % INT_BYTES));
    while (word-- > 0) {
        register unsigned int val = (*adata++ | *bdata++);
        *ddata++ = val;
        if (val) ++set;
    }
    return set;
}


/*  Function:           token_charset
 *      Cook the character set specification. The conversion specification includes all
 *      subsequent characters in the format string up to and including the matching
 *      right square bracket (]).
 *
 *  Parameters:
 *      re -                Regular expression working storage.
 *      cursor -            Buffer cursor.
 *
 *  Returns:
 *      Resulting cursor position, NULL on error.
 *
 */
static const char *
parse_charset(recompile_t *re, const char *cursor)
{
    const char *start = cursor;
    int lvalue, range, v;

    if ('^' == *cursor) {
        /* [^.. negative */
        cset_all(re->charset);
        ++cursor;
        v = 0;
    } else {
        cset_zero(re->charset);
        v = 1;
    }

    if (']' == *cursor || '-' == *cursor) {     /* [].., [^].., [-.. or [^-.. treat as initial character */
        charset_assign(re, (unsigned char) *cursor++, v);
    }

    range = lvalue = 0;

    while (cursor < re->end) {
        int c = *cursor++;                      /* next */

        if (']' == c) {                         /* end */
#if defined(DFA_CSET_DEBUG)
            char csetbuffer[128], *end = csetbuffer;

            for (c = 32; c < 128; ++c)
                if (cset_tst(re->charset, c)) {
                    *end++ = (char)(isprint(c) ? c : '.');
                }
            *end = 0;
            DFA_DEBUG(("regdfa: -> cs:[%.*s={%s}]\n", cursor - start, start, csetbuffer));
#endif
            return cursor;
        }

        if (range) {                            /* right-side */
            /*  Charater range
             *    To avoid confusion, error if the range is not numerically greater
             *    than the left side character.
             */
            if ('[' == c) {
                ewprintf("regdfa: r-value cannot be character-class/sequence '%%[%.*s'",
                        (int)(cursor - start) + 1, start);
                return NULL;
            }

            if (c <= lvalue) {
                ewprintf("regdfa: invalid collating order '%%[%.*s'",
                        (int)(cursor - start) + 1, start);
                return NULL;
            }

            while (lvalue <= c) {               /* .. mark range */
                charset_assign(re, lvalue++, v);
            }

            range = 0;                          /* range open */
            lvalue = c;

        } else if ('\\' == c) {
            /* An escaped character */
            c = *cursor++;
            switch (c) {
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            default:
                break;
            }
            charset_assign(re, c, v);
            lvalue = c;

        } else if ('[' == c && cursor < re->end && ':' == *cursor) {
            /* Character-classes */
            const char *cc = ++cursor;

            while (cursor < re->end && 0 != (c = *cursor++))
                if (':' == c && ']' == *cursor) {
                    /* look for closing :] */
                    int len = (int)((cursor - cc) - 1);

                    if (-1 == charset_alias(re, v, cc, len)) {
                        ewprintf("regdfa: unknown character-class '%.*s'", len, cc);
                        return NULL;
                    }
                    ++cursor;
                    break;
                }
            if (0 == c) {
                ewprintf("regdfa: unmatched ':]' within '%%[%.*s'", (int)(cursor - start) + 1, start);
                return NULL;
            }
            lvalue = 0;                         /* cannot be l-value */

        } else if ('-' == c) {
            /* -' is not considered to define a range if the character following is a closing bracket */
            if (cursor < re->end && ']' == *cursor) {
                charset_assign(re, '-', v);
                return ++cursor;
            }

            if (0 == lvalue) {                  /* missing right-side */
                ewprintf("regdfa: unmatched range '%%[%.*s'", (int)(cursor - start) + 1, start);
                return NULL;
            }

            range = 1;                          /* range open */

        } else {
            /* character */
            charset_assign(re, (unsigned char)c, v);
            lvalue = c;
        }
    }

    ewprintf("regdfa: unexpected end-of-format '%%[%.*s'", (int)(cursor - start) + 1, start);
    return NULL;
}


#if !defined(HAVE_ISASCII)
static int
isascii(int c)
{
    return (c > 0 && c <= 0x7f);
}
#endif


#if !defined(HAVE_ISBLANK)
static int
isblank(int c)
{
    return (' ' == c || '\t' == c);
}
#endif


static int
is_word(int c)
{
    return ('_' == c || isalnum(c));
}


static int
charset_alias(recompile_t *re, int value, const char *alias, int length)
{
    int i;

    if (length < 0 && alias) {
        length = (int)strlen(alias);
    }

    for (i = (sizeof(charclasses)/sizeof(charclasses[0]))-1; i >= 0; --i)
        if (length == charclasses[i].len)
            if (0 == strncmp(alias, charclasses[i].name, charclasses[i].len)) {
                register int c;                 /* match class */

                for (c = 0; c < CSET_MAX && c < 256; ++c) {
                    switch (c) {
                    case CSET_ACCEPT:
                    case CSET_ACCEPT2:
                    case CSET_ANCHOR:
                    case CSET_SOL:
                    case CSET_EOL:
                        break;
                    default:
                        if ((*charclasses[i].isa)(c)) {
                            charset_assign(re, c, value);
                        }
                    }
                }
                return i;
            }
    return -1;
}


static void
charset_assign(recompile_t *re, unsigned chr, int value)
{
    if (value) {
        cset_set(re->charset, chr);
    } else {
        cset_clr(re->charset, chr);
    }
}


static void
cset_all(charset_t cs)
{
    cset_fill(cs);
    cset_clr(cs, CSET_ACCEPT);
    cset_clr(cs, CSET_ACCEPT2);
    cset_clr(cs, CSET_ANCHOR);
    cset_clr(cs, CSET_SOL);
    cset_clr(cs, CSET_EOL);
}


/*  Function:           parse_token
 *      Parse the next token within the stream, returning the related character set.
 *
 *  Return:
 *      1 on success, 0 when end of the stream, otherwise -1 on error.
 *
 *  Supported Escapes Sequences:
 *
 *      w -     Word character (alphanumeric plus "_").
 *      s -     White-space character (space or tab).
 *      d -     Digit character.
 *      x -     Hex digit character.
 *      t -     Tab.
 *      n -     Newline.
 *      r -     Return.
 *      f -     Form-feed.
 *      a -     Alarm (bell, beep, etc).
 *      e -     Escape (\027).
 *      Q..E -  Ignore all special characters between the \Q and \E symbols.
 *
 */
static int
parse_token(recompile_t *re)
{
    const char *cursor = re->cursor;
    int anchor = 0;
    unsigned char c;

    /* deal with \Q...\E markers */
    for (;;) {
        if (cursor >= re->end) {
            re->type = -1;
            return 0;
        }

        if ('\\' == *cursor) {
            if (0 == re->quote && cursor[1] == 'Q') {
                re->quote = 1;                  /* \Q quote text */
                cursor += 2;
                continue;

            } else if (1 == re->quote && cursor[1] == 'E') {
                re->quote = 0;                  /* \E end quoted text */
                cursor += 2;
                continue;
            }
        }
        break;
    }

    /* process next token */
    c = (unsigned char) *cursor++;

    if (re->quote) {
        cset_zero(re->charset);
        cset_set(re->charset, c);
        re->type = 0;

    } else {
        if ('\\' == c && 'c' == *cursor) {      /* anchor */
            c = (unsigned char) *++cursor;
            ++cursor;
            anchor = 1;
        }

        switch (c) {            /* operators */
        case '(': case ')':
        case '*': case '+':
        case '?':
        case '|':
            re->type = c;
            break;

#if defined(TODO)
//      case '{':               /* interval {[<lower>,] <upper>} */
//          if (NULL == (cursor = parse_interval(re, cursor))) {
//              re->type = -1;
//              return -1;
//          }
//          re->type = '{';
//          break;
#endif

        case '[':               /* character set */
            if (NULL == (cursor = parse_charset(re, cursor))) {
                re->type = -1;
                return -1;
            }
            re->type = 0;
            break;

        case '.':               /* any */
            cset_all(re->charset);
            cset_clr(re->charset, '\n');
            re->type = 0;
            break;

        case '^':               /* start-of-line/string */
            cset_zero(re->charset);
            cset_set(re->charset, CSET_SOL);
            re->type = 0;
            break;

        case '$':               /* end-of-line/string */
            cset_zero(re->charset);
            cset_set(re->charset, CSET_EOL);
            cset_set(re->charset, '\n');
            re->type = 0;
            break;

        case '\\':              /* escaped character */
            if (cursor >= re->end) {
                ewprintf("regdfa: unexpected end-of-format, within character quote");
                re->type = -1;
                return -1;
            }
            c = 0;
            re->type = 0;
            cset_zero(re->charset);
            switch (*cursor) {
            case 'w': charset_alias(re, 1, "word",   4); break;
            case 's': charset_alias(re, 1, "blank",  5); break;
            case 'd': charset_alias(re, 1, "digit",  5); break;
            case 'x': charset_alias(re, 1, "xdigit", 6); break;
            case 't': c = '\t'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 'f': c = '\f'; break;
            case 'a': c = '\a'; break;
            case 'e': c = 0x1b; break;
            default:
                c = *cursor;
            }
            if (c > 0) {
                cset_set(re->charset, c);
            }
            ++cursor;
            break;

        case CSET_SOL:          /* specials, convert */
        case CSET_EOL:
            c = ' ';
            /*FALLTHRU*/

        default:                /* others/character */
            if ((REGDFA_ICASE & re->flags) && isalpha(c)) {
                cset_zero(re->charset);
                c = (unsigned char)toupper(c);
                cset_set(re->charset, c);
                c = (unsigned char)tolower(c);
                cset_set(re->charset, c);
                break;
            }
            /*FALLTHRU*/

        case CSET_ACCEPT:
        case CSET_ACCEPT2:
        case CSET_ANCHOR:
            cset_zero(re->charset);
            cset_set(re->charset, c);
            re->type = 0;
            break;
        }
    }

    if (anchor) {
        cset_set(re->charset, CSET_ANCHOR);
    }

    re->cursor = cursor;
    return 1;
}


static renode_t *
node_alloc(recompile_t *re, nodetype_t type, renode_t *left, renode_t *right)
{
    renode_t *n;

    if (NULL == (n =
            (renode_t *)heap_alloc(re->temp, sizeof(renode_t)))) {
        return NULL;
    }
    n->type  = type;
    n->left  = left;
    n->right = right;
    return n;
}


static renode_t *
parse_4(recompile_t *re)
{
    renode_t *n;

    switch (re->type) {
    case 0:
        if (NULL == (n =
                node_alloc(re, NODE_CHARSET, NULL, NULL))) {
            return NULL;
        }
        cset_copy(n->charset, re->charset);
        parse_token(re);                        /* match charset */
        break;

    case '(':
        parse_token(re);                        /* match '(' */
        n = parse_1(re);
        if (re->type != ')') {
            ewprintf("regdfa: missing ')'");
            return 0;
        }
        parse_token(re);                        /* match ')' */
        break;

    default:
        n = NULL;
        break;
    }
    return n;
}


static renode_t *
parse_3(recompile_t *re)
{
    renode_t *l, *n;
    int ntype;

    if (NULL == (l = parse_4(re))) {
        return NULL;
    }

    for (;;) {
        switch (re->type) {
        case '*': ntype = NODE_STAR;  break;
        case '+': ntype = NODE_PLUS;  break;
        case '?': ntype = NODE_QUEST; break;
//TODO  case '{': ntype = NODE_RANGE; break;
        default:
            return l;
        }
        if (NULL == (n =
                node_alloc(re, ntype, l, NULL))) {
            return NULL;
        }
        parse_token(re);
        l = n;
    }
    return n;
}


static renode_t *
parse_2(recompile_t *re)
{
    renode_t *l, *r;

    if (NULL == (l = parse_3(re))) {
        return NULL;
    }

    if ('|' == re->type) {
        return l;
    }

    if (NULL == (r = parse_2(re))) {
        return l;
    }

    return node_alloc(re, NODE_CAT, l, r);
}


static renode_t *
parse_1(recompile_t *re)
{
    renode_t *l, *r;

    if (NULL == (l = parse_2(re))) {
        return NULL;
    }

    if ('|' != re->type) {
        return l;
    }

    parse_token(re);                            /* match '|' */

    if (NULL == (r = parse_1(re))) {
        ewprintf("regdfa: badly formed '|' expression");
        return NULL;
    }

    return node_alloc(re, NODE_OR, l, r);
}


static renode_t *
parse_expression(recompile_t *re, const char *begin, const char *end)
{
    renode_t *rx;

    re->cursor = begin;
    re->end = end;

    parse_token(re);
    if (NULL == (rx = parse_1(re))) {
        ewprintf("regdfa: parse error in regular expression");
    }
    re->cursor = re->end = NULL;
    return rx;
}


static int
node_visit(recompile_t *re, renode_t *n)
{
    int count = 1;

    assert((n->type != NODE_OR) || (n->left && n->right));

    if (n->left) {
        count += node_visit(re, n->left);
    }

    if (n->right) {
        count += node_visit(re, n->right);
    }

    if (re) {
        re->nodetable[re->nodecursor++] = n;
    }

    return count;
}


/*  Function:           dfa_functions
 *      Construct the functions 'nullable', 'firstpos', 'lastpos' and 'followpos' by
 *      making a depth-first traversal of the NFA expression tree.
 *
 *  Parameters:
 *      regdfa -            Regular expression control structure.
 *
 *      rx -                Root of expression tree.
 *
 *  Attributes:
 *      followpos(n) -      is the set of positions which can follow the position i in
 *                          the strings generated by the augmented regular expression.
 *
 *      To evaluate followpos, we need three more functions to be defined for the nodes
 *      (not just for leaves) of the syntax tree.
 *
 *      firstpos(n) -       the set of the positions of the first symbols of strings
 *                          generated by the sub-expression rooted by n.
 *
 *      lastpos(n) -        the set of the positions of the last symbols of strings
 *                          generated by the sub-expression rooted by n.
 *
 *      nullable(n) -       *true* if the empty string is a member of strings generated
 *                          by the sub-expression rooted by n, *false* otherwise
 *
 *  Logic:
 *
 *      o Firstpos, lastpos, nullable;
 *
 *(start code)
 *          n                   firstpos(n)         lastpos(n)          nullable(n)
 *
 *          leaf labelled e     {o}                 {o}                 true
 *
 *          leaf labelled with
 *          position i          {i}                 {i}                 false
 *
 *          c1 | c2             firstpos(c1)        lastpos(c1)         nullable(c1)
 *                              u firstpos(c2)      u lastpos(c2)       or nullable(c2)
 *
 *          c1 & c2             if (nullable(c1))   if (nullable(c2))   nullable(c1)
 *                                  firstpos(c1)        lastpos(c1)     and nullable(c2)
 *                                  u firstpos(c2)      u lastpos(c2)
 *                              else firstpos(c1)   else lastpos(c2)
 *
 *          * c1                firstpos(c1)        lastpos(c1)         true
 *(end code)
 *
 *      o Two-rules define the function followpos:
 *
 *          If n is concatenation-node with left child c1 and right child c2,and i is a
 *          position in lastpos(c1), then all positions in firstpos(c2) are in
 *          followpos(i).
 *
 *          If n is a star-node, and i is a position in lastpos(n), then all positions
 *          in firstpos(n) are in followpos(i).
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
static int
dfa_functions(recompile_t *re, renode_t *rx)
{
    int nodenumber, accepted = 1;
    renode_t *n, *c1, *c2;
    int i, x;

    /* fill the table, perform a depth first search of the tree and flatten. */
    nodenumber = node_visit(NULL, rx);

    re->nodenumber = nodenumber;
    if (NULL == (re->nodetable =
            (renode_t **)heap_alloc(re->temp, sizeof(renode_t *) * nodenumber))) {
        return -1;
    }

    re->nodecursor = 0;
    nodenumber = node_visit(re, rx);
    assert(nodenumber == re->nodenumber);

    /* initialise entries */
    for (i = 0; i < nodenumber; ++i) {
        n = re->nodetable[i];

        n->firstpos  = bm_create(re->temp, nodenumber);
        n->lastpos   = bm_create(re->temp, nodenumber);
        n->followpos = bm_create(re->temp, nodenumber);
    }

    /* walk table */
    for (i = 0; i < nodenumber; ++i) {

        n   = re->nodetable[i];
        c1  = n->left;
        c2  = n->right;

        /*accept states*/
        if (cset_tst(n->charset, CSET_ACCEPT)) {
            n->accept = accepted++;

        } else if (cset_tst(n->charset, CSET_ACCEPT2)) {
            n->accept = accepted++ * -1;

        } else if (cset_tst(n->charset, CSET_ANCHOR)) {
            n->anchor = 1;
        }

        /*firstpos, lastpos and nullable*/
        switch (n->type) {
        case NODE_CAT:
            if (c1->nullable) {
                bm_union(n->firstpos, c1->firstpos, c2->firstpos);
            } else {
                bm_copy(n->firstpos, c1->firstpos);
            }

            if (c2->nullable) {
                bm_union(n->lastpos, c1->lastpos, c2->lastpos);
            } else {
                bm_copy(n->lastpos, c2->lastpos);
            }
            n->nullable = c1->nullable && c2->nullable;
            break;

        case NODE_PLUS:
            bm_copy(n->firstpos, c1->firstpos);
            bm_copy(n->lastpos, c1->lastpos);
            n->nullable = c1->nullable;
            break;

        case NODE_OR:
            bm_union(n->firstpos, c1->firstpos, c2->firstpos);
            bm_union(n->lastpos, c1->lastpos, c2->lastpos);
            n->nullable = c1->nullable || c2->nullable;
            break;

        case NODE_QUEST:
        case NODE_STAR:
            bm_copy(n->firstpos, c1->firstpos);
            bm_copy(n->lastpos, c1->lastpos);
            n->nullable = 1;
            break;

#if defined(TODO)
//      case NODE_RANGE: {
//              if (n->upper <= 0) {
//                  continue;
//              }
//              n->nullable = (n->lower <= 0 ? 1 : 0);
//          }
//          break;
#endif

        case NODE_CHARSET:
            bm_set(n->firstpos, i);
            bm_set(n->lastpos, i);
            n->nullable = 0;
            break;

        default:
            ewprintf("regdfa: unknown node type %d", n->type);
            return -1;
        }

        /*followpos*/
        switch (n->type) {
        case NODE_CAT:
            for (x = 0; x < nodenumber; ++x) {
                if (bm_tst(c1->lastpos, x)) {
                    renode_t *t_n = re->nodetable[x];

                    bm_union(t_n->followpos, t_n->followpos, c2->firstpos);
                }
            }
            break;
        case NODE_PLUS:
        case NODE_STAR:
            for (x = 0; x < nodenumber; ++x) {
                if (bm_tst(n->lastpos, x)) {
                    renode_t *t_n = re->nodetable[x];

                    bm_union(t_n->followpos, t_n->followpos, n->firstpos);
                }
            }
            break;
        default:
            break;
        }
    }
    return 0;
}


static dfastate_t *
dfa_new(recompile_t *re, const bitmap_t bm)
{
    register dfatree_t *rbtree = &re->rbtree;
    struct regdfa *regex = re->regex;
    dfastate_t *dfa;

    /*
     *  create node and insert into tree
     */
    if (NULL == (dfa =
            (dfastate_t *)heap_alloc(re->heap, sizeof(dfastate_t)))) {
        return NULL;
    }
    if (NULL == (dfa->bits = bm_create(re->temp, bm->bits))) {
        return NULL;
    }
    bm_copy(dfa->bits, bm);

    if (RB_INSERT(dfatree, rbtree, dfa) != NULL) {
        ewprintf("regdfa: failed to add dfa node");
        return NULL;
    }
    assert(dfa == dfa_find(re, bm));

    /*
     *  insert into runtime table, allowing indices instead of dfastate
     *  references resulting in a (75% - 50%) space saving dependent
     *  on LOOKUP_MAX
     */
    if (dfa->index >= LOOKUP_MAX) {
        ewprintf("regdfa: too many dfa states, lookup table full");
        return NULL;
    }

    if (regex->cursor >= regex->slots) {
        if (0 == regex->slots) {
            regex->slots = 32;                  /* initial slots (32*2) */
        }
        regex->slots *= 2;                      /* double current */

        if (NULL == (regex->table =
                (dfastate_t **)chk_realloc(regex->table, sizeof(dfastate_t *) * regex->slots))) {
            ewprintf("regdfa: memory error, lookup table");
            return NULL;
        }

        if (0 == regex->cursor) {               /* slot0=NULL */
            regex->table[ regex->cursor++ ] = NULL;
        }
    }
    dfa->index = (dfaindex_t)(regex->cursor++); /* 1...x */
    regex->table[ dfa->index ] = dfa;

    return dfa;
}


static dfastate_t *
dfa_find(recompile_t *re, const bitmap_t bits)
{
    register dfatree_t *rbtree = &re->rbtree;
    dfastate_t dfa = {0};

    dfa.bits = bits;
    return RB_FIND(dfatree, rbtree, &dfa);
}


static int
dfa_compare(const dfastate_t *a, const dfastate_t *b)
{
    int r;

    if (0 == (r = bm_compare(a->bits, b->bits))) {
        return 0;                               /* same */
    } else if (r > 0) {
        return 1;                               /* left */
    }
    return -1;                                  /* right */
}

RB_GENERATE(dfatree, dfastate, dfanode, dfa_compare);


/*  Function:           dfa_states
 *      Construct the DFA states.
 *
 *  Parameters:
 *      regdfa -            Regular expression control structure.
 *      rx -                Root of expression tree.
 *
 *  Returns:
 *      0 on success, otherwise -1 on error.
 */
static int
dfa_states(recompile_t *re, renode_t *rx)
{
    dfastate_t *dfa, *tail;
    int nonempty = 0, count = 0;
    bitmap_t accum, dfa_bits;
    int i, c;

    /* create first state */
    RB_INIT(&re->rbtree);

    if (NULL == (accum = bm_create(re->temp, re->nodenumber))) {
        goto error;
    }

    if (NULL == (dfa = dfa_new(re, rx->firstpos))) {
        goto error;
    }

    re->regex->start = dfa;                     /* initial parser state */

    /*
     *  iterate through all states processing umarked states
     */
    for (tail = dfa; dfa; dfa = dfa->next) {
        /*
         *  iterate through all the inputs for this state
         */
        dfa_bits = dfa->bits;
        bm_zero(accum);

        for (c = 0; c < CSET_MAX; ++c) {
            /*
             *  iterate through all the states in firstpos
             */
            for (i = bm_first(dfa_bits); i >= 0; i = bm_next(dfa_bits, i)) {
                if (cset_tst(re->nodetable[i]->charset, c)) {
                    if (CSET_ACCEPT == c || CSET_ACCEPT2 == c) {
                        DFA_DEBUG(("regdfa: accept state %d -> %d\n", dfa->accept, re->nodetable[i]->accept))
                        dfa->accept = re->nodetable[i]->accept;

                    } else if (CSET_ANCHOR == c) {
                        DFA_DEBUG(("regdfa: anchor state %d -> %d\n", dfa->anchor, re->nodetable[i]->anchor))
                        dfa->anchor = re->nodetable[i]->anchor;
                    }

                    nonempty += bm_union(accum, accum, re->nodetable[i]->followpos);
                }
            }

            if (nonempty) {
                dfastate_t *ndfa;

                if (NULL == (ndfa = dfa_find(re, accum))) {
                    /*
                     *  New DFA state, add to the end of the chain to be processed.
                     */
                    if (NULL == (ndfa = dfa_new(re, accum))) {
                        goto error;
                    }
                    tail->next = ndfa;
                    tail = ndfa;
                    ++count;
                }
#if defined(LOOKUP_INDEX)
                dfa->lookup[c] = ndfa->index;   /* indirect reference */
#else
                dfa->lookup[c] = ndfa;          /* direct reference */
#endif
                nonempty = 0;
                bm_zero(accum);
            }
        }
    }

    DFA_DEBUG(("regdfa: built %d states\n", count))
    return 0;

error:;
    ewprintf("regdfa: out of local memory at state %d", count);
    return -1;
}


/*  Function:           regdfa_create
 *      Create a DFA base engine for the specified regular expression.
 *
 *  Logic:
 *      parse_expression -
 *
 *          o Create the syntax tree of (r)
 *
 *      dfa_functions -
 *
 *          o Calculate the functions: followpos, firstpos, lastpos, nullable.
 *
 *      dfa_states -
 *
 *          o Put firstpos(root) into the states of DFA as an unmarked state.
 *
 *          o while (there is an unmarked state S in the states of DFA) do
 *              mark S
 *              for each input symbol a do
 *                  let s1,...,sn are positions in S and
 *                          symbols in those positions are a S
 *                      followpos(s1) .. followpos(sn)
 *                          move(S,a) S
 *
 *                  if (S is not empty and not in the states of DFA)
 *                      put S into the states of DFA as an unmarked state.
 *                  endif
 *            endwhile
 *
 *          o the start state of DFA is firstpos(root)
 *
 *          o the accepting states of DFA are all states containing the position of #
 *
 *  References:
 *      "Dragron Book", 'Section 3.9, 'From a regular Expression to a DFA', Pages 141-143
 *
 *  Others:
 *      "An Introduction to Formal Languages and Automata", Fourth Edition Peter Linz,
 *      University of California, Davis, California
 *      ISBN-13: 9780763737986, ISBN-10: 0763737984
 *
 *  Returns:
 *      Express object, otherwise NULL on error.
 *
 */
struct regdfa *
regdfa_create(const char **patterns, int num_patterns, unsigned flags)
{
#if (SIZEOF_VOID_P == 8) || (SIZEOF_LONG == 8)
#define WORK_SIZE   (256 * 1024)                /* ~200 states */
#define TEMP_SIZE   (128 * 1024)
#else
#define WORK_SIZE   (128 * 1024)                /* ~200 states */
#define TEMP_SIZE   (64 * 1024)
#endif

    recompile_t recomp;
    dfaheap_t *heap, *temp = NULL;
    struct regdfa *regex = NULL;
    char *pattern, *end;
    renode_t *rx;
    unsigned len;
    int i;

    memset(&recomp, 0, sizeof(recomp));

    /* allocation regdfa local storage */
    if (NULL == (heap = heap_create(WORK_SIZE)) ||
            NULL == (regex = (struct regdfa *)heap_alloc(heap, sizeof(struct regdfa)))) {
        goto error;
    }

    regex->heap = heap;
    regex->flags = flags;

    /* allocate working area */
    for (len = 0, i = 0; i < num_patterns; ++i) {
        len += (int)(strlen(patterns[i]) + 16);
    }

    if (NULL == (temp = heap_create(TEMP_SIZE)) ||
            NULL == (pattern = (char *)heap_alloc(temp, len + 1))) {
        goto error;
    }

    /* Join the expressions together, marking the acception state with CSET_ACCEPT[2]
     *
     *  Expression construction:
     *
     *      Basicly given a set of expressions are E1,E2,...,En, we construct the
     *      combined regular expression e as;
     *
     *          e = (E1 #1) | (E2 #2) | ... | (En #n)
     *
     *      where #1,...,#n as endmarkers for an accept, represented by the
     *      non-character tokens ACCEPT/ACCEPT2. The DFA is then constructed from this
     *      expression. Only the fact of an accept position need be recorded as the
     *      value can be derived whilst analysising the expression.
     *
     *      Now any state within the "Deterministic finite automata" (DFA) which has a
     *      transition on #k (where k is one of 1,...,n) represents a state which shall
     *      be considered end of an expression.
     *
     *  Greedyness:
     *
     *      Generally when running a DFA there is no control on the way the expression
     *      returns a match, always returning the longest-leftmost match, no matter how
     *      you have crafted your expression.
     *
     *      DFAs compare each character of the input string to the regular expression,
     *      keeping track of all matches in progress. Since each character is examined
     *      at most once, the DFA engine is the fastest as the matching is in linear
     *      time. The down-side with DFAs is that the alternation metasequence is
     *      greedy. When more than one option in an alternation (foo|foobar) matches,
     *      the longest one is selected.
     *
     *      This greedy feature of DFA based expressions hence disallows the following
     *      two expression working together, as the second shall always match.
     *
     *          "(\*.*\*)" and "(\*.*$"         Block and open-block comments.
     *
     *      To signal a non-greedy construct, expressions prefixed with the character
     *      (0x01) shall force the DFA engine to stop when the related accept state is
     *      reached, without executing the DFA any further.
     *
     */
    end = pattern;
    for (i = 0; i < num_patterns; ++i) {
        const char c1 = patterns[i][0];

        if (c1) {
            if (pattern != end) {
                *end++ = '|';
            }
            if (0x01 == c1) {                   /* priority, accept2 */
                end += sprintf(end, "(%s)%c", patterns[i] + 1, CSET_ACCEPT2);
            } else {                            /* normal */
                end += sprintf(end, "(%s)%c", patterns[i], CSET_ACCEPT);
            }
        }
    }

    recomp.regex = regex;
    recomp.heap  = heap;
    recomp.flags = flags;
    recomp.temp  = temp;

    /* parse this expression */
    if (NULL != (rx = parse_expression(&recomp, pattern, end))) {
        int ret;

        /* regdfa to DFA conversion */
        if (0 == (ret = dfa_functions(&recomp, rx))) {
            ret = dfa_states(&recomp, rx);
        }

        if (0 == ret) {
            /* success,
             *    trim working storage
             *    release temporary storage
             *      warn of patterns which can not be reached
             *          note no warnings does not mean all patterns shall be matched
             *          due to the greedy nature of DFA based evaluations, only that all
             *          patterns are paths within the DFA.
             */
            if (regex && regex->table) {
                bitmap_t bm = bm_create(temp, num_patterns);

                if (bm) {
                    unsigned c;

                    bm_fill(bm);
                    for (c = 1; c < regex->cursor; ++c) {
                        int accepted = regex->table[c]->accept;

                        if (accepted) {
                            if (accepted < 0) { /* special short-ciruit states */
                                accepted *= -1;
                            }
                            if (--accepted < num_patterns) {
                                bm_clr(bm, accepted);
                            }
                        }
                    }

                    for (i = bm_first(bm); i >= 0; i = bm_next(bm, i)) {
                        ewprintf("regdfa: pattern '%s' not reached", patterns[i]);
                    }
                }
            }
            heap_destroy(temp);
            heap_shrink(heap);
            return regex;
        }
    }

    /* error release all resources */
error:;
    DFA_DEBUG(("regdfa: ret=NULL\n"))
    heap_destroy(temp);
    regdfa_destroy(regex);
    return NULL;
}


int
regdfa_check(const char *pattern)
{
    recompile_t recomp;
    dfaheap_t *temp = NULL;
    int ret = -1;

    memset(&recomp, 0, sizeof(recomp));
    if (NULL != (temp = heap_create(TEMP_SIZE/4))) {
        recomp.temp = temp;
        if (NULL != parse_expression(&recomp, pattern, pattern + strlen(pattern))) {
            ret = 0;
        }
        heap_destroy(temp);
    }
    return ret;
}


void
regdfa_destroy(struct regdfa *regex)
{
    if (regex) {
        chk_free((void *)regex->table);
        heap_destroy((dfaheap_t *)regex->heap);
    }
}


void
regdfa_export(struct regdfa *regex, FILE *fd)
{
    unsigned i, c;

    if (! regex || ! regex->table) {
        return;
    }
    fprintf(fd, "%u\n", regex->cursor);
    if (regex->table) {
        for (i = 1; i < regex->cursor; ++i) {
            struct dfastate *dfa = regex->table[i];

            fprintf(fd, "%3u:", i);
            fprintf(fd, "%3d:", dfa->accept);
            fprintf(fd, "%3d:", dfa->anchor);
            for (c = 0; c <= 255; ++c) {
#if defined(LOOKUP_INDEX)
                fprintf(fd, " %-2u",  dfa->lookup[c]);
#else
                fprintf(fd, " %-2u", (dfa->lookup[c] ? dfa->lookup[c]->index : 0));
#endif

            }
            fprintf(fd, "\n");
        }
    }
}


struct regdfa *
regdfa_import(FILE *fd)
{
    __CUNUSED(fd)
    return NULL;
}


static __CINLINE const dfastate_t *
move(const struct regdfa *regex, const dfastate_t *dfa, unsigned char ch)
{
    if (ch <= CSET_SPECIAL_MAX) ch = 0x7f;      /* remap specials, 22/09/10 */
#if defined(LOOKUP_INDEX)
    return regex->table[dfa->lookup[ch]];
#else
    return dfa->lookup[ch];
#endif
}


int
regdfa_match(struct regdfa *regex, const char *start, const char *end, int sol,
        const char **startp, const char **endp)
{
    const dfastate_t *dfa = regex->start;
    int accepted = 0;
    const char *s;

    if (sol) {                                  /* start-of-line */
        if (NULL == (dfa = move(regex, dfa, CSET_SOL))) {
            dfa = regex->start;
            sol = 0;
        } else {
            accepted = dfa->accept;
        }
    }

retry:;
    *startp = start;
    for (s = start; s < end; ++s) {             /* run DFA */
        register unsigned char ch;

        if ((ch = *((unsigned char *)s)) <= CSET_EOL) {
            ch = CSET_EOL + 1;                  /* map specials */
        }

        if (NULL == (dfa = move(regex, dfa, ch))) {
            if (sol && 0 == accepted) {         /* restart, ignore start-of-line */
                dfa = regex->start;
                sol = 0;
                goto retry;
            }
            break;
        }

        if (dfa->accept) {
            const int t_accept = dfa->accept;

            ED_TRACE(("regdfa: accept[%d]=<%.8s...>\n", t_accept, s))
            *endp = s;

            if (accepted && accepted != t_accept) {
                *startp = start;                /* new anchor */
            }

            if ((accepted = t_accept) < 0) {
                ED_TRACE(("regdfa: =%d\n", accepted * -1))
                return accepted * -1;           /* accept2 */
            }

        } else if (dfa->anchor) {
            ED_TRACE(("regdfa: anchor[%d]=<%.8s...>\n", dfa->anchor, s))
            *startp = s;
        }
    }

    if (dfa) {                                  /* end-of-line/string */
        assert(s == end);
        if (NULL != (dfa = move(regex, dfa, CSET_EOL)))
            if (dfa->accept) {
                *endp = s - 1;
                if ((accepted = dfa->accept) < 0) {
                    accepted *= -1;             /* accept2 */
                }
            }
    }

    ED_TRACE(("regdfa: =%d\n", accepted - 1))
    return (accepted - 1);                      /* return -1 on error, otherwise the pattern index */
}


int
regdfa_pmatch(struct regdfa *regex, const char *str, int sol, const char **startp, const char **endp)
{
    const char *start = NULL;
    size_t slen = strlen(str);
    int idx;

    while (slen--) {
        if ((idx = regdfa_match(regex, str, str + slen, sol, &start, endp)) >= 0) {
            *startp = (start ? start : str);
            return idx;                         /* match */
        }
        sol = (*++str == '\n' ? 1 : 0);
    }
    return -1;
}
/*end*/
