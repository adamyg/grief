dnl $Id: libmagic.m4,v 1.5 2017/01/30 04:03:27 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     libmagic options
dnl

AC_DEFUN([CF_WITH_LIBMAGIC],[
	AC_MSG_CHECKING(whether to enable use of magic)
	AC_ARG_WITH(magic,
		[  --with-magic            enable libmagic use (default=yes)],
		[case "$withval" in
			yes)  with_libmagic=yes;;
			no)   with_libmagic=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libmagic);;
		esac],
		[with_libmagic=yes])
	AC_MSG_RESULT($with_libmagic)
	if test "$with_libmagic" = no; then
		AC_DEFINE(NO_LIBMAGIC, 1,
				[Define to disable magic functionality])
	fi

	AC_ARG_WITH(magic-h,
		[  --with-magic-h=DIR      libmagic header location],
		[LIBMAGIC_H_DIR=$withval; with_libmagic=yes])

	AC_ARG_WITH(magic-lib,
		[  --with-magic-lib=DIR    libmagic library location],
		[LIBMAGIC_LIB_DIR=$withval; with_libmagic=yes])

	if test "$with_libmagic" = yes; then
		CF_LIB_MAGIC
	fi
])


dnl ---------------------------------------------------------------------------
dnl libmagic with optional libz requirement
dnl

AC_DEFUN([CF_LIB_MAGIC],[
	if test -n "$LIBMAGIC_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit magic.h path $LIBMAGIC_H_DIR])
		AC_CHECK_HEADERS($LIBMAGIC_H_DIR/magic,h,[
			CFLAGS="$CFLAGS -I$LIBMAGIC_H_DIR"
			libmagic_header=yes],
			[AC_MSG_ERROR([magic.h not found at specified location])])
	else
		AC_CHECK_HEADERS(magic.h,[libmagic_header=yes])
	fi

	AC_CACHE_CHECK(for magic_open, cf_cv_libmagic,[
		cf_save_LIBS="$LIBS"
		AC_TRY_LINK([#include <magic.h>],
			[magic_t x = magic_open(MAGIC_NONE); magic_close(x);],
			[cf_cv_libmagic=std],[
			LIBS="$cf_save_LIBS -lmagic"
			AC_TRY_LINK([#include <magic.h>],
				[magic_t x = magic_open(MAGIC_NONE); magic_close(x);],
				[cf_cv_libmagic=yes],[
				LIBS="$cf_save_LIBS -lmagic -lz"
				AC_TRY_LINK([#include <magic.h>],
					[magic_t x = magic_open(MAGIC_NONE); magic_close(x);],
					[cf_cv_libmagic="yes, libmagic with libz"],
					[cf_cv_libmagic=no])
			])
		])
		LIBS="$cf_save_LIBS"
	])

	LIBMAGIC=
	if test "$LIBZ" = "" ; then
		LIBZ=
	fi
	if test "$cf_cv_libmagic" != "no" ; then
		if test "$cf_cv_libmagic" = "yes" ; then
			LIBMAGIC="-lmagic"

		elif test "$cf_cv_libmagic" = "yes, along with libz" ; then
			LIBMAGIC="-lmagic -lz"
			LIBZ="-lz"
			AC_DEFINE([HAVE_LIBZ], 1, [Define if libz is available])
		fi
		AC_DEFINE(HAVE_LIBMAGIC, 1, [<magic.h> and magic_open().])
		AC_DEFINE(HAVE_MAGIC_H, 1, [magic.h available.])
	fi
	AC_SUBST(LIBMAGIC)
	AC_SUBST(LIBZ)
])

dnl end

