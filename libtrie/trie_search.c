/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <ctype.h>
#include <assert.h>

/* Core search functions. */

size_t
trie_binary_search(struct trie *self, struct trie **child,
        struct trieptr **ptr, const unsigned char *key)
{
    size_t i = 0;
    unsigned found = 1;
    trie_char_t c;

    *ptr = 0;
    while (found && 0 != (c = key[i])) {
        int first = 0;
        int last = self->nchildren - 1;
        int middle;

        found = 0;
        while (first <= last) {
            struct trieptr *p;
            middle = (first + last) / 2;
            p = &self->children[middle];
            if (p->c < c) {
                first = middle + 1;
            } else if (p->c == c) {
                self = p->trie;
                *ptr = p;
                found = 1;
                ++i;
                break;
            } else {
                last = middle - 1;
            }
        }
    }
    *child = self;
    return i;
}


static unsigned char
to_lower(unsigned char c)
{
    return (isupper(c) ? c - 'A' + 'a' : c);
}


size_t
trie_binary_search_i(struct trie *self, struct trie **child,
        struct trieptr **ptr, const unsigned char *key)
{
    size_t i = 0;
    unsigned found = 1;
    trie_char_t c;

    *ptr = 0;
    while (found && 0 != (c = key[i])) {
        int first = 0;
        int last = self->nchildren - 1;
        int middle;

        found = 0;
        c = to_lower(c);
        while (first <= last) {
            struct trieptr *p;
            middle = (first + last) / 2;
            p = &self->children[middle];
            if (p->c < c) {
                first = middle + 1;
            } else if (p->c == c) {
                self = p->trie;
                *ptr = p;
                found = 1;
                ++i;
                break;
            } else {
                last = middle - 1;
            }
        }
    }
    *child = self;
    return i;
}


void *
trie_search(const struct trie *self, const char *key)
{
    struct trie *child;
    struct trieptr *parent;
    unsigned char *ukey = (unsigned char *)key;
    const size_t depth =
        (self->icase ? trie_binary_search_i((struct trie *)self, &child, &parent, ukey) :
            trie_binary_search((struct trie *)self, &child, &parent, ukey));
    return (0 == key[depth] ? child->data : NULL);
}


void *
trie_search_ambiguous(const struct trie *self, const char *key, unsigned *ambiguous, void **partial)
{
    struct trie *child;
    struct trieptr *parent;
    unsigned char *ukey = (unsigned char *)key;
    const size_t depth =
        (self->icase ? trie_binary_search_i((struct trie *)self, &child, &parent, ukey) :
            trie_binary_search((struct trie *)self, &child, &parent, ukey));

    if (key[depth] || 0 == depth) {
        if (ambiguous) {                        // ambiguous count
            *ambiguous = 0;
        }
        if (partial) {                          // partial, first child
            *partial = NULL;
        }
        return NULL;                            // unmatched
    }

    if (ambiguous) {                            // ambiguous count
        *ambiguous = child->nchildren;
    }

    if (partial) {                              // partial, first child
        struct trie *cursor = child;

        *partial = NULL;
        while (cursor->nchildren) {             // depth-first search
            cursor = cursor->children[ (cursor->nchildren - 1) / 2 ].trie;
            if (cursor->data) {
                *partial = cursor->data;
                break;
            }
        }
    }
    return child->data;                         // matched
}

/*end*/

