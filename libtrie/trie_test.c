/*
 *  libtrie tests
 */

#include "libtrie.h"
#include "trie_private.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#ifndef snprintf
#define snprintf    _snprintf
#endif
#endif /*_MSC_VER*/

static void
test(int val, const char *msg)
{
#define TEST(__x) test(__x, #__x)
#define TEST2(__x, __z) test(__x, __z)
        printf("trie: [%s] %s\n", (val ? " OK  " : "ERROR"), msg);
        assert(val);
}

#define EXP_TOTAL   5
#define EXP_WORDS   13


static void
die(const char *s)
{
        fprintf(stderr, "%s\n", s);
        exit(EXIT_FAILURE);
}


static uint32_t
pcg32(uint64_t s[1])
{
        const uint64_t h = s[0];
        const uint32_t x = (int32_t)( ((h >> 18) ^ h) >> 27 );
        const unsigned r = h >> 59;
        s[0] = h * UINT64_C(0x5851f42d4c957f2d) + UINT64_C(0xd737232eeccdf7ed);
        return (x >> r) | (x << (-r & 31u));
}


static char *
generate(uint64_t *s)
{
        int min = 8;
        int max = 300;
        int i, len = min + pcg32(s) % (max - min);
        char *key = malloc(len + 1);

        if (!key)
                die("out of memory");
        for (i = 0; i < len; i++)
                key[i] = 1 + pcg32(s) % 255;
        key[len] = 0;
        return key;
}


static void *
replacer(const char *key, void *value, void *arg)
{
        (void)key;
        free(value);
        return arg;
}


static void
hexprint(const char *label, const char *s)
{
        unsigned char *p = (unsigned char *)s;
        size_t i;

        fputs(label, stdout);
        for (i = 0; *p; i++)
                printf("%c%02x", i % 16 ? ' ' : '\n', *p++);
        putchar('\n');
}


static int
check_order(const char *key, void *value, void *arg)
{
        char **prev = arg;
        if (*prev && strcmp(*prev, key) >= 0) {
                hexprint("first:", *prev);
                hexprint("second:", key);
                die("FAIL: keys not ordered");
        }
        *prev = value;
        return 0;
}


static void
test1(void)
{
        uint64_t rng = 0xabf4206f849fdf21;
        struct trie *t = trie_create();
        int i;

        for (i = 0; i < 1 << EXP_TOTAL; i++) {
                long count = (1L << EXP_WORDS) + pcg32(&rng) % (1L << EXP_WORDS);
                uint64_t rngcopy, rngsave = rng;
                struct trie_it *it;
                char *prev = 0;
                long j;

                for (j = 0; j < count; j++) {
                        char *key = generate(&rng);
                        if (trie_insert(t, key, key))
                                die("out of memory");
                }

                /* Check that all keys are present. */
                rngcopy = rngsave;
                for (j = 0; j < count; j++) {
                        char *key = generate(&rngcopy);
                        char *r = trie_search(t, key);
                        if (!r)
                                die("FAIL: missing key");
                        if (strcmp(r, key))
                                die("FAIL: value mismatch");
                        free(key);
                }

                /* Check that keys are sorted (visitor) */
                if (trie_visit(t, "", check_order, &prev))
                    die("out of memory");

                /* Check that keys are sorted (iterator) */
                prev = 0;
                it = trie_it_create(t, "");
                for (; !trie_it_done(it); trie_it_next(it)) {
                        const char *key = trie_it_key(it);
                        if (prev && strcmp(prev, key) >= 0)
                                die("FAIL: keys not ordered");
                        prev = trie_it_data(it);
                }
                if (trie_it_error(it))
                        die("out of memory");
                trie_it_free(it);

                /* Remove all entries. */
                rngcopy = rngsave;
                for (j = 0; j < count; j++) {
                        char *key = generate(&rngcopy);
                        if (trie_replace(t, key, replacer, 0))
                                die("out of memory");
                        free(key);
                }

                /* Check that all keys are gone. */
                rngcopy = rngsave;
                for (j = 0; j < count; j++) {
                        char *key = generate(&rngcopy);
                        char *r = trie_search(t, key);
                        if (r)
                                die("FAIL: key not removed");
                        free(key);
                }

                /* Print out current trie size (as progress) */
                {       const double mb = trie_size(t) / 1024.0 / 1024.0;
                        char buffer[80];
                        snprintf(buffer, sizeof(buffer), "%-2d trie_size() = %10.3f MiB", i, mb);
                        TEST2(1, buffer);
                }

                /* Prune trie every quarter. */
                if (i && i % (1 << (EXP_TOTAL - 2)) == 0) {
                        /* Insert a check key to make sure it survives. */
                        char tmpkey[32];
                        unsigned long long v = rngsave;
                        snprintf(tmpkey, sizeof(tmpkey), "%llx", v);
                        if (trie_insert(t, tmpkey, tmpkey))
                                die("out of memory");
                        trie_prune(t);
                        if (trie_search(t, tmpkey) != tmpkey)
                                die("FAIL: trie_prune() removed live key");
                        trie_insert(t, tmpkey, 0); /* Cleanup */
                }
        }

        trie_free(t);
}


