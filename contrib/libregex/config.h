/*
 *  libregexp config.h
 */

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <../contrib_config.h>

#include <math.h>
#if defined(_WIN32) || defined(WIN32)
#if !defined(_POSIX2_RE_DUP_MAX)
#define _POSIX2_RE_DUP_MAX              255
#endif
#endif

#undef INFINITY                         /*redef*/

#if !defined(_DIAGASSERT)
#define _DIAGASSERT(__x)                /*not used*/
#endif

#if !defined(__UNCONST)
#define __UNCONST(__s)                  ((char *) __s)
#endif

/*end*/

