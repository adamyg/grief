/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"

#if defined(__GNUC__)
#if !defined(alloca)
#define alloca __builtin_alloca
#endif
#elif defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#else
#include <alloca.h>
#endif
#include <ctype.h>
#include <assert.h>

/* Insertion functions. */

static struct trie *
grow(struct trie *self)
{
    int size = self->size * 2;
    assert(size <= 256);
    if (size > 255) size = 255;
    {   const size_t children_size = sizeof(struct trieptr) * size;
        struct trie *resized = realloc(self, sizeof(*self) + children_size);
        if (NULL == resized)
            return NULL;
        resized->size = (short)size;
        return resized;
    }
}


static int
ptr_cmp(const void *a, const void *b)
{
    return ((struct trieptr *)a)->c - ((struct trieptr *)b)->c;
}


static struct trie *
node_add(struct trie *self, unsigned char c, struct trie *child)
{
    if (self->size == self->nchildren) {
        self = grow(self);
        if (NULL == self)
            return NULL;
    }
    {   const int i = self->nchildren++;
        self->children[i].c = c;
        self->children[i].trie = child;
        qsort(self->children, self->nchildren, sizeof(self->children[0]), ptr_cmp);
    }
    return self;
}


static struct trie *
create(void)
{
    int size = 1;
    struct trie *trie = malloc(sizeof(*trie) + sizeof(struct trieptr) * size);
    if (NULL == trie)
        return 0;
    trie->size = (short)size;
    trie->nchildren = 0;
    trie->data = 0;
    return trie;
}


static unsigned char
to_lower(unsigned char c)
{
    return (isupper(c) ? c - 'A' + 'a' : c);
}


static size_t
binary_search_wild(struct trie *self, struct trie **child,
        struct trieptr **ptr, const unsigned char *key, trie_char_t *ch)
{
    const int icase = self->icase;
    unsigned found = 1;
    size_t i = 0;
    trie_char_t c;

    *ptr = 0;
    while (found && 0 != (c = key[i])) {
        int first = 0;
        int last = self->nchildren - 1;
        int middle;

        if ('+' == c) { // wild
            c = WILD_MARKER;
        } else {
            if ('\\' == c) { // escape
                if (key[i + 1]) c = key[++i];
            }
            if (icase) c = to_lower(c);
        }

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
    *ch = c;
    return i;
}


int
trie_replace_wild(struct trie *self, const char *key, trie_replacer f, void *arg)
{
    struct trie *last;
    struct trieptr *parent;
    unsigned char *ukey = (unsigned char *)key;
    const int icase = self->icase;
    trie_char_t ch = 0;

    size_t depth = binary_search_wild(self, &last, &parent, ukey, &ch);
    if (ch) {
        assert(ukey[depth]);
        while (1) {
            struct trie *added, *subtrie = create();
            if (NULL == subtrie)
                return 1;
            added = node_add(last, ch, subtrie);
            if (NULL == added) {
                free(subtrie);
                return 1;
            }
            if (parent) {
                parent->trie = added;
                parent = 0;
            }
            last = subtrie;

            if (0 == (ch = ukey[++depth]))
                break;

            if ('+' == ch) { // wild
                ch = WILD_MARKER;
            } else {
                if ('\\' == ch) { // escape
                    if (key[depth + 1])
                        ch = key[++depth];
                }
                if (icase) ch = to_lower(ch);
            }
        }
    }

    last->data = (NULL != f ? f(key, last->data, arg) : arg);
    return 0;
}


int
trie_insert_wild(struct trie *trie, const char *key, void *data)
{
    return trie_replace_wild(trie, key, NULL, data);
}


int
trie_insert_nwild(struct trie *trie, const char *key, int length, void *data)
{
    char *t_key = (length ? alloca(length + /*nul*/ 1) : NULL);
    if (NULL == t_key)
        return -1;

    memcpy(t_key, key, length);
    t_key[length] = 0;

    return trie_replace_wild(trie, t_key, NULL, data);
}

/*end*/
