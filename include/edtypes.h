#ifndef GR_EDTYPES_H_INCLUDED
#define GR_EDTYPES_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edtypes_h,"$Id: edtypes.h,v 1.40 2022/05/31 16:18:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edtypes.h,v 1.40 2022/05/31 16:18:22 cvsuser Exp $
 * Editor base types.
 *
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#if !defined(HAVE_CONFIG_H)
#error HAVE_CONFIG_H expected to be defined ...
#endif
#include <config.h>

#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>                          /* off_t */
#endif
#if defined(HAVE_FCNTL_H)
#include <fcntl.h>                              /* off_t */
#endif

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif
#if defined(HAVE_STDINT_H)
#include <stdint.h>                             /* [u]int8_t, [u]int16_t, [u]int32_t optional [u]int64_t */
#endif
#if defined(HAVE_STDBOOL_H)
#include <stdbool.h>                            /* bool, true, false */
#endif

#if defined(STDC_HEADERS)
#include <limits.h>                             /* LONG_MAX?MIN */
#endif
#if defined(STDC_HEADERS) || defined(HAVE_FLOAT_H)
#include <float.h>                              /* DBL_MAX/MIN */
#else
#include <math.h>
#endif

#include <edatom.h>

#if defined(_BSD_SOURCE)
#if !defined(_BSDTYPES_DEFINED)
#if !defined(u_char)
typedef unsigned char u_char;
#endif
#if !defined(u_short)
typedef unsigned short u_short;
#endif
typedef unsigned int u_int;
typedef unsigned long u_long;
#define _BSDTYPES_DEFINED                       /* winsock[2].h */
#endif /*!_BSDTYPES_DEFINE*/
#endif /*_BSD_SOURCE*/

#ifndef VSIZEOF                                 /* vector sizing */
#define VSIZEOF(__type)         (sizeof(__type)/sizeof(__type[0]))
#endif

#ifndef _MSC_VER
#ifndef _countof
#define _countof(__type)        (sizeof(__type)/sizeof(__type[0]))
#endif
#endif


/*
 *  Useful type definitions
 *
 *    config.h
 *
 *        int8_t, int16_t and int32_t
 *        uint8_t, uint16_t and uint32_t
 *
 *    plus the following alternatives
 *
 *        int8, int16, int32
 *        u_int8, u_int16 and u_int32
 */

#if !defined(int8)
#define int8                    int8_t
#endif
#if !defined(int16)
#define int16                   int16_t
#endif
#if !defined(int32)
#define int32                   int32_t
#endif

#if !defined(u_int8)
#define u_int8                  uint8_t
#endif
#if !defined(u_int16)
#define u_int16                 uint16_t
#endif
#if !defined(u_int32)
#define u_int32                 uint32_t
#endif

#define __UNUSED(x)             (void)x;
#ifndef __CUNUSED
#define __CUNUSED(x)            (void)x;
#endif

typedef unsigned char uchar_t;
typedef unsigned int uint_t;
typedef off_t FSIZE_t;                          /* file/region sizes */
typedef int32_t KEY;                            /* internal keystrokes */

#define LINEMAX                 0x7fffffff

typedef int32_t LINENO;                         /* line number management */
typedef int32_t IDENTIFIER_t;                   /* general identifier, buffer/windows etc */

typedef unsigned char LINECHAR;                 /* line number management */
typedef unsigned char LINEATTR;                 /* line attribute management */


