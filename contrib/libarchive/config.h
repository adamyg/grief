#ifndef __LIBARCHIVE_CONFIG_H_INCLUDED
#define __LIBARCHIVE_CONFIG_H_INCLUDED
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

//  #define PACKAGE_VERSION "3.1.2"
//  #define PACKAGE_STRING "libarchive 3.1.2"
//  #define LIBARCHIVE_VERSION_STRING "3.1.2"
//  #define LIBARCHIVE_VERSION_NUMBER "3001002"

//  #define PACKAGE_VERSION "3.3.3"
//  #define PACKAGE_STRING "libarchive 3.3.3"
//  #define LIBARCHIVE_VERSION_STRING "3.3.3"
//  #define LIBARCHIVE_VERSION_NUMBER "3003003"

#define PACKAGE_VERSION "3.6.1"
#define PACKAGE_STRING "libarchive 3.6.1"
#define LIBARCHIVE_VERSION_STRING "3.6.1"
#define LIBARCHIVE_VERSION_NUMBER "3006001"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601                     // Windows 7; bcrypt requirement
#endif
#undef  WINVER
#define WINVER _WIN32_WINNT                     // MingW

#include "../contrib_config.h"

#if !defined(HAVE_CTYPE_H)
#define HAVE_CTYPE_H 1                          // TODO: makelib.pl
#endif
#define USE_NATIVE_DIRECT                       // libw32

#include <unistd.h>

#if !defined(HAVE_BCRYPT_H)
#if !defined(__MINGW32__)
#error <bcrypt.h> missing ...
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1500)     // 2008; missing Win7 SDK components
#include <windows.h>
#include <bcrypt.h>
#ifndef NT_SUCCESS
#define NT_SUCCESS(x) ((x)>=0)
#define STATUS_SUCCESS ((NTSTATUS)0)
#endif //NT_SUCCESS
#ifndef BCRYPT_SUCCESS
#define BCRYPT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
NTSTATUS WINAPI BCryptDeriveKeyPBKDF2(BCRYPT_ALG_HANDLE hPrf, PUCHAR pbPassword, ULONG cbPassword, PUCHAR pbSalt, ULONG cbSalt, ULONGLONG cIterations, PUCHAR pbDerivedKey, ULONG cbDerivedKey, ULONG dwFlags);
#endif //_MSC_VER

#undef lstat
#undef open
#undef read
#undef write

#if defined(__WATCOMC__)
/*
 *  patches required: (WATCOMC < 1300)
 *   archive_entry.h:
 *
 *   -   #elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__BORLANDC__)
 *       # define __LA_MODE_T   unsigned short
 *
 *   +   #elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__BORLANDC__) && !defined(__WATCOMC__)
 *       # define __LA_MODE_T   unsigned short
 */
 
#define HAVE_DECL_SIZE_MAX 1
#define HAVE_WCSCPY 1
#define HAVE_WCSLEN 1
#define _SSIZE_T_DEFINED 1

#undef S_ISLNK
#define S_ISLNK(m) (0)
#undef S_ISSOCK

#if (__WATCOMC__ >= 1300)
#undef _S_IXGRP
#undef _S_IWGRP
#undef _S_IRGRP
#undef _S_IRWXO
#undef _S_IXOTH
#undef _S_IWOTH
#undef _S_IROTH
#undef _S_IFBLK
#undef _S_IFLNK
#undef _S_IFSOCK
#endif
#endif /*__WATCOMC__*/

#if defined(_MSC_VER)
#undef S_ISLNK
#undef S_ISSOCK
#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#undef S_IRWXG

#undef S_IXGRP
#undef S_IWGRP
#undef S_IRGRP
#endif

#if defined(__MINGW32__)
#if defined(__MINGW64_VERSION_MAJOR)
#include <ntdef.h>
#endif

#undef S_ISLNK
#undef S_ISSOCK
#undef S_ISUID
#undef S_ISGID
#undef S_ISVTX
#undef S_IRWXG

#undef S_IXGRP
#undef S_IWGRP
#undef S_IRGRP
#endif /*__MINGW32__*/

#endif /*__LIBARCHIVE_CONFIG_H_INCLUDED*/

