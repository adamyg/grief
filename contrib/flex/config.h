#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  flex <config.h> ...
 */

/* see: configure.ac */
#define PACKAGE_NAME "flex"
#define PACKAGE_TARNAME "flex"
#define PACKAGE_VERSION "2.5.10"
#define PACKAGE_STRING "flex 2.5.10"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_URL "https://www.freshports.org/textproc/flex"
#define VERSION PACKAGE_VERSION

#include <../contrib_config.h>

#if defined(HAVE_STDINT_H) || \
	(defined(_MSC_VER) && (_MSC_VER >= 1900))
#include <stdint.h>
#define FLEXINT_H
#endif

#include <stdlib.h>
#ifndef _STDLIB_H
#define _STDLIB_H
#endif

#undef INFINITY

#endif  /*CONFIG_H_INCLUDED*/
