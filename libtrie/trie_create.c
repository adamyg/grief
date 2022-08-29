/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

/* Constructor and destructor. */

static struct trie *
trie_allocate(void)
{
    /* Root never needs to be resized. */
    const size_t tail_size = sizeof(struct trieptr) * 255;
    struct trie *root = calloc(sizeof(*root) + tail_size, 1);
    if (NULL == root)
        return NULL;
    root->size = 255;
    root->nchildren = 0;
    root->data = 0;
    root->icase = 0;
    return root;
}


struct trie *
trie_create(void)
{
    return trie_allocate();
}


struct trie *
trie_icreate(void)
{
    struct trie *root = trie_allocate();
    if (root)
        root->icase = 1;
    return root;
}

/*end*/
