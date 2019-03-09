#ifndef GR_EDENV_H_INCLUDED
#define GR_EDENV_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edenv_h,"$Id: edenv.h,v 1.15 2018/10/04 01:27:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edenv.h,v 1.15 2018/10/04 01:27:59 cvsuser Exp $
 * Environment interface.
 *
 *
 *
 * Copyright (c) 1998 - 2018, Adam Young.
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

#include <edsym.h>

__CBEGIN_DECLS

extern void                 env_init(void);
extern void                 env_shutdown(void);

extern const char *         ggetenv(const char *name);
extern const char *         ggetnenv(const char *name, int length);
extern int                  gputenv(const char *name);
extern int                  gputenv2(const char *tag, const char *value);
extern int                  gputenvi(const char *tag, int value);

__CEND_DECLS

#endif /*GR_EDENV_H_INCLUDED*/
/*end*/
