/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

int
trie_stack_init(struct stack *s)
{
    s->size  = 256;
    s->fill  = 0;
    s->stack = calloc(s->size * sizeof(struct stack_node), 1);
    return !s->stack ? -1 : 0;
}


void
trie_stack_free(struct stack *s)
{
    free(s->stack);
    s->stack = 0;
}


int
trie_stack_grow(struct stack *s)
{
    size_t newsize = s->size * 2 * sizeof(struct stack_node);
    struct stack_node *resize = realloc(s->stack, newsize);
    if (!resize) {
        trie_stack_free(s);
        return -1;
    }
    s->size *= 2;
    s->stack = resize;
    return 0;
}


int
trie_stack_push(struct stack *s, struct trie *trie)
{
    if (s->fill == s->size)
        if (trie_stack_grow(s) != 0)
            return -1;
    {   const size_t fill = s->fill++;
        s->stack[fill].trie = trie;
        s->stack[fill].i = 0;
    }
    return 0;
}


struct trie *
trie_stack_pop(struct stack *s)
{
    return s->stack[--s->fill].trie;
}


struct stack_node *
trie_stack_peek(struct stack *s)
{
    return &s->stack[s->fill - 1];
}

/*end*/
