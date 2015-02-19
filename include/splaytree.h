#ifndef GR_SPLAYTREE_H_INCLUDED
#define GR_SPLAYTREE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_splaytree_h,"$Id: splaytree.h,v 1.6 2014/10/19 23:45:15 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $NetBSD: tree.h,v 1.8 2004/03/28 19:38:30 provos Exp $ */
/* $OpenBSD: tree.h,v 1.7 2002/10/17 21:51:54 art Exp $ */
/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*  Package:        Splay Trees

        These macros define data structures for managing splay trees.

    Description:

        A splay tree is a self-organizing data structure. Every
        operation on the tree causes a splay to happen. The splay moves
        the requested node to the root of the tree and partly rebalances
        it.

        This has the benefit that request locality causes faster lookups
        as the requested nodes move to the top of the tree. On the other
        hand, every lookup causes memory writes.

        The Balance Theorem bounds the total access time for m
        operations and n inserts on an initially empty tree as O((m +
        n)lg n). The amortized cost for a sequence of m accesses to a
        splay tree is O(lg n).

    See also:
        <Red-Black Trees> and <http://en.wikipedia.org/wiki/Splay_tree>

    Requirements:

        Kernel -    v2.0 or greater
        Header -    Declared in eossplay.h
        Library -   n/a

    Topic:          Usage
        
        Standard usage

        In the macro definitions, *TYPE* is the name tag of a user
        defined structure that must contain a field of type
        <SPLAY_ENTRY>, named *ENTRYNAME*. The argument *HEADNAME* is the
        name tag of a user defined structure that must be declared using
        the macros <SPLAY_HEAD>. The argument *NAME* has to be a unique
        name prefix for every tree that is defined. (See <Usage> for
        more details).

        The function prototypes are declared using <SPLAY_PROTOTYPE>.
        The function bodies are generated with <SPLAY_GENERATE>. See the
        examples below for further explanation of how these macros are
        used.

        A splay tree is headed by a structure defined by the
        <SPLAY_HEAD()> macro. A structure is declared as follows:

(start code)
            SPLAY_HEAD(HEADNAME, TYPE) head;
(end)

        where *HEADNAME* is the name of the structure to be defined, and
        struct TYPE is the type of the elements to be inserted into the
        tree.

        The <SPLAY_ENTRY()> macro declares a structure that allows
        elements to be connected in the tree.

        In order to use the functions that manipulate the tree
        structure, their prototypes need to be declared with the
        <SPLAY_PROTOTYPE()> macro, where *NAME* is a unique identifier
        for this particular tree. The *TYPE* argument is the type of the
        structure that is being managed by the tree. The *FIELD*
        argument is the name of the element defined by <SPLAY_ENTRY()>.

        The function bodies are generated with the <SPLAY_GENERATE()>
        macro. It takes the same arguments as the <SPLAY_PROTOTYPE()>
        macro, but should be used only once.

        Finally, the *CMP* argument is the name of a function used to
        compare tree nodes with each other. The function takes two
        arguments of type struct TYPE *. If the first argument is
        smaller than the second, the function returns a value smaller
        than zero. If they are equal, the function returns zero.
        Otherwise, it should return a value greater than zero. The
        compare function defines the order of the tree elements.

        The <SPLAY_INIT()> macro initializes the tree referenced by head.

        The splay tree can also be initialized statically by using the
        <SPLAY_INITIALIZER()> macro like this:

(start code)
            SPLAY_HEAD(HEADNAME, TYPE) head = SPLAY_INITIALIZER(&head);
(end)

        The <SPLAY_INSERT()> macro inserts the new element elm into the
        tree.

        The <SPLAY_REMOVE()> macro removes the element elm from the tree
        pointed by head.

        The <SPLAY_FIND()> macro can be used to find a particular
        element in the tree.

(start code)
            struct TYPE find, *res;
            find.key = 30;
            res = SPLAY_FIND(NAME, head, &find);
(end)

        The <SPLAY_ROOT()>, <SPLAY_MIN()>, <SPLAY_MAX()>, and
        <SPLAY_NEXT()> macros can be used to traverse the tree:

