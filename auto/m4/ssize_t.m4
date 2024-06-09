dnl $Id: ssize_t.m4,v 1.3 2024/05/02 14:34:32 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl Determine whether ssize_t is available
dnl 

AC_DEFUN([CF_TYPE_SSIZE_T],[
	AC_CACHE_CHECK([for ssize_t], [cf_cv_ssize_t],
		[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>]],
			[[int x = sizeof (ssize_t *) + sizeof (ssize_t);
			return !x;]])],
		[cf_cv_ssize_t=yes], [cf_cv_ssize_t=no])])
	if test $cf_cv_ssize_t = no; then
		AC_DEFINE([ssize_t], [int],
			[Define as a signed type of the same size as size_t.])
	else
		AC_DEFINE(HAVE_SSIZE_T, 1,
			[Define if you have ssize_t.])
	fi
])dnl

