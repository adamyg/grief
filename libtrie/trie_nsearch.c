/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <ctype.h>
#include <assert.h>

/* Core search functions. */

static size_t
trie_binary_search_n(struct trie *self, struct trie **child,
        struct trieptr **ptr, const unsigned char *key, size_t length)
{
    size_t i = 0;
    unsigned found = 1;
    trie_char_t c;

    *ptr = 0;
    while (found && i != length && 0 != (c = key[i])) {
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


static size_t
trie_binary_search_ni(struct trie *self, struct trie **child,
        struct trieptr **ptr, const unsigned char *key, size_t length)
{
    size_t i = 0;
    unsigned found = 1;
    unsigned char c;

    *ptr = 0;
    while (found && i != length && 0 != (c = key[i])) {
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
trie_nsearch(const struct trie *self, const char *key, size_t length)
{
    struct trie *child;
    struct trieptr *parent;
    unsigned char *ukey = (unsigned char *)key;
    const size_t depth =
        (self->icase ? trie_binary_search_ni((struct trie *)self, &child, &parent, ukey, length) :
            trie_binary_search_n((struct trie *)self, &child, &parent, ukey, length));
    return (depth == length ? child->data : NULL);
}

/*end*/
