/*
 *  libtrie - prefix tree.
 */

#include "trie_private.h"
#include <assert.h>

/* Mini buffer library. */

struct buffer {
    trie_char_t *buffer;
    size_t size, fill;
};

static int
buffer_init(struct buffer *b, const char *prefix)
{
    b->fill = strlen(prefix);
    b->size = b->fill >= 256 ? b->fill * 2 : 256;
    b->buffer = malloc(b->size);
    if (b->buffer)
        memcpy(b->buffer, prefix, b->fill + 1);
    return !b->buffer ? -1 : 0;
}

static void
buffer_free(struct buffer *b)
{
    free(b->buffer);
    b->buffer = 0;
}

static int
buffer_grow(struct buffer *b)
{
    trie_char_t *resize = realloc(b->buffer, sizeof(trie_char_t) * b->size * 2);
    if (NULL == resize) {
        buffer_free(b);
        return -1;
    }
    b->buffer = resize;
    b->size *= 2;
    return 0;
}

static int
buffer_push(struct buffer *b, trie_char_t c)
{
    if (b->fill + 1 == b->size)
        if (buffer_grow(b) != 0)
            return -1;
    b->buffer[b->fill++] = c;
    b->buffer[b->fill] = 0;
    return 0;
}

static void
buffer_pop(struct buffer *b)
{
    if (b->fill > 0)
        b->buffer[--b->fill] = 0;
}

/* Core visitation functions. */

static int
visit(struct trie *self, const char *prefix, trie_visitor visitor, void *arg)
{
    struct buffer buffer, *b = &buffer;
    struct stack stack, *s = &stack;
    if (buffer_init(b, prefix) != 0)
        return -1;
    if (trie_stack_init(s) != 0) {
        buffer_free(b);
        return -1;
    }
    trie_stack_push(s, self);
    while (s->fill > 0) {
        struct stack_node *node = trie_stack_peek(s);
        if (node->i == 0 && node->trie->data) {
            if (visitor((const char *)b->buffer, node->trie->data, arg) != 0) {
                buffer_free(b);
                trie_stack_free(s);
                return 1;
            }
        }
        if (node->i < node->trie->nchildren) {
            struct trie *trie = node->trie->children[node->i].trie;
            trie_char_t c = node->trie->children[node->i].c;
            node->i++;
            if (trie_stack_push(s, trie) != 0) {
                buffer_free(b);
                return -1;
            }
            if (buffer_push(b, c) != 0) {
                trie_stack_free(s);
                return -1;
            }
        } else {
            buffer_pop(b);
            trie_stack_pop(s);
        }
    }
    buffer_free(b);
    trie_stack_free(s);
    return 0;
}

int
trie_visit(struct trie *self, const char *prefix, trie_visitor v, void *arg)
{
    struct trie *start = self;
    struct trieptr *ptr;
    unsigned char *uprefix = (unsigned char *)prefix;
    int r, depth = trie_binary_search(self, &start, &ptr, uprefix);
    if (prefix[depth])
        return 0;
    r = visit(start, prefix, v, arg);
    return r >= 0 ? 0 : -1;
}

/* Miscellaneous functions. */

static int
visitor_counter(const char *key, void *data, void *arg)
{
    size_t *count = arg;
    count[0]++;
    (void) key;
    (void) data;
    return 0;
}

size_t
trie_count(struct trie *trie, const char *prefix)
{
    size_t count = 0;
    trie_visit(trie, prefix, visitor_counter, &count);
    return count;
}

/* Iterator */

struct trie_it {
    struct stack stack;
    struct buffer buffer;
    void *data;
    int error;
};

struct trie_it *
trie_it_create(struct trie *trie, const char *prefix)
{
    struct trie_it *it = malloc(sizeof(*it));
    if (!it)
        return 0;
    if (trie_stack_init(&it->stack)) {
        free(it);
        return 0;
    }
    if (buffer_init(&it->buffer, prefix)) {
        trie_stack_free(&it->stack);
        free(it);
        return 0;
    }
    trie_stack_push(&it->stack, trie); /* first push always successful */
    it->data = 0;
    it->error = 0;
    trie_it_next(it);
    return it;
}

int
trie_it_next(struct trie_it *it)
{
    while (!it->error && it->stack.fill) {
        struct stack_node *node = trie_stack_peek(&it->stack);

        if (node->i == 0 && node->trie->data) {
            if (!it->data) {
                it->data = node->trie->data;
                return 1;
            } else {
                it->data = 0;
            }
        }

        if (node->i < node->trie->nchildren) {
            struct trie *trie = node->trie->children[node->i].trie;
            trie_char_t c = node->trie->children[node->i].c;
            node->i++;
            if (trie_stack_push(&it->stack, trie)) {
                it->error = 1;
                return 0;
            }
            if (buffer_push(&it->buffer, c)) {
                it->error = 1;
                return 0;
            }
        } else {
            buffer_pop(&it->buffer);
            trie_stack_pop(&it->stack);
        }
    }
    return 0;
}

const char *
trie_it_key(struct trie_it *it)
{
    return (const char *)(it->buffer.buffer);
}

void *
trie_it_data(struct trie_it *it)
{
    return it->data;
}

int
trie_it_done(struct trie_it *it)
{
    return it->error || !it->stack.fill;
}

int
trie_it_error(struct trie_it *it)
{
    return it->error;
}

void
trie_it_free(struct trie_it *it)
{
    buffer_free(&it->buffer);
    trie_stack_free(&it->stack);
    free(it);
}

/*end*/
