#include <edidentifier.h>
__CIDENT_RCSID(gr_object_c,"$Id: object.c,v 1.14 2014/10/22 02:33:14 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: object.c,v 1.14 2014/10/22 02:33:14 ayoung Exp $
 * Functions for manipulating objects.
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

#define  OBJECT_H_INTERNALS
#include "object.h"
#include "debug.h"
#include "lisp.h"
#include "main.h"


/*
 *  Size to round up to when expanding object memory objects
 */
#define OBJ_ALLOCROUND      0x3ff

static void                 obj_size(object_t *obj, int len);

#define ED_OBJTRACE(x)
#if defined(ED_LEVEL)
#if (ED_LEVEL >= 2)
#undef  ED_OBJTRACE
#define ED_OBJTRACE(x)      { ED_TRACE(x) obj_trace(obj); }
#endif
#endif


/*
 *  obj_init ---
 *      Initialise an object.
 */
void
obj_init(object_t *obj)
{
    obj->obj_magic = OBJECT_MAGIC;
    obj->obj_type = F_NULL;
    obj->obj_length = 0;
    obj->obj_attributes = 0;
    obj->obj_memptr = NULL;
    obj->obj_ival = 0;
}


object_t *
obj_alloc(void)
{
    object_t *obj;

    if (NULL != (obj = chk_alloc(sizeof(object_t)))) {
        obj_init(obj);
    }
    return obj;
}


object_t *
obj_copy(const object_t *copy)
{
    object_t *obj;

    if (NULL != (obj = obj_alloc())) {
        obj_assign(obj, copy);
    }
    return obj;
}


void
obj_free(object_t *obj)
{
    if (obj) {
        obj_zap(obj);
        chk_free(obj);
    }
}


void
obj_zap(object_t *obj)
{
    obj_clear(obj);
    chk_free(obj->obj_memptr);
    obj_init(obj);
}


/*
 *  obj_clear ---
 *      Clear memory in object before assigning new value.
 */
void
obj_clear(object_t *obj)
{
    assert(OBJECT_MAGIC == obj->obj_magic);
    switch (obj->obj_type) {
    case F_INT:
    case F_FLOAT:
    case F_NULL:
        obj->obj_fval = 0;
        break;

    case F_LIT:
    case F_STR:
        obj->obj_sval = NULL;
        break;

    case F_RSTR:
    case F_RLIST:
        r_dec(obj->obj_rval);
        obj->obj_rval = NULL;
        break;

    default:
        panic("bad object type ? (%d)", obj->obj_type);
        break;
    }
}


/*
 *  obj_expand ---
 *      Expand the object (if needed) to accomdate at least 'len' bytes.
 */
void *
obj_expand(object_t *obj, int len)
{
    assert(OBJECT_MAGIC == obj->obj_magic);
    assert(len >= 0);

    if ((unsigned) len > obj->obj_length) {
        /*
         *  expand storage
         */
        obj->obj_length = (len | OBJ_ALLOCROUND) + 1;
        obj->obj_memptr = chk_realloc(obj->obj_memptr, obj->obj_length);
    }
    return obj->obj_memptr;
}


/*
 *  obj_size ---
 *      Size the object to accomdate 'len' bytes.
 */
static void
obj_size(object_t *obj, int len)
{
    assert(OBJECT_MAGIC == obj->obj_magic);
    assert(len >= 0);

    if ((DB_PURIFY|DB_MEMORY) & x_dflags) {
        /*
         *  Allow purify/memcheck style memory protection.
         *
         *  Release and alloc a new block allowing runtime memory block usage
         *  traces to occur.
         */
        if (obj->obj_memptr) {
            chk_free(obj->obj_memptr);
        }
        obj->obj_memptr = chk_alloc(obj->obj_length = len);

    } else {
        /*
         *  If current string we're holding onto is too small for the new string,
         *  then expand it
         */
        if ((unsigned) len > obj->obj_length) {
            obj->obj_length = (len | OBJ_ALLOCROUND) + 1;
            obj->obj_memptr = chk_realloc(obj->obj_memptr, obj->obj_length);
        }
    }
}


void
obj_assign(object_t *obj, const object_t *copy)
{
    obj_clear(obj);
    if (copy) {
        switch (copy->obj_type) {
        case F_INT:
        case F_FLOAT:
        case F_LIT:
            obj->obj_type = copy->obj_type;
            obj->obj_val = copy->obj_val;
            break;
        case F_STR:
            obj_assign_str(obj, copy->obj_sval, -1);
            break;
        case F_RSTR:
        case F_RLIST:
            obj_assign_ref(obj, copy->obj_rval);
            break;
        case F_NULL:
            break;
        default:
            panic("obj_assign: type? (%d)", copy->obj_type);
            break;
        }
    }
    ED_OBJTRACE(("obj_assign()"))
}


/*
 *  obj_assign_argv ---
 *      Assign an argv element to the object.
 */
void
obj_assign_argv(object_t *obj, const LISTV *lvp)
{
    switch (lvp->l_flags) {
    case F_INT:
        obj_assign_int(obj, lvp->l_int);
        break;
    case F_FLOAT:
        obj_assign_float(obj, lvp->l_float);
        break;
    case F_LIT:
    case F_STR:
        obj_assign_str(obj, lvp->l_str, -1);
        break;
    case F_RSTR:
    case F_RLIST:
        obj_assign_ref(obj, lvp->l_ref);
        break;
    case F_NULL:
        obj_assign_null(obj);
        break;
    default:
        panic("obj_assign_argv: type? (%d)", lvp->l_flags);
        break;
    }
    ED_OBJTRACE(("obj_assign_argv()"))
}


/*
 *  obj_assign_int ---
 *      Assign an integer to the object.
 */
void
obj_assign_int(object_t *obj, accint_t val)
{
    obj_clear(obj);
    obj->obj_type = F_INT;
    obj->obj_ival = val;
    ED_OBJTRACE(("obj_assign_int()" ))
}


/*
 *  obj_assign_float ---
 *      Assign a float to the object.
 */
void
obj_assign_float(object_t *obj, accfloat_t val)
{
    obj_clear(obj);
    obj->obj_type = F_FLOAT;
    obj->obj_fval = val;
    ED_OBJTRACE(("obj_assign_float()" ))
}


/*
 *  obj_assign_lit ---
 *     Assign a literal string (one that cannot be modified or freed) to the object.
 */
void
obj_assign_lit(object_t *obj, const char *str)
{
    obj_clear(obj);
    obj->obj_lval = str;
    obj->obj_type = F_LIT;
    ED_OBJTRACE(("obj_assign_lit()"))
}


/*
 *  obj_assign_str ---
 *      Assign a string to the object.
 */
void
obj_assign_str(object_t *obj, const char *str, int len)
{
    obj_clear(obj);
    if (len < 0) len = (int)strlen(str);
    obj_size(obj, len + 1);
    obj->obj_sval = obj->obj_memptr;
    memmove(obj->obj_sval, str, len);
    obj->obj_sval[len] = '\0';
    obj->obj_type = F_STR;
    ED_OBJTRACE(("obj_assign_str()"))
}


/*
 *  obj_assign_str2 ---
 *      Concat and assign the two strings to the object. If either
 *      length is less then zero, then we need to perform a strlen() on the string.
 */
void
obj_assign_str2(object_t *obj, const char *str1, int len1, const char *str2, int len2)
{
    int len;

    obj_clear(obj);
    if (len1 < 0) len1 = (int)strlen(str1);
    if (len2 < 0) len2 = (int)strlen(str2);
    len = len1 + len2;
    obj_size(obj, len + 1);
    obj->obj_sval = obj->obj_memptr;
    memmove(obj->obj_sval, str1, len1);
    memmove(obj->obj_sval+len1, str2, len2);
    obj->obj_sval[len] = '\0';
    obj->obj_type = F_STR;
    ED_OBJTRACE(("obj_assign_str2()"))
}


/*
 *  obj_assign_ref ---
 *      Assign a reference pointer to the object.
 */
void
obj_assign_ref(object_t *obj, ref_t *rp)
{
    obj_clear(obj);
    if (rp) {
        obj->obj_rval = r_inc(rp);
        obj->obj_type = (F_LIST == r_type(rp) ? F_RLIST : F_RSTR);
    } else {
        obj_assign_null(obj);
    }
    ED_OBJTRACE(("obj_assign_ref()"))
}


/*
 *  obj_assign_list ---
 *      Assign a list to the object.
 */
void
obj_assign_list(object_t *obj, const LIST *lp, int llen)
{
    __CUNUSED(llen)
    obj_clear(obj);
    obj->obj_rval = rlst_clone(lp);
    obj->obj_type = F_RLIST;
    ED_OBJTRACE(("obj_assign_list()"))
}


/*
 *  obj_donate_list ---
 *      Assign a newly created list to the object We are given the
 *      list so we don't need to allocate any more memory to it.
 */
void
obj_donate_list(object_t *obj, LIST *lp, int llen)
{
    obj_clear(obj);
    lst_check(lp);
    if (-1 == llen) {
        llen = lst_length(lp);
    } else {
        assert(llen == lst_length(lp));
    }
    obj->obj_rval = rlst_create(lp, llen);
    obj->obj_type = F_RLIST;
    ED_OBJTRACE(("obj_donate_list()"))
}


/*
 *  obj_assign_null ---
 *      Assign the NULL value to the object.
 */
void
obj_assign_null(object_t *obj)
{
    obj_clear(obj);
    obj->obj_type  = F_NULL;
    ED_OBJTRACE(("obj_assign_null()"))
}


/*
 *  obj_attributes ---
 *      Object attributes.
 */
uint32_t
obj_attributes(const object_t *obj)
{
    return obj->obj_attributes;
}


/*
 *  obj_get_type ---
 *      Return type of variable stored in object.
 */
OPCODE
obj_get_type(const object_t *obj)
{
#if (TODO)
    switch (obj->obj_type) {
    case OT_INT:
        return F_INT;
    case OT_FLOAT:
        return F_FLOAT;
    case OT_STR:
    case OT_LIT:
        return F_STR;
    case OT_LIST:
        return F_LIST;
    case OT_NULL:
        return F_NULL;
    default:
        break;
    }
    return F_DECLARE;
#endif
    return obj->obj_type;
}


/*
 *  obj_isnull ---
 *      Function to return whether NULL.
 */
int
obj_isnull(const object_t *obj)
{
    if (F_NULL == obj->obj_type) {
        return TRUE;
    }
    return FALSE;
}


/*
 *  obj_get_ival ---
 *      Return object integer value.
 */
accint_t
obj_get_ival(const object_t *obj)
{
    switch (obj->obj_type) {
    case F_INT:
        return obj->obj_ival;
    case F_FLOAT:
        return (accint_t)obj->obj_fval;
    default:
        break;
    }
    return 0;
}


/*
 *  obj_get_fval ---
 *      Return object float value.
 */
accfloat_t
obj_get_fval(const object_t *obj)
{
    switch (obj->obj_type) {
    case F_INT:
        return (accfloat_t)obj->obj_ival;
    case F_FLOAT:
        return obj->obj_fval;
    default:
        break;
    }
    return 0;
}


/*
 *  obj_get_sval ---
 *      Return pointer to string in object, otherwise NULL if not a string.
 */
const char *
obj_get_sval(const object_t *obj)
{
    switch (obj->obj_type) {
    case F_LIT:
        return obj->obj_lval;
    case F_STR:
        return obj->obj_sval;
    case F_RSTR:
        return r_ptr(obj->obj_rval);
    default:
        break;
    }
    return NULL;
}


/*
 *  obj_get_ref ---
 *      Return pointer to the reference value stored in the object.
 */
ref_t *
obj_get_ref(const object_t *obj)
{
    switch (obj->obj_type) {
    case F_RSTR:
    case F_RLIST:
        return obj->obj_rval;
    default:
        break;
    }
    return NULL;
}


/*
 *  obj_get_sbuf ---
 *      Retrieve address of modifiable buffer.
 */
char *
obj_get_sbuf(object_t *obj)
{
    switch (obj->obj_type) {
    case F_LIT:
        obj_assign_str(obj, obj->obj_lval, -1);
        return obj->obj_sval;
    case F_STR:
        return obj->obj_sval;
    case F_RSTR:
        if (1 == r_refs(obj->obj_rval)) {
            return r_ptr(obj->obj_rval);
        }
        obj_assign_str(obj, r_ptr(obj->obj_rval), r_used(obj->obj_rval));
        return obj->obj_sval;
    default:
        break;
    }
    return NULL;
}


/*
 *  obj_trace ---
 *      Trace assignments to object.
 */
void
obj_trace(const object_t *obj)
{
    /* Only trace object if debugging turned on and its value has changed */
    if (x_dflags)
        switch (obj->obj_type) {
        case F_INT:
            trace_ilog("  iOBJ=%" ACCINT_FMT "\n", obj->obj_ival);
            break;
        case F_FLOAT:
            trace_ilog("  fOBJ=%" ACCFLOAT_FMT "\n", obj->obj_fval);
            break;
        case F_LIT:
            trace_ilog("  litOBJ='%s'\n", c_string(obj->obj_lval));
            break;
        case F_STR:
            trace_ilog("  sOBJ='%s'\n", c_string(obj->obj_sval));
            break;
        case F_RSTR:
            trace_ilog("  rsOBJ%d='%s'\n", r_refs(obj->obj_rval), c_string(r_ptr(obj->obj_rval)));
            break;
        case F_RLIST:
            trace_ilog("  lOBJ=");
            trace_list((LIST *) r_ptr(obj->obj_rval));
            break;
        case F_NULL:
            trace_ilog("  NULL\n");
            break;
        default:
            panic("obj_trace: type ? (%d)", obj->obj_type);
            break;
        }
}

/*end*/

