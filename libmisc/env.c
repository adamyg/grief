#include <edidentifier.h>
__CIDENT_RCSID(cr_env_c,"$Id: env.c,v 1.33 2024/04/27 14:40:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: env.c,v 1.33 2024/04/27 14:40:05 cvsuser Exp $
 * Environment related functions.
 *
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#include <editor.h>
#include <edenv.h>
#include <libstr.h>

#include <tailqueue.h>

#ifndef MKMAGIC
#define MKMAGIC(a,b,c,d)    (((unsigned long)(a))<<24|((unsigned long)(b))<<16|(c)<<8|(d))
#endif

#define MAGIC_ELEMENT       MKMAGIC('E','n','v','N')

#if defined(_VMS) || !defined(HAVE_SETENV)
#define NEED_SETENV_CACHE
#endif

#if defined(NEED_SETENV_CACHE)
typedef TAILQ_HEAD(_EnvList, EnvElement)
                            ENVLIST_t;

struct EnvElement {
    unsigned long magic;
    TAILQ_ENTRY(EnvElement) node;
    const char *value;
};

static ENVLIST_t            x_envtq;

static struct EnvElement *  gelement(const char *);
static const char *         glookup(const char *);
static void                 greplace(const char *arg);


void
env_init(void)
{
    if (NULL == x_envtq.tqh_last) {
        TAILQ_INIT(&x_envtq);
    }
}


void
env_shutdown(void)
{
    if (x_envtq.tqh_last) {
        struct EnvElement *mp;
        while (NULL != (mp = TAILQ_FIRST(&x_envtq))) {
            assert(mp->magic == MAGIC_ELEMENT);
            TAILQ_REMOVE(&x_envtq, mp, node);
            free(mp);
        }
    }
}


static struct EnvElement *
gelement(const char *name)
{
    if (x_envtq.tqh_last) {
        struct EnvElement *mp;
        int len;

        for (len = 0; name[len] != '=' && name[len];) {
            ++len;
        }

        TAILQ_FOREACH(mp, &x_envtq, node) {
            assert(mp->magic == MAGIC_ELEMENT);
            if (0 == strncmp(name, mp->value, len) && '=' == mp->value[len]) {
                return mp;
            }
        }
    }
    return NULL;
}


static const char *
glookup(const char *name)
{
    struct EnvElement *mp = gelement(name);
    if (mp) {
        return mp->value;
    }
    return NULL;
}


static void
greplace(const char *name)
{
    if (name && *name) {
        struct EnvElement *mp;

        env_init();
        if (NULL != (mp = gelement(name))) {
            mp->value = name;

        } else {
            if (NULL != (mp = (struct EnvElement *)calloc(1, sizeof(struct EnvElement)))) {
                mp->magic = MAGIC_ELEMENT;
                mp->value = name;
                TAILQ_INSERT_TAIL(&x_envtq, mp, node);
            }
        }
    }
}


#else /*NEED_SETENV_CACHE*/
void
env_init(void)
{
}


void
env_shutdown(void)
{
}
#endif  /*NEED_SETENV_CACHE*/


#if !defined(_VMS) && !defined(HAVE_SETENV)
#if !defined(NEED_SETENV_CACHE)
#error requires NEED_SETENV_CACHE ...
#endif

/*  Function:           myputenv
 *      putenv() libc wrapper function
 *
 *  Notes:
 *      When using putenv() the string pointed to by 'nv' becomes part of the
 *      environment, so altering the string will change the environment. The space
 *      used by 'nv' is no longer used once a new string-defining the same variable
 *      is passed to putenv().
 *
 *      The difference to the setenv function is that the exact string given as the
 *      parameter string is put into the environment. If the user should change the
 *      string after the putenv call this will reflect in automatically in the
 *      environment. This also requires that string is no automatic variable which
 *      scope is left before the variable is removed from the environment. The same
 *      applies of course to dynamically allocated variables which are freed later.
 *
 *  Notes:
 *      Possible memory leak can result from putenv() usage.
 */
static int
myputenv(char *arg)
{
    int rc;

#if (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
    if (0 == (rc = _putenv(arg))) {
#else
    if (0 == (rc = putenv(arg))) {
#endif
        greplace(arg);
    }
    return rc;
}
#endif  /*_VMS && !HAVE_SETENV*/


/*  Function:           ggetenv
 *      Retrieve a value from the current environment.
 *
 *  Return Value:
 *      The return value is NULL if varname is not found in the environment
 *      table.
 *
 *  Notes:
 *      Returns a pointer to the environment table entry containing varname.
 *
 *      It is not safe to modify the value of the environment variable using
 *      the returned pointer, as the behavior is undefined. Use the gputenv()
 *      function to modify the value of an environment variable.
 *
 */
const char *
ggetenv(const char * name)
{
#if !defined(_VMS)
    return getenv(name);

#else /*_VMS*/
    const char *cp = glookup(name);

    if (NULL != cp) {
        while (*cp != '=' && *cp) {
            ++cp;
        }
        return (*cp == '=' ? cp + 1 : cp);
    }
    return getenv(name);
#endif
}


/*  Function:           ggetnenv
 *      Retrieve a value from the current environment.
 *
 *  Return Value:
 *      The return value is NULL if varname is not found in the environment
 *      table.
 *
 *  Notes:
 *      Returns a pointer to the environment table entry containing varname.
 *
 *      It is not safe to modify the value of the environment variable using
 *      the returned pointer, as the behavior is undefined. Use the gputenv()
 *      function to modify the value of an environment variable.
 *
 */
const char *
ggetnenv(const char * name, int length)
{
    char t_name[256],                           /* MAGIC */
        *dpend = t_name + (sizeof(t_name) - 2),
        *dp = t_name;

    while (*name && --length >= 0 && dp < dpend) {
        *dp++ = *name++;
    }
    *dp = 0;
    return ggetenv(t_name);
}


/*  Function:           gputenv
 *      Create, modify, or remove environment variables
 *
 *  Description:
 *      The gputenv() function adds new environment variables or modifies the
 *      values of existing environment variables.
 *
 *      The environment value described by 'arg' is copied by this function.
 *
 *  Returns:
 *      Return 0 if successful, or -1 in the case of an error.
 */
int
gputenv(const char * arg)
{
    int r = -1;

    if (arg && *arg) {
        const char *eq = strchr(arg, '=');
        char t_buffer[180] = {0};
        int taglen = 0;

        if (NULL == eq) {                       /* missing =<value> */
            if ((taglen = strlen(arg)) >= (int) (sizeof(t_buffer) - 4)) {
                errno = EINVAL;
                return -1;
            }
            sxprintf(t_buffer, sizeof(t_buffer), "%s=1", arg);
            eq = (arg = t_buffer) + taglen;

        } else {
            taglen = eq - arg;
        }

        if (eq) {
            char *env = strdup(arg);            /* local working copy */

            if (env) {
#if defined(_VMS)
                greplace(env);
                ret = 0;
#else
#if defined(HAVE_SETENV)
                env[ taglen ] = '\0';           /* replace '=' */
                r = setenv(env, env + taglen + 1, 1);
                free(env);
#else
                r = myputenv(env);
#endif
#endif
            }
        }
    }
    return r;
}


/*  Function:           gputenv2
 *      Create, modify, or remove environment variables
 *
 *  Description:
 *      The gsetenv2() function shall update or add a variable in the environment of
 *      the calling process.
 *
 *      The 'tag' argument points to a string containing the name of an environment
 *      variable to be added or altered. The function shall fail if 'envname' points to
 *      a string which contains an '=' character.
 *
 *      The environment variable shall be set to the value to which 'value' points.
 *
 *      If the environment variable named by 'tag' already exists and the value of
 *      overwrite is non-zero, the function shall return success and the environment
 *      shall be updated.
 *
 *      If the environment variable named by 'tag' already exists and the value of
 *      overwrite is zero, the function shall return success and the environment shall
 *      remain unchanged.
 *
 *      If the application modifies environ or the pointers to which it points, the
 *      behavior of gsetenv2() is undefined. The gsetenv2() function shall update the
 *      list of pointers to which environ points.
 *
 *      The strings described by 'tag' and envval are copied by this function.
 *
 *  Returns:
 *      Return 0 if successful, or -1 in the case of an error.
 */
int
gputenv2(const char *tag, const char *value)
{
    const char *eq = strchr(tag, '=');
    size_t taglen = (tag ? strlen(tag) : 0);
    int r;

    assert(NULL == eq);

    if (0 == taglen) {
        r = 0;

    } else if (NULL != eq) {
        r = -1;

    } else  {
        const size_t len = taglen + (value ? strlen(value) : 0) + 2;
        char *nv = malloc(len);

        if (nv) {
            sprintf(nv, "%s=%s", tag, (value ? value : ""));

#if defined(_VMS)
            greplace(nv);
            r = 0;
#else
#if defined(HAVE_SETENV)
            nv[ taglen ] = '\0';                /* replace '=' */
            r = setenv(nv, nv + taglen + 1, 1);
            free(nv);
#else
            r = myputenv(nv);
#endif
#endif
        } else {
            r = -1;
        }
    }
    return r;
}


int
gputenvi(const char *tag, int value)
{
    char buffer[32];

    sxprintf(buffer, sizeof(buffer), "%d", value);
    gputenv2(tag, buffer);
    return 0;
}

/*end*/
