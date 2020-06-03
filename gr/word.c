#include <edidentifier.h>
__CIDENT_RCSID(gr_word_c,"$Id: word.c,v 1.26 2020/05/03 21:41:54 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: word.c,v 1.26 2020/05/03 21:41:54 cvsuser Exp $
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

#if defined(WORD_INLINE)
#error Unexpected WORD_INLINE definition ...
#endif
#include "wordimpl.h"

const unsigned __CCACHEALIGN sizeof_atoms[] = {
    1,                      /* F_HALT   -- EOL */                   /* F_HALT   */
    CM_ATOMSIZE+1,          /* F_INT    -- Integer. */              /* F_INT    <integer> */
    1+8,                    /* F_FLOAT  -- Float */                 /* F_FLOAT  <stored in native format, 64-bit double precision IEEE 754> */
    CM_ATOMSIZE+1,          /* F_STR    -- Symbol/macro name. */    /* F_STR    <string> */ 
    CM_ATOMSIZE+1,          /* F_LIT    -- Literal string */        /* F_LIT    <string> */
    CM_ATOM_LIST_SZ,        /* F_LIST   */                          /* F_LIST   <length:16bit> ... */
    CM_ATOMSIZE+1,          /* F_ARRAY  */                          /* TODO     */
    1,                      /* F_NULL   */                          /* F_NULL   */
    CM_ATOMSIZE+1,          /* F_RSTR   -- String reference */      /* F_RSTR   <ptr> */
    CM_ATOMSIZE+1,          /* F_RLIST  -- List reference */        /* F_RLIST  <ptr> */
    CM_ATOMSIZE+1,          /* F_RARRAY -- Array reference */       /* F_RARRAY <ptr> */

    1+2,                    /* F_ID     -- Keyword/primitive. */    /* F_ID     <builtin:16bit> */
    CM_ATOMSIZE+1,          /* F_SYM    -- Symbol/macro name. */    /* F_SYM    <symbol> */
    CM_ATOMSIZE+2,          /* F_REG    -- Symbol + index. */       /* F_REG    <symbol> <idx> */
    };

const char * const nameof_atoms[] = {
    "HALT",                 /* F_HALT   */
    "INT",                  /* F_INT    */
    "FLOAT",                /* F_FLOAT  */
    "STR",                  /* F_STR    */
    "LIT",                  /* F_LIT    */
    "LIST",                 /* F_LIST   */
    "ARRAY",                /* F_ARRAY  */
    "NULL",                 /* F_NULL   */
    "END",                  /* F_END    */
    "RSTR",                 /* F_RSTR   */
    "RLIST",                /* F_RLIST  */
    "RARRAY",               /* F_RARRAY */

    "ID",                   /* F_ID     */
    "SYM",                  /* F_SYM    */
    "REG"                   /* F_REG    */
    };

static int16_t              one       = 1;
static const char *         onep      = (const char *) &one;
static uint8_t              one234[4] = {0x01, 0x02, 0x03, 0x04};


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
    union word_double_int32 uval = {{0}};
    uint32_t l = *(uint32_t *) one234;

    assert(8 == sizeof(uval));
    assert(4 == sizeof(one234));
    assert(sizeof(double) == sizeof(union word_double_int32));

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