/*
 *  prototype/language support,
 *
 *      __CINLINE
 *      __CCACHEALIGN
 *      __CRESTRICT
 *      __CUNUSEDARGUMENT
 *          Example:
 *              void foo(int __CUNUSEDARGUMENT(bar)) { ... }
 *
 *      __CEXPORT
 *              void __CEXPORT function() { }
 *
 *      __CUNUSEDFUNCTION
 *          Example:
 *              static void __CUNUSEDFUNCTION(foo)(int bar) { ... }
 *
 *      __CPUBLIC
 *
 *      __CFUNCTION__
 *
 *      __CBEGIN_DECLS
 *
 *      __CEND_DECLS
 *
 *      __CSTRINGIZE
 *
 *      __CSTRCAT
 *
 *      __ATTRIBUTE_FORMAT__
 *
 *          Usage:  __ATTRIBUTE_FORMAT__((interface, fmt, args)
 *
 *              interface -     Interface function. e.g printf.
 *              fmt -           Index of the format specification.
 *              args -          Argument index.
 *
 *          Example:
 *              trace(const char *fmt, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
 *
 *      __ATTRIBUTE_CHECKRETURN__
 *          Example:
 *              __ATTRIBUTE_CHECKRETURN__ int foo() { ... }
 *
 *      __ATTRIBUTE_NORETURN__
 *          Example:
 *              __ATTRIBUTE_NORETURN__ void foo() { ... }
 *
 *      __CIFDEBUG( .. debug declarations )
 */

#if !defined(___CINLINE)
#if defined(HAVE_INLINE)
# if defined(_MSC_VER) /*FIXME: override*/
#define __CINLINE               __inline
#if defined(HAVE___FORCEINLINE)
#define __CFORCEINLINE          __forceinline
#endif
# else
#define __CINLINE               inline
# endif
#elif defined(HAVE___INLINE)
#define __CINLINE               __inline
#elif (__STDC_VERSION__ >= 199901L)             /* C99 or better */
#define __CINLINE               inline
#elif defined(_MSC_VER)
#define __CINLINE               __inline
#if defined(HAVE___FORCEINLINE)
#define __CFORCELINE            __forceline
#endif
#elif defined(__WATCOMC__)
#define __CINLINE               __inline
#elif defined(__GNUC__)
#define __CINLINE               __inline
#elif defined(__SUNPRO_C) && defined(__C99FEATURES__)
#define __CINLINE               inline
#else
#define __CINLINE
#endif
#endif  /*___CINLINE*/
#if !defined(__CFORCEINLINE)
#define __CFORCEINLINE          __CINLINE
#endif

#if !defined(__CCACHEALIGN)
// XXX: 64 bytes for x86 CPUs and 128 bytes for ARMs.
#if defined(__GNUC__)                           /* clang and GCC */
#define __CCACHEALIGN           __attribute__((aligned(64)))
#elif defined(_MSC_VER)                         /* MSVC */
#define __CCACHEALIGN           __declspec(align(64))
#else
#define __CCACHEALIGN
#endif
#endif /*_CCACHEALIGNED*/

#if !defined(__CEXPORT)
#   if defined(__CSTATICLIB)                    /* building static library */
#       define __CEXPORT        /**/
#   else
#       if !defined(__CIMPORTLIB) && !defined(__CEXPORTLIB)
#           define __CEXPORTLIB                 /* default, export symbols */
#       endif
#       if defined(__CEXPORTLIB)                /* export marked symbols */
#           if defined(__GNUC__)
#               define __CEXPORT __attribute__((dllexport)) extern
#           else
#               define __CEXPORT extern __declspec(dllexport)
#           endif
#       else
#           if defined(__GNUC__)
#               define __CEXPORT /**/
#           else
#               define __CEXPORT extern __declspec(dllimport)
#           endif
#       endif   /*__CIMPORT*/
#   endif
#endif  /*__CEXPORT*/

#if !defined(___CBOOL)
#if defined(HAVE_BOOL) || defined(HAVE_STDBOOL) || \
        defined(__bool_true_false_are_defined)  /* C99 or better */
#define __CBOOL bool
#elif defined(HAVE__BOOL)
#define __CBOOL _bool
#else
#define __CBOOL int
# if !defined(true)
#define true 1
#define false 0
# endif
#endif
#endif  /*__CBOOL*/

#if !defined(___CRESTRICT)
#if defined(HAVE_RESTRICT)
#define __CRESTRICT             restrict
#elif (__STDC_VERSION__ >= 199901L)             /* C99 or better */
#define __CRESTRICT             restrict
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
#define __CRESTRICT             __restrict
#elif defined(__WATCOMC__)
#define __CRESTRICT             __restrict
#elif __GNUC__
#define __CRESTRICT             __restrict
#else
#define __CRESTRICT
#endif
#endif  /*___CRESTRICT*/

