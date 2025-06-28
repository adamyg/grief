#ifndef LIBW32_SYS_FILE_H_INCLUDED
#define LIBW32_SYS_FILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_sys_file_h,"$Id: file.h,v 1.4 2025/06/28 11:07:21 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 sys/file.h
 *
 * Copyright (c) 2020 - 2025, Adam Young.
 * All rights reserved.
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

#include <sys/cdefs.h>

#define LOCK_SH             0x01
#define LOCK_EX             0x02
#define LOCK_NB             0x04
#define LOCK_UN             0x08

__BEGIN_DECLS

LIBW32_API int              w32_flock(int fd, int operation);

__END_DECLS

#endif /*LIBW32_SYS_FILE_H_INCLUDED*/
