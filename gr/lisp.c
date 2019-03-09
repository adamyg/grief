#include <edidentifier.h>
__CIDENT_RCSID(gr_lisp_c,"$Id: lisp.c,v 1.42 2019/01/26 22:27:08 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: lisp.c,v 1.42 2019/01/26 22:27:08 cvsuser Exp $
 * List primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_... */
#include "builtin.h"
#include "debug.h"                              /* trace */
#include "echo.h"
#include "eval.h"
#include "keywd.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "math.h"                               /* do_cmp_op */
#include "symbol.h"
#include "word.h"

/*
 *  Internal LIST header/
 *      all dynamically created list objects are prefixed with this header.
 */
struct listhead {
    MAGIC_t             lh_magic1;              /* structure magic */
#define LIST_MAGIC          MKMAGIC('L', 'i', 'S', 't')
#define LIST_MAGIC2         MKMAGIC('L', 'i', 's', 'T')

    int                 lh_length;              /* current length */
    int                 lh_alloced;             /* alloced length */
    int                 lh_atoms;               /* number of atoms (top level) */
    const LIST *        lh_cursor;              /* list_each() cursor */
 // const LIST *        lh_last;                /* pop() cursor -- TODO */
    int                 lh_index;               /* and the associated element index */
 // int                 lh_editseq;             /* edit sequence -- TODO */
 // int                 lh_cursorseq;           /* sequence tied to cursor reference -- TODO */
 // int                 lh_lastseq;             /* sequence tied to last reference -- TODO */
    MAGIC_t             lh_magic2;              /* structure magic */
};

static LIST *           clonelist(const LIST *list, int len);
static int              lstsizeof(const LIST *lp);
static void             rlst_delete(ref_t *rp);


/*  Function:           atom_size
 *      Retrieve the size of the head atom.
 *
 *   Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      Sizeof the current atom in bytes.
 */
int
atom_size(const LIST *lp)
{
    register const LIST atom = *lp;

    if (F_LIST == atom) {
        return LGET_LEN(lp);
    }
    return sizeof_atoms[atom];
}


/*  Function:           atom_push_sym
 *      Push the specified symbol name 'svalue' into the list 'lp',
 *      returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      sym - Symbol value.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_sym(LIST *lp, const char *sym)
{
    *lp = F_STR;                                /* execute() requirement */
    LPUT_PTR(lp, sym);
    return lp + sizeof_atoms[F_STR];
}


/*  Function:           atom_push_str
 *      Push the specified string 'svalue' into the list 'lp', returning the address
 *      of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      svalue - String value.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_str(LIST *lp, const char *svalue)
{
    *lp = F_RSTR;
    LPUT_PTR(lp, r_string(svalue));
    return lp + sizeof_atoms[F_RSTR];
}


/*  Function:           atom_push_nstr
 *      Push the specified string 'svalue' of length 'slength' into the list 'lp',
 *      returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      svalue - String value.
 *      slength - Length of string buffer, excluding NUL terminator.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_nstr(LIST *lp, const char *svalue, int slength)
{
    assert(slength >= 0);
    *lp = F_RSTR;
    LPUT_PTR(lp, r_nstring(svalue, slength));
    return lp + sizeof_atoms[F_RSTR];
}


/*  Function:           atom_push_const
 *      Push the specified const string 'svalue' into the list 'lp',
 *      returning the address of next atom location.
 *
 *      Constant strings are assumed to be const during the life of the list as
 *      such are not referenced and are used directly.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      svalue - String value.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_const(LIST *lp, const char *svalue)
{
    *lp = F_LIT;                                /* non-copied on reference */
    LPUT_PTR(lp, svalue);
    return lp + sizeof_atoms[F_LIT];
}


/*  Function:           atom_push_int
 *      Push the specified integer 'ivalue' into the list 'lp',
 *      returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      ivalue - Integer value.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_int(LIST *lp, accint_t ivalue)
{
    *lp = F_INT;
    LPUT_INT(lp, ivalue);
    return lp + sizeof_atoms[F_INT];
}


/*  Function:           atom_push_float
 *      Push the specified float 'fvalue' into the list 'lp',
 *      returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      fvalue - Float value.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_float(LIST *lp, accfloat_t fvalue)
{
    *lp = F_FLOAT;
    LPUT_FLOAT(lp, fvalue);
    return lp + sizeof_atoms[F_FLOAT];
}


/*  Function:           atom_push_ref
 *      Push the reference into list 'lp', returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *      ref - Reference object.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_ref(LIST *lp, ref_t *rp)
{
    switch(r_type(rp)) {
    case F_STR:
        *lp = F_RSTR;
        LPUT_PTR(lp, r_inc(rp));
        return lp + sizeof_atoms[F_RSTR];
    case F_LIST:
        *lp = F_RLIST;
        LPUT_PTR(lp, r_inc(rp));
        return lp + sizeof_atoms[F_RLIST];
    default:
        panic("atom_push_ref: what? (%d).", (int)r_type(rp));
        break;
    }
    return lp;
}


/*  Function:           atom_push_null
 *      Push the a NULL atom into the list 'lp', returning the address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_null(LIST *lp)
{
    *lp = F_NULL;
    return lp + sizeof_atoms[F_NULL];
}


/*  Function:           atom_push_halt
 *      Push the a HALT atom into the list 'lp', returning the
 *      address of next atom location.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      Address of next atom location.
 */
LIST *
atom_push_halt(LIST *lp)
{
    *lp = F_HALT;
    return lp + sizeof_atoms[F_HALT];
}


/*  Function:           atom_next
 *      Determine the address of the next atom within the specified
 *      list 'lp'.
 *
 *      If we're pointing to the F_HALT at the end of the list
 *      then stick there.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      Address of next atom location.
 *
 *  Examples:
 *      Standard method used to iterate thru a list of atoms.
 *
 *>         for (; *F_HALT != *lp; lp = atom_next(lp)) {
 *>                 .. process atom ..
 *>         }
 *
 *      alternative
 *
 *>         while ((nextlp = atom_next(lp)) != lp) {
 *>                 .. process atom ..
 *>             lp = nextlp;
 *>         }
 *
 */
const LIST *
atom_next(const LIST *lp)
{
    if (lp) {
        register LIST atom = *lp;

        assert(atom < F_MAX);
        switch (atom) {
        case F_HALT:
            break;
        case F_LIST:
            lp += LGET_LEN(lp);
            assert(F_HALT == lp[-1]);
            break;
        default:
            lp += sizeof_atoms[atom];
            break;
        }
    }
    return lp;
}


const LIST *
atom_nth(const LIST *lp, int idx)
{
    assert(idx >= 0);                           /* 0 .. (length-1) */
    if (lp && idx > 0) {
        register LIST atom;

        while (idx > 0 && F_HALT != (atom = *lp)) {
            if (F_LIST == atom) {
                lp += LGET_LEN(lp);
                assert(F_HALT == lp[-1]);
            } else {
                lp += sizeof_atoms[atom];
            }
            --idx;
        }
        if (idx) {
            lp = NULL;                          /* out-of-bounds */
        }
    }

    if (lp && F_HALT == *lp) {
        return NULL;
    }
    return lp;
}


/*  Function:           atom_number
 *      Retrieve the number of atoms in the top level element.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      Atom count.
 */
int
atom_number(const LIST *lp)
{
    register LIST atom;
    int cnt = 0;

    while (F_HALT != (atom = *lp)) {
        if (F_LIST == atom) {
            lp += LGET_LEN(lp);
            assert(F_HALT == lp[-1]);
        } else {
            lp += sizeof_atoms[atom];
        }
        ++cnt;
    }
    return cnt;
}


/*  Function:           atom_xnull
 *      Determine whether a NUL list.
 *
 *  Parameters:
 *      lp - LIST atom address, maybe NULL.
 *
 *  Returns:
 *      *true* if a NUL atom, otherwise *false*.
 */
int /*boolean*/
atom_xnull(const LIST *lp)
{
    if (lp) {
        if (F_NULL == *lp) {
            return TRUE;
        }
    }
    return FALSE;
}


/*  Function:           atom_xint
 *      Retrieve the integer value of the current atom.
 *
 *  Parameters:
 *      lp - LIST atom address, maybe NULL.
 *      val - Address of variable populated with the integer value.
 *
 *  Returns:
 *      *true* if a integer atom, otherwise *false*.
 */
int /*boolean*/
atom_xint(const LIST *lp, accint_t *val)
{
    if (lp) {
        switch (*lp) {
        case F_INT:
            if (val) {
                *val = LGET_INT(lp);
            }
            return (TRUE);
        }
    }
    return (FALSE);
}


/*  Function:           atom_xfloat
 *      Retrieve the floatr value of the current atom.
 *
 *  Parameters:
 *      lp - LIST atom address, maybe NULL.
 *      val - Address of variable populated with the float value.
 *
 *  Returns:
 *      *true* if a float atom, otherwise *false*.
 */
int /*boolean*/
atom_xfloat(const LIST *lp, accfloat_t *val)
{
    if (lp) {
        switch (*lp) {
        case F_FLOAT:
            if (val) {
                *val = LGET_FLOAT(lp);
            }
            return (TRUE);
        }
    }
    return (FALSE);
}


/*  Function:           atom_xstr
 *      Retrieve the string value of the current atom.
 *
 *  Parameters:
 *      lp - LIST atom address, maybe NULL.
 *
 *  Returns:
 *      Address of the string buffer, otherwise NULL.
 */
const char *
atom_xstr(const LIST *lp)
{
    if (lp) {
        switch (*lp) {
        case F_LIT:
        case F_STR:
            return LGET_PTR2(const char, lp);
        case F_RSTR:
            return (const char *) r_ptr(LGET_PTR2(ref_t, lp));
        case F_ID: {
                const int id = LGET_ID(lp);
                const char *name = builtin[id].b_name;

                assert(id >= 0 || (unsigned)id < builtin_count);
                return name;
            }
        }
    }
    return NULL;
}


/*  Function:           atom_xlist
 *      Retrieve the list value of the current atom.
 *
 *  Parameters:
 *      lp - LIST atom address, maybe NULL.
 *
 *  Returns:
 *      Address of the list buffer, otherwise NULL.
 */
