#include <edidentifier.h>
__CIDENT_RCSID(cr_env_c,"$Id: env.c,v 1.27 2015/02/19 00:17:11 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: env.c,v 1.27 2015/02/19 00:17:11 ayoung Exp $
 * Environment related functions.
 *
 *
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

#include <editor.h>
#include <edenv.h>
#include <libstr.h>

#if defined(_VMS) || !defined(HAVE_SETENV)
#define NEED_SETENV_CACHE
#endif

#if defined(NEED_SETENV_CACHE)
static Head_p               hd_env = NULL;

static List_p               glookup(const char *);
static void                 greplace(char *arg);


void
env_init(void)
{
    if (NULL == hd_env) {
        hd_env = ll_init();
    }
}


void
env_shutdown(void)
{
    if (hd_env) {
        ll_clear2(hd_env, free);
        ll_free(hd_env);
        hd_env = NULL;
    }
}


static List_p
glookup(const char *name)
{
    int len;
    List_p lp;

    if (hd_env) {
        for (len = 0; name[len] != '=' && name[len];) {
            ++len;
        }

        for (lp = ll_first(hd_env); lp; lp = ll_next(lp)) {
            const char *lname = (const char *) ll_elem(lp);

            if (0 == strncmp(name, lname, len) && '=' == lname[len]) {
                return lp;
            }
        }
    }
    return NULL;
}


static void
greplace(char *arg)
{
    if (NULL == hd_env) {
        env_init();
    }

    if (arg && hd_env) {
        List_p lp;

        if (NULL != (lp = glookup(arg))) {
            free(ll_elem(lp));
            ll_delete(lp);
        }

        if (NULL == hd_env) {
            env_init();
        }

        ll_append(hd_env, arg);
    }
}


#else   /*NEED_SETENV_CACHE*/
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

#if defined(WIN32) && defined(_MSC_VER)
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

#else   /*_VMS*/
    List_p lp = glookup(name);
    const char *cp;

    if (NULL == lp) {
        return getenv(name);
    }

    for (cp = (char *) ll_elem(lp); *cp != '=' && *cp;) {
        ++cp;
    }
    return *cp == '=' ? cp + 1 : cp;
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
 *      The envirnoment value described by 'arg' is copied by this function.
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
