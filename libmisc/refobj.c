#include <edidentifier.h>
__CIDENT_RCSID(gr_refobj_c,"$Id: refobj.c,v 1.30 2020/04/20 23:09:57 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: refobj.c,v 1.30 2020/04/20 23:09:57 cvsuser Exp $
 * Reference counted objects.
 *
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

#define EDREFOBJ_H_INTERNALS

#include <string.h>
#include <assert.h>

#include <vm_alloc.h>
#include <edrefobj.h>

static vmpool_t             hd_rstr;


ref_t *
r_string(const char *str)
{
    return r_init(F_STR, str, strlen(str));
}


ref_t *
r_nstring(const char *str, int len)
{
    assert(len >= 0);
    return r_init(F_STR, str, len);
}


ref_t *
r_list(const LIST *list, int len, refdelete_t func)
{
    return r_initx(F_LIST, list, len, func);
}


static ref_t *
new_rp(void)
{
    ref_t *rp;

    if (NULL != (rp = vm_alloc(&hd_rstr, sizeof(ref_t)))) {
        rp->r_magic = R_MAGIC;
    }
    return rp;
}


ref_t *
r_alloc(int size)
{
    ref_t *rp;

    if (NULL != (rp = new_rp())) {
        rp->r_ptr    = chk_calloc(size, 1);
        rp->r_refs   = 1;
        rp->r_type   = F_HALT;
        rp->r_used   = 0;
        rp->r_size   = size;
        rp->r_delete = NULL;
    }
    return rp;
}


/*
//  Function: r_init
//      Create a reference to a new object. 
//
//      Strings are allocated memory to be copied into, but lists are assumed as 
//      donated so only a reference is taken.
*/
ref_t *
r_init(OPCODE type, const void *object, int len)
{
    return r_initx(type, object, len, NULL);
}


ref_t *
r_initx(OPCODE type, const void *object, int len, refdelete_t func)
{
    ref_t *rp = new_rp();

    assert(F_LIST == type || F_STR == type);
    assert(len >= 0);

    if (rp) {
        if (F_LIST == type) {
            rp->r_ptr = (void *)object;
            rp->r_size = len;
            assert(func);                       /* required */

        } else {
            if (NULL == object || len <= 0) {
                char *ptr;

                if (NULL == (ptr = chk_calloc(rp->r_size = 16, 1))) {
                    rp->r_size = 0;
                    r_dec(rp);
                    return NULL;
                }
                rp->r_ptr = ptr;
                ptr[len = 0] = 0;

            } else {                            /* copy the buffer */
                char *ptr;

                if (NULL == (ptr = chk_alloc(len + 1))) {
                    r_dec(rp);
                    return NULL;
                }
                rp->r_ptr = ptr;
                rp->r_size = len + 1;
                memcpy(ptr, object, len);
                ptr[len] = 0;
            }
            assert(NULL == func);               /* not required */
        }
        rp->r_refs = 1;
        rp->r_used = len;
        rp->r_type = type;
        rp->r_delete = func;
    }
    return rp;
}


/*
//  Function: r_append
//      Append the specified data block 'mem' of size 'len' in bytes 
//      to the end of the object.
//        
//      The object shall be expanded to accommodate the storage of
//      'len' plus the additional storage of 'expansion' bytes, which
//      may be used to preallocate future needs reducing/avoiding
//      reallocation on later operations.
//
//  Returns:
//      Address of the possiblity new object.
*/
ref_t *
r_append(ref_t *rp, const char *mem, int len, int expansion)
{
    const int used = rp->r_used;
    const int newused = used + len;
    char *ptr;

    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_type == F_STR);
    assert(rp->r_refs >= 1);
    assert(NULL == rp->r_delete);
    assert(len >= 0);

    if (1 == rp->r_refs) {
        if (NULL == (ptr = rp->r_ptr) || rp->r_size <= (newused + 1)) {
            const int newsize =                 /* round to 32 bytes */
                ((newused + 1) + (expansion > 0 ? expansion : 32)) | 0x1f;

            if (NULL == (ptr = chk_alloc(newsize))) {
                return NULL;
            }
            memcpy(ptr, (const char *)rp->r_ptr, used);
            if (newsize > used) memset(ptr + used, 0, newsize - used);
            chk_free(rp->r_ptr);
            rp->r_ptr = ptr;
            rp->r_size = newsize;
        }

        if (len > 0) {
            if (mem) {
                memcpy(ptr + used, mem, len);
            } else {
                memset(ptr + used, 0, len);
            }
            rp->r_used += len;
        }   

        if (F_STR == rp->r_type) {
            ptr[newused] = 0;                   /* nul terminate */
        }

    } else {
        ref_t *rp1 = NULL;

        rp1 = r_alloc(newused + 1);

        rp1->r_refs = 1;
        rp1->r_used = newused;
        rp1->r_type = rp->r_type;

        ptr = rp1->r_ptr;
        memcpy(ptr, (const char *)rp->r_ptr, used);

        if (len > 0) {
            if (mem) {
                memcpy(ptr + used, mem, len);
            } else {
                memset(ptr + used, 0, len);
            }
        }

        if (F_STR == rp1->r_type) {
            ptr[newused] = 0;                   /* nul terminate */
        }

        r_dec(rp);                              /* free up the original object */
        rp = rp1;
    }
    return rp;
}


