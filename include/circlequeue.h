#ifndef GR_CIRCLEQUEUE_H_INCLUDED
#define GR_CIRCLEQUEUE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_circlequeue_h,"$Id: circlequeue.h,v 1.8 2014/07/11 21:25:10 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1991, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*<<NaturalDoc>>

    Section:        Circular Queues
        Circular queues macros.

    Description:
        A circle queue is headed by a pair of pointers, one to the head of the list and
        the other to the tail of the list. The elements are doubly linked so that an
        arbitrary element can be removed without a need to traverse the list.

        New elements can be added to the list before or after an existing element, at
        the head of the list, or at the end of the list. A circle queue may be
        traversed in either direction, but has a more complex end of list detection.

        These macros are one of set of three that act on list/queue style data
        structues; lists <LIST_INIT>, tail queues <TAILQ_INIT>, and circular queues
        <CIRCLEQ_INIT>.

    Topic:          Details

        Basic functionality:

        All three structures support the following functionality;

        1.  Insertion of a new entry at the head of the list.

        2.  Insertion of a new entry before or after any element in the list.

        3.  Removal of any entry in the list.

        4.  Forward traversal through the list.

    Tail queues:

        Tail queues add the following functionality;

        1.  Entries can be added at the end of a list.

        However;

        1.  All list insertions and removals, except insertion before another
            element, must specify the head of the list.

        2.  Each head entry requires two pointers rather than one.

        3.  Code size is about 15% greater and operations run about 20% slower
            than lists.

    Circular queues:

        Circular queues add the following functionality;

        1.  Entries can be added at the end of a list.

        2.  They may be traversed backwards, from tail to head.

        However;

        1.  All list insertions and removals must specify the head of the list.

        2.  Each head entry requires two pointers rather than one.

        3.  The termination condition for traversal is more complex.

        4.  Code size is about 40% greater and operations run about 45% slower
            than lists.

    Usage:
        A circular queue is headed by a structure defined by the macro CIRCLEQ_HEAD.
        This structure contains a pair of pointers, one to the first element in the
        circular queue and the other to the last element in the circular queue. The
        elements are doubly linked so that an arbitrary element can be removed without
        traversing the queue. New elements can be added to the queue after an existing
        element, before an existing element, at the head of the queue, or at the end of
        the queue.

        A structure is declared as follows;

(start code)
        CIRCLEQ_HEAD(HEADNAME, TYPE) head;
(end)

        where is the name of the structure to be defined, and is the type of the
        elements to be linked into the circular queue. A pointer to the head of the
        circular queue can later be declared as:

(start code)
        struct HEADNAME *headp;
(end)
        Note:   The names head and are user selectable.

    Example:

(start code)

    CIRCLEQ_HEAD(circleq, entry) head;

    struct circleq *headp;                      // Circular queue head.

    struct entry {
        ...
        CIRCLEQ_ENTRY(entry) entries;           // Circular queue.
        ...
    } *n1, *n2, *np;

    CIRCLEQ_INIT(&head, entry);                 // Initialize the circular queue

    n1 = malloc(sizeof(struct entry));          // Insert at the head.
    CIRCLEQ_INSERT_HEAD(&head, entry, n1, entries);

    n1 = malloc(sizeof(struct entry));          // Insert at the tail.
    CIRCLEQ_INSERT_TAIL(&head, entry, n1, entries);

    n2 = malloc(sizeof(struct entry));          // Insert after.
    CIRCLEQ_INSERT_AFTER(&head, entry, n1, n2, entries);

    n2 = malloc(sizeof(struct entry));          // Insert before.
    CIRCLEQ_INSERT_BEFORE(&head, entry, n1, n2, entries);

                                                // Forward traversal.
    for (np = head.cqh_first; np != (struct entry *)&head;
            np = np->entries.cqe_next)
        np-> ...

OR  for (np = CIRCLEQ_FIRST(&head); np != CIRCLEQ_END(&head);
            np = CIRCLQ_NEXT(np, entries))
        np-> ...

                                                // Reverse traversal.
    for (np = head.cqh_last; np != (struct entry *)&head;
            np = np->entries.cqe_prev)
        np-> ...

OR  for (np = CIRCLEQ_LAST(&head); np != CIRCLEQ_END(&head);
            np = CIRCLQ_PREV(np, entries))
        np-> ...

                                                // Delete.
    while (head.cqh_first != (struct entry *)&head)
        CIRCLEQ_REMOVE(&head, entry, head.cqh_first, entries);

    History:
        The queue functions first appeared in 4.4BSD.

