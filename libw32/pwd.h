#ifndef GR_PWD_H_INCLUDED
#define GR_PWD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_pwd_h,"$Id: pwd.h,v 1.4 2015/02/19 00:17:26 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
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

/*
 *  The <pwd.h> header shall provide a definition for struct passwd,
 *   which shall include at least the following members:
 *
 *      char    *pw_name            User's login name.
 *      uid_t    pw_uid             Numerical user ID.
 *      gid_t    pw_gid             Numerical group ID.
 *      char    *pw_dir             Initial working directory.
 *      char    *pw_shell           Program to use as shell.
 */

struct passwd {
    const char *        pw_name;
    const char *        pw_passwd;
    int			pw_uid;
    int			pw_gid;
    const char *        pw_age;
    const char *        pw_comment;
    const char *        pw_gecos;
    const char *        pw_dir;
    const char *        pw_shell;
    long		pw_audid;
    int			pw_audflg;
};

struct passwd *         getpwent(void);
struct passwd *         getpwuid(int);
struct passwd *         getpwnam(const char *);
void                    setpwent(void);
void                    endpwent(void);

//  int                 getpwnam_r(const char *, struct passwd *, char *, size_t, struct passwd **);
//  int                 getpwuid_r(uid_t, struct passwd *, char *, size_t, struct passwd **);

__END_DECLS

#endif /*GR_PWD_H_INCLUDED*/
