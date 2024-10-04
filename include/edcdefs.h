#ifndef GR_EDCDEFS_H_INCLUDED
#define GR_EDCDEFS_H_INCLUDED
#include <edidentifier.h>
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edcdefs.h,v 1.4 2024/10/01 12:55:03 cvsuser Exp $
 * <sys/cdefs.h> interface
 *
 * __BEGIN_DECLS
 * __END_DECLS
 * __P
 * __attribute__
 *
 */

#if !defined(HAVE_CONFIG_H)
#error HAVE_CONFIG_H expected to be defined ...
#endif
#include <config.h>

#if defined(HAVE_SYS_CDEFS_H)
#include <sys/cdefs.h>

#else

#if defined(__sun)
#include <stdio.h>
#endif

#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#     define __BEGIN_DECLS      extern "C" {
#     define __END_DECLS        }
#  else
#     define __BEGIN_DECLS
#     define __END_DECLS
#  endif
#endif
#ifndef __P
#  if (__STDC__) || defined(__cplusplus) || \
         defined(_MSC_VER) || defined(__PARADIGM__) || defined(__GNUC__) || \
         defined(__BORLANDC__) || defined(__WATCOMC__)
#     define __P(x)             x
#  else
#     define __P(x)             ()
#  endif
#endif

#if !defined(__GNUC__) && !defined(__clang__)
#ifndef __attribute__           /*FIXME: HAVE_ATTRIBUTE*/
#define __attribute__(__x)
#endif
#endif

#ifndef __unused
#define __unused
#endif

#endif /*HAVE_SYS_CDEFS_H*/

#endif /*GR_EDCDEFS_H_INCLUDED*/