(start code)
            for (np = SPLAY_MIN(NAME, &head); np != NULL; np = SPLAY_NEXT(NAME, &head, np))
(end)

        or, for simplicity, one can use the SPLAY_FOREACH() macro;

(start code)
            SPLAY_FOREACH(np, NAME, head)
(end)

        The <SPLAY_EMPTY()> macro should be used to check whether a
        splay tree is empty.

    Topic:          Notes

        Trying to free a tree in the following way is a common error

(start code)
        SPLAY_FOREACH(var, NAME, head) {
            SPLAY_REMOVE(NAME, head, var);
            free(var);
        }
        free(head);
(end)

        Since var is freed, the <SPLAY_FOREACH()> macro refers to a
        pointer that may have been reallocated already. Proper code
        needs a second variable.

(start code)
        for (var = SPLAY_MIN(NAME, head); var != NULL; var = nxt) {
            nxt = SPLAY_NEXT(NAME, head, var);
            SPLAY_REMOVE(NAME, head, var);
            free(var);
        }
(end)

    Topic:          Example

        Coding example. The following example is an interface to an event 
        structure maintained within 'ev_timeout' order splay tree;

(start code)
#include <eosstd.h>
#include <eosrbtree.h>
#include <eosassert.h>

struct event {
        SPLAY_ENTRY (event)     ev_node;           // tree node
        long                    ev_timeout;        // key value
            :
            // additional fields ....
            :
};

static      SPLAY_HEAD(evtree, event) rbtree;      // Tree head construct

static int
compare(struct event *a, struct event *b)
{
        if (a->ev_timeout < b->ev_timeout)
                return (-1);
        else if (a->ev_timeout > b->ev_timeout)
                return (1);
        return (0);
}

SPLAY_PROTOTYPE(evtree, event, ev_node, compare);  // Generate interface prototypes
SPLAY_GENERATE(evtree, event, ev_node, compare);   // and definition


//      Insert a new node into the tree, if the timer value is 
//      non-unique scan the tree to find and locate the next 
//      incrementally unique timer value.
//
void
event_insert(struct event *ev)
{
        struct event *tmp;

        tmp = SPLAY_FIND(evtree, &rbtree, ev);     // Node already exists ?

        if (tmp != NULL) {                      
                do {                               // Find unique time
                        ev->ev_timeout++;
                        tmp = SPLAY_NEXT(evtree, &rbtree, tmp);
                } while (tmp != NULL && tmp->ev_timeout == ev->ev_timeout);
        }

        tmp = SPLAY_INSERT(evtree, &rbtree, ev);
        assert(tmp == NULL);
}


//
//      Remove a node from the tree
//
void
event_remove(struct event *ev)
{
        SPLAY_REMOVE(evtree, &rbtree, ev);
}
(end)

-------
    Macro:          SPLAY_PROTOTYPE

        Declares with the interface prototypes of a splay tree construct.

    Synopsis:
        SPLAY_PROTOTYPE(NAME, TYPE, FIELD, CMP)


-------
    Macro:          SPLAY_GENERATE

        Generates with the interface definition of a splay tree construct.

    Synopsis:
        SPLAY_GENERATE(NAME, TYPE, FIELD, CMP)

-------
    Macro:          SPLAY_ENTRY

        Declares a structure that allows elements to be connected in the splay tree.

    Synopsis:
        SPLAY_ENTRY(TYPE)


-------
    Macro:          SPLAY_HEAD

        Declares a structure that contents the splay tree head.

    Synopsis:
        SPLAY_HEAD(HEADNAME, TYPE)

    Example:
(start code)
        SPLAY_HEAD(HEADNAME, TYPE) head;
(end)

-------
    Macro:          SPLAY_INIT

        Initialize the splay tree referenced by _head_.

    Synopsis:
        void SPLAY_INIT(SPLAY_HEAD *head)

    See also:
        <SPLAY_INITIALIZER> and <SPLAY_HEAD>.

-------
    Macro:          SPLAY_INITIALIZER

        Static initialization of the splay tree referenced by _head_.

    Synopsis:
        SPLAY_INITIALIZER(SPLAY_HEAD *head)

    Example:
