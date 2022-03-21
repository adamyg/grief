#ifndef GR_SPHEADERS_H_INCLUDED
#define GR_SPHEADERS_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(cr_spheaders_h,"$Id: spheaders.h,v 1.12 2022/03/21 15:17:20 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: spheaders.h,v 1.12 2022/03/21 15:17:20 cvsuser Exp $
 * splay library
 *
 *
 * Copyright (c) 1998 - 2022, Adam Young.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include <chkalloc.h>
#include <libsplay.h>
#include <libmisc.h>
#include <vm_alloc.h>

SPLAY_PROTOTYPE(libsplay, _sproot, _spblk, _sp_node)

#endif /*GR_SPHEADERS_H_INCLUDED*/