*/

/*  Macro:      CIRCLEQ_HEAD
 *      Declares a structure that connects the circular queue head.
 *
 *  Synopsis:
 *      CIRCLEQ_HEAD(HEADNAME, TYPE)
 */
#define CIRCLEQ_HEAD(name, type)                                        \
struct name {                                                           \
    struct type *cqh_first;     /* first element */                     \
    struct type *cqh_last;      /* last element */                      \
}


/*  Macro:      CIRCLEQ_ENTRY
 *      The macro CIRCLEQ_ENTRY declares a structure that connects the elements in the
 *      circular queue.
 *
 *  Synopsis:
 *      CIRCLEQ_ENTRY(NAME)
 */
#define CIRCLEQ_ENTRY(type)                                             \
struct {                                                                \
    struct type *cqe_next;      /* next element */                      \
    struct type *cqe_prev;      /* previous element */                  \
}


/*  Macro:      CIRCLEQ_INIT
 *      The macro CIRCLEQ_INIT initializes the circular queue referenced by head.
 *
 *  Synopsis:
 *      CIRCLEQ_INIT(CIRCLEQ_HEAD *head, TYPE type)
 */
#if defined(__cplusplus)
#define CIRCLEQ_INIT(head, type) {                                      \
    (head)->cqh_first = (struct type *)(head);                          \
    (head)->cqh_last = (struct type *)(head);                           \
}
#else
#define CIRCLEQ_INIT(head) {                                            \
    (head)->cqh_first = (void *)(head);                                 \
    (head)->cqh_last = (void *)(head);                                  \
}
#endif

    /* Circular queue functions */


/*  Macro:      CIRCLEQ_FIRST
 *      The macro CIRCLEQ_FIRST retrieves the first element within the queue.
 *
 *  Synopsis:
 *      CIRCLEQ_FIRST(CIRCLEQ_HEAD *head)
 */
#define CIRCLEQ_FIRST(head)         ((head)->cqh_first)


/*  Macro:      CIRCLEQ_LAST
 *      The macro CIRCLEQ_LAST retrieves the last element within the queue.
 *
 *  Synopsis:
 *      CIRCLEQ_LAST(CIRCLEQ_HEAD *head)
 */
#define CIRCLEQ_LAST(head)          ((head)->cqh_last)


/*  Macro:      CIRCLEQ_NEXT
 *      The macro CIRCLEQ_NEXT retrieves the next element within the queue.
 *
 *  Synopsis:
 *      CIRCLEQ_NEXT(TYPE *elm, CIRCLEQ_ENTRY NAME)
 */
#define CIRCLEQ_NEXT(elm, field)    ((elm)->field.cqe_next)


/*  Macro:      CIRCLEQ_PREV
 *      The macro CIRCLEQ_PREV retrieves the previous element within the queue.
 *
 *  Synopsis:
 *      CIRCLEQ_NEXT(TYPE *elm, CIRCLEQ_ENTRY NAME)
 */
#define CIRCLEQ_PREV(elm, field)    ((elm)->field.cqe_prev)


/*  Macro:      CIRCLEQ_END
 *      The macro CIRCLEQ_END represents the end-of-queue condition
 *
 *  Synopsis:
 *      CIRCLEQ_END(CIRCLEQ_HEAD *head)
 */
#define CIRCLEQ_END(head, type)     ((struct type *)(head))


/*  Macro:      CIRCLEQ_FOREACH
 *      Walks the queue elements
 *
 *  Synopsis:
 *      CIRCLEQ_FOREACH(TYPE *elm, CIRCLEQ_HEAD *head, CIRCLEQ_ENTRY NAME)
 */
#define CIRCLEQ_FOREACH(var, head, field)                               \
        for ((var) = CIRCLEQ_FIRST((head));                             \
            (var) != (void *)(head);                                    \
            (var) = CIRCLEQ_NEXT((var), field))


/*  Macro:      CIRCLEQ_FOREACH_REVERSE
 *      Walks the queue elements, in reverse order.
 *
 *  Synopsis:
 *      CIRCLEQ_FOREACH_REVERSE(TYPE *elm, CIRCLEQ_HEAD *head, CIRCLEQ_ENTRY NAME)
 */
#define CIRCLEQ_FOREACH_REVERSE(var, head, field)                       \
        for ((var) = CIRCLEQ_LAST((head));                              \
            (var) != (void *)(head);                                    \
            (var) = CIRCLEQ_PREV((var), field))


