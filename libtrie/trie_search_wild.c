/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

/* Wild card search */

struct backtrack {
    struct trie *self;
    struct trieptr *ptr;
    int i;
};


static size_t
binary_search_wild(struct trie *self,
        struct trie **child, struct trieptr **ptr, const unsigned char *key)
{
#define FOUND   0x01
#define WILD    0x02

    struct backtrack backtrack = {NULL, NULL, -1};
    unsigned flgs = FOUND;
    trie_char_t c;
    size_t i = 0;

    *ptr = NULL;
    while ((flgs & FOUND) && 0 != (c = key[i])) {
        struct trie *t_self = self;
        int first = 0, last = self->nchildren - 1;
        int middle;

        /* match */
        flgs &= ~FOUND;
        while (first <= last) {
            struct trieptr *p;
            middle = (first + last) / 2;
            p = &t_self->children[middle];
            if (p->c < c) {
                first = middle + 1;
            } else if (p->c == c) {
                self = p->trie;
                *ptr = p;
                flgs = FOUND;
                ++i;
                break;
            } else {
                last = middle - 1;
            }
        }

        /* wild match */
        if (WILD == flgs) {
            flgs = FOUND|WILD;
            ++i;
            continue;
        }

        /* search for junction */
        if (0 == (flgs & FOUND) || NULL == backtrack.self) {
#if (WILD_MARKER == 0x01)
            struct trieptr *p = &t_self->children[0];
            if (p->c == WILD_MARKER) {
                if (flgs & FOUND) {
                    assert(NULL == backtrack.self);
                    backtrack.self = p->trie;
                    backtrack.ptr = *ptr;
                    backtrack.i = i;
                } else {
                    self = p->trie;
                    *ptr = p;
                    flgs = FOUND|WILD;
                }
            }
#else
            first = 0, last = t_self->nchildren - 1;
            while (first <= last) {
                struct trieptr *p;
                middle = (first + last) / 2;
                p = &t_self->children[middle];
                if (p->c < WILD_MARKER) {
                    first = middle + 1;
                } else if (p->c == WILD_MARKER) {
                    if (flgs & FOUND) {
                        assert(NULL == backtrack.self);
                        backtrack.self = p->trie;
                        backtrack.ptr = *ptr;
                        backtrack.i = i;
                    } else {
                        self = p->trie;
                        *ptr = p;
                        flgs = FOUND|WILD;
                    }
                    break;
                } else {
                    last = middle - 1;
                }
            }
#endif
        }

        if (0 == (flgs & FOUND) && backtrack.self) {
            self = backtrack.self;
            *ptr = backtrack.ptr;
            i = backtrack.i;
            backtrack.self = NULL;
            flgs = FOUND|WILD;
        }
    }

    *child = self;
    return i;
}


void *
trie_search_wild(const struct trie *self, const char *key)
{
    struct trie *child;
    struct trieptr *parent;
    const size_t depth =
        binary_search_wild((struct trie *)self, &child, &parent, (const unsigned char *)key);
    return (0 == key[depth]) ? child->data : 0;
}

/*end*/
