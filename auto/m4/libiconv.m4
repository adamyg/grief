dnl $Id: libiconv.m4,v 1.4 2013/03/23 00:07:31 cvsuser Exp $
dnl iconv autoconf
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl

AC_DEFUN([CF_WITH_LIBICONV],[
	AC_MSG_CHECKING(whether to enable use of iconv)
	AC_ARG_WITH(iconv,
		[  --with-iconv            enable libiconv use (default=no)],
		[case "$withval" in
			yes)  with_libiconv=yes;;
			no)   with_libiconv=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libiconv);;
		esac],
		[with_libiconv=no])
	AC_MSG_RESULT($with_libiconv)
	if test "$with_libiconv" = yes; then
		AC_DEFINE(NEED_LIBICONV, 1,
				[Define to enable iconv functionality])
	fi

	AC_ARG_WITH(iconv-h,
		[  --with-iconv-h=DIR      libiconv header location],
		[LIBICONV_H_DIR=$withval; with_libiconv=yes])

	AC_ARG_WITH(iconv-lib,
		[  --with-iconv-lib=DIR    libiconv library location],
		[LIBICONV_LIB_DIR=$withval; with_libiconv=yes])

	LIBICONV=""
	if test "$with_libiconv" = yes; then
		CF_LIB_ICONV
	fi
	AC_SUBST(LIBICONV)
])


dnl ---------------------------------------------------------------------------
dnl libiconv
dnl
dnl     Determine iconv library requirements, being either within libc or seperate.
dnl

AC_DEFUN([CF_LIB_ICONV],[
	AC_CACHE_CHECK(for libiconv, cf_cv_libiconv, [
		cf_cv_libiconv=no
		AC_TRY_LINK([
#include <stdlib.h>
#include <iconv.h>],
		[iconv_t x = iconv_open("", "");
		iconv(x, NULL, NULL, NULL, NULL);
		iconv_close(x);],
			[cf_cv_libiconv=yes],
			[cf_cv_libiconv=no])

		if test "$cf_cv_libiconv" != yes ; then
			cf_save_LIBS="$LIBS"
			LIBS="$LIBS -liconv"
			AC_TRY_LINK([
#include <stdlib.h>
#include <iconv.h>],
			[iconv_t x = iconv_open("", "");
			iconv(x, NULL, NULL, NULL, NULL);
			iconv_close(x);],
				[cf_cv_libiconv="need libiconv"],
				[cf_cv_libiconv=no])
			LIBS="$cf_save_LIBS"
		fi
	])

	if test "$cf_cv_libiconv" != "no" ; then
		if test "$cf_cv_libiconv" = "need libiconv" ; then
			LIBICONV="-liconv"
		fi

		AC_DEFINE(HAVE_LIBICONV, 1,
			[Define if you have libiconv.])
		AC_DEFINE(HAVE_ICONV_H, 1,
			[Define if you have <iconv.h>.])

		# iconv, libcharset
		AC_CHECK_HEADERS(charset.h)
		AC_CHECK_HEADERS(localcharset.h)
		AC_CHECK_LIB(charset, locale_charset)
		AC_CHECK_FUNCS(locale_charset)
	fi
])dnl
