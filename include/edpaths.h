#ifndef GR_EDPATHS_H_INCLUDED
#define GR_EDPATHS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edpaths_h,"$Id: edpaths.h,v 1.23 2024/05/02 16:33:27 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edpaths.h,v 1.23 2024/05/02 16:33:27 cvsuser Exp $
 * Default system paths ...
 *
 *  Example:
 *
 *      o Unix, ROOT=/usr/local/share/gr
 *      o WIN32, ROOT=C:/Program Files/Grief
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#include <config.h>
#if defined(HAVE_PATHS_H)
#include <paths.h>
#endif

#include <edbuildinfo.h>                        /* exported configure settings */

#if (defined(_WIN32) || defined(WIN32)) && !defined(__CYGWIN__)
#define _PATH_GRIEF_NAME    "Grief"
#define _PATH_GRIEF_ROOT    "$(ProgramFiles)/Grief"
#else
#define _PATH_GRIEF_NAME    "Grief"

#if defined(GR_BUILD_DATADIR)
#define _PATH_GRIEF_ROOT    GR_BUILD_DATADIR
#elif defined(GR_BUILD_LIBDIR)
#define _PATH_GRIEF_ROOT    GR_BUILD_LIBDIR
#else
#define _PATH_GRIEF_ROOT    GRIEF_LIBDIR
#endif
#endif

#define _PATH_GRIEF_HELP    _PATH_GRIEF_ROOT "/help"
#define _PATH_GRIEF_MACROS  _PATH_GRIEF_ROOT "/macros"
#define _PATH_GRIEF_SOURCE  _PATH_GRIEF_ROOT "/src"
#define _PATH_GRIEF_DICT    _PATH_GRIEF_ROOT "/dictionaries"
#define _PATH_GRIEF_CXMOD   _PATH_GRIEF_ROOT "/ctbl"

#ifndef _PATH_TERMCAP
#define _PATH_TERMCAP       "/etc/termcap"
#endif

#endif /*GR_EDPATHS_H_INCLUDED*/
