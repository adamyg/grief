#include <edidentifier.h>
__CIDENT_RCSID(gr_stable_c,"$Id: stable.c,v 1.14 2017/01/29 04:33:31 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: stable.c,v 1.14 2017/01/29 04:33:31 cvsuser Exp $
 * String tables ...
 *
 *   Linear Hashing is a dynamic hash table algorithm invented by Witold Litwin in 1980.
 *   Linear hashing allows for the expansion of the hash table one slot at a time. The
 *   frequent single slot expansion can very effectively control the length of the collision
 *   chain. The cost of hash table expansion is spread out across each hash table insertion
 *   operation, as opposed to be incurred all at once.
 *
 * Algorithm Details:
 *
 *   As usual, a hash function controls the address calculation of linear hashing. In linear
 *   hashing, the address calculation is always bounded by a size that is a power of two. A
 *   hash function (or hash algorithm) is a way of creating a small digital fingerprint from
 *   any kind of data. ...
 *
 *      address(level,key) = hash(key) mod (2level)
 *
 *   The 'split' variable controls the read operation, and the expansion operation.
 *
 *   A read operation would use address(level,key) (himask) if address(level,key) is greater
 *   than or equal to the 'split' variable. Otherwise, address(level+1,key) is used (lomask).
 *
 *   A linear hashing table expansion operation would consist of rehashing the entries at
 *   slot location indicated by the 'split' variable to the target slot location of
 *   address(level+1,key). The 'split' variable is incremented by 1 at the end of the
 *   expansion operation. If the 'split' variable reaches 2level, then the 'level' variable
 *   is incremented by 1, and the 'split' variable is reset to 0.
 *
 *   There is some flexibility in choosing how often the expansion operations are performed.
 *   This implementation controls the expansion using the defined load factor, which when
 *   exceeded forces the table to double.
 *
 * Handling collisions:
 *
 *   Collsions within the hash are handling by the buckets, i.e., each slot holds a linked
 *   list of table entries, all with the same reduced hash value.
 *
 * Copyright (c) 1998 - 2017, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */
#include <editor.h>

#if defined(MAIN)
#undef NDEBUG           /*enforce asserts*/
#endif
#include <assert.h>
#include "stable.h"

#if defined(MAIN)
#define _D(x)           printf x;
#else
#define _D(x)
#endif

#ifndef MKMAGIC
#define MKMAGIC(a, b, c, d) \
                (((unsigned long)(a))<<24|((unsigned long)(b))<<16|(c)<<8|(d))
#endif

#define STMAGIC         MKMAGIC('s','t','b','l')
#define SNMAGIC         MKMAGIC('s','t','n','d')
#define SCMAGIC         MKMAGIC('s','t','c','r')

int
stbl_open(stable_t *st, unsigned buckets, unsigned factor)
{
    size_t size;
    int i;

#define MAXBITS         24
    st->magic = STMAGIC;
    st->buckets = 1;
    for (i = 1; i < MAXBITS; i++) {
        if ((st->buckets <<= 1) >= buckets) {
            break;
        }
    }
    size = sizeof(stblnode_t *) * buckets;
    if (i >= MAXBITS || 0 == size ||
                (st->table = chk_alloc(size)) == NULL) {
        st->buckets = 0;
        return (-1);
    }
    (void)memset(st->table, 0, size);

    st->himask = (0xffffffffL >> (32 - i));
    st->lomask = st->himask >> 1;
    st->split  = st->buckets = buckets;
    st->nodes  = 0;
    if ((st->factor = factor) < 1) {
        st->factor = 1;                         /* buckets * factor */
    }
    _D(("open(buckets=%ld, mask=0x%lx)\n", st->buckets, st->himask))
    return (0);
}


void
stbl_close(stable_t *st)
{
    assert(st->magic == STMAGIC);

    stbl_clear(st);
    chk_free(st->table);
    st->table = NULL;
    st->buckets = 0;
}


void
stbl_clear(stable_t *st)
{
    stblnode_t *p, *p0;
    unsigned bucket;

    assert(st->magic == STMAGIC);

    for (bucket = 0; bucket < st->buckets; bucket++) {
        p = st->table[ bucket ];
        while (p) {
            p0 = p->__next;
            p->__magic = 0;
            chk_free(p);
            p = p0;
        }
        st->table[ bucket ] = NULL;
    }
}


static unsigned
strhash(const char *str, int len)
{
    /*  Source: djb2
     *
     *      this algorithm (k=33) was first reported by dan bernstein many years ago in
     *      comp.lang.c. another version of this algorithm (now favored by bernstein)
     *      uses xor:
     *
     *          hash(i) = hash(i - 1) * 33 ^ str[i];
     *
     *      the magic of number 33 (why it works better than many other constants,
     *      prime or not) has never been adequately explained.
     */
    unsigned hash = 5381; int c;

    while (0 != (c = *str++) && len-- > 0) {
        hash = ((hash << 5) + hash) + c;        /* hash * 33 + c */
    }
    return hash;
}


static stblnode_t *
strfind(stable_t *st, const char *str, int len, unsigned hash)
{
    unsigned bucket;
    stblnode_t *p;

    assert(str && len > 0);
    bucket = hash & st->himask;
    if (bucket >= st->split) {
        bucket = hash & st->lomask;             /* not split */
    }
    assert(bucket < st->buckets);
    p = st->table[ bucket ];
    assert(p == NULL || p->__magic == SNMAGIC);

    while (p && (p->__hash != hash || strncmp(p->__str, str, len) != 0)) {
        assert(p->__magic == SNMAGIC);
        p = p->__next;
    }
    return p;
}


static void
strsplit(stable_t *st)
{
    stblnode_t *p, *p0, *l, *h;
    unsigned long low, bucket;
    int moved = 0;

    if (st->split >= st->buckets) {
        return;
    }
    low = st->split - (st->buckets >> 1);
    _D(("splitting %lu->%lu/%lu\n", low, low, st->split))
    p = st->table[ low ];
    st->table[ low ] = NULL;
    l = h = NULL;
    while (p) {
        assert(p->__magic == SNMAGIC);

        p0 = p->__next;
        bucket = p->__hash & st->himask;

        _D((" %lu ", bucket))
        assert(bucket == low || bucket == st->split);
        if (bucket == low) {
            if (l == NULL) {
                st->table[ bucket ] = p;
            } else {
                l->__next = p;
            }
            l = p;

        } else {
            if (h == NULL) {
                st->table[ bucket ] = p;
            } else {
                h->__next = p;
            }
            h = p;
        }
        p->__next = NULL;
        p = p0;
        moved++;
    }
    _D((".. moved %d \n", moved))
    ++st->split;
}


static stblnode_t *
strinsert(stable_t *st, const char *str, unsigned hash)
{
    unsigned bucket;
    stblnode_t *node;
    int len;

    /* pre-existing */
    if (0 == st->buckets) {
        return NULL;
    }

    /* split required? */
    if (st->split == st->buckets &&
            st->nodes >= (st->buckets * st->factor)) {
        /* split table */
        size_t size;
        char *ntable;

        size = sizeof(stblnode_t *) * (st->buckets << 1);
        if ((ntable = chk_realloc(st->table, size)) == NULL) {
            return (NULL);
        }
        memset(ntable + (size >> 1), 0, size >> 1);
        st->table = (void *)ntable;
        st->buckets <<= 1;
        st->lomask = st->himask;
        st->himask = (st->himask << 1) | 1;

        _D(("new split (nodes=%lu, buckets=%lu, mask=0x%lx)\n", \
                st->nodes, st->buckets, st->himask))
    }

    /* split if required */
    strsplit(st);

    /* new node */
    len = strlen(str);
    if ((node = chk_alloc(sizeof(stblnode_t) + len + 1)) == 0) {
        return NULL;
    }

    node->__magic = SNMAGIC;
    node->__hash  = hash;
    (void) strcpy(node->__str, str);
    node->__u.i32 = 0;
    node->__u.ptr = NULL;

    bucket = hash & st->himask;
    if (bucket >= st->split) {
        bucket = hash & st->lomask;             /* not split */
    }

    assert(bucket < st->buckets);
    assert(NULL == st->table[ bucket ] || st->table[ bucket ]->__magic == SNMAGIC);

    node->__next = st->table[ bucket ];
    st->table[ bucket ] = node;
    ++st->nodes;
    return (node);
}


stblnode_t *
stbl_new(stable_t *st, const char *str)
{
    const int len = strlen(str);
    unsigned hash;
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    hash = strhash(str, len);
    if ((p = strfind(st, str, len, hash)) != NULL) {
        return ((void *) -1);
    }
    return strinsert(st, str, hash);
}


stblnode_t *
stbl_nnew(stable_t *st, const char *str, int len)
{
    unsigned hash;
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    hash = strhash(str, len);
    if ((p = strfind(st, str, len, hash)) != NULL) {
        return ((void *) -1);
    }
    return strinsert(st, str, hash);
}


int
stbl_delete(stable_t *st, const char *str)
{
    return stbl_ndelete(st, str, strlen(str));
}


int
stbl_ndelete(stable_t *st, const char *str, int len)
{
    unsigned hash, bucket;
    stblnode_t *p, *p0;
    int ret;

    assert(st);
    assert(st->magic == STMAGIC);
    assert(str && len > 0);

    if (st->buckets == 0 || str == NULL || *str == '\0') {
        ret = -1;                               /* error */

    } else {
        /* hash */
        hash = strhash(str, len);
        bucket = hash & st->himask;
        if (bucket >= st->split) {
            bucket = hash & st->lomask;         /* not split */
        }

        /* locate */
        assert(bucket < st->buckets);
        p0 = NULL;
        p = st->table[ bucket ];
        assert(p == NULL || p->__magic == SNMAGIC);

        while (p && (p->__hash != hash || strcmp(p->__str, str) != 0)) {
            assert(p->__magic == SNMAGIC);
            p0 = p;
            p = p->__next;
        }

        /* and remove (if found) */
        if (NULL == p) {
            ret = 1;                            /* not found */
        } else {
            if (p == st->table[ bucket ]) {     /* head */
                st->table[ bucket ] = p->__next;
                assert(NULL == p0);
            } else {
                assert(NULL != p0);
                p0->__next = p->__next;         /* .. within chain */
            }
            p->__magic = 0;
            chk_free(p);
            --st->nodes;
            ret = 0;                            /* success */
        }
    }
    return (ret);
}


int
stbl_count(stable_t *st)
{
    assert(st);
    assert(st->magic == STMAGIC);
    return st->nodes;
}


const char *
stbl_insert(stable_t *st, const char *str)
{
    const int len = strlen(str);
    unsigned hash;
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    hash = strhash(str, len);
    if ((p = strfind(st, str, len, hash)) == NULL) {
        p = strinsert(st, str, hash);
    }
    return (p ? p->__str : NULL);
}


const char *
stbl_ninsert(stable_t *st, const char *str, int len)
{
    unsigned hash;
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    hash = strhash(str, len);
    if ((p = strfind(st, str, len, hash)) == NULL) {
        p = strinsert(st, str, hash);
    }
    return (p ? p->__str : NULL);
}


const char *
stbl_exist(stable_t *st, const char *str)
{
    const int len = strlen(str);
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    p = strfind(st, str, len, strhash(str, len));
    return (p ? p->__str : NULL);
}


const char *
stbl_nexist(stable_t *st, const char *str, int len)
{
    stblnode_t *p;

    assert(st);
    assert(st->magic == STMAGIC);
    p = strfind(st, str, len, strhash(str, len));
    return (p ? p->__str : NULL);
}


stblnode_t *
stbl_find(stable_t *st, const char *str)
{
    const int len = strlen(str);

    return strfind(st, str, len, strhash(str, len));
}


stblnode_t *
stbl_nfind(stable_t *st, const char *str, int len)
{
    return strfind(st, str, len, strhash(str, len));
}


stblnode_t *
stbl_first(stable_t *st, stblcursor_t *cursor)
{
    assert(st && cursor);
    assert(st->magic == STMAGIC);
    memset(cursor, 0, sizeof(stblcursor_t));
    cursor->magic = SCMAGIC;
    cursor->st = st;
 /* cursor->seqno = st->seqno; */
    return stbl_next(cursor);
}


stblnode_t *
stbl_next(stblcursor_t *cursor)
{
    stblnode_t *node = NULL;
    stable_t *st;

    assert(cursor);
    assert(cursor->magic == SCMAGIC);
    st = cursor->st;
 /* assert(cursor->seqno == st->seqno); */
    if (NULL != (node = cursor->node)) {
        node = node->__next;
    }
    if (NULL == node) {
        unsigned bucket = cursor->bucket;

        while (NULL == node && bucket < st->buckets) {
            node = st->table[ bucket++ ];
        }
        cursor->bucket = bucket;
    }
    cursor->node = node;
    return node;
}


#if defined(MAIN)
void *  chk_alloc(size_t size) { return malloc(size); }
void *  chk_salloc(const char *s) { return strdup(s); }
void *  chk_realloc(void *p, size_t size) { return realloc(p, size); }
void    chk_free(void *p) { free(p); }

#define STRINGS         10000

static const char *     strings[STRINGS];


int
main(void)
{
    stable_t tbl;
    char buffer[32];
    const char *p;
    int i;

    stbl_open(&tbl, 8, 3);
    for (i = 1; i < STRINGS; i++) {
        sprintf(buffer, "0x%l06x", (long) i>>1);
        p = stbl_insert(&tbl, buffer);
        strings[i] = stbl_insert(&tbl, buffer);
        assert(strings[i] != NULL);
        assert(stbl_exist(&tbl, buffer) == strings[i]);
    }
    for (i = 1; i < STRINGS; i++) {
        sprintf(buffer, "0x%l06x", (long) i>>1);
        assert(stbl_exist(&tbl, buffer) == strings[i]);
    }
    stbl_close(&tbl);
    return 0;
}
#endif  /*XXX*/
/*end*/
