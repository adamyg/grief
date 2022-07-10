#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_hunspell_c,"$Id: w32_hunspell.c,v 1.20 2022/07/08 14:01:00 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 hunspell dynamic loader.
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 *
 * Notice: Portions of this text are reprinted and reproduced in electronic form. from
 * IEEE Portable Operating System Interface (POSIX), for reference only. Copyright (C)
 * 2001-2003 by the Institute of. Electrical and Electronics Engineers, Inc and The Open
 * Group. Copyright remains with the authors and the original Standard can be obtained
 * online at http://www.opengroup.org/unix/online.html.
 * ==extra==
 */

#if defined(_WIN32) || defined(WIN32)

#include <editor.h>
#include <eddebug.h>
#include <libstr.h>

#include "win32_include.h"
#include "win32_hunspell.h"

#if defined(LIBW32_STATIC)
#define DO_TRACE_LOG                            // trace_log()
#endif

#if defined(HAVE_LIBHUNSPELL_DLL)
#define DLLLINKAGE      __cdecl
#else
#define DLLLINKAGE      /**/
#endif
typedef void *          (DLLLINKAGE * initialisefn_t)(const char *aff, const char *dic);
typedef void *          (DLLLINKAGE * initialise_keyfn_t)(const char *aff, const char *dic, const char *key);
typedef void            (DLLLINKAGE * uninitialisefn_t)(void *handle);
typedef const char *    (DLLLINKAGE * get_dic_encodingfn_t)(void *handle);
typedef int             (DLLLINKAGE * spellfn_t)(void *handle, const char *word);
typedef int             (DLLLINKAGE * suggestfn_t)(void *handle, const char *word, char ***sugglist);
typedef void            (DLLLINKAGE * free_listfn_t)(void *handle, char ***sugglist, int suggcnt);
typedef int             (DLLLINKAGE * addfn_t)(void *handle, const char *word);
typedef int             (DLLLINKAGE * add_with_affixfn_t)(void *handle, const char *work, const char *affix);

static HINSTANCE        x_hunspelldll;

static initialisefn_t   x_initialise;
static initialise_keyfn_t x_initialise_key;
static uninitialisefn_t x_uninitialise;
static get_dic_encodingfn_t x_get_dic_encoding;
static spellfn_t        x_spell;
static suggestfn_t      x_suggest;
static free_listfn_t    x_free_list;
static addfn_t          x_add;
static add_with_affixfn_t x_add_with_affix;

static void             strxcopy(char *dest, const char *src, int len);
static FARPROC          hunspell_resolve(const char *name);


LIBW32_API int
w32_hunspell_connect(int verbose)
{
   static const char *hunspellnames[] = {
#if defined(HAVE_LIBHUNSPELL_DLL)
        HAVE_LIBHUNSPELL_DLL,                   // see: config.h
#else
        "libhunspell.dll",
        "hunspell.dll",
#endif
        NULL
        };
    char fullname[1024], *end;
    const char *name;
    unsigned i;

    if (x_hunspelldll) {
        return TRUE;
    }

    fullname[0] = 0;
    GetModuleFileNameA(GetModuleHandle(NULL), fullname, sizeof(fullname));
    if (NULL != (end = (char *)strrchr(fullname, '\\'))) {
        *++end = 0;
    }

    for (i = 0; NULL != (name = hunspellnames[i]); ++i) {
#if defined(DO_TRACE_LOG)
        trace_log("hunspell_dll(%s)\n", name);
#endif
        if (end) {
            strxcopy(end, name, sizeof(fullname) - (end - fullname));
            if (NULL != (x_hunspelldll = LoadLibraryA(fullname))) {
                break;                          // relative to task
            }
        }
        if (NULL != (x_hunspelldll = LoadLibraryA(name))) {
            break;                              // general, within search path
        }
    }

    if (NULL == x_hunspelldll) {                // environment override
        const char *env;

        if (NULL != (env = getenv("HUNSPELLDLL"))) {
#if defined(DO_TRACE_LOG)
            trace_log("hunspell_dll(%s)\n", env);
#endif
            x_hunspelldll = LoadLibraryA(env);
        }
    }

    if (NULL == x_hunspelldll) {
        if (verbose) {
            const DWORD err = GetLastError();
            char buffer[128];

            (void) _snprintf(buffer, sizeof(buffer), "Unable to load %s, rc: %ld", "hunspell.dll", (long)err);
            buffer[sizeof(buffer) - 1] = 0;
            MessageBoxA(0, buffer, "Error", MB_OK);
        }
        w32_hunspell_shutdown();
        x_hunspelldll = (HINSTANCE)-1;
        return FALSE;
    }

    fullname[0] = 0;                            // resolve symbols
    GetModuleFileNameA(x_hunspelldll, fullname, sizeof(fullname));

    x_initialise       = (initialisefn_t)       hunspell_resolve("initialize");
    x_initialise_key   = (initialise_keyfn_t)   hunspell_resolve("initialize_key");
    x_uninitialise     = (uninitialisefn_t)     hunspell_resolve("uninitialize");
    x_get_dic_encoding = (get_dic_encodingfn_t) hunspell_resolve("get_dic_encoding");
    x_spell            = (spellfn_t)            hunspell_resolve("spell");
    x_suggest          = (suggestfn_t)          hunspell_resolve("suggest");
    x_free_list        = (free_listfn_t)        hunspell_resolve("free_list");
    x_add              = (addfn_t)              hunspell_resolve("add");
    x_add_with_affix   = (add_with_affixfn_t)   hunspell_resolve("add_with_affix");

#if defined(DO_TRACE_LOG)
    trace_log("hunspell Functions (%p, %s)\n", x_hunspelldll, fullname);
    trace_log("\thunspell_initialise:       %p\n", x_initialise);
    trace_log("\thunspell_initialise_key:   %p\n", x_initialise_key);
    trace_log("\thunspell_uninitialise:     %p\n", x_uninitialise);
    trace_log("\thunspell_get_dic_encoding: %p\n", x_get_dic_encoding);
    trace_log("\thunspell_spell:            %p\n", x_spell);
    trace_log("\thunspell_suggest:          %p\n", x_suggest);
    trace_log("\thunspell_free_list:        %p\n", x_free_list);
    trace_log("\thunspell_add:              %p\n", x_add);
    trace_log("\thunspell_add_with_affix:   %p\n", x_add_with_affix);
#endif

    if (NULL == x_initialise ||
            NULL == x_initialise_key ||
            NULL == x_uninitialise  ||
            NULL == x_get_dic_encoding ||
            NULL == x_spell ||
            NULL == x_suggest ||
            NULL == x_free_list ||
            NULL == x_add ||
            NULL == x_add_with_affix) {
        if (verbose) {
            char buffer[128];

            (void) _snprintf(buffer, sizeof(buffer), "Unable to resolve symbols from\n <%s>", fullname);
            buffer[sizeof(buffer) - 1] = 0;
            MessageBoxA(0, buffer, "Error", MB_OK);
        }
        w32_hunspell_shutdown();
        x_hunspelldll = (HINSTANCE)-1;
        return FALSE;
    }
    return TRUE;
}


