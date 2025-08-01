/*	$NetBSD: citrus_module.c,v 1.13 2018/01/04 20:57:28 kamil Exp $	*/

/*-
 * Copyright (c)1999, 2000, 2001, 2002 Citrus Project,
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Paul Kranenburg.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: citrus_module.c,v 1.13 2018/01/04 20:57:28 kamil Exp $");
#endif /* LIBC_SCCS and not lint */

#include "namespace.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <paths.h>
#include "citrus_namespace.h"
#include "citrus_bcs.h"
#include "citrus_module.h"

#if defined(_I18N_DYNAMIC)
#elif defined(_I18N_STATIC)
#elif defined(_I18N_NONE)
#else
#error Neither DYNAMIC, STATIC or NONE ...
#endif

#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>

#ifdef _I18N_DYNAMIC

static unsigned int _getdewey(int [], char *);
static int _cmpndewey(int [], unsigned int, int [], unsigned int);
static const char *_findshlib(char *, int *, int *);

static const char *_pathI18nModule = NULL;

/* from libexec/ld.aout_so/shlib.c */
#undef major
#undef minor
#define MAXDEWEY	3	/*ELF*/

static unsigned int
_getdewey(int dewey[], char *cp)
{
	unsigned int	i, n;

	_DIAGASSERT(dewey != NULL);
	_DIAGASSERT(cp != NULL);

	for (n = 0, i = 0; i < MAXDEWEY; i++) {
		if (*cp == '\0')
			break;

#if defined(WIN32)					/* [.-]<major>.<minor> */
		if (*cp == '.' || (n == 0 && *cp == '-')) cp++;
#else
		if (*cp == '.') cp++;
#endif
		if (*cp < '0' || '9' < *cp)
			return 0;

		dewey[n++] = (int)_bcs_strtol(cp, &cp, 10);
	}

	return n;
}

/*
 * Compare two dewey arrays.
 * Return -1 if `d1' represents a smaller value than `d2'.
 * Return  1 if `d1' represents a greater value than `d2'.
 * Return  0 if equal.
 */
static int
_cmpndewey(int d1[], unsigned int n1, int d2[], unsigned int n2)
{
	register unsigned int	i;

	_DIAGASSERT(d1 != NULL);
	_DIAGASSERT(d2 != NULL);

	for (i = 0; i < n1 && i < n2; i++) {
		if (d1[i] < d2[i])
			return -1;
		if (d1[i] > d2[i])
			return 1;
	}

	if (n1 == n2)
		return 0;

	if (i == n1)
		return -1;

	if (i == n2)
		return 1;

	/* XXX cannot happen */
	return 0;
}

static const char *
_findshlib(char *name, int *majorp, int *minorp)
{
	int		dewey[MAXDEWEY];
	unsigned int	ndewey;
	int		tmp[MAXDEWEY];
	int		i;
	int		len;
	char		*lname;
	static char	path[PATH_MAX];
	int		major, minor;
	const char	*search_dirs[1];
	const int	n_search_dirs = 1;

	_DIAGASSERT(name != NULL);
	_DIAGASSERT(majorp != NULL);
	_DIAGASSERT(minorp != NULL);

	major = *majorp;
	minor = *minorp;
	path[0] = '\0';
	search_dirs[0] = _pathI18nModule;
	len = strlen(name);
	lname = name;

	ndewey = 0;

	for (i = 0; i < n_search_dirs; i++) {
		DIR		*dd = opendir(search_dirs[i]);
		struct dirent	*dp;
		int		found_dot_a = 0;
		int		found_dot_so = 0;

		if (dd == NULL)
			continue;

		while ((dp = readdir(dd)) != NULL) {
			unsigned int n = 0;

#if defined(WIN32)
			/*
			 *      <module>.dll.<major>[.<minor>]
			 *   or <module>.<major>[.<minor>].dll
			 *   or <module>.dll
			 */
			if (dp->d_namlen < len + 4)
				continue;
			if (_strnicmp(dp->d_name, lname, (size_t)len) != 0)
				continue;
			if (0 == strncmp(dp->d_name+len, ".dll.", 5)) {
				if ((n = _getdewey(tmp, dp->d_name + len + 5)) == 0)
					continue;
			} else {
				char version[80];

				if (0 == _stricmp(dp->d_name + (dp->d_namlen - 5), ".dll"))
					continue;
				if (dp->d_namlen == (len + 4)) {
					// no version, assume <major>.0
					tmp[0] = major, tmp[1] = 0;
				} else {
					// major, with option minor
					const size_t versionlen = (dp->d_namlen - (len + 4));

					if (versionlen >= sizeof(version))
						continue;
					memcpy(version, dp->d_name + len, versionlen);
					version[versionlen] = 0;
					tmp[0] = -1, tmp[1] = 0;
					if ((n = _getdewey(tmp, version)) == 0) {
						if (-1 == tmp[0]) {
							//no major
							continue;
						}
					}
				}
			}
#else
			/*
			 *  <module>.so.<version>
			 */
			if (dp->d_namlen < len + 4)
				continue;
			if (strncmp(dp->d_name, lname, (size_t)len) != 0)
				continue;
			if (strncmp(dp->d_name+len, ".so.", 4) != 0)
				continue;

			if ((n = _getdewey(tmp, dp->d_name+len+4)) == 0)
				continue;
#endif //WIN32

			if (major != -1 && found_dot_a)
				found_dot_a = 0;

			/* XXX should verify the library is a.out/ELF? */

			if (major == -1 && minor == -1) {
				goto compare_version;
			} else if (major != -1 && minor == -1) {
				if (tmp[0] == major)
					goto compare_version;
			} else if (major != -1 && minor != -1) {
				if (tmp[0] == major) {
					if (n == 1 || tmp[1] >= minor)
						goto compare_version;
				}
			}

			/* else, this file does not qualify */
			continue;

		compare_version:
			if (_cmpndewey(tmp, n, dewey, ndewey) <= 0)
				continue;

			/* We have a better version */
			found_dot_so = 1;
			snprintf(path, sizeof(path), "%s/%s", search_dirs[i],
			    dp->d_name);
			found_dot_a = 0;
			bcopy(tmp, dewey, sizeof(dewey));
			ndewey = n;
			*majorp = dewey[0];
			*minorp = dewey[1];
		}
		closedir(dd);

		if (found_dot_a || found_dot_so)
			/*
			 * There's a lib in this dir; take it.
			 */
			return path[0] ? path : NULL;
	}

	return path[0] ? path : NULL;
}

