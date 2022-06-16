#ifndef LIBW32_MSVC_STDBOOL_H_INCLUDED
#define LIBW32_MSVC_STDBOOL_H_INCLUDED

/* -*- mode: c; indent-width: 4; -*- */
// ISO C9x  compliant inttypes.h for Microsoft Visual Studio
// Based on ISO/IEC 9899:TC2 Committee draft (May 6, 2005) WG14/N1124
//
// Public domain
//

#ifndef _MSC_VER
#error "Use this header only with Microsoft Visual C++ compilers!"
#endif
#if _MSC_VER > 1000
#pragma once
#endif

/**
 *  Microsoft C/C++
 *          version 14.00.50727.762, which comes with Visual C++ 2005,
 *      and version 15.00.30729.01, which comes with Visual C++ 2008,
 *      and version 16.00.30319.01, which comes with Visual C++ 2010,
 *  do not define _Bool.
 */
#if defined(_MSC_VER) && _MSC_VER <= 1600
typedef int _Bool;
#endif

#ifndef __cplusplus
#define bool _Bool
#define true 1
#define false 0
#endif

#endif /*LIBW32_MSVC_STDBOOL_H_INCLUDED*/