/////////////////////////////////////////////////////////////////////////////////////////
//  general matching
//

static const char *test2_words[] = {
        "+_BINARY_DIR",
        "+_SOURCE_DIR",
        "ARCHIVE_OUTPUT_DIRECTORY_+",
        "ARCHIVE_OUTPUT_NAME_+",
        "CMAKE_+_ARCHIVE_APPEND",
        "CMAKE_+_ARCHIVE_CREATE",
        "CMAKE_+_ARCHIVE_FINISH",
        "CMAKE_+_COMPILER",
        "CMAKE_+_COMPILER_ABI",
        "CMAKE_+_COMPILER_ID",
        "CMAKE_+_COMPILER_LOADED",
        "CMAKE_+_COMPILER_VERSION",
        "CMAKE_+_COMPILE_OBJECT",
        "CMAKE_+_CREATE_SHARED_LIBRARY",
        "CMAKE_+_CREATE_SHARED_MODULE",
        "CMAKE_+_CREATE_STATIC_LIBRARY",
        "CMAKE_+_FLAGS",
        "CMAKE_+_FLAGS_DEBUG",
        "CMAKE_+_FLAGS_MINSIZEREL",
        "CMAKE_+_FLAGS_RELEASE",
        "CMAKE_+_FLAGS_RELWITHDEBINFO",
        "CMAKE_+_IGNORE_EXTENSIONS",
        "CMAKE_+_IMPLICIT_INCLUDE_DIRECTORIES",
        "CMAKE_+_IMPLICIT_LINK_DIRECTORIES",
        "CMAKE_+_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES",
        "CMAKE_+_IMPLICIT_LINK_LIBRARIES",
        "CMAKE_+_LIBRARY_ARCHITECTURE",
        "CMAKE_+_LINKER_PREFERENCE",
        "CMAKE_+_LINKER_PREFERENCE_PROPAGATES",
        "CMAKE_+_LINK_EXECUTABLE",
        "CMAKE_+_OUTPUT_EXTENSION",
        "CMAKE_+_PLATFORM_ID",
        "CMAKE_+_POSTFIX",
        "CMAKE_+_SIZEOF_DATA_PTR",
        "CMAKE_+_SOURCE_FILE_EXTENSIONS",
        "CMAKE_+_VISIBILITY_PRESET",
        "CMAKE_DISABLE_FIND_PACKAGE_+",
        "CMAKE_EXE_LINKER_FLAGS_+",
        "CMAKE_MODULE_LINKER_FLAGS_+",
        "CMAKE_SHARED_LINKER_FLAGS_+",
        "CMAKE_STATIC_LINKER_FLAGS_+",
        "CMAKE_USER_MAKE_RULES_OVERRIDE_+",
        "COMPILE_DEFINITIONS_+",
        "EXCLUDE_FROM_DEFAULT_BUILD_+",
        "IMPORTED_IMPLIB_+",
        "IMPORTED_LINK_DEPENDENT_LIBRARIES_+",
        "IMPORTED_LINK_INTERFACE_LANGUAGES_+",
        "IMPORTED_LINK_INTERFACE_MULTIPLICITY_+",
        "IMPORTED_LOCATION_+",
        "IMPORTED_NO_SONAME_+",
        "IMPORTED_SONAME_+",
        "INTERPROCEDURAL_OPTIMIZATION_+",
        "LIBRARY_OUTPUT_DIRECTORY_+",
        "LIBRARY_OUTPUT_NAME_+",
        "LINK_FLAGS_+",
        "LINK_INTERFACE_LIBRARIES_+",
        "LINK_INTERFACE_MULTIPLICITY_+",
        "LOCATION_+",
        "MAP_IMPORTED_CONFIG_+",
        "OSX_ARCHITECTURES_+",
        "OUTPUT_NAME_+",
        "PDB_NAME_+",
        "PDB_OUTPUT_DIRECTORY_+",
        "RUNTIME_OUTPUT_DIRECTORY_+",
        "RUNTIME_OUTPUT_NAME_+",
        "STATIC_LIBRARY_FLAGS_+",
        "VS_GLOBAL_+",
        "VS_GLOBAL_SECTION_POST_+",
        "VS_GLOBAL_SECTION_PRE_+",
        "XCODE_ATTRIBUTE_+",
        };


