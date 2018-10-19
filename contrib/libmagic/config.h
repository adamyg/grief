#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  libmagic <config.h> ...
 */

#define PACKAGE_NAME "file"
#define PACKAGE_TARNAME "file"
#define PACKAGE_VERSION "5.29"
#define PACKAGE_STRING "file 5.29"
#define PACKAGE_BUGREPORT "christos@astron.com"
#define PACKAGE_URL ""
#define VERSION PACKAGE_VERSION

#include <../contrib_config.h>

#undef  HAVE_UTIME_H
#undef  HAVE_SYS_UTIME_H

#if defined(WIN32) && defined(__MINGW32__)
#undef  HAVE_GETOPT_H
#endif

#include <unistd.h>

#if defined(__MINGW32__)
#if !defined(lstat)
#define lstat w32_lstat
#define readlink w32_readlink
#endif
#endif

#ifndef UINT32_MAX
#define UINT32_MAX  ((uint32_t)-1)
#endif

#endif  /*CONFIG_H_INCLUDED*/
