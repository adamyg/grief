#ifndef GR_NETDB_H_INCLUDED
#define GR_NETDB_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_netdb_h,"$Id: netdb.h,v 1.8 2015/02/19 00:17:26 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <netdb.h> implementation
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

#include <sys/socket.h>                         /* winsock etc */
#include <sys/cdefs.h>

__BEGIN_DECLS

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || \
        defined(__WATCOMC__)
const char *            gai_strerror(int ecode);
#else
const char *            w32_gai_strerror(int ecode);
#endif

__END_DECLS

#endif /*GR_NETDB_H_INCLUDED*/