(start code)
            SPLAY_HEAD(HEADNAME, TYPE) head = SPLAY_INITIALIZER(&head);
(end)

    See also:
        <SPLAY_INIT> and <SPLAY_HEAD>.


-------
    Macro:          SPLAY_INSERT

        Inserts the new element _elm_ into the splay tree pointed by _head_.

    Synopsis:
        struct TYPE *SPLAY_INSERT(NAME, SPLAY_HEAD *head, struct TYPE *elm)

    Returns:
        SPLAY_INSERT return NULL if the element was inserted in the tree
        successfully, otherwise they return a pointer to the element
        with the colliding key.

    See also:
        <SPLAY_REMOVE>

-------
    Macro:          SPLAY_REMOVE

        Removes the element _elm_ from the splay tree pointed by _head_.

    Synopsis:
        struct TYPE *SPLAY_REMOVE(NAME, SPLAY_HEAD *head, struct TYPE *elm)

    Returns:
        SPLAY_REMOVE() returns the pointer to the removed element otherwise
        they return NULL to indicate an error.

    See also:
        <SPLAY_INSERT>


-------
    Macro:          SPLAY_FIND

        Find a particular element in a splay tree.

    Synopsis:
        struct TYPE *SPLAY_FIND(NAME, SPLAY_HEAD *head, struct TYPE *elm)

    Example:
(start code)
           struct TYPE find, *res;
           find.key = 30;
           res = SPLAY_FIND(NAME, head, &find);
(end)

    See also:
        <SPLAY_FOREACH>

-------
    Macro:          SPLAY_EMPTY

        Determine whether a splay tree is empty.

    Synopsis:
        bool SPLAY_EMPTY(SPLAY_HEAD *head)


-------
    Macro:          SPLAY_MIN

        Retrieve the first node, by key order, within a splay tree.

    Synopsis:
        struct TYPE *SPLAY_MIN(NAME, SPLAY_HEAD *head)

    Example:
(start code)
        for (var = SPLAY_MIN(NAME, head); var != NULL; var = nxt) {
            nxt = SPLAY_NEXT(NAME, head, var);
            SPLAY_REMOVE(NAME, head, var);
            free(var);
        }
(end)

    See also:
        <SPLAY_NEXT>, <SPLAY_MAX> and <SPLAY_ROOT>.

-------
    Macro:          SPLAY_NEXT

        Retrieve the next node, by key order, within a splay tree.

    Synopsis:
        struct TYPE *SPLAY_NEXT(NAME, SPLAY_HEAD *head, struct TYPE *elm)

    See also:
        <SPLAY_MIN>, <SPLAY_MAX> and <SPLAY_ROOT>.

-------
    Macro:          SPLAY_MAX

        Retrieve the last node, by key order, within a splay tree.

    Synopsis:
        struct TYPE *SPLAY_MAX(NAME, SPLAY_HEAD *head)

    See also:
        <SPLAY_MIN> and <SPLAY_NEXT>

-------
    Macro:          SPLAY_ROOT

        Retrieve the root node of a splay tree.

    Synopsis:
         struct TYPE *SPLAY_ROOT(SPLAY_HEAD *head)

    See also:
        <SPLAY_LEFT>, <SPLAY_RIGHT> and <SPLAY_PARENT>.

-------
    Macro:          SPLAY_LEFT

        Retrieve the left child of a splay tree node.

    Synopsis:
        struct TYPE *SPLAY_LEFT(struct TYPE *elm, SPLAY_ENTRY NAME)

    See also:
        <SPLAY_ROOT>, <SPLAY_RIGHT> and <SPLAY_PARENT>.


-------
    Macro:          SPLAY_RIGHT

        Retrieve the right child of a splay tree node.

    Synopsis:
        struct TYPE *SPLAY_RIGHT(struct TYPE *elm, SPLAY_ENTRY NAME)

    See also:
        <SPLAY_ROOT>, <SPLAY_LEFT> and <SPLAY_PARENT>.

-------
    Macro:          SPLAY_PARENT

        Retrieve the parent a splay tree node.

    Synopsis:
        struct TYPE *SPLAY_PARENT(struct TYPE *elm, SPLAY_ENTRY NAME)

    See also:
        <SPLAY_ROOT>, <SPLAY_RIGHT> and <SPLAY_PARENT>.

