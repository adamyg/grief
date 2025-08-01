#ifndef GR_EDLARGEFILE_H_INCLUDED
#define GR_EDLARGEFILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edlargefile_h,"$Id: edlargefile.h,v 1.15 2025/01/13 16:20:06 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edlargefile.h,v 1.15 2025/01/13 16:20:06 cvsuser Exp $
 * Large file support interface.
 *
 *
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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if defined(sun)
#define _FILE_OFFSET_BITS       64
#define _LARGEFILE_SOURCE

#elif defined(__DGUX__)
#define _FILE_OFFSET_BITS       64

#elif defined(_AIX)
#define _LARGE_FILES

#elif defined (__hpux) && !defined(BB64BIT)
#define _APP32_64BIT_OFF_T
#endif

#endif  /*GR_EDLARGEFILE_H_INCLUDED*/
