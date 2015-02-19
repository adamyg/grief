#ifndef GR_OBJECT_H_INCLUDED
#define GR_OBJECT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_object_h,"$Id: object.h,v 1.8 2014/10/22 02:33:14 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: object.h,v 1.8 2014/10/22 02:33:14 ayoung Exp $
 * Generic Object.
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

#include <edtypes.h>                            /* accxxx_t */
#include <edopcode.h>                           /* OPCODE and LIST */

/*
 *  Base Object types
 *
 *      T_NULL          Undefined/NIL.
 *      T_INT           accint_t.
 *      T_FLOAT         accfloat_t.
 *      T_LIT           Literal string.
 *      T_STR           String.
 *      T_RSTR          String reference.
 *      T_RLIST         List reference.
 *      T_ROBJECT       Object reference.
 */
typedef enum {                                  /* base-types */
    T_NULL,
    T_INT,
    T_FLOAT,
    T_LIT,
    T_STR,
    T_RSTR,
    T_RLIST,
    T_OBJECT
} OBJBASE_t;

typedef enum {
    OBJECT_FREADONLY = 0x0001                   /* read-only */
} OBJATTRS_t;


/*
 *  Following data structures used to hold the current value of the object.
 */
#if defined(OBJECT_H_INTERNALS)
typedef struct object {
    MAGIC_t             obj_magic;              /* structure magic */
#define OBJECT_MAGIC        MKMAGIC('O','b','j','t')
    OPCODE              obj_type;               /* current object type */
    uint32_t            obj_length;             /* length of composite object */
    uint32_t            obj_attributes;         /* attributes */
    char *              obj_memptr;             /* storage base */

    /*
     *  TODO - compress object
     *      obj_attributes :    16
     *      obj_type :          16
     */
    union {
        accint_t        _int_val;               /* F_INT */
        accfloat_t      _float_val;             /* F_FLOAT */
        const char     *_lit_val;               /* F_LIT */
        char *          _str_val;               /* F_STR */
        ref_t *         _ref_val;               /* F_RSTR || F_LIST */
    } obj_val;

#define obj_ival        obj_val._int_val
#define obj_fval        obj_val._float_val
#define obj_lval        obj_val._lit_val
#define obj_sval        obj_val._str_val
#define obj_rval        obj_val._ref_val
} object_t;

#else
typedef struct object object_t;                 /* opaque */
#endif

extern object_t *           obj_alloc(void);
extern object_t *           obj_copy(const object_t *copy);
extern void                 obj_free(object_t *obj);
extern void                 obj_init(object_t *obj);
extern void                 obj_clear(object_t *obj);
extern void                 obj_zap(object_t *obj);
extern void *               obj_expand(object_t *obj, int len);

extern void                 obj_assign(object_t *obj, const object_t *copy);
extern void                 obj_assign_argv(object_t *obj, const LISTV *lvp);
extern void                 obj_assign_str(object_t *obj, const char *str, int len);
extern void                 obj_assign_str2(object_t *obj, const char *str1, int len1, const char *str2, int len2);
extern void                 obj_assign_strcat(object_t *obj, const char *str1, int len1, const char *str2, int len2);
extern void                 obj_assign_lit(object_t *obj, const char *str);
extern void                 obj_assign_ref(object_t *obj, ref_t *rp);
extern void                 obj_assign_list(object_t *obj, const LIST *lp, int len);
extern void                 obj_donate_list(object_t *obj, LIST *lp, int len);
extern void                 obj_assign_null(object_t *obj);
extern void                 obj_assign_int(object_t *obj, accint_t val);
extern void                 obj_assign_float(object_t *obj, accfloat_t val);

extern uint32_t             obj_attributes(const object_t *obj);
extern int                  obj_isnull(const object_t *obj);
extern OBJBASE_t            obj_base(const object_t *obj);
extern OPCODE               obj_get_type(const object_t *obj);
extern accint_t             obj_get_ival(const object_t *obj);
extern accfloat_t           obj_get_fval(const object_t *obj);
extern const char *         obj_get_sval(const object_t *obj);
extern ref_t *              obj_get_ref(const object_t *obj);
extern char *               obj_get_sbuf(object_t *obj);

extern void                 obj_trace(const object_t *obj);

extern void                 acc_assign_object(object_t *obj);

#endif /*GR_OBJECT_H_INCLUDED*/
