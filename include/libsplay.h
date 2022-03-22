#ifndef GR_LIBSPLAY_H_INCLUDED
#define GR_LIBSPLAY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libsplay_h,"$Id: libsplay.h,v 1.20 2022/03/21 14:55:28 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libsplay.h,v 1.20 2022/03/21 14:55:28 cvsuser Exp $
 *  A SPLAY tree is a self-adjusting binary search tree with the additional property that
 *  recently accessed elements are quick to access again. It performs basic operations such as
 *  insertion, look-up and removal in O(log n) amortized time. For many sequences of non-random
 *  operations, SPLAY trees perform better than other search trees, even when the specific
 *  pattern of the sequence is unknown. The SPLAY tree was invented by Daniel Dominic Sleator and
 *  Robert Endre Tarjan in 1985.
 *
 *  All normal operations on a binary search tree are combined with one basic operation, called
 *  splaying. Splaying the tree for a certain element rearranges the tree so that the element is
 *  placed at the root of the tree. One way to do this is to first perform a standard binary tree
 *  search for the element in question, and then use tree rotations in a specific fashion to
 *  bring the element to the top. Alternatively, a top-down algorithm can combine the search and
 *  the tree reorganization into a single phase.
 *
 *  Reference: https://www.cs.cmu.edu/~sleator/papers/self-adjusting.pdf
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#include <edtypes.h>
#include <splaytree.h>

__CBEGIN_DECLS

#ifndef SPLAY_INTERNAL_STATS
#define SPLAY_INTERNAL_STATS 0
#endif
#ifndef SPLAY_GLOBAL_STATS
#define SPLAY_GLOBAL_STATS 0
#endif

typedef struct _spblk {
    SPLAY_ENTRY(_spblk) _sp_node;
    void *              key;
    void *              data;
} SPBLK;

typedef SPLAY_HEAD(_sproot, _spblk)
                        _sproot;

typedef int (* SPCOMPARE)(struct _sproot *, struct _spblk *, struct _spblk *);
typedef void (* SPDELETE)(void *node, void *udate);

typedef struct _sptree {
    _sproot             sp_root;                /* root node (MUST BE FIRST, see compare) */
    int                 sp_count;               /* Number of items in the tree. */
    SPCOMPARE           sp_compare;

#if (SPLAY_INTERNAL_STATS)
    unsigned long       sp_lookups;             /* number of splookup's */
    unsigned long       sp_lkpcmps;             /* number of lookup comparisons */

    unsigned long       sp_enqs;                /* number of spenq's */
    unsigned long       sp_enqcmps;             /* compares in spenq */

    unsigned long       sp_splays;
    unsigned long       sp_splayloops;

#define SPI_INCR(x)     (x)++
#else
#define SPI_INCR(x)
#endif

} SPTREE;


#if (SPLAY_GLOBAL_STATS)
typedef struct SPSTATS {
    unsigned long       trees_alloced;
    unsigned long       spblk_alloced;
    unsigned long       spinit;
    unsigned long       spfree;
    unsigned long       spblk;
    unsigned long       spfreeblk;
    unsigned long       spempty;
    unsigned long       sphead;
    unsigned long       spenq;
    unsigned long       spenq_loops;
    unsigned long       spdeq;
    unsigned long       splay;
    unsigned long       sprotate;
    unsigned long       splookup;
    unsigned long       splookup_loops;
} SPSTATS;

extern SPSTATS          __spstats;
#define SP_INCR(x)      __spstats.x++
#define SP_DECR(x)      __spstats.x--

#else
#define SP_INCR(x)
#define SP_DECR(x)

#endif /*SPLAY_GLOBAL_STATS*/

       /* Return pointer to a new splay tree */
extern SPTREE *         spinit(void);

       /* New splay tree, using a user specified key comparator */
extern SPTREE *         spinit2(SPCOMPARE compare);

       /* Destroy the tree */
extern void             spzap(SPTREE * tree, SPDELETE deleter, void *udata);

       /* Free memory associated with root of tree */
extern void             spfree(SPTREE * tree);

       /* Allocate SPBLK structure */
extern SPBLK *          spblk(size_t sz);

       /* Free SPBLK structure */
extern void             spfreeblk(SPBLK * sp);

       /* Return TRUE if splay tree is empty */
extern int              spempty(SPTREE * tree);

       /* Return head of tree */
extern void             spenq(SPBLK * q, SPTREE * tree);

       /* Remove an item from the tree */
extern void             spdeq(SPBLK *, SPTREE *);

       /* Cause the tree to be resplayed with the designated node at the top of the tree 
        */
extern void             splay(SPBLK *, SPTREE *);

       /* Lookup a key in the tree, and bring the node to the top of the tree.
        */
extern SPBLK *          splookup(const void *, SPTREE *);

       /* Lookup a key in the tree, same as splookup(), but if we don't
        * find entry tells us if we found a partial match (ie. an ambiguity) 
        *
        * Note, only functions correctly if the tree was created using the
        * default comparsion function (i.e. spinit()).
        */
extern SPBLK *          sp_partial_lookup(const void *, SPTREE *, int *, SPBLK **);

       /* Return pointer to node at top of tree */
extern SPBLK *          sphead(SPTREE *);

       /* Visit each node in tree and call user callback
        * function, but not safe to manipulate tree using splay functions 
        */
extern void             spwalk(SPTREE *, void (*)(SPBLK *, void *), void *);

       /* Returns number of nodes in tree */
extern int              spsize(SPTREE *);

       /* Flattens splay tree by returning array of pointers to all nodes
        * in tree.  Last element is NULL. Array must be freed by caller 
        */
extern SPBLK **         spflatten(SPTREE *);

       /* Returns a string containing statistics on splay tree
        * operations of the passed tree 
        */
extern const char *     spstats(const SPTREE *, char *buffer, unsigned length);

        /* Dump the splay tree */
extern void             spprint(const SPTREE * tree);

__CEND_DECLS

#endif /*GR_LIBSPLAY_H_INCLUDED*/
