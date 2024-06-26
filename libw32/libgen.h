#ifndef LIBW32_LIBGEN_H_INCLUDED
#define LIBW32_LIBGEN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libgen_h,"$Id: libgen.h,v 1.5 2024/03/31 15:57:24 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <libgen.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2024 Adam Young.
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

#include <sys/cdefs.h>

__BEGIN_DECLS

LIBW32_API char *	w32_basename (char *path);
LIBW32_API char *	w32_dirname (char *path);

__END_DECLS

#endif /*LIBW32_LIBGEN_H_INCLUDED*/
