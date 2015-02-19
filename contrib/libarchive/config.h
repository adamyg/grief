#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED
/*
 *  libarchive <config.h> ...
 */

#define PACKAGE_NAME "libarchive"
#define PACKAGE_TARNAME "libarchive"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_URL ""
#define VERSION PACKAGE_VERSION

//  #define PACKAGE_VERSION "3.0.4"
//  #define PACKAGE_STRING "libarchive 3.0.4"
//  #define LIBARCHIVE_VERSION_STRING "3.0.4"
//  #define LIBARCHIVE_VERSION_NUMBER "3000004"

#define PACKAGE_VERSION "3.1.2"
#define PACKAGE_STRING "libarchive 3.1.2"
#define LIBARCHIVE_VERSION_STRING "3.1.2"
#define LIBARCHIVE_VERSION_NUMBER "3001002"

#include "../contrib_config.h"

#undef lstat
#undef open
#undef read
#undef write

#if defined(__WATCOMC__)
/*
 *  patches required:
 *   archive_entry.h:
 *
 *   -   #elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__BORLANDC__)
 *       # define	__LA_MODE_T	unsigned short
 *
 *   +   #elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__BORLANDC__) && !defined(__WATCOMC__)
 *       # define	__LA_MODE_T	unsigned short
 */
#define HAVE_DECL_SIZE_MAX 1
#define HAVE_WCSCPY 1
#define HAVE_WCSLEN 1
#define _SSIZE_T_DEFINED 1

#undef S_ISLNK
#undef S_ISSOCK
#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#undef S_IRWXG
#undef S_IXGRP
#undef S_IWGRP
#undef S_IRGRP
#undef S_IRWXO
#undef S_IXOTH
#undef S_IWOTH
#undef S_IROTH
#endif /*__WATCOMC__*/

#endif /*CONFIG_H_INCLUDED*/
