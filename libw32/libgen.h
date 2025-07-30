#ifndef LIBW32_LIBGEN_H_INCLUDED
#define LIBW32_LIBGEN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libgen_h,"$Id: libgen.h,v 1.7 2025/06/28 11:07:20 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <libgen.h> implementation
 *
 * Copyright (c) 2007, 2012 - 2025 Adam Young.
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
LIBW32_API char *	w32_basenameA (char *path);
LIBW32_API wchar_t *	w32_basenameW (wchar_t *path);

LIBW32_API char *	w32_dirname (char *path);
LIBW32_API size_t	w32_dirname_r (const char *path, char *buf, size_t buflen);

LIBW32_API char *	w32_dirnameA (char *path);
LIBW32_API wchar_t *	w32_dirnameW (wchar_t *path);
LIBW32_API size_t	w32_dirnameA_r(const char *path, char *buf, size_t buflen);
LIBW32_API size_t	w32_dirnameW_r(const wchar_t *path, wchar_t *buf, size_t buflen);

__END_DECLS

#endif /*LIBW32_LIBGEN_H_INCLUDED*/
