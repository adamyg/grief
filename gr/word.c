#include <edidentifier.h>
__CIDENT_RCSID(gr_word_c,"$Id: word.c,v 1.22 2015/02/19 22:11:05 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: word.c,v 1.22 2015/02/19 22:11:05 ayoung Exp $
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

#include <editor.h>
#include <edendian.h>
#include <edcm.h>

#include "word.h"
#include "system.h"

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
#if (SIZEOF_LONG != SIZEOF_VOID_P)
#error unsupported sizeof(long)
#endif
#if (SIZEOF_DOUBLE != 8)
#error unsupported sizeof(double)
#endif

#define ATOMSIZE           SIZEOF_LONG

const unsigned sizeof_atoms[] = {
    1,                      /* F_HALT  */
    ATOMSIZE+1,             /* F_INT   -- integer. */
    ATOMSIZE+1,             /* F_STR   -- symbol/macro name. */
    3,                      /* F_LIST  */
    1,                      /* F_NULL  */
    3,                      /* F_ID    -- keyword/primitive. */
    1,                      /* F_END   */
    1,                      /* F_POLY  */
    ATOMSIZE+1,             /* F_LIT   -- literal string */
    ATOMSIZE+1,             /* F_RSTR  -- string reference */
    9,                      /* F_FLOAT -- stored in native format. */
    ATOMSIZE+1,             /* F_RLIST */
    };

const char * nameof_atoms[] = {
    "HALT",                 /* F_HALT  */
    "INT",                  /* F_INT   -- integer. */
    "STR",                  /* F_STR   -- symbol/macro name. */
    "LIST",                 /* F_LIST  */
    "NULL",                 /* F_NULL  */
    "ID",                   /* F_ID    -- keyword/primitive. */
    "END",                  /* F_END   */
    "POLY",                 /* F_POLY  */
    "LIT",                  /* F_LIT   -- literal string */
    "RSTR",                 /* F_RSTR  -- string reference */
    "FLOAT",                /* F_FLOAT -- stored in native format. */
    "RLIST"                 /* F_RLIST */
    };

static int16_t              one       = 1;
static const char *         onep      = (const char *) &one;
static uint8_t              one234[4] = {0x01, 0x02, 0x03, 0x04};

union double_int32 {
    uint32_t    i[2];
    double      d;
};


void
cm_xdr_import(struct CM *cm)
{
    cm->cm_magic        = WGET16(cm->cm_magic);
    cm->cm_version      = WGET16(cm->cm_version);
    cm->cm_builtin      = WGET32(cm->cm_builtin);
    cm->cm_signature    = WGET32(cm->cm_signature);
    cm->cm_num_macros   = WGET32(cm->cm_num_macros);
    cm->cm_num_atoms    = WGET32(cm->cm_num_atoms);
    cm->cm_globals      = WGET32(cm->cm_globals);
    cm->cm_num_globals  = WGET32(cm->cm_num_globals);
    cm->cm_num_strings  = WGET32(cm->cm_num_strings);
}


void
cm_xdr_export(struct CM *cm)
{
    cm->cm_magic        = WPUT16(cm->cm_magic);
    cm->cm_version      = WPUT16(cm->cm_version);
    cm->cm_builtin      = WPUT32(cm->cm_builtin);
    cm->cm_signature    = WPUT32(cm->cm_signature);
    cm->cm_num_macros   = WPUT32(cm->cm_num_macros);
    cm->cm_num_atoms    = WPUT32(cm->cm_num_atoms);
    cm->cm_globals      = WPUT32(cm->cm_globals);
    cm->cm_num_globals  = WPUT32(cm->cm_num_globals);
    cm->cm_num_strings  = WPUT32(cm->cm_num_strings);
}


uint16_t
WPUT16(const uint16_t n)
{
    return WGET16(n);
}


uint16_t
WGET16(const uint16_t n)
{
    if (0 == *onep) {
        return n;
    }
    return (uint16_t)((n >> 8) & 0xff) | (uint16_t)((n & 0xff) << 8);
}


uint32_t
WPUT32(const uint32_t n)
{
    return WGET32(n);
}


uint32_t
WGET32(const uint32_t n)
{
    union double_int32 uval = {{0}};
    uint32_t l = *(uint32_t *) one234;

    assert(8 == sizeof(uval));
    assert(4 == sizeof(one234));
    assert(sizeof(double) == sizeof(union double_int32));

    if (sizeof(void *) != sizeof(accint_t)) {
        printf("WGET32: panic, sizeof(void *) not sizeof(accint_t)\n");
        sys_abort();
    }

    /*
     *  LITTLE_ENDIAN       1234        LSB first: i386, vax.
     *  BIG_ENDIAN          4321        MSB first: 68000, ibm, net.
     *  PDP_ENDIAN          3412        LSB first in word, MSW first in long.
     */
    if (0x01020304 == l) {
        return n;
    }

    if (0x04030201 == l) {
        uint8_t buf[4];

        *((uint32_t *) buf) = n;
        return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) | (buf[2] << 8) | buf[3];
    }

    printf("WGET32: l=%08lx, what?\n", (unsigned long)l);
    sys_abort();
    /*NOTREACHED*/
    return 0;
}