static int
test2_visitor(const char *key, void *data, void *arg)
{
        printf("%s=%p\n", key, (const char *)data);
        return 0;
}


static void
test2(void)
{
        struct trie *t = trie_create();
        unsigned i = 0;

        for (i = 0; i < _countof(test2_words); ++i) {
                const int ret = trie_insert(t, test2_words[i], (void *)test2_words[i]);
                TEST2(0 == ret, "trie_insert");
        }

        trie_visit(t, "", test2_visitor, NULL);

        for (i = 0; i < _countof(test2_words); ++i) {
                void *ret = trie_search(t, test2_words[i]);
                TEST(ret == test2_words[i]);

                ret = trie_nsearch(t, test2_words[i], strlen(test2_words[i]));
                TEST(ret == test2_words[i]);
        }

        {       const char *prefix = "CMAKE_EXE_LINKER_FLAGS";
                size_t count = trie_count(t, prefix);
                TEST(1 == count);
        }

        {       const char *prefix = "CMAKE_+_";
                size_t count = trie_count(t, prefix);
                TEST(32 == count);
        }

        /* replace then prune, implied deletion */
        {
                const size_t size = trie_size(t);
                const int count = trie_count(t, "");
                int ret  = trie_replace(t, "CMAKE_+_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES", NULL, NULL);
                TEST2(0 == ret, "trie_replace");
                TEST(size == trie_size(t));             /* node count */
                TEST((count - 1) == trie_count(t, "")); /* non-null element count */

                ret = trie_prune(t);
                TEST2(1 == ret, "trie_prune()");
                TEST(size > trie_size(t));              /* node pruned */
                TEST((count - 1) == trie_count(t, "")); /* no change in elements */
        }

        /* iterator */
        {       struct trie_it *it = trie_it_create(t, "");
                unsigned count = 0;

                printf("\n=== iterator\n");
                if (! trie_it_done(it)) {
                        do {
                                printf(" %s=%p\n",
                                    (const char *)trie_it_key(it), (const char *)trie_it_data(it));
                                ++count;
                        } while (trie_it_next(it));
                }
                TEST(count == trie_count(t, ""));
                trie_it_free(it);
                printf("\n");
        }

        trie_free(t);
}


