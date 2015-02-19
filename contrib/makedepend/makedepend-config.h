/*
 *  makedepend configuration, for makedepend-1.0.4
 *
 *  Introduction to makedepend
 *
 *      The makedepend package contains a C-preprocessor like utility to
 *      determine build-time dependencies.
 *
 *      This package is known to build and work properly using an LFS-7.2
 *      platform. Package Information
 *
 *          Download (HTTP):    http://xorg.freedesktop.org/releases/individual/util/makedepend-1.0.4.tar.bz2
 *
 *          Download (FTP):     ftp://ftp.x.org/pub/individual/util/makedepend-1.0.4.tar.bz2
 *
 * Copyright (c) 2012 Adam Young.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ==end==
 */

#include <../contrib_config.h>

#if !defined(HAVE_RENAME)
#define HAVE_RENAME
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#if defined(HAVE_IO_H)
#include <io.h>
#endif

#define _X_ATTRIBUTE_PRINTF(_a,__b)

#if defined(_WIN32)
#define OBJSUFFIX       "obj"

#if !defined(S_ISDIR)
#define __S_ISTYPE(mode,mask) \
                        (((mode) & S_IFMT) == mask)
#define S_ISDIR(m)      __S_ISTYPE(m, S_IFDIR)
#endif

#ifndef open
#define open            _open
#define read            _read
#define close           _close
#define chmod           _chmod
#define unlink          _unlink
#endif
#ifndef strdup
#define strdup          _strdup
#endif
#else
#define OBJSUFFIX       "o"
#endif

/*end*/
