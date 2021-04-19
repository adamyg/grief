#ifndef GR_GETPWD_H_INCLUDED
#define GR_GETPWD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_getpwd_h,"$Id: getpwd.h,v 1.1 2021/04/18 15:54:44 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getpwd.h,v 1.1 2021/04/18 15:54:44 cvsuser Exp $
 * Pasword support.
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

__CBEGIN_DECLS

typedef struct {
    void *buffer;
    void *result;
} passwd_t;

struct passwd *sys_getpwnam(passwd_t *pwd, const char *user);
struct passwd *sys_getpwuid(passwd_t *pwd, int uid);
struct passwd *sys_getpwlogin(passwd_t *pwd);
void sys_getpwend(passwd_t *pwd, struct passwd *result);

__CEND_DECLS

#endif /*GR_GETPWD_H_INCLUDED*/


