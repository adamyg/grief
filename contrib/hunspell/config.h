#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  libhunspell <config.h>
 */

#define VERSION "1.3.3"

#include "../contrib_config.h"

#if defined(__WATCOMC__)
/*
 *  patches required:
 *      src/hunvisapi.h:
 *
 *          -   #elif defined(_MSC_VER)
 *          +   #elif defined(_MSC_VER) || defined(__WATCOMC__)
 *
 *      src/relocatable.h:
 *
 *          -   #elif defined _MSC_VER && BUILDING_DLL
 *          +   #elif (defined(_MSC_VER) || defined(__WATCOMC__)) && BUILDING_DLL
 *
 *      src/csutil.hxx:
 *          
 *          +   #include <stdio.h>
 *              #include <string.h>
 *
 *      src/replist.hxx/.cxx:
 *
 *          -   int near(const char * word);
 *          +   int isnear(const char * word);
 *
 *      src/parsers/xmlparser.cxx
 *      src/parsers/odfparser.cxx
 *
 *          -   static const char * __PATTERN2__[][2] = {
 *          -   };
 *          -   #define __PATTERN_LEN2__ (sizeof(__PATTERN2__) / (sizeof(char *) * 2))
 *
 *          +   #define __PATTERN2__ NULL
 *          +   #define __PATTERN_LEN2__ 0
 */
#endif  /*__WATCOMC__*/

#endif  /*CONFIG_H_INCLUDED*/