const LIST *
atom_xlist(const LIST *lp)
{
    if (lp) {
        switch (*lp) {
        case F_RLIST:
            return (const LIST *) r_ptr(LGET_PTR2(ref_t, lp));
        case F_LIST:
            return lp + sizeof_atoms[F_LIST];
        }
    }
    return (NULL);
}


/*  Function:           atom_assign_acc
 *      Assign the first atom in a list, i.e. copy it to the accumulator.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *   Returns:
 *       0 on success, 1 if the list is NULL, otherwise -1 on error.
 */
int
atom_assign_acc(const LIST *lp)
{
    if (lst_isnull(lp)) {
        return 1;
    }

    switch (*lp) {
    case F_INT:
        acc_assign_int(LGET_INT(lp));
        break;

    case F_FLOAT:
        acc_assign_float(LGET_FLOAT(lp));
        break;

    case F_STR:
    case F_LIT:
        acc_assign_str(LGET_PTR2(const char, lp), -1);
        break;

    case F_ID: {
            const int id = LGET_ID(lp);
            const char *name = builtin[id].b_name;

            assert(id >= 0 || (unsigned)id < builtin_count);
            acc_assign_str(name, -1);
        }
        break;

    case F_RSTR: {      /* copy on reference */
            ref_t *rp = LGET_PTR2(ref_t, lp);
            acc_assign_str(r_ptr(rp), r_used(rp));
        }
        break;

    case F_LIST: {      /* clone */
            int len;

            len = LGET_LEN(lp);
            len -= sizeof_atoms[F_LIST];
            lp  += sizeof_atoms[F_LIST];
            assert(F_HALT == lp[len-1]);
            acc_assign_list(lp, len);
        }
        break;

    case F_RLIST:       /* reference */
        acc_assign_ref(LGET_PTR2(ref_t, lp));
        break;

    case F_NULL:
        acc_assign_null();
        return 1;

    case F_HALT:
    default:
        acc_assign_null();
        return -1;
    }
    return 0;
}


/*  Function:           atom_assign_sym
 *      Assign the first atom in a list to the specified symbol 'sp'.
 *
 *  Parameters:
 *      lp - LIST atom address
 *
 *   Returns:
 *       0 on success, 1 if the list is NULL, otherwise -1 on error.
 */
int
atom_assign_sym(const LIST *lp, SYMBOL *sp)
{
    if (lst_isnull(lp)) {
        return 1;
    }

    switch (*lp) {
    case F_INT:
        sym_assign_int(sp, LGET_INT(lp));
        break;

    case F_FLOAT:
        sym_assign_float(sp, LGET_FLOAT(lp));
        break;

    case F_STR:         /* copy */
    case F_LIT:
        sym_assign_str(sp, LGET_PTR2(const char, lp));
        break;

    case F_ID: {
            const int id = LGET_ID(lp);
            const char *name = builtin[id].b_name;

            assert(id >= 0 || (unsigned)id < builtin_count);
            sym_assign_str(sp, name);
        }
        break;

    case F_LIST:        /* clone */
        sym_donate_ref(sp, rlst_build(atom_xlist(lp), -1));
        break;

    case F_RSTR:         /* reference */
        sym_assign_ref(sp, LGET_PTR2(ref_t, lp));
        break;

    case F_RLIST:       /* reference */
        sym_assign_ref(sp, LGET_PTR2(ref_t, lp));
        break;

    case F_NULL:
        return 1;

    case F_HALT:
    default:
        return -1;
    }
    return 0;
}


/*  Function:           atom_copy
 *      Copy an atom from one list to another list.
 *
 *      o Sublists are copied over as a single atom.
 *      o Reference counters are incremented where necessary.
 *
 *  Parameters:
 *      dstlp - Destination atom list address.
 *      srclp - Source atom list address.
 *
 *  Returns:
 *      Count in bytes of atom copied.
 */
static int
atom_copy(LIST *dstlp, const LIST *srclp)
{
    register LIST atom = *srclp;

    *dstlp = atom;
    switch (atom) {
    case F_INT:
        LPUT_INT(dstlp, LGET_INT(srclp));
        break;

    case F_FLOAT:
        LPUT_FLOAT(dstlp, LGET_FLOAT(srclp));
        break;

    case F_LIT:         /* non-copy */
        LPUT_PTR(dstlp, LGET_PTR(srclp));
        break;

    case F_RLIST:       /* reference */
    case F_RSTR:
        LPUT_PTR(dstlp, r_inc(LGET_PTR2(ref_t, srclp)));
        break;

    case F_LIST: {      /* clone */
            LIST *lp = dstlp;
            int n = atom_size(srclp);
            const LIST *end_lp = srclp + n - 1;

            assert(n == LGET_LEN(srclp));
            assert(F_HALT == *end_lp);
            assert(n <= LIST_MAXLEN);

            LPUT_LEN(dstlp, (int16_t)n);
            dstlp += sizeof_atoms[F_LIST];
            srclp += sizeof_atoms[F_LIST];
            while (srclp < end_lp) {
                n = atom_copy(dstlp, srclp);
                dstlp += n;
                srclp += n;
            }
            dstlp = atom_push_halt(dstlp);

            assert(F_HALT == *srclp);
            return dstlp - lp;
        }

    case F_STR:
        LPUT_PTR(dstlp, LGET_PTR2(const char, srclp));
        panic("atom_copy: STR.");               /* compiler bug! */
        break;

    case F_ID: {
            const int id = LGET_ID(srclp);

            assert(id >= 0 || (unsigned)id < builtin_count);
            LPUT_ID(dstlp, (int16_t)id);
        }
        break;

    case F_NULL:
        break;

    case F_HALT:
        panic("atom_copy: HALT.");
        break;

    default:
        panic("atom_copy: Unknown type (%d).", atom);
        break;
    }
    return sizeof_atoms[atom];
}


/*  Function:           atom_copy_seq
 *      copy a list to another list, but do not include the terminating F_HALT.
 *
 *  Parameters:
 *      dstlp - Destination atom list address.
 *      atoms - Address of counter incremented with total atom count.
 *      srclp - Source atom list address.
 *
 *  Returns:
 *      Count in bytes of atom copied.
 */
static int
atom_copy_seq(LIST *dstlp, int *atoms, const LIST *srclp)
{
    LIST *start = dstlp;
    int n, cnt = 0;

    while (F_HALT != *srclp) {
        n = atom_copy(dstlp, srclp);
        dstlp += n;
        srclp += n;
        ++cnt;
    }
    *atoms += cnt;
    return dstlp - start;
}


/*  Function:           argv_size
 *      Size the argument from the argv array.
 *
 *  Parameters:
 *      lvp - Argument vector element.
 *
 *  Returns:
 *      Size of the argument in bytes.
 */
int
argv_size(const LISTV *lvp)
{
    register const LIST atom = lvp->l_flags;

    switch (atom) {
    case F_INT:
    case F_FLOAT:
    case F_LIT:
    case F_STR:
    case F_RSTR:
    case F_NULL:
    case F_RLIST:
        break;

    case F_LIST: {
            const LIST *lp = lvp->l_list;
            const int len = atom_size(lp);

            assert(len > 0);
            assert(F_HALT == lp[len-1]);
            return len;
        }

    case F_HALT:
        panic("argv_size: HALT.");
        break;

    default:
        panic("argv_size: Unknown type (%d).", atom);
        break;
    }
    return sizeof_atoms[atom];
}


/*  Function:           argv_copy
 *      Copies an argument from the argv array 'lvp' to the destination list 'lp'.
 *
 *      Sublists are copied over as a single atom. Reference counters are incremented
 *      where necessary.
 *
 *  Parameters:
 *      lp - Destination list.
 *      lvp - Argument vector element.
 *
 *  Returns:
 *      int - size of list element in bytes.
 */
int
argv_copy(LIST *lp, const LISTV *lvp)
{
    register LIST atom = lvp->l_flags;

    *lp = atom;
    switch (atom) {
    case F_INT:
        LPUT_INT(lp, lvp->l_int);
        break;

    case F_FLOAT:
        LPUT_FLOAT(lp, lvp->l_float);
        break;

    case F_LIT:         /* non-copy */
        LPUT_PTR(lp, lvp->l_str);
        break;

    case F_STR:         /* copy on reference */
        *lp = atom = F_RSTR;
        LPUT_PTR(lp, r_string(lvp->l_str));
        break;

    case F_LIST: {      /* clone */
            LIST *dstlp = lp;
            const LIST *srclp = lvp->l_list;
            int len = atom_size(srclp);
            const LIST *end_lp = srclp + len - 1;

            assert(len > 0);
            assert(len <= LIST_MAXLEN);
            assert(F_HALT == srclp[len-1]);
            assert(F_HALT == *end_lp);

            LPUT_LEN(dstlp, (int16_t)len);
            dstlp += sizeof_atoms[F_LIST];
            srclp += sizeof_atoms[F_LIST];
            while (srclp < end_lp) {
                int n = atom_copy(dstlp, srclp);
                dstlp += n;
                srclp += n;
            }

            assert(F_HALT == *srclp);
            assert(srclp == end_lp);
            dstlp = atom_push_halt(dstlp);
            assert(len == (dstlp - lp));
            return dstlp - lp;
        }

    case F_RSTR:        /* reference */
    case F_RLIST:
        LPUT_PTR(lp, r_inc(lvp->l_ref));
        break;

    case F_NULL:
        break;

    case F_HALT:
        panic("argv_copy: HALT.");
        break;

    default:
        panic("argv_copy: Unknown type (%d).", atom);
        break;
    }
    return sizeof_atoms[atom];
}


/*  Function:           argv_make
 *      Make an argument from the argv array 'lvp' to the destination list 'lp'.
 *
 *      Unlike <argv_copy> the make operation does not increment reference objects nor
 *      built references to string or literals. As such the interface is intended only
 *      to build temporary argument lists nor should the argument list be destroyed.
 *
 *  Parameters:
 *      lvp - Argument vector element.
 *      lp - Source list.
 *
 *  Returns:
 *      int - size of list element in bytes.
 */