/*  Macro:      CIRCLEQ_INSERT_AFTER
 *      The macro CIRCLEQ_INSERT_AFTER inserts the new element elm after the element listelm.
 *
 *  Synopsis:
 *      CIRCLEQ_INSERT_AFTER(CIRCLEQ_HEAD *head, TYPE *listelm, TYPE *elm, CIRCLEQ_ENTRY NAME);
 */
#if defined(__cplusplus)
#define CIRCLEQ_INSERT_AFTER(head, listelm, elm, field, type) {         \
    (elm)->field.cqe_next = (listelm)->field.cqe_next;                  \
    (elm)->field.cqe_prev = (listelm);                                  \
    if ((listelm)->field.cqe_next == (struct type *)(head))             \
        (head)->cqh_last = (elm);                                       \
    else                                                                \
        (listelm)->field.cqe_next->field.cqe_prev = (elm);              \
    (listelm)->field.cqe_next = (elm);                                  \
}
#else
#define CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) {               \
    (elm)->field.cqe_next = (listelm)->field.cqe_next;                  \
    (elm)->field.cqe_prev = (listelm);                                  \
    if ((listelm)->field.cqe_next == (void *)(head))                    \
        (head)->cqh_last = (elm);                                       \
    else                                                                \
        (listelm)->field.cqe_next->field.cqe_prev = (elm);              \
    (listelm)->field.cqe_next = (elm);                                  \
}
#endif


/*  Macro:      CIRCLEQ_INSERT_BEFORE
 *      The macro CIRCLEQ_INSERT_BEFORE inserts the new element _elm_ before the element
 *      _listelm_.
 *
 *  Synopsis:
 *      CIRCLEQ_INSERT_BEFORE(CIRCLEQ_HEAD *head, TYPE *listelm, TYPE *elm, CIRCLEQ_ENTRY NAME);
 */
#if defined(__cplusplus)
#define CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field, type) {        \
    (elm)->field.cqe_next = (listelm);                                  \
    (elm)->field.cqe_prev = (listelm)->field.cqe_prev;                  \
    if ((listelm)->field.cqe_prev == (struct type *)(head))             \
        (head)->cqh_first = (elm);                                      \
    else                                                                \
        (listelm)->field.cqe_prev->field.cqe_next = (elm);              \
    (listelm)->field.cqe_prev = (elm);                                  \
}
#else
#define CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) {              \
    (elm)->field.cqe_next = (listelm);                                  \
    (elm)->field.cqe_prev = (listelm)->field.cqe_prev;                  \
    if ((listelm)->field.cqe_prev == (void *)(head))                    \
        (head)->cqh_first = (elm);                                      \
    else                                                                \
        (listelm)->field.cqe_prev->field.cqe_next = (elm);              \
    (listelm)->field.cqe_prev = (elm);                                  \
}
#endif


/*  Macro:      CIRCLEQ_INSERT_HEAD
 *      The macro CIRCLEQ_INSERT_HEAD inserts the new element _elm_ at the head of the
 *      circular queue.
 *
 *  Synopsis:
 *      CIRCLEQ_INSERT_HEAD(CIRCLEQ_HEAD *head, TYPE *elm, CIRCLEQ_ENTRY NAME);
 */
#if defined(__cplusplus)
#define CIRCLEQ_INSERT_HEAD(head, elm, field, type) {                   \
    (elm)->field.cqe_next = (head)->cqh_first;                          \
    (elm)->field.cqe_prev = (struct type *)(head);                      \
    if ((head)->cqh_last == (struct type *)(head))                      \
        (head)->cqh_last = (elm);                                       \
    else                                                                \
        (head)->cqh_first->field.cqe_prev = (elm);                      \
    (head)->cqh_first = (elm);                                          \
}
#else
#define CIRCLEQ_INSERT_HEAD(head, elm, field) {                         \
    (elm)->field.cqe_next = (head)->cqh_first;                          \
    (elm)->field.cqe_prev = (void *)(head);                             \
    if ((head)->cqh_last == (void *)(head))                             \
        (head)->cqh_last = (elm);                                       \
    else                                                                \
        (head)->cqh_first->field.cqe_prev = (elm);                      \
    (head)->cqh_first = (elm);                                          \
}
#endif


/*  Macro:      CIRCLEQ_INSERT_TAIL
 *      The macro CIRCLEQ_INSERT_TAIL inserts the new element elm at the end of the
 *      circular queue.
 *
 *  Synopsis:
 *      CIRCLEQ_INSERT_TAIL(CIRCLEQ_HEAD *head, TYPE *elm, CIRCLEQ_ENTRY NAME);
 */