/* UNUSED macro, for function argument */
#if !defined(__CUNUSEDARGUMENT)
#ifdef __GNUC__
#define __CUNUSEDARGUMENT(x)    UNUSED_ ## x __attribute__((__unused__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#define __CUNUSEDARGUMENT(x)    __pragma(warning(suppress:4100)) UNUSED_ ## x
#else
#define __CUNUSEDARGUMENT(x)    UNUSED_ ## x
#endif
#endif  /*__CUNUSEDARGUMENT*/

#if !defined(__CUNUSEDFUNCTION)
#ifdef __GNUC__
#define __CUNUSEDFUNCTION(x)    __attribute__((__unused__)) UNUSED_ ## x
#else
#define __CUNUSEDFUNCTION(x)    UNUSED_ ## x
#endif
#endif  /*__CUNUSEDFUNCTION*/

#if !defined(__CPUBLIC)
#if defined(__GNUC__) || (defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590))
#define __CPUBLIC               __attribute__((visibility("default")))
#elif defined(_MSC_VER) || defined(__WATCOMC__)
#define __CPUBLIC               __declspec(dllexport)
#else
#define __CPUBLIC
#endif
#endif  /*___CPUBLIC*/

#if !defined(__CFUNCTION__)
#if defined(__GNUC__) || defined(__FUNCTION__)
#define __CFUNCTION__           __FUNCTION__
#elif defined(__VMS)
#define __CFUNCTION__           "VMS$NL:"
#elif (!defined __GNUC__) && (!defined __xlC__) && \
            (!defined(_MSC_VER) || _MSC_VER < 1300)
#if (__STDC_VERSION__ >= 199901L) /* C99 */ || \
            (defined(__SUNPRO_C) && defined(__C99FEATURES__))
#define __CFUNCTION__           __func__
#endif
#endif
#if !defined(__CFUNCTION__)
#define __CFUNCTION__           "<unknown>"
#endif
#endif  /*__CFUNCTION__*/

#ifndef __CBEGIN_DECLS
#ifdef  __cplusplus
#define __CBEGIN_DECLS          extern "C" {
#define __CEND_DECLS            };
#else
#define __CBEGIN_DECLS
#define __CEND_DECLS
#endif
#endif  /*__BEGIN_DECLS*/

#ifndef __CSTRINGIZE
#define ____CSTRINGIZE(s)       #s
#define __CSTRINGIZE(s)         ____CSTRINGIZE(s)
#endif

#ifndef __CSTRCAT
#if defined(__STDC__) && !defined(__GNUC__)
#define ____CSTRCAT(a,b)        a##b
#else
#define ____CSTRCAT(a,b)        a/**/b
#endif
#define __CSTRCAT(a,b)          ____CSTRCAT(a,b)
#endif

#ifndef __ATTRIBUTE_FORMAT__
#if defined(__GNUC__)
#define __ATTRIBUTE_FORMAT__(__x) __attribute__((format __x))
#else
#define __ATTRIBUTE_FORMAT__(__x)
#endif
#endif

#ifndef __ATTRIBUTE_NONNULL__
#if defined(__GNUC__)
#define __ATTRIBUTE_NONNULL__(__x) __attribute__((__nonnull__ __x))
#else
#define __ATTRIBUTE_NONNULL__(__x)
#endif
#endif

#ifndef __ATTRIBUTE_CHECKRETURN__
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define __ATTRIBUTE_CHECKRETURN__ __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
#define __ATTRIBUTE_CHECKRETURN__ _Check_return_
#else
#define __ATTRIBUTE_CHECKRETURN__
#endif
#endif  /*__ATTRIBUTE_CHECKRETURN__*/

#ifndef __ATTRIBUTE_NORETURN__
#if defined(__GNUC__)
#define __ATTRIBUTE_NORETURN__ __attribute__((__noreturn__))
//  #elif defined(_MSC_VER)
//  #define __ATTRIBUTE_NORETURN__ __declspec(noreturn)
#else
#define __ATTRIBUTE_NORETURN__
#endif
#endif  /*__ATTRIBUTE_NORETURN__*/

