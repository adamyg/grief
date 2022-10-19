/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

size_t
trie_size(struct trie *trie)
{
    struct stack stack, *s = &stack;
    size_t size = 0;

    if (trie_stack_init(s) != 0)
        return 0;
    trie_stack_push(s, trie);
    while (s->fill > 0) {
        struct stack_node *node = trie_stack_peek(s);
        const int i = node->i++;
        if (i < node->trie->nchildren) {
            if (trie_stack_push(s, node->trie->children[i].trie) != 0)
                return 0;
        } else {
            struct trie *t = trie_stack_pop(s);
            size += sizeof(*t) + sizeof(*t->children) * t->size;
        }
    }
    trie_stack_free(s);
    return size;
}

/*end*/
