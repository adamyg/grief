#ifndef GR_WIN32_INCLUDE_H_INCLUDED
#define GR_WIN32_INCLUDE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_include_h,"$Id: win32_include.h,v 1.5 2015/02/19 00:17:34 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * winsock2.h and windows.h include guard
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE            /* disable deprecate warnings */
#endif

#if !defined(HAVE_WINSOCK2_H_INCLUDED)
#define HAVE_WINSOCK2_H_INCLUDED
#undef gethostname                          /* unistd.h name mangling */
#if defined(u_char)
#undef u_char                               /* namespace issues (_BSD_SOURCE) */
#endif
#include <winsock2.h>
#include <ws2tcpip.h>                       /* getaddrinfo() */
#endif

#if !defined(HAVE_WINDOWS_H_INCLUDED)
#define HAVE_WINDOWS_H_INCLUDED
#define WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#endif /*HAVE_WINDOWS_H_INCLUDED*/

#endif /*GR_WIN32_INCLUDE_H_INCLUDED*/