-------
    Macro:          SPLAY_FOREACH

        Walk each node within the splay tree.

    Synopsis:
        SPLAY_FOREACH(VARNAME, NAME, SPLAY_HEAD *head)

    Example:
(start code)
        SPLAY_FOREACH(var, NAME, head) {
            print_node(var);
        }
(end)

    See also:
        <SPLAY_MIN> and <SPLAY_ROOT>.

 *...........................................................................
 */

#define SPLAY_HEAD(name, type)                                          \
struct name {                                                           \
        struct type *sph_root; /* root of the tree */                   \
}

#define SPLAY_INITIALIZER(root)                                         \
        { NULL }

#define SPLAY_INIT(root) do {                                           \
        (root)->sph_root = NULL;                                        \
} while (/*CONSTCOND*/ 0)

#define SPLAY_ENTRY(type)                                               \
struct {                                                                \
        struct type *spe_left; /* left element */                       \
        struct type *spe_right; /* right element */                     \
}

#define SPLAY_LEFT(elm, field)          (elm)->field.spe_left
#define SPLAY_RIGHT(elm, field)         (elm)->field.spe_right
#define SPLAY_ROOT(head)                (head)->sph_root
#define SPLAY_EMPTY(head)               (SPLAY_ROOT(head) == NULL)

/* SPLAY_ROTATE_{LEFT,RIGHT} expect that tmp hold SPLAY_{RIGHT,LEFT} */
#define SPLAY_ROTATE_RIGHT(head, tmp, field) do {                       \
        SPLAY_LEFT((head)->sph_root, field) = SPLAY_RIGHT(tmp, field);  \
        SPLAY_RIGHT(tmp, field) = (head)->sph_root;                     \
        (head)->sph_root = tmp;                                         \
} while (/*CONSTCOND*/ 0)

#define SPLAY_ROTATE_LEFT(head, tmp, field) do {                        \
        SPLAY_RIGHT((head)->sph_root, field) = SPLAY_LEFT(tmp, field);  \
        SPLAY_LEFT(tmp, field) = (head)->sph_root;                      \
        (head)->sph_root = tmp;                                         \
} while (/*CONSTCOND*/ 0)

#define SPLAY_LINKLEFT(head, tmp, field) do {                           \
        SPLAY_LEFT(tmp, field) = (head)->sph_root;                      \
        tmp = (head)->sph_root;                                         \
        (head)->sph_root = SPLAY_LEFT((head)->sph_root, field);         \
} while (/*CONSTCOND*/ 0)

#define SPLAY_LINKRIGHT(head, tmp, field) do {                          \
        SPLAY_RIGHT(tmp, field) = (head)->sph_root;                     \
        tmp = (head)->sph_root;                                         \
        (head)->sph_root = SPLAY_RIGHT((head)->sph_root, field);        \
} while (/*CONSTCOND*/ 0)

#define SPLAY_ASSEMBLE(head, node, left, right, field) do {             \
        SPLAY_RIGHT(left, field) = SPLAY_LEFT((head)->sph_root, field); \
        SPLAY_LEFT(right, field) = SPLAY_RIGHT((head)->sph_root, field);\
        SPLAY_LEFT((head)->sph_root, field) = SPLAY_RIGHT(node, field); \
        SPLAY_RIGHT((head)->sph_root, field) = SPLAY_LEFT(node, field); \
} while (/*CONSTCOND*/ 0)

/* Generates prototypes */

#define SPLAY_PROTOTYPE(prefix, name, type, field)                      \
void prefix##_SPLAY(struct name *, struct type *);                      \
void prefix##_SPLAY_MINMAX(struct name *, int);                         \
struct type *prefix##_SPLAY_INSERT(struct name *, struct type *);       \
struct type *prefix##_SPLAY_REMOVE(struct name *, struct type *);       \
struct type *prefix##_SPLAY_FIND(struct name *head, struct type *elm);  \
struct type *prefix##_SPLAY_NEXT(struct name *head, struct type *elm);  \
struct type *prefix##_SPLAY_MIN_MAX(struct name *head, int val);

