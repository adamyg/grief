#ifndef LIBTRE_TRE_CONFIG_H_INCLUDED
#define LIBTRE_TRE_CONFIG_H_INCLUDED
/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tre-config.h,v 1.1 2014/07/08 22:50:18 ayoung Exp $
 * libtre tre-config.h
 *
 *
 */

#ifndef RC_INVOKED
#include "config.h"
#endif

/* Define if you want to enable approximate matching functionality. */
#define TRE_APPROX 1

/* Define to enable multibyte character set support. */
/* #undef TRE_MULTIBYTE */

/* Define to the absolute path to the system regex.h */
/* #undef TRE_SYSTEM_REGEX_H_PATH */

/* Define if you want TRE to use alloca() instead of malloc() when allocating
   memory needed for regexec operations. */
#define TRE_USE_ALLOCA 1

/* Define to include the system regex.h from TRE regex.h */
/* #undef TRE_USE_SYSTEM_REGEX_H */

/* Define to enable wide character (wchar_t) support. */
#define TRE_WCHAR 1

/* TRE version string. */
#define TRE_VERSION "0.8.0"

/* TRE version level 1. */
#define TRE_VERSION_1 0

/* TRE version level 2. */
#define TRE_VERSION_2 8

/* TRE version level 3. */
#define TRE_VERSION_3 0

#endif  /*LIBTRE_CONFIG_H_INCLUDED*/
/*end*/
