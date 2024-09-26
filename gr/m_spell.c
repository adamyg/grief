#include <edidentifier.h>
__CIDENT_RCSID(gr_m_spell_c,"$Id: m_spell.c,v 1.46 2024/07/20 09:22:39 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_spell.c,v 1.46 2024/07/20 09:22:39 cvsuser Exp $
 * Spell primitives.
 *
 *      Enchant - AbiWord spell-checker generic interface
 *          (aspell, ispell, hunspell, myspell, uspell, hspell, AppleSpell and few others)
 *
 *          http://www.abisource.com/projects/enchant
 *
 *      Hunspell
 *          http://sourceforge.net/hunspell/projects
 *          http://hunspell.sourceforge.net
 *              OpenOffice 1.x - based on myspell
 *
 *      aspell
 *          http://aspell.net
 *
 *      myspell
 *          http://linguscomponent.openoffice.org
 *              OpenOffice original - based on pspell now aspell
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

/*#define ED_LEVEL 1*/

#include <editor.h>
#include <edfileio.h>
#include <edthreads.h>
#include <edenv.h>                              /* gputenv(), ggetenv() */

#include <assert.h>
#include <stable.h>
#include <chkalloc.h>
#include <bsd_ndbm.h>
#if defined(WIN32)
#include <win32_misc.h>
#endif
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_spell.h"

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "debug.h"
#include "eval.h"
#include "lisp.h"                               /* lst_...() */
#include "lock.h"
#include "main.h"                               /* curbp */
#include "map.h"                                /* vm_...() */
#include "mchar.h"
#include "spell.h"
#include "sysinfo.h"                            /* sys_...() */

static void                 spell_init_impl(void);
static int                  spell_init_thread(void *p);
static LIST *               spell_suggest(const char *word, int length, int *llen);
static LIST *               spell_check_buffer(LINENO start, LINENO end, int token, int suggest, int unique, int *llen);
static LIST *               spell_check_string(BUFFER_t *bp, const char *buffer, int length, int tokenize, int suggest,
                                    const int line, stable_t *wordtbl, LIST *lst, int *llen);
static void                 spell_dbopen(void);
static void                 spell_dbclose(void);
static void                 spell_save(Spell_t *spell, const char *path);
static void                 spell_load(Spell_t *spell, const char *path);
static int                  spell_push(Spell_t *spell, const char *word);

static Spell_t * volatile   x_spell;
static const char *         x_spelldbname;
static DBM *                x_spelldb;


void
spell_init(void)
{
    if (NULL == x_spell) {
        /*
         *  reduce startup delays,
         *      hunspell can take a second or so too initialise.
         */
#if !defined(NO_THREADS)
#if defined(HAVE_LIBASPELL) && defined(sun)     /* libaspell threading issues reported */
#define NO_THREADS
#endif  //HAVE_LIBASPELL
#endif  //NOTHRREADS

#if !defined(NO_THREADS)
        thrd_t thr /*= 0*/;
        if (thrd_success == thrd_create(&thr, spell_init_thread, NULL)) {
            thrd_detach(thr);
            thrd_yield();
            return;
        }
#endif
        spell_init_impl();
    }
}


static int
spell_init_thread(void *p)
{
    __CUNUSED(p)
    spell_init_impl();
    return 0;
}


static void
spell_init_impl(void)
{
    /*
     *  Environment resources:
     *      One of the following
     *          GRDICTIONARY    Comma separated list of dictionary names ("en_GB,en_US").
     *          GRDICTIONARIES  Dictionary search path.
     *
     *      Plus
     *          DICTIONARY      Comma separated list of dictionary names ("en_GB,en_US").
     *          DICPATH         Dictionary search path.
     *
     *          WORDLIST        Private dictionary path.
     *
     *  otherwise locale, of the form:
     *      <language[_territory]>[.<encoding>>][@<modifier>]
     */
    const char *dictpaths[8+1] = {0}, *bdictionaries = NULL;
    const char *dicts[32+1] = {0}, *dict = NULL;
    char locale[128] = {0};
    size_t cnt;

    if (NULL == (dict = ggetenv("GRDICTIONARY")) &&
                NULL == (dict = ggetenv("DICTIONARY"))) {
        const char *env;

        if ((NULL != (env = ggetenv("LC_ALL")) && *env) ||
                (NULL != (env = ggetenv("LC_CTYPE")) && *env) ||
                (NULL != (env = ggetenv("LANG")) && *env)) {
            char *delim;                        /* isolate language */

            strxcpy(locale, env, sizeof(locale));
            if (NULL != (delim = strchr(locale, '.'))) *delim = '\0';
            if (NULL != (delim = strchr(locale, '@'))) *delim = '\0';
            if (NULL != (delim = strchr(locale, '-'))) *delim = '_';

            if (0 == strcmp(locale, "C") || 0 == strcmp(locale, "POSIX")) {
                dict = "en_US";
            } else {
                dict = locale;
            }
        }

        if (NULL == dict) {
            locale[0] = 0;
#if defined(WIN32)
            if (w32_getlanguage(locale, sizeof(locale))) {
                strxcat(locale, ",", sizeof(locale));
            }
#endif
            strxcat(locale, "en_US,en_GB,default", sizeof(locale));
            dict = locale;
        }
    }
                                                /* Split language components */
    for (cnt = 0; *dict && cnt < ((sizeof(dicts)/sizeof(dicts[0]))-1);) {
        const char *comma = strchr(dict, ',');
        const size_t len = (comma ? (size_t)(comma - dict) : strlen(dict));

        dicts[cnt++] = chk_snalloc(dict, len);
        dict += len + (comma ? 1 : 0);
    }

    if (NULL != (bdictionaries = ggetenv("GRDICTIONARIES")) ||
                NULL != (bdictionaries = ggetenv("DICPATH"))) {
        /*
         *  Split GRDICTIONARIES/DICPATH into path components
         */
        for (cnt = 0; *bdictionaries && cnt < ((sizeof(dictpaths)/sizeof(dictpaths[0]))-1);) {
            const char *delim = strchr(bdictionaries,  FILEIO_DIRDELIM);
            const size_t len = (delim ? (size_t)(delim - bdictionaries) : strlen(bdictionaries));

            dictpaths[cnt++] = chk_snalloc(bdictionaries, len);
            bdictionaries += len + (delim ? 1 : 0);
        }
    }

#if defined(HAVE_LIBENCHANT)
    if (NULL == x_spell) {
        x_spell = spell_enchant_init(dicts);
    }
#endif
#if defined(HAVE_LIBHUNSPELL) || defined(WIN32)
    if (NULL == x_spell) {
        x_spell = spell_hunspell_init(dicts, dictpaths);
    }
#endif
#if !defined(HAVE_LIBENCHANT)
#if defined(HAVE_LIBASPELL)
    if (NULL == x_spell) {
        x_spell = spell_aspell_init(dicts);
    }
#elif defined(HAVE_LIBMYSPELL)
    if (NULL == x_spell) {
        x_spell = spell_myspell_init(dicts);
    }
#elif defined(HAVE_LIBSPELL)
    if (NULL == x_spell) {
        x_spell = spell_lib_init(dicts);
    }
#endif
#endif  /*!HAVE_ENCHANT*/

    for (cnt = 0; NULL != (dict = dicts[cnt]); ++cnt) {
        chk_free((void *)dict);
    }
    for (cnt = 0; NULL != (dict = dictpaths[cnt]); ++cnt) {
        chk_free((void *)dict);
    }
}


void
spell_close(void)
{
    Spell_t *spell;

    spell_dbclose();
    if (NULL != (spell = x_spell)) {
        x_spell = NULL;
        if (spell->sf_close) {
            (*spell->sf_close)(spell);
        }
    }
}


static LIST *
spell_suggest(const char *word, int length, int *llen)
{
    char t_word[128 + 1];

    assert(word);
    if (x_spell && x_spell->sf_suggest) {
        const char **ret = NULL;

        if (length >= 0) {
            if (length <= 128) {
                memcpy(t_word, word, length);
                t_word[length] = 0;
                ret = (*x_spell->sf_suggest)(x_spell, 0, (const char *)t_word, length);

            } else {
                char *xword;
                if (NULL != (xword = chk_snalloc(word, length))) {
                    ret = (*x_spell->sf_suggest)(x_spell, 0, (const char *)xword, length);
                    chk_free(xword);
                }
            }
            ED_TRACE(("spell_suggest(%.*s) : %p\n", length, word, ret))

        } else {
            ret = (*x_spell->sf_suggest)(x_spell, 0, word, (int)strlen(word));
            ED_TRACE(("spell_suggest(%s) : %p\n", word, ret))
        }

        if (ret) {
            const char *suggestion;
            int words = 0, r;

            for (r = 0; NULL != (suggestion = ret[r]); ++r) {
                if (*suggestion) {
                    trace_log("\t==> <%s>\n", suggestion);
                    ++words;
                }
            }

            if (words > 0) {
                const int suglen = (words * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
                LIST *suglst = NULL;

                if (NULL != (suglst = lst_alloc(suglen, words))) {
                    LIST *lp = suglst;

                    for (r = 0; NULL != (suggestion = ret[r]); ++r) {
                        if (*suggestion) {
                            lp = atom_push_str(lp, suggestion);
                            --words;
                        }
                    }
                    assert(0 == words);
                    *llen = suglen;
                    atom_push_halt(lp);
                    return suglst;
                }
            }
        }
    }
    *llen = 0;
    return NULL;
}


static LIST *
spell_check_buffer(LINENO start, LINENO end, int tokenize, int suggest, int unique, int *llen)
{
    BUFFER_t *bp = curbp;

#define TABLESIZE       512
#define TABLEFACTOR     4
    if (start <= 0) {
        start = 1;
    }
    if (end < 0 || end >= bp->b_numlines) {
        end = bp->b_numlines;
    }

    /*
     *  Buffer specific next-character
     *    "       "     next-word
     *                      -> syntax and/or character-set
     *  Buffer specific keyword
     *                      -> syntax lookup
     */
    ED_TRACE(("spell_check_buffer(start:%d, end:%d)\n", (int) start, (int) end))
    if (start <= end) {
        stable_t wordtbl = {0};
        LINENO line;
        LIST *lst = NULL;
        int len = 0;

        stbl_open(&wordtbl, TABLESIZE, TABLEFACTOR);

        for (line = start; line < end; ++line) {
            LINE_t *clp;

            if (NULL != (clp = vm_lock_line2(line))) {
                const LINECHAR *text;
                int length;

                if (NULL != (text = ltext(clp)) && (length = llength(clp)) > 0) {
                    lst = spell_check_string(bp, (const char *)text, length,
                                tokenize, suggest, line, (unique ? &wordtbl : NULL), lst, &len);
                }
                vm_unlock(line);
            }
        }
        stbl_clear(&wordtbl);
        if (lst) {
            *llen = len;
            return lst;
        }
    }
    *llen = 0;
    return NULL;
}


//  Parameters:
//      buffer -  Base address of the buffer.
//      length -  Buffer length in bytes.
//      wordlen - [in/out] Length of previous word, populated with length of next word.
//      offset -  [in/out] Offset within buffer, populated with starting offset of next word.
//      column -  [in/out] Column position, populated with column of next word.
//      chars -   [in/out] Character within previous word. populated with the character count of the next word.
//
const char *
spell_nextword(BUFFER_t *bp,
        const char *buffer, const int length, int *wordlen, int *offset, int *chars, int *column)
{
    static const unsigned char punctuationchars[] = "_-'";
#if defined(__clang__)
#pragma clang diagnostic ignored "-Winvalid-source-encoding"
#endif
    static const unsigned char wordchars[] =    /* XXX/FIXME - ascii/latin-1 plus backspace */
            "\b" "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZéáúõûóüöíÉÁÕÚÖÜÓÛÍ";
    static unsigned char wordtable[256] = {0};

    mchar_iconv_t *iconv = bp->b_iconv;         /* character conversion */

    int t_offset = *offset + *wordlen,
            t_column = *column + *chars;

    const unsigned char *cursor = ((const unsigned char *)buffer) + t_offset,
            *end = ((const unsigned char *)buffer) + length;

    int32_t ch, wraw;

    if (0 == wordtable[0]) {                    /* once, TODO -- spell_init() */
        const unsigned char *c;

        for (c = wordchars; *c; ++c)
            wordtable[*c] = 1;
        for (c = punctuationchars; *c; ++c)
            wordtable[*c] = 2;
        wordtable[0] = 1;
    }

    while (cursor < end && *cursor) {           /* FIXME - character set specific isword() */
        const LINECHAR *t_cursor =
                iconv->ic_decode(iconv, cursor, end, &ch, &wraw);

        if ('_' == ch && BFTST(bp, BF_MAN)) {   /* _\bX */
            if ((t_cursor + 1) < end && '\b' == *t_cursor) {
                int32_t t_ch = 0;

                if (iconv->ic_decode(iconv, t_cursor + 1, end, &t_ch, &wraw) &&
                        1 == wordtable[t_ch]) {
                    ch = 'a';
                }
            }
        }

        if (ch <= 0xff && 1 == wordtable[ch]) {
            const LINECHAR *start = cursor;
            int t_wordlen, t_chars = 1;
            int32_t ch2 = 0;

            *offset = t_offset;
            *column = t_column;
            while (cursor < end &&              /* decode or character_decode() */
                    (t_cursor = iconv->ic_decode(iconv, cursor, end, &ch, &wraw)) != NULL &&
                        ch < 0xff && wordtable[ch]) {
                cursor = t_cursor;
                ++t_chars;
                ch2 = ch;
            }
            if ((t_wordlen = cursor - start) > 1 &&
                        ch2 < 0xff && 2 == wordtable[ch2]) {
                --t_wordlen;                    /* trailing punctuation, remove */
                --t_chars;
            }
            *wordlen = t_wordlen;
            *chars = t_chars;
            return (const char *)start;
        }
        ++t_offset, ++t_column;
        cursor = t_cursor;
    }
    return NULL;
}


static LIST *
spell_check_string(BUFFER_t *bp, const char *buffer, int length, int tokenize, int suggest,
        const int line, stable_t *wordtbl, LIST *lst, int *len)
{
    int wordlen = 0, offset = 0, column = 1, chklen = 0, chars = 0;
    const int ielements = ((wordtbl ? 1 : 0) + (line > 0 ? 1 : 0) + 2);
    LIST *lp = NULL, *chklst;
    const char *word;

    __CUNUSED(tokenize)

    if (NULL != (chklst = lst)) {
        chklen = (*len - sizeof_atoms[F_HALT]);
        lp = chklst + chklen;
    }

    while (NULL != (word = spell_nextword(bp, buffer, length, &wordlen, &offset, &chars, &column))) {
        int ret;

        if (wordtbl) {
            stblnode_t *dupword;

            if (NULL != (dupword = stbl_nfind(wordtbl, word, wordlen))) {
                LIST *t_lp = chklst + dupword->stbl_ui32;
                accint_t val = 1;

                atom_xint(t_lp, &val);          /* increment count */
                atom_push_int(t_lp, val + 1);
                continue;
            }
        }

        if ((ret = spell_check(word, wordlen)) > 0) {
            LIST *suglst = NULL;
            int suglen = 0, newlen;

            /* check personal dictionary */

            /* optional suggest list */
            if (suggest) {
                suglst = spell_suggest(word, wordlen, &suglen);
            }

            /* size node, alloc or extend */
            newlen = sizeof_atoms[F_RSTR] +
                        (suglst ? sizeof_atoms[F_RLIST] : sizeof_atoms[F_NULL]) +
                        (ielements * sizeof_atoms[F_INT]);

            if (NULL == chklst) {
                if (NULL == (chklst = lp = lst_alloc(newlen + sizeof_atoms[F_HALT], ielements + 2))) {
                    return NULL;
                }
            } else {
                LIST *newlp;
                if (NULL == (newlp = lst_extend(chklst, newlen, ielements + 2))) {
                    break;
                }
                assert((lp - chklst) == chklen);
                lp = (chklst = newlp) + chklen;
            }
            chklen += newlen;

            /* push node ---
             *
             *      [<word>, <suggest-list|NULL>, <offset>, <column>, <line>, <count>]
             *         :             :              :         :         :       :
             */
            lp = atom_push_nstr(lp, word, wordlen);
            if (suglst) {                       /* suggest-list|NULL */
                lp = atom_push_ref(lp, rlst_create(suglst, suglen));
            } else {
                lp = atom_push_null(lp);
            }
            lp = atom_push_int(lp, offset);     /* offset */
            lp = atom_push_int(lp, column);     /* column */
            if (line > 0) {                     /* [line] */
                lp = atom_push_int(lp, line);
            }
            if (wordtbl) {                      /* [count] */
                stblnode_t *dupword;

                if (NULL != (dupword = stbl_nnew(wordtbl, word, wordlen))) {
                    dupword->stbl_ui32 = lp - chklst;
                }
                lp = atom_push_int(lp, (accint_t) 1);
            }
        }
        ED_TRACE(("\t==> word <%.*s> : %d\n", wordlen, word, ret))
    }

    if (chklst) {
        lp = atom_push_halt(lp);
        chklen += sizeof_atoms[F_HALT];
        *len = chklen;
        assert((lp - chklst) == chklen);
    }

    ED_TRACE(("spell_check_string(length:%d, buffer:%.*s) : %d\n", length, length, buffer, chklen))
    trace_flush();
    return chklst;
}


int
spell_check(const char *word, int length)
{
    const int olength = length;
    char t_word[128 + 1];
    int ret = -1;

    assert(word);
    __CUNUSED(olength);
    if (x_spell && x_spell->sf_check) {
        if (length >= 0) {
            if (length <= 128) {                // local buffer
                if (length > 1 && '\b' == word[1]) {
                    const char *cursor = word, *end = word + length;

                                                // unman word
                    for (length = 0; cursor < end; ++cursor) {
                        if ('\b' == *cursor) {
                            if (length) --length;
                        } else {
                            t_word[length++] = *cursor;
                        }
                    }
                } else {
                    memcpy(t_word, word, length);
                }
                t_word[length] = 0;
                ret = (*x_spell->sf_check)(x_spell, 0, (const char *)t_word, length);

            } else {
                char *nword;

                if (NULL != (nword = chk_snalloc(word, length))) {
                    ret = (*x_spell->sf_check)(x_spell, 0, (const char *)nword, length);
                    chk_free(nword);
                }
            }
        } else {
            ret = (*x_spell->sf_check)(x_spell, 0, word, (int)strlen(word));
        }
    }
    ED_TRACE(("spell_check(%.*s) : %d\n", olength, word, ret))
    return ret;
}


static void
spell_save(Spell_t *spell, const char *path)
{
    FILE *fd;

    __CUNUSED(spell)

    if (path && path[0]) {
        if (NULL != (fd = fopen(path, "w"))) {
            /*
             *  TODO ...
             */
            fclose(fd);
        }
    }
}


static void
spell_load(Spell_t *spell, const char *path)
{
    if (path && path[0]) {
        char buffer[1024 + 1];
        FILE *fd;

        if (NULL != (fd = fopen(path, "r"))) {
            buffer[sizeof(buffer) - 1] = 0;
            while (NULL != fgets(buffer, sizeof(buffer) - 1, fd)) {
                const size_t len = strlen(buffer);
                if (len > 1) {
                    if ('\n' == buffer[len - 1]) {
                        buffer[len - 1] = 0;
                    }
                    spell_push(spell, buffer);
                }
            }
            fclose(fd);
        }
    }
}


static void
spell_dbopen(void)
{
    if (! x_spelldb) {
        const char *bpersonal = ggetenv("GRPERSONAL");

        if (NULL != bpersonal ||
                NULL != (bpersonal = sysinfo_homedir(NULL, -1))) {
            /*
             *  Open/create local/personal dictionary
             */
            if (NULL == x_spelldbname) {
                char personaldb[MAX_PATH];
                sxprintf(personaldb, sizeof(personaldb), "%s/.grspell", bpersonal);
                x_spelldbname = chk_salloc(personaldb);
            }

            if (0 == flock_set(x_spelldbname, FALSE)) {
                x_spelldb = bsddbm_open(x_spelldbname, O_RDWR|O_CREAT, 0666);
            }
        }
    }
}


int
spell_dblookup(const char *word)
{
    if (x_spelldb)  {
        datum result, key = {0};

        key.dptr = (void *)word;
        key.dsize = strlen(word);
        result = bsddbm_fetch(x_spelldb, key);
        if (result.dptr && result.dsize) {
            return TRUE;
        }
    }
    return FALSE;
}


static void
spell_dbclose(void)
{
    if (x_spelldb) {
        bsddbm_close(x_spelldb);
        flock_clear(x_spelldbname);
        x_spelldb = NULL;
    }
}


static int
spell_push(Spell_t *spell, const char *word)
{
    int ret = -1;

    if (spell) {
        switch (*word) {
        case '+':       /* +<add> */
            if (spell->sf_add) {
                ret = (*spell->sf_add)(spell, 0, word, NULL);
            }
            break;

        case '/':       /* /<word>/[/]<pattern>[/] */
            if (spell->sf_add) {
                char *word2;

                if (NULL != (word2 = (char *)strstr(++word, "/"))) {
                    *word2++ = 0;
                    if (*word2 == '/') {
                        ++word2;
                    }
                    ret = (*spell->sf_add)(spell, 0, word, word2);
                }
            }
            break;

        case '-':       /* -<remove> */
            if (spell->sf_remove) {
                ret = (*spell->sf_remove)(spell, 0, word);
            }
            break;

        case '*':       /* *<ignore> */
            if (spell->sf_add) {
                ret = (*spell->sf_add)(spell, 0, word, NULL);
            }
            break;
        }
    }
    return ret;
}


/*  Function:           EditDistance
 *      Edit distance computes the "distance" between two words by counting
 *      the number of insert, replace, and delete operations required to
 *      permute one word into another. In general the fewer operations
 *      required, the closer the match. Some implementations assign varying
 *      scores to the insert, replace, and delete operations. Another common
 *      variation varies which operations are considered when computing the
 *      distance; for example, the replace operation may not be considered,
 *      thereby defining the edit distance solely in terms of insert and
 *      delete options.
 *
 *      This algorithm uses one of the more popular algorithms in the edit
 *      distance known as the "Levenshtein distance".
 *
 *  Parameters:
 *       s1 - String one
 *       s2 - Second string.
 *
 *  Returns:
 *      Edit cost.
 */
static unsigned int
EditDistance(const char *s1, const char *s2)
{
    unsigned  cost_del = 1;                     // TODO, allow costing
    unsigned  cost_ins = 1;
    unsigned  cost_sub = 1;
    unsigned  n1 = (unsigned)strlen(s1);
    unsigned  n2 = (unsigned)strlen(s2);
    unsigned *dist = chk_alloc(sizeof(unsigned) * (n1+1) * (n2+1));
    unsigned  a, b, i, j, r;

    if (NULL == dist) {
        return 0;
    }

    dist[0] = 0;

    for (a = 1; a <= n1; ++a) {
        dist[a*(n2+1)] = dist[(a-1)*(n2+1)] + cost_del;
    }

    for (b = 1; b <= n2; ++b) {
        dist[b] = dist[b-1] + cost_ins;
    }

    for (i = 1; i <= n1; ++i)
        for (j = 1; j <= n2; ++j) {
            unsigned dist_del = dist[(i-1) * (n2+1) + (j )]  + cost_del;
            unsigned dist_ins = dist[(i )  * (n2+1) + (j-1)] + cost_ins;
            unsigned dist_sub = dist[(i-1) * (n2+1) + (j-1)] + (s1[i-1] == s2[j-1] ? 0 : cost_sub);

#ifndef __MIN
#define __MIN(X,Y)  ((X) < (Y) ? (X) : (Y))
#endif
            dist[i*(n2+1) + j] = __MIN(__MIN(dist_del, dist_ins), dist_sub);
#undef  __MIN
        }

    r = dist[n1*(n2+1) + n2];
    chk_free(dist);
    return r;
}


/*  Function:           do_spell_suggest
 *      spell_buffer() and spell_string() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: spell_buffer - Spell the specified buffer

        list
        spell_buffer(int start_line, [int end_line],
                [int tokenize = 1], [int suggest])

    Macro Description:
        The 'spell_buffer()' primitive spell checks the specified
        lines within the current buffer.

        The string passed in should only be split on white space
        characters.

        Furthermore, between calls to reset each string should be
        passed in exactly once and in the order they appeared in the
        document. Passing in stings out of order, skipping strings or
        passing them in more than once may lead to undefined results.

    Macro Parameters:
        start_line - Integer line number, being the first line at
            which spell

        end_line - Optional integer line number, denoting the line
            at which spell check shall complete. If omitted checks
            are performed until the end-of-buffer.

        tokenize - Optional integer flag, if specified as zero
            the buffer content shall not be split into tokens during
            parsing.

        suggest - Optional integer flag, if specified as non-zero
            spelling suggestions shall be returned against each
            misspelt word.

    Macro Returns:
        The 'spell_buffer()' primitive returns a list of possible
        spelling errors in the form.

>           [<word>, <suggest-list|NULL>, <offset>, <column>, <line> [, <count>]]

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance

 *<<GRIEF>>
    Macro: spell_string - Spell the specified word or line.

        int
        spell_string(string word, [int length],
                [int tokenize = 0], [int suggest = FALSE])

    Macro Description:
        The 'spell_string()' primitive spell checks the specified
        string 'word'

        The string passed in should only be split on white space
        characters.

        Furthermore, between calls to reset each string should be
        passed in exactly once and in the order they appeared in the
        document. Passing in stings out of order, skipping strings or
        passing them in more than once may lead to undefined results.

    Macro Parameters:
        word - String containing the text to be checked.

        length - Optional integer, stating the number of characters
            within the string to be parsed.

        tokenize - Optional integer flag, if specified as non-zero
            the string content shall be split into tokens during
            parsing.

        suggest - Optional integer flag, if specified as non-zero
            spelling suggestions shall be returned against each
            misspelt word.

    Macro Returns:
        The 'spell_string()' primitive returns a value dependent on
        the input mode.

        For word checks, 0 on success, other non-zero.

        For string or line checks, list of possible incorrect words
        offset/line + length pairs with optional suggestion list,
        with offsets starting from 1.

>         [<word>, <suggest-list|NULL>, <offset>, <column>]

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance
 */
void
do_spell_check(int mode)        /*   ([string word], [int length], [int tokenize = 0], [int suggest = FALSE])
                                  or (int startline, [int endline], [int tokenize = 1], [int suggest = FALSE], [int unique = TRUE]) */
{
    switch (mode) {
    case 1:     /* string_string */
        if (isa_string(1)) {                    /* string */
            const char *str    = get_str(1);
            const int length   = get_xinteger(2, get_strlen(1));
            const int tokenize = get_xinteger(3, 0);
            const int suggest  = get_xinteger(4, FALSE);

            if (0 == tokenize) {                /* word */
                if (NULL == x_spell) {
                    acc_assign_int(-1);
                } else {
                    acc_assign_int(spell_check(str, length));
                }

            } else {                            /* line */
                LIST *lst = NULL;
                int len = 0;

                if (NULL == x_spell ||
                        NULL == (lst = spell_check_string(curbp, str, length, tokenize, suggest, 0, NULL, NULL, &len))) {
                    acc_assign_null();
                } else {
                    acc_donate_list(lst, len);
                }
            }
            return;
        }
        acc_assign_int(-1);
        break;

    case 2:     /* string_buffer */
        if (isa_integer(1)) {                   /* buffer region */
            const int startline = get_xinteger(1, -1);
            const int endline   = get_xinteger(2, -1);
            const int tokenize  = get_xinteger(3, 1);
            const int suggest   = get_xinteger(4, FALSE);
            const int unique    = get_xinteger(5, TRUE);
            LIST *chklp = NULL;
            int chklen = 0;

            if (NULL == x_spell ||
                    NULL == (chklp = spell_check_buffer(startline, endline, tokenize, suggest, unique, &chklen))) {
                acc_assign_null();
            } else {
                acc_donate_list(chklp, chklen);
            }
            return;

        } else {
            /* TODO -- current word under cursor within buffer */
        }
        acc_assign_null();
        return;
    }
}


/*  Function:           do_spell_suggest
 *      spell_suggest() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: spell_suggest - Suggest spelling of the the specified word.

        list
        spell_suggest(string word, [int length])

    Macro Description:
        The 'spell_distance()' primitive spell checks the specified
        'word' building a list of possible suggestions.

    Macro Parameters:
        word - xxx
        length - xxx

    Macro Returns:
        The 'spell_suggest()' primitive returns a list of possible
        suggestions, otherwise null.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance
 */
void
do_spell_suggest(void)          /* list (string word, [int length]) */
{
    const char *word = get_str(1);
    const int length = get_xinteger(2, get_strlen(1));

    if (NULL == x_spell) {
        acc_assign_null();
    } else {
        int sugllen = 0;
        LIST *suglst = spell_suggest(word, length, &sugllen);

        if (NULL == suglst) {
            acc_assign_null();
        } else {
            acc_donate_list(suglst, sugllen);
        }
    }
}


/*  Function:           do_spell_control
 *      spell_control() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: spell_control - Spell control

        declare
        spell_control(int action, ...)

    Macro Description:
        The 'spell_control()' primitive manipulates the spell engine
        attribute associated with the specified 'action'. Additional
        arguments are specific to the action or attribute being
        modified.

    Macro Parameters:
        action - Integer identifier of the engine attribute to be
            manipulated.

        ... - Action specific value.

    Modes::

(table,format=nd)
        [Action             [Description                        ]

     !  SPELL_DESCRIPTION   Spell implementation description,
                            returns a string containing the name
                            of the current speller.

     !  SPELL_DICTIONARIES  List of available dictionaries,
                            retrieves a list of string each the
                            name of a available dictionary.

     !  SPELL_LOAD          Load a personal dictionary.

     !  SPELL_SAVE          Save to a personal dictionary.

     !  SPELL_ADD           Add to the personal dictionary.

     !  SPELL_IGNORE        Add to the spell ignore list.

     !  SPELL_REPLACE       Add to the spell replacement list.

     !  SPELL_LANG_ADD      n/a

     !  SPELL_LANG_PRIMARY  n/a

     !  SPELL_LANG_REMOVE   n/a
(end)

    Macro Returns:
        The 'spell_control()' primitive usually, on success returns
        zero, otherwise -1 on error.

        A few 'spell_control' requests return non-integer values,
        either a string or list, based on the attribute that was
        manipulated by the specified 'action', see table above.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance
 */
void
do_spell_control(void)          /* int (int action, ...) */
{
    const int action = get_xinteger(1, 0);
    int ret = -1;

    switch (action) {
    case SPELL_DESCRIPTION: {       /* spell implementation description */
            const char *desc = "none";

            if (x_spell) {
                desc = x_spell->sf_description;
            }
            acc_assign_str(desc, -1);
        }
        return;

    case SPELL_DICTIONARIES: {      /* list of available dictionaries */
            const int flags = get_xinteger(2, 0);
            const char **dicts = NULL;

            if (x_spell && NULL != (dicts = (*x_spell->sf_dictionaries)(x_spell, flags))) {
                int dictlen, dictcnt = 0;
                LIST *dictlp, *lp;

                while (dicts[dictcnt]) {
                    ++dictcnt;
                }
                dictlen = (dictcnt * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
                if (NULL != (dictlp = lst_alloc(dictlen, dictcnt))) {
                    lp = dictlp;
                    while (*dicts) {
                        lp = atom_push_str(lp, *dicts);
                        ++dicts;
                    }
                    atom_push_halt(lp);
                    acc_donate_list(dictlp, dictlen);
                } else {
                    dicts = NULL;
                }
            }
            if (NULL == dicts) {
                acc_assign_null();
            }
        }
        return;
    }

    if (NULL == x_spell) {
        acc_assign_int(-1);
        return;
    }

    switch (action) {
    case SPELL_LOAD: {              /* load personal dictionary */
            const char *path = get_xstr(2);

            if (path && *path) {
                spell_load(x_spell, path);
            }
        }
        break;

    case SPELL_SAVE: {              /* save personal dictionary */
            const char *path = get_xstr(2);

            if (path && *path) {
                spell_save(x_spell, path);
            }
        }
        break;

    case SPELL_LANG_ADD:            /* language control */
    case SPELL_LANG_PRIMARY:
    case SPELL_LANG_REMOVE:
        //TODO - language selection
        break;

    case SPELL_ADD:                 /* add to the personal dictionary. */
        if (x_spell->sf_add) {
            const char *word = get_xstr(2);

            if (word && *word) {
                ret = (*x_spell->sf_add)(x_spell, 0, word, NULL);
            }
        }
        break;

    case SPELL_IGNORE:              /* add to the spell ignore list. */
        if (x_spell->sf_add) {
            const char *word = get_xstr(2);

            if (word && *word) {
                ret = (*x_spell->sf_add)(x_spell, 0, word, NULL);
            }
        }
        break;

    case SPELL_REPLACE:             /* add to the spell replacement list. */
        if (x_spell->sf_add) {
            const char *word = get_xstr(2), *word2 = get_xstr(3);

            if (word && *word && word2) {
                ret = (*x_spell->sf_add)(x_spell, 0, word, word2);
            }
        }
        break;
    }
    acc_assign_int(ret);
}



/*  Function:           do_spell_dictionary
 *      spell_dictionary() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: spell_dictionary - Spell dictionary modifications.

        int
        spell_dictionary(int, string|list)

    Macro Description:
        The 'spell_dictionary()' primitive is reserved.

    Macro Parameters:
        n/a

    Macro Returns:
        The 'spell_dictionary()' primitive returns -0 on success,
        otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance
 */
void
do_spell_dictionary(void)           /* (int, string|list) */
{
    //TODO
}


/*  Function:           do_spell_distance
 *      spell_distance() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: spell_distance - Edit distance

        int
        spell_distance(string a, string b)

    Macro Description:
        The 'spell_distance()' primitive computes the "distance"
        between two words by counting the number of insert, replace,
        and delete operations required to permute one word into
        another.

        In general the fewer operations required, the closer the
        match. Some implementations assign varying scores to the
        insert, replace, and delete operations. Another common
        variation varies which operations are considered when
        computing the distance; for example, the replace operation
        may not be considered, thereby defining the edit distance
        solely in terms of insert and delete options.

        This algorithm uses one of the more popular algorithms in the
        edit distance known as the "Levenshtein distance".

        The costs assigned to each move are equal, that is of the
        following operations have a cost of '1'.

            o Delete a character
            o Insert a character
            o Character substitution.

    Macro Parameters:
        s1 - String one.
        s2 - Second string.

    Macro Returns:
        The 'spell_distance()' primitive returns the edit distance
        between the two words.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        spell_buffer, spell_string, spell_suggest, spell_control,
        spell_distance
 */
void
do_spell_distance(void)         /* int (string a, string b) */
{
    const char *s1 = get_str(1), *s2 = get_str(2);
    unsigned ret;

    ret = EditDistance(s1, s2);
    acc_assign_int(ret);
}

/*end*/
