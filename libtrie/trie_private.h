/**
 * C99 Trie Library
 *
 * This trie associates an arbitrary void pointer with a NUL-terminated
 * C string key. All lookups are O(n), n being the length of the string.
 * Strings are stored in sorted order, so visitor functions visit keys
 * in lexicographical order. The visitor can also be used to visit keys
 * by a string prefix. An empty prefix "" matches all keys (the prefix
 * argument should never be NULL).
 *
 * Except for trie_free() and trie_prune(), memory is never freed by the
 * trie, even when entries are "removed" by associating a NULL pointer.
 *
 * @see http://en.wikipedia.org/wiki/Trie
 */

#include "libtrie.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define WILD_MARKER 0x01

#ifndef _MSC_VER
#ifndef _countof
#define _countof(__type)    (sizeof(__type)/sizeof(__type[0]))
#endif
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4200) // nonstandard extension used: zero-sized array in struct/union
#endif

typedef unsigned char trie_char_t;

struct trieptr {
    struct trie *trie;
    trie_char_t c;
};

struct trie {
    void *data;
    short nchildren, size, icase;
    struct trieptr children[];
};

struct stack_node {
    struct trie *trie;
    int i;
};

struct stack {
    struct stack_node *stack;
    size_t fill, size;
};

int trie_stack_init(struct stack *s);
void trie_stack_free(struct stack *s);
int trie_stack_grow(struct stack *s);
int trie_stack_push(struct stack *s, struct trie *trie);
struct trie *trie_stack_pop(struct stack *s);
struct stack_node *trie_stack_peek(struct stack *s);

size_t trie_binary_search(struct trie *self, struct trie **child, struct trieptr **ptr, const unsigned char *key);
size_t trie_binary_search_i(struct trie *self, struct trie **child, struct trieptr **ptr, const unsigned char *key);

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

/*end*/