int
argv_make(LISTV *lvp, const LIST *lp)
{
    register const LIST atom = *lp;

    lvp->l_flags = atom;
    switch (atom) {
    case F_INT:
        lvp->l_int = LGET_INT(lp);
        break;

    case F_FLOAT:
        lvp->l_float = LGET_FLOAT(lp);
        break;

    case F_LIT:
        lvp->l_str = LGET_PTR2(const char, lp);
        break;

    case F_STR:
        lvp->l_str = LGET_PTR2(const char, lp);
        break;

    case F_LIST:
        lvp->l_list = lp;
        return sizeof_atoms[F_LIST] + lstsizeof(lp);

    case F_RSTR:
    case F_RLIST:
        lvp->l_ref = LGET_PTR(lp);
        break;

    case F_NULL:
        break;

    case F_HALT:
        panic("argv_make: HALT.");

    default:
        panic("argv_make: Unknown type (%d).", atom);
    }
    return sizeof_atoms[atom];
}


/*  Function:           argv_list
 *      Create a LIST from an argv array.
 *
 *  Parameters:
 *      lvp - Argument vector element.
 *      atoms - element count.
 *      len - refaddr of field populated with the total length.
 *
 *  Returns:
 *      LIST * address of new list, with *len being the size of the list in bytes.
 */
LIST *
argv_list(const LISTV *lvp, int atoms, int *len)
{
    LIST *newlp;
    register int i;
    register LIST *lp;
    int new_len = 0;

    ED_TRACE(("argv_list(%p, %d)\n", lvp, atoms))

    /* empty /null ist */
    if (atoms <= 0)
        return NULL;

    /* first work out how long the list is going to be */
    for (i = 0; i < atoms; ++i) {
        new_len += argv_size(lvp + i);
    }
    new_len += sizeof_atoms[F_HALT];
    if (len) {
        *len = new_len;
    }

    /* allocate memory for new list */
    if (NULL == (newlp = lst_alloc(new_len, atoms))) {
        return NULL;
    }

    /* copy */
    lp = newlp;
    for (i = 0; i < atoms; ++i) {
        lp += argv_copy(lp, lvp++);
    }
    atom_push_halt(lp);

    ED_TRACE(("argv_list(%d) = %p\n", new_len, newlp))
    return newlp;
}


/*  Function:           alloc_size
 *      Determine the size of the allocated arena needed to maintain a list
 *      containing the specified atom tsorage.
 *
 *      The resulting storage shall include list header overheads and shall
 *      be rounded to either a ^2 size or 1MB alignment, which ever comes first
 *
 *  Parameters:
 *      len - Required minimum length, in bytes, of atom storage.
 *
 *  Returns:
 *      Length rounded.
 */
static int
alloc_size(int len)
{
    int alloc_len = 1;

    len += sizeof(struct listhead);             /* overhead */
    if (len > (1 << 20)) {
        alloc_len = len | ((1 << 20)-1);        /* 1Mb rounding */

    } else {
        do {
            alloc_len <<= 1;                    /* round the next x^2 base */
        } while (len > alloc_len);
    }
    return alloc_len;
}


/*  Function:           lst_alloc
 *      Create a list with at least 'len' bytes in storage.
 *
 *  Parameters:
 *      len - Required minimum length. in bytes.
 *      atoms - Number of atoms.
 *
 *  Returns:
 *      Address of the allocation list.
 */
LIST *
lst_alloc(int len, int atoms)
{
    struct listhead *lh;
    int alloc_len;

    assert(atoms <= len);
    assert(len > 0);
    assert(len <= LIST_MAXLEN);
    alloc_len = alloc_size(len);
    if (NULL == (lh = (struct listhead *)chk_calloc((size_t)alloc_len, 1))) {
        return NULL;
    }
    lh->lh_magic1  = LIST_MAGIC;
    lh->lh_alloced = alloc_len - sizeof(struct listhead);
    lh->lh_length  = len;
    lh->lh_atoms   = atoms;
    lh->lh_cursor  = NULL;                      /* list_each() */
 /* lh->lh_tail    = NULL; - TODO */
    lh->lh_index   = 0;
    lh->lh_magic2  = LIST_MAGIC2;
    return (LIST *)(lh + 1);
}


/*  Function:           lstsizeof
 *      Return the raw length of a list in allocated bytes.  The origin of the list
 *      is not assumed, as such the length is derived by walking the list atoms.
 *
 *  Parameters:
 *      list - List atom address, maybe NULL.
 *
 *  Returns:
 *      Length of the list, in bytes.
 */
int
lstsizeof(const LIST *list)
{
    register const LIST *lp;
    register LIST atom;

    if (NULL == (lp = list)) {
        return 0;
    }

    while (F_HALT != (atom = *lp)) {
        if (F_LIST == atom) {
            lp += LGET_LEN(lp);
            assert(F_HALT == lp[-1]);           /* verify list length */
        } else {
            lp += sizeof_atoms[atom];
        }
    }
    return (lp - list) + sizeof_atoms[F_HALT];  /* includes terminator */
}


/*  Function:           lst_isnull
 *      Determine whether the specifiied list is blank/null, if so assign a null result
 *      to the accumulator.
 *
 *  Parameters:
 *      lp - LIST atom address.
 *
 *  Returns:
 *      *TRUE* if the stated list was null, otherwise *FALSE*.
 */
int
lst_isnull(const LIST *lp)
{
    if (lp && F_HALT != *lp) {
        return FALSE;
    }
    acc_assign_null();
    return TRUE;
}


/*  Function:           lst_atoms_set
 *      Set the LIST atom count.
 *
 *  Parameters:
 *      lp - List base address.
 *      atoms - Absoluate atom count.
 *
 *  Returns:
 *      nothing.
 */
static void
lst_atoms_set(LIST *lp, int atoms)
{
    struct listhead *lh;

    lh = ((struct listhead *)lp)-1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);
    lh->lh_atoms = atoms;
    assert(lh->lh_atoms == (atoms = atom_number(lp)));
}


/*  Function:           lst_atoms_get
 *      Retrieve the LIST atom count.
 *
 *  Parameters:
 *      lp - List base address.
 *
 *  Returns:
 *      Atom count.
 */
int
lst_atoms_get(const LIST *lp)
{
    const struct listhead *lh;
    int atoms;

    lh = ((const struct listhead *)lp)-1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);
    assert(lh->lh_atoms  == (atoms = atom_number(lp)));
    assert(lh->lh_atoms  <= LIST_MAXLEN);
    return lh->lh_atoms;
}


/*  Function:           lst_atoms_inc
 *      Increment the LIST atom count.
 *
 *  Parameters:
 *      lp - List base address.
 *      atominc - Atom increments.
 *
 *  Returns:
 *      nothing.
 */
#if (NOT_USED)
static void
lst_atoms_inc(LIST *lp, int atominc)
{
    struct listhead *lh;

    lh = ((struct listhead *)lp)-1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);
    lh->lh_atoms += atominc;
    assert(lh->lh_atoms == (atominc = atom_number(lp)));
}
#endif


/*  Function:           lst_size
 *      Set the size of list length, limited to its existing storage.
 *
 *  Parameters:
 *      lp - List base address.
 *      newlen - New length of the list.
 *      newatoms - New atom count.
 *
 *  Returns:
 *      Address of the list.
 */
LIST *
lst_size(LIST *lp, int newlen, int newatoms)
{
    struct listhead *lh;
    int curlen;

    assert(lp != NULL);

    lh = ((struct listhead *)lp) - 1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);

    assert(newlen > 0);
    assert(newlen <= LIST_MAXLEN);
    assert(newlen <= lh->lh_alloced);
    assert(newatoms >= 0);
    lh->lh_length = newlen;
    lh->lh_atoms  = newatoms;
    assert((curlen = lstsizeof(lp)) == newlen);
    assert(lh->lh_atoms == (newatoms = atom_number(lp)));
    return (LIST *)lp;
}


/*  Function:           lst_expand
 *      Expand list the storage, if possible, without relocating the image
 *
 *  Parameters:
 *      lp - List base address.
 *      leninc - Length in bytes to expand storage.
 *
 *  Returns:
 *      Address of the expanded list, otherwise NULL in the event the current
 *      allocation could not be expanded.
 *
 *  Notes:
 *      Implementation replies on chk_expand() being supported for the host memory
 *      allocation services.
 */
LIST *
lst_expand(LIST *lp, int leninc)
{
    struct listhead *lh;
    int newlen;

    assert(lp != NULL);

    lh = ((struct listhead *)lp) - 1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);

    assert(leninc >= 0);
    if (leninc > 0) {
        if ((newlen = lh->lh_length + leninc) > lh->lh_alloced) {
            int min_len = sizeof(struct listhead) + newlen;
            int alloc_len = alloc_size(newlen);

            /* try expanding the block */
            if ((int) chk_expand(lh, (size_t)alloc_len) >= min_len) {
                lh->lh_alloced = alloc_len - sizeof(struct listhead);

            } else {
                return NULL;
            }
        }
        assert(newlen > 0);
        assert(newlen <= LIST_MAXLEN);
        lh->lh_length = newlen;
    }
    return (LIST *)lp;
}


/*  Function:           lst_extend
 *      Extend list the storage, possiblity relocating the image
 *
 *  Parameters:
 *      lp - List base address.
 *      leninc - Length in bytes to extend storage.
 *      atominc - Atom increment.
 *
 *  Returns:
 *      Address of the expanded list, otherwise NULL in the event the current
 *      allocation could not be expanded.
 */
LIST *
lst_extend(LIST *lp, int leninc, int atominc)
{
    struct listhead *lh = ((struct listhead *)lp) - 1;
    LIST *nlp;

    assert(leninc > 0);
    assert(atominc <= leninc);                  /* smallest atom is a single byte */

    if (NULL != (nlp = lst_expand(lp, leninc))) {
        lh->lh_atoms += atominc;                /* update header */
        assert(nlp == lp);

    } else {
        const int newlen = lh->lh_length + leninc;
        const int alloc_len = alloc_size(newlen);

        if (NULL == (lh = chk_realloc(lh, (size_t)alloc_len))) {
            nlp = NULL;                         /* failure */

        } else {                                /* update header */
            lh->lh_alloced = alloc_len - sizeof(struct listhead);
            assert(newlen > 0);
            assert(newlen <= LIST_MAXLEN);
            lh->lh_length = newlen;
            lh->lh_atoms += atominc;
            nlp = (LIST *)(lh + 1);
        }
    }
    return nlp;
}


/*  Function:           lst_check
 *      Verify a list construction, returning its length in bytes.
 *
 *  Parameters:
 *      lp - List base address.
 *
 *  Returns:
 *      Length of the list in bytes.
 */