/*
//  Function: r_cat
//      Append the string 'str' to the end of the object.
*/
ref_t *
r_cat(ref_t *rp, const char *str)
{
    if (str) {
        rp = r_append(rp, str, strlen(str), 0);
    }
    return rp;
}


/*
//  Function: r_push
//      Push the specified data block 'mem' of size 'len' in bytes 
//      to the front of the object. 
//        
//      The object shall be expanded to accommodate the storage of
//      'len' plus the additional storage of 'expansion' bytes, which
//      may be used to preallocate future needs reducing/avoiding
//      reallocation on later operations.
//
//  Returns:
//      Address of the possiblity new object.
*/
ref_t *
r_push(ref_t *rp, const char *mem, int len, int expansion)
{
    const int used = rp->r_used;
    const int newused = used + len;
    char *ptr;

    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_type == F_STR);
    assert(rp->r_refs >= 1);
    assert(NULL == rp->r_delete);
    assert(len >= 0);

    if (1 == rp->r_refs) {
        if (NULL == (ptr = rp->r_ptr) || rp->r_size <= (newused + 1)) {
            const int newsize =                 /* round to 32 bytes */
                ((newused + 1) + (expansion > 0 ? expansion : 32)) | 0x1f;

            if (NULL == (ptr = chk_alloc(newsize))) {
                return NULL;
            }
            memcpy(ptr + len, (const char *)rp->r_ptr, used);
            if (newsize > newused) memset(ptr + newused, 0, newsize - newused);
            chk_free(rp->r_ptr);
            rp->r_ptr = ptr;
            rp->r_size = newsize;
        }

        if (len > 0)  {
            if (mem) {
                memcpy(ptr, mem, len);
            } else {
                memset(ptr, 0, len);
            }
            rp->r_used += len;
        }

        if (F_STR == rp->r_type) {
            ptr[newused] = 0;                   /* nul terminate */
        }

    } else {
        ref_t *rp1 = NULL;

        rp1 = r_alloc(newused + 1);

        rp1->r_refs = 1;
        rp1->r_used = newused;
        rp1->r_type = rp->r_type;

        ptr = rp1->r_ptr;
        memcpy(ptr + len, (const char *)rp->r_ptr, used);

        if (len > 0) {
            if (mem) {
                memcpy(ptr, mem, len);
            } else {
                memset(ptr, 0, len);
            }
        }

        if (F_STR == rp1->r_type) {
            ptr[newused] = 0;                   /* nul terminate */
        }

        r_dec(rp);                              /* free up the original object */
        rp = rp1;
    }
    return rp;
}


/*
//  Function: r_pop
//      Pop the specified number of bytes 'len' from the front of the object.
//
*/
ref_t *
r_pop(ref_t *rp, int len)
{
    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_type == F_STR);
    assert(rp->r_refs >= 1);
    assert(NULL == rp->r_delete);
    assert(len >= 1);
    assert(len <= rp->r_used);

    if (1 == rp->r_refs) {
        if (len < rp->r_used) {                 /* need. alt interface */
            rp->r_used -= len;
            memmove(rp->r_ptr, ((const char *)rp->r_ptr) + len, rp->r_used);
        } else {
            rp->r_used = 0;
        }
        return rp;
    }
    assert(0);                                  /* not supported */
    return NULL;
}


/*
//  Function: r_clear
//      Clear the object.
//
*/
void
r_clear(ref_t *rp)
{
    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_refs >= 1);
    if (F_STR == rp->r_type) {
        *((char *)rp->r_ptr) = 0;
    }
    rp->r_used = 0;
}


void
r_incused(ref_t *rp, int inc)
{
    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_refs >= 1);
    rp->r_used += inc;
    assert(F_LIST == rp->r_type || rp->r_used <= rp->r_size);
}


/*
//  r_inc ---
//      Increment reference count.
 */
ref_t *
r_inc(ref_t *rp)
{
    assert(R_MAGIC == rp->r_magic);
    assert(rp->r_refs >= 1);
    ++rp->r_refs;
    return rp;
}


/*
//  r_dec---
//      Decrement reference count, releasing resources up zero.
 */
ref_t *
r_dec(ref_t *rp)
{
    if (rp) {
        assert(R_MAGIC == rp->r_magic);
        assert(rp->r_refs >= 1);
        if (--rp->r_refs == 0) {
            if (rp->r_ptr) {
                if (rp->r_delete) {
                    (rp->r_delete)(rp);
                } else {
                    chk_free(rp->r_ptr);
                }
                rp->r_delete = NULL;
                rp->r_ptr = NULL;
            }
            rp->r_magic = (uint32_t)~R_MAGIC;   /* invalidate */
            vm_free(&hd_rstr, rp);
            rp = NULL;
        }
    }
    return rp;
}

/*end*/