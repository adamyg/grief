dnl $Id: libz.m4,v 1.1 2017/01/30 04:03:27 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     libz options
dnl

AC_DEFUN([CF_WITH_LIBZ],[
	AC_MSG_CHECKING(whether to enable use of zlib)
	AC_ARG_WITH(zlib,
		[  --with-zlib             enable zlib use (default=yes)],
		[case "$withval" in
			yes)  with_zlib=yes;;
			no)   with_zlib=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-zlib);;
		esac],
		[with_zlib=yes])
	AC_MSG_RESULT($with_zlib)
	if test "$with_zlib" = no; then
		AC_DEFINE(NO_LIBZ, 1, [Define to disable zlib functionality])
	fi

	AC_ARG_WITH(zlib-h,
		[  --with-zlib-h=DIR       zlib header location],
		[LIBZ_H_DIR=$withval; with_zlib=yes])

	AC_ARG_WITH(zlib-lib,
		[  --with-zlib-lib=DIR     zlib library location],
		[LIBZ_LIB_DIR=$withval; with_zlib=yes])

	if test "$with_zlib" = yes; then
		CF_LIB_Z_IMPL
	fi
])

AC_DEFUN([CF_LIB_Z_IMPL],[
	if test -n "$LIBZ_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit <zlib.h> path $LIBZ_H_DIR])
		AC_CHECK_HEADERS($LIBZ_H_DIR/zlib,h,[
			CFLAGS="$CFLAGS -I$LIBZ_H_DIR"
			zlib_header=yes],
			[AC_MSG_ERROR([<zlib.h> not found at specified location])])
	else
		AC_CHECK_HEADERS(zlib.h,[have_zlib_header=yes])
	fi

	LIBZ=
	if test "$have_zlib_header" = yes; then
		AC_CHECK_LIB(z, gzopen, have_zlib_lib=yes)
		if test "$have_zlib_lib" = yes; then
			AC_DEFINE(HAVE_LIBZ, 1, [Define if zlib is available])
			LIBZ="-lz"
		fi
	fi
	AC_SUBST(LIBZ)
])

dnl end