int
lst_check(const LIST *lp)
{
    const struct listhead *lh;
    int length, atoms;

    lh = ((const struct listhead *)lp)-1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    assert(lh->lh_length <= lh->lh_alloced);
    assert(lh->lh_magic2 == LIST_MAGIC2);
    assert((length = lstsizeof(lp)) > 0);
    assert(lh->lh_length > 0);
    assert(lh->lh_length <= LIST_MAXLEN);
    assert(F_HALT == lp[lh->lh_length-1]);
    assert(lh->lh_length == length);
    assert((atoms = atom_number(lp)) >= 0);
    assert(lh->lh_atoms == atoms);
    return lh->lh_length;
}


/*  Function:           lst_length
 *      Retrieve the length of the specified list 'lp', optimised lst_check().
 *
 *  Parameters:
 *      lp - List base address.
 *
 *  Returns:
 *      Length of the list in bytes.
 */
int
lst_length(const LIST *lp)
{
    const struct listhead *lh;

    lh = ((const struct listhead *)lp)-1;
    assert(lh->lh_magic1 == LIST_MAGIC);
    return lh->lh_length;
}


/*  Function:           lst_free
 *      Free a list and any strings which are pointed to by the elements in a list
 *
 *  Parameters:
 *      lp - List base address.
 *
 *  Returns:
 *      nothing.
 */
void
lst_free(LIST *list)
{
    register LIST *lp;
    int list_len, depth = 0;

    list_len = lst_check(list);
    for (lp = list;;) {
        const LIST atom = *lp;

        switch (atom) {
        case F_RLIST:
        case F_RSTR:
            r_dec(LGET_PTR2(ref_t, lp));
            break;

        case F_LIST:
            ++depth;
            break;

        case F_HALT:
            if (0 == depth) {
                assert(lp == list + (list_len-1));
                chk_free(((struct listhead *)list)-1);
                return;
            }
            --depth;
            break;
        }
        lp += sizeof_atoms[atom];
        assert(lp < list+list_len);
    }
    assert(NULL);
    /*NOTREACHED*/
}


/*  Function:           clonelist
 *      Creates a clone of a list including duplicating all the internal
 *      pointers and reference counts.
 *
 *      Reference counters are incremented where necessary, with all other
 *      atoms including lists imaged as within the original.
 *
 *  Parameters:
 *      list - List base address.
 *      len - Length of the list, in bytes.
 *
 *  Return:
 *      List address, otherwise NULL in the event of an empty list.
 */
static LIST *
clonelist(const LIST *list, int len)
{
    const LIST *lp;
    LIST *newlp;
    int atoms, depth;

    assert(len >= 0);

    if (0 == len || NULL == (newlp = lst_alloc(len, -1))) {
        return NULL;
    }

    (void) memcpy(newlp, list, (size_t)len);    /* raw copy */

    for (atoms = 0, depth = 0, lp = newlp;;) {  /* fixup */
        const LIST atom = *lp;

        switch (atom) {
        case F_RLIST:
        case F_RSTR:
            r_inc(LGET_PTR2(ref_t, lp));
            break;

        case F_STR:
            /*
             *  note: should *only* occur when sourced from a quote_list()
             *          command, atoms must be retained as F_STR.
             */
            break;

        case F_LIST:
            ++depth;
            break;

        case F_HALT:
            if (0 == depth) {                   /* done */
                lst_atoms_set(newlp, atoms);
                assert(lp == newlp + (len-1));
                lst_check(newlp);
                return newlp;
            }
            --depth;
            break;
        }

        lp += sizeof_atoms[atom];
        if (0 == depth) {
            ++atoms;
        }
        assert(lp < newlp+len);
    }
    /*NOTREACHED*/
}


/*  Function:           lst_raw_clone
 *      Create a copy if the specified list, assuming nothing about the list source.
 *
 *  Parameters;
 *      list - List base address.
 *      len - Length of the list, in bytes.
 *
 *  Return:
 *      List address, otherwise NULL in the event of an empty list.
 */
LIST *
lst_build(const LIST *list, int llen)
{
    assert(lstsizeof(list) == llen);
    return clonelist(list, llen);
}


/*  Function:           lst_clone
 *      Create a copy if the specified list, assuming the list is dynamic.
 *
 *  Parameters;
 *      list - List base address.
 *      lenp - ariable populated with the length of the
 *             returning list, bytes.
 *
 *  Return:
 *      List address, otherwise NULL in the event of an empty list.
 */
LIST *
lst_clone(const LIST *list, int *llenp)
{
    int llen;

    llen = lst_check(list);
    if (llenp) {
        *llenp = llen;
    }
    return clonelist(list, llen);
}


/*  Function:           lvp_size
 *      Determine the size of head atom of lvp object,
 *      stripping off any outer LIST coating.
 *
 *  Parameters;
 *      lvp - LISTV object address.
 *
 *  Returns:
 *      Sizeof the current atom in bytes.
 */
static int
lvp_size(const LISTV *lvp)
{
    const OPCODE l_flags = lvp->l_flags;

    switch (l_flags) {
    case F_LIST:
        return lstsizeof(lvp->l_list) - sizeof_atoms[F_HALT];

    case F_RLIST:
        assert(r_used(lvp->l_ref) == lst_check(r_ptr(lvp->l_ref)));
        return r_used(lvp->l_ref) - sizeof_atoms[F_HALT];

    default:
        break;
    }
    return sizeof_atoms[l_flags];
}


/*  Function:           lvp_copy
 *      Copy a lvp object, stripping off any outer LIST coating.
 *
 *  Parameters;
 *      dstlp - Destination list address.
 *      atoms - Address osr vairable populated with copied atom count.
 *      lvp - LISTV object address.
 *
 *  Returns:
 *      Sizeof the current atom in bytes.
 */
static int
lvp_copy(LIST *dstlp, int *atoms, const LISTV *lvp)
{
    switch (lvp->l_flags) {
    case F_LIST:
        return atom_copy_seq(dstlp, atoms, lvp->l_list);

    case F_RLIST:
        assert(r_used(lvp->l_ref) == lst_check(r_ptr(lvp->l_ref)));
        return atom_copy_seq(dstlp, atoms, (LIST *) r_ptr(lvp->l_ref));

    default:
        break;
    }
    (*atoms)++;
    return argv_copy(dstlp, lvp);
}


/*  Function:           lvp_join
 *      Join two lists together -- returning a new list and length of new list.
 *
 *  Parameters;
 *      lp1 - List base address of the first list.
 *      len - Length of the list in bytes.
 *      lvp - List vector.
 *      lenp - Address of variable populated with the length of the
 *             resulting list.
 *
 *  Returns:
 *      Address of the new list.
 */
LIST *
lst_join(const LIST *lp1, int llen, const LISTV *lvp, int *lenp)
{
    const LIST *srclp;
    LIST *dstlp, *newlp;
    int newllen, atoms, n;

    lst_check(lp1);

    /* Work out how much space is needed for new list */
    if (llen < 0) {
        llen = lstsizeof(lp1);
    }
    newllen = llen + lvp_size(lvp);

    if (NULL == (newlp = lst_alloc(newllen, -1))) {
        *lenp = 0;
        return NULL;
    }

    /* First take a copy of the old list */
    atoms = 0;
    dstlp = newlp;
    if (lp1) {
        srclp = lp1;
        while (F_HALT != *srclp) {
            n = atom_copy(dstlp, srclp);
            dstlp += n;
            srclp += n;
            ++atoms;
        }
    }

    /* Append the argument and terminate the list */
    dstlp += lvp_copy(dstlp, &atoms, lvp);
    *dstlp = F_HALT;
    lst_atoms_set(newlp, atoms);

    *lenp = newllen;
    return newlp;
}


/*  Function:           rlst_create
 *      RLIST construction, assuming ownership of the LIST storage.
 *
 */
ref_t *
rlst_create(LIST *lp, int llen)
{
    return r_list(lp, llen, rlst_delete);
}


/*  Function:           rlst_clone
 *      RLIST construction, cloning the LIST.
 *
 */
ref_t *
rlst_clone(const LIST *lp)
{
    int llen;
    LIST *newlp = lst_clone(lp, &llen);

    return r_list(newlp, llen, rlst_delete);
}


/*  Function:           rlst_build
 *      RLIST construction, copying the raw LIST.
 *
 */
ref_t *
rlst_build(const LIST *lp, int llen)
{
    if (-1 == llen) llen = lstsizeof(lp);
    return r_list(lst_build(lp, llen), llen, rlst_delete);
}


/*  Function:           rlst_delete
 *      RLIST destruction, deleting the LIST storage.
 *
 */
static void
rlst_delete(ref_t *rp)
{
    lst_check((const LIST *)r_ptr(rp));
    lst_free(r_ptr(rp));
}


/*  Function:           rlst_splice
 *
 *      RLIST remove and/or add elements from/to a list work horse, padding lists
 *      if required.
 *
 *      Removes the elements designated by 'offset' and 'datoms' from the list 'rp'
 *      and replaces them with the elements of 'lvp' of the container set of 'natoms'.
 *
 *      Negative 'offsets' refer from the end of the list, hence -1 = EOL.
 *
 *      If 'datoms' is -1, removes everything from 'offset' onward.
 *
 *      Flat specfies the 'flatten mode' of the source elements which are to added.
 *
 *      The list grows or shrinks as necessary, padding with NULLS if required.
 *
 *  Flatten mode;
 *      Note list handling flattens list at level 0. Hence when used in the following
 *      context the result is;
 *
 *          list lst = make_list("a", "b");
 *
 *          lst += lst;
 *          // result = "a", "b", "a", "b"
 *
 *      The alternative is the splice function
 *
 *          splice(lst, length_of_list(lst), 0, lst);
 *
 *      which shall place a list reference within the list.
 *
 *  Parameters:
 *      rp - List reference.
 *      nth - Starting offset (atom index).
 *      datoms - Number of atoms to remove.
 *      lvp - Value list.
 *      natoms - Number of atoms to add.
 *      flat - Whether to flat the elements being inserted.
 *
 *  Returns:
 *      Returns the new/modified list.
 */
