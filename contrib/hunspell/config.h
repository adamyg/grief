#ifndef HUNSPELL_CONFIG_H_INCLUDED
#define HUNSPELL_CONFIG_H_INCLUDED
/*
 *  libhunspell <config.h>
 */
 
#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif //_MSC_VER

#if defined(__WATCOMC__)
#pragma disable_message(391)                    // assignment found in boolean expression
#include <ios>
    namespace std {
        namespace ios_base {
            typedef ios::openmode openmode;
        };
    };
#endif //__WATCOMC__

#include "version.h"
#include <unistd.h>

#endif /*HUNSPELL_CONFIG_H_INCLUDED*/
