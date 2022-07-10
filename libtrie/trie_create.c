/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

/* Constructor and destructor. */

struct trie *
trie_create(void)
{
    /* Root never needs to be resized. */
    const size_t tail_size = sizeof(struct trieptr) * 255;
    struct trie *root = calloc(sizeof(*root) + tail_size, 1);
    if (NULL == root)
        return NULL;
    root->size = 255;
    root->nchildren = 0;
    root->data = 0;
    return root;
}

/*end*/
