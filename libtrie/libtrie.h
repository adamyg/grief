#ifndef LIBTRIE_H_INCLUDED
#define LIBTRIE_H_INCLUDED

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
 *
 * Original Source: https://github.com/skeeto/trie [Unlicense'ed]
 */

#include <stddef.h>

struct trie;
struct trie_it;

typedef int (*trie_visitor)(const char *key, void *data, void *arg);
typedef void *(*trie_replacer)(const char *key, void *current, void *arg);

/**
 * @return a freshly allocated trie, NULL on allocation error
 */
struct trie *trie_create(void);
struct trie *trie_icreate(void);


/**
 * Destroys a trie created by trie_create().
 * @return 0 on success
 */
int trie_free(struct trie *trie);

/**
 * Finds for the data associated with KEY.
 * @return the previously inserted data
 */
void *trie_search(const struct trie *trie, const char *key);
void *trie_nsearch(const struct trie *trie, const char *key, size_t length);

/**
 * Finds for the data associated with KEY.
 * @return the previously inserted data and details of any children, representing an ambiguous match.
 * Optionally also one of the next matching elements.
 */
void *trie_search_ambiguous(const struct trie *self, const char *key, unsigned *ambiguous, void **partial);
void *trie_nsearch_ambiguous(const struct trie *self, const char *key, size_t length, unsigned *ambiguous, void **partial);

/**
 * Finds for the data associated with KEY, treating '+' as simple wild-card; one or more characters.
 * @return the previously inserted data
 */
void *trie_search_wild(const struct trie *trie, const char *key);
void *trie_search_nwild(const struct trie *trie, const char *key, size_t length);

/**
 * Insert or replace DATA associated with KEY. Inserting NULL is the equivalent of
 * unassociating that key, though no memory will be released.
 * @return 0 on success
 */
int trie_insert(struct trie *trie, const char *key, void *data);
int trie_ninsert(struct trie *trie, const char *key, int length, void *data);

/**
 * Insert or replace DATA associated with KEY. Inserting NULL is the equivalent of
 * unassociating that key, though no memory will be released treating '+' as simple wild-card; one or more characters.
 * @return 0 on success
 */
int trie_insert_wild(struct trie *trie, const char *key, void *data);
int trie_insert_nwild(struct trie *trie, const char *key, int length, void *data);

/**
 * Replace data associated with KEY using a replacer function. The replacer function gets the key,
 * the original data (NULL if none) and ARG. Its return value is inserted into the trie.
 * @return 0 on success
 */
int trie_replace(struct trie *trie, const char *key, trie_replacer f, void *arg);

/**
 * Replace data associated with KEY using a replacer function. The replacer function gets the key,
 * the original data (NULL if none) and ARG. Its return value is inserted into the trie
 * treating '+' as simple wild-card; one or more characters.
 * @return 0 on success
 */
int trie_replace_wild(struct trie *trie, const char *key, trie_replacer f, void *arg);

/**
 * Visit in lexicographical order each key that matches the prefix. An empty prefix visits every key in the trie.
 * The visitor must accept three arguments: the key, the data, and ARG. Iteration is aborted (with success)
 * if visitor returns non-zero.
 * @return 0 on success
 */
int trie_visit(struct trie *trie, const char *prefix, trie_visitor v, void *arg);

/**
 * Remove all unused branches in a trie.
 * @return 0 on success
 */
int trie_prune(struct trie *trie);

/**
 * Count the number of entries with a given prefix. An empty prefix
 * counts the entire trie.
 * @return the number of entries matching PREFIX
 */
size_t trie_count(struct trie *trie, const char *prefix);

/**
 * Compute the total memory usage of a trie.
 * @return the size in bytes, or 0 on error
 */
size_t trie_size(struct trie *trie);

/**
 * Create an iterator that visits each key with the given prefix, in
 * lexicographical order. Making any modifications to the trie
 * invalidates the iterator.
 * @return a fresh iterator pointing to the first key
 */
struct trie_it *trie_it_create(struct trie *trie, const char *prefix);

/**
 * Advance iterator to the next key in the sequence.
 * @return 0 if done, else 1
 */
int trie_it_next(struct trie_it *trie);

/**
 * Returned buffer is invalidated on the next trie_it_next().
 * @return a buffer containing the current key
 */
const char *trie_it_key(struct trie_it *trie);

/**
 * @return the data pointer for the current key
 */
void *trie_it_data(struct trie_it *trie);

/**
 * @return 1 if the iterator has completed (including errors)
 */
int trie_it_done(struct trie_it *trie);

/**
 * @return 1 if the iterator experienced an error
 */
int trie_it_error(struct trie_it *trie);

/**
 * Destroys the iterator.
 * @return 0 on success
 */
void trie_it_free(struct trie_it *trie);

#endif /*LIBTRIE_H_INCLUDED*/
