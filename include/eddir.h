#ifndef GR_EDDIR_H_INCLUDED
#define GR_EDDIR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_eddir_h,"$Id: eddir.h,v 1.15 2023/01/01 11:26:58 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: eddir.h,v 1.15 2023/01/01 11:26:58 cvsuser Exp $
 * Directory management.
 *
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
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

#if defined(HAVE_NDIR_H)
#include <ndir.h>
#endif

#if defined(HAVE_VMSDIR_H)
#include "vmsdir.h"
#endif

#if defined(HAVE_SYS_DIR_H)
#include <sys/dir.h>
#endif

#if defined(_WIN32) || defined(WIN32) /* Native and MingW builds */
#include <../libw32/dirent.h>
#else
#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#elif defined(HAVE_DIRECT_H)
#include <direct.h>
#define dirent          direct
#endif
#endif

#if !defined(DIRSIZ) || !(DIRSIZ)
#undef  DIRSIZ
#ifdef  MAX_PATH
#define DIRSIZ          MAX_PATH
#else
#define DIRSIZ          255
#endif
#endif

#endif /*GR_EDDIR_H_INCLUDED*/