ref_t *
rlst_splice(ref_t *rp, int nth, int datoms, const LISTV *lvp, int natoms, int flat)
{
    const LIST *lp, *movlp, *endlp, *nthlp;
    struct listhead *lh;
    int llen, newllen, dellen, addlen;
    int t_atoms;

    ED_TRACE(("rlst_splice(%p, nth:%d, datoms:%d, lvp:%p, natoms:%d, flat:%d)\n", \
        rp, nth, datoms, lvp, natoms, flat))

    lp = (const LIST *) r_ptr(rp);
    llen = lst_check(lp);
    lh = ((struct listhead *)lp)-1;
    endlp = lp + llen;
    lh->lh_cursor = NULL;

    /*
     *  Find nth point and size delete and add arenas.
     */

    /* nth */
    nthlp = lp;
    if (-1 == nth) {                            /* EOL */
        nthlp = (llen ? (endlp - sizeof_atoms[F_HALT]) : lp);
        datoms = nth = 0;

    } else {
        if (nth < 0) {                          /* backreference */
            nth = lst_atoms_get(lp) + nth + 1;
            ED_TRACE(("back nth=%d\n", nth))
            if (nth < 0) {
                errorf("splice: bad offset");
                return NULL;                    /* bad offset (SOL) */
            }
        }

        while (F_HALT != *nthlp && nth > 0) {
            nthlp = (LIST *) atom_next(nthlp);
            assert(nthlp < endlp);
            --nth;
        }

        if (nth) {
            assert(nth >= 0);
            if (natoms <= 0) {                  /* can be -1 or 0 */
                return NULL;
            }
            ED_TRACE(("splice: nth > length, padding list"))
            datoms = 0;
        }
    }

    /* delete */
    dellen = 0;
    movlp = nthlp;
    if (datoms <= -1) {
        datoms = lst_atoms_get(lp) - nth;       /* delete to EOL */
    }

    if ((t_atoms = datoms) > 0) {
        while (F_HALT != *movlp && t_atoms > 0) {
            int l = atom_size(movlp);

            movlp += l;
            assert(movlp < endlp);
            dellen += l;
            --t_atoms;
        }
        datoms -= t_atoms;                      /* trim datom count */
    }

    /* additional */
    addlen = 0;
    if (NULL == lvp || natoms < 0) {
        natoms = 0;

    } else if ((t_atoms = natoms) > 0) {
        const LISTV *t_lvp = lvp;

        if (flat) {
            while (--t_atoms >= 0) {
                addlen += lvp_size(t_lvp++);
            }
        } else {
            while (--t_atoms >= 0) {
                addlen += argv_size(t_lvp++);
            }
        }
    }

    /*
     *  new length
     */
    newllen = llen + (addlen - dellen);         /* final new length */

    if (nth) {                                  /* padding */
        newllen += nth * sizeof_atoms[F_NULL];
    }

    if (0 == llen) {                            /* src == NULL, need terminator */
        newllen += sizeof_atoms[F_HALT];
    }

    ED_TRACE(("\tlp=%p, off=%p, mov=%p, end=%p del=%d/%d, pad=%d, add=%d/%d, old=%d/new=%d", \
        lp, nthlp, movlp, endlp, dellen, datoms, nth, addlen, natoms, llen, newllen))

    /*
     *  If there is single reference and no additional storage is required.
     */
    if (1 == r_refs(rp) && rp != x_halt_list && /* single reference and not special */
            (newllen <= llen || lst_expand((LIST *)lp, newllen - llen))) {
        /*
         *  modify existing
         */
        const int atoms = lst_atoms_get(lp);
        LIST *dstlp = (LIST *)lp;
        int xatoms = 0;                         /* extra atoms */

        ED_TRACE(("(resizing)\n"))

        /* remove */
        movlp = nthlp;
        if ((t_atoms = datoms) > 0) {
            while (t_atoms-- > 0) {
                switch (*movlp) {
                case F_RLIST:
                case F_RSTR:
                    r_dec(LGET_PTR2(ref_t, movlp));
                    break;
                }
                movlp = (LIST *) atom_next(movlp);
            }
            assert(nthlp + dellen == movlp);
        }

        /* relocate/padding */
        dstlp = (LIST *)nthlp;

        if (0 == nth) {
            if (addlen != dellen) {
                ED_TRACE(("\tcompressing(%p, %p, %d)\n", \
                    nthlp + addlen, movlp, (endlp - movlp) + 1))
                memmove((void *)(nthlp + addlen), (const void *)movlp, (size_t)(endlp - movlp));
            }

        } else {
            ED_TRACE(("\tpadding (%d)\n", nth))
            while (nth-- > 0) {
                *dstlp++ = F_NULL;
                ++xatoms;
            }
            dstlp[addlen] = F_HALT;             /* terminate new list */
        }
        assert(F_HALT == lp[newllen-1]);

        /* add/insert */
        if ((t_atoms = natoms) > 0) {
            if (flat) {
                while (t_atoms-- > 0) {
                    dstlp += lvp_copy(dstlp, &xatoms, lvp++);
                }
            } else {
                while (t_atoms-- > 0) {
                    dstlp += argv_copy(dstlp, lvp++);
                    ++xatoms;
                }
            }
        }

        /* resize */
        lst_size((LIST *)lp, newllen, atoms + (xatoms - datoms));
        r_incused(rp, newllen - llen);          /* resize symbol */

    } else {
        /*
         *  create new image
         */
        register const LIST *srclp;
        LIST *newlp, *dstlp;
        int tatoms = 0;                         /* total atoms */

        ED_TRACE((" (new)\n"))

        /* allocate new storage */
        if (NULL == (newlp = lst_alloc(newllen, -1))) {
            return NULL;
        }
        dstlp = newlp;

        /* prior */
        srclp = lp;
        while (srclp < nthlp) {
            const int l = atom_copy(dstlp, srclp);
            dstlp += l;
            srclp += l;
            ++tatoms;
        }

        /* padding */
        while (nth-- > 0) {
            *dstlp++ = F_NULL;
            ++tatoms;
        }

        /* add */
        if ((t_atoms = natoms) > 0) {
            if (flat) {
                while (t_atoms-- > 0) {
                    dstlp += lvp_copy(dstlp, &tatoms, lvp++);
                }
            } else {
                while (t_atoms-- > 0) {
                    dstlp += argv_copy(dstlp, lvp++);
                    ++tatoms;
                }
            }
        }

        /* post */
        srclp = movlp;
        while (F_HALT != *srclp) {
            const int l = atom_copy(dstlp, srclp);
            assert(srclp < endlp);
            dstlp += l;
            srclp += l;
            ++tatoms;
        }
        atom_push_halt(dstlp);                  /* terminate new list */

        /* size */
        lst_atoms_set(newlp, tatoms);
        assert(newllen == lst_check(newlp));
        rp = rlst_create(newlp, newllen);
    }
    return rp;
}


/*  Function:           do_make_list
 *      make_list primitive, used to encapsulate arguments in a new list.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: make_list - Build an evaluated list.

        list
        make_list(...)

    Macro Description:
        The 'make_list()' primitive builds a new list constructed
        from the specified arguments. Each of the specified arguments
        shall be evaluated and appended the end of the list. This is
        in contrast to the <quote_list> primitive which does not
        evaluate any of the arguments.

    Macro Parameters:
        ... - One or more values to be made into list elements.

    Macro Returns:
        The 'make_list()' primitive returns the built list.

    Macro Portability:
        n/a

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_extract
 */
void
do_make_list(void)              /* list (...) */
{
    LIST *list;
    int len;
                                                /* skip arg0 */
    if (NULL == (list = argv_list(margv + 1, margc - 1, &len)))  {
        acc_assign_null();
    } else {
        acc_donate_list(list, len);
    }
}


/*  Function:           do_car
 *      car primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: car - Retrieve the first list element.

        declare
        car(list lst)

    Macro Description:
        The 'car()' primitive retrieves a copy of the first element
        (also known as an atom) in the list 'lst'; effectively
        removing all elements after the first.

        'car' is non-destructive. It does not remove any elements
        from the source list only returning a copy of what is the
        first element, as such the source list is unchanged.

        Note!:
        As lists may contain any data type, it is advised to assigned
        the result to a polymorphic variable, so that its type can be
        ascertained.

    Macro Example:
        TODO

    Macro Returns:
        The 'car()' primitive returns value of the first element from
        the source.

        Like <cdr>, the 'car' primitive is built on their Lisp
        counter parts, which are also non-destructive; that is,
        neither modify or change lists to which they are applied.

    Macro Portability:
        n/a

    Macro See Also:
        car, cdr, nth, put_nth, splice, list, pop, push
 */
void
do_car(void)                    /* declare (list expr) */
{
    if (atom_assign_acc(get_list(1))) {
        errorf("car: empty list.");
    }
}


/*  Function:           do_cdr
 *      cdr primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: cdr - Retrieve all secondary list elements.

        list
        cdr(list lst)

    Macro Description:
        The 'cdr()' primitive retrieves a copy of everything but the
        first element (also known as an atom) in the list 'lst';
        effectively removing the first element.

        'cdr' is non-destructive. It does not remove any elements
        from the source list only returning a copy of what are the
        second and subsequent elements, as such the source list is
        unchanged.

        Note!:
        The primitives 'car' and 'cdr' can be utilised to manipulate
        all elements on a list. However, it is more efficient to use
        <nth>, <pop>, <splice> to extract individual elements, since
        internally it avoids having to copy sub-lists.

    Macro Example:
        TODO

    Macro Returns:
        The 'cdr()' primitive returns a list containing the second and
        subsequent elements from the source.

        Like <car>, the 'cdr' primitive is built on their Lisp
        counter parts, which also non-destructive; that is, neither
        modify or change lists to which they are applied.

    Macro Portability:
        n/a

    Macro See Also:
        car, cdr, nth, put_nth, splice, list, pop, push
 */
void
do_cdr(void)                    /* list (list expr) */
{
    const LIST *lp = get_list(1);

    if (lst_isnull(lp)) {
        acc_assign_null();
    } else {
        lp = atom_next(lp);
        acc_assign_list(lp, -1);                /* FIXME - costly */
    }
}