/* Main splay operation.
 * Moves node close to the key of elm to top
 */
#define SPLAY_GENERATE(prefix, name, type, field, cmp)                  \
struct type *                                                           \
prefix##_SPLAY_INSERT(struct name *head, struct type *elm)              \
{                                                                       \
    if (SPLAY_EMPTY(head)) {                                            \
            SPLAY_LEFT(elm, field) = SPLAY_RIGHT(elm, field) = NULL;    \
    } else {                                                            \
            int __comp;                                                 \
            prefix##_SPLAY(head, elm);                                  \
            __comp = (cmp)(head, elm, (head)->sph_root);                \
            if(__comp < 0) {                                            \
                    SPLAY_LEFT(elm, field) = SPLAY_LEFT((head)->sph_root, field);\
                    SPLAY_RIGHT(elm, field) = (head)->sph_root;         \
                    SPLAY_LEFT((head)->sph_root, field) = NULL;         \
            } else if (__comp > 0) {                                    \
                    SPLAY_RIGHT(elm, field) = SPLAY_RIGHT((head)->sph_root, field);\
                    SPLAY_LEFT(elm, field) = (head)->sph_root;          \
                    SPLAY_RIGHT((head)->sph_root, field) = NULL;        \
            } else                                                      \
                    return ((head)->sph_root);                          \
    }                                                                   \
    (head)->sph_root = (elm);                                           \
    return (NULL);                                                      \
}                                                                       \
                                                                        \
struct type *                                                           \
prefix##_SPLAY_REMOVE(struct name *head, struct type *elm)              \
{                                                                       \
        struct type *__tmp;                                             \
        if (SPLAY_EMPTY(head))                                          \
                return (NULL);                                          \
        prefix##_SPLAY(head, elm);                                      \
        if ((cmp)(head, elm, (head)->sph_root) == 0) {                  \
                if (SPLAY_LEFT((head)->sph_root, field) == NULL) {      \
                        (head)->sph_root = SPLAY_RIGHT((head)->sph_root, field);\
                } else {                                                \
                        __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
                        (head)->sph_root = SPLAY_LEFT((head)->sph_root, field);\
                        prefix##_SPLAY(head, elm);                      \
                        SPLAY_RIGHT((head)->sph_root, field) = __tmp;   \
                }                                                       \
                return (elm);                                           \
        }                                                               \
        return (NULL);                                                  \
}                                                                       \
                                                                        \
void                                                                    \
prefix##_SPLAY(struct name *head, struct type *elm)                     \
{                                                                       \
        struct type __node, *__left, *__right, *__tmp;                  \
        int __comp;                                                     \
\
        SPLAY_LEFT(&__node, field) = SPLAY_RIGHT(&__node, field) = NULL;\
        __left = __right = &__node;                                     \
\
        while ((__comp = (cmp)(head, elm, (head)->sph_root)) != 0) {    \
                if (__comp < 0) {                                       \
                        __tmp = SPLAY_LEFT((head)->sph_root, field);    \
                        if (__tmp == NULL)                              \
                                break;                                  \
                        if ((cmp)(head, elm, __tmp) < 0){               \
                                SPLAY_ROTATE_RIGHT(head, __tmp, field); \
                                if (SPLAY_LEFT((head)->sph_root, field) == NULL)\
                                        break;                          \
                        }                                               \
                        SPLAY_LINKLEFT(head, __right, field);           \
                } else if (__comp > 0) {                                \
                        __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
                        if (__tmp == NULL)                              \
                                break;                                  \
                        if ((cmp)(head, elm, __tmp) > 0){               \
                                SPLAY_ROTATE_LEFT(head, __tmp, field);  \
                                if (SPLAY_RIGHT((head)->sph_root, field) == NULL)\
                                        break;                          \
                        }                                               \
                        SPLAY_LINKRIGHT(head, __left, field);           \
                }                                                       \
        }                                                               \
        SPLAY_ASSEMBLE(head, &__node, __left, __right, field);          \
}                                                                       \
                                                                        \
