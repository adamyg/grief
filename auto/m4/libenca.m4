dnl $Id: libenca.m4,v 1.2 2024/05/02 14:34:31 cvsuser Exp $
dnl libenca support autoconf
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl

AC_DEFUN([CF_WITH_LIBENCA],[
	AC_MSG_CHECKING(whether to enable use of enca)
	AC_ARG_WITH(enca,
		[  --with-enca            enable libenca use (default=no)],
		[case "$withval" in
			yes)  with_libenca=yes;;
			no)   with_libenca=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libenca);;
		esac],
		[with_libenca=no])
	AC_MSG_RESULT($with_libenca)

	AC_ARG_WITH(enca-h,
		[  --with-enca-h=DIR      libenca header location],
		[LIBENCA_H_DIR=$withval; with_libenca=yes])

	AC_ARG_WITH(enca-lib,
		[  --with-enca-lib=DIR    libenca library location],
		[LIBENCA_LIB_DIR=$withval; with_libenca=yes])

	if test "$with_libenca" = yes; then
		CF_LIB_ENCA
	fi
])


dnl ---------------------------------------------------------------------------
dnl libenca
dnl

AC_DEFUN([CF_LIB_ENCA],[
	AC_CACHE_CHECK(for libenca, cf_cv_libenca, [
		cf_cv_libenca=no
		cf_save_LIBS="$LIBS"
		LIBS="$LIBS -lenca"
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdlib.h>
#include <enca.h>]],[[
	EncaAnalyser a;
	EncaEncoding e; 
	a = enca_analyser_alloc("utf-8");
	enca_set_multibyte(a, 1);
	enca_set_ambiguity(a, 1);
	enca_set_filtering(a, 1);
	enca_set_garbage_test(a, 1);
	enca_set_termination_strictness(a, 0);
	enca_set_threshold(a, 1);
	e = enca_analyse_const(a, "helloworld", 10);
	enca_analyser_free(a);]])],
			[cf_cv_libenca=yes],
			[cf_cv_libenca=no])
	])
	LIBS="$cf_save_LIBS"

	LIBENCA=
	if test "$cf_cv_libenca" = "yes" ; then
		AC_DEFINE(HAVE_LIBENCA, 1,
			[Define if you have libenca.])
		AC_DEFINE(HAVE_ENCA_H, 1,
			[Define if you have <enca.h>.])
		AC_CHECK_HEADERS(enca.h)
		LIBENCA="-lenca"
	fi
	AC_SUBST(LIBENCA)
])

dnl
