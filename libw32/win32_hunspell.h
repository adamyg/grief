#ifndef GR_WIN32_HUNSPELL_H_INCLUDED
#define GR_WIN32_HUNSPELL_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_win32_hunspell_h,"$Id: win32_hunspell.h,v 1.10 2019/03/15 23:12:22 cvsuser Exp $")
__CPRAGMA_ONCE

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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <sys/cdefs.h>
#include <unistd.h>

__CBEGIN_DECLS

LIBW32_API int              w32_hunspell_connect(int verbose);
LIBW32_API extern void      w32_hunspell_shutdown(void);

LIBW32_API void *           w32_hunspell_initialise(const char *aff, const char *dic);
LIBW32_API void *           w32_hunspell_initialise_key(const char *aff, const char *dic, const char *key);
LIBW32_API void             w32_hunspell_uninitialise(void *handle);
LIBW32_API const char *     w32_hunspell_get_dic_encoding(void *handle);
LIBW32_API int              w32_hunspell_spell(void *handle, const char *word);
LIBW32_API int              w32_hunspell_suggest(void *handle, char ***sugglist, const char *word);
LIBW32_API void             w32_hunspell_free_list(void *handle, char ***sugglist, int suggcnt);
LIBW32_API int              w32_hunspell_add(void *handle, const char *word);
LIBW32_API int              w32_hunspell_add_with_affix(void *handle, const char *word, const char *affix);

#if defined(WIN32_HUNSPELL_MAP)
typedef void *Hunhandle;

#define Hunspell_create(_aff, _dic) \
                        w32_hunspell_initialise(_aff, _dic)
#define Hunspell_create_key(_aff,_dic,_key) \
                        w32_hunspell_initialise_key(_aff, _dic, _key)
#define Hunspell_destroy(_handle) \
                        w32_hunspell_uninitialise(_handle)
#define Hunspell_get_dic_encoding(_handle) \
                        w32_hunspell_get_dic_encoding(_handle)
#define Hunspell_spell(_handle, _word) \
                        w32_hunspell_spell(_handle, _word)
#define Hunspell_suggest(_handle, _sugglist, _word) \
                        w32_hunspell_suggest(_handle, _sugglist, _word)
#define Hunspell_free_list(_handle, _sugglist, _suggcnt) \
                        w32_hunspell_free_list(_handle, _sugglist, _suggcnt)
#define Hunspell_add(_handle, _word) \
                        w32_hunspell_add(_handle, _word)
#define Hunspell_add_with_affix(_handle, _word, _affix) \
                        w32_hunspell_add_with_affix(_handle, _word, _affix)
#endif /*WIN32_HUNSPELL_MAP*/

__CEND_DECLS

#endif /*GR_WIN32_HUNSPELL_H_INCLUDED*/