static void
strxcopy(char *dest, const char *src, int len)
{
    (void) strncpy(dest, src, len);
    if (len > 0) {
        dest[len - 1] = '\0';
    }
}


LIBW32_API void
w32_hunspell_shutdown(void)
{
    x_initialise = NULL;
    x_initialise_key = NULL;
    x_uninitialise = NULL;
    x_get_dic_encoding = NULL;
    x_spell = NULL;
    x_suggest = NULL;
    x_free_list = NULL;
    x_add = NULL;
    x_add_with_affix = NULL;
    if (x_hunspelldll && (HINSTANCE)-1 != x_hunspelldll) {
        FreeLibrary(x_hunspelldll);
        x_hunspelldll = 0;
    }
}


static FARPROC
hunspell_resolve(const char *name)
{
    static const char *hunspellprefix[] = {
        //  o MSC, Underscore character (_) is prefixed to names, except when exporting __cdecl
        //  	functions that use C linkage.
        //  o WATCOMC, Underscore character (_) is prefixed in all cases.
        //
        "_hunspell_",                           // __cdecl
        "hunspell_",                            // __cdecl
        "libhunspell_",
        NULL
        };
    const char *prefix;
    char fullname[128];
    unsigned i;

    for (i = 0; NULL != (prefix = hunspellprefix[i]); ++i) {
        FARPROC proc;

        _snprintf(fullname, sizeof(fullname), "%s%s", prefix, name);
        fullname[sizeof(fullname) - 1] = 0;
        if (0 != (proc = GetProcAddress(x_hunspelldll, fullname))) {
            return proc;
        }
    }
    return 0;
}


void *
w32_hunspell_initialise(const char *aff, const char *dic)
{
    (void) w32_hunspell_connect(TRUE);
    if (x_initialise) {
        return (* x_initialise)(aff, dic);
    }
    return NULL;
}


void *
w32_hunspell_initialise_key(const char *aff, const char *dic, const char *key)
{
    (void) w32_hunspell_connect(TRUE);
    if (x_initialise_key) {
        return (* x_initialise_key)(aff, dic, key);
    }
    return NULL;
}


LIBW32_API void
w32_hunspell_uninitialise(void *handle)
{
    if (x_uninitialise) {
        (* x_uninitialise)(handle);
    }
}


LIBW32_API const char *
w32_hunspell_get_dic_encoding(void *handle)
{
    if (x_get_dic_encoding) {
        return (* x_get_dic_encoding)(handle);
    }
    return "";
}


LIBW32_API int
w32_hunspell_spell(void *handle, const char *word)
{
    if (x_spell) {
        return (* x_spell)(handle, word);
    }
    return -1;
}


LIBW32_API int
w32_hunspell_suggest(void *handle, char ***sugglist, const char *word)
{
    if (x_suggest) {
        return (* x_suggest)(handle, word, sugglist);
    }
    return -1;
}


LIBW32_API void
w32_hunspell_free_list(void *handle, char ***sugglist, int suggcnt)
{
    if (x_free_list) {
        (* x_free_list)(handle, sugglist, suggcnt);
    }
}


LIBW32_API int
w32_hunspell_add(void *handle, const char *word)
{
    if (x_add) {
        return (* x_add)(handle, word);
    }
    return -1;
}


LIBW32_API int
w32_hunspell_add_with_affix(void *handle, const char *word, const char *affix)
{
    if (x_add_with_affix) {
        return (* x_add_with_affix)(handle, word, affix);
    }
    return -1;
}

#endif  /*WIN32*/
