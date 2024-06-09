dnl $Id: string.m4,v 1.2 2024/05/02 14:34:32 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     string functions
dnl

AC_DEFUN(CF_CHECK_STRLCPY,[
	AC_MSG_CHECKING(for strlcpy)
	AC_CACHE_VAL(cf_cv_have_strlcpy,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <string.h>
]],[[
char buf[32]={0};
strlcpy(buf, "test", sizeof(buf));
]])],
		[cf_cv_have_strlcpy=yes],
		[cf_cv_have_strlcpy=no])
	])dnl
	AC_MSG_RESULT($cf_cv_have_strlcpy)
	test "$cf_cv_have_strlcpy" = yes && \
		AC_DEFINE([HAVE_STRLCPY], 1, [strlcpy() available.])
])dnl

AC_DEFUN(CF_CHECK_STRLCAT,[
	AC_MSG_CHECKING(for strlcat)
	AC_CACHE_VAL(cf_cv_have_strlcat,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <string.h>
]],[[
char buf[32]={0};
strlcat(buf, "test", sizeof(buf));
]])],
		[cf_cv_have_strlcat=yes],
		[cf_cv_have_strlcat=no])
	])dnl
	AC_MSG_RESULT($cf_cv_have_strlcat)
	test "$cf_cv_have_strlcat" = yes && \
		AC_DEFINE([HAVE_STRLCAT], 1, [strlcat() available.])
])dnl

dnl
