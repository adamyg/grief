#ifndef GR_LIBLLIST_H_INCLUDED
#define GR_LIBLLIST_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_llist_h,"$Id: libllist.h,v 1.22 2025/01/13 16:20:07 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: libllist.h,v 1.22 2025/01/13 16:20:07 cvsuser Exp $
 * Linked list management system
 *
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#ifndef LLIST_STATS
#define LLIST_STATS     0
#endif

#include <edtypes.h>
#include <tailqueue.h>

__CBEGIN_DECLS

typedef void (*ListNodeFree_t)(void *);

typedef struct llist {
    long                ll_magic;
    TAILQ_ENTRY(llist)  ll_node;
    void *              ll_owner;
    char *              ll_elem;
    ListNodeFree_t *    ll_freeno;
} List, *List_p;

typedef struct lhead {
    long                lh_magic;
    TAILQ_HEAD(lqueue, llist)
                        lh_queue;
    int                 lh_count;
} Head, *Head_p;

#define LL_HEAD             0x4c48
#define LL_LIST             0x4c4c

#define LL_MAGIC(lp)    { \
                            if (lp == 0 || lp->ll_magic != LL_LIST) \
                            ll_magic(lp); \
                        }

#define LL_HMAGIC(hp)   { \
                            if (hp == 0 || hp->lh_magic != LL_HEAD) \
                            ll_hmagic(hp); \
                        }

#if defined(LLIST_STATS) && (LLIST_STATS)

#define LIST_INCR(x)        __list_stats.x++
#define LIST_DECR(x)        __list_stats.x--

extern struct list_stats {
    unsigned long       hd_alloced;
    unsigned long       lp_alloced;
    unsigned long       ll_alloc;
    unsigned long       ll_append;
    unsigned long       ll_clear;
    unsigned long       ll_delete;
    unsigned long       ll_elem;
    unsigned long       ll_first;
    unsigned long       ll_last;
    unsigned long       ll_free;
    unsigned long       ll_hook;
    unsigned long       ll_init;
    unsigned long       ll_insert;
    unsigned long       ll_lookup;
    unsigned long       ll_next;
    unsigned long       ll_pop;
    unsigned long       ll_prev;
    unsigned long       ll_push;
    unsigned long       ll_remove;
    unsigned long       ll_unhook;
} __list_stats;

extern struct list_stats    __list_stats;

#else

#define LIST_INCR(x)
#define LIST_DECR(x)

#endif

extern Head_p               ll_init(void);
extern void                 ll_free(Head_p);
extern int                  ll_length(Head_p);

extern struct llist *       ll_alloc(void * atom);
extern struct llist *       ll_append(Head_p, void * elem);
extern void                 ll_clear(Head_p);
extern void                 ll_clear2(Head_p, ListNodeFree_t freenode);
extern void                 ll_delete(List_p);
extern void *               ll_elem(List_p);
extern struct llist *       ll_first(Head_p);
extern struct llist *       ll_last(Head_p);
extern struct llist *       ll_hook(Head_p, List_p);
extern struct llist *       ll_insert(Head_p hp, void * atom, int pos);
extern struct llist *       ll_lookup(Head_p gp, void * atom, int (*)(void const *, void const *));
extern void                 ll_hmagic(Head_p);
extern void                 ll_magic(List_p);
extern int                  ll_hcheck(Head_p hp);
extern int                  ll_check(List_p lp);
extern struct llist *       ll_next(List_p);
extern struct llist *       ll_prev(List_p);
extern void *               ll_push(Head_p, void *);
extern int                  ll_pop(Head_p);
extern void                 ll_remove(List_p);
extern struct llist *       ll_unhook(List_p);

__CEND_DECLS

#endif /*GR_LIBLLIST_H_INCLUDED*/