void
WGET32_block(uint32_t *warray, int size)
{
    register uint32_t *words = warray;
    register int i;

    for (i = 0; i < size; ++i, ++words) {
        *words = WGET32(*words);
    }
}


void
LPUT16(LIST *lp, register const int16_t n)
{
    register uint8_t *cp = (uint8_t *) lp;

    *++cp = (uint8_t) (n >> 8);
    *++cp = (uint8_t) n;
}


int16_t
LGET16(register const LIST *lp)
{
    return (((uint16_t)lp[1] << 8) |
            ((uint16_t)lp[2]));
}


void
LPUT32(LIST *lp, register const int32_t n)
{
    register uint8_t *cp = (uint8_t *) lp;

    *++cp = (uint8_t) (n >> (8*3));
    *++cp = (uint8_t) (n >> (8*2));
    *++cp = (uint8_t) (n >> (8*1));
    *++cp = (uint8_t) n;
}


int32_t
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
void
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
int64_t
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


void
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


void *
LGET_PTR(register const LIST *lp)
{
    register const uint8_t *cp = (const uint8_t *)lp;
#if (SIZEOF_VOID_P == 8)
    register uint64_t n =
            (((uint64_t) cp[1] << (8*7)) |
             ((uint64_t) cp[2] << (8*6)) |
             ((uint64_t) cp[2] << (8*5)) |
             ((uint64_t) cp[3] << (8*4)) |
             ((uint64_t) cp[4] << (8*3)) |
             ((uint64_t) cp[5] << (8*2)) |
             ((uint64_t) cp[6] << (8*1)) |
             ((uint64_t) cp[7]));

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
 *  Routine to read a 64-bit double precision floating point number.
 *
 *  We use LGET32() to preserve byte orderedness amongst machines, but
 *  we don't have a generic mapping from one FP representation to another.
 */
double
LGET_FLOAT(const LIST *lp)
{
    union double_int32 uval;

#if defined(HOST_BIG_ENDIAN) || \
        (defined(__arm__) && !defined(__VFP_FP__) && !defined(__MAVERICK__))    /*not IEEE-754*/
    uval.i[0] = LGET32(lp);
    lp += 4;
    uval.i[1] = LGET32(lp);

#else
    uval.i[1] = LGET32(lp);
    lp += 4;
    uval.i[0] = LGET32(lp);
#endif
    return uval.d;
}


/*
 *  Function to store a float in a hopefully machine independent manner.
 */
void
#if defined(__GNUC__)
LPUT_FLOAT(LIST *lp, const volatile double val)
#else
LPUT_FLOAT(LIST *lp, const double val)
#endif
{
    union double_int32 uval;

    uval.d = val;

#if defined(HOST_BIG_ENDIAN) || \
        (defined(__arm__) && !defined(__VFP_FP__) && !defined(__MAVERICK__))    /*not IEEE-754*/
    LPUT32(lp, uval.i[0]);
    lp += 4;
    LPUT32(lp, uval.i[1]);

#else
    LPUT32(lp, uval.i[1]);
    lp += 4;
    LPUT32(lp, uval.i[0]);
#endif
}


/*  If porting to a new system, the following:
 *      a.out 01020304
 *
 *  should generate:
 *      03 04 01 02 03 04
 *
 *  as output in order for the .cm files to be portable across to other CPU types.
 */
#if defined(LOCAL_MAIN)
int
main(int argc, char **argv)
{
    static const double doubles[] = {
                0, 1, -1, 1.234, 2.345
                };
    LIST list[32];
    double d;
    int32_t i;
    int j;

    if (argc > 1) {
        char buf[6];
        int32_t i32;
        int16_t i16;

        sscanf(argv[1], "%x", &i32);
        i16 = (i32 & 0xffff);

        *((int16_t *) buf) = WGET16(i16);
        *((int32_t *) (buf + 2)) = WGET32(i32);

        for (j = 0; j < 6; ++j) {
            printf("%c", buf[j]);
        }
    }

    LPUT32(list, 1234);
    assert(1234 == (i = LGET32(list)));
    printf("int32:  %d\n", i);

    for (j = 0; j < (sizeof(doubles)/sizeof(doubles[0])); ++j) {
        LPUT_FLOAT(list, doubles[j]);
        assert(doubles[j] == (d = LGET_FLOAT(list)));
        printf("double: %g\n", d);
    }
    return 0;
}
#endif  /*LOCAL_MAIN*/
