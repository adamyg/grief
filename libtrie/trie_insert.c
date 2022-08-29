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
        return NULL;
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


int
trie_replace(struct trie *self, const char *key, trie_replacer f, void *arg)
{
    struct trie *last;
    struct trieptr *parent;
    unsigned char c, *ukey = (unsigned char *)key;
    const int icase = self->icase;
    size_t depth =
        (icase ? trie_binary_search_i(self, &last, &parent, ukey) :
            trie_binary_search(self, &last, &parent, ukey));

    while (0 != (c = ukey[depth])) {
        struct trie *added, *subtrie = create();
        if (NULL == subtrie)
            return 1;
        if (icase) c = to_lower(c);
        added = node_add(last, c, subtrie);
        if (NULL == added) {
            free(subtrie);
            return 1;
        }
        if (parent) {
            parent->trie = added;
            parent = 0;
        }
        last = subtrie;
        depth++;
    }

    last->data = (NULL != f ? f(key, last->data, arg) : arg);
    return 0;
}


int
trie_insert(struct trie *trie, const char *key, void *data)
{
    return trie_replace(trie, key, NULL, data);
}


int
trie_ninsert(struct trie *trie, const char *key, int length, void *data)
{
    char *t_key = (length ? alloca(length + /*nul*/ 1) : NULL);
    if (NULL == t_key)
        return -1;

    memcpy(t_key, key, length);
    t_key[length] = 0;

    return trie_replace(trie, t_key, NULL, data);
}

/*end*/