static void
test3(void)
{
        struct trie *t = trie_icreate(); //icase
        const char *word;
        char *t_cursor, t_buffer[128];
        unsigned i = 0;

        for (i = 0; i < _countof(test2_words); ++i) {
                const int ret = trie_insert(t, test2_words[i], (void *)test2_words[i]);
                TEST2(0 == ret, "trie_insert");
        }

        trie_visit(t, "", test2_visitor, NULL);

        for (i = 0; i < _countof(test2_words); ++i) {

                // upper/orginal
                void *ret = trie_search(t, test2_words[i]);
                TEST(ret == test2_words[i]);
                ret = trie_nsearch(t, test2_words[i], strlen(test2_words[i]));
                TEST(ret == test2_words[i]);

                // lower
                for (word = test2_words[i], t_cursor = t_buffer; *word; ++word)
                    *t_cursor++ = tolower(*word);
                *t_cursor = 0;
                ret = trie_search(t, t_buffer);
                TEST(ret == test2_words[i]);
                ret = trie_nsearch(t, t_buffer, strlen(t_buffer));
                TEST(ret == test2_words[i]);

                // mixed
                for (word = test2_words[i], t_cursor = t_buffer; *word; ++word)
                    *t_cursor++ = ((size_t)t_cursor & 1) ? tolower(*word) : *word;
                *t_cursor = 0;
                ret = trie_search(t, t_buffer);
                TEST(ret == test2_words[i]);
                ret = trie_nsearch(t, t_buffer, strlen(t_buffer));
                TEST(ret == test2_words[i]);

                // mixed
                for (word = test2_words[i], t_cursor = t_buffer; *word; ++word)
                    *t_cursor++ = ((size_t)t_cursor & 1) ? *word : tolower(*word);
                *t_cursor = 0;
                ret = trie_search(t, t_buffer);
                TEST(ret == test2_words[i]);
                ret = trie_nsearch(t, t_buffer, strlen(t_buffer));
                TEST(ret == test2_words[i]);
        }

        {       const char *prefix = "CMAKE_EXE_LINKER_FLAGS";
                size_t count = trie_count(t, prefix);
                TEST(1 == count);
        }

        {       const char *prefix = "CMAKE_+_";
                size_t count = trie_count(t, prefix);
                TEST(32 == count);
        }

        /* replace then prune, implied deletion */
        {
                const size_t size = trie_size(t);
                const int count = trie_count(t, "");
                int ret  = trie_replace(t, "CMAKE_+_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES", NULL, NULL);
                TEST2(0 == ret, "trie_replace");
                TEST(size == trie_size(t));             /* node count */
                TEST((count - 1) == trie_count(t, "")); /* non-null element count */

                ret = trie_prune(t);
                TEST2(1 == ret, "trie_prune()");
                TEST(size > trie_size(t));              /* node pruned */
                TEST((count - 1) == trie_count(t, "")); /* no change in elements */
        }

        /* iterator */
        {       struct trie_it *it = trie_it_create(t, "");
                unsigned count = 0;

                printf("\n=== iterator\n");
                if (! trie_it_done(it)) {
                        do {
                                printf(" %s=%p\n",
                                    (const char *)trie_it_key(it), (const char *)trie_it_data(it));
                                ++count;
                        } while (trie_it_next(it));
                }
                TEST(count == trie_count(t, ""));
                trie_it_free(it);
                printf("\n");
        }

        trie_free(t);
}


/////////////////////////////////////////////////////////////////////////////////////////
//  wild-card support
//

static void
test4(void)
{
        struct trie *t = trie_create();
        unsigned i;

        /* wild-card speical handling */
        for (i = 0; i < _countof(test2_words); ++i) {
                const int ret = trie_insert_wild(t, test2_words[i], (void *)test2_words[i]);
                TEST2(0 == ret, "trie_insert");
        }

        /* spot tests */

        // leading
        {       const char *word = "XXX_BINARY_DIR";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "+_BINARY_DIR"));
        }

        {       const char *word = "XXX_SOURCE_DIR";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "+_SOURCE_DIR"));
        }

        // tail
        {       const char *word = "CMAKE_MODULE_LINKER_FLAGS_XXX";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "CMAKE_MODULE_LINKER_FLAGS_+"));
        }

        // middle
        {       const char *word = "CMAKE_XXX_VISIBILITY_PRESET";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "CMAKE_+_VISIBILITY_PRESET"));
        }

        {       const char *word = "CMAKE_X_LINKER_PREFERENCE";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "CMAKE_+_LINKER_PREFERENCE"));
        }

        {       const char *word = "CMAKE_Y_LINKER_PREFERENCE_PROPAGATES";
                void *result = trie_search_wild(t, word);
                TEST(result && 0 == strcmp(result, "CMAKE_+_LINKER_PREFERENCE_PROPAGATES"));
        }

        /* iterator search */
        {       char search_word[128] = {0};
                unsigned w, l = 0;

                for (l = 1; l <= 3; ++l) {
                        for (w = 0; w < _countof(test2_words); ++w) {
                                const char *word = test2_words[w], *word_cursor = word;
                                char *search, *result;
                                int length = 0;

                                for (search = search_word;;) {
                                        const char c = *word_cursor++;

                                        if ('+' == c) {
                                                unsigned x;
                                                for (x = 0; x < l; ++x) {
                                                        *search++ = 'A' + x;
                                                }
                                        } else if (0 == (*search++ = c)) {
                                                --search;
                                                break;
                                        }
                                }
                                length = search - search_word;
                                assert(search_word[length] == 0);

                                result = trie_search_wild(t, search_word);
                                TEST(result && 0 == strcmp(result, word));

                                if (w & 0x01) { // remove nul-terminator
                                    search_word[length] = 'X';
                                    search_word[length+1] = 0;
                                }
                                result = trie_search_nwild(t, search_word, length);
                                TEST(result && 0 == strcmp(result, word));
                        }
                }
        }
        trie_free(t);
}


int
main(void)
{
        test1();
        test2();
        test3();
        test4();
}

/*end*/
