#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  extags <config.h>
 */

#include "../contrib_config.h"

#if defined(_MSC_VER)
# define MANUAL_GLOBBING 1

# ifndef HAVE_IO_H
# define HAVE_IO_H
# endif

# ifndef HAVE_STRSTR
# define HAVE_STRSTR
# endif
# ifndef HAVE_STRCASECMP
# define HAVE_STRCASECMP
# endif

# if !defined(HAVE_FINDFIRST) && !defined(HAVE__FINDFIRST)
#  define HAVE__FINDFIRST
# endif
# if _MSC_VER >= 1300
#  define findfirst_t intptr_t                  /* Visual Studio 7 */
# else
#  define findfirst_t long                      /* Visual Studio 6 or earlier */
# endif
#endif  /*MSC_VER*/

#define tempnam(dir,pfx) _tempnam(dir,pfx)
#define TMPDIR "\\"

#endif  /*CONFIG_H_INCLUDED*/

