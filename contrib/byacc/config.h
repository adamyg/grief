#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  byacc <config.h> ...
 */

#define PACKAGE_NAME "byacc"
#define PACKAGE_TARNAME "byacc"
#define PACKAGE_VERSION "20240109"
#define PACKAGE_STRING "byacc 20240109"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_URL ""

#if defined(__WATCOMC__)
#pragma disable_message(107)  // Missing return value for function 'xxx'
#pragma disable_message(131)  // No prototype found for function 'xxx'
#pragma disable_message(304)  // Return type 'int' assumed for function 'xxx'
#endif
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable:4013) // 'xxx' undefined; assuming extern returning int
#pragma warning(disable:4033) // 'xxx' must return a value
#pragma warning(disable:4716) // 'xxx': must return a value
#pragma warning(disable:4996) // 'xxx': The POSIX name for this item is deprecated.
#endif

#include <../contrib_config.h>

#include <unistd.h>

#if !defined(HAVE_MKSTEMP)
#define HAVE_MKSTEMP 1
#endif
#if !defined(HAVE_GETOPT)
#define HAVE_GETOPT 1
#endif

#endif  /*CONFIG_H_INCLUDED*/