#if defined(__cplusplus)
#define CIRCLEQ_INSERT_TAIL(head, elm, field, type) {                   \
    (elm)->field.cqe_next = (struct type *)(head);                      \
    (elm)->field.cqe_prev = (head)->cqh_last;                           \
    if ((head)->cqh_first == (struct type *)(head))                     \
        (head)->cqh_first = (elm);                                      \
    else                                                                \
        (head)->cqh_last->field.cqe_next = (elm);                       \
    (head)->cqh_last = (elm);                                           \
}
#else
#define CIRCLEQ_INSERT_TAIL(head, elm, field) {                         \
    (elm)->field.cqe_next = (void *)(head);                             \
    (elm)->field.cqe_prev = (head)->cqh_last;                           \
    if ((head)->cqh_first == (void *)(head))                            \
        (head)->cqh_first = (elm);                                      \
    else                                                                \
        (head)->cqh_last->field.cqe_next = (elm);                       \
    (head)->cqh_last = (elm);                                           \
}
#endif


/*  Macro:      CIRCLEQ_REMOVE
 *      The macro CIRCLEQ_REMOVE removes the element elm from the circular queue.
 *
 *  Synopsis:
 *      CIRCLEQ_REMOVE(CIRCLEQ_HEAD *head, TYPE *elm, CIRCLEQ_ENTRY NAME);
 */
#if defined(__cplusplus)
#define CIRCLEQ_REMOVE(head, elm, field, type) {                        \
    if ((elm)->field.cqe_next == (struct type *)(head))                 \
        (head)->cqh_last = (elm)->field.cqe_prev;                       \
    else                                                                \
        (elm)->field.cqe_next->field.cqe_prev =                         \
            (elm)->field.cqe_prev;                                      \
    if ((elm)->field.cqe_prev == (struct type *)(head))                 \
        (head)->cqh_first = (elm)->field.cqe_next;                      \
    else                                                                \
        (elm)->field.cqe_prev->field.cqe_next =                         \
            (elm)->field.cqe_next;                                      \
}
#else
#define CIRCLEQ_REMOVE(head, elm, field) {                              \
    if ((elm)->field.cqe_next == (void *)(head))                        \
        (head)->cqh_last = (elm)->field.cqe_prev;                       \
    else                                                                \
        (elm)->field.cqe_next->field.cqe_prev =                         \
            (elm)->field.cqe_prev;                                      \
    if ((elm)->field.cqe_prev == (void *)(head))                        \
        (head)->cqh_first = (elm)->field.cqe_next;                      \
    else                                                                \
        (elm)->field.cqe_prev->field.cqe_next =                         \
            (elm)->field.cqe_next;                                      \
}
#endif


/*  *****************************************************************   */

    /* Shared memory cirular queue definitions */

#define SH_CIRCLEQ_HEAD(name)                                           \
struct name {                                                           \
        size_t scqh_first;      /* first element */                     \
        size_t scqh_last;       /* last element */                      \
}

#define SH_CIRCLEQ_ENTRY                                                \
struct {                                                                \
        size_t scqe_next;       /* next element */                      \
        size_t scqe_prev;       /* previous element */                  \
}


    /* Shared memory circular queue functions */

#define SH_CIRCLEQ_FIRSTP(head, type)                                   \
        ((struct type *)(((unsigned char *)(head)) + (head)->scqh_first))

#define SH_CIRCLEQ_FIRST(head, type)                                    \
        ((head)->scqh_first == -1 ?                                     \
        (void *)head : SH_CIRCLEQ_FIRSTP(head, type))

#define SH_CIRCLEQ_LASTP(head, type)                                    \
        ((struct type *)(((unsigned char *)(head)) + (head)->scqh_last))

#define SH_CIRCLEQ_LAST(head, type)                                     \
        ((head)->scqh_last == -1 ? (void *)head : SH_CIRCLEQ_LASTP(head, type))

#define SH_CIRCLEQ_NEXTP(elm, field, type)                              \
        ((struct type *)(((unsigned char *)(elm)) + (elm)->field.scqe_next))

#define SH_CIRCLEQ_NEXT(head, elm, field, type)                         \
        ((elm)->field.scqe_next == SH_PTR_TO_OFF(elm, head) ?           \
            (void *)head : SH_CIRCLEQ_NEXTP(elm, field, type))

#define SH_CIRCLEQ_PREVP(elm, field, type)                              \
        ((struct type *)(((unsigned char *)(elm)) + (elm)->field.scqe_prev))