/* Splay with either the minimum or the maximum element */              \
/* Used to find minimum or maximum element in tree. */                  \
void prefix##_SPLAY_MINMAX(struct name *head, int __comp)               \
{                                                                       \
        struct type __node, *__left, *__right, *__tmp;                  \
                                                                        \
        SPLAY_LEFT(&__node, field) = SPLAY_RIGHT(&__node, field) = NULL;\
        __left = __right = &__node;                                     \
                                                                        \
        while (1) {                                                     \
                if (__comp < 0) {                                       \
                        __tmp = SPLAY_LEFT((head)->sph_root, field);    \
                        if (__tmp == NULL)                              \
                                break;                                  \
                        if (__comp < 0){                                \
                                SPLAY_ROTATE_RIGHT(head, __tmp, field); \
                                if (SPLAY_LEFT((head)->sph_root, field) == NULL)\
                                        break;                          \
                        }                                               \
                        SPLAY_LINKLEFT(head, __right, field);           \
                } else if (__comp > 0) {                                \
                        __tmp = SPLAY_RIGHT((head)->sph_root, field);   \
                        if (__tmp == NULL)                              \
                                break;                                  \
                        if (__comp > 0) {                               \
                                SPLAY_ROTATE_LEFT(head, __tmp, field);  \
                                if (SPLAY_RIGHT((head)->sph_root, field) == NULL)\
                                        break;                          \
                        }                                               \
                        SPLAY_LINKRIGHT(head, __left, field);           \
                }                                                       \
        }                                                               \
        SPLAY_ASSEMBLE(head, &__node, __left, __right, field);          \
}                                                                       \
                                                                        \
/* Finds the node with the same key as elm */                           \
/*static _inline*/ struct type *                                        \
prefix##_SPLAY_FIND(struct name *head, struct type *elm)                \
{                                                                       \
        if (SPLAY_EMPTY(head))                                          \
                return(NULL);                                           \
        prefix##_SPLAY(head, elm);                                      \
        if ((cmp)(head, elm, (head)->sph_root) == 0)                    \
                return (head->sph_root);                                \
        return (NULL);                                                  \
}                                                                       \
                                                                        \
/*static _inline*/ struct type *                                        \
prefix##_SPLAY_NEXT(struct name *head, struct type *elm)                \
{                                                                       \
        prefix##_SPLAY(head, elm);                                      \
        if (SPLAY_RIGHT(elm, field) != NULL) {                          \
                elm = SPLAY_RIGHT(elm, field);                          \
                while (SPLAY_LEFT(elm, field) != NULL) {                \
                        elm = SPLAY_LEFT(elm, field);                   \
                }                                                       \
        } else                                                          \
                elm = NULL;                                             \
        return (elm);                                                   \
}                                                                       \
                                                                        \
/*static *_inline*/ struct type *                                       \
prefix##_SPLAY_MIN_MAX(struct name *head, int val)                      \
{                                                                       \
        prefix##_SPLAY_MINMAX(head, val);                               \
        return (SPLAY_ROOT(head));                                      \
}


#define SPLAY_NEGINF                    -1
#define SPLAY_INF                       1

#define SPLAY_INSERT(prefix, x, y)      prefix##_SPLAY_INSERT(x, y)
#define SPLAY_REMOVE(prefix, x, y)      prefix##_SPLAY_REMOVE(x, y)
#define SPLAY_FIND(prefix, x, y)        prefix##_SPLAY_FIND(x, y)
#define SPLAY_NEXT(prefix, x, y)        prefix##_SPLAY_NEXT(x, y)
#define SPLAY_MIN(prefix, x)            (SPLAY_EMPTY(x) ? NULL          \
                                        : prefix##_SPLAY_MIN_MAX(x, SPLAY_NEGINF))
#define SPLAY_MAX(prefix, x)            (SPLAY_EMPTY(x) ? NULL          \
                                        : prefix##_SPLAY_MIN_MAX(x, SPLAY_INF))

#define SPLAY_FOREACH(x, name, head)                                    \
        for ((x) = SPLAY_MIN(name, head);                               \
             (x) != NULL;                                               \
             (x) = SPLAY_NEXT(name, head, x))

#endif /*GR_SPLAYTREE_H_INCLUDED*/