/*  Function:           do_quote_list
 *      quote_list primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: quote_list - Build an unevaluated list.

        list
        quote_list(...)

    Macro Description:
        The 'quote_list()' primitive builds a list of the specified
        arguments unchanged and unevaluated. This is in contrast to
        the <make_list> primitive which shall evaluate which of the
        arguments.

        'quote_list' is the one of the mechanisms for creating list
        literal constants, also see <make_list>. It is normally used
        for assigning initial values to lists, or for passing an
        anonymous macro to another macro.

      Example::

        An example usage results in the list contains two strings
        followed by two integers.

>           list l;
>           l = quote_list("first", "second", 1, 2);

        As an alternative the following syntax implies the use of
        quote_list by the GRIEF Macro compiler.

>	    list l = {"first", "second", 1, 2};

    Macro Parameters:
        ... - One or more values to be made into list elements.

    Macro Returns:
        The 'quote_list()' primitive returns the built list.

    Macro Portability:
        n/a

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_extract
 */
void
do_quote_list(void)             /* list (list expr) */
{
    acc_assign_list(get_list(1), get_listlen(1));
}


/*  Function:           do_length_of_list
 *      length_of_list primitive, returns the length of list in atoms.
 *
 *  Macro Arguments:
 *      list - List.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: length_of_list - Determine list element count.

        int
        length_of_list(list lst)

    Macro Description:
        The 'length_of_list()' primitive retrieves the number of
        top-level elements (atoms) within the list 'lst'; with
        sub-lists being counted as single elements.

    Macro Returns:
        The 'length_of_list()' primitive returns the top-level
        element count.

    Macro Portability:
        n/a

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_extract
 */
void
do_length_of_list(void)         /* int (list expr) */
{
    acc_assign_int(lst_atoms_get(get_list(1)));
}


/*  Function:           do_is_type
 *      is_type primitive, which checks the type of a polymorphic expression.
 *
 *  Parameters:
 *      type - Type to match
 *
 *  Macro Arguments:
 *      expr - Expression.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: is_float - Determine whether a float type.

        int
        is_float(declare &symbol)

    Macro Description:
        The 'is_float()' primitive determines the type of a
        polymorphic expression and tests whether the specified
        'symbol' has of a floating-point type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if a float type, otherwise *false*.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        float, typeof

 *<<GRIEF>>
    Macro: is_integer - Determine whether an integer type.

        int
        is_integer(declare &symbol)

    Macro Description:
        The 'is_integer()' primitive determines the type of a
        polymorphic expression and tests whether the specified
        'symbol' has of an integer type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if an integer type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        int, typeof

 *<<GRIEF>>
    Macro: is_list - Determine whether a list type.

        int
        is_list(declare &symbol)

    Macro Description:
        The 'is_list()' primitive determines the type of a polymorphic
        expression and tests whether the specified 'symbol' has of a
        list type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if a list type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        list, typeof

 *<<GRIEF>>
    Macro: is_null - Determine whether a NULL type.

        int
        is_null(declare &symbol)

    Macro Description:
        The 'is_null()' primitive determines the type of a
        polymorphic expression and tests whether the specified
        'symbol' has of a NULL type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if a NULL type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        list, typeof

 *<<GRIEF>>
    Macro: is_string - Determine whether a string type.

        int
        is_string(declare &symbol)

    Macro Description:
        The 'is_string()' primitive determines the type of a polymorphic
        expression and tests whether the specified 'symbol' has of a
        string type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if a string type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        string, typeof

 *<<GRIEF>>
    Macro: is_array - Determine whether an array type.

        int
        is_array(declare &symbol)

    Macro Description:
        The 'is_array()' primitive determines the type of a
        polymorphic expression and tests whether the specified
        'symbol' has of a array type.

    Macro Parameters:
        symbol - Symbol reference.

    Macro Returns:
        *true* if a array type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        array, typeof

 *<<GRIEF>>
    Macro: is_type - Determine whether an explicit type.

        int
        is_type(declare &symbol, int|string type)

    Macro Description:
        The 'is_type()' primitive determines the type of a polymorphic
        expression and tests whether the specified 'symbol' is of the
        type 'type'.

    Macro Parameters:
        symbol - Symbol reference.

        type - Type identifier or name as follows.

(start table,format=nd)
        [Type           [Name                       ]
      ! F_INT           integer
      ! F_STR           string
      ! F_FLOAT         float, double
      ! F_LIST          list
      ! F_ARRAY         array
      ! F_NULL          NULL
      ! F_HALT          undef
(end)

    Macro Returns:
        *true* if the stated type, otherwise *false*.

    Macro Portability:
        n/a

    Macro See Also:
        array, typeof
 */
void
do_is_type(int type)            /* int (declare symbol, [int | string type]) */
{
    SYMBOL *sp = get_symbol(1);
    int ret = 0;

    if (255 == type) {                          /* is_type() */
        if (isa_string(2)) {
            const char *name = get_str(2);
            const int c0 = tolower(*(unsigned char *)name);

            switch (c0) {
            case 'i':
                if (0 == str_icmp(name, "integer") ||
                        0 == str_icmp(name, "int")) {
                    type = F_INT;
                }
                break;
            case 'b':
                if (0 == str_icmp(name, "boolean") ||
                        0 == str_icmp(name, "bool")) {
                    type = F_INT;
                }
                break;
            case 's':
                if (0 == str_icmp(name, "string") ||
                        0 == str_icmp(name, "str")) {
                    type = F_STR;
                }
                break;
            case 'f':
                if (0 == str_icmp(name, "float"))  {
                    type = F_FLOAT;
                }
                break;
            case 'd':
                if (0 == str_icmp(name, "double"))  {
                    type = F_FLOAT;
                }
                break;
#if defined(F_ARRRAY)
            case 'a':
                if (0 == str_icmp(name, "array"))  {
                    type = F_ARRAY;
                }
                break;
#endif
            case 'l':
                if (0 == str_icmp(name, "list"))   {
                    type = F_LIST;
                }
                break;
            case 'n':
                if (0 == str_icmp(name, "NULL"))   {
                    type = F_NULL;
                }
                break;
            case 'u':
                if (0 == str_icmp(name, "undef"))  {
                    acc_assign_int(isa_undef(1) ? 1 : 0);
                    return;
                }
                break;
            }
        } else {
            type = get_xinteger(2, -1);
        }
    }

    if (sp) {
        if (F_NULL == type && isa_list(1)) {
            const LIST *lp = (const LIST *)r_ptr(sp->s_obj);
            ret = (NULL == lp || F_HALT == *lp);
        } else {
            ret = (margv[1].l_flags == (accint_t) type);
        }
    }
    acc_assign_int(ret ? 1 : 0);
}


/*  Function:           do_typeof
 *      typeof primitive, which returns the type of an expression.
 *
 *  Macro Arguments:
 *      expr - Expression.
 *
 *  Macro Return:
 *      Type description, otherwise 'unknown-type'.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: typeof - Determine the symbol type.

        string
        typeof(declare &symbol)

    Macro Description:
        The 'typeof()' primitive determines the type of a polymorphic
        expression returning a string describing the underlying type.

    Macro Returns:
        The 'typeof()' primitive returns one of following strings
        dependent on the type of the specified symbol.

            o "integer"      - An integer type.
            o "string"       - A string type.
            o "float"        - A floating-point type.
            o "list"         - A List.
            o "array"        - An array (reserved for future use).
            o "NULL"         - NULL.
            o "undef"        - Undefined or omitted.
            o "unknown-type" - Unknown type.

    Macro Portability:
        n/a

    Macro See Also:
        is_integer, is_string, is_float, is_list, is_null
 */
void
do_typeof(void)                 /* (declare symbol) */
{
    if (isa_integer(1)) {
        acc_assign_str("integer", 7);

    } else if (isa_string(1)) {
        acc_assign_str("string", 6);

    } else if (isa_float(1)) {
        acc_assign_str("float", 5);

#if defined(F_ARRRAY)
    } else if (isa_array(1)) {
        acc_assign_str("array", 5);
#endif

    } else if (isa_list(1)) {
        acc_assign_str("list", 4);

    } else if (TRUE == isa_null(1)) {
        acc_assign_str("NULL", 4);

    } else if (isa_undef(1)) {
        acc_assign_str("undef", 5);

    } else {
        acc_assign_str("unknown-type", 12);
    }
}


/*  Function:           do_nth
 *      nth primitive, which returns the nth element of a list.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: nth - Extract the indexed list item.

        declare
        nth(list expr, int idx, [int idx2 ...])

    Macro Description:
        The 'nth()' primitive retrieves the value of the referenced
        list element at the index 'idx' from the list expression
        'expr'. The first element of a list is element 0; the 'nth'
        primitive allows lists to be treated like arrays. The last
        element of a list is <length_of_list> minus one.

        The 'nth()' primitive is an efficient way of accessing random
        elements within a list, than for example using the lists
        primitive <car> and <cdr>. In addition 'nth' can access
        access multidimensional lists as a single operation.

        Note!:
        This primitive is the underlying implementation of the list
        offset operator '[]', which are converted by the GRIEF Macro
        Compiler.

        Furthermore, this interface should be considered as an
        *internal* primitive. As this primitive is not compatible
        with the 'nth' macro of CRiSPEdit its direct usage is not
        recommended, instead rely on the offset operator '[]'.

    Macro Parameters:
        expr - List expression.

        idx - Integer index.

        ... - None or more integer indexs of sub-elements.

    Macro Returns:
        The 'nth()' primitive returns the value of the specified
        element from the referenced list, otherwise NULL on error.

    Macro Portability:
        GRIEF uses an alternative prototype to support support
        multidimensional lists, unlike CRiSPEdit which only supports a
        single dimension.

>	    nth(expr, list_expr)

        As such for portability 'nth' use should be restricted.

    Macro See Also:
        car, cdr, nth, put_nth, splice, list, pop, push
 */
void
do_nth(void)
{
    const LIST *lp = get_list(1);
    int i = 2;

    while (lp) {
        if (!isa_integer(i)) {                  /* index */
            errorf("nth: non integer index");
            lp = NULL;

        } else {
            const int nth = get_xinteger(i++, 0);

            if (nth < 0 || NULL == (lp = atom_nth(lp, nth))) {
                errorf("nth: subscript out of range");
                lp = NULL;
                break;
            } else if (i >= margc) {
                break;                          /* end of indices list */
            }

            /* dereference */
            if (F_RLIST == *lp) {               /* .. read and ref */
                lp = r_ptr(LGET_PTR2(ref_t, lp));
            } else if (F_LIST == *lp) {         /* .. skip length */
                lp += sizeof_atoms[F_LIST];
            } else {                            /* ??? */
                errorf("nth: non-list sub member");
                lp = NULL;
            }
        }
    }
    atom_assign_acc(lp);
}