void *
_citrus_find_getops(_citrus_module_t handle, const char *modname,
		    const char *ifname)
{
	char name[PATH_MAX];
	void *p;

	_DIAGASSERT(handle != NULL);
	_DIAGASSERT(modname != NULL);
	_DIAGASSERT(ifname != NULL);

	snprintf(name, sizeof(name), _C_LABEL_STRING("_citrus_%s_%s_getops"),
	    modname, ifname);
	p = dlsym((void *)handle, name);
#if defined(WIN32) && defined(__LEADING_UNDERSCORE)
	if (!p) {
		snprintf(name, sizeof(name), "_citrus_%s_%s_getops",
		    modname, ifname);
		p = dlsym((void *)handle, name);
	}
#endif //WIN32
	return p;
}

int
_citrus_load_module(_citrus_module_t *rhandle, const char *encname)
 {
	const char *p;
	char path[PATH_MAX];
	int maj, min;
	void *handle;

	_DIAGASSERT(rhandle != NULL);

	if (_pathI18nModule == NULL) {
		p = getenv("PATH_I18NMODULE");
#if defined(WIN32)
		if (p == NULL) {
			p = _PATH_I18NMODULE;
			if (NULL == p)
				p = _DEF_PATH_I18NMODULE;
		}
#else
		if (p == NULL || issetugid()) {
			_pathI18nModule = _PATH_I18NMODULE;
#ifdef MLIBDIR
			p = strrchr(_pathI18nModule, '/');
			if (p != NULL) {
				snprintf(path, sizeof(path), "%.*s/%s/%s",
				    (int)(p - _pathI18nModule),
				    _pathI18nModule, MLIBDIR, p + 1);
				p = path;
			} else
				p = NULL;
#endif
		}
#endif //WIN32
		if (p != NULL) {
			_pathI18nModule = strdup(p);
			if (_pathI18nModule == NULL)
				return ENOMEM;
		}
	}

	(void)snprintf(path, sizeof(path), "lib%s", encname);
	maj = I18NMODULE_MAJOR;
	min = -1;
	p = _findshlib(path, &maj, &min);
	if (!p)
		return (EINVAL);
	handle = dlopen(p, RTLD_LAZY);
	if (!handle)
		return (EINVAL);

	*rhandle = (_citrus_module_t)handle;

	return (0);
}

void
_citrus_unload_module(_citrus_module_t handle)
{
	if (handle)
		dlclose((void *)handle);
}
#else
/* !_I18N_DYNAMIC */

void *
/*ARGSUSED*/
_citrus_find_getops(_citrus_module_t handle, const char *modname,
		    const char *ifname)
{
	return (NULL);
}

int
/*ARGSUSED*/
_citrus_load_module(_citrus_module_t *rhandle, char const *modname)
{
	return (EINVAL);
}

void
/*ARGSUSED*/
_citrus_unload_module(_citrus_module_t handle)
{
}
#endif
