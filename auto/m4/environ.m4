dnl $Id: environ.m4,v 1.2 2024/05/02 14:34:31 cvsuser Exp $
dnl environ support.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl

AC_DEFUN([CF_ENVIRON],[
	AC_CHECK_FUNCS(setenv putenv)

	AC_MSG_CHECKING(for environ global)

	AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#if defined(__cplusplus)
extern "C" char **environ;
#else
extern char **environ;
#endif]],
		[[char **env = environ; while (*env) ++env;]])],
		[cf_cv_have_environ=environ;
			AC_DEFINE([HAVE_ENVIRON], 1, [have environ])],
		[cf_cv_have_environ=no])

	if test "$cf_cv_have_environ" = "environ" ; then
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdlib.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif]],
			[[char **env = environ; while (*env) ++env;]])],
			[cf_cv_need_environ=no],
			[cf_cv_need_environ=yes; 
				AC_DEFINE([NEED_EXTERN_ENVIRON], 1, [extern ])]
			)
	fi

	if test "$cf_cv_have_environ" = "no" ; then
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#if defined(__cplusplus)
extern "C" char **_environ;
#else
extern char **_environ;
#endif]],
			[[char **env = _environ; while (*env) ++env;]])],
			[cf_cv_have_environ=_environ;
				AC_DEFINE([HAVE__ENVIRON], 1, [extern ])],
			[cf_cv_have_environ=no]
			)
	fi

	if test "$cf_cv_have_environ" = "no" ; then
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#if defined(__cplusplus)
extern "C" char **__environ;
#else
extern char **__environ;
#endif]],
			[[char **env = __environ; while (*env) ++env;]])],
			[cf_cv_have_environ=__environ;
				AC_DEFINE([HAVE___ENVIRON], 1, [extern ])],
			[cf_cv_have_environ=no]
			)
	fi

	AC_MSG_RESULT($cf_cv_have_environ)
])
dnl