/*  Function:           do_get_nth
 *      get_nth primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: get_nth - Retrieve a list element.

        declare
        get_nth(int idx, list expr)

    Macro Description:
        The 'get_nth()' primitive retrieves the element positioned at
        offset 'idx' within the specified list expression 'expr'.

        Note!:
        This primitive is generally not utilised directly as the
        GRIEF language supports list offset operator of the form
        '[idx]'.

    Macro Returns:
        The 'get_nth()' primitive returns the value of the referenced
        element, otherwise NULL if the index is out of bounds.

    Macro Portability:
        n/a

    Macro See Also:
        car, cdr, nth, put_nth, splice, list, pop, push
 */
void
do_get_nth(void)
{
    const int nth = get_xinteger(1, 0);
    const LIST *lp = get_list(2);

    if (nth < 0 || NULL == (lp = atom_nth(lp, nth))) {
        errorf("get_nth: subscript out of range");
        acc_assign_null();
    } else {
        lp = atom_nth(lp, nth);
        atom_assign_acc(lp);
    }
}


/*  Function:           do_put_nth
 *      put_nth primitive.
 *
 *   Macro Arguments:
 *       list - List available reference (LVAL).
 *       expr - Primary indices.
 *       [expr...] - Optional secondary indices.
 *       value - Value.
 *
 *  Macro Returns:
 *      value
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: put_nth - Modify a list element.

        declare
        put_nth(symbol, ...)

    Macro Description:
        The 'put_nth()' primitive modifies the element within the
        specified list.

        Note!:
        This primitive is generally not utilised directly as the
        GRIEF language supports a list offset operator of the form
        '[idx]'.

    Macro Returns:
        The 'put_nth()' primitive returns the value of the referenced
        element, otherwise NULL if the index is out of bounds.

    Macro Portability:
        Standard Crisp implementation utilises an alternative
        prototype which does not support multidimensional lists.

>	    put_nth(expr, list_expr, value)

        As such for portability 'put_nth' use should be restricted.

    Macro See Also:
        car, cdr, nth, put_nth, get_nth, splice, list, pop, push
 */
void
do_put_nth(void)                /* (symbol, ...) */
{
    SYMBOL *sp = get_symbol(1);
    const LISTV *lvp = NULL;
    ref_t *rp, *nrp;

    if (4 == margc) {
        const int nth = get_xinteger(2, 0);

        /* replace and update symbol */
        rp = sp->s_obj;
        lvp = margv + 3;
        if (NULL != (nrp = rlst_splice(rp, nth, 1, lvp, 1, TRUE))) {
            if (sp->s_obj != nrp) {
                sym_donate_ref(sp, nrp);
            } else if (x_dflags) {
                trace_sym(sp);
            }
         }

    } else if (margc > 4) {
        LIST *lp = NULL;
        int nth, i = 2;

        /* redirection */
        rp = sp->s_obj;
        while (1) {
            if (!isa_integer(i)) {
                errorf("put_nth: non integer index");
                acc_assign_null();
                return;
            }
            nth = get_xinteger(i, 0);

            if (++i >= (margc - 1)) {
                break;
            }

            lp = (LIST *)r_ptr(rp);
            if (NULL == (lp = (LIST *)atom_nth(lp, nth))) {
                errorf("put_nth: subscript out of range");
                acc_assign_null();
                return;

            } else if (F_RLIST != *lp) {
                errorf("put_nth: non-rlist sub member");
                acc_assign_null();
                return;
            }
            rp = LGET_PTR2(ref_t, lp);
        }

        /* replace and update owner */
        lvp = margv + i;
        if (NULL != (nrp = rlst_splice(rp, nth, 1, lvp, 1, TRUE))) {
            if (nrp != rp) {
                LPUT_PTR(lp, nrp);
                r_dec(rp);
            }
        }
    }

    acc_assign_argv(lvp);
}


/*  Function:           do_delete_nth
 *       delete_nth primitive.
 *
 *  Macro Arguments:
 *       list - List available reference (LVAL).
 *       offset - Starting offset.
 *       [length] - Length.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: delete_nth - Remove one or more elements from a list.

        void
        delete_nth(list lst,
            [int offset = 0], [int length = 1])

    Macro Description:
        The 'delete_nth()' primitive deletes 'length' elements from the
        specified list 'lst' starting at the index 'offset'.

    Macro Parameters:
        lst - List reference which shall have elements removed.

        offset - Optional integer offset within list, if omitted
            defaults to the first element being offset 0.

        length - Optional integer length, states the number of
            elements to be removed; if omitted only one element is
            removed. If 0 or less, than no elements will be removed.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        get_nth, nth, splice
 */
void
do_delete_nth(void)             /* (list lst, int offset = 0, [int length = 1]) */
{
    SYMBOL *sp = get_symbol(1);
    const int offset = get_xinteger(2, 0);
    const int length = get_xinteger(3, 1);
    ref_t *rp;

    if (length > 0) {
        if (NULL != (rp = rlst_splice(sp->s_obj, offset, length, NULL, 0, FALSE))) {
            if (sp->s_obj != rp) {
                sym_donate_ref(sp, rp);

            } else if (x_dflags) {
                trace_sym(sp);
            }
        }
    }
}


/*  Function:           do_splice
 *      splice primitives, splice values within a list.
 *
 *  Macro Parameters:
 *      list - List available reference (LVAL).
 *      offset - Offset within list.
 *      [length] - Optional length of delete arena.
 *      [value] - Value.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: splice - Splice a list, removing and/or adding elements.

        int
        splice(list lst, int offset = 0,
            [int length], [declare value])

    Macro Description:
        The 'splice()' primitive removes the elements designated by
        'offset' and 'length' from an list, and replaces them with
        the elements of 'value', if any.

        If 'offset' is negative then it starts that far from the end
        of the list.

        If 'length' is negative, removes the elements from 'offset'
        onward except for '-length' elements at the end of the list.

        If 'length' is omitted, removes everything from 'offset'
        onward. If both 'offset' and 'length' are omitted, removes
        everything.

        If 'offset' is past the end of the list, warning is issued
        and splices at the end of the list.

    Macro Parameters:
        lst - List expression.

        offset - Non-negative list offset from the front at which
            element are to be removed, when negative then it starts
            from the end. If omitted removes from the head of the list.

        length - Optional number of elements to be remove.

        value - Optional value to be inserted at the deletion point.

    Macro Examples:

      Equivalent::

        The splice primitive provides a general purpose list
        manipulation tool, which can be as a alternative to a number
        of specialise primitives.

        The following macro primitives <push>, <+=>, <pop>, <cdr>,
        <unshift> use on the left are equivalent to right splice usage.

>           push(lst, x)	
>           or lst += x	        splice(lst, length_of_list(a), 0, x)
>           pop(lst)	        splice(lst, -1)
>           cdr(lst)	        splice(lst, 0, 1)
>           unshift(lst, x)	splice(lst, 0, 0, x)
>           lst[i] = y	        splice(lst, i, 1, y)

      Flatten mode::

        Splice usage implies the non-flatting of a source list when
        appending into another, whereas list append operations shall
        flatten the source list at level 0, as such when used in the
        following context the result is;

>           list lst = make_list("a", "b");
>           lst += lst;
>           // result = "a", "b", "a", "b"

        The alternative splice usage which shall place a list
        reference within the list.

>           splice(lst, length_of_list(lst), 0, lst);

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        pop, push, shift, splice, car, cdr
 */
void
do_splice(void)                 /* void (list lst, int offset = 0, [int length], [declare value]) */
{
    SYMBOL *sp = get_symbol(1);
    const int nth = get_xinteger(2, 0);
    int length;
    ref_t *rp;

    /* length, NULL equal EOL */
    if (isa_integer(3)) {
        length = get_xinteger(3, 0);
    } else {
        length = -1;                            /* upon EOL */
    }

    /* delete and/or insert value */
    if (NULL != (rp = rlst_splice(sp->s_obj, nth, length, margv + 4, margc - 4, FALSE))) {
        if (sp->s_obj != rp) {
            sym_donate_ref(sp, rp);
        } else if (x_dflags) {
            trace_sym(sp);
        }
    }
}


/*  Function:           do_push
 *      Push onto the bottom of a list.
 *
 *  Macro Arguments:
 *      list - List available reference (LVAL).
 *      [value..] - Value.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: push - Add an element onto a list.

        declare
        push(list lst, declare value ...)

    Macro Description:
        The 'push()' primitive adds one or more elements 'value' to
        the end of the list.

    Macro Returns:
        The 'push()' primitive returns a copy of the value added.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        pop, push, shift, splice, car, cdr
 */
void
do_push(void)                   /* void (list lst, declare value ...) */
{
    SYMBOL *sp = get_symbol(1);
    const LIST *lp = (const LIST *)r_ptr(sp->s_obj);

    if (margc >= 2) {                           /* values exist */
        int atoms = lst_atoms_get(lp);
        ref_t *rp;

        if (NULL != (rp = rlst_splice(sp->s_obj, atoms, 0, margv + 2, margc - 2, FALSE))) {
            if (sp->s_obj != rp) {
                sym_donate_ref(sp, rp);
            } else if (x_dflags) {
                trace_sym(sp);
            }
        }
    }
}


/*  Function:           do_shift
 *      Remove and return the element from the head of the list.
 *
 *  Macro Arguments:
 *      list - List available reference (LVAL).
 *
 *  Return:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: shift - Shift the first list element.

        declare
        shift(list lst)

    Macro Description:
        The 'shift()' primitive removes (shifts) the first value of
        the list 'lst' off and returns its, shortening the list by 1
        and moving everything down.

        If there are no elements in the list, the following is echoed
        onthe command promot and 'shift' returns the null value.

>           shift: empty list.

    Macro Parameters:
        lst - List expression.

    Macro Returns:
        The 'shift()' primitive returns the first element on the list.

    Macro Examples:

        Iterator the list from the front, removing and then printing
        each element in the process;

>       list l = {"one", "two", "three", "four"};
>
>       while (length_of_list(l)) {
>           message("%s", shift(l));
>       }

        the resulting output

>           one
>           two
>           three
>           four

    Macro Portability:
        A Grief extension

    Macro See Also:
        pop, push, shift, splice, car, cdr
 */
