#ifndef LIBW32_WIN32_INCLUDE_H_INCLUDED
#define LIBW32_WIN32_INCLUDE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_include_h,"$Id: win32_include.h,v 1.19 2025/02/03 02:27:36 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * winsock2.h and windows.h include guard
 *
 * Copyright (c) 1998 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

/*
 *  WinSock/Windows definitions
 */

/* compiler tweaks */

#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif
    //#if !defined(_CRT_NO_POSIX_ERROR_CODES)
    //#define _CRT_NO_POSIX_ERROR_CODES             /* disable POSIX error number, see <errno.h> */
    //#endif
#endif /*_MSC_VER*/

    //#if defined(__WATCOMC__) && (__WATCOMC__ < 1300)
    //#if !defined(NTDDI_VERSION)
    //#define NTDDI_VERSION 0x06000000              /* iphlpapi.h requirement, inet_ntop .. */
    //#endif
    //#endif

#if defined(__GNUC__)   /*BOOST_GCC_VERSION equiv*/
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/* winsock and friends */

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#if !defined(_WINSOCK2_H)                       /* MINGW32 guard */

#undef gethostname                              /* unistd.h name mangling */
#if defined(u_char)
#undef u_char                                   /* namespace issues (_BSD_SOURCE) */
#endif

#if defined(__MINGW32__) && defined(SLIST_ENTRY)
#pragma push_macro("SLIST_ENTRY")               /* <sys/queue.h> */
#undef SLIST_ENTRY
#include <winsock2.h>
#pragma pop_macro("SLIST_ENTRY")
#else
#include <winsock2.h>
#if defined(__MINGW32__)
#undef SLIST_ENTRY
#endif
#endif

#include <ws2tcpip.h>                           /* getaddrinfo() */
#include <mswsock.h>                            /* IOCP */

#endif /*_WINSOCK2_H*/
#endif /*HAVE_WINSOCK2_H_INCLUDED*/

/* windows.h*/

#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#ifndef WINDOWS_NOT_MEAN_AND_LEAN
#define WINDOWS_MEAN_AND_LEAN
#endif
#include <windows.h>
#endif /*HAVE_WINDOWS_H_INCLUDED*/

#endif /*LIBW32_WIN32_INCLUDE_H_INCLUDED*/
