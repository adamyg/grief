dnl $Id: libm.m4,v 1.3 2024/07/13 08:12:36 cvsuser Exp $
dnl libm autoconf
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl

AC_DEFUN([CF_NEED_LIBM],[
	AC_CACHE_CHECK(if -m is needed for math functions,cf_cv_need_libm,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <math.h>]],
		[[double x = log10(1.0); char buf[256]; sprintf(buf, "result = %g/%g\n", sin(x), tan(x));]])],
			[cf_cv_need_libm=no],
			[cf_cv_need_libm=yes])

		if test "$cf_cv_need_libm" != yes ; then
			cf_save_LIBS="$LIBS"
			LIBS="$LIBS -lm"
			AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <math.h>]],
		[[double x = log10(1.0); char buf[256]; sprintf(buf, "result = %g/%g\n", sin(x), tan(x));]])],
				[cf_cv_need_libm=yes],
				[cf_cv_need_libm=missing])
			LIBS="$cf_save_LIBS"
		fi
	])

	if test x"$cf_cv_need_libm" = xyes; then
		ifelse($1,,[LIBS="$LIBS -lm"],[$1=-lm])
	fi
])

dnl
