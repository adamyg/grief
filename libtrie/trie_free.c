/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

/* Constructor and destructor. */

int
trie_free(struct trie *trie)
{
    struct stack stack, *s = &stack;

    if (NULL == trie)
        return 0;
    if (trie_stack_init(s) != 0)
        return 1;
    trie_stack_push(s, trie); /* first push always successful */
    while (s->fill > 0) {
        struct stack_node *node = trie_stack_peek(s);
        if (node->i < node->trie->nchildren) {
            int i = node->i++;
            if (trie_stack_push(s, node->trie->children[i].trie) != 0)
                return 1;
        } else {
            free(trie_stack_pop(s));
        }
    }
    trie_stack_free(s);
    return 0;
}

/*end*/
