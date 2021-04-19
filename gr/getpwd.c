#include <edidentifier.h>
__CIDENT_RCSID(gr_getpwd_c,"$Id: getpwd.c,v 1.3 2021/04/19 13:59:54 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getpwd.c,v 1.3 2021/04/19 13:59:54 cvsuser Exp $
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

#include <editor.h>

#if defined(LOCAL_MAIN)
#undef NDEBUG
#endif

#if defined(HAVE_PWD_H) && !defined(WIN32)

    //#undef HAVE_GETPWNAM_R
    //#undef HAVE_GETPWUID_R
    //#undef HAVE_GETLOGIN

#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "getpwd.h"

#if !defined(HAVE_GETPWNAM_R) || !defined(HAVE_GETPWUID_R)

static int
string_length(const char *value, int length)
{
    if (!value || !*value) return length;
    return strlen(value) + 1 + length;
}


static char *
string_clone(const char *value, char **cursor)
{
    if (value && *value) {
        const size_t length = strlen(value);
        char *buffer = *cursor;

        memcpy(buffer, value, length + 1);
        *cursor += length + 1;
        return buffer;
    }
    return "";
}


static struct passwd *
passwd_clone(const struct passwd *result, passwd_t *pwd)
{
    struct passwd *t_result;
    char *buffer, *cursor;
    int length = 0;

    if (!result) {
        return NULL;
    }

    length = string_length(result->pw_name, length);
    length = string_length(result->pw_passwd, length);
    length = string_length(result->pw_gecos, length);
    length = string_length(result->pw_dir, length);
    length = string_length(result->pw_shell, length);

    if (NULL == (buffer =
            calloc(1, sizeof(struct passwd) + length))) {
        return NULL;
    }

    t_result = (struct passwd *)buffer;
    cursor = (char *)(t_result + 1);

    t_result->pw_name   = string_clone(result->pw_name, &cursor);
    t_result->pw_passwd = string_clone(result->pw_passwd, &cursor);
    t_result->pw_gecos  = string_clone(result->pw_gecos, &cursor);
    t_result->pw_dir    = string_clone(result->pw_dir, &cursor);
    t_result->pw_shell  = string_clone(result->pw_shell, &cursor);
    t_result->pw_uid    = result->pw_uid;
    t_result->pw_gid    = result->pw_gid;

    pwd->buffer = buffer;
    pwd->result = t_result;

    return t_result;
}
#endif  //!HAVE_GETPWNAM_R || !HAVE_GETPWUID_R


static size_t
getpwlimit(void)
{
#if defined(_SC_GETPW_R_SIZE_MAX)
    const long limit = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (limit > 0)
	return (size_t)limit;
#endif	//_SC_GETPW_R_SIZE_MAX
    return 4096;
}


struct passwd *
sys_getpwnam(passwd_t *pwd, const char *user)
{
#if defined(HAVE_GETPWNAM_R)
    const size_t length_limit = getpwlimit();
    size_t length = 128;
    void *buffer = NULL;

    assert(pwd && user);
    if (NULL == pwd || NULL == user) {
	return NULL;
    }

    pwd->buffer = pwd->result = NULL;
    do {
        struct passwd *result = NULL;
        void *t_buffer;
        int ret;

        if (NULL == (t_buffer =
                realloc(buffer, sizeof(struct passwd) + length))) {
            free(buffer);
            return NULL;
        }

        buffer = t_buffer;
        ret = getpwnam_r(user, buffer, sizeof(struct passwd) + (char *) buffer, length, &result);
        if (0 == ret && result) {
            pwd->buffer = buffer;
            pwd->result = result;
            return result;
        }
        length <<= 1;

    } while ((length <<= 1) <= length_limit);

    free(buffer);
    return NULL;

#else   //HAVE_GETPWNAM_R
#error

    assert(pwd && user);
    if (NULL == pwd || NULL == user)
        return NULL;

    pwd->buffer = pwd->result = NULL;
    return passwd_clone(getpwnam(user), pwd);

#endif  //HAVE_GETPWNAM_R
}


struct passwd *
sys_getpwuid(passwd_t *pwd, int uid)
{
#if defined(HAVE_GETPWUID_R)
    const size_t length_limit = getpwlimit();
    size_t length = 128;
    void *buffer = NULL;

    assert(pwd);
    if (NULL == pwd)
        return NULL;

    pwd->buffer = pwd->result = NULL;
    do {
        struct passwd *result = NULL;
        void *t_buffer;
        int ret;

        if (NULL == (t_buffer =
                realloc(buffer, sizeof(struct passwd) + length))) {
            free(buffer);
            return NULL;
        }

        buffer = t_buffer;
        ret = getpwuid_r((uid_t)uid, buffer, sizeof(struct passwd) + (char *) buffer, length, &result);
        if (0 == ret && result) {
            pwd->buffer = buffer;
            pwd->result = result;
            return result;
        }

    } while ((length <<= 1) <= length_limit);

    free(buffer);
    return NULL;

#else   //HAVE_GETPWNAM_R
#error

    assert(pwd);
    if (NULL == pwd) {
        return NULL;
    }

    pwd->buffer = pwd->result = NULL;
    return passwd_clone(getpwuid(uid), pwd);

#endif  //HAVE_GETPWNAM_R
}


struct passwd *
sys_getpwlogin(passwd_t *pwd)
{
#if defined(HAVE_GETLOGIN)
    const char *login;
    if (NULL != (login = getlogin())) {
        return sys_getpwnam(pwd, login);
    }
    return NULL;

#else   //HAVE_GETLOGIN

    return sys_getpwuid(pwd, getuid());

#endif  //HAVE_GETLOGIN
}


void
sys_getpwend(passwd_t *pwd, struct passwd *result)
{
    assert(pwd && pwd->result == result);
    if (pwd && pwd->result == result) {
        free(pwd->buffer);
    }
    endpwent();
}


///////////////////////////////////////////////////////////////////////////////

#else   //HAVE_PWD_H

#include "getpwd.h"

struct passwd *
sys_getpwnam(passwd_t *pwd, const char *user)
{
    assert(pwd && user);
    if (NULL == pwd || NULL == user) return NULL;

    pwd->buffer = pwd->result = NULL;
    return NULL;
}


struct passwd *
sys_getpwuid(passwd_t *pwd, int uid)
{
    assert(pwd);
    if (NULL == pwd) return NULL;
    pwd->buffer = pwd->result = NULL;
    return NULL;
}


struct passwd *
sys_getpwlogin(passwd_t *pwd)
{
    assert(pwd);
    if (NULL == pwd) return NULL;
    pwd->buffer = pwd->result = NULL;
    return NULL;
}


void
sys_getpwend(passwd_t *pwd, struct passwd *result)
{
    (void)pwd;
    (void)result;
}

#endif  //HAVE_PWD_H


///////////////////////////////////////////////////////////////////////////////

#if defined(LOCAL_MAIN)

static void
passwd_compare(const struct passwd *a, const struct passwd *b)
{
    assert(0 == strcmp(a->pw_name, b->pw_name));
    assert(0 == strcmp(a->pw_passwd, b->pw_passwd));
    assert(0 == strcmp(a->pw_gecos, b->pw_gecos));
    assert(0 == strcmp(a->pw_dir, b->pw_dir));
    assert(0 == strcmp(a->pw_shell, b->pw_shell));
    assert(a->pw_uid == b->pw_uid);
    assert(a->pw_gid == b->pw_gid);
}


static void
passwd_print(const struct passwd *p, const char *label)
{
    printf("get%s:\n", label);
    printf("  name:   %s\n", p->pw_name);
    printf("  passwd: %s\n", p->pw_passwd);
    printf("  gecos:  %s\n", p->pw_gecos);
    printf("  dir:    %s\n", p->pw_dir);
    printf("  shell:  %s\n", p->pw_shell);
    printf("  uid:    %d\n", (int)p->pw_uid);
    printf("  gid:    %d\n", (int)p->pw_gid);
}


static struct passwd *
native_getpwlogin()
{
#if defined(HAVE_GETLOGIN)
    const char *login;
    if (NULL != (login = getlogin())) {
        return getpwnam(login);
    }
    return NULL;

#else
    return getpwuid(getuid());
#endif
}


int
main()
{
    {	const size_t length_limit = getpwlimit();
	assert(length_limit);
        printf("length_limit: %u\n", (unsigned)length_limit);
    }

    {   passwd_t pwd = {0};
        uid_t uid = getuid();
        struct passwd *a = sys_getpwuid(&pwd, uid),
            *b = getpwuid(uid);

        passwd_compare(a, b);
        passwd_print(a, "uid(a)");
        passwd_print(b, "uid(b)");
        sys_getpwend(&pwd, a);
    }

    {   passwd_t pwd = {0};
        struct passwd *a = sys_getpwlogin(&pwd),
            *b = native_getpwlogin();

        passwd_compare(a, b);
        passwd_print(a, "login(a)");
        passwd_print(b, "login(b)");
        sys_getpwend(&pwd, a);
    }
}
#endif  //LOCAL_MAIN

/*end*/


