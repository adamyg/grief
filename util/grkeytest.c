/* -*- mode: c; indent-width: 4; -*- */
/* $Id: grkeytest.c,v 1.4 2024/08/30 15:11:38 cvsuser Exp $
 * console key-test
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(_WIN32)

#if !defined(_WIN32_WINNT)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#elif (_WIN32_WINNT < 0x601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#undef  WINVER
#define WINVER _WIN32_WINNT

#include "w32keytest.c"
#include "getoptlong.c"

#else

#include "conkeytest.c"

#endif

//end
