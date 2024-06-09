dnl $Id: libutf8.m4,v 1.3 2024/05/02 14:34:32 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     libutf8 support
dnl

AC_DEFUN([CF_WITH_LIBUTF8],[
	AC_MSG_CHECKING(whether to enable use of libutf8)
	AC_ARG_WITH(libutf8,
		[  --with-libutf8          enable libutf8 use (default=no)],
		[case "$withval" in
			yes) with_libutf8=yes;;
			no)  with_libutf8-no;;
			*)   AC_MSG_ERROR(bad value ${withval} for --with-libuf8);;
		esac],
		[with_libutf8=no])
	AC_MSG_RESULT($with_libutf8)
	if test "$with_libutf8" = yes; then
		AC_DEFINE(NEED_LIBUTF8, 1,
			[Define to enable libutf8 functionality])
	fi

	LIBUTF8_H_DIR=""
	AC_ARG_WITH(libutf8-h,
		[  --with-libutf8-h=DIR    libutf8 header location],
		[LIBUTF8_H_DIR=$withval; with_libutf8=yes])

	LIBUTF8_LIB_DIR=""
	AC_ARG_WITH(libutf8-lib,
		[  --with-libutf8-lib=DIR  libutf8 library location],
		[LIBUTF8_LIB_DIR=$withval; with_libutf8=yes])
	if test x"$with_libutf8" = xyes; then
		CF_LIB_UTF8
	fi
])


dnl ---------------------------------------------------------------------------
dnl libutf8
dnl
dnl     Determine utf8 library requirements.
dnl
AC_DEFUN([CF_LIB_UTF8],[
	if test -n "$LIBUTF8_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit utf8.h path $LIBUTF8_H_DIR])
		AC_CHECK_HEADERS($LIBUTF8_H_DIR/utf8,h,[libutf8_header=yes],
			[AC_MSG_ERROR([utf8.h not found a specified location])])
	else
		AC_CHECK_HEADERS(utf8.h,[libutf8_header=yes])
	fi

	AC_CACHE_CHECK(for multibyte character support using libutf8, cf_cv_lib_utf8, [
		cf_save_INC=$INC
		cf_save_LIBS=$LIB
		if test -n "$LIBUTF8_H_DIR"; then
			INC="-I$LIBUTF8_LIB_DIR $INC"
		fi
		if test -n "$LIBUTF8_LIB_DIR"; then
			LIBS="-L$LIBUTF8_LIB_DIR $LIBS"
		fi
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[
	#include <utf8.h>]],[[
	struct utf8_encode_state dctx;
	struct utf8_decode_state dctx;
	utf8_encoder(&ectx);
	utf8_decoder(&dctx);]])],utf8,
				[cf_cv_lib_utf8=yes],
				[cf_cv_lib_utf8=no])
		])
		LIBS=$cf_save_LIBS
		INC=$cf_save_INC
	])

	if test "$cf_cv_lib_utf8" = "yes" ; then
		EXTRALIBS="-lutf8 $EXTRALIBS"
		AC_DEFINE([HAVE_LIBUTF8], 1, [Have libutf8])
		AC_DEFINE([HAVE_UTF8_H], 1, [utf8.h, libutf8 interface header])
	fi
])dnl