#ifndef __ATTRIBUTE_ERROR__
#if defined(__GNUC__)
#define __ATTRIBUTE_ERROR__(__x) __attribute__((__error__ __x))
#else
#define __ATTRIBUTE_ERROR__
#endif
#endif

#ifndef __ATTRIBUTE_DEPRECATED__
#if defined(__GNUC__)
#define __ATTRIBUTE_DEPRECATED__(__x) __attribute__((deprecated __x))
#else
#define __ATTRIBUTE_DEPRECATED__(__x)
#endif
#endif

#if defined(NDEBUG)
#define __CIFDEBUG(__x)
#else
#define __CIFDEBUG(__x) __x                     /* DEBUG only code/declarations */
#endif


/*
 *  accumulator base types
 */
#define ACCINT_SIZEOF           CM_ATOMSIZE

#if (CM_ATOMSIZE == SIZEOF_LONG)
#define ACCINT_FMT              "ld"
#if defined(LONG_MIN)
#define ACCINT_MIN              LONG_MIN
#define ACCINT_MAX              LONG_MAX
#else                                           /* old school */
#define ACCINT_MIN              MAXLONG
#define ACCINT_MAX              MINLONG
#endif
#elif (SIZEOF_LONG_LONG == SIZEOF_VOID_P)
#define ACCINT_FMT              "lld"
#define ACCINT_MIN              LLONG_MIN
#define ACCINT_MAX              LLONG_MAX
#else
#error unable to determine atom size format/range.
#endif

#define ACCFLOAT_SIZEOF         SIZEOF_DOUBLE
#define ACCFLOAT_FMT            "g"             /* dynamic 'e' or 'f' */
#if defined(DBL_MAX)
#define ACCFLOAT_MIN            DBL_MIN
#define ACCFLOAT_MAX            DBL_MAX
#elif defined(MAXDOUBLE)
#define ACCFLOAT_MIN            MINDOUBLE
#define ACCFLOAT_MAX            MAXDOUBLE
#elif defined(FLT_MAX)
#define ACCFLOAT_MIN            FLT_MIN

#define ACCFLOAT_MAX            FLT_MAX
#elif defined(MAXFLOAT)
#define ACCFLOAT_MIN            MINFLOAT
#define ACCFLOAT_MAX            MAXFLOAT
#else
#define ACCFLOAT_MIN            -HUGE
#define ACCFLOAT_MAX            HUGE
#endif
      
#define accstrtoi(_a, _b, _c)   strtol(_a, _b, _c)
#define accstrtou(_a, _b, _c)   strtoul(_a, _b, _c)

/*
 *  Alignment
 */
#ifndef ALIGNTO
#define ALIGNTO(_n,_to)         ((((size_t)_n) + _to - 1) & ~(_to - 1))
#define ALIGNU16(_n)            ALIGNTO(_n, sizeof(uint16_t))
#define ALIGNU32(_n)            ALIGNTO(_n, sizeof(uint32_t))
#define ALIGNU64(_n)            ALIGNTO(_n, sizeof(uint64_t))
#define ISALIGNU16(_n)          (0 == (((size_t)_n) & 0x1))
#define ISALIGNU32(_n)          (0 == (((size_t)_n) & 0x3))
#define ISALIGNU64(_n)          (0 == (((size_t)_n) & 0x7))
#define ISALIGNTO(_n2,_to2)     (((size_t)_n2) == ALIGNTO(_n2,_to2))
#endif

/*
 *  Structure magic
 */
typedef uint32_t MAGIC_t;

#ifndef MKMAGIC
#define MKMAGIC(__a, __b, __c, __d) \
              (((uint32_t)(__a))<<24 | ((uint32_t)(__b))<<16 | (__c)<<8 | (__d))
#endif
#ifndef MKMAGIC32
#define MKMAGIC32(__a, __b, __c, __d) \
              (((uint32_t)(__a))<<24 | ((uint32_t)(__b))<<16 | (__c)<<8 | (__d))
#endif

#endif /*GR_EDTYPES_H_INCLUDED*/
