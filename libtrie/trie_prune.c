/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

static void
node_remove(struct trie *self, int i)
{
    size_t len = (--self->nchildren - i) * sizeof(self->children[0]);
    memmove(self->children + i, self->children + i + 1, len);
}


int
trie_prune(struct trie *trie)
{
    struct stack stack, *s = &stack;
    if (trie_stack_init(s) != 0)
        return -1;
    trie_stack_push(s, trie);
    while (s->fill > 0) {
        struct stack_node *node = trie_stack_peek(s);
        int i = node->i++;
        if (i < node->trie->nchildren) {
            if (trie_stack_push(s, node->trie->children[i].trie) != 0)
                return 0;
        } else {
            struct trie *t = trie_stack_pop(s);
            for (i = 0; i < t->nchildren; i++) {
                struct trie *child = t->children[i].trie;
                if (!child->nchildren && !child->data) {
                    node_remove(t, i--);
                    free(child);
                }
            }
        }
    }
    trie_stack_free(s);
    return 1;
}

/*end*/
