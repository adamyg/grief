/*
 *  MSVC/OWC/MINGW <unistd.h>
 */
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined(__WATCOMC__)
#if defined(__cplusplus)
#pragma disable_message(7)
#pragma disable_message(88)
#pragma disable_message(391)
#else
#pragma disable_message(392)
#endif
#endif

#if defined(_MSC_VER) || defined(__WATCOMC__)
#include <direct.h>
#endif
#include <process.h>
#include <io.h>

#include "../hunspell_mktemp.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
#if !defined(mode_t)
#define mode_t      unsigned short
#endif
#endif

#if defined(__MINGW32__)
#define S_IRWXG     0
#define S_IRWXO     0
#endif

#if defined(_MSC_VER)
#define S_IWUSR     S_IWRITE
#define S_IRUSR     S_IREAD
#define S_IXUSR     0
#define S_IRWXG     0
#define S_IRWXO     0
#endif

/*end*/
