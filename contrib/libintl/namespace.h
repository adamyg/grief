/*
 *  libintl, namespace.h
 */

#ifdef  _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#pragma warning(disable:4996)                   /* 'xxx' was declared deprecated */
#endif

#include <sys/types.h>
#include <sys/utypes.h>
#include <limits.h>

#ifndef snprintf
#define snprintf        _snprintf
#define vsnprintf       _vsnprintf
#define strdup          _strdup
#define open            _open
#define read            _read
#define write           _write
#endif

#if defined(_MSC_VER)
#define LC_MESSAGES     (LC_MAX + 1)
#endif
