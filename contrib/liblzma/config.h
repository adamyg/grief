#ifndef XZ_CONFIG_H_INCLUDED
#define XZ_CONFIG_H_INCLUDED
/*
 *  liblzma <config.h> ...
 */

#define PACKAGE_NAME "xz"
#define PACKAGE_TARNAME "xz"
#define PACKAGE_VERSION "5.2.3"
#define PACKAGE_STRING "xz 5.2.3"
#define PACKAGE_BUGREPORT "lasse.collin@tukaani.org"
#define PACKAGE_URL "http://tukaani.org/xz/"
#define VERSION PACKAGE_VERSION

#include <../contrib_config.h>

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>

#undef HAVE_UTIME_H
#undef HAVE_SYS_UTIME_H

#if (defined(_WIN32) || defined(WIN32)) && defined(__MINGW32__)
#undef HAVE_GETOPT_H
#endif

#if defined(_MSC_VER)
#include <msvcversions.h>
#include <stdint.h>
#define inline __inline
#   if (_MSC_VER >= _MSC_VER_2013)
#       define restrict __restrict
#   else
#       define restrict
#   endif
#elif defined(__WATCOMC__)
#    define  restrict __restrict
#endif

#include <unistd.h>

#ifndef SSIZE_MAX
#define SSIZE_MAX ((ssize_t)(SIZE_MAX >> 1))
#endif

/* How many MiB of RAM to assume if the real amount cannot be determined. */
#define ASSUME_RAM 1000 /*1GB*/

#define HAVE_ENCODERS
#define HAVE_ENCODER_LZMA1
#define HAVE_ENCODER_LZMA2
#define HAVE_ENCODER_X86
#define HAVE_ENCODER_POWERPC
#define HAVE_ENCODER_IA64
#define HAVE_ENCODER_ARM
#define HAVE_ENCODER_ARMTHUMB
#define HAVE_ENCODER_SPARC
#define HAVE_ENCODER_DELTA

#define HAVE_DECODERS
#define HAVE_DECODER_LZMA1
#define HAVE_DECODER_LZMA2
#define HAVE_DECODER_X86
#define HAVE_DECODER_POWERPC
#define HAVE_DECODER_IA64
#define HAVE_DECODER_ARM
#define HAVE_DECODER_ARMTHUMB
#define HAVE_DECODER_SPARC
#define HAVE_DECODER_DELTA

#endif /*XZ_CONFIG_H_INCLUDED*/
