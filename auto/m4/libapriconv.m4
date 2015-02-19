dnl $Id: libapriconv.m4,v 1.1 2010/10/07 16:45:34 cvsuser Exp $
dnl apriconv autoconf support
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl

AC_DEFUN([CF_WITH_LIBAPRICONV],[
	AC_MSG_CHECKING(whether to enable use of apriconv)
	AC_ARG_WITH(apriconv,
		[  --with-apriconv         enable libapriconv use (default=no)],
		[case "$withval" in
			yes)  with_libapriconv=yes;;
			no)   with_libapriconv=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libapriconv);;
		esac],
		[with_libapriconv=no])
	AC_MSG_RESULT($with_libapriconv)
	if test "$with_libapriconv" = yes; then
		AC_DEFINE(NEED_LIBAPRICONV, 1,
				[Define to enable apriconv functionality])
	fi

	AC_ARG_WITH(apriconv-h,
		[  --with-apriconv-h=DIR   libapriconv header location],
		[LIBAPRICONV_H_DIR=$withval; with_libapriconv=yes])

	AC_ARG_WITH(apriconv-lib,
		[  --with-apriconv-lib=DIR libapriconv library location],
		[LIBAPRICONV_LIB_DIR=$withval; with_libapriconv=yes])

	if test "$with_libapriconv" = yes; then
		CF_LIB_APRICONV
	fi
])


dnl ---------------------------------------------------------------------------
dnl libapriconv
dnl
dnl     Determine apriconv library requirements.
dnl

AC_DEFUN([CF_LIB_APRICONV],[
	if test -n "$LIBAPRICONV_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit apr_iconv.h path $LIBAPRICONV_H_DIR])
		AC_CHECK_HEADERS($LIBAPRICONV_H_DIR/apriconv,h,[libapriconv_header=yes],
			[AC_MSG_ERROR([apr_iconv.h not found a specified location])])
	else
		AC_CHECK_HEADERS(apr_iconv.h,[libapriconv_header=yes])
	fi

	AC_CACHE_CHECK(for apr_iconv_open, cf_cv_libapriconv, [
		cf_save_INC="$INC"
		cf_save_LIBS="$LIBS"
		if test -n "$LIBAPRICONV_H_DIR"; then
			INC="-I$LIBAPRICONV_LIB_DIR $INC"
		fi
		if test -n "$LIBAPRICONV_LIB_DIR"; then
			LIBS="-L$LIBAPRICONV_LIB_DIR $LIBS"
		fi
		LIBS="$LIBS -lapriconv"
		AC_TRY_LINK([
#include <stdlib.h>
#include <apr_iconv.h>],
		[apri_conv_t x = apr_iconv_open("", "");
		apriconv(x, NULL, NULL, NULL, NULL);
		apriconv_close(x);],
			[cf_cv_libapriconv=yes],
			[cf_cv_libapriconv=no])
		])
		LIBS="$cf_save_LIBS"
		INC="$cf_save_INC"
	])

	LIBAPRICONV=
	if test "$cf_cv_libapriconv" = "yes" ; then
		if test -n "$LIBAPRICONV_LIB_DIR"; then
			LIBAPRICONV="-L$LIBAPRICONV_LIB_DIR -lapriconv"
		else
			LIBAPRICONV="-lapriconv"
		fi

		AC_DEFINE(HAVE_LIBAPRICONV, 1,
			[Define if you have <apr_iconv.h> and libapriconv.])
		AC_DEFINE(HAVE_APR_ICONV_H)
	fi
])

dnl
