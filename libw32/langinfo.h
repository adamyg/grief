#ifndef GR_LANGINFO_H_INCLUDED
#define GR_LANGINFO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_langinfo_h,"$Id: langinfo.h,v 1.6 2015/02/19 00:17:25 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <langinfo.h> implementation
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

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef enum {
    CODESET         = 0
} nl_item;


const char *        nl_langinfo(nl_item);

__END_DECLS

#endif /*GR_LANGINFO_H_INCLUDED*/
