#include <edidentifier.h>
__CIDENT_RCSID(gr_wordimpl_inc,"$Id: wordimpl.h,v 1.5 2025/01/10 16:51:45 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: wordimpl.h,v 1.5 2025/01/10 16:51:45 cvsuser Exp $
 * Portable mappings to and from internal word and byte order.
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

#include <edsym.h>
#include <edendian.h>

#if defined(HOST_BIG_ENDIAN)
#define DEFENDIAN           1
#elif defined(HOST_LITTLE_ENDIAN)
#define DEFENDIAN           0
#else
#error Unsupported target architecture ...
#endif

#if (SIZEOF_VOID_P != 4 && SIZEOF_VOID_P != 8)
#error unsupported sizeof(void *)
#endif
    // #if (SIZEOF_LONG != SIZEOF_VOID_P)
    // #error unsupported sizeof(long)
    // #endif
#if (SIZEOF_DOUBLE != 8)
#error unsupported sizeof(double)
#endif

#if defined(WORDIMPL_INLINE)
#define WORD_DECL static __CFORCEINLINE
#else
#define WORD_DECL
#endif

union word_double_int32 {
    uint32_t    i[2];
    double      d;
};

WORD_DECL void
LPUT16(LIST *lp, register const uint16_t n)
{
    register uint8_t *cp = (uint8_t *) lp;

    *++cp = (uint8_t) (n >> 8);
    *++cp = (uint8_t) n;
}


WORD_DECL uint16_t
LGET16(register const LIST *lp)
{
    return (((uint16_t)lp[1] << 8) |
            ((uint16_t)lp[2]));
}


WORD_DECL void
LPUT32(LIST *lp, register const int32_t n)
{
    register uint8_t *cp = (uint8_t *) lp;

    *++cp = (uint8_t) (n >> (8*3));
    *++cp = (uint8_t) (n >> (8*2));
    *++cp = (uint8_t) (n >> (8*1));
    *++cp = (uint8_t) n;
}


WORD_DECL int32_t
LGET32(register const LIST *lp)
{
    register const uint8_t *cp = (const uint8_t *) lp;

    return (((int32_t) cp[1] << (8*3)) |
            ((int32_t) cp[2] << (8*2)) |
            ((int32_t) cp[3] << (8*1)) |
            ((int32_t) cp[4]));
}


#if (defined(HAVE_LONG_LONG_INT) && (SIZEOF_LONG_LONG >= 8)) || \
        (SIZEOF_INT >= 8)
WORD_DECL void
LPUT64(LIST *lp, register const int64_t n)
{
    register uint8_t *cp = (uint8_t *) lp;

    *++cp = (uint8_t) (n >> (8*7));
    *++cp = (uint8_t) (n >> (8*6));
    *++cp = (uint8_t) (n >> (8*5));
    *++cp = (uint8_t) (n >> (8*4));
    *++cp = (uint8_t) (n >> (8*3));
    *++cp = (uint8_t) (n >> (8*2));
    *++cp = (uint8_t) (n >> (8*1));
    *++cp = (uint8_t) n;
}
#endif /*64bit*/


#if (defined(HAVE_LONG_LONG_INT) && (SIZEOF_LONG_LONG >= 8)) || \
        (SIZEOF_INT >= 8)
WORD_DECL int64_t
LGET64(register const LIST *lp)
{
    register const uint8_t *cp = (const uint8_t *) lp;

    return (((int64_t) cp[1] << (8*7)) |
            ((int64_t) cp[2] << (8*6)) |
            ((int64_t) cp[3] << (8*5)) |
            ((int64_t) cp[4] << (8*4)) |
            ((int64_t) cp[5] << (8*3)) |
            ((int64_t) cp[6] << (8*2)) |
            ((int64_t) cp[7] << (8*1)) |
            ((int64_t) cp[8]));
}
#endif /*64BIT*/


WORD_DECL void
LPUT_PTR(LIST *lp, const void *p)
{
    register uint8_t *cp = (uint8_t *)lp;
#if (SIZEOF_VOID_P == 8)
    register uint64_t n = (uint64_t)p;
    *++cp = (uint8_t) (n >> (8*7));
    *++cp = (uint8_t) (n >> (8*6));
    *++cp = (uint8_t) (n >> (8*5));
    *++cp = (uint8_t) (n >> (8*4));
    *++cp = (uint8_t) (n >> (8*3));
    *++cp = (uint8_t) (n >> (8*2));
    *++cp = (uint8_t) (n >> (8*1));
    *++cp = (uint8_t) n;

#elif (SIZEOF_VOID_P == 4)
    register uint32_t n = (uint32_t)p;
    *++cp = (uint8_t) (n >> (8*3));
    *++cp = (uint8_t) (n >> (8*2));
    *++cp = (uint8_t) (n >> (8*1));
    *++cp = (uint8_t) n;

#else
#error LPUT_PTR: missing implementation
#endif
}


WORD_DECL void *
LGET_PTR(register const LIST *lp)
{
    register const uint8_t *cp = (const uint8_t *)lp;
#if (SIZEOF_VOID_P == 8)
    register uint64_t n =
            (((uint64_t) cp[1] << (8*7)) |
             ((uint64_t) cp[2] << (8*6)) |
             ((uint64_t) cp[3] << (8*5)) |
             ((uint64_t) cp[4] << (8*4)) |
             ((uint64_t) cp[5] << (8*3)) |
             ((uint64_t) cp[6] << (8*2)) |
             ((uint64_t) cp[7] << (8*1)) |
             ((uint64_t) cp[8]));

#elif (SIZEOF_VOID_P == 4)
    register uint32_t n =
            (((uint32_t) cp[1] << (8*3)) |
             ((uint32_t) cp[2] << (8*2)) |
             ((uint32_t) cp[3] << (8*1)) |
             ((uint32_t) cp[4]));

#else
#error LGET_PTR: missing implementation
#endif
    return ((void *)n);
}


/*
 *  Retreive a 64-bit double precision floating point number.
 *
 *  Note: Uuse LGET32() to preserve byte orderedness amongst machines, but
 *    we don't have a generic mapping from one FP representation to another.
 */
WORD_DECL double
LGET_FLOAT(const LIST *lp)
{
    union word_double_int32 uval;

#if defined(HOST_BIG_ENDIAN) || \
        (defined(__arm__) && !defined(__VFP_FP__) && !defined(__MAVERICK__))    /*not IEEE-754*/
    uval.i[0] = LGET32(lp);
    uval.i[1] = LGET32(lp + 4);
#else
    uval.i[1] = LGET32(lp);
    uval.i[0] = LGET32(lp + 4);
#endif
    return uval.d;
}


/*
 *  Store a 64-bit double precision floating point number.
 */
WORD_DECL void
#if defined(__GNUC__)
LPUT_FLOAT(LIST *lp, const volatile double val)
#else
LPUT_FLOAT(LIST *lp, const double val)
#endif
{
    union word_double_int32 uval;

    uval.d = val;

#if defined(HOST_BIG_ENDIAN) || \
        (defined(__arm__) && !defined(__VFP_FP__) && !defined(__MAVERICK__))    /*not IEEE-754*/
    LPUT32(lp, uval.i[0]);
    LPUT32(lp + 4, uval.i[1]);

#else
    LPUT32(lp, uval.i[1]);
    LPUT32(lp + 4, uval.i[0]);
#endif
}

/*end*/
