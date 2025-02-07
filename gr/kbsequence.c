#include <edidentifier.h>
__CIDENT_RCSID(gr_kbsequence_c,"$Id: kbsequence.c,v 1.4 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbsequence.c,v 1.4 2025/02/07 03:03:21 cvsuser Exp $
 * Keyboard input sequences.
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
#include <assert.h>

#include <edalt.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "kbsequence.h"

#define USE_LIBTRIE

#if defined(USE_LIBTRIE)
#include "libtrie.h"
#else
#include "libsplay.h"
#endif

#if defined(USE_LIBTRIE)
static struct trie *x_kseqtrie;                 /* trie of key assignments */
#else
static SPTREE *x_kseqtree;                      /* splay-tree of key assignments */
#endif


/*  Function:       kbsequence_init
 *      Run-time initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      none
 */
void
kbsequence_init(void)
{
#if defined(USE_LIBTRIE)
    if (NULL == x_kseqtrie) {
        x_kseqtrie = trie_create();
    }
#else
    if (NULL == x_kseqtree) {
        x_kseqtree = spinit();
    }
#endif
}


/*  Function:       kbsequence_shutdown
 *      Run-time shutdown/cleanup.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      none
 */
void
kbsequence_shutdown(void)
{
#if defined(USE_LIBTRIE)
    if (x_kseqtrie) {
        struct trie_it *it = trie_it_create(x_kseqtrie, NULL);

        if (it) {
            if (! trie_it_done(it)) {
                do {
                    void *data = trie_it_data(it);
                    chk_free(data);
                } while (trie_it_next(it));
            }
            trie_it_free(it);
        }
        trie_free(x_kseqtrie);
        x_kseqtrie = NULL;
    }

#else
    if (x_kseqtree) {
        while (! spempty(x_kseqtree)) {
            SPBLK *sp = sphead(x_kseqtree);

            spdeq(sp, x_kseqtree);
            chk_free(sp->data);
            spfreeblk(sp);
        }
        spfree(x_kseqtree);
        x_kseqtree = NULL;
    }

#endif //USE_LIBTRIE
}


/*  Function:       kbsequence_lookup
 *      Lookup a sequence association.
 *
 *  Parameters:
 *      seq - Sequence buffer.
 *
 *  Results:
 *      Key sequence definition.
 */
const keyseq_t *
kbsequence_lookup(const char *seq)
{
#if defined(USE_LIBTRIE)
    if (x_kseqtrie) {
        return (const keyseq_t *)trie_search(x_kseqtrie, seq);
    }

#else
    SPBLK *sp;
    if (NULL != (sp = splookup(seq, x_kseqtree))) {
        return (const keyseq_t *) sp->data;
    }

#endif //USE_LIBTRIE
    return NULL;
}


/*  Function:       kbsequence_update
 *      Insert or update the key sequence association.
 *
 *  Parameters:
 *      seq - Sequence buffer.
 *      key - Key to be associated.
 *
 *  Results:
 *      Key sequence definition.
 */

#if defined(USE_LIBTRIE)
struct ReplacerArguments {
    const keyseq_t *ks;
    const char *seq;
    unsigned seqlen;
    KEY key;
};

static void *
replacer(const char *key, void *old_data, void *uarg)
{
    struct ReplacerArguments *arg = (struct ReplacerArguments *)(uarg);
    keyseq_t *ks = (keyseq_t *)old_data;

    __CUNUSED(key)

    if (ks != NULL) {                           // update
        assert(0 == strcmp(ks->ks_buf, arg->seq));

    } else {                                    // addition
        if (NULL == (ks = chk_alloc(sizeof(keyseq_t) + arg->seqlen))) {
            return NULL;
        }
        memcpy(ks->ks_buf, arg->seq, arg->seqlen + 1);
    }
    ks->ks_code = arg->key;
    return (void *)(arg->ks = ks);
}
#endif //USE_LIBTRIE

const keyseq_t *
kbsequence_update(const char *seq, KEY key)
{
    const unsigned seqlen = (unsigned)strlen(seq);
#if defined(USE_LIBTRIE)
    struct ReplacerArguments arg = {NULL, seq, seqlen, key};

    kbsequence_init();
    trie_replace(x_kseqtrie, seq, replacer, &arg);
    assert(NULL != arg.ks);
    return arg.ks;

#else
    SPBLK *sp;
    keyseq_t *ks;

    kbsequence_init();
    if (NULL != (sp = splookup(seq, x_kseqtree))) {
        ks = sp->data;
        assert(0 == strcmp(ks->ks_buf, seq));

    } else {
        sp = spblk(sizeof(keyseq_t) + seqlen);
        ks = (keyseq_t *) sp->data;
        memcpy(ks->ks_buf, seq, seqlen + 1);
        sp->key = ks->ks_buf;
        spenq(sp, x_kseqtree);
    }
    ks->ks_code = key;
    return ks;

#endif //USE_LIBTRIE
}


/*  Function:       kbsequence_match
 *      Retrieve sequence match.
 *
 *  Parameters:
 *      seq - Sequence buffer.
 *      seqlen - Length of buffer, in bytes.
 *      ambiguous - Ambiguous count storage, 0=unmatched or fully-matched.
 *      partial - Optional, one of the ambiguous children.
 *
 *  Results:
 *      Matched element, otherwise NULL.
 */
const keyseq_t *
kbsequence_match(const char *seq, unsigned seqlen, unsigned *ambiguous, const keyseq_t **partial)
{
#if defined(USE_LIBTRIE)
    return (const keyseq_t *)trie_nsearch_ambiguous(x_kseqtrie, seq, seqlen, ambiguous, (void **)partial);

#else
    SPBLK *sp_partial = NULL, *sp;

    assert(0 == seq[seqlen]);
    sp = sp_partial_lookup(seq, x_kseqtree, ambiguous, &sp_partial);
    if (partial) {
        *partial = (sp_partial ? (const keyseq_t *) sp_partial->data : NULL);
    }
    return (sp ? (const keyseq_t *) sp->data : NULL);

#endif //USE_LIBTRIE
}


/*  Function:       kbsequence_flatten
 *      Retrieves the current key-sequence tree as a list.
 *
 *  Parameters:
 *      count - Storage populated by the total symbol number.
 *
 *  Results:
 *      Flatten version of the key sequence tree; chk_free() to destroy.
 */

struct FlatternArguments {
    unsigned count;
    unsigned index;
    keyseq_t **array;
};

#if defined(USE_LIBTRIE)
static int
flattener(const char *key, void *value, void *uarg)
{
    struct FlatternArguments *arg = (struct FlatternArguments *)uarg;
    arg->array[arg->index++] = value;
    __CUNUSED(key);
    return 0;
}

#else
static void
flattener(SPBLK *node, void *uarg)
{
    struct FlatternArguments *arg = (struct FlatternArguments *)uarg;
    arg->array[arg->index++] = node->data;
}
#endif //USE_LIBTRIE

const keyseq_t * const *
kbsequence_flatten(unsigned *count)
{
    struct FlatternArguments arg = {0};

#if defined(USE_LIBTRIE)
    if (x_kseqtrie) {
        arg.count = (unsigned)trie_count(x_kseqtrie, NULL);
        if ((arg.array = chk_alloc((arg.count + 1) * sizeof(keyseq_t *))) != NULL) {
            trie_visit(x_kseqtrie, NULL, flattener, &arg);
            assert(arg.index == arg.count);
            arg.array[arg.index] = NULL;
        }
    }
#else
    if (x_kseqtree) {
        arg.count = spsize(x_kseqtree);
        if ((arg.array = chk_alloc((arg.count + 1) * sizeof(keyseq_t *))) != NULL) {
            spwalk(x_kseqtree, flattener, &arg);
            assert(arg.index == arg.count);
            arg.array[arg.index] = NULL;
        }
    }
#endif //USE_LIBTRIE

    if (count) *count = arg.count;
    return (const keyseq_t * const *)(arg.array);
}

/*end*/