void
do_shift(void)                  /* declare (list lst) */
{
    SYMBOL *sp = get_symbol(1);
    const LIST *lp = (const LIST *)r_ptr(sp->s_obj);
    const int atoms = lst_atoms_get(lp);

    if (atoms <= 0) {
        errorf("shift: empty list.");
        acc_assign_null();

    } else {
        ref_t *rp;

        atom_assign_acc(lp);
        if (NULL != (rp = rlst_splice(sp->s_obj, 0, 1, NULL, 0, FALSE))) {
            if (sp->s_obj != rp) {
                sym_donate_ref(sp, rp);
            } else if (x_dflags) {
                trace_sym(sp);
            }
        }
    }
}


/*  Function:           do_unshift
 *      Add an element to the head of the list.
 *
 *  Macro Arguments:
 *      list - List available reference (LVAL).
 *
 *  Return:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: unshift - Shift the first list element.

        declare
        unshift(list lst, declare value)

    Macro Description:
        The 'unshift()' primitive is reserved for future use.

        The 'unshift()' primitive performs the opposite action to
        that of a <shift>. It prepends 'value' to the front of the
        list 'lst' returns the resulting number of elements within
        the list.

    Macro Parameters:
        lst - List to be modified.

        value - Value to be prepended.

    Macro Returns:
        The 'unshift()' primitive returns the resulting number of
        list elements.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        pop, push, shift, splice, car, cdr
 */
void
do_unshift(void)                /* int (list lst, declare value) */
{
    SYMBOL *sp = get_symbol(1);
    const LIST *lp = (const LIST *)r_ptr(sp->s_obj);

    if (margc >= 2) {                           /* values exist */
        ref_t *rp;

        if (NULL != (rp = rlst_splice(sp->s_obj, 0, 0, margv + 2, margc - 2, FALSE))) {
            if (sp->s_obj != rp) {
                sym_donate_ref(sp, rp);
            } else if (x_dflags) {
                trace_sym(sp);
            }
        }
    }
    acc_assign_int(lst_atoms_get(lp));          /* final count */
}


/*  Function:           do_pop
 *      Pop from the bottom of a list.
 *
 *  Macro Arguments:
 *      list - List available reference (LVAL).
 *
 *  Return:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: pop - Pop the last element.

        declare
        pop(list expr)

    Macro Description:
        The 'pop()' primitive removes and returns the last value of
        the list, shortening the list by one element.

    Macro Returns:
        The 'pop()' primitive returns the element removed, otherwise
        NULL on error.

    Macro Example:

        Iterator the list from the end, removing and then printing
        each element in the process;

>           list l = {"one", "two", "three", "four"};
>
>           while (length_of_list(l)) {
>               message("%s", pop(l));
>           }

        the resulting output

>           four
>           three
>           two
>           one

    Macro Portability:
        A Grief extension

    Macro See Also:
        pop, push, shift, splice, car, cdr
 */
void
do_pop(void)                    /* declare (list lst) */
{
    SYMBOL *sp = get_symbol(1);
    const LIST *lp = (const LIST *)r_ptr(sp->s_obj);
    int atoms = lst_atoms_get(lp);

    if (atoms <= 0) {
        errorf("pop: empty list.");
        acc_assign_null();

    } else {
        ref_t *rp;
        int nth;

        /* locate last atom */
        for (atoms--, nth = 0; nth < atoms; ++nth) {
            lp = atom_next(lp);                 /* XXX - optimise, need cursor */
        }

        /* return */
        assert(F_HALT != *lp);
        atom_assign_acc(lp);
        assert(F_HALT == *atom_next(lp));

        /* remove */
        if (NULL != (rp = rlst_splice(sp->s_obj, atoms, 1, NULL, 0, FALSE))) {
            if (sp->s_obj != rp) {
                sym_donate_ref(sp, rp);
            } else if (x_dflags) {
                trace_sym(sp);
            }
        }
    }
}


/*  Function:           list_each
 *      list_each primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: list_each - Iterator though the list elements.

        int
        list_each(list lst,
            declare &value, [int increment = 1])

    Macro Description:
        The 'list_each()' primitive iterates over a normal list value
        and sets the variable 'value' to be each element of the list
        in turn.

      Example::

        Split the flag list specification 'spec' into its components
        and iterator the results.

>           const list flags = split(spec, ",");
>           string flag;
>
>           while (list_each(flags, flag) >= 0) {
>               process_flag(flag);
>           }

        Warning!:
        If any part of the list is modified, foreach may become very
        confused. Adding or removing elements within the loop body,
        for example with <splice> shall result in unpredictable
        results possibility crashing the editor.

        Secondary list iteration continues until the last element
        within the list is consumed; if the loop is terminated prior
        to the end condition, <list_reset> should be used to reset
        the list cursor for subsequent iterations.

    Macro Parameters:
        lst - List expression.

        value - Variable populated with the element value.

        increment - Optional integer iterator loop increment, if
            omitted defaults to one element.

    Macro Returns:
        The 'list_each()' primitive returns the element index
        retrieved, otherwise -1 upon an end-of-list condition or error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_reset,
        list_extract
 */
void
do_list_each(void)              /* (list lst, declare value, [int increment = 1]) */
{
    const LIST *lp = get_list(1);
    int ret = -1;

    if (! lst_isnull(lp)) {
        SYMBOL *value = get_symbol(2);
        int increment = get_xinteger(3, 0);
        struct listhead *lh = ((struct listhead *)lp)-1;

        /*
         *  next .. move along list by 'increment' elements
         *
         *  TODO:   lh_editseq, incremented on list changes abort the list on change.
         */
        lst_check(lp);
        if (lh->lh_cursor) {
            lp = lh->lh_cursor;
            if (increment <= 0) {
                increment = 1;                  /* must be positive */
            }
            do {
                lp += atom_size(lp);
            } while (--increment > 0 && F_HALT != *lp);
        } else {
            lh->lh_index = 0;
        }

        if (F_HALT == *lp) {
            lh->lh_cursor = NULL;
        } else {
            if (value) {
                atom_assign_sym(lp, value);
            }
            ret = lh->lh_index++;               /* index 0.... */
            lh->lh_cursor = lp;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           list_reset
 *      list_reset primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: list_reset - Reset the list iterator.

        int
        list_reset(list lst)

    Macro Description:
        The 'list_reset()' primitive resets the list iteration cursor.

    Macro Parameters:
        lst - List expression.

    Macro Returns:
        The 'list_reset()' primitive returns the selected element
        index.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_reset,
        list_extract
*/
void
do_list_reset(void)             /* (list lst, [int index = 0]) */
{
    const LIST *lp = get_list(1);
    int ret = -1;

    if (! lst_isnull(lp)) {
        struct listhead *lh = ((struct listhead *)lp)-1;
    //  int cursor = get_xinteger(2, 0);

        lst_check(lp);
        lh->lh_cursor = NULL;
        lh->lh_index = 0;

    //  if (cursor >= 0) {
    //      int t_cursor = cursor;
    //      do {
    //          lp += atom_size(lp);
    //      } while (--t_cursor > 0 && F_HALT != *lp);
    //      if (0 == t_cursor && F_HALT != *lp) {
    //          lh->lh_cursor = lp;
    //          lh->lh_index = cursor;
    //      }
    //  }
        ret = 0;
    }
    acc_assign_int(ret);
}


/*  Function:           list_extract
 *      list_extract primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: list_extract - Extract list elements.

        list
        list_extract(list lst,
            int start, [int end], [int increment])

    Macro Description:
        The 'list_extract()' primitive iterates over the list values
        generating a new list of the referenced elements within the
        list.

    Macro Parameters:
        lst - List expression.

        start - Optional integer index, if specified states the
            first element index at which the iterations begins. When
            omitted the iterations begins from the start of the list.

        end - Optional integer index, if specified states the
            last element index at which the iterations terminates.
            When omitted the iterations completes at the end of the
            list.

        increment - Optional integer iterator loop increment, if
            omitted defaults to one element.

    Macro Returns:
        The 'list_extract()' primitive returns a list containing the
        referenced elements, otherwise a NULL list on error.

    Macro Portability:
        n/a

    Macro See Also:
        make_list, quote_list, length_of_list, list_each, list_extract
 */
void
do_list_extract(void)           /* (list lst, int start, [int end], [int increment]) */
{
    const LIST *lp = get_list(1);
    int start = get_xinteger(2, 0);
    int end = get_xinteger(3, -1);
    int increment = get_xinteger(4, 0);
    LIST *retlp = NULL;                         /* returned lisp */
    int length = 0;                             /* and its length, in bytes */

    if (! lst_isnull(lp)) {
        const LIST *srclp, *nextlp;
        int idx = 0, offset, count = 0;

        /* boundary conditions */
        if (start < 0) {
            start = 0;                          /* must be non-neg */
        }
        if (increment <= 0) {
            increment = 1;                      /* must be positive */
        }

        /* accumulate size requirements */
        idx = 0;
        offset = increment;
        srclp = lp;
        length = 0;
        while ((nextlp = atom_next(srclp)) != srclp) {
            if (idx >= start) {
                if (-1 == end || idx <= end) {
                    if (offset >= increment) {
                        length += atom_size(srclp);
                        offset = 0;
                        ++count;
                    } else {
                        ++offset;
                    }
                } else {
                    break;                      /* end reached */
                }
            }
            ++idx;
            srclp = nextlp;
        }

        /* return elements */
        if (length && count) {
            length += sizeof_atoms[F_HALT];
            if (NULL != (retlp = lst_alloc(length, count))) {
                LIST *dstlp = retlp;

                idx = 0;
                offset = increment;
                srclp = lp;
                length = 0;
                while ((nextlp = atom_next(srclp)) != srclp) {
                    if (idx >= start) {
                        if (-1 == end || idx <= end) {
                            if (offset >= increment) {
                                length += atom_size(srclp);
                                dstlp += atom_copy(dstlp, srclp);
                                offset = 0;
                                ++count;
                            } else {
                                ++offset;
                            }
                        } else {
                            break;              /* end reached */
                        }
                    }
                    ++idx;
                    srclp = nextlp;
                }
                atom_push_halt(dstlp);          /* terminate new list */
                length += sizeof_atoms[F_HALT];
            }
        }
    }

    if (retlp) {
        acc_donate_list(retlp, length);
    } else {
        acc_assign_null();
    }
}
/*end*/