#define SH_CIRCLEQ_PREV(head, elm, field, type)                         \
        ((elm)->field.scqe_prev == SH_PTR_TO_OFF(elm, head) ?           \
            (void *)head : SH_CIRCLEQ_PREVP(elm, field, type))

#define SH_CIRCLEQ_END(head)            ((void *)(head))

#define SH_CIRCLEQ_INIT(head) {                                         \
        (head)->scqh_first = 0;                                         \
        (head)->scqh_last = 0;                                          \
}

#define SH_CIRCLEQ_INSERT_AFTER(head, listelm, elm, field, type) do {   \
        (elm)->field.scqe_prev = SH_PTR_TO_OFF(elm, listelm);           \
        (elm)->field.scqe_next = (listelm)->field.scqe_next +           \
            (elm)->field.scqe_prev;                                     \
        if (SH_CIRCLEQ_NEXTP(listelm, field, type) == (void *)head)     \
                (head)->scqh_last = SH_PTR_TO_OFF(head, elm);           \
        else                                                            \
                SH_CIRCLEQ_NEXTP(listelm,                               \
                    field, type)->field.scqe_prev =                     \
                    SH_PTR_TO_OFF(SH_CIRCLEQ_NEXTP(listelm,             \
                    field, type), elm);                                 \
        (listelm)->field.scqe_next = -(elm)->field.scqe_prev;           \
} while (0)

#define SH_CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field, type) do {  \
        (elm)->field.scqe_next = SH_PTR_TO_OFF(elm, listelm);           \
        (elm)->field.scqe_prev = (elm)->field.scqe_next -               \
                SH_CIRCLEQ_PREVP(listelm, field, type)->field.scqe_next;\
        if (SH_CIRCLEQ_PREVP(listelm, field, type) == (void *)(head))   \
                (head)->scqh_first = SH_PTR_TO_OFF(head, elm);          \
        else                                                            \
                SH_CIRCLEQ_PREVP(listelm,                               \
                    field, type)->field.scqe_next =                     \
                    SH_PTR_TO_OFF(SH_CIRCLEQ_PREVP(listelm,             \
                    field, type), elm);                                 \
        (listelm)->field.scqe_prev = -(elm)->field.scqe_next;           \
} while (0)

#define SH_CIRCLEQ_INSERT_HEAD(head, elm, field, type) do {             \
        (elm)->field.scqe_prev = SH_PTR_TO_OFF(elm, head);              \
        (elm)->field.scqe_next = (head)->scqh_first +                   \
                (elm)->field.scqe_prev;                                 \
        if ((head)->scqh_last == 0)                                     \
                (head)->scqh_last = -(elm)->field.scqe_prev;            \
        else                                                            \
                SH_CIRCLEQ_FIRSTP(head, type)->field.scqe_prev =        \
                    SH_PTR_TO_OFF(SH_CIRCLEQ_FIRSTP(head, type), elm);  \
        (head)->scqh_first = -(elm)->field.scqe_prev;                   \
} while (0)

#define SH_CIRCLEQ_INSERT_TAIL(head, elm, field, type) do {             \
        (elm)->field.scqe_next = SH_PTR_TO_OFF(elm, head);              \
        (elm)->field.scqe_prev = (head)->scqh_last +                    \
            (elm)->field.scqe_next;                                     \
        if ((head)->scqh_first == 0)                                    \
                (head)->scqh_first = -(elm)->field.scqe_next;           \
        else                                                            \
                SH_CIRCLEQ_LASTP(head, type)->field.scqe_next =         \
                    SH_PTR_TO_OFF(SH_CIRCLEQ_LASTP(head, type), elm);   \
        (head)->scqh_last = -(elm)->field.scqe_next;                    \
} while (0)

#define SH_CIRCLEQ_REMOVE(head, elm, field, type) do {                  \
        if (SH_CIRCLEQ_NEXTP(elm, field, type) == (void *)(head))       \
                (head)->scqh_last += (elm)->field.scqe_prev;            \
        else                                                            \
                SH_CIRCLEQ_NEXTP(elm, field, type)->field.scqe_prev +=  \
                    (elm)->field.scqe_prev;                             \
        if (SH_CIRCLEQ_PREVP(elm, field, type) == (void *)(head))       \
                (head)->scqh_first += (elm)->field.scqe_next;           \
        else                                                            \
                SH_CIRCLEQ_PREVP(elm, field, type)->field.scqe_next +=  \
                    (elm)->field.scqe_next;                             \
} while (0)

#endif /*GR_CIRCLEQUEUE_H_INCLUDED*/
