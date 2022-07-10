/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
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


void *
trie_search(const struct trie *self, const char *key)
{
    struct trie *child;
    struct trieptr *parent;
    unsigned char *ukey = (unsigned char *)key;
    size_t depth = trie_binary_search((struct trie *)self, &child, &parent, ukey);
    return !key[depth] ? child->data : NULL;
}

/*end*/
