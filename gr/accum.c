#include <edidentifier.h>
__CIDENT_RCSID(gr_accum_c,"$Id: accum.c,v 1.39 2025/02/07 03:03:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: accum.c,v 1.39 2025/02/07 03:03:20 cvsuser Exp $
 * Accumulator manipulating.
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

#if !defined(ED_LEVEL)
#define ED_LEVEL 1
#endif

#include <editor.h>

#include "accum.h"
#include "debug.h"
#include "lisp.h"
#include "main.h"
#include "object.h"

/*
 *  Accumulator object
 *
 *      Type            Accumulator contains
 *
 *      F_INT           32-bit number.
 *      F_FLOAT         floating point number.
 *      F_LIT           ptr to a literal string which can never be freed.
 *      F_STR           ptr to a private string buffer.
 *      F_RSTR          ptr to a reference string.
 *      F_RLIST         ptr to a list.
 */
typedef struct accumulator {
    OPCODE              ac_type;                /* Current type of accumulator */
    size_t              ac_memlen;              /* Storage length, in bytes */
    char *              ac_memptr;              /* Storage address */
#define ACC_ALLOCROUND       0x3ff

    int                 ac_length;              /* Length of composite object */
    union n {
        accint_t        ival;                   /* F_INT */
        accfloat_t      fval;                   /* F_FLOAT */
        const char *    lval;                   /* F_LIT */
        char *          sval;                   /* F_STR */
        ref_t *         rval;                   /* F_RLIST or F_RSTR */

#define ac_ival         ac.ival
#define ac_fval         ac.fval
#define ac_lval         ac.lval
#define ac_sval         ac.sval
#define ac_rval         ac.rval
    } ac;
} accumulator_t;

static __CINLINE void           acc_zap(void);
static __CINLINE void           acc_size(size_t len);

static accumulator_t __CCACHEALIGN accum = {    /* global accumulator */
        F_INT
        };

#define ED_ACCTRACE(x)
#if defined(ED_LEVEL)
#if (ED_LEVEL >= 2)
#undef  ED_ACCTRACE
#define ED_ACCTRACE(x)  { ED_TRACE(x) acc_trace(); }
#endif
#endif


/*
 *  acc_expand ---
 *      Expand the accumulator (if needed) to accomdate at least 'len' bytes.
 */
void *
acc_expand(size_t len)
{
    acc_zap();
    if (len >= accum.ac_memlen) {
        accum.ac_memlen = (len | ACC_ALLOCROUND) + 1;
        accum.ac_memptr = chk_realloc(accum.ac_memptr, accum.ac_memlen);
    }
    accum.ac_type = F_NULL;
    assert(accum.ac_memptr);
    return accum.ac_memptr;
}


/*
 *  acc_size ---
 *      Size the accumulator to accomdate 'len' bytes.
 */
static __CINLINE void
acc_size(size_t len)
{
#if !defined(NDEBUG)
    if ((DB_PURIFY|DB_MEMORY) & x_dflags) {
        /*
         *  Allow purify/memcheck style memory protection.
         *      Release and alloc a new block allowing runtime memory block usage traces to occur.
         */
        chk_free(accum.ac_memptr);
        accum.ac_memptr = chk_alloc(accum.ac_memlen = len);
        return;
    }
#endif  /*NDEBUG*/

    /*
     *  Reallocate if needed.
     */
    if (len > accum.ac_memlen) {
        void *ptr;

        accum.ac_memlen = (len | ACC_ALLOCROUND) + 1;
        if (NULL == (ptr = chk_realloc(accum.ac_memptr, accum.ac_memlen))) {
            if (NULL != (ptr = chk_alloc(accum.ac_memlen))) {
                chk_free((void *)accum.ac_memptr);
            }
        }
        accum.ac_memptr = ptr;
    }
}


/*
 *  acc_zap ---
 *      Release any external accumulator references.
 */
static __CINLINE void
acc_zap(void)
{
    switch (accum.ac_type) {
    case F_RSTR:
    case F_RLIST:
        r_dec(accum.ac_rval);
        accum.ac_rval = NULL;
        break;
#if defined(NDEBUG)
    default:
        break;
#else
    case F_INT:
    case F_FLOAT:
    case F_LIT:
    case F_STR:
    case F_NULL:
        break;
    default:
        panic("acc_zap: type? (%d)", accum.ac_type);
        break;
#endif
    }
}


/*
 *  acc_clear ---
 *      Clear memory in accumulator before assigning new value.
 */
void
acc_clear(void)
{
    acc_zap();
    accum.ac_type = F_NULL;
}


/*
 *  acc_assign_int ---
 *      Assign an integer to the accumulator.
 */
void
acc_assign_int(accint_t val)
{
    acc_zap();
    accum.ac_ival = val;
    accum.ac_type = F_INT;
    ED_ACCTRACE(("acc_donate_int()"))
}


/*
 *  acc_assign_float ---
 *      Assign a float to the accumulator.
 */
void
acc_assign_float(accfloat_t val)
{
    acc_zap();
    accum.ac_fval = val;
    accum.ac_type = F_FLOAT;
    ED_ACCTRACE(("acc_donate_float()"))
}


/*
 *  acc_assign_lit ---
 *     Assign a literal string (one that cannot be modified or freed) to the accumulator.
 */
void
acc_assign_lit(const char *str)
{
    acc_zap();
    accum.ac_lval = str;
    accum.ac_type = F_LIT;
    ED_ACCTRACE(("acc_assign_lit()"))
}


/*
 *  acc_assign_str ---
 *      Assign a string to the accumulator.
 */
void
acc_assign_nstr(const char *str, size_t len)
{
    acc_zap();
    acc_size(len + 1);
    accum.ac_sval = accum.ac_memptr;
    memmove(accum.ac_sval, str, (size_t)len);
    accum.ac_sval[len] = 0;
    accum.ac_type = F_STR;
    ED_ACCTRACE(("acc_assign_str()"))
}

void
acc_assign_str(const char *str)
{
    acc_assign_nstr(str, strlen(str));
}


/*
 *  acc_assign_strlen ---
 *      Assign the length of the accumulator, excluding NUL.
 */
void
acc_assign_strlen(size_t len)
{
    assert(len < accum.ac_memlen);
    assert(F_NULL == accum.ac_type);            /* assigned by acc_expand() */
    accum.ac_sval = accum.ac_memptr;
    accum.ac_type = F_STR;
    assert(0 == accum.ac_sval[len]);
    ED_ACCTRACE(("acc_assign_strlen()"))
}


/*
 *  acc_assign_str2 ---
 *      Concatenate and assign the two strings to the accumulator.
 */
void
acc_assign_str2(const char *str1, size_t len1, const char *str2, size_t len2)
{
    size_t len;

    acc_zap();
        // if (len1 < 0) len1 = strlen(str1);
        // if (len2 < 0) len2 = strlen(str2);
    len = len1 + len2;
    acc_size(len + 1);
    accum.ac_sval = accum.ac_memptr;
    memmove(accum.ac_sval, str1, (size_t)len1);
    memmove(accum.ac_sval + len1, str2, (size_t)len2);
    accum.ac_sval[len] = 0;
    accum.ac_type = F_STR;
    ED_ACCTRACE(("acc_assign_str2()"))
}


/*
 *  acc_assign_argv ---
 *      Assign an argv element to the accumulator.
 */
void
acc_assign_argv(const LISTV *lvp)
{
    switch (lvp->l_flags) {
    case F_INT:
        acc_assign_int(lvp->l_int);
        break;
    case F_FLOAT:
        acc_assign_float(lvp->l_float);
        break;
    case F_LIT:
    case F_STR:
        acc_assign_str(lvp->l_str);
        break;
    case F_RSTR:
    case F_RLIST:
        acc_assign_ref(lvp->l_ref);
        break;
    case F_NULL:
        acc_assign_null();
        break;
    default:
        panic("acc_assign_argv: type? (%d)", lvp->l_flags);
        break;
    }
    ED_ACCTRACE(("acc_assign_argv()"))
}


/*
 *  acc_assign_ref ---
 *      Assign a reference pointer to the accumulator.
 */
void
acc_assign_ref(ref_t *rp)
{
    acc_zap();
    if (rp) {
        accum.ac_rval = r_inc(rp);
        accum.ac_type = (F_LIST == r_type(rp) ? F_RLIST : F_RSTR);
    } else {
        acc_assign_null();
    }
    ED_ACCTRACE(("acc_assign_ref()"))
}


/*
 *  acc_assign_list ---
 *      Assign a list to the accumulator.
 */
void
acc_assign_list(const LIST *lp, size_t llen)
{
    acc_zap();
    accum.ac_rval = rlst_build(lp, llen);
    accum.ac_type = F_RLIST;
    ED_ACCTRACE(("acc_assign_list()"))
}


/*
 *  acc_donate_list ---
 *      Assign a newly created list to the accumulator We are given the
 *      list so we don't need to allocate any more memory to it
 */
void
acc_donate_list(LIST *lp, size_t llen)
{
    __CIFDEBUG(size_t t_llen = lst_length(lp);)
    assert(llen == t_llen);

    acc_zap();
    lst_check(lp);
    accum.ac_rval = rlst_create(lp, (int)llen);
    accum.ac_type = F_RLIST;
    ED_ACCTRACE(("acc_donate_list()"))
}


/*
 *  acc_assign_null ---
 *      Assign the NULL value to the accumulator.
 */
void
acc_assign_null(void)
{
    acc_zap();
    accum.ac_type = F_NULL;
    ED_ACCTRACE(("acc_assign_null()"))
}


/*
 *  acc_assign_object ---
 *      Assign the specified object to accumulator.
 */
void
acc_assign_object(object_t *obj)
{
    const OPCODE type = obj_get_type(obj);

    switch(type) {
    case F_INT:
        acc_assign_int(obj_get_ival(obj));
        break;
    case F_FLOAT:
        acc_assign_float(obj_get_fval(obj));
        break;
    case F_LIT:
    case F_STR:
        acc_assign_str(obj_get_sval(obj));
        break;
    case F_RSTR:
    case F_RLIST:
        acc_assign_ref(obj_get_ref(obj));
        break;
    case F_NULL:
        acc_assign_null();
        break;
    default:
        panic("acc_assign_obj: type? (%d)", type);
        break;
    }
    ED_ACCTRACE(("acc_assign_obj()"))
}


/*
 *  acc_get_ival ---
 *      Return accumulator integer value.
 */
accint_t
acc_get_ival(void)
{
    switch (accum.ac_type) {
    case F_INT:
        return accum.ac_ival;
    case F_FLOAT:
        return (accint_t) accum.ac_fval;
    default:
        break;
    }
    return 0;
}


/*
 *  acc_get_fval ---
 *      Return accumulator float value.
 */
accfloat_t
acc_get_fval(void)
{
    switch (accum.ac_type) {
    case F_INT:
        return (accfloat_t) accum.ac_ival;
    case F_FLOAT:
        return accum.ac_fval;
    default:
        break;
    }
    return 0;
}


/*
 *  acc_get_sval ---
 *      Return pointer to string in accumulator, otherwise NULL if not a string.
 */
const char *
acc_get_sval(void)
{
    switch (accum.ac_type) {
    case F_LIT:
        return accum.ac_lval;
    case F_STR:
        return accum.ac_sval;
    case F_RSTR:
        return r_ptr(accum.ac_rval);
    default:
        break;
    }
    return NULL;
}


/*
 *  acc_get_ref ---
 *      Return pointer to the reference value stored in the accumulator.
 */
ref_t *
acc_get_ref(void)
{
    switch (accum.ac_type) {
    case F_RSTR:
    case F_RLIST:
        return accum.ac_rval;
    default:
        break;
    }
    return NULL;
}


/*
 *  acc_get_sbuf ---
 *      Retrieve address of modifiable buffer.
 */
char *
acc_get_sbuf(void)
{
    switch (accum.ac_type) {
    case F_LIT:
        acc_assign_str(accum.ac_lval);
        return accum.ac_sval;
    case F_STR:
        return accum.ac_sval;
    case F_RSTR:
        if (1 == r_refs(accum.ac_rval)) {
            return r_ptr(accum.ac_rval);
        }
        acc_assign_nstr(r_ptr(accum.ac_rval), (size_t)r_used(accum.ac_rval));
        return accum.ac_sval;
    default:
        break;
    }
    return NULL;
}


/*
 *  acc_get_type ---
 *      Return type of variable stored in accumulator.
 */
OPCODE
acc_get_type(void)
{
    return accum.ac_type;
}


/*
 *  acc_trace ---
 *      Trace assignments to accumulator, only trace accumulator if debugging turned on
 *      and its value has changed.
 */
void
acc_trace(void)
{
    if (x_dflags) {
        switch (accum.ac_type) {
        case F_INT:
            trace_ilog("  iACC=%" ACCINT_FMT "\n", accum.ac_ival);
            break;
        case F_FLOAT:
            trace_ilog("  fACC=%" ACCFLOAT_FMT  "\n", accum.ac_fval);
            break;
        case F_LIT:
            trace_ilog("  lACC='%s'\n", c_string(accum.ac_lval));
            break;
        case F_STR:
            trace_ilog("  sACC='%s'\n", c_string(accum.ac_sval));
            break;
        case F_RSTR:
            trace_ilog("  SACC%u='%s'\n", (unsigned)r_refs(accum.ac_rval), c_string(r_ptr(accum.ac_rval)));
            break;
        case F_RLIST:
            trace_ilog("  LACC=");
            trace_list((LIST *) r_ptr(accum.ac_rval));
            break;
        case F_NULL:
            trace_ilog("  NULL\n");
            break;
        default:
            panic("acc_trace: type? (%d)", accum.ac_type);
            break;
        }
    }
}

/*end*/
