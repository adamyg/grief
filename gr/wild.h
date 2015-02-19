#ifndef GR_WILD_H_INCLUDED
#define GR_WILD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_wild_h,"$Id: wild.h,v 1.20 2014/10/22 02:33:25 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: wild.h,v 1.20 2014/10/22 02:33:25 ayoung Exp $
 * Wild card pattern matching.
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

#include <edsym.h>
#include <patmatch.h>

__CBEGIN_DECLS

extern char **              shell_expand(const char *file);
extern const char *         shell_expand0(const char *file, char *buf, int len);
extern void                 shell_release(char **files);

extern int                  wild_file(const char *file, const char *pattern);
extern int                  wild_match(const char *file, const char *pattern, int flags);

extern char *               wild_glob(const char *str);
extern void                 wild_globfree(char *out);

__CEND_DECLS

#endif /*GR_WILD_H_INCLUDED*/
