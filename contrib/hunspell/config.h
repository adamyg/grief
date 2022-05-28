#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  libhunspell <config.h>
 */
 
#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
      
#define VERSION "1.7.0"

#include "hunspell_mktemp.h"
#include <stdio.h>
#include <unistd.h>

#if defined(__MINGW32__)
#define fdopen(__a,__b)     hunspell_fdopen(__a,__b)
#else
#if !defined(WIN32)
#define WIN32
#endif
#define fdopen(__a,__b)     _fdopen(__a,__b)
#endif

#endif  /*CONFIG_H_INCLUDED*/
